#ifndef __MAINUI_LOGIC__H_INCLUDE__
#define __MAINUI_LOGIC__H_INCLUDE__

#pragma once

#include "IMainUILogic.h"

#include <QObject>
#include <QTimer>

#include "MediaDefs.h"
#include "vhalldebugform.h"
#include "vhareamark.h"
#include "vhdesktoptip.h"
#include "vhallLiveListViewDlg.h"
#include "VhallLivePlugInUnitDlg.h"
#include "VhallActivityModeChoiceDlg.h"
#include "vhallwaiting.h"
#include "Msg_MainUI.h"
#include "Msg_VhallRightExtraWidget.h"
#include <atomic>
#include "vhAliveinteraction.h"
#include "VhallUI_define.h"
#include "CriticalSection.h"
#include "VHPaasSDKCallback.h"
#include "VHMonitorCapture.h"
#include "ThreadToopTask.h"

class VhallSharedQr;
class DesktopShareSelectWdg;

class CustomVhallLiveHttpDataEvent : public QEvent {
public:
   CustomVhallLiveHttpDataEvent(QEvent::Type type, QString url, QString httpResponse, int code, QString uid, bool notice) :
      QEvent(type) {
      mUrl = url;
      mHttpResponseMsg = httpResponse;
      mCode = code;
      mUid = uid;
      mbNoticeMsg = notice;
   };
   QString mUrl;
   QString mHttpResponseMsg;
   QString mUid;
   int mCode;
   bool mbNoticeMsg;
};

#define DEF_ILIVESDK_TIMEOUT  20000

class VhallJoinActivity;
class VhallLiveMainDlg;
class FullScreenLabel;
class CaptureSrcUI;
class MultiMediaUI;
class AudioSliderUI;
class QTimer;
class VHDesktopSharingUI;
//class RoomManager;
class STRU_VHALLRIGHTEXTRAWIDGET_INIT_USERINFO;
class STRU_VHALLRIGHTEXTRAWIDGET_COMMONINFO;
class IPluginWebServer;
class CPluginDataProcessor;
class CRecordDlg;
class VhallLoginWidget;
class HttpTaskWorkThread;
class CDesktopModeChatParentWdg;
class VhallIALive;


class MainUILogic : public QObject, public IMainUILogic{
   Q_OBJECT
public:
   MainUILogic(void);
   ~MainUILogic(void);

public:
   BOOL Create();
   void Destroy();

