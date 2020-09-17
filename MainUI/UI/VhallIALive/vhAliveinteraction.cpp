#include "vhAliveinteraction.h"
#include <QDebug>
#include <QJsonArray>
#include "DebugTrace.h"
#include "ConfigSetting.h"
#include "pathManager.h"
#include "pathManage.h"
#include "VhallNetWorkInterface.h"
#include <QCoreApplication>
#include "VhallUI_define.h"
#include "ICommonData.h"
#include "Msg_CommonToolKit.h"
#include "VH_Macro.h"
#include "MainUIIns.h"
#include "IMainUILogic.h"
#include "CRPluginDef.h"

using namespace vlive;
bool AnalysisEventObject(QJsonObject &obj, Event &e) {
    if (obj.empty()) {
        return false;
    }
    e.m_eMsgType = (MsgRQType)obj["m_eMsgType"].toInt();
    e.m_iPageCount = obj["m_iPageCount"].toInt();
    e.m_currentPage = obj["m_currentPage"].toInt();
    e.m_sumNumber = obj["m_sumNumber"].toInt();
    e.context = obj["context"].toString();
    e.pushTime = obj["pushTime"].toString();
    e.m_bResult = obj["m_bResult"].toBool();
    e.meDevice = (eDevice)obj["Device"].toInt();
    e.m_bSendMsg = obj["m_bSendMsg"].toInt();
    e.m_memberSlider = obj["m_memberSlider"].toInt();

    QJsonArray m_oUserList = obj["m_oUserList"].toArray();

    for (int i = 0; i<m_oUserList.count(); i++) {
        QJsonObject userObj = m_oUserList[i].toObject();
        QString szUserName = userObj["m_szUserName"].toString();
        QString szUserID = userObj["m_szUserID"].toString();
        QString szRole = userObj["m_szRole"].toString();
        QString headImage = userObj["m_avatar"].toString();
        int devType = userObj["m_devType"].toInt();
        VhallAudienceUserInfo userInfo;
        userInfo.userName = szUserName.toStdWString();        userInfo.userId = szUserID.toStdWString();
        userInfo.role = szRole.toStdWString();
        userInfo.headImage = headImage.toStdWString();
        userInfo.mDevType = devType;
        userInfo.gagType = (VhallShowType)userObj["m_isbanded"].toInt();
        userInfo.kickType = (VhallShowType)userObj["m_iskickout"].toInt();
        userInfo.mIsRtcPublisher = userObj["m_is_speak"].toInt();
        e.m_oUserList.push_back(userInfo);
    }

    QJsonObject userInfo = obj["userInfo"].toObject();
    QString szUserName = userInfo["m_szUserName"].toString();
    QString szUserID = userInfo["m_szUserID"].toString();
    QString szRole = userInfo["m_szRole"].toString();
    QString headImage = userInfo["m_avatar"].toString();
    int devType = userInfo["m_devType"].toInt();
    bool m_isGag = obj["m_isGag"].toBool();
    e.m_oUserInfo.headImage = headImage.toStdWString();
    e.m_oUserInfo.userName = szUserName.toStdWString();
    e.m_oUserInfo.userId = szUserID.toStdWString();
    e.m_oUserInfo.role = szRole.toStdWString();
    e.m_oUserInfo.gagType = (VhallShowType)obj["m_isbanded"].toInt();
    e.m_oUserInfo.kickType = (VhallShowType)obj["m_iskickout"].toInt();
    e.m_oUserInfo.mIsRtcPublisher = obj["m_is_speak"].toInt();
    e.m_oUserInfo.mDevType = devType;
    e.m_isGag = m_isGag;
    QString m_wzText = obj["m_wzText"].toString();
    strcpy(e.m_wzText, m_wzText.toUtf8().data());
    QJsonArray m_oImageList = obj["m_oImageList"].toArray();
    for (int i = 0; i<m_oImageList.count(); i++) {
        QJsonObject img = m_oImageList[i].toObject();
        if (img.keys().count() > 0) {
            std::string key = img.keys()[0].toStdString();
            std::string value = img.value(QString::fromStdString(key)).toString().toStdString();
            e.m_oImageList[value] = key;
        }
    }
    return true;
}

