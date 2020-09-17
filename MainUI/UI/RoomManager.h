#ifndef MSGPROCESSOR_H
#define MSGPROCESSOR_H

#include <QObject>
#include <QThread>
#include <QMutex>
#include <QTimer>
#include <windows.h>
#include <QMap>
#include "VH_ConstDeff.h"
#include "IInterActivitySDK.h"

#define MAX_JOINER   4         //目前上麦最多仅支持4人。
#define MAX_USER_ID_LEN 32
#define MAX_NOTIFY_TIMEOUT 10000
#define MAX_NOTIFY_PUSHSTREAM_TIMEOUT 2000

enum REFUSE_REASON {
   REFUSE_REASON_NONE = -1,
   REFUSE_MAX_GUEST_ONLINE = 1,
   REFUSE_HOST_NO_PUSHTING = 2,
};

/// 成员事件id。参考EndpointEventId
enum SDKEventID {
   SDKEvent_ID_NONE = 0, ///< 无。
   SDKEVENT_ID_ENDPOINT_ENTER = 1, ///< 进入房间事件。
   SDKEVENT_ID_ENDPOINT_EXIT = 2, ///< 退出房间事件。
   SDKEVENT_ID_ENDPOINT_HAS_CAMERA_VIDEO = 3, ///< 有发摄像头视频(也就是来至摄像头设备AVCameraDevice或外部视频捕获设备AVExternalCapture的视频)事件。
   SDKEVENT_ID_ENDPOINT_NO_CAMERA_VIDEO = 4, ///< 无发摄像头视频(也就是来至摄像头设备AVCameraDevice或外部视频捕获设备AVExternalCapture的视频)事件。
   SDKEVENT_ID_ENDPOINT_HAS_AUDIO = 5, ///< 有发音频事件。
   SDKEVENT_ID_ENDPOINT_NO_AUDIO = 6, ///< 无发音频事件。
   SDKEVENT_ID_ENDPOINT_HAS_SCREEN_VIDEO = 7, ///< 有发屏幕视频(也就是捕获计算机屏幕画面所得到的视频)事件。
   SDKEVENT_ID_ENDPOINT_NO_SCREEN_VIDEO = 8, ///< 无发屏幕视频(也就是捕获计算机屏幕画面所得到的视频)事件。
   SDKEVENT_ID_ENDPOINT_HAS_MEDIA_VIDEO = 9, ///< 有播放视频事件。
   SDKEVENT_ID_ENDPOINT_NO_MEDIA_VIDEO = 10, ///< 无播放视频事件。
};

struct CustomMsg {
public:
   QString user;
   QString json;

   CustomMsg(QString id = QString(), QString data = QString())
      :user(id)
      , json(data) {
   }
};

struct RoomUser {
public:
   QString userid;
   QString headUrl;                    //头像地址
   E_RoomUserType 	eRole;            //用户角色
   unsigned long long    joinTime;     //加入的时间	
   bool  bOpenCamera;	   //是否开启摄像头
   bool  bOpenMic;		   //是否开始麦克风
   bool  bOpenWndShare;	   //是否开启屏幕共享
   bool  bMemJoined;       //是否嘉宾加入活动已确认，已经加入房间
   bool  bInviteJoinedUser;//是否是主播邀请加入的用户。
   bool  bHostKickOffGuest; //是否是主播主动踢出的。
   RoomUser()
      : eRole(E_RoomUserInvalid)
      , joinTime(0)
      , bOpenCamera(false)
      , bOpenMic(false)
      , bOpenWndShare(false)
      , bMemJoined(false)
      , bInviteJoinedUser(false)
      , bHostKickOffGuest(false) {
   };

   RoomUser& operator = (const RoomUser& rhs) {
      userid = rhs.userid;
      headUrl = rhs.headUrl;
      eRole = rhs.eRole;
      joinTime = rhs.joinTime;
      bOpenCamera = rhs.bOpenCamera;
      bOpenMic = rhs.bOpenMic;
      bOpenWndShare = rhs.bOpenWndShare;
      bMemJoined = rhs.bMemJoined;
      bInviteJoinedUser = rhs.bInviteJoinedUser;
      bHostKickOffGuest = rhs.bHostKickOffGuest;

      return *this;
   }
};

class RoomManager : public QThread, public IInterAcivtityUser {
   Q_OBJECT

public:
   RoomManager(QObject *parent = NULL);
   ~RoomManager();

   /***
   * 每次开始新活动和停止活动时调用。
   ***/
   void JoinInteractive(const QString& id, bool bHost);
   void LeaveInteractive();

