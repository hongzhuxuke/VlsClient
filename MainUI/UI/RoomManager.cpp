#include "RoomManager.h"

#include "Msg_MainUI.h"
#include "ICommonData.h"
#include "MainUIIns.h"
#include <QJsonObject>
#include <QDateTime>
#include "IInterActivitySDK.h"
#include "DebugTrace.h"

QMutex   RoomManager::m_msgMutex;
QList<CustomMsg>  RoomManager::m_msgList;
HANDLE RoomManager::m_hEvent = NULL;


RoomManager::RoomManager(QObject *parent)
: QThread(parent) {
   m_hEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
   memset(m_hostIdArry, 0, sizeof(m_hostIdArry));
   m_pNotifyTimer = new QTimer(this);
   if (m_pNotifyTimer) {
      connect(m_pNotifyTimer, SIGNAL(timeout()), this, SLOT(Slot_NotifyTimeOut()));
   }
}

RoomManager::~RoomManager() {
   if (m_pNotifyTimer) {
      disconnect(m_pNotifyTimer, SIGNAL(timeout()), this, SLOT(Slot_NotifyTimeOut()));
   }
   LeaveInteractive();
   if (m_hEvent != NULL) {
      CloseHandle(m_hEvent);
      m_hEvent = NULL;
   }
}

void RoomManager::RegisterActivitySDKObject(IInterActivitySDK* sdk) {
   m_pSDK = sdk;
}

void RoomManager::Init() {
   ExitThread(false);
   start();
}

void RoomManager::Uninit() {
   if (isRunning()) {
      ExitThread(true);
      quit();
      wait();
   }
}

void RoomManager::run() {
   TRACE6("RoomManager::run() start\n");
   while (!IsExitThread()) {
      if (m_hEvent) {
         ::WaitForSingleObject(m_hEvent, 50);
      }
      m_msgMutex.lock();
      CustomMsg msg;
      bool bDealMsg = false;
      if (m_msgList.size() > 0) {
         msg = m_msgList.front();
         m_msgList.pop_front();
         bDealMsg = true;
      }
      m_msgMutex.unlock();

      if (bDealMsg) {
         qDebug() << "thread run data " << msg.json;
         DealCustomCmdMsg(msg.user, msg.json);
      }
   }
   TRACE6("RoomManager::run() stop\n");
}

void RoomManager::OnRecvCustomCmdMsg(const QString& user, const QString& json) {
   m_msgMutex.lock();
   CustomMsg msg(user, json);
   m_msgList.push_back(msg);
   m_msgMutex.unlock();
   qDebug() << "OnRecvCustomCmdMsg:recv data " << json;
   if (m_hEvent) {
      ::SetEvent(m_hEvent);
   }
}

int RoomManager::GetJoinerCount() {
   m_joinUserMutex.lock();
   int count = m_joinUserMap.size();
   m_joinUserMutex.unlock();
   qDebug() << " joiner count " << m_joinUserMap.size();
   return count;
}

void RoomManager::UpdataJoinerCount(const QString& user, const bool add, const bool bHostInviteUser/* = false*/) {
   m_joinUserMutex.lock();
   if (add) {
      QMap<QString, RoomUser>::iterator iter = m_joinUserMap.find(user);
      if (iter == m_joinUserMap.end()) {
         RoomUser roomUser;
         if (m_bHost && user.compare(m_LoginID) == 0) {
            roomUser.eRole = E_RoomUserCreator;
            roomUser.bMemJoined = true;
         } else {
            roomUser.eRole = E_RoomUserJoiner;
         }
         roomUser.joinTime = QDateTime::currentMSecsSinceEpoch();
         roomUser.bInviteJoinedUser = bHostInviteUser;
         roomUser.userid = user;
         m_joinUserMap[user] = roomUser;
         TRACE6("%s add a new joiner:%s .joiner count:%d\n", __FUNCTION__, user.toStdString().c_str(), m_joinUserMap.size());
      } else {
         iter->bMemJoined = true;
         iter->joinTime = QDateTime::currentMSecsSinceEpoch();
         emit Sig_UpdateMemberMediaState(user, iter->bOpenMic, iter->bOpenCamera, iter->bOpenWndShare);
      }
   } else {
      QMap<QString, RoomUser>::iterator iter = m_joinUserMap.find(user);
      if (iter != m_joinUserMap.end()) {
         m_joinUserMap.erase(iter);
         TRACE6("%s remove a new joiner:%s .joiner count:%d\n", __FUNCTION__, user.toStdString().c_str(), m_joinUserMap.size());
      }
   }
   m_joinUserMutex.unlock();
}

void RoomManager::SetJoinerHeadUrl(const QString& user, const QString& headUrl) {
   m_joinUserMutex.lock();
   QMap<QString, RoomUser>::iterator iter = m_joinUserMap.find(user);
   if (iter != m_joinUserMap.end()) {
      iter->headUrl = headUrl;
   }
   m_joinUserMutex.unlock();
}

void RoomManager::DealCustomCmdMsg(QString user, QString json) {
   QString qmsg = json;
   QJsonDocument doc = QJsonDocument::fromJson(qmsg.toLocal8Bit());
   if (doc.isObject()) {
      QJsonObject obj = doc.object();
      QVariantMap varmap = obj.toVariantMap();
      int nUserAction = AVIMCMD_None;
      QString szActionParam;
      if (varmap.contains(UserAction)) {
         nUserAction = varmap.value("userAction").toInt();
      }
      if (varmap.contains(ActionParam)) {
         szActionParam = varmap.value("actionParam").toString();
      }
      ParseCustomMessage(user, nUserAction, szActionParam);
   }
}

