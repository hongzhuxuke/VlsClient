#include "MemberParam.h"
#include <windows.h>
#include "IInteractionClient.h"
#include "vhalluserinfomanager.h"


MemberParam::MemberParam(QObject *parent)
    : QObject(parent)
{
}

MemberParam::~MemberParam()
{
}

void MemberParam::ParamToFlashMemberInfo(int libCurlCode, const QString& msg, int memberListType, int& outHttpCode, QString &outMsg, Event* memberInfo) {
   QByteArray ba(msg.toStdString().c_str(), msg.length());
   QJsonDocument doc = QJsonDocument::fromJson(ba);
   QJsonObject ObjValue = doc.object();
   if (ObjValue.contains("code")) {
      if (ObjValue["code"].isString()) {
         outHttpCode = ObjValue["code"].toString().toInt();
      }
      else {
         outHttpCode = ObjValue["code"].toInt();
      }
   }

   QString code = ObjValue.value("code").toString();
   QString msgValue = ObjValue.value("msg").toString();
   if ("200" == code && "success" == msgValue) {
      if (ObjValue.value("data").isObject())
      {
         QJsonObject ObjData = ObjValue.value("data").toObject();
         if (ObjData.value("users").isArray()){
            QJsonArray jsonUsers = ObjData.value("users").toArray();
            int iSize = jsonUsers.size();
            QJsonObject tmp;
            for (int i = 0; i < iSize; i++) {
               VhallAudienceUserInfo oUserInfo;
               tmp = jsonUsers.at(i).toObject();
               QString user_id = tmp.value("id").toString();
               QString nick_name = tmp.value("nick_name").toString();
               QString role = tmp.value("role_name").toString();
               oUserInfo.userName = nick_name.toStdWString();
               oUserInfo.userId = user_id.toStdWString();
               oUserInfo.role = role.toStdWString();
               memberInfo->m_oUserList.push_back(oUserInfo);
            }
         }
      }
   }
}


void MemberParam::ParamToMemberInfo(int libCurlCode, const QString& msg,int memberListType, int& outHttpCode, QString &outMsg, Event* memberInfo) {
    QByteArray ba(msg.toStdString().c_str(), msg.length());
    QJsonDocument doc = QJsonDocument::fromJson(ba);
    QJsonObject obj = doc.object();
    if (obj.contains("code")) {
        if (obj["code"].isString()) {
            outHttpCode = obj["code"].toString().toInt();
        }
        else {
            outHttpCode = obj["code"].toInt();
        }
    }
    
    if (obj.contains("msg") && obj["msg"].isString()) {
        outMsg = obj["msg"].toString();
    }
    memberInfo->m_eMsgType = (MsgRQType)memberListType;
    memberInfo->m_eventType = e_eventType_msg;
    if (outHttpCode == 200) {
        //特殊成员列表只有一页
        if (obj.contains("data") && obj["data"].isArray()) {
            QJsonArray dataArray = obj["data"].toArray();
            for (int i = 0; i < dataArray.count(); i++) {
                QJsonObject memberObj = dataArray.at(i).toObject();
                int nRet = ParamToDetailMember(memberListType,memberObj, memberInfo);
                if (nRet == -1 && memberListType == e_RQ_UserOnlineList) {
                    if (memberInfo->m_sumNumber > 0) {
                        memberInfo->m_sumNumber--;
                    }
                }
            }
        }
        else {
            if (obj.contains("data") && obj["data"].isObject()) {
                QJsonObject data = obj["data"].toObject();
                //总人数
                if (data.contains("total")) {
                    if (data["total"].isString()) {
                        memberInfo->m_sumNumber = data["total"].toString().toInt();
                    }
                    else {
                        memberInfo->m_sumNumber = data["total"].toInt();
                    }
                }
                //每页人数
                if (data.contains("pagesize")) {
                    int pagesize = 0;
                    if (data["pagesize"].isString()) {
                        pagesize = data["pagesize"].toString().toInt();
                    }
                    else {
                        pagesize = data["pagesize"].toInt();
                    }
                    memberInfo->m_iPageCount = (memberInfo->m_sumNumber + pagesize)/ pagesize;
                }
                //当前页
                if (data.contains("page")) {
                    if (data["page"].isString()) {
                        memberInfo->m_currentPage = data["page"].toString().toInt();
                    }
                    else {
                        memberInfo->m_currentPage = data["page"].toInt();
                    }
                }

                QJsonArray dataArray = data["list"].toArray();
                for (int i = 0; i < dataArray.count(); i++) {
                    QJsonObject memberObj = dataArray.at(i).toObject();
                    int nRet = ParamToDetailMember(memberListType,memberObj, memberInfo);
                    if (nRet == -1 && memberListType == e_RQ_UserOnlineList) {
                        if (memberInfo->m_sumNumber > 0) {
                            memberInfo->m_sumNumber--;
                        }
                    }
                }
            }
        }
    }
}

