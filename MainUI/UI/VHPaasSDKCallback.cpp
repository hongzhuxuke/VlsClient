#include "VHPaasSDKCallback.h"
#include "WebRtcSDKInterface.h"
#include <QCoreApplication>
#include <QApplication>
#include "vlive_def.h"   
#include "VhallUI_define.h"
#include "IInteractionClient.h"
#include "DebugTrace.h"

#include "ICommonData.h"
#include "CRPluginDef.h"
#include "MainUIIns.h"
#include "Msg_MainUI.h"
#include "pathManager.h"
#include "pathManage.h"
#include "VhallNetWorkInterface.h"
#include "ConfigSetting.h"
#include "VH_ConstDeff.h"

#define USER_HOST "host"
#define USER_GUEST "guest"
#define USER_ASSISTANT "assistant"
#define USER_USER "user"

#define Def_service_type    "service_type"
#define Def_channel    "channel"
#define Def_service_room    "service_room"
#define Def_service_document    "service_document"
#define Def_data            "data"
#define Def_type            "type"
#define Def_target_id       "target_id"
#define Def_vrtc_definition "vrtc_definition"
#define Def_room_join_id    "room_join_id"
#define Def_nick_name       "nick_name"
#define Def_room_role       "room_role"
#define Def_role_name       "role_name"
#define Def_prohibit_speak  "prohibit_speak"
#define Def_kicked_out      "kicked_out"
#define Def_device_type     "device_type"  //设备类型0其他1手机端2PC
#define Def_device_status   "device_status" //设备状态0未检测1可上麦2不可上麦
#define Def_room_announcement_text  "room_announcement_text"

//event
#define Def_live_start                      "live_start" //开始直播
#define Def_live_over                       "live_over"  //结束直播

//嘉宾请求上麦处理
#define Def_vrtc_connect_apply	           "vrtc_connect_apply"            //互动连麦连接申请 (嘉宾请求上麦)
#define Def_vrtc_connect_apply_cancel		  "vrtc_connect_apply_cancel"     //互动连麦连接申请取消（嘉宾取消请求上麦）
#define Def_vrtc_connect_agree		        "vrtc_connect_agree"            //互动连麦连接同意（主持人同意嘉宾上麦）
#define Def_vrtc_connect_refused            "vrtc_connect_refused"          //互动连麦连接拒绝（主持人拒绝嘉宾上麦）

//主持人邀请用户上麦
#define Def_vrtc_connect_invite             "vrtc_connect_invite"           //互动连麦连接邀请 （主持人邀请嘉宾上麦）
#define Def_vrtc_connect_invite_refused     "vrtc_connect_invite_refused"   //互动连麦连接邀请拒绝 （嘉宾拒绝主持人嘉宾上麦）
#define Def_vrtc_connect_invite_agree       "vrtc_connect_invite_agree"     //互动连麦连接邀请同意 （嘉宾同意主持人嘉宾上麦）

#define Def_vrtc_connect_success            "vrtc_connect_success"          //上麦
#define Def_vrtc_disconnect_success         "vrtc_disconnect_success"       //下麦

#define Def_vrtc_big_screen_set             "vrtc_big_screen_set"           //设置大画面
#define Def_vrtc_speaker_switch             "vrtc_speaker_switch"           //切换主讲人
#define Def_disable                         "disable"                       ////禁言
#define Def_permit                          "permit"                        ////取消禁言
#define Def_room_kickout                    "room_kickout"                  //踢出
#define Def_room_kickout_cancel             "room_kickout_cancel"           //取消踢出
#define Def_vrtc_connect_open               "vrtc_connect_open"             //允许举手
#define Def_vrtc_connect_close              "vrtc_connect_close"            //关闭举手

//设备
#define Def_vrtc_mute_cancel                "vrtc_mute_cancel"              //开启麦克风
#define Def_vrtc_mute                       "vrtc_mute"                     //关闭麦克风
#define Def_vrtc_frames_display             "vrtc_frames_display"           //开启摄像头
#define Def_vrtc_frames_forbid              "vrtc_frames_forbid"            //关闭摄像头

#define Def_room_announcement               "room_announcement"             //公告
#define Def_vrtc_definition_set             "vrtc_definition_set"           //设置清晰度
#define Def_question_answer_open            "question_answer_open"          //问答开启
//#define Def_vrtc_connect_apply_cancel     "vrtc_connect_apply_cancel"