void RoomManager::ParseCustomMessage(const QString& sender, int nUserAction, QString szActionParam) {
   switch (nUserAction) {
   case AVIMCMD_Multi_Interact_GuestSendJoinRequest:{    //主播收到嘉宾发送的上麦请求
      TRACE6("AVIMCMD_Multi_Interact_GuestSendJoinRequest %s\n", sender.toStdString().c_str());
      HostDealGuestSendJoinRequest(sender, szActionParam);
      break;
   }
   case AVIMCMD_Multi_Interact_HostAgreeGuestJoin:{     // 嘉宾收到主播同意嘉宾上麦请求
      TRACE6("AVIMCMD_Multi_Interact_HostAgreeGuestJoin %s\n", sender.toStdString().c_str());
      GuestDealHostAgreeGuestJoinMultiActive(sender, szActionParam);
      break;
   }
   case AVIMCMD_Multi_Interact_HostInvietGuestJoin:{    //主播邀请嘉宾上麦
      TRACE6("AVIMCMD_Multi_Interact_HostInvietGuestJoin %s\n", sender.toStdString().c_str());
      GuestDealHostInviteJoin();
      break;
   }
   case AVIMCMD_Multi_Interact_GuestChangeAuthToGuestSuc:{  //嘉宾设置上麦权限成功,回复主播嘉宾加入活动成功。
      TRACE6("%s AVIMCMD_Multi_Interact_GuestChangeAuthToGuestSuc %s\n",__FUNCTION__, sender.toStdString().c_str());
      break;
   }
   case AVIMCMD_Multi_Interact_GuestChangeAuthToGuestErr:{  //嘉宾设置上麦权限失败,回复主播嘉宾加入活动失败。
      TRACE6("AVIMCMD_Multi_Interact_GuestChangeAuthToGuestErr %s\n", sender.toStdString().c_str());
      break;
   }
   case AVIMCMD_Multi_Interact_GuestJoinActiveSuc:{      //主播收到嘉宾加入活动成功回复
      TRACE6("AVIMCMD_Multi_Interact_GuestJoinActiveSuc %s\n", sender.toStdString().c_str());
      HostDealGuestJoinActiveSuc(sender, szActionParam);
      break;
   }
   case AVIMCMD_Multi_Interact_GuestJoinActiveErr:{     // 主播收到嘉宾加入活动失败回复
      TRACE6("AVIMCMD_Multi_Interact_GuestJoinActiveErr %s\n", sender.toStdString().c_str());
      HostDealGuestJoinActiveErr(sender);
      break;
   }
   case  AVIMCMD_Multi_Interact_HostNotifyJoinedUser:{   //主播广播目前加入活动的用户
      TRACE6("AVIMCMD_Multi_Interact_HostNotifyJoinedUser %s\n", sender.toStdString().c_str());
      GuestDealHostNotifyJoinedUser(szActionParam);
      break;
   }
   case AVIMCMD_Multi_Interact_HostRefuseGuestJoin:{    //主播拒绝嘉宾上麦请求.
      TRACE6("AVIMCMD_Multi_Interact_HostRefuseGuestJoin %s\n", sender.toStdString().c_str());
      GuestDealHostRefuseGuestJoin(szActionParam);
      break;
   }
   case AVIMCMD_Multi_Interact_GuestRefuseHostInvite:{   //嘉宾拒绝主播上麦邀请
      TRACE6("AVIMCMD_Multi_Interact_GuestRefuseHostInvite %s\n", sender.toStdString().c_str());
      HostDealGuestRefuseHostInvite(sender);
      break;
   }
   case AVIMCMD_Multi_Interact_EndpointEnter:{       // 主播收到有用户加入时，群组内广播主播的ID ,推流状态、音视频状态、嘉宾列表等信息。
      HostDealEndpointEnterRoom();
      break;
   }
   case AVIMCMD_Multi_Interact_NotifyHostID:{        //  接收主播广播的当前房间主播的ID
      GuestDealNotifyHostID(sender, szActionParam);
      break;
   }
   case AVIMCMD_Multi_Interact_GuestSendLeaveActiveRequest:{  //主播收到嘉宾下麦请求
      TRACE6("AVIMCMD_Multi_Interact_GuestSendLeaveActiveRequest %s\n", sender.toStdString().c_str());
      HostDealGuestSendLeaveActiveRequest(sender);
      break;
   }
   case AVIMCMD_Multi_Interact_HostAgreeGuestLeave:{// 嘉宾收到主播回复的同意下麦请求
      TRACE6("AVIMCMD_Multi_Interact_HostAgreeGuestLeave %s\n", sender.toStdString().c_str());
      GuestDealSendCancelJoinRequest(sender);
      break;
   }
   case AVIMCMD_Multi_Interact_ChangeGuestToWatcher:{      //  主播将活动嘉宾踢出下麦
      TRACE6("GuestDealSendCancelJoinRequest %s\n", sender.toStdString().c_str());
      GuestDealSendCancelJoinRequest(sender);
      break;
   }
   case AVIMCMD_Multi_Interact_GuestChangeAuthToWatcherSuc:{    //嘉宾设置下麦权限成功,回复主播嘉宾下麦成功
      TRACE6("GuestDealChangeAuthRecvSuc %s\n", sender.toStdString().c_str());
      break;
   }
   case AVIMCMD_Multi_Interact_GuestChangeAuthToWatcherErr:{    //嘉宾设置下麦权限失败,回复主播嘉宾下麦失败
      TRACE6("AVIMCMD_Multi_Interact_GuestChangeAuthToWatcherErr %s\n", sender.toStdString().c_str());
      break;
   }
   case AVIMCMD_Multi_Interact_GuestLeaveActiveSuc:{    //收到嘉宾发送的下麦成功消息
      TRACE6("HostDealGuestLeaveActiveSuc %s\n", sender.toStdString().c_str());
      HostDealGuestLeaveActiveSuc(sender);
      break;
   }
   case AVIMCMD_Multi_Interact_GuestLeaveActiveErr:{    // 主播收到嘉宾发送的下麦失败消息
      TRACE6("HostDealGuestLeaveActiveErr %s\n", sender.toStdString().c_str());
      HostDealGuestLeaveActiveErr(sender);
      break;
   }
   case AVIMCMD_Multi_Interact_HostNotifyLeaveUser:{    // 收到主播广播的某个用户下麦通知。
      TRACE6("GuestDealHostNotifyLeaveUser %s\n", sender.toStdString().c_str());
      GuestDealHostNotifyLeaveUser(szActionParam);
      break;
   }
   case AVIMCMD_Multi_Interact_CloseMic:{// 主播关闭嘉宾麦克风，C2C消息 ：
      GuestDealHostCloseGuestMic(sender);
      break;
   }
   case AVIMCMD_Multi_Interact_OpenMic:{ // 主播打开嘉宾麦克风，C2C消息 ：
      GuestDealHostOpenGuestMic(sender);
      break;
   }
   case AVIMCMD_Multi_Interact_CloseCamera:{ // 主播打开嘉宾摄像头 ，C2C消息 ：
      GuestDealHostCloseGuestCamera(sender);
      break;
   }
   case AVIMCMD_Multi_Interact_OpenCamera:{ // 主播关闭嘉宾摄像头  ，C2C消息 ：
      GuestDealHostOpenGuestCamera(sender);
      break;
   }
   case AVIMCMD_Multi_Interact_HostNotifyPushStreamState:{  //嘉宾收到主播广播的推流状态。
      GuestDealHostPushState(szActionParam);
      break;
   }
   case AVIMCMD_Multi_Interact_HostNotifyLockViewUser:{    //嘉宾收到主播广播的锁定状态。
      GuestDealHostNotifyLockUser(szActionParam);
      break;
   }
   case AVIMCMD_Multi_Interact_HostNotifyCloseScreen:{   //嘉宾收到主播广播的请求嘉宾关闭屏幕共享状态。
      GuestDealHostNotifyCloseScreen();
      break;
   }
   case AVIMCMD_Multi_Interact_HostNotifyCloseScreenResp:{     //主播收到的嘉宾关闭屏幕共享响应。
      HostDealGuestCloseScreenResp();
      break;
   }
   case AVIMCMD_Multi_Interact_ActiveMemberNotifyMediaState:{    //成员收到各个嘉宾广播的音视频状态。
      MemberDealMedaiState(szActionParam);
      break;
   }
   case AVIMCMD_Multi_Interact_PushStreamCallBack:{
      HostDealPushSteamCallBack();
      break;
   }
   case AVIMCMD_Multi_Interact_HostNotifyExitRoom:{
      GuestDealHostExitRoom();
      break;
   }
   case AVIMCMD_Multi_Interact_HostKickOutUser:{  //嘉宾收到主播将自己踢出。
      TRACE6("AVIMCMD_Multi_Interact_HostKickOutUser %s\n", sender.toStdString().c_str());
      emit Sig_HostKickOutUser();
      break;
   }
   case AVIMCMD_Multi_Interact_HostKickOutUserMsgSendSuc:{
      TRACE6("KickOutUserSuc %s\n", sender.toStdString().c_str());
      KickOutUserSuc(szActionParam);
      break;
   }
   case AVIMCMD_Multi_Interact_GuestExitRoom:{  //嘉宾发来退出房间消息
      break;
   }
   case AVIMCMD_Multi_Interact_InviteMsgSendErr:{
      TRACE6("AVIMCMD_Multi_Interact_InviteMsgSendErr %s\n", sender.toStdString().c_str());
      UpdataJoinerCount(szActionParam, false);
      emit Sig_SendMsgErr("");
      break;
   }
   case AVIMCMD_Multi_Interact_AgreeJoinMsgSendErr:{
      TRACE6("AVIMCMD_Multi_Interact_AgreeJoinMsgSendErr %s\n", sender.toStdString().c_str());
      HostSendAgreeJoinMsg(szActionParam);
      break;
   }
   case AVIMCMD_Multi_Interact_HostReSendDisAgreenJoin:{
      HostSendDisAgreeJoinMsg(szActionParam);
      break;
   }
   case AVIMCMD_Multi_Interact_SendMsgErr:{
      emit Sig_SendMsgErr("");
      break;
   }
   case AVIMCMD_EnterLive:{
      if (m_bHost) {
         TRACE6("RoomManager::AVIMCMD_EnterLive\n");
         AddAppLoginUser(szActionParam);
      }
      break;
   }
   case AVIMCMD_ExitLive:{
      if (m_bHost) {
         TRACE6("RoomManager::AVIMCMD_ExitLive\n");
         HostDealGuestLeaveActiveSuc(szActionParam);
         DeleteLoginUser(szActionParam);
      }
      break;
   }
   case AVIMCMD_Multi_Interact_ReSendJoinSucMsg:{
      TRACE6("%s AVIMCMD_Multi_Interact_ReSendJoinSucMsg\n", __FUNCTION__);
      GuestDealChangeAuthAllSuc();
      break;
   }
   case AVIMCMD_Multi_Interact_ReSendLeaveActive:{
      TRACE6("%s AVIMCMD_Multi_Interact_ReSendLeaveActive\n",__FUNCTION__);
      GuestDealSendCancelJoinRequest(QString());
      break;
   }
   case AVIMCMD_Multi_Interact_GuestJoinRoomNotify:{
      NotifyHostPushState();
      break;
   }
   default:
      break;
   }
}

void RoomManager::HostDealGuestSendJoinRequest(const QString& sender, QString szActionParam) {
   TRACE6("%s guest:%s\n", __FUNCTION__, sender.toStdString().c_str());
   QString qmsg = szActionParam;
   QString headUrl;
   QJsonDocument doc = QJsonDocument::fromJson(qmsg.toLocal8Bit());
   if (doc.isObject()) {
      QJsonObject obj = doc.object();
      QVariantMap varmap = obj.toVariantMap();
      int nUserAction = AVIMCMD_None;
      QString szActionParam;
      if (varmap.contains(HeadUrl)) {
         headUrl = varmap.value(HeadUrl).toString();
      }
   }
   if (GetJoinerCount() < MAX_JOINER && m_pSDK && m_pSDK->GetStreamState()) {
      TRACE6("%s guest count < MAX_JOINER\n", __FUNCTION__);
      UpdataJoinerCount(sender, true, false);
      SetJoinerHeadUrl(sender, headUrl);
      HostSendAgreeJoinMsg(sender);
   } else {
      TRACE6("%s HostSendDisAgreeJoinMsg sender:%s\n", __FUNCTION__, sender.toStdString().c_str());
      HostSendDisAgreeJoinMsg(sender);
   }
}

