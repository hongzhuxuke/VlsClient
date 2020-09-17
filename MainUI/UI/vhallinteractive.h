#ifndef VHALLINTERACTIVE_H
#define VHALLINTERACTIVE_H

#include <QWidget>
#include <QTimer>
#include <QMenu>
#include <QAction>
#include <QDateTime>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QPropertyAnimation>

#include "vhallspeakerlist.h"
#include "vhallhorlistwidget.h"
#include "vhdesktopcameralist.h"
#include "IInterActivitySDK.h"
#include "vhallmenu.h"
#include "vhallrenderwidget.h"
#include "InviteJoinWidget.h"
#include "vhallstreamremux.h"
#include "ToolButton.h"
#include "start_button.h"
#include "AudioSliderUI.h"
#include "vhfadeouttipwidget.h"
#include "vhallwaiting.h"
#include "CFloatRenderWdg.h"
#include "CScreenShareToolWgd.h"
#include "CInterActiveCtrlWdg.h"
#include <atomic>
#include "pub.Struct.h"

#define MAX_HOST_OFFLINE_TIMEOUT 5000
#define MAINWINDGETWIDTH 1032
#define MAINWINDMINHEIGHT 604
#define HIDCTRLWDG_TIMEOUT 3000
#define MAX_CHECK_MIX_STREAM_SERVER_TIMEOUT 5000



#define STREAM_TAIL "_helper"

class VhallExtraWidgetUserInfo;
class TitleButton;
class CVideoRenderWdg;
class CRecordDlg;
class MainUILogic;
class AlertDlg;
class HttpNetWork;

class VhallInteractive : public QWidget {
   Q_OBJECT

public:
   explicit VhallInteractive(QWidget *parent = 0);
   ~VhallInteractive();
   void Init(int appid, int bizid);
   void Reset(bool m_bIsActiveDisconnect = false);
   void SetStreamId(bool master, QString streamId);
   void SetActiveTitle(const QString& name,const QString& streamID);
   void SetUserId(QString);

   void SetThirdUrl(QString url);
   void AddRightExWidget(QWidget *w);
   void ResetExternalWidget(QWidget* w);
   void RemoveRightExWidget(QWidget *parent);
   void AddOnlineList(QWidget *w);
   void RemoveOnlineList();
   void SetInterActivitySDK(IInterActivitySDK *m);
   void SetInterActivityUsers(IInterAcivtityUser *m);
   void Render(VHVideoFrame);
	int GetMediaPlayUIWidth();
   //获得腾讯云房间
   QString GetTencentStreamId(QString streamId, QString userId, QString type/*main | aux*/);

   //第三方拉流
   void ThirdPullStart(QString streamId, QString url);
   void ThirdPullStop(QString streamId);
   void OnPushStreamSuccess(const char *);
   void OnPushStreamFailed();
   void OnScreenExit(const char *);
   void OnCameraExit(const char *,bool bRemix = true);
   void OnScreenEnter(const char *);
   void OnCameraEnter(const char *);
   void OnPlayMediaEnter(const char*);
   void OnPlayMaideExit(const char*);
   void SetUserName(QString &);
   void SetUserImage(QString &);
   void Show();
   void Hide();
   void OnMemStatusChanged(int event_id, const char * identifier);
   void CreateRenderWidget(const char *id);
   void RemoveRenderWidget(const char *id);
   void EnableRenderWidgetCamera(const char *id, bool bEnable);
   void EnableRenderWidgetScreen(const char *id, bool bEnable);
   CVideoRenderWdg *GetVideoRender(VhallHorListWidgetKey);
   void ShowRightMouseClickedUserMenu(const QString& id, const QString& role, const int type, const bool centerWnd);
   void RefreshOnlineList();
   //设置中间渲染窗口类型
   void SetCenterRenderViewType(const int type);
   void ReOpenCamera();
   bool GetStreamStatus() { return m_streamStatus; }
   bool GetIsMaster();
   void ReOpenSpeaker();
   void SetShrink(bool);
   void MouseMovedExtraWidget();
   void CenterWindow(QWidget* parent);
   void CloseWaiting();
   void ShowWaiting();
   void OnDeviceCallBack(int oper, int retCode, void* data);
   void OnCloseDevice();
   void OnOpenDevice();
   void OnRoomDisconnect();
   void OnDeviceDetectCallback();
   void ShowLiveTool(bool);
   bool IsShareScreen();
   void SetPointRecordUrl(QString strUrl);
   //void SetHostType(const int iType); ///账户类型 0 标准版 1 专业版 2 旗舰版  ，标注版本用户不提供互动功能。
   void SetRecordState(const int iState);
   void SetMainUILogic(MainUILogic* pMainUILogic);
   int GetRecordState();
   QWidget* GetRenderWidget();
   void SetVedioPlayUi(QWidget* pVedioPlayUI);