QJsonObject VHAliveInteraction::MakeEventObject(Event *e) {
    QJsonObject obj;
    obj["m_eMsgType"] = (int)e->m_eMsgType;
    obj["m_iPageCount"] = e->m_iPageCount;
    obj["m_currentPage"] = e->m_currentPage;
    obj["m_sumNumber"] = e->m_sumNumber;
    obj["m_bResult"] = e->m_bResult;
    obj["m_bSendMsg"] = e->m_bSendMsg;
    obj["m_memberSlider"] = e->m_memberSlider;
    QJsonArray m_oUserList;
    for (int i = 0; i < e->m_oUserList.size(); i++) {
        QJsonObject userObj;
        userObj["m_szUserName"] = QString::fromStdWString(e->m_oUserList[i].userName);
        userObj["m_szUserID"] = QString::fromStdWString(e->m_oUserList[i].userId);
        userObj["m_szRole"] = QString::fromStdWString(e->m_oUserList[i].role);
        userObj["m_avatar"] = QString::fromStdWString(e->m_oUserList[i].headImage);
        userObj["m_is_speak"] = e->m_oUserList[i].mIsRtcPublisher;
        userObj["m_devType"] = e->m_oUserList[i].mDevType;
        userObj["m_iskickout"] = (int)e->m_oUserList[i].kickType;
        userObj["m_isbanded"] = (int)e->m_oUserList[i].gagType;
        m_oUserList.append(userObj);
    }

    obj["m_oUserList"] = m_oUserList;
    QJsonObject userInfo;
    userInfo["m_szUserName"] = QString::fromStdWString(e->m_oUserInfo.userName);
    userInfo["m_szUserID"] = QString::fromStdWString(e->m_oUserInfo.userId);
    userInfo["m_szRole"] = QString::fromStdWString(e->m_oUserInfo.role);
    userInfo["m_avatar"] = QString::fromStdWString(e->m_oUserInfo.headImage);
    userInfo["m_is_speak"] = e->m_oUserInfo.mIsRtcPublisher;
    userInfo["m_iskickout"] = (int)e->m_oUserInfo.kickType;
    userInfo["m_isbanded"] = (int)e->m_oUserInfo.gagType;
    userInfo["m_devType"] = e->m_oUserInfo.mDevType;
    obj["m_isGag"] = e->m_isGag;
    obj["userInfo"] = userInfo;
    obj["m_wzText"] = e->m_wzText;
    obj["Device"] = e->meDevice;
    obj["context"] = e->context;
    obj["pushTime"] = e->pushTime;
    QJsonArray m_oImageList;
    if (e->m_oImageList.size() > 0) {
        for (auto itor = e->m_oImageList.begin(); itor != e->m_oImageList.end(); itor++) {
            QJsonObject img;
            img[QString::fromStdString(itor->second)] = QString::fromStdString(itor->first);
            m_oImageList.append(img);
        }
    }
    obj["m_oImageList"] = m_oImageList;
    return obj;
}

void VHAliveInteraction::OnOpen() {
    TRACE6("%s\n", __FUNCTION__);
}

void VHAliveInteraction::OnFail() {
    TRACE6("%s\n", __FUNCTION__);
}

void VHAliveInteraction::OnReconnecting() {
    TRACE6("%s\n", __FUNCTION__);
    if (mEventReciver) {
       QEvent *socketMsg = new QEvent(CustomEvent_WebSocketReconnecting);
       if (socketMsg) {
          QCoreApplication::postEvent(mEventReciver, socketMsg, Qt::LowEventPriority);
       }
    }
}

void VHAliveInteraction::OnReconnect(unsigned code, unsigned num) {
    TRACE6("%s %d %d\n", __FUNCTION__, code, num);
}

void VHAliveInteraction::OnClose() {
    TRACE6("%s\n", __FUNCTION__);
}

