#pragma once
#include <QString>
#include <QEvent>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>




enum TipsType {
   TipsType_None = 0,
   TipsType_Normal = 1,
   TipsType_ChangeProfile_Standard = 2,
   TipsType_ChangeProfile_Fluency = 3,
   TipsType_Success = 4,
   TipsType_Error = 5,
   TipsType_SystemBusy = 6,
   TipsType_OnlyOne_Context = 7,
   TipsType_MediaFilePlay = 8,
   TipsType_Bitrate_Down = 9,
   TipsType_MIC_NOTICE = 10,
};

class VhallLiveDeviceInfo {
public:
   VhallLiveDeviceInfo(QString id, QString name, int index) {
      devId = id;
      devName = name;
      devIndex = index;
   }

   QString devId;		//设备id
   QString devName;  //名字
   int devIndex;		//索引
};

enum ResponseInviteType {
   ResponseInviteType_None = 0,
   ResponseInviteType_Accept = 1,
   ResponseInviteType_Refuse = 2,
   ResponseInviteType_Err = 3,
};


enum RenderViewState {
   RenderView_None = 0,
   RenderView_NoJoin = 1,
   RenderView_HasCamera = 2,
   RenderView_NoCamera = 3,
};

enum VhallSDKDevRequest {
   DevRequest_StartLive = 1,	//打开设置页面获取设备列表
   DevRequest_GetDevList = 2,	    //开播时获取设备列表
   DevRequest_InitSDK = 3,
};

//各个角色当前的演讲状态。
enum SpeakState {
   Host_InMainView = 1,
   Host_InListView = 2,
   Guest_InMainView = 3,
   Guest_InListView = 4,
   Other_InMainView = 5,
   Other_InListView = 6,
};

enum LAYOUT_MODE {
   LAYOUT_MODE_GRID = 0, //均分模式
   LAYOUT_MODE_FLOAT = 1,//悬浮模式
   LAYOUT_MODE_TILED = 2, //主次模式
   LAYOUT_MODE_ONLY = 3 //一个大画面
};


enum VhallLiveStreamType {
   VhallLiveStreamType_Video = 2,
   VhallLiveStreamType_Desktop = 3,
   VhallLiveStreamType_MeidaFile = 4,
};


//自定义互动直播ILIVESDK处理事件。由于iliveSDK回调均在主线程执行，通过postEvent进行异步处理防止在回调中重复调用SDK接口。
const QEvent::Type CustomEvent_LoginSuc = (QEvent::Type)5001;
const QEvent::Type CustomEvent_LoginErr = (QEvent::Type)5002;
const QEvent::Type CustomEvent_LogOutSuc = (QEvent::Type)5003;
const QEvent::Type CustomEvent_LogOutErr = (QEvent::Type)5004;
const QEvent::Type CustomEvent_CreateRoomSuc = (QEvent::Type)5005;
const QEvent::Type CustomEvent_CreateRoomErr = (QEvent::Type)5006;
const QEvent::Type CustomEvent_JoinRoomSuc = (QEvent::Type)5007;
const QEvent::Type CustomEvent_JoinRoomErr = (QEvent::Type)5008;
const QEvent::Type CustomEvent_ReleaseSuc = (QEvent::Type)5009;
const QEvent::Type CustomEvent_ReleaseErr = (QEvent::Type)50010;
const QEvent::Type CustomEvent_LeaveRoomSuc = (QEvent::Type)5011;
const QEvent::Type CustomEvent_LeaveRoomErr = (QEvent::Type)50012;
const QEvent::Type CustomEvent_ShowMainWdg = (QEvent::Type)50013;
const QEvent::Type CustomEvent_ShowLoginDlg = (QEvent::Type)50014;
const QEvent::Type CustomEvent_ExitApp = (QEvent::Type)50015;
const QEvent::Type CustomEvent_CheckMixStreamServerErr = (QEvent::Type)50016;
const QEvent::Type CustomEvent_VhallLiveHttpData = (QEvent::Type)50017;