int MemberParam::ParamToDetailMember(int memberListType,QJsonObject memberObj, Event* memberInfo) {
    QString user_id;
    QString nick_name;
    QString account_id;
    QString role;
    int devType = 2;
    VhallAudienceUserInfo oUserInfo;
    if (memberObj.contains("join_id")) {
        user_id = QString::number(memberObj["join_id"].toInt());
    }
    //1主持人2观众3助理4嘉宾
    if (memberObj.contains("role_name") && memberObj["role_name"].isString()) {
        role = memberObj["role_name"].toString();
        if (role == "1") {
            role = USER_HOST;
        }
        else if (role == "2") {
            role = USER_USER;
        }
        else if (role == "3") {
            role = USER_ASSISTANT;
        }
        else if (role == "4") {
            role = USER_GUEST;
        }
    }
    if (memberObj.contains("nickname") && memberObj["nickname"].isString()) {
        nick_name = memberObj["nickname"].toString();
    }
    if (memberObj.contains("account_id") && memberObj["account_id"].isString()) {
        account_id = memberObj["account_id"].toString();
        //用户ID替换成accout_id 。paas平台统一使用此id
        user_id = account_id;
    }
    if (memberObj.contains("is_kicked")) {
       oUserInfo.kickType = (VhallShowType)memberObj["is_kicked"].toInt();
    }
    if (memberObj.contains("is_banned")) {
       oUserInfo.gagType = (VhallShowType)memberObj["is_banned"].toInt();
    }
    if (memberObj.contains("device_type")) {
        if (memberObj["device_type"].isString()) {
           oUserInfo.mDevType = memberObj["device_type"].toString().toInt();
        }
        else {
           oUserInfo.mDevType = memberObj["device_type"].toInt();
        }
    }
    if (memberObj.contains("device_status")) {
       if (memberObj["device_status"].isString()) {
          oUserInfo.mDevStatus = memberObj["device_status"].toString().toInt();
       }
       else {
          oUserInfo.mDevStatus = memberObj["device_status"].toInt();
       }
    }
    if (memberObj.contains("is_speak")) {
       if (memberObj["is_speak"].isString()) {
          oUserInfo.mIsRtcPublisher = memberObj["is_speak"].toString().toInt();
       }
       else {
          oUserInfo.mIsRtcPublisher = memberObj["is_speak"].toInt();
       }
    }
    if (memberObj.contains("avatar")) {
       if (memberObj["avatar"].isString()) {
          QString avatar = memberObj["avatar"].toString();
          oUserInfo.headImage = avatar.toStdWString();
       }
    }
    oUserInfo.userName = nick_name.toStdWString();
    oUserInfo.userId = user_id.toStdWString();
    oUserInfo.role = role.toStdWString();
    oUserInfo.account_id = account_id.toStdWString();
    memberInfo->m_oUserList.push_back(oUserInfo);
    return 0;
}