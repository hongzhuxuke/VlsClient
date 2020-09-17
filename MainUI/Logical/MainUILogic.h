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

   /////////////////////////////IUnknown�ӿ�/////////////////////////////////////////
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
   //�����Ƿ���ʾ
   bool IsDesktopShow();
   //�û��Ƿ��ܹ�ʹ������ת������
   virtual void STDMETHODCALLTYPE EnableVoiceTranslate(bool enable = false);
   virtual bool STDMETHODCALLTYPE GetVoiceTranslate();
   //����ת�����ܰ����Ƿ���úͿ���
   virtual void STDMETHODCALLTYPE OpenVoiceTranslateFun(bool open = false);
   virtual bool STDMETHODCALLTYPE IsOpenVoiceTranslateFunc();
   virtual void STDMETHODCALLTYPE SetVoiceTranslateFontSize(int size);
   virtual bool STDMETHODCALLTYPE IsShareDesktop();
   virtual void STDMETHODCALLTYPE SetRecordState(const int iState);
   virtual void* GetCAliveDlg();
   /**
   @brief ���ļ����š�
   @details ��ʼ���ű�����Ƶ\��Ƶ�ļ��������ļ�ǰ������ȵ���isValidMediaFile()����ļ��Ŀ����ԡ�
   @param [in] szMediaFile �ļ�·��(�����Ǳ����ļ�·����Ҳ������һ�������ļ���url);
   @remark
   1��֧�ֵ��ļ�����:<br/>
   *.aac,*.ac3,*.amr,*.ape,*.mp3,*.flac,*.midi,*.wav,*.wma,*.ogg,*.amv,
   *.mkv,*.mod,*.mts,*.ogm,*.f4v,*.flv,*.hlv,*.asf,*.avi,*.wm,*.wmp,*.wmv,
   *.ram,*.rm,*.rmvb,*.rpm,*.rt,*.smi,*.dat,*.m1v,*.m2p,*.m2t,*.m2ts,*.m2v,
   *.mp2v, *.tp,*.tpr,*.ts,*.m4b,*.m4p,*.m4v,*.mp4,*.mpeg4,*.3g2,*.3gp,*.3gp2,
   *.3gpp,*.mov,*.pva,*.dat,*.m1v,*.m2p,*.m2t,*.m2ts,*.m2v,*.mp2v,*.pss,*.pva,
   *.ifo,*.vob,*.divx,*.evo,*.ivm,*.mkv,*.mod,*.mts,*.ogm,*.scm,*.tod,*.vp6,*.webm,*.xlmv��<br/>
   2��Ŀǰsdk��Դ���640*480����Ƶ�ü���640*480;<br/>
   3���ļ����ź�ϵͳ�����ɼ���Ӧ��ͬʱ�򿪣������ļ����ŵ������ֻᱻϵͳ�����ɼ�����������������;
   @note
   ��Ļ����Ͳ�Ƭ���ܶ���ͨ����·�����䣬������Ļ����Ͳ�Ƭ����ʹ��;
   ��·�����Լ�ռ�ã��豸�����ص��У����ش�����AV_ERR_EXCLUSIVE_OPERATION(�������github�ϵĴ������);
   ��������������Առ�ã��豸�����ص��У����ش�����AV_ERR_RESOURCE_IS_OCCUPIED(�������github�ϵĴ������);
   */
   virtual int OpenPlayMediaFile(const char*  szMediaFile, int liveType);

   virtual int SetPlayMediaFileVolume(int vol, int liveType);
   /**
   @brief �ر��ļ����š�
   */
   virtual void ClosePlayMediaFile(int liveType);
   /**
   @brief ��ͷ�����ļ���
   @return ���������NO_ERR��ʾ�޴���
   @note ֻ���ڴ��ڲ���״̬��(E_PlayMediaFilePlaying)���˽ӿڲ���Ч�����򷵻�ERR_WRONG_STATE;
   */
   virtual int RestartMediaFile(int liveType);
   /**
   @brief ��ͣ�����ļ���
   @return ���������NO_ERR��ʾ�޴���
   */
   virtual int PausePlayMediaFile(int liveType);
   /**
   @brief �ָ������ļ���
   @return ���������NO_ERR��ʾ�޴���
   */
   virtual int	ResumePlayMediaFile(int liveType);
   /**
   @brief ���ò����ļ����ȡ�
   @param [in] n64Pos ����λ��(��λ: ��)
   @return ���������NO_ERR��ʾ�޴���
   */
   virtual int SetPlayMediaFilePos(const signed long long& n64Pos, int liveType);
   /**
   @brief ��ȡ�����ļ����ȡ�
   @param [out] n64Pos ��ǰ����λ��(��λ: ��)
   @param [out] n64MaxPos ��ǰ�������ļ����ܳ���(��λ: ��)
   @return ���������NO_ERR��ʾ�޴���
   */
   virtual int GetPlayMediaFilePos(signed long long& n64Pos, signed long long& n64MaxPos, int liveType);
   virtual int GetPlayFileState(int liveType);

   virtual void OnRecvPushStreamErrorEvent();
   void SendMsg(int eMsgType, RQData& vData);

   virtual void RunTask(int type,void *appdata = nullptr);