void RoomManager::HostSendDisAgreeJoinMsg(const QString& sender) {
   QVariantMap varmap;
   varmap.insert("userAction", (int)AVIMCMD_Multi_Interact_HostRefuseGuestJoin);
   if (GetJoinerCount() >= MAX_JOINER) {
      varmap.insert(ActionParam, (int)REFUSE_MAX_GUEST_ONLINE);
   } else if (!m_pSDK->GetStreamState()) {
      varmap.insert(ActionParam, (int)REFUSE_HOST_NO_PUSHTING);
   } else {
      varmap.insert(ActionParam, (int)REFUSE_REASON_NONE);
   }
   QJsonDocument doc;
   char* user = new char[MAX_USER_ID_LEN];
   if (user) {
      memset(user, 0, sizeof(user));
      memcpy(user, sender.toStdString().c_str(), MAX_USER_ID_LEN);
   }
   doc.setObject(QJsonObject::fromVariantMap(varmap));
   string json = QString(doc.toJson()).toStdString();
   m_pSDK->SendC2CCustomCmd(sender.toStdString().c_str(), json.c_str(), OnHostSendDisAgreeJoinMsgSuc, OnHostSendDisAgreeJoinMsgErr, user);
}

void RoomManager::OnHostSendDisAgreeJoinMsgSuc(void *data) {
   if (data) {
      QString id;
      char* p = (char*)(data);
      if (p) {
         TRACE6("%s start get id\n",__FUNCTION__);
         id = QString(p);
         TRACE6("%s get id %s\n",__FUNCTION__,p);
         delete[]p;
         data = NULL;
         p = NULL;
      }
   }
}
void RoomManager::OnHostSendDisAgreeJoinMsgErr(const int code, const char *desc, void* data) {
   if (data) {
      QString id;
      char* p = (char*)(data);
      if (p) {
         TRACE6("%s start get id\n", __FUNCTION__);
         id = QString(p);
         TRACE6("%s get id %s\n", __FUNCTION__, p);
         delete[]p;
         data = NULL;
         p = NULL;
      }

      TRACE6("%s  RoomManager::OnHostSendDisAgreeJoinMsgErr code : %d ,desc :%s\n", __FUNCTION__, code, desc);
      m_msgMutex.lock();
      QVariantMap varmap;
      varmap.insert(UserAction, (int)AVIMCMD_Multi_Interact_HostReSendDisAgreenJoin);
      varmap.insert(ActionParam, id);
      QJsonDocument doc;
      doc.setObject(QJsonObject::fromVariantMap(varmap));
      string json = QString(doc.toJson()).toStdString();
      CustomMsg msg("", QString::fromStdString(json));
      m_msgList.push_back(msg);
      m_msgMutex.unlock();
      if (m_hEvent) {
         ::SetEvent(m_hEvent);
      }
   }
}

void RoomManager::HostSendAgreeJoinMsg(const QString& id) {
   QVariantMap varmap;
   varmap.insert(UserAction, (int)AVIMCMD_Multi_Interact_HostAgreeGuestJoin);
   varmap.insert(ActionParam, id);
   QJsonDocument doc;
   doc.setObject(QJsonObject::fromVariantMap(varmap));
   string json = QString(doc.toJson()).toStdString();
   char* user = new char[MAX_USER_ID_LEN];
   if (user) {
      memset(user, 0, sizeof(user));
      memcpy(user, id.toStdString().c_str(), MAX_USER_ID_LEN);
   }
   TRACE6("%s host send AgreeGuestJoin\n", __FUNCTION__);
   m_pSDK->SendC2CCustomCmd(id.toStdString().c_str(), json.c_str(), OnHostSendAgreeJoinSuc, OnHostSendAgreeJoinErr, (void*)user);
}

void RoomManager::OnHostSendAgreeJoinSuc(void *data) {
   TRACE6("%s\n",__FUNCTION__);
   if (data) {
      QString id;
      char* p = (char*)(data);
      if (p) {
         TRACE6("OnHostSendAgreeJoinSuc start get id\n");
         id = QString(p);
         TRACE6("OnHostSendAgreeJoinSuc get id %s\n", p);
         delete[]p;
         data = NULL;
         p = NULL;
      }
   }
}

void RoomManager::OnHostSendAgreeJoinErr(const int code, const char *desc, void* data) {
   if (data) {
      QString id;
      char* p = (char*)(data);
      if (p) {
         TRACE6("OnHostSendAgreeJoinErr start get id\n");
         id = QString(p);
         TRACE6("OnHostSendAgreeJoinErr get id %s\n", p);
         delete[]p;
         data = NULL;
         p = NULL;
      }
      TRACE6("%s send err OnHostSendAgreeJoinErr code : %d ,desc :%s\n", __FUNCTION__, code, desc);
      if (!id.isEmpty()) {
         m_msgMutex.lock();
         QVariantMap varmap;
         varmap.insert(UserAction, (int)AVIMCMD_Multi_Interact_AgreeJoinMsgSendErr);
         varmap.insert(ActionParam, id);
         QJsonDocument doc;
         doc.setObject(QJsonObject::fromVariantMap(varmap));
         string json = QString(doc.toJson()).toStdString();
         CustomMsg msg("", QString::fromStdString(json));
         m_msgList.push_back(msg);
         m_msgMutex.unlock();
         if (m_hEvent) {
            ::SetEvent(m_hEvent);
         }
      }
   }
}

void RoomManager::GuestDealHostAgreeGuestJoinMultiActive(const QString& sender, QString szActionParam) {
   TRACE6("%s host is %s\n", __FUNCTION__, sender.toStdString().c_str());
   GuestDealChangeAuthAllSuc();
}

void RoomManager::OnSendC2CCmdSucess(std::string& data) {

}

void RoomManager::OnSendCustomMsgErr(int code, const std::string& desc, std::string& json) {
   TRACE6("OnSendCustomMsgErrCallback code:%d ,desc: %s, json :%s\n", code, desc.c_str(), json.c_str());
}

void RoomManager::JoinInteractive(const QString& id, bool bHost) {
   TRACE6("RoomManager::JoinInteractive() start\n");
   LeaveInteractive();
   Init();
   //清空缓存
   m_msgMutex.lock();
   m_msgList.clear();
   m_msgMutex.unlock();

   m_joinUserMutex.lock();
   m_joinUserMap.clear();
   m_joinUserMutex.unlock();

   m_LoginID = id;
   m_bHost = bHost;
   m_lockUserView.clear();

   m_pNotifyTimer->start(MAX_NOTIFY_TIMEOUT);
   if (bHost) {
      UpdataJoinerCount(id, true);
      SetJoinerHeadUrl(id, "");
   } else {
      GuestSendJoinRoomNotify();
   }
   TRACE6("RoomManager::JoinInteractive() ok\n");
}

void RoomManager::LeaveInteractive() {
   TRACE6("RoomManager::LeaveInteractive() start\n");
   Uninit();
   m_pNotifyTimer->stop();
   //清空缓存
   m_msgMutex.lock();
   m_msgList.clear();
   m_msgMutex.unlock();
   m_lockUserView.clear();
   m_joinUserMutex.lock();
   m_joinUserMap.clear();
   m_joinUserMutex.unlock();
   m_hostID = QString();
   m_allUserMutex.lock();
   m_allRoomUserMap.clear();
   m_allUserMutex.unlock();
   TRACE6("RoomManager::LeaveInteractive() ok\n");
}
void RoomManager::ExitThread(bool bExit) {
   m_threadMutex.lock();
   m_bExitThread = bExit;
   m_threadMutex.unlock();
}

bool RoomManager::IsExitThread() {
   bool bExit = false;
   m_threadMutex.lock();
   bExit = m_bExitThread;
   m_threadMutex.unlock();
   return bExit;
}

void RoomManager::GetActiveUserState(const char *user, bool& bIsOpenMic, bool &bIsOpenCamera, bool &bIsOpenDesktopSharing) {
   bIsOpenMic = false;
   bIsOpenCamera = false;
   bIsOpenDesktopSharing = false;

   m_joinUserMutex.lock();
   QString userID(user);
   QMap<QString, RoomUser>::iterator iter = m_joinUserMap.find(userID);
   if (iter != m_joinUserMap.end()) {
      bIsOpenMic = iter->bOpenMic;
      bIsOpenCamera = iter->bOpenCamera;
      bIsOpenDesktopSharing = iter->bOpenWndShare;
   }
   m_joinUserMutex.unlock();
}

void RoomManager::OnMemStatusChanged(int event_id, std::string identifier) {
   //主播如果收到成员状态消息。
   if (m_bHost) {
      HostDealJoinerMemberState(event_id, identifier);
      DealEnterRoomMemberState(event_id, identifier);
   } else {
      GuestDealJoinerMemberState(event_id, identifier);
   }
}

void RoomManager::GuestDealHostInviteJoin() {
   TRACE6("GuestDealHostInviteJoin\n");
   emit Sig_HostInviteJoin();
}

void RoomManager::HostDealEndpointEnterRoom() {
   Sleep(10);
   NotifyHostID();
   Sleep(10);
   NotifyJoinerToAllUsers();
   Sleep(10);
   NotifyLockViewUser();
   Sleep(10);
   NotifySelfMediaState();
   Sleep(10);
   NotifyHostPushState();
}

void RoomManager::NotifyHostID() {
   if (m_bHost) {
      QVariantMap varmap;
      varmap.insert("userAction", (int)AVIMCMD_Multi_Interact_NotifyHostID);
      varmap.insert("actionParam", m_LoginID);
      QJsonDocument doc;
      doc.setObject(QJsonObject::fromVariantMap(varmap));
      string json = QString(doc.toJson()).toStdString();
      m_pSDK->SendGroupCustomCmd(json.c_str());
   }
}