using namespace vlive;
VHPaasSDKCallback::VHPaasSDKCallback(QObject *parent)
   : QObject(parent)
{
}

VHPaasSDKCallback::~VHPaasSDKCallback()
{
}

void VHPaasSDKCallback::SetEventReciver(QObject* mainLogic) {
   mainLogicReciver = mainLogic;
}

void VHPaasSDKCallback::SetVLiveExit(bool exit) {
   mbIsExit = exit;
}

/*
 *   监听互动房间内HTTP业务API调用成功事件
 *   code: RoomEvent 事件类型
 *   userData: 附带的json数据
 */
void VHPaasSDKCallback::OnSuccessedEvent(RoomEvent code, std::string userData/* = std::string()*/) {
   CustomOnHttpResMsgEvent* event = new CustomOnHttpResMsgEvent(CustomEvent_OnRoomSuccessedEvent);
   event->mMsg = QString::fromStdString(userData);
   event->mEventCode = code;
   QCoreApplication::postEvent(mainLogicReciver, event, Qt::LowEventPriority);
}
/*
*   监听互动房间内HTTP业务API调用失败事件
*   respCode: 错误码
*   msg：错误信息
*   userData: 附带的json数据
*/
void VHPaasSDKCallback::OnFailedEvent(RoomEvent code, int libCurlCode,  std::string msg) {
   CustomOnHttpResMsgEvent* event = new CustomOnHttpResMsgEvent(CustomEvent_OnRoomFailedEvent);
   event->mCode = libCurlCode;
   event->mMsg = QString::fromStdString(msg);
   event->mEventCode = code;
   QCoreApplication::postEvent(mainLogicReciver, event, Qt::LowEventPriority);
}
/*
*   获取互动房间成员列表回调
**/
void VHPaasSDKCallback::OnGetVHRoomMembers(const std::string& third_party_user_id, std::list<VHRoomMember>&) {

}
/*
*   获取被踢出互动房间人列表回调
**/
void VHPaasSDKCallback::OnGetVHRoomKickOutMembers(std::list<VHRoomMember>&) {

}
/*
*   接收CMD或这Flash消息
*/

