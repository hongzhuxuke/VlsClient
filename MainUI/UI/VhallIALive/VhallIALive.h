#pragma once
#include <QWidget>
#include <QEvent>
#include <QThread>
#include "ui_VhallIALive.h"
#include "CriticalSection.h"
#include "DesktopCameraRenderWdg.h"
#include <QTimer>
#include <QMutex>
#include "RtcGetDeviceList.h"
#include <QPropertyAnimation>
#include <QMoveEvent>
#include "VH_ConstDeff.h"
#include "VhallUI_define.h"
#include "AudioSliderUI.h"
#include "httpnetwork.h"
#include "IInteractionClient.h"
#include "vhalluserinfomanager.h"   
#include <thread> 
#include "Msg_CommonToolKit.h"
#include "Msg_MainUI.h"
#include <QList>
#include <QMutex>
#include <QMap>
#include "WebRtcSDKInterface.h"
#include "VHPaasSDKCallback.h"
#include "LiveToolDlg.h"
#include "ShowTipsWdg.h"
#include "DesktopShareSelectWdg.h"
#include "FullDocWdg.h"
#include "LibCefViewWdg.h"
#include "VhallConfigBroadCast.h"
#include "DeviceTestingWdg.h"

#define MAX_ROOM_RECONNECT_TIMEOUT	5000
#define CHECK_OFFLINE_TIMEOUT       20000
#define MAX_REMOTE_VIEW		         5

#define ENABLESPEAKTIME             5000
#define WEBRTC_JOIN_UID             "join_uid" 
#define WEBRTC_JOIN_UNAME           "join_uname" 
#define WEBRTC_JOIN_UROLE           "join_role" 


class CMemberListDlg;
class VhallIALiveSettingDlg;
class VHDesktopCameraList;
class VhallWaiting;
class VhallRenderWdg;
class VHFadeOutTipWidget;
class InviteJoinWidget;
class VHDesktopSharingUI;
class AlertTipsDlg;
class QTimer;
class CSuspensionDlg;
class MainUILogic;
class ApplyJoinWdg;
class VhallLivePlugInUnitDlg;
class VhallSharedQr;
class PerformanceStatistics;

enum MAIN_VIEW_BACK_GROUD {
   MAIN_VIEW_IS_LIVING = 0,
   MAIN_VIEW_IS_NOT_START = 1,
   MAIN_VIEW_IS_END = 2,
};

#define MAX_CPU_CHECK_TIME  300
#define MIN_CPU_CHECK_TIME  10
#define CPU_CHECK_VALUE     80

class CustomRecvSocketIOMsgEvent : public QEvent {
public:
   CustomRecvSocketIOMsgEvent(QEvent::Type type, Event data) :
      QEvent(type) {
      msg = data;
   };
   Event msg;
};

class GetDeviceList {
public:
   GetDeviceList();
   ~GetDeviceList();
   void SetSubscribe(DevicelistSubscribe* sub);
   void SetRequestType(int type);
   void ProcessTask();
   static DWORD WINAPI ThreadProcess(LPVOID p);
protected:
   DevicelistSubscribe *mDevSub = NULL;
   int mRequestType;
   std::thread* mGetDevThread = nullptr;
   static HANDLE threadHandle;
   static std::atomic_bool bManagerThreadRun;
};

class VhallIALive : public CAliveDlg, public DevicelistSubscribe, public DeviceChangedListener, public IPlayMediaFileObj ,public MsgCallbackDelegate
{
   Q_OBJECT

public:
   VhallIALive(bool showTeaching,QWidget *parent = Q_NULLPTR);
   ~VhallIALive();
   void Init(MainUILogic* obj, VHPaasSDKCallback* callback);