void RoomManager::GuestDealNotifyHostID(const QString& sender, const QString& szActionParam) {
   m_hostIDMutex.lock();
   m_hostID = szActionParam;
   m_hostIDMutex.unlock();
}

const char* RoomManager::GetHostID() {
   TRACE6("%s\n",__FUNCTION__);
   m_hostIDMutex.lock();
   memset(m_hostIdArry, 0, sizeof(m_hostIdArry));
   memcpy(m_hostIdArry, m_hostID.toStdString().c_str(), m_hostID.toStdString().length());
   m_hostIDMutex.unlock();
   return m_hostIdArry;
}

void RoomManager::GuestDealChangeAuthAllSuc() {
   TRACE6("%s \n", __FUNCTION__);
   if (!m_bHost) {
      QVariantMap varmapHead;
      varmapHead.insert(HeadUrl, m_headUrl);
      QJsonDocument docHead;
      docHead.setObject(QJsonObject::fromVariantMap(varmapHead));
      QString jsonHead = QString(docHead.toJson());

      QVariantMap varmap;
      varmap.insert(UserAction, (int)AVIMCMD_Multi_Interact_GuestJoinActiveSuc);
      varmap.insert(ActionParam, jsonHead);
      QJsonDocument doc;
      doc.setObject(QJsonObject::fromVariantMap(varmap));
      string json = QString(doc.toJson()).toStdString();
      m_pSDK->SendC2CCustomCmd(GetHostID(), json.c_str(), OnGuestSendJoinSuc, OnGuestSendJoinErr, this);
   }
}

void RoomManager::OnGuestSendJoinSuc(void *data) {
   if (data != NULL) {
      RoomManager *manager = (RoomManager *)data;
      if (manager) {
         TRACE6("sendsuc OnGuestSendJoinSuc  emit manager->Sig_GuestJoinActiveSuc(true)\n");
         emit manager->Sig_GuestJoinActiveSuc(true);
         emit manager->Sig_ChangetoLiveGuest();
      }
   }
}

void RoomManager::OnGuestSendJoinErr(const int code, const char *desc, void* data) {
   if (data != NULL) {
      RoomManager *manager = (RoomManager *)data;
      TRACE6("senderr OnGuestSendJoinErr code : %d ,desc :%s\n", code, desc);
      emit manager->Sig_GuestSendJoinActiveMsgErr();

      m_msgMutex.lock();
      QVariantMap varmap;
      varmap.insert(UserAction, (int)AVIMCMD_Multi_Interact_ReSendJoinSucMsg);
      QJsonDocument doc;
      doc.setObject(QJsonObject::fromVariantMap(varmap));
      string json = QString(doc.toJson()).toStdString();
      CustomMsg msg("", QString::fromStdString(json));
      m_msgList.push_back(msg);
      m_msgMutex.unlock();
      if (m_hEvent) {
         ::SetEvent(m_hEvent);
      }
   }
}

void RoomManager::HostDealJoinerMemberState(int state, std::string identifier) {
   m_joinUserMutex.lock();
   bool bNotify = false;
   QString userID = QString::fromStdString(identifier);
   QMap<QString, RoomUser>::iterator iter = m_joinUserMap.find(userID);
   if (iter != m_joinUserMap.end()) {
      switch (state) {
      case SDKEVENT_ID_ENDPOINT_HAS_AUDIO:{
                                             iter->bOpenMic = true;
                                             break;
      }
      case SDKEVENT_ID_ENDPOINT_NO_AUDIO:{
                                            iter->bOpenMic = false;
                                            break;
      }
      case SDKEVENT_ID_ENDPOINT_HAS_CAMERA_VIDEO:{
                                                    iter->bOpenCamera = true;
                                                    break;
      }
      case SDKEVENT_ID_ENDPOINT_NO_CAMERA_VIDEO:{
                                                   iter->bOpenCamera = false;
                                                   break;
      }
      case SDKEVENT_ID_ENDPOINT_HAS_SCREEN_VIDEO:{
                                                    iter->bOpenWndShare = true;
                                                    break;
      }
      case SDKEVENT_ID_ENDPOINT_NO_SCREEN_VIDEO:{
                                                   iter->bOpenWndShare = true;
                                                   break;
      }
      case SDKEVENT_ID_ENDPOINT_EXIT:{
                                        m_joinUserMap.erase(iter);
                                        TRACE6("Sig_ActiveMemberState(userID,false)\n");
                                        emit Sig_ActiveMemberState(userID, false);
                                        bNotify = true;
                                        break;
      }
      default:
         break;
      }
   }
   m_joinUserMutex.unlock();
   if (state == SDKEVENT_ID_ENDPOINT_EXIT && bNotify) {
      HostDealGuestLeaveActiveSuc(userID);
   }
}

void RoomManager::GuestDealJoinerMemberState(int state, std::string identifier) {
   if (state == SDKEVENT_ID_ENDPOINT_EXIT) {
      emit Sig_ActiveMemberState(QString::fromStdString(identifier), false);
   }
}

void RoomManager::DealEnterRoomMemberState(int state, std::string identifier) {
   //收到用户加入事件。
   if (SDKEVENT_ID_ENDPOINT_ENTER == state) {
      m_msgMutex.lock();
      QVariantMap varmap;
      varmap.insert(UserAction, (int)AVIMCMD_Multi_Interact_EndpointEnter);
      QJsonDocument doc;
      doc.setObject(QJsonObject::fromVariantMap(varmap));
      string json = QString(doc.toJson()).toStdString();

      CustomMsg msg("", QString::fromStdString(json));
      m_msgList.push_back(msg);
      m_msgMutex.unlock();

      if (m_hEvent) {
         ::SetEvent(m_hEvent);
      }
   }
}

void RoomManager::HostDealGuestJoinActiveSuc(const QString& sender, QString szActionParam) {
   QString qmsg = szActionParam;
   QString headUrl;
   QJsonDocument doc = QJsonDocument::fromJson(qmsg.toLocal8Bit());
   if (doc.isObject()) {
      QJsonObject obj = doc.object();
      QVariantMap varmap = obj.toVariantMap();
      int nUserAction = AVIMCMD_None;
      QString szActionParam;
      if (varmap.contains(HeadUrl)) {
         headUrl = varmap.value(HeadUrl).toString();
      }
   }

   UpdataJoinerCount(sender, true);
   NotifyJoinerToAllUsers();
   SetJoinerHeadUrl(sender, headUrl);
   emit Sig_ActiveMemberState(sender, true, headUrl);
}

void RoomManager::HostDealGuestJoinActiveErr(const QString& sender) {
   bool bInviteUser = false;
   m_joinUserMutex.lock();
   QMap<QString, RoomUser>::iterator iter = m_joinUserMap.find(sender);
   if (iter != m_joinUserMap.end()) {
      bInviteUser = iter->bInviteJoinedUser;
      m_joinUserMap.erase(iter);
   }
   m_joinUserMutex.unlock();

   if (bInviteUser) {
      //主动邀请的嘉宾。通知发消息，邀请嘉宾失败
   }
}

void RoomManager::OnGuestDealSendJoinActiveErr() {
   emit Sig_GuestSendJoinActiveMsgErr();
}

void RoomManager::NotifyJoinerToAllUsers() {
   if (m_bHost && m_pSDK && !m_pSDK->GetStreamState()) {
      return;
   }

   m_joinUserMutex.lock();
   QMap<QString, RoomUser>::iterator iterUser = m_joinUserMap.begin();
	QList<RoomUser> qlist;
   while (iterUser != m_joinUserMap.end()) {
      //如果主播主动邀请的用户，超时120秒没有回复则认为邀请失败。
      if (/*iterUser->bInviteJoinedUser && */!iterUser->bMemJoined && (iterUser->joinTime + 120000 < QDateTime::currentMSecsSinceEpoch())) {
         TRACE6("clear int user not joined .id %s\n", iterUser.key().toStdString().c_str());
			QMap<QString, RoomUser>::iterator iterTmep = iterUser + 1;
         m_joinUserMap.erase(iterUser);
			iterUser = iterTmep/*m_joinUserMap.begin()*/;
         continue;
      }
		qlist.push_back(iterUser.value());
      iterUser++;
   }

   //QList<RoomUser> qlist;
   //QMap<QString, RoomUser>::const_iterator iter;
   //for (iter = m_joinUserMap.constBegin(); iter != m_joinUserMap.constEnd(); ++iter) {
   //   qlist.push_back(iter.value());
   //}

   for (int i = 0; i < qlist.size(); i++) {
      for (int j = i; j < qlist.size(); j++) {
         if (qlist[i].joinTime > qlist[j].joinTime) {
            RoomUser temp = qlist[i];
            qlist[i] = qlist[j];
            qlist[j] = temp;
         }
      }
   }
   // 构造QJsonArray  
   QJsonArray json_array;
   QString allUser;
   for (int i = 0; i < qlist.size(); i++) {
      if (qlist[i].bMemJoined) {
         QJsonObject json_object;
         json_object.insert(MemberID, qlist[i].userid);
         json_object.insert(HeadUrl, qlist[i].headUrl);
         json_array.push_back(json_object);
      }
   }
   QByteArray byte_array = QJsonDocument(json_array).toJson();

   if (!byte_array.isEmpty()) {
      QVariantMap varmap;
      varmap.insert(UserAction, (int)AVIMCMD_Multi_Interact_HostNotifyJoinedUser);
      varmap.insert(ActionParam, byte_array);
      QJsonDocument doc;
      doc.setObject(QJsonObject::fromVariantMap(varmap));
      string json = QString(doc.toJson()).toStdString();
      m_pSDK->SendGroupCustomCmd(json.c_str());
      emit Sig_HostNotifyAllActiveMember(QString(byte_array.data()));
   }
   m_joinUserMutex.unlock();
}