   void DealSelectMediaFile();
signals:
   void SigSDKReturn();
   void SigMasterExitRoom();
   void SigMasterDisConnect();
   void Sig_GuestAgreeHostInvite();
   void SigCheckMixStreamServerErr();

protected:
   virtual bool nativeEvent(const QByteArray &eventType, void *message, long *result);
   virtual void closeEvent(QCloseEvent *e);
   virtual void keyPressEvent(QKeyEvent *e);
   virtual void showEvent(QShowEvent *);
   virtual void resizeEvent(QResizeEvent *);
   virtual void moveEvent(QMoveEvent *);
   virtual void mouseMoveEvent(QMouseEvent *event);
   virtual void mousePressEvent(QMouseEvent *event);
   virtual void mouseReleaseEvent(QMouseEvent *event);

public slots:
   /**
   * ================
   * 控制栏相关槽函数
   * ================
   **/
   void Slot_BtnStreamClicked();
   void Slot_BtnLeaveLiveClicked();

   void Slot_BtnScreenClicked();
   void Slot_ScreenShared(bool);

   void Slot_CameraItemClicked(bool, DeviceInfo);
   void Slot_ShowCameraList();
   void Slot_CameraBtnClicked();

   void Slot_SpeakerItemClicked(bool, DeviceInfo);
   void Slot_PlayerButtonClicked();
   void Slot_ShowSpeakerList();
   void Slot_PlayerButtonEnter();
   void Slot_PlayerButtonLeave();
   void Slot_PlayerVolumnChanged(int);