   /////////////////////////////IUnknown接口/////////////////////////////////////////
   virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void ** appvObject);
   virtual ULONG STDMETHODCALLTYPE   AddRef(void);
   virtual ULONG STDMETHODCALLTYPE   Release(void);

   virtual void STDMETHODCALLTYPE GetMainUIWidget(void** apMainUIWidget, int type = 0);

   virtual void STDMETHODCALLTYPE GetShareUIWidget(void** apShareUIWidget, int type = 0);

   virtual void STDMETHODCALLTYPE GetContentWidget(void** apContentWidget, int type = 0);

   virtual void STDMETHODCALLTYPE GetRenderPos(int& x, int& y);

   virtual void STDMETHODCALLTYPE GetLiveActiveMediaPlayBasePos(void** apContentWidget, int type = 0);

   virtual int STDMETHODCALLTYPE GetMediaPlayUIWidth(int type = 0);

   virtual void STDMETHODCALLTYPE GetHwnd(void** apMsgWnd, void** apRenderWnd);

   virtual void STDMETHODCALLTYPE SetTipForceHide(bool);

   virtual void STDMETHODCALLTYPE SetDesktopSharingUIShow(bool);

   virtual void STDMETHODCALLTYPE GetDesktopSharingUIWinId(HWND *);

   virtual void STDMETHODCALLTYPE SetMicMute(bool bMute);

   virtual void STDMETHODCALLTYPE SetSpeakerMute(bool bMute);

   virtual void STDMETHODCALLTYPE AddImageWidget(void *);

   virtual void STDMETHODCALLTYPE RemoveImageWidget(void *);

   virtual int STDMETHODCALLTYPE  GetVHDialogShowCount();

   virtual void STDMETHODCALLTYPE FadoutTip(wchar_t *);

   virtual void STDMETHODCALLTYPE UpdateVolumn(int);

   virtual bool STDMETHODCALLTYPE ISHasActivityList();

   virtual void * STDMETHODCALLTYPE GetLiveToolWidget();

   virtual void STDMETHODCALLTYPE KitoutEvent(bool, wchar_t *userId, wchar_t *role);

   virtual void STDMETHODCALLTYPE OnlineEvent(bool, wchar_t *userId, wchar_t *role, wchar_t *name, int synType);

   virtual bool STDMETHODCALLTYPE IsShowMainWidget();

   virtual void STDMETHODCALLTYPE SetHttpProxy(const bool enable = false, const char* ip = NULL, const char* port = NULL, const char* userName = NULL, const char* pwd = NULL);
   virtual void STDMETHODCALLTYPE ShowLoginDlg(bool* exist);
   virtual void * STDMETHODCALLTYPE GetParentWndForTips();
   virtual bool STDMETHODCALLTYPE IsInteractive();
   virtual QString STDMETHODCALLTYPE StrPointURL();
   virtual bool STDMETHODCALLTYPE IsStopRecord();
   virtual void STDMETHODCALLTYPE SetCutRecordDisplay(const int iCutRecord);
   virtual bool STDMETHODCALLTYPE IsRecordBtnhide();
   virtual void STDMETHODCALLTYPE SetVedioPlayUi(QWidget* pVedioPlayUI, int iLiveType);
   virtual bool STDMETHODCALLTYPE IsLoadUrlFinished();
   virtual char* STDMETHODCALLTYPE GetMsgToken();
   virtual bool STDMETHODCALLTYPE SetEnabledInteractive(const bool& bEnabled = false);
   virtual void  SetMessageSocketIOCallBack(InteractionMsgCallBack);
   virtual void  StopInteraction();

   int GetDisplayCutRecord();
   //窗口是否显示
   bool IsDesktopShow();
   //用户是否能够使用语音转换功能
   virtual void STDMETHODCALLTYPE EnableVoiceTranslate(bool enable = false);
   virtual bool STDMETHODCALLTYPE GetVoiceTranslate();
   //语音转换功能按键是否禁用和开启
   virtual void STDMETHODCALLTYPE OpenVoiceTranslateFun(bool open = false);
   virtual bool STDMETHODCALLTYPE IsOpenVoiceTranslateFunc();
   virtual void STDMETHODCALLTYPE SetVoiceTranslateFontSize(int size);
   virtual bool STDMETHODCALLTYPE IsShareDesktop();
   virtual void STDMETHODCALLTYPE SetRecordState(const int iState);
   virtual void* GetCAliveDlg();
   /**
   @brief 打开文件播放。
   @details 开始播放本地音频\视频文件，播放文件前，最好先调用isValidMediaFile()检查文件的可用性。
   @param [in] szMediaFile 文件路径(可以是本地文件路径，也可以是一个网络文件的url);
   @remark
   1、支持的文件类型:<br/>
   *.aac,*.ac3,*.amr,*.ape,*.mp3,*.flac,*.midi,*.wav,*.wma,*.ogg,*.amv,
   *.mkv,*.mod,*.mts,*.ogm,*.f4v,*.flv,*.hlv,*.asf,*.avi,*.wm,*.wmp,*.wmv,
   *.ram,*.rm,*.rmvb,*.rpm,*.rt,*.smi,*.dat,*.m1v,*.m2p,*.m2t,*.m2ts,*.m2v,
   *.mp2v, *.tp,*.tpr,*.ts,*.m4b,*.m4p,*.m4v,*.mp4,*.mpeg4,*.3g2,*.3gp,*.3gp2,
   *.3gpp,*.mov,*.pva,*.dat,*.m1v,*.m2p,*.m2t,*.m2ts,*.m2v,*.mp2v,*.pss,*.pva,
   *.ifo,*.vob,*.divx,*.evo,*.ivm,*.mkv,*.mod,*.mts,*.ogm,*.scm,*.tod,*.vp6,*.webm,*.xlmv。<br/>
   2、目前sdk会对大于640*480的视频裁剪到640*480;<br/>
   3、文件播放和系统声音采集不应该同时打开，否则文件播放的声音又会被系统声音采集到，出现重音现象;
   @note
   屏幕分享和播片功能都是通过辅路流传输，所以屏幕分享和播片互斥使用;
   辅路流被自己占用，设备操作回调中，返回错误码AV_ERR_EXCLUSIVE_OPERATION(错误码见github上的错误码表);
   被房间内其他成员占用，设备操作回调中，返回错误码AV_ERR_RESOURCE_IS_OCCUPIED(错误码见github上的错误码表);
   */
   virtual int OpenPlayMediaFile(const char*  szMediaFile, int liveType);

   virtual int SetPlayMediaFileVolume(int vol, int liveType);
   /**
   @brief 关闭文件播放。
   */
   virtual void ClosePlayMediaFile(int liveType);
   /**
   @brief 从头播放文件。
   @return 操作结果，NO_ERR表示无错误。
   @note 只有在处于播放状态下(E_PlayMediaFilePlaying)，此接口才有效，否则返回ERR_WRONG_STATE;
   */
   virtual int RestartMediaFile(int liveType);
   /**
   @brief 暂停播放文件。
   @return 操作结果，NO_ERR表示无错误。
   */
   virtual int PausePlayMediaFile(int liveType);
   /**
   @brief 恢复播放文件。
   @return 操作结果，NO_ERR表示无错误。
   */
   virtual int	ResumePlayMediaFile(int liveType);
   /**
   @brief 设置播放文件进度。
   @param [in] n64Pos 播放位置(单位: 秒)
   @return 操作结果，NO_ERR表示无错误。
   */
   virtual int SetPlayMediaFilePos(const signed long long& n64Pos, int liveType);
   /**
   @brief 获取播放文件进度。
   @param [out] n64Pos 当前播放位置(单位: 秒)
   @param [out] n64MaxPos 当前所播放文件的总长度(单位: 秒)
   @return 操作结果，NO_ERR表示无错误。
   */
   virtual int GetPlayMediaFilePos(signed long long& n64Pos, signed long long& n64MaxPos, int liveType);
   virtual int GetPlayFileState(int liveType);

   virtual void OnRecvPushStreamErrorEvent();
   void SendMsg(int eMsgType, RQData& vData);

   virtual void RunTask(int type,void *appdata = nullptr);