void RoomManager::GuestDealHostNotifyJoinedUser(const QString& szActionParam) {
   QByteArray byte_array = szActionParam.toUtf8();
   QJsonArray json_array = QJsonDocument::fromJson(byte_array).array();
   for (int i = 0; i < json_array.size(); i++) {
      QString userid, headUrl;
      QJsonObject obj = json_array.at(i).toObject();
      QVariantMap varmap = obj.toVariantMap();
      int nUserAction = AVIMCMD_None;
      if (varmap.contains(MemberID)) {
         userid = varmap.value(MemberID).toString();
      }
      if (varmap.contains(HeadUrl)) {
         headUrl = varmap.value(HeadUrl).toString();
      }
      emit Sig_ActiveMemberState(userid, true, headUrl);
   }
   emit Sig_HostNotifyAllActiveMember(szActionParam);
}

void RoomManager::GuestDealHostRefuseGuestJoin(QString reson) {
   if (reson.toInt() == REFUSE_MAX_GUEST_ONLINE) {
      TRACE6("GuestDealHostRefuseGuestJoin  emit Sig_GuestJoinActiveSuc(false);)\n");
      //通知UI更新自己的上麦按键。
      emit Sig_GuestJoinActiveSuc(false);
      emit Sig_SendMsgErr(QStringLiteral("当前上麦人数已满"));
   } else {
      //通知UI更新自己的上麦按键。
      TRACE6("GuestDealHostRefuseGuestJoin  emit Sig_GuestJoinActiveSuc(false);)\n");
      emit Sig_GuestJoinActiveSuc(false);
   }
}

void RoomManager::HostDealGuestRefuseHostInvite(const QString& sender) {
   UpdataJoinerCount(sender, false);
   //提示用户嘉宾拒绝加入互动。
   emit Sig_GuestRefuseHostInviteResp(sender);
}

void RoomManager::GuestDealSendCancelJoinRequest(const QString& sender) {
   TRACE6("%s\n",__FUNCTION__);
   if (!m_bHost) {
      QVariantMap varmap;
      varmap.insert("userAction", (int)AVIMCMD_Multi_Interact_GuestLeaveActiveSuc);
      QJsonDocument doc;
      doc.setObject(QJsonObject::fromVariantMap(varmap));
      string json = QString(doc.toJson()).toStdString();
      m_pSDK->SendGroupCustomCmd(json.c_str(), OnGuestSendLeaveSuc, OnGuestSendLeaveErr, this);
      TRACE6("GuestDealChangeAuthRecvSuc \n");
   }
}
void RoomManager::OnGuestSendLeaveSuc(void *data) {
   TRACE6("%s \n",__FUNCTION__);
   if (data != NULL) {
      RoomManager *manager = (RoomManager *)data;
      emit manager->Sig_GuestLeavActiveSuc(true);
   }
}

void RoomManager::OnGuestSendLeaveErr(const int code, const char *desc, void* data) {
   TRACE6("emit manager->Sig_GuestLeavActiveSuc(false)");
   if (data != NULL) {
      TRACE6("senderr OnGuestSendLeaveErr code : %d ,desc :%s\n", code, desc);
      RoomManager *manager = (RoomManager *)data;
      emit manager->Sig_GuestLeavActiveSuc(false);
   }
}

void RoomManager::HostDealGuestLeaveActiveSuc(const QString& sender) {
   TRACE6("RoomManager::HostDealGuestLeaveActiveSuc 1 %s\n", sender.toStdString().c_str());
   {
      TRACE6("RoomManager::HostDealGuestLeaveActiveSuc 2 %s\n", sender.toStdString().c_str());
      if (m_bHost) {
         UpdataJoinerCount(sender, false);        
         //To Do 取消请求媒体视频。
         //取消请求XXX的视频
         emit Sig_ActiveMemberState(sender, false);
         QVariantMap varmap;
         varmap.insert(UserAction, (int)AVIMCMD_Multi_Interact_HostNotifyLeaveUser);
         varmap.insert(ActionParam, sender);
         QJsonDocument doc;
         doc.setObject(QJsonObject::fromVariantMap(varmap));
         string json = QString(doc.toJson()).toStdString();
         m_pSDK->SendGroupCustomCmd(json.c_str());
      }
   }
}

void RoomManager::GuestDealHostNotifyLeaveUser(const QString& szActionParam) {
   //取消请求XXX的视频
   emit Sig_ActiveMemberState(szActionParam, false);
}

void RoomManager::HostDealGuestSendLeaveActiveRequest(const QString& sender) {
   TRACE6("%s\n",__FUNCTION__);
   if (m_bHost && m_pSDK) {
      QVariantMap varmap;
      varmap.insert(UserAction, (int)AVIMCMD_Multi_Interact_HostAgreeGuestLeave);
      varmap.insert(ActionParam, sender);
      QJsonDocument doc;
      doc.setObject(QJsonObject::fromVariantMap(varmap));
      string json = QString(doc.toJson()).toStdString();
      m_pSDK->SendC2CCustomCmd(sender.toStdString().c_str(), json.c_str(), OnHostAgreeGuestLeaveSuc, OnHostAgreeGuestLeaveErr);
   }
}

void RoomManager::OnHostAgreeGuestLeaveSuc(void * data) {
   TRACE6("%s\n",__FUNCTION__);
}
void RoomManager::OnHostAgreeGuestLeaveErr(const int code, const char *desc, void* data) {
   TRACE6("%s code %d  desc %s\n", __FUNCTION__, code, desc);
}

void RoomManager::HostDealGuestLeaveActiveErr(const QString& sender) {
   TRACE6("RoomManager::GuestDealChangeAuthAllErr() err");
   bool bHostKickOffGuest = false;
   m_joinUserMutex.lock();
   QMap<QString, RoomUser>::iterator iter = m_joinUserMap.find(sender);
   if (iter != m_joinUserMap.end()) {
      bHostKickOffGuest = iter->bHostKickOffGuest;
      m_joinUserMap.erase(iter);
   }
   m_joinUserMutex.unlock();

   if (bHostKickOffGuest) {
      TRACE6("emit Sig_ActiveMemberState(sender,false,"")\n");
      //如果是主动踢出的用户。踢出失败进行提示。
      emit Sig_ActiveMemberState(sender, false, "");
      NotifyJoinerToAllUsers();
   }
}

void RoomManager::SetGuestKickOffState(const QString& user, bool bKickOff) {
   m_joinUserMutex.lock();
   QMap<QString, RoomUser>::iterator iter = m_joinUserMap.find(user);
   if (iter != m_joinUserMap.end()) {
      iter->bHostKickOffGuest = bKickOff;
   }
   m_joinUserMutex.unlock();
}

bool RoomManager::IsActiveUser(const char *user) {
   bool isActiveUser = false;
   m_joinUserMutex.lock();
   if (user != NULL) {
      QMap<QString, RoomUser>::iterator iter = m_joinUserMap.find(QString(user));
      if (iter != m_joinUserMap.end() && iter->bMemJoined) {
         TRACE6("IsActiveUser %s\n", user);
         isActiveUser = true;
      }
   }
   m_joinUserMutex.unlock();
   return isActiveUser;
}

bool RoomManager::IsAllowInviteJoinActive(const char* user) {
   bool isAllow = false;
   m_joinUserMutex.lock();
   isAllow = m_joinUserMap.size() < MAX_JOINER ? true : false;
   if (isAllow) {
      QMap<QString, RoomUser>::iterator iter = m_joinUserMap.find(QString(user));
      if (iter != m_joinUserMap.end()) {
         isAllow = false;
      }
   }
   m_joinUserMutex.unlock();
   return isAllow;
}

void RoomManager::GuestDealHostCloseGuestMic(const QString& sender) {
   TRACE6("%s\n", __FUNCTION__);
   emit Sig_HostNotifyGuestCloseMic(true);
}

void RoomManager::GuestDealHostOpenGuestMic(const QString& sender) {
   TRACE6("%s\n", __FUNCTION__);
   emit Sig_HostNotifyGuestCloseMic(false);
}

void RoomManager::GuestDealHostCloseGuestCamera(const QString& sender) {
   TRACE6("%s\n", __FUNCTION__);
   emit Sig_HostNotifyGuestCloseCamera(true);
}

void RoomManager::GuestDealHostOpenGuestCamera(const QString& sender) {
   TRACE6("%s\n", __FUNCTION__);
   emit Sig_HostNotifyGuestCloseCamera(false);
}