QJsonObject MakeEventObject(Event *e) {
   QJsonObject obj;
   obj["m_eMsgType"] = (int)e->m_eMsgType;
   obj["m_iPageCount"] = e->m_iPageCount;
   obj["m_currentPage"] = e->m_currentPage;
   obj["m_sumNumber"] = e->m_sumNumber;
   obj["m_bResult"] = e->m_bResult;
   QJsonArray m_oUserList;
   for (int i = 0; i < e->m_oUserList.size(); i++) {
      QJsonObject userObj;
      userObj["m_szUserName"] = QString::fromStdWString(e->m_oUserList[i].userName);
      userObj["m_szUserID"] = QString::fromStdWString(e->m_oUserList[i].userId);
      userObj["m_szRole"] = QString::fromStdWString(e->m_oUserList[i].role);
      userObj["m_avatar"] = QString::fromStdWString(e->m_oUserList[i].headImage);
      m_oUserList.append(userObj);
   }

   obj["m_oUserList"] = m_oUserList;
   QJsonObject userInfo;
   userInfo["m_szUserName"] = QString::fromStdWString(e->m_oUserInfo.userName);
   userInfo["m_szUserID"] = QString::fromStdWString(e->m_oUserInfo.userId);
   userInfo["m_szRole"] = QString::fromStdWString(e->m_oUserInfo.role);
   userInfo["m_avatar"] = QString::fromStdWString(e->m_oUserInfo.headImage);
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

void ParamTypeToEvent(const QString typeData, Event &oEvent) {
   if (typeData == Def_live_start) {
      oEvent.m_eMsgType = e_RQ_setPublishStart;
      oEvent.m_eventType = e_eventType_msg;
   }
   else if (typeData == Def_question_answer_open) {
      oEvent.m_eMsgType = e_RQ_question_answer_open;
      oEvent.m_eventType = e_eventType_msg;
   }
   else if (typeData == Def_live_over) {
      oEvent.m_eMsgType = e_RQ_setOver;
      oEvent.m_eventType = e_eventType_msg;
   }
   else if (Def_vrtc_connect_apply == typeData) {
      oEvent.m_eMsgType = e_RQ_handsUp;
      oEvent.m_eventType = e_eventType_msg;
   }
   else if (Def_vrtc_connect_apply_cancel == typeData) {
      oEvent.m_eMsgType = e_RQ_CancelHandsUp;
      oEvent.m_eventType = e_eventType_msg;
   }
   else if (Def_vrtc_connect_agree == typeData) {
      oEvent.m_eMsgType = e_RQ_agreeSpeaker;
      oEvent.m_eventType = e_eventType_msg;
   }
   else if (Def_vrtc_connect_refused == typeData) {
      oEvent.m_eMsgType = e_RQ_RejectSpeak;
      oEvent.m_eventType = e_eventType_msg;
   }
   else if (Def_vrtc_connect_invite == typeData) {
      oEvent.m_eMsgType = e_RQ_sendInvite;
      oEvent.m_eventType = e_eventType_msg;
   }
   else if (Def_vrtc_connect_invite_refused == typeData) {
      oEvent.m_eMsgType = e_RQ_replyInvite_Fail;
      oEvent.m_eventType = e_eventType_msg;
   }
   else if (Def_vrtc_connect_invite_agree == typeData) {
      oEvent.m_eMsgType = e_RQ_replyInvite_Suc;
      oEvent.m_eventType = e_eventType_msg;
   }
   else if (Def_vrtc_big_screen_set == typeData) {
      oEvent.m_eMsgType = e_RQ_setMainShow;
      oEvent.m_eventType = e_eventType_msg;
   }
   else if (Def_vrtc_speaker_switch == typeData) {
      oEvent.m_eMsgType = e_RQ_setMainSpeaker;
      oEvent.m_eventType = e_eventType_msg;
   }
   else if (Def_disable == typeData || Def_permit == typeData) {
      oEvent.m_eMsgType = Def_disable == typeData ? e_RQ_UserProhibitSpeakOneUser : e_RQ_UserAllowSpeakOneUser;
      oEvent.m_eventType = e_eventType_msg;
   }
   else if (Def_room_kickout == typeData || Def_room_kickout_cancel == typeData) {
      oEvent.m_eMsgType = Def_room_kickout == typeData ? e_RQ_UserKickOutOneUser : e_RQ_UserAllowJoinOneUser;
      oEvent.m_eventType = e_eventType_msg;
   }
   else if (Def_vrtc_connect_open == typeData) {
      oEvent.m_eMsgType = e_RQ_switchHandsup_open;
      oEvent.m_eventType = e_eventType_msg;
   }
   else if (Def_vrtc_connect_close == typeData) {
      oEvent.m_eMsgType = e_RQ_switchHandsup_close;
      oEvent.m_eventType = e_eventType_msg;
   }
   else if (Def_vrtc_mute_cancel == typeData) {
      oEvent.m_eMsgType = e_RQ_switchDevice_open;
      oEvent.m_eventType = e_eventType_msg;
      oEvent.meDevice = eDevice_Mic;
   }
   else if (Def_vrtc_mute == typeData) {
      oEvent.m_eMsgType = e_RQ_switchDevice_close;
      oEvent.m_eventType = e_eventType_msg;
      oEvent.meDevice = eDevice_Mic;
   }
   else if (Def_vrtc_frames_display == typeData) {
      oEvent.m_eMsgType = e_RQ_switchDevice_open;
      oEvent.m_eventType = e_eventType_msg;
      oEvent.meDevice = eDevice_Camera;
   }
   else if (Def_vrtc_frames_forbid == typeData) {
      oEvent.m_eMsgType = e_RQ_switchDevice_close;
      oEvent.m_eventType = e_eventType_msg;
      oEvent.meDevice = eDevice_Camera;
   }
   else if (Def_vrtc_disconnect_success == typeData) {
      oEvent.m_eMsgType = e_RQ_notSpeak;
      oEvent.m_eventType = e_eventType_msg;
   }
   else if (Def_vrtc_connect_success == typeData) {
      oEvent.m_eMsgType = e_RQ_addSpeaker;
      oEvent.m_eventType = e_eventType_msg;
   }
}

void VHPaasSDKCallback::OnRecvSocketIOMsg(SocketIOMsgType msgType, std::string msg) {
   Event oEvent;
   oEvent.m_eventType = e_eventType_msg;
   QByteArray ba(msg.c_str(), msg.length());
   QJsonDocument doc = QJsonDocument::fromJson(ba);
   QJsonObject obj = doc.object();
   if (obj.contains(Def_service_type)) {
      if (obj[Def_service_type].isString()) {
         QString serviceType = obj[Def_service_type].toString();
         QString channel = obj[Def_channel].toString();
         VH::CComPtr<ICommonData> pCommonData;
         DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
         VSSGetRoomInfo attributes;
         pCommonData->GetVSSRoomBaseInfo(attributes);

         if (serviceType == Def_service_room && channel == attributes.channel_id) {
            if (obj.contains(Def_data)) {
               QJsonObject dataobj;
               if (obj[Def_data].isString()) {
                  QString dataMsg = obj[Def_data].toString();
                  QByteArray dataArray(dataMsg.toStdString().c_str(), dataMsg.toStdString().length());
                  QJsonDocument dataDoc = QJsonDocument::fromJson(dataArray);
                  dataobj = dataDoc.object();
               }
               else if (obj[Def_data].isObject()) {
                  dataobj = obj[Def_data].toObject();
               }

               QString strJoinUid, strNickName, strRoleName;
               int iAudioState = obj["audio"].toInt();
               int iVideoState = obj["video"].toInt();
               oEvent.m_bAudioState = (1 == iAudioState);
               oEvent.m_bVideoState = (1 == iVideoState);
               if (dataobj.contains(Def_room_join_id)) {
                  if (dataobj[Def_room_join_id].isString()) {
                     strJoinUid = dataobj[Def_room_join_id].toString();
                  }
                  else {
                     strJoinUid = QString::number(dataobj[Def_room_join_id].toInt());
                  }
               }

               if (dataobj.contains(Def_nick_name)) {
                  if (dataobj[Def_nick_name].isString()) {
                     strNickName = dataobj[Def_nick_name].toString();
                  }
               }

               if (dataobj.contains(Def_room_role) || dataobj.contains(Def_role_name)) {
                  if (dataobj.contains(Def_role_name)) {
                     if (dataobj[Def_role_name].isString()) {
                        strRoleName = dataobj[Def_role_name].toString();
                     }
                     else {
                        strRoleName = QString::number(dataobj[Def_role_name].toInt());
                     }
                  }
                  else {
                     if (dataobj[Def_room_role].isString()) {
                        strRoleName = dataobj[Def_room_role].toString();
                     }
                     else {
                        strRoleName = QString::number(dataobj[Def_room_role].toInt());
                     }
                  }
                  if (dataobj["avatar"].isString()) {
                     oEvent.m_oUserInfo.headImage = dataobj["avatar"].toString().toStdWString();
                  }
                  
                  // （1:老师 2 : 学员 3 : 助教 4 : 嘉宾 5 : 监课）
                  if (strRoleName == "1" || strRoleName == USER_HOST) {
                     strRoleName = USER_HOST;
                  }
                  else if (strRoleName == "2" || strRoleName == USER_USER) {
                     strRoleName = USER_USER;
                  }
                  else if (strRoleName == "3" || strRoleName == USER_ASSISTANT) {
                     strRoleName = USER_ASSISTANT;
                  }
                  else if (strRoleName == "4" || strRoleName == USER_GUEST) {
                     strRoleName = USER_GUEST;
                  }
               }
               if (dataobj.contains(Def_type)) {
                  QString typeData = dataobj[Def_type].toString();
                  if (Def_room_announcement == typeData) {
                     oEvent.m_eMsgType = e_RQ_ReleaseAnnouncement;
                     oEvent.m_eventType = e_eventType_msg;
                     if (dataobj.contains("room_announcement_text")) {
                        oEvent.context = dataobj["room_announcement_text"].toString();
                     }
                     if (dataobj.contains("push_time")) {
                        oEvent.pushTime = dataobj["push_time"].toString();
                     }
                     VH::CComPtr<ICommonData> pCommonData;
                     DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
                     ClientApiInitResp  mRespInitData;
                     pCommonData->GetInitApiResp(mRespInitData);
                     if (strJoinUid == mRespInitData.user_id) {
                        return;
                     }
                  }
                  else if (Def_vrtc_definition_set == typeData) {
                     oEvent.m_eMsgType = e_RQ_setDefinition;
                     oEvent.m_eventType = e_eventType_msg;
                     if (dataobj.contains(Def_vrtc_definition)) {
                        if (dataobj[Def_vrtc_definition].isString()) {
                           QString definition = dataobj[Def_vrtc_definition].toString();
                           memcpy(oEvent.m_wzText, definition.toStdString().c_str(), definition.toStdString().length());
                        }
                     }
                  }
                  else if (Def_vrtc_connect_apply_cancel == typeData){
                     ParamTypeToEvent(typeData, oEvent);
                  }
                  else {
                     ParamTypeToEvent(typeData, oEvent);
                     if (Def_vrtc_connect_apply == typeData) {
                        if (dataobj.contains("prohibit_speak")) {
                           int prohibit_speak = 0;
                           if (dataobj["dataobj.contains"].isString()) {
                              prohibit_speak = dataobj["prohibit_speak"].toString().toInt();
                           }
                           else {
                              prohibit_speak = dataobj["prohibit_speak"].toInt();
                           }
                           if (prohibit_speak == 1) {
                              return;
                           }
                        }
                     }
                  }
                  if (dataobj.contains(Def_target_id)) {
                     if (dataobj[Def_target_id].isString()) {
                        strJoinUid = dataobj[Def_target_id].toString();
                     }
                     else {
                        strJoinUid = QString::number(dataobj[Def_target_id].toInt());
                     }
                  }
               }
               oEvent.m_oUserInfo.userId = strJoinUid.toStdWString();
               oEvent.m_oUserInfo.role = strRoleName.toStdWString();
               oEvent.m_oUserInfo.userName = strNickName.toStdWString();

               if (e_RQ_None != oEvent.m_eMsgType && e_Ntf_UserOnline != oEvent.m_eMsgType && e_Ntf_UserOffline != oEvent.m_eMsgType) {
                  QJsonObject eventJson = MakeEventObject(&oEvent);
                  if (mainLogicReciver) {
                     QCoreApplication::postEvent(mainLogicReciver, new CustomSocketIOMsg(CustomEvent_CustomRecvSocketIOMsgEvent, eventJson), Qt::LowEventPriority);
                  }
               }
            }
         }
      }
   }
}
/*
*   收到申请上麦消息 ,消息为广播消息，收到消息后，通过进入互动房间时获取的权限列表，判断用户是否有审核权限
*/
void VHPaasSDKCallback::OnRecvApplyInavPublishMsg(const std::wstring& third_party_user_id) {

}
/*
*   收到审核上麦消息 ,消息为广播消息，收到同意上麦后执行上麦操作
*/
void VHPaasSDKCallback::OnRecvAuditInavPublishMsg(const std::wstring& third_party_user_id, AuditPublish) {

}
/*
*   邀请上麦消息  消息为广播消息，收到消息后，提示邀请信息
*/
void VHPaasSDKCallback::OnRecvAskforInavPublishMsg(const std::wstring& third_party_user_id) {

}
/*
*   踢出流消息  消息为广播消息，收到消息后，执行踢出流
*/
void VHPaasSDKCallback::OnRecvKickInavStreamMsg(const std::wstring& third_party_user_id) {

}
/*
*   踢出互动房间 , 消息为广播消息，收到消息后，判断是当前用户后，执行踢出房间操作
*/
void VHPaasSDKCallback::OnRecvKickInavMsg(const std::wstring& third_party_user_id) {

}
/*
*   上/下/拒绝上麦消息 消息为广播消息
*/
void VHPaasSDKCallback::OnUserPublishCallback(const std::wstring& third_party_user_id, const std::string& stream_id, PushStreamEvent event) {

}
/*
*   互动房间关闭消息  接受到该消息后，所有在互动房间的人员，全部下麦，退出房间
*/
void VHPaasSDKCallback::OnRecvInavCloseMsg() {

}
/*
*   用户上下线通知
*   online： true 用户上线/ false 用户下线
*   user_id： 用户id
*/
void VHPaasSDKCallback::OnRecvChatCtrlMsg(const vlive::ChatMsgType msgType, const char* ms) {
   /*std::string roleNameStr;
   if (rolename == "1") {
      roleNameStr = USER_HOST;
   }
   else if (rolename == "2") {
      roleNameStr = USER_USER;
   }
   else if (rolename == "3") {
      roleNameStr = USER_ASSISTANT;
   }
   else if (rolename == "4") {
      roleNameStr = USER_GUEST;
   }*/
   /*if (mainLogicReciver) {
      CustomOnHttpResMsgEvent *customMsg = new CustomOnHttpResMsgEvent(CustomEvent_ChatMsg);
      customMsg->mMsg = QString::fromStdString(type);
      customMsg->mRecvUserId = QString::fromStdWString(user_id);
      customMsg->mRoleName = QString::fromStdString(roleNameStr);
      customMsg->mNickName = QString::fromStdWString(nickname);
      customMsg->is_banned = is_banned;
      customMsg->devType = devType;
      customMsg->mUv = uv;
      QCoreApplication::postEvent(mainLogicReciver, customMsg, Qt::LowEventPriority);
   }*/
}
/*
*   房间链接事件
*/
void VHPaasSDKCallback::OnRtcRoomNetStateCallback(const vlive::RtcRoomNetState type/*,const std::string&*/) {
   switch (type)
   {
   case VHRoomConnect_ROOM_CONNECTED: {
      QCoreApplication::postEvent(mainLogicReciver, new QEvent(CustomEvent_ConnectRoomSuc));
      break;
   }
   case VHRoomConnect_ROOM_MIXED_STREAM_READY: {
      QCoreApplication::postEvent(mainLogicReciver, new QEvent(CustomEvent_RoomStreamMixed));
      break;
   }
   case VHRoomConnect_ROOM_ERROR:
   case VHRoomConnect_ROOM_DISCONNECTED: {
      if (!mbIsExit) {
         QCoreApplication::postEvent(mainLogicReciver, new QEvent(CustomEvent_ConnectRoomErr));
         QCoreApplication::postEvent(mainLogicReciver, new QEvent(CustomEvent_RePublishLocalStream));
         QCoreApplication::postEvent(mainLogicReciver, new QEvent(CustomEvent_RePublishDeskTopStream));
         QCoreApplication::postEvent(mainLogicReciver, new QEvent(CustomEvent_RePublishMediaStream));
      }
      break;
   }
   case VHRoomConnect_ROOM_RECONNECTING:
   case VHRoomConnect_ROOM_NETWORKCHANGED: {
      QCoreApplication::postEvent(mainLogicReciver, new CustomReConnectEvent(CustomEvent_RoomReconnecting, NULL));
      break;
   }
   //case VHRoomConnect_RE_CONNECTED:
   case VHRoomConnect_ROOM_NETWORKRECOVER:
   case VHRoomConnect_ROOM_RECOVER: {
      QCoreApplication::postEvent(mainLogicReciver, new QEvent(CustomEvent_RoomReconnected));
      break;
   }
   default:
      break;
   }
}
/*
*   本地打开采集设备,包括摄像头、桌面共享、开始插播视频回调,
*/
void VHPaasSDKCallback::OnOpenCaptureCallback(vlive::VHStreamType streamType, vlive::VHCapture code, bool hasVideo, bool hasAudio) {
   switch (streamType)
   {
   case VHStreamType_AVCapture: {
      switch (code)
      {
      case vlive::VHCapture_OK:
         QCoreApplication::postEvent(mainLogicReciver, new CustomRecvMsgEvent(CustomEvent_GetLocalStreamSuc, QString(), hasVideo));
         break;
      case vlive::VHCapture_ACCESS_DENIED:
         QCoreApplication::postEvent(mainLogicReciver, new QEvent(CustomEvent_GetLocalStreamErr));
         break;
      case vlive::VHCapture_VIDEO_DENIED:
         QCoreApplication::postEvent(mainLogicReciver, new QEvent(CustomEvent_OpenCameraErrMsgEvent));
         break;
      case vlive::VHCapture_AUDIO_DENIED:
         QCoreApplication::postEvent(mainLogicReciver, new QEvent(CustomEvent_OpenMicErrMsgEvent));
         break;
      case vlive::VHCapture_STREAM_SOURCE_LOST:
         QCoreApplication::postEvent(mainLogicReciver, new QEvent(CustomEvent_LostStream));
         break;
      case vlive::VHCapture_AUDIODEV_REMOVED:
         QCoreApplication::postEvent(mainLogicReciver, new QEvent(CustomEvent_AudioDevRemoved));
         break;
      case vlive::VHCapture_AUDIO_CAPTRUE_ERR:
         QCoreApplication::postEvent(mainLogicReciver, new QEvent(CustomEvent_AudioCaptureErr));
         break;
      case VHCapture_VIDEO_CAPTURE_ERROR:          
         QCoreApplication::postEvent(mainLogicReciver, new QEvent(CustomEvent_VIDEO_CAPTURE_ERROR));
         break;
      case vlive::VHCapture_PLAY_FILE_ERR:
         break;
      default:
         break;
      }
      break;
   }
   case VHStreamType_Desktop: {
      switch (code)
      {
      case vlive::VHCapture_OK:
         QCoreApplication::postEvent(mainLogicReciver, new QEvent(CustomEvent_GetDeskTopStreamSuc));
         break;
      case vlive::VHCapture_ACCESS_DENIED:
         QCoreApplication::postEvent(mainLogicReciver, new QEvent(CustomEvent_GetDeskTopStreamErr));
         break;
      default:
         break;
      }
      break;
   }
   case VHStreamType_MediaFile: {
      switch (code)
      {
      case vlive::VHCapture_OK:
         QCoreApplication::postEvent(mainLogicReciver, new QEvent(CustomEvent_GetLocalFileStreamSuc));
         break;
      case vlive::VHCapture_ACCESS_DENIED:
      case vlive::VHCapture_PLAY_FILE_ERR:
         QCoreApplication::postEvent(mainLogicReciver, new QEvent(CustomEvent_GetLocalFileStreamErr));
         break;
      default:
         break;
      }
      break;
   }
   case VHStreamType_Preview_Video: {
      if (code == vlive::VHCapture_OK) {
         QApplication::postEvent(mainLogicReciver, new CustomRecvMsgEvent(CustomEvent_SettingGetCameraDevSuc, QString::number(0)));
      }
      else {
         QApplication::postEvent(mainLogicReciver, new CustomRecvMsgEvent(CustomEvent_SettingGetCameraDevErr, QString::number(code)));
      }
      break;
   }
   default:
      break;
   }
}
/*
*   推流成功回调
*/
void VHPaasSDKCallback::OnPushStreamSuc(vlive::VHStreamType streamType, std::string& streamid, bool hasVideo, bool hasAudio, std::string& attributes) {

   switch (streamType)
   {
   case vlive::VHStreamType_AVCapture:
      QCoreApplication::postEvent(mainLogicReciver, new CustomRecvMsgEvent(CustomEvent_PushLocalStreamSuc, QString::fromStdString(streamid)));
      break;
   case vlive::VHStreamType_Desktop:
      QCoreApplication::postEvent(mainLogicReciver, new CustomRecvMsgEvent(CustomEvent_PushDeskTopStreamSuc, QString::fromStdString(streamid)));
      break;
   case vlive::VHStreamType_MediaFile:
      QCoreApplication::postEvent(mainLogicReciver, new CustomRecvMsgEvent(CustomEvent_PushMediaFileStreamSuc, QString::fromStdString(streamid)));
      break;
   default:
      break;
   }

}
/*
*   推流失败回调
*/
void VHPaasSDKCallback::OnPushStreamError(vlive::VHStreamType streamType, const int codeErr /*= 0*/, const std::string& msg/* = std::string()*/) {
   QString join_uid;
   CustomRecvMsgEvent* customEvent = new CustomRecvMsgEvent(CustomEvent_STREAM_FAILED, join_uid);
   if (customEvent) {
      customEvent->mStreamType = streamType;
      QCoreApplication::postEvent(mainLogicReciver, customEvent);

      //switch (streamType)
      //{
      //case vlive::VHStreamType_AVCapture: {
      //    QCoreApplication::postEvent(mainLogicReciver, customEvent);
      //    break;
      //}
      //case vlive::VHStreamType_Desktop: {
      //    QCoreApplication::postEvent(mainLogicReciver, customEvent);
      //    break;
      //}
      //case vlive::VHStreamType_MediaFile: {
      //    QCoreApplication::postEvent(mainLogicReciver, customEvent);
      //    break;
      //}
      //default:
      //    break;
      //}
   }
}

/*
*   本地网络重连成功之后重推流，流ID发生改变通知
**/
void VHPaasSDKCallback::OnRePublishStreamIDChanged(vlive::VHStreamType streamType, const std::wstring& user_id, const std::string& old_streamid, const std::string& new_streamid) {
   switch (streamType)
   {
   case vlive::VHStreamType_AVCapture:
      QCoreApplication::postEvent(mainLogicReciver, new CustomRecvMsgEvent(CustomEvent_PushLocalStreamSuc, QString::fromStdString(new_streamid)));
      break;
   case vlive::VHStreamType_Desktop:
      QCoreApplication::postEvent(mainLogicReciver, new CustomRecvMsgEvent(CustomEvent_PushDeskTopStreamSuc, QString::fromStdString(new_streamid)));
      break;
   case vlive::VHStreamType_MediaFile:
      QCoreApplication::postEvent(mainLogicReciver, new CustomRecvMsgEvent(CustomEvent_PushMediaFileStreamSuc, QString::fromStdString(new_streamid)));
      break;
   default:
      break;
   }
}
/*
*   停止推流回调
*   code: 0成功， 其他失败
*/
void VHPaasSDKCallback::OnStopPushStreamCallback(vlive::VHStreamType streamType, int code, const std::string& msg) {

}

void VHPaasSDKCallback::OnSubScribeStreamErr(const std::string& stream_d, const std::string& msg, int errorCode, const std::string& join_id) {
   if (errorCode == 40002) {
      return;
   }
   else if (errorCode == 40005) {
      return;
   }

   CustomRecvMsgEvent* event = new CustomRecvMsgEvent(CustomEvent_SubScribedError);
   event->mStreamId = QString::fromStdString(stream_d);
   event->mode = errorCode;
   event->msg = QString::fromStdString(msg);
   event->mJoinUserId = QString::fromStdString(join_id);
   QCoreApplication::postEvent(mainLogicReciver, event);
}

/**
*   接收到远端推流
*/
void VHPaasSDKCallback::OnRemoteStreamAdd(const std::string& user_id, const std::string& stream_id, vlive::VHStreamType streamType) {
   CustomRecvMsgEvent* event = new CustomRecvMsgEvent(CustomEvent_RemoteStreamAdd, QString::fromStdString(user_id));
   event->mStreamId = QString::fromStdString(stream_id);
   event->mStreamType = streamType;
   QCoreApplication::postEvent(mainLogicReciver, event);
}

/*
*    接收到远端的媒体流  hasVideo ：是否包含视频
*    当接收到远端媒体流包括插播视频流或者桌面共享流，如果本地已经打开插播或桌面共享时自动停止。
*/
void VHPaasSDKCallback::OnReciveRemoteUserLiveStream(const std::wstring& user, const std::string& streamid, const vlive::VHStreamType type, bool hasVideo, bool hasAudio, const std::string user_data) {
   CustomRecvMsgEvent* event = new CustomRecvMsgEvent(CustomEvent_SubScribSuc, QString::fromStdWString(user), hasVideo);
   event->mStreamId = QString::fromStdString(streamid);
   event->mStreamType = type;
   event->mUserData = QString::fromStdString(user_data); 
   QCoreApplication::postEvent(mainLogicReciver, event);
}
/*
*   远端的媒体流退出了
*/
void VHPaasSDKCallback::OnRemoteUserLiveStreamRemoved(const std::wstring& user, const std::string& streamid, const vlive::VHStreamType type) {
   CustomRecvMsgEvent* event = new CustomRecvMsgEvent(CustomEvent_STREAM_REMOVED, QString::fromStdWString(user), true);
   event->mStreamId = QString::fromStdString(streamid);
   event->mStreamType = type;
   QCoreApplication::postEvent(mainLogicReciver, event);
}
/*
*   订阅大小流回调
*/
void VHPaasSDKCallback::OnChangeSubScribeUserSimulCast(const std::wstring& user_id, vlive::VHSimulCastType type, int code, std::string msg) {

}