   void RegisterActivitySDKObject(IInterActivitySDK* sdk);
   void OnRecvCustomCmdMsg(const QString& user, const QString& json);
   void OnSendCustomMsgErr(int code, const std::string& desc, std::string& data);
   void OnSendC2CCmdSucess(std::string& data);
   void OnMemStatusChanged(int event_id, std::string identifier);
   void OnPushStreamSuccess(bool suc);
   void OnChangeToWatcherGuestSuc(void *data);
   void OnChangeToWatcherGuestErr(const int code, const char* desc, void* data);
   void OnChangeToLiveGuestSuc(void *data);
   void OnChangeToLiveGuestErr(const int code, const char* desc, void* data);

   void GuestDealChangeAuthAllSuc();
public:
   /*嘉宾发送上麦请求*/
   virtual void SendJoinRequest(const char* headUrl);
   /*嘉宾发送下麦请求*/
   virtual void SendCancelJoinRequest();
   /*主播邀请嘉宾上麦*/
   virtual void SendInviteGuestJoin(const char* id);
   /*嘉宾拒绝主播上麦*/
   virtual void SendRefuseInvite();

   virtual bool IsVhallLiveLoginUser(const char *user);
   virtual void GetActiveUserState(const char *user, bool& bIsOpenMic, bool &bIsOpenCamera, bool &bIsOpenDesktopSharing);
   virtual const char* GetHostID();
   virtual bool IsActiveUser(const char *user);
   virtual bool IsAllowInviteJoinActive(const char* user);
   virtual bool IsMemberEnableInvite(const char *user);
   virtual void SetLockUserView(const char *lockUser);
   virtual  std::string GetLockUserView();
   virtual bool IsLockUserView();
   virtual void NotifySelfMediaState();
   virtual void NotifyHostPushState();
   virtual void RemoveActiveMember(const char* userID);
   virtual void NotifyHostExitRoom();
   virtual void ClearGuestUser(bool push);
   virtual void SetUserHeadImageUrl(const char* url);
   virtual void KickOutUser(const char*  user);
   virtual void GuestOffLine(const char* user);
   virtual void SetHasPushStream(bool start);
public:
   static void OnSendJoinRequest(void *);
   static void OnSendJoinRequestErr(const int code, const char *desc, void* data);

   static void OnSendCancelJoinRequest(void *);
   static void OnSendCancelJoinRequestErr(const int code, const char *desc, void* data);

   static void OnSendInviteGuest(void *);
   static void OnSendInviteGuestErr(const int code, const char *desc, void* data);

   static void OnGuestSendJoinSuc(void *);
   static void OnGuestSendJoinErr(const int code, const char *desc, void* data);

   static void OnGuestSendLeaveSuc(void *);
   static void OnGuestSendLeaveErr(const int code, const char *desc, void* data);

   static void OnHostSendAgreeJoinSuc(void *);
   static void OnHostSendAgreeJoinErr(const int code, const char *desc, void* data);

   static void OnHostSendDisAgreeJoinMsgSuc(void *);
   static void OnHostSendDisAgreeJoinMsgErr(const int code, const char *desc, void* data);

   static void OnHostSendKickOffLineSuc(void *);
   static void OnHostSendKickOffLineErr(const int code, const char *desc, void* data);

   static void OnGuestSendRefuseInviteSuc(void *);
   static void OnGuestSendRefuseInviteErr(const int code, const char *desc, void* data);

   static void OnHostAgreeGuestLeaveSuc(void *);
   static void OnHostAgreeGuestLeaveErr(const int code, const char *desc, void* data);
protected:
   virtual void run();

   private slots:
   void Slot_NotifyTimeOut();
private:
   void Init();
   void Uninit();

   void DealCustomCmdMsg(QString user, QString json);
   void ParseCustomMessage(const QString& sender, int nUserAction, QString szActionParam);
   void HostSendAgreeJoinMsg(const QString& id);
   void HostSendDisAgreeJoinMsg(const QString& id);
   void HostDealGuestSendJoinRequest(const QString& sender, QString szActionParam);
   void HostDealGuestJoinActiveSuc(const QString& sender, QString szActionParam);
   void HostDealGuestJoinActiveErr(const QString& sender);
   void HostDealGuestRefuseHostInvite(const QString& sender);
   void HostDealEndpointEnterRoom();
   void HostDealGuestSendLeaveActiveRequest(const QString& sender);
   void HostDealGuestLeaveActiveSuc(const QString& sender);
   void HostDealGuestLeaveActiveErr(const QString& sender);
   void HostDealJoinerMemberState(int state, std::string identifier);
   void GuestDealNotifyHostID(const QString& sender, const QString& szActionParam);
   void HostDealGuestCloseScreenResp();
   void HostDealPushSteamCallBack();