public:
   int CreateRecord(bool isClose = false);
   // 处理消息
   void DealMessage(DWORD adwMessageID, void* apData, DWORD adwLen);

   // 处理显示Widget
   void DealShowWidget(void* apData, DWORD adwLen);

   void DealPrivateStart(void* apData, DWORD adwLen);

   // 处理Click控制
   void DealClickControl(void* apData, DWORD adwLen);

   //处理右侧插件创建消息
   void DealExtraWidgetCreate(void* apData, DWORD adwLen);

   //处理右侧插件活跃消息
   void DealExtraWidgetActive(void* apData, DWORD adwLen);

   //处理软件共享和区域共享
   void DealProcessSrc(void* apData, DWORD adwLen);


   //处理关闭主窗口
   void DealDoCloseMainWindow(void* apData, DWORD adwLen);

   //区域共享
   void DealShareRegion(void* apData, DWORD adwLen);

   //码率统计
   void DealPushStreamStatus(void* apData, DWORD adwLen);

   //隐藏区域共享
   void DealHideShareRegion(void* apData, DWORD adwLen);

   //底层流信息通知
   void DealStreamNotify(void* apData, DWORD adwLen);

   void DealPushStreamBitRateDown();

   //处理初始化
   void DealInitCommonData(void* apData, DWORD adwLen);

   //处理实时字幕
   void DealVoiceTransition(void *apData, DWORD adwLen);

   //处理日志信息
   void DealLog(void *apData, DWORD adwLen);
   //处理刷新完毕消息
   void DealEndFefresh(void *apData, DWORD adwLen);

   void DealStreamPushSuccess();
   void DealRecordChange(void* apData, DWORD adwLen);
   //处理打点录制请求回应
   void DeaPointRecordRs(void* apData, DWORD adwLen);
   void DealHttpTask(void* apData, DWORD adwLen);
   void HandleCloseVhallActive(bool exitToLiveList, int exitReason);
   //设置logo是否隐藏
   void DoHideLogo(bool);

   //询问并且弹出网页
   void AskForOpenWebPage(QString sureBtn, QString cancelBtn, QString title, QString content, QString url);

   void DealVoiceTranslate();
   void SlotControlUser(RQData* RqData);