const QEvent::Type CustomEvent_SelectGetDev = (QEvent::Type)30000;
const QEvent::Type CustomEvent_GetLocalStreamSuc = (QEvent::Type)30003;
const QEvent::Type CustomEvent_GetLocalStreamErr = (QEvent::Type)30004;
const QEvent::Type CustomEvent_RecvMsg = (QEvent::Type)30005;
const QEvent::Type CustomEvent_StartLiveGetDev = (QEvent::Type)30006;
const QEvent::Type CustomEvent_DevChanged = (QEvent::Type)30007;
const QEvent::Type CustomEvent_ConnectRoomSuc = (QEvent::Type)30008;
const QEvent::Type CustomEvent_ConnectRoomErr = (QEvent::Type)30009;
const QEvent::Type CustomEvent_SubScribSuc = (QEvent::Type)30010;
const QEvent::Type CustomEvent_STREAM_FAILED = (QEvent::Type)30011;
const QEvent::Type CustomEvent_ROOM_DISCONNECTED = (QEvent::Type)30012;
const QEvent::Type CustomEvent_STREAM_REMOVED = (QEvent::Type)30013;
const QEvent::Type CustomEvent_InitVhallSDK = (QEvent::Type)30014;
const QEvent::Type CustomEvent_PushLocalStreamSuc = (QEvent::Type)30015;
const QEvent::Type CustomEvent_ConfigBoradCast = (QEvent::Type)30016;
const QEvent::Type CustomEvent_GetLocalFileStreamSuc = (QEvent::Type)30018;
const QEvent::Type CustomEvent_GetLocalFileStreamErr = (QEvent::Type)30019;
const QEvent::Type CustomEvent_PushMediaFileStreamSuc = (QEvent::Type)30020;
const QEvent::Type CustomEvent_ConfigBoradCastOnlyOne = (QEvent::Type)30021;
const QEvent::Type CustomEvent_ConfigMediaFileMainView = (QEvent::Type)30022;

const QEvent::Type  CustomEvent_GetDeskTopStreamErr = (QEvent::Type)30023;
const QEvent::Type CustomEvent_PushDeskTopStreamSuc = (QEvent::Type)30024;
const QEvent::Type CustomEvent_GetDeskTopStreamSuc = (QEvent::Type)30025;

const QEvent::Type CustomEvent_UnPushMediaFileStream = (QEvent::Type)30026;
const QEvent::Type CustomEvent_SubStream = (QEvent::Type)30027;
const QEvent::Type CustomEvent_ConfigDesktopMainView = (QEvent::Type)30028;
const QEvent::Type CustomEvent_UnPushDeskTopStream = (QEvent::Type)30029;
const QEvent::Type CustomEvent_HostExitRoom = (QEvent::Type)30030;
const QEvent::Type CustomEvent_RenderView = (QEvent::Type)30031;

const QEvent::Type CustomEvent_SettingGetLocalStreamSuc = (QEvent::Type)30032;
const QEvent::Type CustomEvent_SettingGetLocalStreamErr = (QEvent::Type)30033;
const QEvent::Type CustomEvent_SettingGetLocalDev = (QEvent::Type)30034;
const QEvent::Type CustomEvent_Show = (QEvent::Type)30035;
const QEvent::Type CustomEvent_Hide = (QEvent::Type)30036;

const QEvent::Type CustomEvent_SettingGetCameraDev = (QEvent::Type)30037;
const QEvent::Type CustomEvent_SettingGetMicDev = (QEvent::Type)30038;
const QEvent::Type CustomEvent_SettingGetPlayerDev = (QEvent::Type)30039;

const QEvent::Type CustomEvent_SettingGetCameraDevSuc = (QEvent::Type)30040;
const QEvent::Type CustomEvent_SettingGetMicDevSuc = (QEvent::Type)30041;
const QEvent::Type CustomEvent_SettingGetPlayerDevSuc = (QEvent::Type)30042;