   virtual void OnRecvMsg(std::string fun_name, std::string msg);
   virtual void OnWebViewDestoryed();
   virtual void OnWebViewLoadEnd();
   virtual void OnTitleChanged(const int id, std::string title_name);
   virtual void OnDevicelistNotify(const QList<VhallLiveDeviceInfo>& micList, const QList<VhallLiveDeviceInfo>& cameraList, const QList<VhallLiveDeviceInfo>& playerList, int type);
   virtual void OnDevChanged(const QString &cameraDevId, const int cameraIndex, const QString &micDevId, const int micIndex, const QString &playerDevId, const int playerIndex);
   virtual void OnChangeLayOutMode(LAYOUT_MODE layoutMode);
   virtual bool IsPlayMediaFile();
   virtual bool IsPlayDesktopShare();
   virtual QString GetStreamName();
   virtual QString GetMainViewID();
   virtual QString GetMainSpeakerID();
   virtual QString GetMainViewRole();
   void setMemberAuthority(const bool& bAuth) { mbAuthority = bAuth; }
   bool GetMemberAuthority() { return mbAuthority; }
   void HandsUpTimeout(QString user_id);
   void HandleJoinMember(VhallAudienceUserInfo* event);
   void HandleLeaveMember(VhallAudienceUserInfo* event);
   virtual long long GetShowUserMember();
   virtual bool IsInteractiveHandsUp();
   QWidget* GetShareUIWidget();
   QString GetJoinId();
   QString GetJoinRole();
   QString GetHostId();
   bool GetIsLiving();
   void CheckGuestUserManagerPermisson();
   //添加远端视频成员窗体
   int AppendRenderUser(const QString& uid, const QString &role, const QString& nickName, const bool audio, const bool video, bool showInMainView, bool isInitSpeakList, bool show = true);
   //删除远端视频窗体
   void RemoveRemoteUser(const QString& uid);
   //是否有渲染窗体
   bool IsExistRenderWnd(const QString& uid);
   //设置用户麦克风状态。
   void SetUserMicState(const QString& uid, bool open);
   void SetUserCameraState(const QString& uid, bool open);
   void FadeOutTip(QString str, int type, int showTime = 3000);
   void PreviewCameraCallback(bool success, int code = 0);
   void CloseFloatChatWdg();
   //文件插播相关接口
    //static void OnPlayMediaFileCb(int, void *);
   void SetVedioPlayUi(QWidget*);
   void StopPlayMediaFile();
   void StopShowMediaVideo();
   int GetMediaPlayUIWidth();
   int GetPlayFileState();
   int GetPlayMediaFilePos(signed long long& n64Pos, signed long long& n64MaxPos);
   int OpenPlayMediaFile(const char *szMediaFile);
   int PausePlayMediaFile();
   int ResumePlayMediaFile();
   int SetPlayMediaFilePos(const signed long long& n64Pos);
   void SetPlayMeidaFileVolume(const int volume);
   QWidget* GetRenderWidget();
   //控制打开桌面共享页面
   void OpenScreenShare();
   void CloseScreenShare();