void VHAliveInteraction::OnSocketOpen(std::string const& nsp) {
    TRACE6("%s %s\n", __FUNCTION__, nsp.c_str());
    onConnected(nullptr);
    if (mEventReciver) {
       CustomOnHttpResMsgEvent* event = new CustomOnHttpResMsgEvent(CustomEvent_OnRoomSuccessedEvent);
       event->mEventCode = RoomEvent_OnNetWork_Reconnect;
       QCoreApplication::postEvent(mEventReciver, event, Qt::LowEventPriority);
    }

}

void VHAliveInteraction::OnSocketClose(std::string const& nsp) {
    TRACE6("%s  %s\n", __FUNCTION__, nsp.c_str());
}

void VHAliveInteraction::OnRecvChatCtrlMsg(const char* msgType, const char* msg) {

}

void VHAliveInteraction::onConnected(Event* event) {
    TRACE6("VHInteraction::onConnected TRUE!\n");
    RQData vData;

    vData.m_eMsgType = e_RQ_UserOnlineList;                          //在线列表   
    vData.m_iCurPage = 1;
    MessageRQ(vData.m_eMsgType, &vData);

    vData.m_eMsgType = e_RQ_UserProhibitSpeakList;                 //禁言列表
    MessageRQ(vData.m_eMsgType, &vData);

    vData.m_eMsgType = e_RQ_UserKickOutList;                       //踢出列表
    MessageRQ(vData.m_eMsgType, &vData);

    vData.m_eMsgType = e_RQ_UserSpecialList;                       //主持人、嘉宾、助理
    MessageRQ(vData.m_eMsgType, &vData);
    TRACE6("%s end\n", __FUNCTION__);

    if (mEventReciver) {
        QEvent *socketMsg = new QEvent(CustomEvent_SocketIOConnect);
        if (socketMsg) {
            QCoreApplication::postEvent(mEventReciver, socketMsg, Qt::LowEventPriority);
        }
    }
}

void VHAliveInteraction::onMessaged(Event* event) {
    QJsonObject obj = MakeEventObject(event);
    if (mEventReciver) {
        QCoreApplication::postEvent(mEventReciver, new CustomSocketIOMsg(CustomEvent_CustomRecvSocketIOMsgEvent, obj), Qt::LowEventPriority);
    }
}

void VHAliveInteraction::onError(Event* event) {
    TRACE6("VHInteraction::onError!\n");
}

VHAliveInteraction::VHAliveInteraction(bool enableProxy, QObject *parent)
:QObject(parent)
,mEnableProxy(enableProxy){
    TRACE6("%s enter mEnableProxy:%d\n", __FUNCTION__, mEnableProxy);
}

VHAliveInteraction::~VHAliveInteraction() { 
    TRACE6("%s enter mEnableProxy:%d\n", __FUNCTION__, mEnableProxy);
    Destory();
    TRACE6("%s\n", __FUNCTION__);
}

void VHAliveInteraction::SetEventReciver(QObject *obj) {
    mEventReciver = obj;
}

BOOL VHAliveInteraction::Create(){
    SetMessageSocketIOCallBack(nullptr);
    return TRUE;
}

void VHAliveInteraction::Destory() {
    TRACE6("%s enter mEnableProxy:%d\n", __FUNCTION__, mEnableProxy);
    if (mEnableProxy) {
        m_mutex.lock();
        if (m_clinet) {
            TRACE6("%s DisConnect\n", __FUNCTION__);
            m_clinet->DisConnect();
            TRACE6("VHInteraction DisConnect ok \n");
            m_clinet->UnInit();
            TRACE6("VHInteraction::run() end \n");
            DestroyAliveInteractionClient(&m_clinet);
            TRACE6("DestroyAliveInteractionClient end \n");
            m_clinet = NULL;
        }
        m_mutex.unlock();
    }
    else {
        if (mpVHWebSocketInterface) {
            TRACE6("%s DisConnectWebSocket\n", __FUNCTION__);
            mpVHWebSocketInterface->DisConnectWebSocket();
            TRACE6("%s SyncDisConnectServer\n", __FUNCTION__);
            mpVHWebSocketInterface->SyncDisConnectServer();
            TRACE6("%s SyncDisConnectServer end\n", __FUNCTION__);
            delete mpVHWebSocketInterface;
            mpVHWebSocketInterface = nullptr;
        }
        if (mWebSocketMsgParamClient != NULL) {
            DestroyAliveInteractionClient(&mWebSocketMsgParamClient);
            mWebSocketMsgParamClient = NULL;
        }
    }
    emit this->SigUnInitFinished();
    TRACE6("%s\n", __FUNCTION__);
}

