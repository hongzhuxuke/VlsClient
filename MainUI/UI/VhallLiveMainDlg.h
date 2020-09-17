#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QDialog>
#include <windows.h>
#include <QPushButton>
#include <QTimer>
#include <QMoveEvent>
#include <QCloseEvent>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include "DeviceTestingWdg.h"
#include <atomic>
#include "FullDocWdg.h"
#include "cbasedlg.h"
#include "cvoicetranslatewdg.h"
#include "vhallexitwidget.h"
#include "IInteractionClient.h"
#include "ui_VhallLiveMainDlg.h"
#include "LibCefViewWdg.h"

#define LIVEMAINDLGWIDTH 900
#define CHATUIWIDTH  321

class StreamStatus;
class TitleBarWidget;
class RenderWnd;
class ToolBarWidget;
class MainUILogic;
class VHFadeOutTipWidget;
class CRecordDlg;
class ToolButton;
class CMemberListDlg;
class VhallExtraWidgetUserInfo;
class CameraTeachTips;
class LiveContentCtrl;
class PerformanceStatistics;
class LiveToolDlg;
class VhallLivePlugInUnitDlg;
class VhallSharedQr;
class ShowTipsWdg;
class VHWebViewInterface;


enum CenterPageIndex {
   CenterPageIndex_Render = 0,
   CenterPageIndex_Doc = 1,
};

enum SmallWndType {
   SmallWndType_None = -1,
   SmallWndType_Render = 0,
   SmallWndType_Doc = 1,
};

class VhallLiveMainDlg : public CAliveDlg,public MsgCallbackDelegate {
   Q_OBJECT
public:
   VhallLiveMainDlg(QWidget *parent = 0);
   ~VhallLiveMainDlg();

   void InitUI();
   void DestroyUI();

   virtual void OnRecvMsg(std::string fun_name, std::string msg);
   virtual void OnWebViewDestoryed();
   virtual void OnWebViewLoadEnd();
   virtual void OnTitleChanged(const int id, std::string title_name);
   virtual QString GetMainViewID() { return QString(); }
   virtual QString GetMainSpeakerID() { return QString(); }
   virtual QString GetMainViewRole() { return QString(); }
   void GetRenderPos(int& x, int& y);
   void SetStreamId(const QString &streamId);
   void StartLiveWithTime(int liveTime);
   //void StaticFun(int cpu, int sendNetData, int recvNetData);
   QPoint GetTipPoint(/*int& iWidth*/);

   virtual void HandleJoinMember(VhallAudienceUserInfo*);
   virtual void HandleLeaveMember(VhallAudienceUserInfo*);
   long long GetShowUserMember();
   long GetStartTime();
   void HandleSendNotice(QEvent* event);
   void CloseFloatChatWdg();
   void HideCaptureSrcUi();
   void setMemberAuthority(const bool& bAuth) {}
   bool GetMemberAuthority() { return true; }
   void SetTeachingPage(bool showTeaching);
   bool IsInteractiveHandsUp() { return false; }
   virtual void HandsUpTimeout(QString user_id){}
   void ClickedDocBtn();
   void StopCPUState();

   void HandleDownLoadHead(QEvent* event);
public:
   void ShowInteractiveTools(const int& x, const int& y);
   void AddVoiceTranslate();
   void RemoveVoiceTranslate();
   bool IsFirstLoadTool();
   void HandleCustomEvent(QEvent* event);
   //void setUserInfo(const QString& userData);
   virtual bool  GetIsLiving();
   virtual QString GetJoinId();
   virtual QString GetJoinRole();
   virtual QString GetHostId();
   virtual QString GetStreamName();
   virtual bool IsExistRenderWnd(const QString& uid);

   //void layoutChatWebView();
   void InitChatWebView(const QString& strUrl);