   void AddWebViewFromPlguinDlg(LibCefViewWdg* webView);
   void HandleOnRecvHttpResponse(QEvent* event);
   void HandleRoomReconnected();
   void HandleCustomEvent(QEvent* event);
   void HandleDeviceEvent(QEvent* event);
   void HandleDownLoadHead(QEvent* event);
   void HandleRecvSocketIOMsgEvent(Event& event);
   void HandleCloseAndExit();
   void RoomMixStreamState(bool success);
   void HandleSetLayout(QEvent* event);
   void HandleRecord(QEvent* event);
   void HandleSubScribedError(QEvent *event);
   void HandleMsgReConnect();
   void ProhibiteLedPutDownWheat(const QString& id);
   void CheckHostShowUpperInMemberList();
   void SetChangeRemoteDefinition();
   void ShowInteractiveTools(const int& x, const int& y);
   void ReleaseDocWebView();
   void CreateDocWebView();
   void ShowCheckDev();
   void HandleSendNotice(QEvent* event);
   void HandleSocketIOConeect();
   int ShowScreenSelectDlg(vector<VHD_WindowInfo> &, QWidget* reference);
   void static commitRecordRS(const std::string& msg, int code, const std::string userDataconst, const int& eRequestType, QObject* obj);
   void logCPUAndMemory();
signals:
   void signal_MemberListRefresh();
   void sig_OperationUser(RQData& RqData);
   void sig_ReqApointPageUserList(const int& iApoint);
   void sigInteractiveClicked(bool isReload ,const int& x, const int& y);
   void sigToStopDesktopSharing();
   void sig_RunningState(int, int, int);
   void sigCancelClicked();
protected:
   virtual void resizeEvent(QResizeEvent *event);
   virtual void moveEvent(QMoveEvent *event);
   virtual void showEvent(QShowEvent *event);
   virtual void hideEvent(QHideEvent *event);
   virtual void closeEvent(QCloseEvent *event);
   virtual void customEvent(QEvent *event);
   virtual bool eventFilter(QObject *, QEvent *);
   virtual void keyPressEvent(QKeyEvent *event);
public slots:
   void JsCallAlert(QString url);
   void JsCallQtOpenUrl(QString url);
   void JSCallQtUserOnline(QString param);
   void JSCallQtUserOffline(QString param);
   void JSCallQtPageReady();
   void JsCallQtMsg(QString param);
   void slot_HostPublish(const bool& publish);
   void slot_Upper(const QString& strId, const bool& bOperation = true);
   void slot_setToSpeaker(QString uid, QString userRole);
private slots:
   void slot_OnFloatWndSwitch();
   void slot_RunningState(int cpu, int netSend, int netRecv);
   void slot_OnNetWordDataStat();
   void slot_OnCPUCheckTimeout();
   void slotFadOutTips(const QString& strMsg);
   void slotMouseMoved();
   void slotApplyTimeout();
   void slot_PressKeyEsc();
   void slotLiveToolChoise(const int& iOpType);
   //void slot_EnableSpeak();
   void slot_LiveToolFromDesktopClicked();
   void slot_closeDesktopCameraRender();
   void slot_OnShareSelect();
   void slot_MemberList();
   void slot_DocWebloadFinished(bool);
   void slot_OnClose();
   void slot_OnFresh();
   void slot_OnFullSize();
   void slot_OnMinSize();
   void slot_OnSetting();
   void slot_OnShare();
   void slot_OnCopy();
   void slot_DocLoading();
   void slot_DocLoadEnd();
   void slot_DestroyApplyWdg(QString strId);
   void slot_CameraBtnClicked();
   void slot_MicButtonClicked();
   void slot_PlayerButtonClicked();
   void slot_BtnScreenClicked();
   void slot_BtnStreamClicked();
   void slot_RejectSpeak(const QString& strId);
   void slot_OnPlayFileClicked();
   void slot_OnToSpeakClicked();
   void slot_CancleApplyToSpeak();

   void slot_CheckSpeakUserTimeOut();
   void slot_RoomReConnectTimer();
   void slot_MicVolumnChanged(int);
   void slot_PlayerVolumnChanged(int);
   void slot_setInMainView(QString uid, QString userRole);

   void slot_ClickedNotSpeak(QString);	 //点击渲染窗口中的下麦。
   void slot_SwitchMainViewLayout();
   void slot_ClickedCamera(QString, bool);
   void slot_ClickedMic(QString, bool);

   void slot_AllowRaiseHands(const bool& checked);

   void slotMemberClose();
   void slotAgreeUpper(const QString& strId);
   void slot_SetMainViewTimeout();
   void slot_SetDesktopMainViewTimeout();
   void slot_SetMediaFileMainViewTimeout();
   void slot_PublishLocalStreamTimeout();
   void slot_PublishDesktopStreamTimeout();
   void slot_PublishMediaFileStreamTimeout();
   void slot_ReSetLocalCaptureTimeout();
   void slot_CheckSubScribeTimeout();

   void slot_ReSetLayoutTimeout();
   void slot_GuestAgreeHostInvite();
   void slot_GuestRefuseHostInvite();
   //void slot_featurePermissionRequested(const QUrl &, QWebEnginePage::Feature);
   void slot_HidePlayUI();
   void ApplyToSpeak();
   QString GetDocSetMainViewJson(const QString &uid, const QString& type);