const QEvent::Type CustomEvent_SettingGetCameraDevErr = (QEvent::Type)30043;
const QEvent::Type CustomEvent_SettingGetMicDevErr = (QEvent::Type)30044;
const QEvent::Type CustomEvent_SettingGetPlayerDevErr = (QEvent::Type)30045;
const QEvent::Type CustomEvent_RePublishLocalStream = (QEvent::Type)30046;
const QEvent::Type CustomEvent_RePublishDeskTopStream = (QEvent::Type)30047;
const QEvent::Type CustomEvent_RePublishMediaStream = (QEvent::Type)30048;
const QEvent::Type CustomEvent_ReSubScribCameraStream = (QEvent::Type)30049;
const QEvent::Type CustomEvent_ReSubScribDesktopStream = (QEvent::Type)30050;
const QEvent::Type CustomEvent_ReSubScribMediaStream = (QEvent::Type)30051;
const QEvent::Type CustomEvent_InitSDKConnectRoomSuc = (QEvent::Type)30052;
const QEvent::Type CustomEvent_PushStreamReConnect = (QEvent::Type)30053;
const QEvent::Type CustomEvent_CloseSettingDlg = (QEvent::Type)30054;
//const QEvent::Type CustomEvent_StartLiving = (QEvent::Type)30055;
const QEvent::Type CustomEvent_StopLiving = (QEvent::Type)30056;
const QEvent::Type CustomEvent_CloseExit = (QEvent::Type)30057;
const QEvent::Type CustomEvent_LostStream = (QEvent::Type)30058;
const QEvent::Type CustomEvent_JSCallUserOnLine = (QEvent::Type)30059;
const QEvent::Type CustomEvent_JSCallUserOffLine = (QEvent::Type)30060;
const QEvent::Type CustomEvent_CustomRecvSocketIOMsgEvent = (QEvent::Type)30061;
const QEvent::Type CustomEvent_OpenCameraErrMsgEvent = (QEvent::Type)30062;
const QEvent::Type CustomEvent_OpenMicErrMsgEvent = (QEvent::Type)30063;
const QEvent::Type CustomEvent_OnRecvHttpResponse = (QEvent::Type)30064;
const QEvent::Type CustomEvent_SetToSpeakEnble = (QEvent::Type)30065;
const QEvent::Type CustomEvent_DeleteVhallLive = (QEvent::Type)30066;
const QEvent::Type CustomEvent_ConfigMainView = (QEvent::Type)30067;
const QEvent::Type CustomEvent_RoomReconnected = (QEvent::Type)30068;
const QEvent::Type CustomEvent_RoomReconnecting = (QEvent::Type)30069;

const QEvent::Type CustomEvent_PushLocalStreamErr = (QEvent::Type)30070;
const QEvent::Type CustomEvent_PushDeskTopStreamErr = (QEvent::Type)30071;
const QEvent::Type CustomEvent_PushMediaFileStreamErr = (QEvent::Type)30072;