   bool Create(MainUILogic*);
   void Destroy();
   void* GetRenderWidget();
   void* GetRenderPos();  
   void SetRenderWidgetUpdate(bool enable_updata);
   int GetMediaPlayUIWidth();
   //void UpdateStreamSpeedUI(QString qStrStreamSpeed, int stageValue);
   void UpdateStreamStateUI(QString qStrStreamState, int iStreamCnt);
   void StartTimeClock(bool bStart);
   void ClearTimeClock();
   //	int GetPlayTime();
   void SetStreamButtonStatus(bool);
   void showMinized();
   void showWidget();
   void SetStreamStatus(StreamStatus*);
   float GetPushStreamLostRate();
   //显示
   void ShowSuspend(int msec);
   void SetTipForceHide(bool);
   void CenterWindow(QWidget* parent);

   //麦克风静音
   void SetMicMute(bool bMute);
   //扬声器静音
   void SetSpeakerMute(bool bMute);
   //窗口是否显示
   bool IsDesktopShow();
   //窗口是否显示
   //bool IsMultiMediaShow();
   //录制窗口是否显示
   bool IsRecordShow();
   void SetDesktopSharing(bool bShow);
   void FadeOutTip(QString, int type, int showTime = 3000);
   void HandleDevCheck();
   void ActiveExtraWidget();
   void ReActive();
   void RemoveImageWidget(QWidget *);
   void SetTipsClose(bool bClose);
   void EnableShow();
   void Hide();

   void HideLogo(bool);
   void BeHide();

   void ReInit();
   void SetStreamStatus(bool bStart);
   void SetWebinarName(QString name, QString id);
   void ClearWebinarName();
   void ClearMemberList();
   void ShowVoiceTranslateWdg(bool show);

   void SetVoiceTransitionText(QString text);
   void SetVoiceTransitionFontSize(int size);
   void SetRecordState(const int& eState);
   void ShowRecordDlg();
   int GetRecordState();
   bool IsRecordBtnhide();
   void ToExitAppWithAnimation();
   void SetStartLiveState(bool start);
   void HandleRecvSocketIOMsgEvent(Event& event);
   void HandleChatMsg(QEvent* customEvent);
   //bool IsLoadWebEngienViewFinished();
   void CreateDocWebView();
   void ReleaseDocWebView();
   void ReleaseChatWebView();
   void CallDocWebViewtartLive();
   void LoadDocUrl(const QString& url);
   int GetMixAudioVolume();

public slots:
   void slotMaxClicke();
   void closeSuspendTip();
   void DoSuspendClose();
   void ShowNormal();
   void TitleBarPressed();
   void slot_OnShared();
   void slot_OnClickedDoc();
   void slot_ClickedWhiteboard();
   void slot_OnClickedPluginUrl(const int& iX, const int& iy);
   void slot_showChangeViewCtrl(bool);
   void ToExit();
   void slot_CloseAndExitApp();
   //void Slot_ShowChatWdg();
   //void slot_renderProcessTerminated(QWebEnginePage::RenderProcessTerminationStatus terminationStatus, int exitCode);
   void slotMemberClicked();
   void Slot_Copy();
   void slot_OnMicVolumeShow();
   void hide();
   void slot_RunningState(int, int, int);
   void slot_PressKeyEsc();         
   void slot_changeProfile(int);
   void slot_ClickedMin();
   void slot_mouseMoved();
protected:
   virtual void mouseMoveEvent(QMouseEvent *);
   virtual bool nativeEvent(const QByteArray &eventType, void *message, long *result);
   virtual void changeEvent(QEvent *);
   virtual void moveEvent(QMoveEvent *);
   virtual void leaveEvent(QEvent *);
   virtual void showEvent(QShowEvent *e);
   virtual void hideEvent(QHideEvent *event);
   virtual void closeEvent(QCloseEvent *e);
   virtual bool eventFilter(QObject *, QEvent *);
   virtual void keyPressEvent(QKeyEvent *event);
   virtual void customEvent(QEvent *event);
   virtual void resizeEvent(QResizeEvent *event);
   //处理外部消息
   void ProcessExternalMsg(unsigned int uiMsg, void* param);
   //处理设备改变消息
   void ProcessDeviceChgMsg(WPARAM wparam, LPARAM lparam);


signals:
   void sigMove();
   void exitClicked(bool);
   void sig_showToTop();
   void sig_ExitApp();
   void signal_MemberListRefresh();
   void sig_OperationUser(RQData& RqData);
   void sig_ReqApointPageUserList(const int& iApoint);
   void sig_RunningState(int, int, int);
private:
   void ReSetTipsPos();
   void ReposVedioPlayUI(bool bShow);
   void ShowSuspendTip(bool bShow);
   void ReposVoiceTranslateWdg(bool bShow);
   bool IsShareScreen();
   void createMemberListDlg();
   void HandleRecvAnnouncement(Event &e);
   void ShowCheckDev();

public slots:
   void slotMemberListClicked();
   void JSCallQtUserOnline(QString param);
   void JSCallQtUserOffline(QString param);
   void JsCallQtOpenUrl(QString url);
   void slot_OnFresh();
   void slotChatClicked();
   void slot_DocLoading();
   void slot_OnChangeView();
   void slot_HidePlayUI();
   void JsCallQtMsg(QString param);
private slots:

   void slotLiveToolChoise(const int& iOpType);
   void slot_changed(bool bShowMem);

   //void slot_closeTab(int);                    //关闭tab的槽函数
   void loadFinished(bool);
   void slotMemberClose();
   void slot_LoadUrlCloseTool();
   void slotClickedCamera();
   void slot_removeTipsItem();
   void slot_LeaveRenderEvent();
private:
   void MoveCameraTips();
   void ShowPlayUIState(bool show);
private:
   Ui::VhallLiveMainDlg ui;
   QString mDocUrl;
   RenderWnd* m_pContentWidget = nullptr;

   //直播暂停提示
   QTimer *m_suspendTipTimer = nullptr;
   int     m_suspendCloseCount;
   QTimer *m_suspendTipCloseTimer = nullptr;
   QPushButton *m_suspendTipButton = nullptr;

   bool m_bShowSuspendTipForceHide = false;
   bool m_bShowSuspendTip = false;
   bool m_bForceHide = false;
   bool m_bTitleBarPressed = false;
   bool m_bTipsClose = false;
   bool m_bShow = false;
   bool m_bShowTeaching = true;
   bool m_bIsStreamStart = false;

   MainUILogic* m_pLogic = nullptr;
   QGridLayout *mpGridLayout = nullptr;
   bool mbIsLoadFinished = false;

   CVoiceTranslateWdg *m_pVoiceTranslateWdg = nullptr;
   CRecordDlg* mPRecordDlg = nullptr;

   CMemberListDlg* mpMemberListDlg = nullptr;
   QString mStreamId;

   LiveContentCtrl *mLiveContentCtrlPtr = nullptr;
   int mSmallWdgType = -1;

   PerformanceStatistics* mPerformanceStatisticsPtr = nullptr;

   QTimer* mpMicVolumeTimerPtr = nullptr;
   LiveToolDlg* mLiveToolDlg = nullptr;
   VhallSharedQr* mpVhallSharedQr = nullptr;

   bool mbIsMaxSize = false;
   QRect mNormalRect;
   QTimer mDocLoadTimer;
   bool mbIsFirstLoad = true;
   bool mbShowTips = true;
   CameraTeachTips* mCameraTeachTips = nullptr;
   QTimer* mPlayUIHideTimer = nullptr;
   QTimer* mDocLoadingTimer = nullptr;
   int mDocLoadingPointCount = 0;
   ShowTipsWdg *mShowTipsWdgPtr = nullptr;
   int mNetDevDataCount = 0;
   QWidget *mTeachingWidget = nullptr;
   int mTeachingPageIndex = 0;
   FullDocWdg* mFullDocWdg = nullptr;
   bool mLastIsMaxSize;
   bool mbIsDestory = false;
   int mCPUValue = 0;
   LibCefViewWdg* mDocCefWidget = nullptr;
   int mLoadCount = 0;
   int mMicVolume = 0;
   DeviceTestingWdg *mDeviceTestingWdg = nullptr;
};

#endif // MAINWIDGET_H