   void Slot_ShowMicList();
   void Slot_MicItemClicked(bool, DeviceInfo);
   void Slot_MicButtonClicked();
   void Slot_MicButtonEnter();
   void Slot_MicButtonLeave();
   void Slot_MicVolumnChanged(int);
   void Slot_MenuTriggered(QAction *);
   void Timeout();
   void Slot_MouseEnterCtrlWdg();
   void Slot_OnPlayFileClicked();
   void Slot_ContextCenterNoticeMenuEvent();
   void Slot_OnCheckMixStreamServer();
   void Slot_OnTimeExprise();
   void Slot_SetFullWnd();
   /**
   * ================
   * title相关槽函数
   * ================
   **/
   void Slot_btnCloseClicked();
   void Slot_btnFullScreenClicked();
   void Slot_btnMinClicked();
   void Slot_SharedLiveUrl();
   /**
   * ================
   * 渲染区域相关槽函数
   * ================
   **/
   void Slot_GuesetUserItemClicked(QString);
   void ThirdStreamFinished(QNetworkReply *);
   void Slot_WidgetClicked(QWidget *);
   void SlotRemixEnd(bool);
   void Slot_TimeExpired();
   void ReMix();
   void Slot_HostKickOutUser();
   void Slot_OnlineEvent(bool, QString, QString, QString,int type);//处理上下线消息
   void Slot_KikoutEvent(bool, QString, QString);
   void FadeOutTip(QString str);

private slots:
   void Slot_JoinActivity();
   void Slot_CancelJoinActivity();
   void Slot_HostInviteJoin();
   void Slot_SendMsgErr(QString);
   void Slot_GuestJoinActiveSuc(bool suc);
   void Slot_GuestLeavActiveSuc(bool suc);
   void Slot_ShowChatWdg();
   void Slot_ActiveMemberJoin(QString id, bool joined, QString headUrl);
   void Slot_HostStartPush(bool push);
   void Slot_HostNotifyGuestCloseCamera(bool bClose);
   void Slot_HostNotifyGuestCloseMic(bool bClose);
   void Slot_GuestRecvLockView(QString, bool);
   void Slot_HostExitRoom();
   void Slot_RecvCloseScreenNotify(QString);
   void Slot_ChangetoLiveGuest();
   void Slot_ChangetoWatcherGuest();
   void Slot_RecvCloseScreenResp();
   void Slot_UpdateMemberMediaState(QString id, bool bMicOpen, bool bCameraOpen, bool bShareOpen);
   void Slot_GuestSendJoinActiveMsgErr();
   void Slot_GuestAgreeHostInvite();
   void Slot_GuestRefuseHostInvite();
   void Slot_GuestRefuseHostResp(QString);
   void Slot_DealMic(bool);
   void Slot_DealCamera(bool);
   void Slot_DealInviteUser(bool);
   void Slot_DealLockView(bool);
   void Slot_HostNotifyAllActiveMember(QString activeMembers);
   void Slot_HostOfflineEvent();
   void Slot_GuestExitRoom(QString id, QString name);
   void Slot_OnRenderDrawImage(QString id);
   void OnVideoRenderFullScreen(CVideoRenderWdg* pRender);
   void OnExitVideoRenderFullScreen(CVideoRenderWdg* pRender, bool bIsLiveStop = false);
   void Slot_OnHideToolWidget();
   void Slot_OnSendExternalCapture();
   void slotCurRecordState(int curRecordState);
   void slotRecordStateChange(int iSate);
   void slotStopLive();
   void slotChatClicked();
   void Slot_OnCheckMixStreamServerFinished(QByteArray data, int);
   void Slot_OnCheckRenderStream();
   void ReCheckMixUser(QSet<QString>& cameraList, QSet<QString>& micList);
private:
   int GetCurrentStatus();
   void ReOpenMic();
   void ReCheckCameraBtnStatus();
   void ReCheckPlayerBtnStatus();
   void ReCheckMicBtnStatus();
   void ReCheckScreenBtnStatus();
   void OpenExternalCapture();
   bool IsExistScreenShare();
   bool IsEnableClicked();
   void ResetMixStream();
   void ResetAuthToWatcherUI();
   void SelectRender(QWidget *w);
   void ExitToActiveListWnd(int reason = 0);
   void SendUserJoinRoomState(int state);
   void ChangeCenterRendUser(QString showUser, QString hideUser);
   void SetTopWidgetShowState();
   void ResetCenterRenderMemberInfo(CVideoRenderWdg* selectRender);
   void MoveRenderFromTopWndToFloatWnd();
   void MoveRenderFromFloatWndToTopWnd();
   void ShowAllTopUserWnd();
   void ResetStateFromFullOrShare();
   void ResetToNormalSize();
   void ResetToFullScreenSize();
   bool GetCameraState(QString& id);
   void SetCameraState(QString& id, bool hasCamera);
   void RemoveUserCameraState(QString& id);
   void UserStopShareScreen(const QString &id);
	void commitRecord(const int eRequestType);
   bool IsPlayMediaFile();
   bool IsPlayUiShow();
   void CloseMicAndSetVolume();
   void RoomNoMediaFileStream(const char* identifier);
   void RoomHasMediaFileStream(const char* identifier);
   void ReleaseAlert();
   void ResetFloatWndState();

   void SetMixStreamUserID(const char* id, bool add = true);
private:
   Ui::VhallInteractive ui;
   QWidget *m_pExWidget = NULL;
   QWidget *m_pOnlineList = NULL;
   QWidget *m_centerRenderWidget = NULL;
   QWidget *m_selectedWidget = NULL;

   VhallSpeakerList *m_pSpeakerList = NULL;
   VhallHorListWidget *m_pHorWidget = NULL;
   
   VHDesktopCameraList *m_pCameraList = NULL;
   VHDesktopCameraList *m_pMicList = NULL;
   VHDesktopCameraList *m_pPlayerList = NULL;
   IInterActivitySDK *m_SDK = NULL;
   IInterAcivtityUser *m_pUserMananger = NULL;
   InviteJoinWidget* m_pInviteJoinWidget = NULL;