const QEvent::Type CustomEvent_VSSRejectInviteResp = (QEvent::Type)30073;
const QEvent::Type CustomEvent_VSSSetStream = (QEvent::Type)30074;
const QEvent::Type CustomEvent_VSSRespParam = (QEvent::Type)30075;
const QEvent::Type CustomEvent_VSSSetMainScreenResp = (QEvent::Type)30076;
const QEvent::Type CustomEvent_VSSStartLiveResp = (QEvent::Type)30077;
const QEvent::Type CustomEvent_VSSNoSpeakResp = (QEvent::Type)30078;
const QEvent::Type CustomEvent_VSSChangeToSSpeakResp = (QEvent::Type)30079;
const QEvent::Type CustomEvent_VSSHandsUpRespParam = (QEvent::Type)30080;
const QEvent::Type CustomEvent_VSSAgreeInvite = (QEvent::Type)30081;
const QEvent::Type CustomEvent_VSSStopLive = (QEvent::Type)30082;
const QEvent::Type CustomEvent_VSSSendInvite = (QEvent::Type)30083;
const QEvent::Type CustomEvent_VSSSetHandsUp = (QEvent::Type)30084;
const QEvent::Type CustomEvent_VSSAgreeApply = (QEvent::Type)30085;
const QEvent::Type CustomEvent_VSSSpeak = (QEvent::Type)30086;
const QEvent::Type CustomEvent_VSSApply = (QEvent::Type)30087;
const QEvent::Type CustomEvent_VSSGetRoomBaseInfo = (QEvent::Type)30089;
const QEvent::Type CustomEvent_OnRoomFailedEvent = (QEvent::Type)30090;
const QEvent::Type CustomEvent_OnRoomSuccessedEvent = (QEvent::Type)30091;
const QEvent::Type CustomEvent_VSSGetBannedList = (QEvent::Type)30092;
const QEvent::Type CustomEvent_VSSGetKickedList = (QEvent::Type)30093;
const QEvent::Type CustomEvent_VSSGetOnlineList = (QEvent::Type)30094;
const QEvent::Type CustomEvent_VSSGetSpecialList = (QEvent::Type)30095;
const QEvent::Type CustomEvent_RoomStreamMixed = (QEvent::Type)30096;
const QEvent::Type CustomEvent_RecvVssSocketIoMsg = (QEvent::Type)30097;
const QEvent::Type CustomEvent_VSSGetRoomSpeakList = (QEvent::Type)30098;       //获取上麦列表
const QEvent::Type CustomEvent_ConfigBroadLayOut = (QEvent::Type)30099;
const QEvent::Type CustomEvent_StopCaptureMediaFile = (QEvent::Type)30100;
const QEvent::Type CustomEvent_SendNotice = (QEvent::Type)30101;
const QEvent::Type CustomEvent_VSSRejectApply = (QEvent::Type)30102;
const QEvent::Type CustomEvent_ChatMsg = (QEvent::Type)30103;
const QEvent::Type CustomEvent_CopyShareUrl = (QEvent::Type)30104;
const QEvent::Type CustomEvent_SocketIOConnect = (QEvent::Type)30105;
const QEvent::Type CustomEvent_RecordEvent = (QEvent::Type)30106;
const QEvent::Type CustomEvent_RecordEventErr = (QEvent::Type)30107;
const QEvent::Type CustomEvent_Dispatch_Publish = (QEvent::Type)30108;
const QEvent::Type CustomEvent_RemoteStreamAdd = (QEvent::Type)30109;
const QEvent::Type CustomEvent_SubScribedError = (QEvent::Type)30110;
const QEvent::Type CustomEvent_DownLoadFile = (QEvent::Type)30111;
const QEvent::Type CustomEvent_JsCallQtKickOut = (QEvent::Type)30112;
const QEvent::Type CustomEvent_LibCefMsg = (QEvent::Type)30113;
const QEvent::Type CustomEvent_LibCefLoadFinished = (QEvent::Type)30114;
const QEvent::Type CustomEvent_JsCallQtStartLive = (QEvent::Type)30115;
const QEvent::Type CustomEvent_JsCallQtShareTo = (QEvent::Type)30116;
const QEvent::Type CustomEvent_JsCallQtJoinActivity = (QEvent::Type)30117;
const QEvent::Type CustomEvent_JsCallQtOpenUrl = (QEvent::Type)30118;
const QEvent::Type CustomEvent_JsCallQtStartVoiceLive = (QEvent::Type)30119;
const QEvent::Type CustomEvent_DownLoadSharePic = (QEvent::Type)30120;
const QEvent::Type CustomEvent_VSSCancleApply = (QEvent::Type)30121;
const QEvent::Type CustomEvent_CefOnCreate = (QEvent::Type)30122;
const QEvent::Type CustomEvent_CefOnRecvMsg = (QEvent::Type)30123;
const QEvent::Type CustomEvent_CefOnDestory = (QEvent::Type)30124;
const QEvent::Type CustomEvent_CefOnTitleChanged = (QEvent::Type)30125;
const QEvent::Type CustomEvent_WebSocketReconnecting = (QEvent::Type)30126;
const QEvent::Type CustomEvent_AudioDevRemoved = (QEvent::Type)30127;
const QEvent::Type CustomEvent_AudioCaptureErr = (QEvent::Type)30128;