void RoomManager::Slot_NotifyTimeOut() {
   if (!IsExitThread() && m_bHost && m_pSDK != NULL) {
      if (m_pSDK->GetStreamState()) {
         HostDealEndpointEnterRoom();
      } else {
         NotifyHostID();
         Sleep(5);
         NotifyHostPushState();
      }
   } else if (!IsExitThread() && !m_bHost && m_pSDK && m_pSDK->GetStreamState()) {
      NotifySelfMediaState();
   }
}

void RoomManager::OnPushStreamSuccess(bool suc) {
   if (!IsExitThread() && m_bHost) {
      m_msgMutex.lock();
      QVariantMap varmap;
      varmap.insert(UserAction, (int)AVIMCMD_Multi_Interact_PushStreamCallBack);
      varmap.insert(ActionParam, suc);
      QJsonDocument doc;
      doc.setObject(QJsonObject::fromVariantMap(varmap));
      string json = QString(doc.toJson()).toStdString();

      CustomMsg msg("", QString::fromStdString(json));
      m_msgList.push_back(msg);
      m_msgMutex.unlock();

      if (m_hEvent) {
         ::SetEvent(m_hEvent);
      }
   }
}

void RoomManager::NotifyHostPushState() {
   if (!m_bHost || (m_bHost && !HaveAlreadyPushStream())) {
      return;
   }
   QVariantMap varmap;
   varmap.insert(UserAction, (int)AVIMCMD_Multi_Interact_HostNotifyPushStreamState);
   if (m_pSDK->GetStreamState()) {
      varmap.insert(ActionParam, HOST_PUSHING_STEAM);
   } else {
      varmap.insert(ActionParam, HOST_NO_PUSHING_STEAM);
   }
   QJsonDocument doc;
   doc.setObject(QJsonObject::fromVariantMap(varmap));
   string json = QString(doc.toJson()).toStdString();
   m_pSDK->SendGroupCustomCmd(json.c_str());
}

void RoomManager::GuestDealHostPushState(const QString& szActionParam) {
   if (szActionParam.compare(HOST_PUSHING_STEAM) == 0) {
      emit Sig_HostStartPush(true);
   } else {
      emit Sig_HostStartPush(false);
      SetLockUserView("");
   }
}

bool RoomManager::IsMemberEnableInvite(const char *user) {
   if (user == NULL) {
      return false;
   }
   bool bAllow = false;
   m_joinUserMutex.lock();
   QMap<QString, RoomUser>::iterator iter = m_joinUserMap.find(QString(user));
   if (iter != m_joinUserMap.end()) {
      bAllow = false;
   } else if (m_joinUserMap.size() < MAX_JOINER) {
      bAllow = true;
   }
   m_joinUserMutex.unlock();
   if (bAllow) {
      UpdataJoinerCount(QString(user), true, true);
   }
   return bAllow;
}

void RoomManager::GuestDealHostNotifyCloseScreen() {
   TRACE6("%s\n", __FUNCTION__);
   emit Sig_RecvCloseScreenNotify(QString(GetHostID()));
}

void RoomManager::HostDealGuestCloseScreenResp() {
   TRACE6("%s\n", __FUNCTION__);
   emit Sig_RecvCloseScreenResp();
}

void RoomManager::SetLockUserView(const char *lockUser) {
   QString user = QString(lockUser);
   if (user.isEmpty()) {
      emit Sig_GuestRecvLockView(user, false);
   } else {
      emit Sig_GuestRecvLockView(user, true);
   }

   m_lockViewMutex.lock();
   if (lockUser != NULL) {
      m_lockUserView = QString(lockUser);
   } else {
      m_lockUserView.clear();
   }
   m_lockViewMutex.unlock();
   if (m_bHost) {
      NotifyLockViewUser();
   }
}

string RoomManager::GetLockUserView() {
   m_lockViewMutex.lock();
   string lockUser = m_lockUserView.toStdString();
   m_lockViewMutex.unlock();
   return lockUser;
}

bool RoomManager::IsLockUserView() {
   bool bLockView = false;
   m_lockViewMutex.lock();
   if (!m_lockUserView.isEmpty()) {
      bLockView = true;
   }
   m_lockViewMutex.unlock();
   return bLockView;
}

void RoomManager::NotifyLockViewUser() {
   m_lockViewMutex.lock();
   QString lockUser = m_lockUserView;
   m_lockViewMutex.unlock();

   QVariantMap varmap;
   varmap.insert(UserAction, (int)AVIMCMD_Multi_Interact_HostNotifyLockViewUser);
   varmap.insert(ActionParam, lockUser);
   QJsonDocument doc;
   doc.setObject(QJsonObject::fromVariantMap(varmap));
   string json = QString(doc.toJson()).toStdString();
   m_pSDK->SendGroupCustomCmd(json.c_str());
}

void RoomManager::GuestDealHostNotifyLockUser(const QString& lockUser) {
   if (lockUser.isEmpty()) {
      emit Sig_GuestRecvLockView(lockUser, false);
   } else {
      emit Sig_GuestRecvLockView(lockUser, true);
   }
   SetLockUserView(lockUser.toStdString().c_str());
}

void RoomManager::NotifySelfMediaState() {
   QVariantMap state;
   bool bOpenMic = false, bOpemCamera = false, bOpenShare = false;
   if (m_pSDK->GetCurrentCameraState()) {
      state.insert(CameraState, STR_OPEN);
      bOpemCamera = true;
   } else {
      state.insert(CameraState, STR_CLOSE);
   }
   if (m_pSDK->GetCurrentMicState()) {
      state.insert(MicState, STR_OPEN);
      bOpenMic = true;
   } else {
      state.insert(MicState, STR_CLOSE);
   }
   if (m_pSDK->GetScreenStatus()) {
      state.insert(ShareState, STR_OPEN);
      bOpenShare = true;
   } else {
      state.insert(ShareState, STR_CLOSE);
   }
   state.insert(MemberID, m_LoginID);

   QJsonDocument docState;
   docState.setObject(QJsonObject::fromVariantMap(state));
   QString jsonState = QString(docState.toJson());

   QVariantMap varmap;
   varmap.insert(UserAction, (int)AVIMCMD_Multi_Interact_ActiveMemberNotifyMediaState);
   varmap.insert(ActionParam, jsonState);
   QJsonDocument doc;
   doc.setObject(QJsonObject::fromVariantMap(varmap));
   string json = QString(doc.toJson()).toStdString();
   if (m_bHost) {
      m_pSDK->SendGroupCustomCmd(json.c_str());
      UpdateMemberState(m_LoginID, bOpenMic, bOpemCamera, bOpenShare);
   } else if (m_pSDK->GetStreamState()) {
      m_pSDK->SendGroupCustomCmd(json.c_str());
   }
   emit Sig_UpdateMemberMediaState(m_LoginID, bOpenMic, bOpemCamera, bOpenShare);
}

void RoomManager::MemberDealMedaiState(const QString& szActionParam) {
   QJsonDocument doc = QJsonDocument::fromJson(szActionParam.toLocal8Bit());
   if (doc.isObject()) {
      QJsonObject obj = doc.object();
      QVariantMap varmap = obj.toVariantMap();
      QString micOpen, cameraOpen, shareOpen, memberId;
      bool bMicOpen = false, bCameraOpen = false, bShareOpen = false;
      QString szActionParam;
      if (varmap.contains(MicState)) {
         micOpen = varmap.value(MicState).toString();
      }
      if (varmap.contains(CameraState)) {
         cameraOpen = varmap.value(CameraState).toString();
      }
      if (varmap.contains(ShareState)) {
         shareOpen = varmap.value(ShareState).toString();
      }
      if (varmap.contains(MemberID)) {
         memberId = varmap.value(MemberID).toString();
      }

      if (micOpen.compare(STR_OPEN) == 0) {
         bMicOpen = true;
      }
      if (cameraOpen.compare(STR_OPEN) == 0) {
         bCameraOpen = true;
      }
      if (shareOpen.compare(STR_OPEN) == 0) {
         bShareOpen = true;
      }

      if (m_bHost) {
         UpdateMemberState(memberId, bMicOpen, bCameraOpen, bShareOpen);
      }
      emit Sig_UpdateMemberMediaState(memberId, bMicOpen, bCameraOpen, bShareOpen);
   }
}

void RoomManager::UpdateMemberState(const QString& identifier, const bool mic, const bool camera, const bool share) {
   m_joinUserMutex.lock();
   {
      QString userID = identifier;
      QMap<QString, RoomUser>::iterator iter = m_joinUserMap.find(userID);
      if (iter != m_joinUserMap.end()) {
         iter->bOpenCamera = camera;
         iter->bOpenMic = mic;
         iter->bOpenWndShare = share;
      }
   }
   m_joinUserMutex.unlock();
}

void RoomManager::HostDealPushSteamCallBack() {
   Slot_NotifyTimeOut();
}