BOOL VHAliveInteraction::Init(Key_Data &oKeyData) {
   TRACE6("%s enter mEnableProxy:%d\n", __FUNCTION__, mEnableProxy);
   mKeyData = oKeyData;
   if (mEnableProxy) {
       QByteArray ba = QByteArray((char *)&oKeyData, sizeof(Key_Data));
       InitSocketThread(ba);
   }
   else {
       if (mpVHWebSocketInterface == nullptr) {
          TRACE6("%s mpVHWebSocketInterface is empty\n", __FUNCTION__);
           if (mWebSocketMsgParamClient == nullptr) {
               mWebSocketMsgParamClient = CreateAliveInteractionClient();
               mWebSocketMsgParamClient->RegisterDelegate(this);
           }
           mpVHWebSocketInterface = CreateVHWebSocket();
           if (mpVHWebSocketInterface) {
              TRACE6("%s CreateVHWebSocket\n", __FUNCTION__);
               mpVHWebSocketInterface->RegisterCallback(this);
               mpVHWebSocketInterface->On("cmd", [&](const std::string& name, const std::string& msg)->void {
                   TRACE6("%s cmd msg :%s\n", __FUNCTION__, msg.c_str());
                   mWebSocketMsgParamClient->OnHttpMsg(QString::fromStdString(name), QString::fromStdString(msg));
               });
               mpVHWebSocketInterface->On("flashMsg", [&](const std::string& name, const std::string& msg)->void {               
                   mWebSocketMsgParamClient->OnHttpMsg(QString::fromStdString(name), QString::fromStdString(msg));
               });
               mpVHWebSocketInterface->On("msg", [&](const std::string& name, const std::string& msg)->void {
                  mWebSocketMsgParamClient->OnHttpMsg(QString::fromStdString(name), QString::fromStdString(msg));
               });
               QString msg_URL = QString::fromWCharArray(mKeyData.m_msgURL);
               QString qtoken = QString::fromWCharArray(mKeyData.m_szMsgToken);
               TRACE6("%s ConnectServer url :%s ", __FUNCTION__, msg_URL.toStdString().c_str());
               mpVHWebSocketInterface->ConnectServer(msg_URL.toStdString().c_str(),qtoken.toStdString().c_str());
           }
       }
   }
   TRACE6("%s Leave\n",__FUNCTION__);
   return true;
}

void VHAliveInteraction::InitSocketThread(QByteArray pKeyData) {
    TRACE6("VHInteraction::SlotInit\n");
    if (m_bInit) {
        TRACE6("VHInteraction::SlotInit m_bInit true\n");
        UnInitSocketThread();
        m_mutex.lock();
        if (m_clinet) {
            m_clinet->UnInit();
            TRACE6("%s uninit end \n",__FUNCTION__);
		    DestroyAliveInteractionClient(&m_clinet);
            m_clinet = NULL;
        }
        m_mutex.unlock();
    }
    TRACE6("VHInteraction::SlotInit CreateInteractionClient\n");
    m_mutex.lock();
    if (m_clinet != NULL) {
	    DestroyAliveInteractionClient(&m_clinet);
        m_clinet = NULL;
    }
    m_clinet = CreateAliveInteractionClient();
    m_clinet->SetMessageSocketIOCallBack(m_socketIOCallBack);
    Key_Data oKeyData = *(Key_Data *)pKeyData.data();
    TRACE6("VHInteraction::SlotInit CreateInteractionClient Init\n");
    m_clinet->Init(this, oKeyData);
    TRACE6("VHInteraction::SlotInit CreateInteractionClient ConnectServer\n");
    m_clinet->ConnectServer();
    m_mutex.unlock();
    m_bInit = true;
    TRACE6("VHInteraction::SlotInit CreateInteractionClient return\n");
}