   void slot_OnClickedDoc();
   void slot_ClickedWhiteboard();
   void slot_OnClickedPluginUrl(const int& iX, const int& iy);
   void slot_changed(bool bShowMem);
   void slot_ReSetTipsPos();
   void slot_closeItem();
   void slot_changeProfile(int curProfile);
   void slot_SettingDlgProfileChanged();
   void slot_commitRecord(const int& eRequestType);
private:
   enum eStreamType {
      eStreamType_Media,
      eStreamType_DeskTop
   };
   VhallRenderWdg* CopyNewRender(VhallRenderWdg* render, bool bIsMainView);

   void InitBroadCast(bool call_back = false);
   void InitLeftBtnEnableState(bool mainIsUser = false);
   void StartLocalStream();
   void StartPlayMediaStream(bool changePorfile = false);
   void StartDeskTopStream(int index);
   void StopDeskTopStream();
   void InitSelectDev();
   void ClearApplyJoinMap();
   void ShowApplyJoinWnd();
   void InitMaxSize();
   void StopPublishDeskTopStream();
   void PlayRemoteUser(VhallRenderWdg* render, QString user_id, bool hasVideo);
   void createMemberListDlg();
   void InitRenderView(const VhallActiveJoinParam &param);
   void InitVhallSDK();
   void InitMainPlugin(const QString& url);
   int InitMediaStream(const char *szMediaFile);
   void StartLiveWithOpenDevice();
   void HandleInitVhallSDK();
   void ChangeMainView(QString uid);
   bool IsMainRenderNormalPos();
   void ConfigRoomMainView(const QString& uid);
   void SetUserMainView(const QString& uid, QString role, int enableNotice);
   VhallRenderWdg *GetRenderWnd(const QString& uid);
   LayoutMode GetGridUserSize();

   void ExitRoom(int type = 0);

   void ChangeLocalMicDevState(bool open, const QString & uid);
   void ChangeLocalCameraDevState(bool open, const QString & uid);

   void SelectMicDev(QList<VhallLiveDeviceInfo> &micDev);
   void SelectPlayOutDev(QList<VhallLiveDeviceInfo> &playOutDev);
   void StartLocalCapture();
   //上麦请求
   void LocalUserToSpeak();

   void RecvHostInviteMsg();
   void RemoveAllRender();
   void ResetToNospeakState();

   void ResetMediaCtrlWdgPos();
   void ReSizeMediaCtrlWdg();
   void HandleGetLocalStream(QEvent* event);
   void HandlePushLocalStreamSuc(QEvent *event);
   void HandlePaasSetMainView(QEvent* event);

   void HandleStreamRemoved(QEvent *event);
   void HandleSubScribSuc(QEvent *event);

   void HandleStreamFailed(QEvent *event);
   void HandlePushMediaFileStreamSuc(QEvent *event);
   void HandleSetMediaFileMainView(QString streamId);
   void HandleConfigMediaFileMainView(QEvent *event);
   void HandleConfigBoradCast(QEvent *event);
   void HandleConfigBoradCastOnlyOne(QEvent *event);
   void HandleUnPushMediaFileStream(QEvent *event);
   void HandleLayOutMode(QEvent* event);
   void HandleConfigDesktopMainView(QEvent *event);
   void HandlePushStreamReConnect(QEvent *event);
   void HandleConfigMainView(QEvent* event);

   void HandleHostExitRoom();
   void HandleSubStream(QEvent *event);
   void HandleAddSpeaker(Event &e);
   void HandleRecvAnnouncement(Event &e);

   void HandleKickOutUser(Event &e);
   void HandleAgreeSpeaker(Event &e);
   void HandleNoSpeaker(Event &e);
   void HandleSwitchDeviceClose(Event &e);
   void HandleSwitchDeviceOpen(Event &e);
   void HandleSwitchMainView(Event &e);
   void HandleStartLiving();
   void HandleStopLiving();