void RoomManager::RemoveActiveMember(const char* userID) {
   TRACE6("RoomManager::RemoveActiveMember %s\n", userID);
   if (userID != NULL) {
      char* user = new char[MAX_USER_ID_LEN];
      memset(user, 0, sizeof(user));
      memcpy(user, userID, MAX_USER_ID_LEN);

      QVariantMap varmap;
      varmap.insert(UserAction, (int)AVIMCMD_Multi_Interact_ChangeGuestToWatcher);
      varmap.insert(ActionParam, QString(userID));

      QJsonDocument doc;
      doc.setObject(QJsonObject::fromVariantMap(varmap));
      string json = QString(doc.toJson()).toStdString();
      TRACE6("SendC2CCustomCmd  AVIMCMD_Multi_Interact_ChangeGuestToWatcher %s\n", userID);
      m_pSDK->SendC2CCustomCmd(userID, json.c_str(), OnHostSendKickOffLineSuc, OnHostSendKickOffLineErr, (void*)user);
   } else {
      TRACE6("RoomManager::RemoveActiveMember err\n");
   }
}

void RoomManager::NotifyHostExitRoom() {
   if (m_bHost) {
      QVariantMap varmap;
      varmap.insert(UserAction, (int)AVIMCMD_Multi_Interact_HostNotifyExitRoom);
      QJsonDocument doc;
      doc.setObject(QJsonObject::fromVariantMap(varmap));
      string json = QString(doc.toJson()).toStdString();
      m_pSDK->SendGroupCustomCmd(json.c_str());
   }
}

void RoomManager::GuestDealHostExitRoom() {
   emit Sig_HostExitRoom();
}

void RoomManager::SetUserHeadImageUrl(const char* url) {
   if (url) {
      m_headUrl = QString(url);
      if (m_bHost) {
         SetJoinerHeadUrl(m_LoginID, m_headUrl);
      }
   }
}

void RoomManager::ClearGuestUser(bool push) {
   if (!push && m_bHost) {
      m_joinUserMutex.lock();
      QMap<QString, RoomUser>::iterator iter = m_joinUserMap.begin();
      while (iter != m_joinUserMap.end()) {
         if (m_LoginID.compare(iter->userid) != 0) {
            Sig_ActiveMemberState(iter->userid, false);
            m_joinUserMap.erase(iter);
            iter = m_joinUserMap.begin();
            continue;
         } else if (m_joinUserMap.size() == 1 && m_LoginID.compare(iter->userid) == 0) {
            break;
         }
         iter++;
      }
      m_joinUserMutex.unlock();
   }
}

QString RoomManager::GetLockViewUserID() {
   m_lockViewMutex.lock();
   QString lockViewUser = m_lockUserView;
   m_lockViewMutex.unlock();
   return lockViewUser;
}

void RoomManager::KickOutUser(const char* user) {
   if (user != NULL) {
      QVariantMap varmap;
      varmap.insert(UserAction, (int)AVIMCMD_Multi_Interact_HostKickOutUser);
      varmap.insert(ActionParam, QString(user));
      QJsonDocument doc;
      doc.setObject(QJsonObject::fromVariantMap(varmap));
      string json = QString(doc.toJson()).toStdString();

      char* id = new char[MAX_USER_ID_LEN];
      memset(id, 0, sizeof(id));
      memcpy(id, user, MAX_USER_ID_LEN);

      m_pSDK->SendC2CCustomCmd(user, json.c_str(), OnHostSendKickOffLineSuc, OnHostSendKickOffLineErr, (void*)id);
   }
}

void RoomManager::OnHostSendKickOffLineSuc(void *data) {
   TRACE6("RoomManager::OnHostSendKickOffLineSuc ");
   if (data) {
      QString id;
      char* p = (char*)(data);
      if (p) {
         TRACE6("OnHostSendKickOffLineSuc start get id\n");
         id = QString(p);
         TRACE6("OnHostSendKickOffLineSuc get id %s\n", p);
         delete[]p;
         data = NULL;
         p = NULL;
      }
      m_msgMutex.lock();
      QVariantMap varmap;
      varmap.insert(UserAction, (int)AVIMCMD_Multi_Interact_HostKickOutUserMsgSendSuc);
      varmap.insert(ActionParam, id);
      QJsonDocument doc;
      doc.setObject(QJsonObject::fromVariantMap(varmap));
      string json = QString(doc.toJson()).toStdString();

      CustomMsg msg("", QString::fromStdString(json));
      m_msgList.push_back(msg);
      m_msgMutex.unlock();

      if (m_hEvent) {
         ::SetEvent(m_hEvent);
      }
   }
}

void RoomManager::OnHostSendKickOffLineErr(const int code, const char *desc, void* data) {
   TRACE6("RoomManager::OnHostSendKickOffLineErr");
   if (data != NULL) {
      QString id;
      char* p = (char*)(data);
      if (p) {
         TRACE6("OnHostSendKickOffLineErr start get id");
         id = QString(p);
         TRACE6("OnHostSendKickOffLineErr get id %s", p);
         delete[]p;
         data = NULL;
         p = NULL;
      }
      TRACE6("senderr OnHostSendKickOffLineErr code : %d ,desc :%s\n", code, desc);
      m_msgMutex.lock();
      QVariantMap varmap;
      varmap.insert(UserAction, (int)AVIMCMD_Multi_Interact_SendMsgErr);
      QJsonDocument doc;
      doc.setObject(QJsonObject::fromVariantMap(varmap));
      string json = QString(doc.toJson()).toStdString();
      CustomMsg msg("", QString::fromStdString(json));
      m_msgList.push_back(msg);
      m_msgMutex.unlock();
      if (m_hEvent) {
         ::SetEvent(m_hEvent);
      }
   }
}

void RoomManager::KickOutUserSuc(QString szActionParam) {
   TRACE6("RoomManager::HostDealGuestLeaveActiveSuc 2 %s\n", szActionParam.toStdString().c_str());
   UpdataJoinerCount(szActionParam, false);
   //To Do 取消请求媒体视频。
   //取消请求XXX的视频
   emit Sig_ActiveMemberState(szActionParam, false);
   QVariantMap varmap;
   varmap.insert(UserAction, (int)AVIMCMD_Multi_Interact_HostNotifyLeaveUser);
   varmap.insert(ActionParam, szActionParam);
   QJsonDocument doc;
   doc.setObject(QJsonObject::fromVariantMap(varmap));
   string json = QString(doc.toJson()).toStdString();

   m_pSDK->SendGroupCustomCmd(json.c_str());
   NotifyJoinerToAllUsers();
   if (GetLockViewUserID().compare(szActionParam) == 0) {
      SetLockUserView("");
   }
}

bool RoomManager::HaveAlreadyPushStream() {
   m_PushStreamMutex.lock();
   bool bAlreadyPush = m_bAlreadyPushStream;
   m_PushStreamMutex.unlock();
   return bAlreadyPush;
}

void RoomManager::SetAlreadyPushStream(bool set) {
   m_PushStreamMutex.lock();
   m_bAlreadyPushStream = set;
   m_PushStreamMutex.unlock();
}

void RoomManager::SendJoinRequest(const char* url) {
   TRACE6("RoomManager::SendJoinRequest\n");
   QVariantMap headUrl;
   headUrl.insert(HeadUrl, QString(url));
   QJsonDocument docUrl;
   docUrl.setObject(QJsonObject::fromVariantMap(headUrl));
   QString jsonUrl = QString(docUrl.toJson());

   QVariantMap varmap;
   varmap.insert(UserAction, (int)AVIMCMD_Multi_Interact_GuestSendJoinRequest);
   varmap.insert(ActionParam, jsonUrl);
   QJsonDocument doc;
   doc.setObject(QJsonObject::fromVariantMap(varmap));
   string json = QString(doc.toJson()).toStdString();
   m_pSDK->SendC2CCustomCmd(GetHostID(), json.c_str(), OnSendJoinRequest, OnSendJoinRequestErr, this);
}

void RoomManager::OnSendJoinRequest(void *data) {
   TRACE6("%s\n",__FUNCTION__);
}

void RoomManager::OnSendJoinRequestErr(const int code, const char *desc, void* data) {
   if (data != NULL) {
      TRACE6("senderr OnSendJoinRequestErr code : %d ,desc :%s\n", code, desc);
      TRACE6("emit manager->Sig_GuestJoinActiveSuc(false)\n");
      RoomManager *manager = (RoomManager *)data;
      emit manager->Sig_GuestJoinActiveSuc(false);
   }
}

void RoomManager::SendCancelJoinRequest() {
   TRACE6("RoomManager::SendCancelJoinRequest host %s\n", GetHostID());
   QVariantMap varmap;
   varmap.insert(UserAction, (int)AVIMCMD_Multi_Interact_GuestSendLeaveActiveRequest);
   QJsonDocument doc;
   doc.setObject(QJsonObject::fromVariantMap(varmap));
   string json = QString(doc.toJson()).toStdString();
   m_pSDK->SendC2CCustomCmd(GetHostID(), json.c_str(), OnSendCancelJoinRequest, OnSendCancelJoinRequestErr, this);
}

void RoomManager::OnSendCancelJoinRequest(void *) {
   TRACE6("%s OnSendCancelJoinRequest \n",__FUNCTION__);
}

void RoomManager::OnSendCancelJoinRequestErr(const int code, const char *desc, void* data) {
   if (data != NULL) {
      TRACE6("senderr OnSendCancelJoinRequestErr code : %d ,desc :%s\n", code, desc);
      TRACE6(" emit manager->Sig_GuestLeavActiveSuc(false);");
      RoomManager *manager = (RoomManager *)data;
      emit manager->Sig_GuestLeavActiveSuc(false);
   }
}