void VHAliveInteraction::UnInitSocketThread() {
   TRACE6("VHInteraction::SlotUnInit\n");
   if (!m_clinet || m_bInit != true) {
      TRACE6("VHInteraction::SlotUnInit Failed!\n");
      return;
   }
   m_bInit = false;
   TRACE6("VHInteraction::SlotUnInit TRUE!\n");
}

void VHAliveInteraction::MessageRQ(MsgRQType eMsgType, RQData* vData) {   
    WCHAR wzRQUrl[2048] = { 0 };
    switch (eMsgType) {
        case e_RQ_UserOnlineList: {//在线列表   
            TRACE6("%s e_RQ_UserOnlineList\n", __FUNCTION__);
            if (vData) {
                vData->m_iCurPage = vData->m_iCurPage <= 0 ? 1 : vData->m_iCurPage;
                _snwprintf_s(wzRQUrl, 2048, L"%s%s?token=%s&curr_page=%d", mKeyData.m_MSG_SERVER_URL, L"onlineusers", mKeyData.m_szMsgToken, vData->m_iCurPage);
            }
            break;
        }
        case e_RQ_UserProhibitSpeakList: {//禁言列表
            TRACE6("%s e_RQ_UserProhibitSpeakList\n", __FUNCTION__);
            _snwprintf_s(wzRQUrl, 2048, L"%s%s?token=%s", mKeyData.m_MSG_SERVER_URL, L"silencedusers", mKeyData.m_szMsgToken);
            break;
        }
        case e_RQ_UserKickOutList: {//踢出列表
            TRACE6("%s e_RQ_UserProhibitSpeakList\n", __FUNCTION__);
            _snwprintf_s(wzRQUrl, 2048, L"%s%s?token=%s", mKeyData.m_MSG_SERVER_URL, L"kickedusers", mKeyData.m_szMsgToken);
            break;
        }
        case e_RQ_UserSpecialList: {//主持人、嘉宾、助理
            TRACE6("%s e_RQ_UserProhibitSpeakList\n", __FUNCTION__);
            _snwprintf_s(wzRQUrl, 2048, L"%s%s?token=%s", mKeyData.m_MSG_SERVER_URL, L"specialusers", mKeyData.m_szMsgToken);
            break;
        }
        case e_RQ_UserProhibitSpeakOneUser: {
            if (vData) {
                _snwprintf_s(wzRQUrl, 2048, L"%s%s?token=%s&user_id=%s&type=1", mKeyData.m_MSG_SERVER_URL, L"silence", mKeyData.m_szMsgToken, vData->m_oUserInfo.userId.c_str());
            }
            break;
        }
        case e_RQ_UserAllowSpeakOneUser: {
            if (vData) {
                _snwprintf_s(wzRQUrl, 2048, L"%s%s?token=%s&user_id=%s&type=2", mKeyData.m_MSG_SERVER_URL, L"silence", mKeyData.m_szMsgToken, vData->m_oUserInfo.userId.c_str());
            }
            break;
        }
        case e_RQ_UserKickOutOneUser: {
            if (vData) {
                _snwprintf_s(wzRQUrl, 2048, L"%s%s?token=%s&user_id=%s", mKeyData.m_MSG_SERVER_URL, L"kick", mKeyData.m_szMsgToken, vData->m_oUserInfo.userId.c_str());
            }
            break;
        }
        case e_RQ_UserAllowJoinOneUser: {
            if (vData) {
                _snwprintf_s(wzRQUrl, 2048, L"%s%s?token=%s&user_id=%s", mKeyData.m_MSG_SERVER_URL, L"restorekicked", mKeyData.m_szMsgToken, vData->m_oUserInfo.userId.c_str());
            }
            break;
        }
        case e_RQ_ReleaseAnnouncement: {
            QString url = QString(mKeyData.m_domain) + "/api/webinar/v1/webinar/sendmsg?token=" + QString::fromWCharArray(mKeyData.m_szMsgToken) + "&event=cmd&msg=" + QString(vData->m_wzText);
            url.toWCharArray(wzRQUrl);
            break;
        }
        default: {
            return;
            break;
        }
    }

    QString httpUrl = QString::fromStdWString(wzRQUrl);
    QString configPath = CPathManager::GetConfigPath();
    QString proxyHost;
    QString proxyUsr;
    QString proxyPwd;
    int proxyPort;
    int is_http_proxy = ConfigSetting::ReadInt(configPath, GROUP_DEFAULT, PROXY_OPEN, 0);
    if (is_http_proxy) {
        TRACE6("%s is_http_proxy:%d\n", __FUNCTION__, is_http_proxy);
        proxyHost = ConfigSetting::ReadString(configPath, GROUP_DEFAULT, PROXY_HOST, "");
        proxyPort = ConfigSetting::ReadInt(configPath, GROUP_DEFAULT, PROXY_PORT, 80);
        proxyUsr = ConfigSetting::ReadString(configPath, GROUP_DEFAULT, PROXY_USERNAME, "");
        proxyPwd = ConfigSetting::ReadString(configPath, GROUP_DEFAULT, PROXY_PASSWORD, "");
    }
    QObject *recvObj = mEventReciver;
    if (!httpUrl.isEmpty()) {
        HTTP_GET_REQUEST request(httpUrl.toStdString());
        request.SetHttpPost(true);
        QString user_id = QString::fromStdWString(vData->m_oUserInfo.userId);
        GetHttpManagerInstance()->HttpGetRequest(request, [&, eMsgType, httpUrl, recvObj, user_id](const std::string& msg, int code, const std::string userData) {
            if (msg.size() > 0 && code == 0) {
                Event oReq;
                oReq.Init();
                oReq.m_eMsgType = eMsgType;
                VHAliveInteraction::ParseJSon(eMsgType, QString::fromStdString(msg), oReq);
                if (eMsgType == e_RQ_UserProhibitSpeakOneUser || eMsgType == e_RQ_UserAllowSpeakOneUser || eMsgType == e_RQ_UserKickOutOneUser || eMsgType == e_RQ_UserAllowJoinOneUser || eMsgType == e_RQ_ReleaseAnnouncement) {
                   VhallAudienceUserInfo oUserInfo;
                   oUserInfo.userId = user_id.toStdWString();
                   oReq.m_oUserList.push_back(oUserInfo);
                }
                QJsonObject obj = VHAliveInteraction::MakeEventObject(&oReq);
                if (recvObj) {
                    CustomSocketIOMsg *socketMsg = new CustomSocketIOMsg(CustomEvent_CustomRecvSocketIOMsgEvent, obj);
                    if (socketMsg) {
                        QCoreApplication::postEvent(recvObj, socketMsg, Qt::LowEventPriority);
                    }
                }
            }
        });
    }
}