const QEvent::Type CustomEvent_CreateObsLogic = (QEvent::Type)30129;
const QEvent::Type CustomEvent_INIT_DEVICE = (QEvent::Type)30130;
const QEvent::Type CustomEvent_ADD_CAMERA = (QEvent::Type)30131;
const QEvent::Type CustomEvent_VIDEO_CAPTURE_ERROR = (QEvent::Type)30132;
const QEvent::Type CustomEvent_DEV_CHECK = (QEvent::Type)30133;
const QEvent::Type CustomEvent_RTC_LIVE_RECVCAPTURE_FARME = (QEvent::Type)30134;
const QEvent::Type CustomEvent_ReSizeCenterRender = (QEvent::Type)30135;
const QEvent::Type CustomEvent_ShowTeaching = (QEvent::Type)30136;

const QEvent::Type CustomEvent_END = (QEvent::Type)40000;


class CustomDownLoadEvent : public QEvent {
public:
   CustomDownLoadEvent(QEvent::Type type, int code, QString url, QString savePath, QString msg, int index) :
      QEvent(type) {
      mCode = code;
      mDownLoadUrl = url;
      mMsg = msg;
      mSavePath = savePath;
      mIndex = index;
   };
   int mCode;
   int mIndex;
   QString mDownLoadUrl;
   QString mSavePath;
   QString mMsg;
};


class DeviceChangedListener {
public:
   virtual void OnDevChanged(const QString &cameraDevId, const int cameraIndex, const QString &micDevId, const int micIndex, const QString &playerDevId, const int playerIndex) = 0;
   virtual void OnChangeLayOutMode(LAYOUT_MODE layoutMode) = 0;
};

class IPlayMediaFileObj {
public:
   virtual bool IsPlayMediaFile() = 0;
   virtual bool IsPlayDesktopShare() = 0;
};

class  DevicelistSubscribe {
public:
   virtual void OnDevicelistNotify(const QList<VhallLiveDeviceInfo>& micList, const QList<VhallLiveDeviceInfo>& cameraList, const QList<VhallLiveDeviceInfo>& playerList, int type) = 0;
};


class CustomDeleteLiveMsgEvent : public QEvent {
public:
   CustomDeleteLiveMsgEvent(QEvent::Type type, bool bExit, int reaSon) :
      QEvent(type) {
      mbExit = bExit;
      mReason = reaSon;
   };
   bool mbExit;
   int mReason;
};

class CustomRecvMsgEvent : public QEvent {
public:
   CustomRecvMsgEvent(QEvent::Type type, QString data = QString(), bool hasVideo = true) :
      QEvent(type) {
      msg = data;
      mbHasVideo = hasVideo;
   };
   QString msg;
   QString mStreamId;
   QString mJoinUserId;
   QString mUserData;
   int mode;
   bool mbHasVideo;
   int mStreamType;
};

class CefMsgEvent : public QEvent {
public:
   CefMsgEvent(QEvent::Type type, QString fun, QString msg) :
      QEvent(type) {
      mFunName = fun;
      mCefMsgData = msg;
   };
   QString mFunName;
   QString mCefMsgData;
};

class CustomSocketIOMsg : public QEvent {
public:
   CustomSocketIOMsg(QEvent::Type type, QJsonObject data) :
      QEvent(type) {
      recvObj = data;
   };
   QJsonObject recvObj;
};

class RecordEvent : public QEvent {
public:
   RecordEvent(QEvent::Type type, const int& iState) :
      QEvent(type) {
      miState = iState;
   };

   int miState;
};

class RecordEventErr : public QEvent {
public:
   RecordEventErr(QEvent::Type type, const QString& str) :
      QEvent(type) {
      msg = str;
   };

   QString msg;
};

class CustomOnHttpResMsgEvent : public QEvent {
public:
   CustomOnHttpResMsgEvent(QEvent::Type type, QString url = QString(), QString msg = QString(), int code = 0, QString uid = QString(), bool bNoticeMsg = false, QString msgData = QString(), bool hasVideo = true) :
      QEvent(type) {
      mUrl = url;
      mMsg = msg;
      mCode = code;
      mUid = uid;
      mbNoticeMsg = bNoticeMsg;
      mData = msgData;
      bHasVideo = hasVideo;
   };
   QString mUrl;
   QString mMsg;
   QString mData;
   QString mUid;
   QString mRecvUserId;
   QString mRoleName;
   QString mNickName;
   QString mLayout;
   QString mVideoProfile;
   int mEventCode;
   int mRequestTime = 0;
   int devType = 0;
   int status = 0;
   int mUv = 0;
   int mCode;
   bool mbNoticeMsg;
   bool bHasVideo;
   bool mIsAgree;
   bool bIsActiveLive;
   bool is_banned;
   bool is_open;
   bool mediaCoreStreamErr;
};