void RoomManager::SendInviteGuestJoin(const char* id) {
   TRACE6(" RoomManager::SendInviteGuestJoin %s", id);
   char* user = new char[MAX_USER_ID_LEN];
   memset(user, 0, sizeof(user));
   memcpy(user, id, MAX_USER_ID_LEN);

   QVariantMap varmap;
   varmap.insert("userAction", (int)AVIMCMD_Multi_Interact_HostInvietGuestJoin);
   varmap.insert(ActionParam, QString(id));
   QJsonDocument doc;
   doc.setObject(QJsonObject::fromVariantMap(varmap));
   string json = QString(doc.toJson()).toStdString();
   m_pSDK->SendC2CCustomCmd(id, json.c_str(), OnSendInviteGuest, OnSendInviteGuestErr, (void *)user);
}

void RoomManager::OnSendInviteGuest(void *data) {
   TRACE6("RoomManager::OnSendInviteGuestSuc start get id\n");
   if (data) {
      char* p = (char*)(data);
      if (p) {
         delete[]p;
         TRACE6("RoomManager::OnSendInviteGuestSuc delete array\n");
         data = NULL;
         p = NULL;
      }
      TRACE6("RoomManager::OnSendInviteGuestSuc delete array\n");
   }
   
}

void RoomManager::OnSendInviteGuestErr(const int code, const char *desc, void* data) {
   TRACE6("%s  code %d, desc %s\n",__FUNCTION__, code, desc);
   if (data != NULL) {
      QString id;
      if (data) {
         char* p = (char*)(data);
         if (p) {
            TRACE6("RoomManager::OnSendInviteGuestErr  start get id\n");
            id = QString(p);
            TRACE6("RoomManager::OnSendInviteGuestErr  get id %s\n", p);
            delete[]p;
            data = NULL;
            p = NULL;
         }
      }
      TRACE6("senderr OnSendInviteGuestErr code : %d ,desc :%s\n", code, desc);
      m_msgMutex.lock();
      QVariantMap varmap;
      varmap.insert(UserAction, (int)AVIMCMD_Multi_Interact_InviteMsgSendErr);
      varmap.insert(ActionParam, id);
      QJsonDocument doc;
      doc.setObject(QJsonObject::fromVariantMap(varmap));
      string json = QString(doc.toJson()).toStdString();

      CustomMsg msg("", QString::fromStdString(json));
      m_msgList.push_back(msg);
      m_msgMutex.unlock();

      if (m_hEvent) {
         ::SetEvent(m_hEvent);
      }
   }
}

void RoomManager::SendRefuseInvite() {
   TRACE6("%s\n",__FUNCTION__);
   qDebug() << "Slot_CancelJoinActivity";
   QVariantMap varmap;
   varmap.insert(UserAction, (int)AVIMCMD_Multi_Interact_GuestRefuseHostInvite);
   QJsonDocument doc;
   doc.setObject(QJsonObject::fromVariantMap(varmap));
   string json = QString(doc.toJson()).toStdString();
   m_pSDK->SendC2CCustomCmd(GetHostID(), json.c_str(), OnGuestSendRefuseInviteSuc, OnGuestSendRefuseInviteErr, this);

}

void RoomManager::OnGuestSendRefuseInviteSuc(void *) {
   TRACE6("RoomManager::OnGuestSendRefuseInviteSuc\n");
}

void RoomManager::OnGuestSendRefuseInviteErr(const int code, const char *desc, void* data) {
   TRACE6("%s code : %d ,desc :%s\n",__FUNCTION__, code, desc);
   if (data != NULL) {
      RoomManager *manager = (RoomManager *)data;
      emit manager->Sig_SendMsgErr("");
   }
}

void RoomManager::OnChangeToWatcherGuestSuc(void *data) {
   TRACE6("%s\n", __FUNCTION__);
   m_msgMutex.lock();
   QVariantMap varmap;
   varmap.insert(UserAction, (int)AVIMCMD_Multi_Interact_GuestChangeAuthToWatcherSuc);
   QJsonDocument doc;
   doc.setObject(QJsonObject::fromVariantMap(varmap));
   string json = QString(doc.toJson()).toStdString();
   CustomMsg msg("", QString::fromStdString(json));
   m_msgList.push_back(msg);
   m_msgMutex.unlock();

   if (m_hEvent) {
      ::SetEvent(m_hEvent);
   }
}

void RoomManager::OnChangeToWatcherGuestErr(const int code, const char* desc, void* data) {
   TRACE6("%s code: %d  desc:%s\n",__FUNCTION__, code, desc);
   if (!m_bHost) {
      //具体错误码参考https://www.qcloud.com/document/product/268/8423
      if (code == 1001) {  //1001 重复操作 
         return;
      } else if (code == 1003) { //1003 已处于所要状态 .按照成功来处理。
         return;
      } else if (code == 1002) { //互斥操作
         return;
      }
      m_msgMutex.lock();
      QVariantMap varmap;
      varmap.insert(UserAction, (int)AVIMCMD_Multi_Interact_GuestChangeAuthToWatcherErr);
      QJsonDocument doc;
      doc.setObject(QJsonObject::fromVariantMap(varmap));
      string json = QString(doc.toJson()).toStdString();

      CustomMsg msg("", QString::fromStdString(json));
      m_msgList.push_back(msg);
      m_msgMutex.unlock();

      if (m_hEvent) {
         ::SetEvent(m_hEvent);
      }
   }
}

void RoomManager::OnChangeToLiveGuestSuc(void *data) {
   TRACE6("%s\n",__FUNCTION__);
   m_msgMutex.lock();
   QVariantMap varmap;
   varmap.insert(UserAction, (int)AVIMCMD_Multi_Interact_GuestChangeAuthToGuestSuc);
   QJsonDocument doc;
   doc.setObject(QJsonObject::fromVariantMap(varmap));
   string json = QString(doc.toJson()).toStdString();
   CustomMsg msg("", QString::fromStdString(json));
   m_msgList.push_back(msg);
   m_msgMutex.unlock();

   if (m_hEvent) {
      ::SetEvent(m_hEvent);
   }
}

void RoomManager::OnChangeToLiveGuestErr(const int code, const char* desc, void* data) {
   TRACE6("%s code: %d  desc:%s\n",__FUNCTION__, code, desc);
   if (!m_bHost) {
      //具体错误码参考https://www.qcloud.com/document/product/268/8423
      if (code == 1001) {  //1001 重复操作 
         return;
      } else if (code == 1003) { //1003 已处于所要状态 .按照成功来处理。
         return;
      } else if (code == 1002) { //互斥操作
         return;
      }

      m_msgMutex.lock();
      QVariantMap varmap;
      varmap.insert(UserAction, (int)AVIMCMD_Multi_Interact_GuestChangeAuthToGuestErr);
      QJsonDocument doc;
      doc.setObject(QJsonObject::fromVariantMap(varmap));
      string json = QString(doc.toJson()).toStdString();

      CustomMsg msg("", QString::fromStdString(json));
      m_msgList.push_back(msg);
      m_msgMutex.unlock();

      if (m_hEvent) {
         ::SetEvent(m_hEvent);
      }
   }
}

void RoomManager::GuestOffLine(const char* user) {
   if (user && !QString(user).isEmpty()) {
      HostDealGuestLeaveActiveSuc(QString(user));
      DeleteLoginUser(QString(user));
   }
}

void RoomManager::AddAppLoginUser(QString id) {
   TRACE6("%s RoomManager::AddAppLoginUser %s\n",__FUNCTION__, id.toStdString().c_str());
   m_allUserMutex.lock();
   QMap<QString, bool>::iterator iter = m_allRoomUserMap.find(id);
   if (iter == m_allRoomUserMap.end()) {
      m_allRoomUserMap[id] = true;
   }
   m_allUserMutex.unlock();
}

void RoomManager::DeleteLoginUser(QString id) {
   TRACE6("RoomManager::DeleteLoginUser %s\n", id.toStdString().c_str());
   m_allUserMutex.lock();
   QMap<QString, bool>::iterator iter = m_allRoomUserMap.find(id);
   if (iter != m_allRoomUserMap.end()) {
      m_allRoomUserMap.erase(iter);
   }
   m_allUserMutex.unlock();
}

bool RoomManager::IsVhallLiveLoginUser(const char *user) {
   bool bAppLogin = false;
   m_allUserMutex.lock();
   QMap<QString, bool>::iterator iter = m_allRoomUserMap.find(QString::fromStdString(user));
   if (iter != m_allRoomUserMap.end()) {
      TRACE6("RoomManager::IsVhallLiveLoginUser true\n");
      bAppLogin = true;
   }
   m_allUserMutex.unlock();
   return bAppLogin;
}

void RoomManager::GuestSendJoinRoomNotify() {
   QVariantMap varmap;
   varmap.insert(UserAction, (int)AVIMCMD_Multi_Interact_GuestJoinRoomNotify);
   QJsonDocument doc;
   doc.setObject(QJsonObject::fromVariantMap(varmap));
   string json = QString(doc.toJson()).toStdString();
   m_pSDK->SendGroupCustomCmd(json.c_str());
}

void RoomManager::SetHasPushStream(bool start) {
   if (!HaveAlreadyPushStream()) {
      SetAlreadyPushStream(true);
   }
}