public:
   static std::string GetPaasSDKJoinContext();

public slots:
   void myfunction();
   void StopDesktopSharing();

   void MainWidgetMove();
   void SlotSetMicMute(bool);

   void JsCallQtStartLive(QString msg);
   void JsCallQtShareTo(QString msg);
   void JsCallQtDebug(QString msg);
   void JsQtDebug(QString msg);
   void JsCallQtStartVoiceLive(QString url);
   //In the future
   void JsCallQtOpenUrl(QString url);
   void JsCallQtJoinActivity();
   void JSCallQtResize();

   void StartLive(QString url);
   void ShareUrl(QString url);
   void JoinActivity();

   void AppExit(bool);
   void slot_MainDlg();
   void SlotAskForOpenWebPage(QString sureBtn, QString cancelBtn, QString title, QString content, QString url);
   void slot_OnClickedPluginUrlFromDesktop(bool isReload,const int& iX, const int& iY);
   void slot_OnShareSelect();
private slots:
   void Slot_ClosePluginDlg();
   void Slot_ExitApp();
   void Slot_LoginApp();
   void Slot_PwdLogin();
   void slot_hideCaptureSrcUi();
   void Slot_DisableWebListDlg();
   void Slot_EnableWebListDlg();

   void slotCameraClicked();
   void slotSettingClicked();
   void slotLiveClicked();
   void slotMicClicked();
   void Slot_CloseAreaShare();
protected:
   virtual void customEvent(QEvent *);
   void HandleShowMainWdg();

signals:
   void SigFadoutTips(QString);
   void SigSetMicMute(bool);
   void SigAskForOpenWebPage(QString sureBtn, QString cancelBtn, QString title, QString content, QString url);
   void SigSDKReturn();
   void SigOnlineEvent(bool, QString, QString, QString, int);
   void SigKikoutEvent(bool, QString, QString);