   void HandleLostLocalCameraStream(bool restart = false);
   void HandleStartPush(Event &e);
   void HandleOverPush(Event &e);
   void HandlePlayMediaFileSuc();
   void HandleConnectRoomErr();
   void HandleConnectRoomSuc();
   void HandleRePublishLocalStream();
   void HandleRePublishDesktopStream();
   void HandleRePublishMediaStream();
   void HandleDevCheck();
   void HandleVSSGetRoomBaseInfo(int, std::string);
   void HandlePaasSDKFailedEvent(int, std::string, int);
   void HandlePaasSDKConnectSuc(std::string msg, int eventCode);
   void HandleVSSStartLive(int, std::string);
   void HandleStopLive(int, std::string);
   void HandleReSubScribeLocalStream();
   void HandleReSubScribeDesktopStream();
   void HandleReSubScribeMediaStream();
   void HandlePushDeskTopSuc(QString &dataMsg, bool mbVideo);
   void HandleSetDesktopMainView(QString streamId);
   void PushMsgEvent(int type, QString data = "", bool bVideo = true);
   void SetSpeakUserOffLine(const QString &uid, bool offline);

   void RemoveCheckSpeakUser(const QString& uid);
   void CheckSpeakUserOffLine();
   bool IsInSpeakList(std::vector<QString>&);

   void StopPushMeidaFileStream();
   void ShowPlayUIState(bool show);
   void ReleaseMediaStream();
   bool IsEnalbePlayMedia();
   LayoutMode GetCurrentLayoutMode(LAYOUT_MODE type);
   void SetLayOutMode(LayoutMode);
   void UpdataCloseCamera();
   void HandleGetDeskTopSuc();
   void HandleGetDeskTopErr();

   bool IsEnableUsing();
   void GetSpeakUserList();
   void ResetPlayMediaFileAndDesktopShare();
   void SetMainViewBackGroud(int state);
   bool IsLiveTimeShort();
   void StopStream(const QString& uid);
   void AddRenderToMap(const QString &id, VhallRenderWdg*render);
   void RemoveRendFromMap(const QString& id);
   void ResetGridLayOutMode();
   QString GetLayoutStringName(LayoutMode mode);
   LAYOUT_MODE GetLayOutMode(QString& strLayoutMode);
   void ResetListWidgetSmallViewSize();


   void HandleHttpResponseNotSpeak(const QString &url, const QString& msg, int code, const QString& uid, bool isNoticeMsg);
   void HandleHttpResponseChangeToSpeak(const QString &url, const QString& msg, int code, const QString& uid, bool isNoticeMsg);
   void HandleHttpResponseStopwebinarr(const QString &url, const QString& msg, int code, const QString& uid, bool isNoticeMsg);
   void HandleHttpResponseHandsup(const QString &url, const QString& msg, int code, const QString& uid, bool isNoticeMsg);
   void HandleHttpResponseSpeakList(const QString &url, const QString& msg, int code, const QString& uid, bool isNoticeMsg, QString msgData, bool hasVideo);
   void HandleVSSSetStream(int, std::string, std::string, std::string);
   void HandleVSSSendInvite(int, std::string, std::string, std::string);
   void HandleSetHandlsUpMsg(CustomOnHttpResMsgEvent* customEvent);
   void HandleVSSSendMsg(int, std::string, std::string, std::string);
   void HandleRejectInviteResp(int libCurlCode, std::string msg, std::string, std::string);
   void HandleVSSAgreeApply(int libCurlCode, std::string msg, std::string, std::string);
   void HandleVSSRejectApply(int libCurlCode, std::string msg, std::string, std::string);
   void HandleVSSSetMainScreenResp(int libCurlCode, std::string msg, std::string, std::string, QString roleName);
   void HandleVSSNoSpeakResp(int libCurlCode, std::string msg, std::string uid, bool bShowNotice);
   void HandleVSSChangeToSSpeakResp(int, std::string, std::string);
   void HandleVSSAgreeInvite(int libCurlCode, std::string msg, std::string, bool);
   void HandleVSSHandsUpRespParam(int libCurlCode, std::string msg, std::string);
   void HandleVSSCancelHandsUpRespParam(int libCurlCode, std::string msg, std::string);
   void HandleChatMsg(CustomOnHttpResMsgEvent* customEvent);