   void GuestDealHostPushState(const QString& szActionParam);
   void GuestDealHostNotifyJoinedUser(const QString& szActionParam);
   void GuestDealSendCancelJoinRequest(const QString& sender);
   void GuestDealHostAgreeGuestJoinMultiActive(const QString& sender, QString szActionParam);
   void GuestDealHostInviteJoin();
   void GuestSendJoinRoomNotify();

   void GuestDealHostRefuseGuestJoin(QString);
   void GuestDealHostNotifyLeaveUser(const QString& szActionParam);
   void GuestDealJoinerMemberState(int state, std::string identifier);
   void GuestDealHostCloseGuestMic(const QString& sender);
   void GuestDealHostOpenGuestMic(const QString& sender);
   void GuestDealHostCloseGuestCamera(const QString& sender);
   void GuestDealHostOpenGuestCamera(const QString& sender);
   void GuestDealHostNotifyCloseScreen();
   void GuestDealHostNotifyLockUser(const QString& lockUser);
   void GuestDealHostExitRoom();

   void ExitThread(bool bExit);
   bool IsExitThread();
   void DealEnterRoomMemberState(int state, std::string identifier);

   void NotifyHostID();
   void MemberDealMedaiState(const QString& szActionParam);
   void UpdateMemberState(const QString&, const bool mic, const bool camera, const bool share);
   QString GetLockViewUserID();

   void SetAlreadyPushStream(bool set);
   bool HaveAlreadyPushStream();

   void AddAppLoginUser(QString id);
   void DeleteLoginUser(QString id);

signals:
   void Sig_HostInviteJoin();
   void Sig_SendLeaveActiveRequestErr();
   void Sig_GuestLeavActiveSuc(bool);
   void Sig_GuestJoinActiveSuc(bool);
   void Sig_ActiveMemberState(QString user, bool joined, QString head = QString());
   void Sig_HostStartPush(bool);
   void Sig_GuestRefuseHostInviteResp(QString);
   void Sig_HostExitRoom();
   void Sig_HostNotifyGuestCloseCamera(bool close);
   void Sig_HostNotifyGuestCloseMic(bool close);
   void Sig_GuestRecvLockView(QString id, bool lock);
   void Sig_RecvCloseScreenNotify(QString);
   void Sig_RecvCloseScreenResp();
   void Sig_HostNotifyAllActiveMember(QString);
   void Sig_GuestSendJoinActiveMsgErr();
   void Sig_UpdateMemberMediaState(QString, bool, bool, bool);
   void Sig_HostKickOutUser();
   void Sig_GuestExitRoom(QString, QString);
   void Sig_SendMsgErr(QString);
   void Sig_ChangetoLiveGuest();
   void Sig_ChangeToWatchGuest();
   void Sig_HostAgreeGuestLeaveErr(QString id);
private:

   int GetJoinerCount();
   void UpdataJoinerCount(const QString& user, const bool add, const bool bHostInviteUser = false);
   void SetJoinerHeadUrl(const QString& user, const QString& headUrl);
   void OnGuestDealSendJoinActiveErr();
   void NotifyJoinerToAllUsers();
   void NotifyLockViewUser();
   void SetGuestKickOffState(const QString& user, bool bKickOff);
   void KickOutUserSuc(QString);


private:
   static HANDLE m_hEvent;
   IInterActivitySDK *m_pSDK = NULL;

   static QMutex  m_msgMutex;
   static QList<CustomMsg> m_msgList;
   QMutex   m_joinUserMutex;
   QMap<QString, RoomUser> m_joinUserMap;   //嘉宾状态
   QMutex   m_allUserMutex;
   QMap<QString, bool>  m_allRoomUserMap;   // 所有加入房间的成员
   
   QMutex   m_threadMutex;
   bool     m_bExitThread = false;         //线程退出标志位
   QTimer*  m_pNotifyTimer = NULL;
   bool     m_bHost = false;
   QString  m_LoginID;
   QMutex   m_hostIDMutex;
   QString  m_hostID;
   char     m_hostIdArry[128];

   E_RoomUserType m_eUserType;

   QMutex   m_lockViewMutex;
   QString  m_lockUserView;
   QString  m_headUrl;
   QMutex   m_PushStreamMutex;
   bool     m_bAlreadyPushStream = false;

   static QList<QString> m_kickOffLineUser;
};

#endif // MSGPROCESSOR_H