private:
   void InitFlashDocWebServer();
   void HandleStartLive(QEvent* event);
   void HandleWebSocketReconnecting();
   void HandleRecord(QEvent* event);
   void HandleJoinActiveDirect(QEvent *event);
   void HandleJoinActivityInit(QEvent *event);
   void HandleCreateObsLogic();
   void HandleStartCreateOBS();
   void HandleStartRepushStream();
   void HandleDownLoadHead(QEvent *event);
   void HandleJoinActivityByPwd(QEvent *event);
   void HandleCreateRecord(QEvent *event);
   void HandleCloseLiveUi(bool isClose);
   void HandleDefaultRecord(QEvent* event);
   void HandleStopWebniar(QEvent *event);
   void HandleStopUIState(bool closeMainUi = false);
   void HandleStartWebniar(QEvent* event);
   void HandleVSSGetRoomAttributes(QEvent *event);
   void HandleVSSGetPushInfo(QEvent *event);
   void HandleDispatchPublish(QEvent *event);
   void HandlePushStream(bool bIsStartPush, bool mediaCoreStreamErr, bool bScheduling);
   void HandleRecvPaasSocketIOMsg(QEvent *event);
   void HandleRecvCustomRecvSocketIOMsgEvent(QEvent *event);
   void HandleUploadLiveType(QEvent* event);
   void HandleVSSGetRoomBaseInfo(QEvent* event);
   void HandleConnectFailedEvent(QEvent* event);
   void HandleSendNotice(QEvent* event);
   void HandleRemoteStreamAdd(QEvent *event);
   void HandleSubScribedError(QEvent *event);
   void HandleSocketIOConeect();
   void HandleRoomSuccessEvent(QEvent* event);
   void HandleMsgReConnect();
   void HandlePushStreamState(bool bScheduling, int dwPublishState, bool mediaCoreStreamErr, bool isAlreadyLive);
   virtual void StopWebnair(bool isClose);
   static void OnHttpRequestData(const char* data, int dataLen);
   void SetEnabledLiveListWdg(bool enable);
   void StopPlayMediaFile();
   void HidePluginDataDig();
   // 显示活动列表
   void ShowActivityList(bool bShow = true);
   void HandleJoinAcivity(bool bPwdLogin);
   // 显示主界面
   void ShowMainWidget();
   void StartVoiceTranslate(bool open);
   void CheckPrivateStartExitApp();
   // 最小化应用
   void MinimizeApp(void *parent);

   // 关闭应用
   void CloseApp(void *parent, bool bIsActiveDisconnect = false, int reason = 0);

   // 共享源窗口
   void ShareSrcUI(int iX, int iY);

   //区域共享
   void ShareRegion();

   //桌面共享
   void ShareDesktop();
   void StartToCaptureDesktop(VHD_WindowInfo windowInfo);
   //多媒体
   //void ShowMultiMediaUI();
   //录制
   void ShowRecordDlg();
   //推流   dwPublishState要操作的推流状态 
   void PublishStream(DWORD dwPublishState, bool mediaCoreStreamErr, bool isAlreadyLive = false);

   //监控上报推流情况
   void MonitorReportPublish(BOOL bStartPublish);

   //读取配置信息
   void InitConfig();

   //上报用户事件
   void UpLoadEvents();

   //收到websocket消息
   void RecvWebsocketMsg(char *);

   //收到socketIO消息
   void RecvSocketIOMsg(char *);

   void HandleAppExit(bool isNotice = true);
   void HandleCloseMainLiveWnd();
   void CloseTurnToLiveListDlg();
   void HandleShowLoginDlg(const STRU_MAINUI_WIDGET_SHOW& param);
   void ToJoinActivityDirect(QString id, QString nick, QString password, bool isPwdLogin, QString token);
   bool ToJoinActivityByLogin(QString id, QString nick, QString password);
   void ClearHttpTaskThread();
   bool IsUploadLiveTypeSuc(int type, const QString token);
   void SetDefaultRecord(bool closeMainUi);
   void GetVSSMemberList(int pageIndex = -1);
   void ControlUser(RQData &data);
   void ReloadPlguinUrl(bool isReload = true);
   void UploadInitInfo();
   void PostEventToExitApp();
   void SendGetRoomBaseInfo(const std::string vss_token, const std::string vss_room_id,int time = 0);
   void SendGetRoomAttributes(const std::string vss_token, const std::string vss_room_id, int time = 0);
   void SendGetPushInfo(const std::string vss_token, const std::string vss_room_id, int time = 0);
   void ShowExitWaiting();
   void CloseExitWaiting();
signals:
   void sigMainWidgetShowNormal();
   void SigJoinActivetyEnd(bool, QString);
   void SigInteractiveLoopQuit();
public slots:
   void SlotAreaModify(int, int, int, int);
   void SlotResetTimeOut();
   void SlotToJoinActivity(QString id, QString nick, QString password, bool isPwdLogin = false);
   void Slot_HandleAppExit();
   void SlotCloseJoinActivityWnd();

private slots:
   void RecvMsg(QJsonObject);
   void SlotOnConnected();
   void SlotUnInitFinished();
   void SlotReqApointPageUserList(const int& iApoint);

   void slot_physicalSizeChanged(const QSizeF &);
   void slot_physicalDotsPerInchChanged(qreal);
   void slot_logicalDotsPerInchChanged(qreal);

private:
   void DealInitCommonInfo(void* apData, DWORD adwLen);
   int showAliveDlg();
   void CreateVhallAliveInteracton(bool isInteractiveLive = false);
   void createAliveDlg();
   void initCommonInfo(const STRU_VHALLRIGHTEXTRAWIDGET_COMMONINFO* commonInfo);
   void ReleaseCommonData();
   void createLiveMainDlg();
   bool Get(QString method, QString param, bool bExit);
   bool commitRecord();
   virtual int AskGenerateRecord(const char* stopId = NULL, bool isClose = false);
   void HandleStartTurnToLiveMainUI(bool hostJoin);
   int ShowScreenSelectDlg(vector<VHD_WindowInfo> &, QWidget* reference);