   void ParamJsonToUserInfo(std::string json, QString& uid, QString& nickname, QString role);
   bool ParamRtcInfoToUserInfo(QString user_data, VhallAudienceUserInfo& info);
   void HttpSendStartLive();
   void HttpSendStopLive();
   void HttpSendNotSpeak(QString uid, bool showNotice);
   void HttpSendInvite(QString uid, bool showNotice);
   void HttpSendDeviceStatus(QString receiveJoinId, int device, int type);
   void HttpSendMixLayOutAndProfile(QString strLayout, QString videoProfile);
   void HttpSendSwitchSpeaker(QString receive_uid, QString userRole, int enableNotice);
   bool IsEnableClicked();
   bool IsExistScreenShare();
   bool IsPlayUiShow();
   void ClearSpeakerUser();
   void ResetMemberState();
private:
   QMap<QString, ApplyJoinWdg*> mApplyJoinWdg;

   enum eHandsUp {
      eHandsUp_close = 0,
      eHandsUp_open,
   };

   Ui::VhallIALive ui;
   CMemberListDlg* mpMemberListDlg;
   HWND mDesktopView;				//桌面共享回显UI页面

   VhallIALiveSettingDlg *mpSettingDlg = NULL;
   bool mbInitLayOut = false; //用于判断主持人登录时是否需要设置布局。设置布局条件是登录时只有支持人自己或者上麦用户只有主持人一个人。
   bool mIsFullSize = false;

   //QWebEngineView *mpMainDocWebEngineView = NULL;	  //主窗口文档区域
   //QWebChannel* mpMainDocWebChannel = NULL;
   bool mbIsLoadFinished = false;


   LibCefViewWdg* mDocCefWidget = nullptr;

   bool mbDocLoadFinished = false;
   bool mbWebLoadFinished = false;
   QString mLocalStreamID;  //本地流ID，不作为订阅者。
   int mCPUValue = 0;
   VhallActiveJoinParam mActiveParam;
   bool mbIsFirstLoad = true;
   QString mCurCameraDevID;
   QString mCurMicDevID;
   QString mCurPlayerID;
   QString mCurPlayMediaFileStreamID;
   QString mCurDeskTopStreamID;
   QString mCurPlayFileName;
   QString mCurDesktopCaptrueDevId;
   int mCurrentCameraIndex = 0;
   int mCurrentMicIndex = 0;
   int mCurrentPlayerIndex = 0;
   int mLocalVideoProfile = 0;
   QString mRemoteVideoProfile;
   bool mbInitInRoom = false;
   bool mLastIsMaxSize;
   bool mbIsFullDoc = false;
   bool mbIsCameraOpen = false;
   bool mbIsMicOpen = false;
   bool mbIsPlayerOpen = false;
   bool mbIsAlreadyShow = false;
   QMutex mDeviceMutex;
   QList<VhallLiveDeviceInfo> mMicList;
   QList<VhallLiveDeviceInfo> mCameraList;
   QList<VhallLiveDeviceInfo> mPlayerList;
   RtcGetDeviceList mDeviceGetThread;
   QRect mNormalRect;
   QWidget* mpVedioPlayUi = NULL;

   bool mbIsGuestSpeaking = false;//用户是否已上麦，true 已经上麦。false 没有上麦。
   bool mIsCallStartMixStream = false;
   VhallWaiting *mpExitWaiting = NULL;
   VhallWaiting *mpStartWaiting = NULL;
   QTimer mRoomReConnectTimer;	 //房间链接异常时触发此定时器进行重连。	MAX_ROOM_RECONNECT_TIMEOUT
   QTimer mStartLiveTimer;
   QDateTime mStartLiveDateTime;

   int mMicVolume = 0;
   int mPlayerVolume = 0;
   bool mbIsExitToLivelist = true;//是否推出到活动列表
   InviteJoinWidget *mpInviteJoinWdg = NULL;
   QMutex mSpeakUserMapMutex;
   std::map<QString, bool> mSpeakUserCheckMap;
   QTimer mCheckSpeakUserTimeOut;
   QWidget *mMediaFilePlayWdg = NULL;
   QWidget *mDesktopWdg = NULL;