class CustomReConnectEvent : public QEvent {
public:
   CustomReConnectEvent(QEvent::Type type, void* ptr) :
      QEvent(type) {
      msg = ptr;
   };
   void* msg;
};

class CustomDevChangedEvent : public QEvent {
public:
   CustomDevChangedEvent(QEvent::Type type, QString cameraDevId, int cameraIndex, QString micDevId, int micIndex, QString playerDevId, int playerIndex) :
      QEvent(type) {
      this->cameraDevId = cameraDevId;
      this->cameraIndex = cameraIndex;
      this->micDevId = micDevId;
      this->micIndex = micIndex;
      this->playerDevId = playerDevId;
      this->playerIndex = playerIndex;
   };

   QString cameraDevId;
   int cameraIndex;
   QString micDevId;
   int micIndex;
   QString playerDevId;
   int playerIndex;
   int localVideoDefinition;
   QString remoteVideoDefinition;
};


class VhallActiveJoinParam {
public:
   VhallActiveJoinParam() {
      bExitToLivelist = true;
      bIsLiving = false;
      live_time = 0;
      mbIsH5Live = false;
   }
   void Clear() {
      bExitToLivelist = true;
      mbIsH5Live = false;
      device_audio = 0;
      device_video = 0;
      handsUp = 0;
      currPresenter.clear();
      bizRole = 0;
      live_time = 0;
      reportUrl.clear();
      mainShow.clear();
      speaker_list.clear();
      invite_list.clear();
      against_url.clear();
      room_token.clear();
      vss_token.clear();
   }
   bool mbIsH5Live;
   bool bExitToLivelist;
   bool bIsLiving;			//互动是否已经开始直播 true:已经开始
   int  device_audio;		//麦克风状态1开启，0关闭
   int  device_video;		//摄像头状态1开启，0关闭
   int  handsUp;           //是否能举手1可以，0不可以
   int  bizRole;			//sdk参数
   long live_time; //开始直播时长
   QString vss_token;
   QString definition;         //清晰度
   QString currPresenter;	//当前主讲人参会ID
   QString mainShow;			//当前主画面参会ID
   QString mainShowRole;	//当前主画面参会成员的角色
   QString speaker_list;	//上麦参会列表
   QString invite_list;		//已发邀请列表
   QString join_uid;			//当前用户id
   QString user_id;			//用户id
   QString role_name;		//当前用户角色  当前参会角色host主持人，guest嘉宾，assistant助理，user观众
   QString against_url;		//sdk参数
   QString room_token;		//sdk参数
   QString reportUrl;		//sdk参数
   QString chatUrl;			//聊天 url
   QString thirdPushStreamUrl;
   QString hostNickName;
   QString hostJoinUid;
   QString msgToken;
   QString userNickName;
   QString imageUrl;		 //头像url
   QString streamName;	     //活动ID	：366476354
   QString webinarName;	 //活动名称：例如 Test直播互动
   QString pluginUrl;	    //文档插件url
   QString watchLayout;    //画面布局。
   QString vss_room_id;
   QString paas_account_id;
   QString pass_access_token;
   QString pass_app_id;
   QString paas_inav_room_id;
};

class SpeakUser {
public:
   SpeakUser() {}
   SpeakUser(QString id, QString role, QString name, int audio = 1, int video = 1) {
      userId = id;
      roleName = role;
      nickName = name;
      nAudio = audio;
      nVideo = video;
   }
   QString userId;
   QString roleName;
   QString nickName;
   int nAudio;
   int nVideo;
   bool bOffLine = false;
   unsigned long long offLineTime;
};