private:
   VhallLiveListViewDlg *m_pLiveListWdg = NULL;
   VhallLivePlugInUnitDlg *m_pLivePlugInUnitDlg = NULL;
   VhallLiveMainDlg* m_pLiveMainDlg = NULL;
   FullScreenLabel *m_pFullScreenLabel = NULL;
   CaptureSrcUI* m_pCaptureSrcUI = NULL;
   VHDesktopSharingUI *m_pDesktopSharingUI = NULL;
   VHAreaMark *m_pAreaMask = NULL;
   VHDesktopTip *m_pDesktopTip = NULL;
   VhallWaiting *m_pWaiting = NULL;
   VhallWaiting *m_pStartWaiting = NULL;
   VhallWaiting *m_pExitWaiting = NULL;

   VhallLoginWidget* mLoginDlg = NULL;
   long           m_lRefCount;

   int m_iVcodecFPS;
   VhallJoinActivity *m_pJoinActivity = NULL;
   BOOL m_bMuteMic;
   BOOL m_bMuteSpeaker;
   bool m_bLoadExtraRightWidget = true;
   wchar_t m_wzStreamId[16];
   bool m_bHasList = false;
   bool m_bShowList = true;

   bool m_bShowTeaching = false;
   HANDLE *m_pgNameCore = NULL;
   QTimer m_restartTimer;
   std::atomic<bool> m_sdkRet = false;
   QString m_sdkErrDesc;
   int m_sdkErrCode = 0;

   QString m_domain;
   bool m_showMainWidget = true;
   bool m_bIsHostStartLive = true;//主持开起
   STRU_VHALLRIGHTEXTRAWIDGET_COMMONINFO *m_pCommonInfo = NULL;	//腾讯互动加入房间成功之后能设置，所以临时存储信息变量。
   QString msConfPath;
   QString m_strLogReportUrl;
   IPluginWebServer* m_pPluginHttpServer = NULL;
   bool m_bStartPushStreamSuccess = false;
   static CPluginDataProcessor* m_pPluginDataProcessor;

   bool m_bEnableVoiceTranslate = false;  //语音转换功能是否可用
   bool m_bOpenVoiceTranslateSelect = false; //语音转换功能选则
   bool m_bEnableStartLive = true;   //判断是否可以开启活动。
   bool mbIsPwdLogin = false;
   QString mRoomID;
   QString mRoomPwd;
   QString mCurrentVersion;
   bool mIsInteractive;
   QString mStrPointUrl;//打点录制的url
   int mIDisplyCutRecord;	//打点录制显示与否
   std::map<DWORD, HttpTaskWorkThread*> mHttpTaskThreadList;
   QString mLastPlayFile;
   VhallIALive *mVhallIALive = NULL;
   VHAliveInteraction* mAliveInteraction = NULL;
   CCriticalSection mMsgEventListCS;
   QList<Event*> mMsgEventList;
   QString publish_args_accesstoken;
   QString publish_args_mixer;
   QString publish_args_token;
   QString publish_args_vhost;
   int mILiveType;
   QString m_msgToken;
   QString m_msgUrl;

   QString m_lastGetError;
   int m_lastGetCode;
   QString m_stopId;
   Key_Data mKeyData;
   QString hostNickName;
   QString hostJoinUid;
   VHStartParam mStartParam;
   QEventLoop mUploadLiveTypeLoop;
   bool mbIsUploadLiveSuc = true;

   QEventLoop mStopWebniarLoop;
   std::atomic_bool mbIsStopWebniarSuc = false;

   VHPaasSDKCallback callbackReciver;
   bool mbCameraListShow = false;
   VhallSharedQr* mpVhallSharedQr = nullptr;
   DesktopShareSelectWdg* mDesktopWdg = nullptr;

   ThreadToopTask* mThreadToopTask = nullptr;
   WId mRenderWidgetID;
   QString mSessionID;
   LibCefViewWdg *mCefObj = nullptr;
};

#endif //__MAINUI_LOGIC__H_INCLUDE__