public:
   int CreateRecord(bool isClose = false);
   // ������Ϣ
   void DealMessage(DWORD adwMessageID, void* apData, DWORD adwLen);

   // ������ʾWidget
   void DealShowWidget(void* apData, DWORD adwLen);

   void DealPrivateStart(void* apData, DWORD adwLen);

   // ����Click����
   void DealClickControl(void* apData, DWORD adwLen);

   //�����Ҳ���������Ϣ
   void DealExtraWidgetCreate(void* apData, DWORD adwLen);

   //�����Ҳ�����Ծ��Ϣ
   void DealExtraWidgetActive(void* apData, DWORD adwLen);

   //������������������
   void DealProcessSrc(void* apData, DWORD adwLen);


   //����ر�������
   void DealDoCloseMainWindow(void* apData, DWORD adwLen);

   //������
   void DealShareRegion(void* apData, DWORD adwLen);

   //����ͳ��
   void DealPushStreamStatus(void* apData, DWORD adwLen);

   //����������
   void DealHideShareRegion(void* apData, DWORD adwLen);

   //�ײ�����Ϣ֪ͨ
   void DealStreamNotify(void* apData, DWORD adwLen);

   void DealPushStreamBitRateDown();

   //�����ʼ��
   void DealInitCommonData(void* apData, DWORD adwLen);

   //����ʵʱ��Ļ
   void DealVoiceTransition(void *apData, DWORD adwLen);

   //������־��Ϣ
   void DealLog(void *apData, DWORD adwLen);
   //����ˢ�������Ϣ
   void DealEndFefresh(void *apData, DWORD adwLen);

   void DealStreamPushSuccess();
   void DealRecordChange(void* apData, DWORD adwLen);
   //������¼�������Ӧ
   void DeaPointRecordRs(void* apData, DWORD adwLen);
   void DealHttpTask(void* apData, DWORD adwLen);
   void HandleCloseVhallActive(bool exitToLiveList, int exitReason);
   //����logo�Ƿ�����
   void DoHideLogo(bool);

   //ѯ�ʲ��ҵ�����ҳ
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
   // ��ʾ��б�
   void ShowActivityList(bool bShow = true);
   void HandleJoinAcivity(bool bPwdLogin);
   // ��ʾ������
   void ShowMainWidget();
   void StartVoiceTranslate(bool open);
   void CheckPrivateStartExitApp();
   // ��С��Ӧ��
   void MinimizeApp(void *parent);

   // �ر�Ӧ��
   void CloseApp(void *parent, bool bIsActiveDisconnect = false, int reason = 0);

   // ����Դ����
   void ShareSrcUI(int iX, int iY);

   //������
   void ShareRegion();

   //���湲��
   void ShareDesktop();
   void StartToCaptureDesktop(VHD_WindowInfo windowInfo);
   //��ý��
   //void ShowMultiMediaUI();
   //¼��
   void ShowRecordDlg();
   //����   dwPublishStateҪ����������״̬ 
   void PublishStream(DWORD dwPublishState, bool mediaCoreStreamErr, bool isAlreadyLive = false);

   //����ϱ��������
   void MonitorReportPublish(BOOL bStartPublish);

   //��ȡ������Ϣ
   void InitConfig();

   //�ϱ��û��¼�
   void UpLoadEvents();

   //�յ�websocket��Ϣ
   void RecvWebsocketMsg(char *);

   //�յ�socketIO��Ϣ
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
   bool m_bIsHostStartLive = true;//���ֿ���
   STRU_VHALLRIGHTEXTRAWIDGET_COMMONINFO *m_pCommonInfo = NULL;	//��Ѷ�������뷿��ɹ�֮�������ã�������ʱ�洢��Ϣ������
   QString msConfPath;
   QString m_strLogReportUrl;
   IPluginWebServer* m_pPluginHttpServer = NULL;
   bool m_bStartPushStreamSuccess = false;
   static CPluginDataProcessor* m_pPluginDataProcessor;

   bool m_bEnableVoiceTranslate = false;  //����ת�������Ƿ����
   bool m_bOpenVoiceTranslateSelect = false; //����ת������ѡ��
   bool m_bEnableStartLive = true;   //�ж��Ƿ���Կ������
   bool mbIsPwdLogin = false;
   QString mRoomID;
   QString mRoomPwd;
   QString mCurrentVersion;
   bool mIsInteractive;
   QString mStrPointUrl;//���¼�Ƶ�url
   int mIDisplyCutRecord;	//���¼����ʾ���
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