   bool mbHasVideoFrame = false;
   bool mbIsDeviceChecking = false;
   bool mbIsPlayMediaFile = false;		 //正在插播视频文件标识；
   bool mbIsDeskTop = false;
   VHDesktopSharingUI* m_pScreenShareToolWgd = NULL;

   bool mbEnableStopStream = true; //此标志位是用于判断开始桌面共享时GetLocalStream是否获取完毕。
   bool mbEnableStopDesktopStream = true;  //此标志位是用于判断开始桌面共享时GetLocalStream是否获取完毕。
   bool mbEnableStopPlayMediaStream = true;  //此标志位是用于判断开始桌面共享时GetLocalStream是否获取完毕。
   bool mbAdaptation = false;
   bool mbCallRoomDisConnect = false;

   std::atomic_bool mbExitRoom = false;
   AlertTipsDlg* mpHttpResponseTips = NULL;
   QMutex mLayoutMutex;
   //QTimer* mpEnableSpeakTimer = NULL;  //限制频繁上下麦
   bool mbClickedCloseExit = false;
   QMutex mRenderViewMutex;
   std::map<QString, VhallRenderWdg*> mRenderMap;
   std::atomic_bool mbIsDelete = false;
   MainUILogic* mpMainUILogic = NULL;

   VhallConfigBroadCast *mConfigBroadCast = nullptr;
   //QTimer mConfigBroadCastTimer;
   QTimer mSetMainViewTimer;
   QTimer mSetDeskTopMainViewTimer;
   QTimer mSetMediaFileMainViewTimer;
   QTimer mRePublishLocalStreamTimer;
   QTimer mRePublishDesktopStreamTimer;
   QTimer mRePublishMediaFileStreamTimer;
   QTimer mReSetLocalCapture;
   QTimer mCheckSubScribeStream;
   QString mSubScribeUserID;

   QTimer mReSetLayOutTimer;
   QTimer* mCPUCheckTimer = nullptr;
   std::atomic_int mCurrentLayout;
   std::atomic_bool bFirstConnected;
   VHPaasSDKCallback* callbackReciver;
   bool bProhibit = false;  //被禁言
   bool mbIsHostSetGuestToSpeaker = false;     //主持人是否已经设置嘉宾为主讲人
   bool mbIsHostUnPublished = false;           //主持人是否已经下麦
   LiveToolDlg* mLiveToolDlg = nullptr;
   LayoutMode mCurrentMode;

   VhallSharedQr *mpShareDlg = nullptr;
   bool mbAuthority = true;

   QTimer* mDocLoadingTimer = nullptr;
   QTimer* mDocLoadEndTimer = nullptr;
   int mDocLoadingPointCount = 0;
   QTimer* mPlayUIHideTimer = nullptr;

   ShowTipsWdg *mShowTipsWdgPtr = nullptr;
   ShowTipsWdg *mDesktopShowTipsWdgPtr = nullptr;
   PerformanceStatistics* mPerformanceStatisticsPtr = nullptr;
   //WPcap* mpWPcapPtr = nullptr;
   QTimer* mpWCapStatTimerPtr = nullptr;
   int mCPUCheckTimeOut;
   bool mbIsNoticeCPUHigh = false;
   bool mbIsOpenSubDoubleStream = true;
   uint mLastCPUCheckTime = 0;
   int mNetDevDataCount = 0;
   int mCurrentCaptureScreenIndex = 0;
   DesktopCameraRenderWdg *mDesktopCameraRenderWdgPtr = nullptr;
   QWidget *mTeachingWidget = nullptr;
   int mTeachingPageIndex = 0;
   bool mIsShowTeachingWidget = false;
   FullDocWdg* mFullDocWdg = nullptr;
   DesktopShareSelectWdg* mDesktopShareSelectWdg = nullptr;
   bool mbIsPushMediaErr = false;
   DeviceTestingWdg *mDeviceTestingWdg = nullptr;

};