   QTimer m_startLiveTimer;
   QDateTime m_startLiveDateTime;
   
   QNetworkAccessManager m_ThirdPullManager;
   QNetworkReply *mThreadPullReply = NULL;
   VhallStreamRemix m_StreamMixObj;

   QString m_MixStreamUrl;
   QString m_StreamId;
   QString m_removeStreamID;
   QString m_ThirdPullUrl;
   QString m_PushDomain;
   QString m_UserId;
   QString m_userHeadUrl;
   QPoint pressPoint;
   QPoint startPoint;
   VhallExtraWidgetUserInfo *m_userInfo = NULL;
   TitleButton *m_pBtnClose = NULL;
   TitleButton *m_pBtnFullScreen = NULL;
   TitleButton *m_pBtnMin = NULL;
   TitleButton *m_pBtnShared = NULL;
   CVideoRenderWdg *m_pCenterVideoRender = NULL;

   CFloatRenderWdg* m_pFloatRenderWdg = NULL;  //全屏显示时视频浮动窗口。
   CScreenShareToolWgd* m_pScreenShareToolWgd = NULL;
   CInterActiveCtrlWdg* m_pInterActiveCtrlWdg = NULL;
   QPropertyAnimation *m_pAnimation = NULL;
   QPropertyAnimation *m_pVedioPlayUiAnmt = NULL;
   bool m_bIsHiddingCtrlWdg = false;
   bool mbMaster = false;	//是否是主持端
   bool mSDKRet;
   bool m_bPlayerEnable = true;
   VhallMenu *m_pUserMenu = NULL;
   QAction* m_pActionMic = NULL;
   QAction* m_pActionCamera = NULL;
   QAction* m_pActionInviteUser = NULL;
   QAction* m_pLockView = NULL;
   QString m_selectUserID;
   QTimer *m_pHideToolWidgetTimer = NULL;

   QString m_desktopStreamId;
   QMap<QString,bool> m_CameraListMap;   
   AudioSliderUI *m_pMicVolumn = NULL;
   AudioSliderUI *mPlayerVolumn = NULL;

   QPixmap mPhotoPixmap;
   int mRenderType;

   QString m_LastSelectMicID;
   QString m_LastSelectCameraID;
   QString m_LastSelectSpeakerID;

   bool m_bOpenMicState = true;   //上一次打开设备状态
   bool m_bOpenCameraState = true;

   QTimer *m_pHostExitTimer = NULL;
   VhallWaiting *m_pExitWaiting = NULL;
   VhallWaiting *m_pStartWaiting = NULL;

   VHFadeOutTipWidget *m_pFadeOutTipWidget = NULL;
   bool m_streamStatus = false;
   bool m_bStreamPushState = false;
   bool m_bIsUserSharing = false; //是否有人屏幕共享。
   bool m_bScreenStatus = false;
   QString m_SharingUser;

   QWidget *m_pExbarWidget = NULL;
   bool m_bTitlePressed = false;
   bool m_bIsFullScreen = false;
   int m_nMixStreamStatus = 0;
   int m_nLastMixStreamStatus = 0;
   int m_appid = 1400025871;
   int m_bizid = 8136;

   QTimer *m_pExternalCaptureTimer = NULL;//上传数据帧定时器
   QMap<QString, bool> m_userCameraStateMap;
   ToolButton *m_toolChatBtn = NULL;

   CRecordDlg* mPRecordDlg = NULL;
   QString mStrPointRecordUrl;	//打点录制url
   MainUILogic* mpMainUILogic;
   QWidget* m_pExParentWdt;
   QWidget* mpVedioPlayUi = NULL;
   bool mbIsUserPlayMediaFile = false;
   QString mstrPlayMediaUserID;
   AlertDlg* mAlertDlg = NULL;
   unsigned int mLastPlayMediaFileEndTime = 0;
   QTimer mCheckMixStreamServerTimer;
   QTimer mTimeExpriseTimer; 
   HttpNetWork* mHttpNetWork = NULL;
   QTimer mCheckRenderStream;
};

#endif // VHALLINTERACTIVE_H