void VHAliveInteraction::ParseJSon(const MsgRQType& type, const QString& qsJson, Event& oEvent) {
    oEvent.m_eventType = e_eventType_msg;
    QJsonObject ObjValue = CPathManager::GetJsonObjectFromString(qsJson);
    switch (type) {
    case e_RQ_UserOnlineList:				 //上线列表
    case e_RQ_UserProhibitSpeakList:  //禁言用户列表
    case e_RQ_UserKickOutList:			//踢出列表
    case e_RQ_UserSpecialList: {         //麦上列表
        QString code = ObjValue.value("code").toString();
        QString msg = ObjValue.value("msg").toString();
        if ("200" == code && "success" == msg) {
            if (ObjValue.value("data").isObject()){
                QJsonObject ObjData = ObjValue.value("data").toObject();
                if (e_RQ_UserOnlineList == type) {
                    int pageCount = ObjData.value("total_page").toInt();
                    oEvent.m_iPageCount = pageCount;
                    int currentPage = CPathManager::GetObjStrValue(ObjData, "curr_page").toInt();// ObjData.value("curr_page").toInt();
                    oEvent.m_currentPage = currentPage;
                    int sumNumber = ObjData.value("count").toInt();
                    oEvent.m_sumNumber = sumNumber;
                }
                if (ObjData.value("users").isArray()){
                    QJsonArray jsonUsers = ObjData.value("users").toArray();
                    int iSize = jsonUsers.size();
                    QJsonObject tmp;
                    for (int i = 0; i < iSize; i++) {
                        tmp = jsonUsers.at(i).toObject();
                        QString user_id = tmp.value("id").toString();
                        QString nick_name = tmp.value("nick_name").toString();
                        QString role = tmp.value("role_name").toString();
                        VhallAudienceUserInfo oUserInfo;
                        if (e_RQ_UserOnlineList == type || e_RQ_UserSpecialList == type) {
                            if (tmp.contains("device_type")) {
                                if (tmp.value("device_type").isString()) {
                                   oUserInfo.mDevType = tmp.value("device_type").toString().toInt();
                                }
                                else {
                                   oUserInfo.mDevType = tmp.value("device_type").toInt();
                                }
                            }
                            if (tmp.contains("is_speak")) {
                               oUserInfo.mIsRtcPublisher = tmp.value("is_speak").toInt();
                            }
                            if (tmp.contains("is_gag")) {
                               oUserInfo.gagType = (VhallShowType)tmp.value("is_gag").toInt();
                            }
                            if (tmp.contains("is_kicked")) {
                               oUserInfo.kickType = (VhallShowType)tmp.value("is_kicked").toInt();
                            }
                            if (tmp.contains("avatar")) {
                               QString avatar = tmp.value("avatar").toString();
                               oUserInfo.headImage = avatar.toStdWString();
                            }
                        }
                        oUserInfo.userId = user_id.toStdWString();
                        oUserInfo.userName = nick_name.toStdWString();
                        oUserInfo.role = role.toStdWString();
                        oEvent.m_oUserList.push_back(oUserInfo);
                    }
                }
            }
        }
        else if ("500" == code) {
            if (e_RQ_UserOnlineList == type){
                QJsonObject ObjData = ObjValue.value("data").toObject();
                int sumNumber = ObjData.value("count").toInt();
                oEvent.m_sumNumber = sumNumber;
            }
        }
        oEvent.m_eMsgType = type;
        break;
    }                
    case e_RQ_ChatEmotionList: { //表情列表
        QString code = ObjValue.value("code").toString();
        QString msg = ObjValue.value("msg").toString();
        if ("200" == code && "success" == msg) {
            if (ObjValue.value("data").isArray()){
                QJsonArray ObjData = ObjValue.value("data").toArray();
                int iSize = ObjData.size();
                QJsonObject ObjTmp;
                for (int i = 0; i < iSize; i++){
                    ObjTmp = ObjData.at(i).toObject();
                    QStringList names = ObjTmp.keys();
                    QString name = names[0];
                    QString strPath = ObjTmp.value(name).toString();
                    oEvent.m_oImageList[name.toStdString()] = strPath.toStdString();
                }
                oEvent.m_eMsgType = type;
            }
        }
        break;
    }
    default:
        break;
    }
}

//socketIO 回调
void VHAliveInteraction::SetMessageSocketIOCallBack(InteractionMsgCallBack callback) {
    if (NULL == callback) {
        TRACE6("%s :callback IS NULL\n", __FUNCTION__);
        m_socketIOCallBack = callback;
        return;
    }
    m_socketIOCallBack = callback;
    if (mEnableProxy) {
        m_mutex.lock();
        if (m_clinet) {
            m_clinet->SetMessageSocketIOCallBack(m_socketIOCallBack);
        }
        m_mutex.unlock();
    }
    else {
        if (mWebSocketMsgParamClient == nullptr) {
            mWebSocketMsgParamClient = CreateAliveInteractionClient();
            mWebSocketMsgParamClient->RegisterDelegate(this);
        }
        if (mWebSocketMsgParamClient) {
            mWebSocketMsgParamClient->SetMessageSocketIOCallBack(m_socketIOCallBack);
        }
    }
}

void VHAliveInteraction::SetHttpProxy(bool enable, const char *proxy_ip, const char* proxy_port, const char *proxy_username, const char *proxy_password) {
   if (m_clinet) {
      m_clinet->SetProxyAddr(enable, proxy_ip, proxy_port, proxy_username, proxy_password);
   }
}
