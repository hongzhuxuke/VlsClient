#include <QCloseEvent>
#include <QDebug>
#include <QJsonDocument>
#include <QDateTime>
#include <QJsonObject>
#include <QHBoxLayout>
#include <QApplication>
#include <QDesktopWidget>


#include "httpnetwork.h"
#include "VideoRenderWdg.h"
#include "vhallinteractive.h"
#include "windows.h"
#include "MainUIIns.h"
#include "Msg_MainUI.h"
#include "Msg_OBSControl.h"
#include "VHDialog.h"
#include "vhallextrawidgetuserinfo.h"
#include "Msg_VhallRightExtraWidget.h"
#include "IVhallRightExtraWidgetLogic.h"
#include "CRPluginDef.h"
#include "DebugTrace.h"
#include "ConfigSetting.h"
#include "vhallsharedqr.h"
#include "title_button.h"
#include "IInteractionClient.h"
#include "pathmanager.h"
#include "RecordDlg.h"
#include "PublishInfo.h"
#include "ICommonData.h"
#include "Msg_CommonToolKit.h"
#include "AlertDlg.h"
#include "vhallstreamremux.h"
#include "priConst.h"
#include "../../InterActivitySDKNew/include/iLive.h"

VhallInteractive::VhallInteractive(QWidget *parent) :
QWidget(parent),
m_pExParentWdt(NULL) {
   ui.setupUi(this);
   winId();

   //聊天公告显示隐藏控制按钮
   m_toolChatBtn = new ToolButton(":/extern/09.png", ui.widgetRender);
   connect(m_toolChatBtn, SIGNAL(sigClicked()), this, SLOT(Slot_ShowChatWdg()));
   connect(&m_StreamMixObj, SIGNAL(SigFadout(QString)), this, SLOT(FadeOutTip(QString)));
   setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowSystemMenuHint | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint);
   this->setUpdatesEnabled(true);
   //视频显示区域左上角 logo
   QString toolConfPath = CPathManager::GetAppDataPath() + QString::fromStdWString(VHALL_TOOL_CONFIG);
   int hideLogo = ConfigSetting::ReadInt(toolConfPath, GROUP_DEFAULT, KEY_VHALL_LOGO_HIDE, 0);
   if (hideLogo == 0) {
      QPixmap qImageTitleLogo(":/sysButton/titleLogo");
      ui.label_logo->setPixmap(qImageTitleLogo);
      ui.label_logo->setMinimumSize(qImageTitleLogo.size());
      ui.label_logo->setMaximumSize(qImageTitleLogo.size());
   } else {
      ui.label_logo->hide();
   }
   //互动界面 左上角  麦上用户列表
   m_pSpeakerList = new VhallSpeakerList(this);
   if (m_pSpeakerList) {
      ui.layout_left_top->addWidget(m_pSpeakerList);
      connect(m_pSpeakerList, SIGNAL(Sig_JoinActivity()), this, SLOT(Slot_JoinActivity()));
      connect(m_pSpeakerList, SIGNAL(Sig_CancelJoinActivity()), this, SLOT(Slot_CancelJoinActivity()));
      connect(m_pSpeakerList, SIGNAL(Sig_ItemClicked(QString)), this, SLOT(Slot_GuesetUserItemClicked(QString)));
   }

   //嘉宾上麦 后的 视频列表
   m_pHorWidget = new VhallHorListWidget(this); {
      ui.layout_center_center->addWidget(m_pHorWidget);
      connect(m_pHorWidget, SIGNAL(SigClicked(QWidget *)), this, SLOT(Slot_WidgetClicked(QWidget *)));
   }
   int bg_head = 10;
   int bg_body = 30;
   int bg_tail = 10;
   QPixmap bgPixmap = QPixmap(":/interactivity/db");

   //摄像设备列表对话框
   m_pCameraList = new VHDesktopCameraList(this);
   if (m_pCameraList) {
      m_pCameraList->SetBodyShow(bg_head, bg_body, bg_tail);
      m_pCameraList->SetBodySide(20, 20);
      bgPixmap = bgPixmap.scaled(bgPixmap.width(), bg_head + bg_body + bg_tail);
      m_pCameraList->SetPixmap(bgPixmap);
      connect(m_pCameraList, SIGNAL(SigItemClicked(bool, DeviceInfo)), this, SLOT(Slot_CameraItemClicked(bool, DeviceInfo)));
      m_pCameraList->SetMaxCount(1);
      m_pCameraList->SetTipsLabelText(QString::fromWCharArray(L"未检测到视频设备"));
   }

   //麦克风设备列表对话框
   m_pMicList = new VHDesktopCameraList(this);
   if (m_pMicList) {
      m_pMicList->SetBodyShow(bg_head, bg_body, bg_tail);
      m_pMicList->SetPixmap(bgPixmap);
      m_pMicList->SetBodySide(20, 20);
      connect(m_pMicList, SIGNAL(SigItemClicked(bool, DeviceInfo)), this, SLOT(Slot_MicItemClicked(bool, DeviceInfo)));
      m_pMicList->SetMaxCount(1);
      m_pMicList->SetTipsLabelText(QString::fromWCharArray(L"未检测到音频设备"));
   }

   //扬声器列表对话框
   m_pPlayerList = new VHDesktopCameraList(this);
   if (m_pPlayerList) {
      m_pPlayerList->SetBodyShow(bg_head, bg_body, bg_tail);
      m_pPlayerList->SetPixmap(bgPixmap);
      m_pPlayerList->SetBodySide(20, 20);
      connect(m_pPlayerList, SIGNAL(SigItemClicked(bool, DeviceInfo)), this, SLOT(Slot_SpeakerItemClicked(bool, DeviceInfo)));
      m_pPlayerList->SetMaxCount(1);
      m_pPlayerList->SetTipsLabelText(QString::fromWCharArray(L"未检测到音频设备"));
   }

   //定时更新推流时长
   connect(&m_startLiveTimer, SIGNAL(timeout()), this, SLOT(Timeout()));
   m_startLiveDateTime.setTime_t(0);
   connect(&m_ThirdPullManager, SIGNAL(finished(QNetworkReply *)), this, SLOT(ThirdStreamFinished(QNetworkReply *)));
   m_PushDomain = ConfigSetting::ReadString(CPathManager::GetConfigPath(), GROUP_DEFAULT, KEY_URL_DOMAIN, "");
   //主持人邀请你上麦提示对话框
   m_pInviteJoinWidget = new InviteJoinWidget(this);
   if (m_pInviteJoinWidget) {
      m_pInviteJoinWidget->hide();
      connect(m_pInviteJoinWidget, SIGNAL(Sig_GuestAgreeHostInvite()), this, SLOT(Slot_GuestAgreeHostInvite()));
      connect(m_pInviteJoinWidget, SIGNAL(Sig_GuestRefuseHostInvite()), this, SLOT(Slot_GuestRefuseHostInvite()));
   }

   //当前登陆用户信息
   m_userInfo = new VhallExtraWidgetUserInfo(this);
   if (m_userInfo) {
      ui.layout_userInfo->addWidget(m_userInfo);
   }
   if (hideLogo == 0) {
      //分享按钮
      m_pBtnShared = new TitleButton(this);
      if (m_pBtnShared) {
         m_pBtnShared->loadPixmap(":/sysButton/shared_button");
         m_pBtnShared->setToolTip(QString::fromWCharArray(L"分享"));
         ui.layout_title_btn->addWidget(m_pBtnShared);
         connect(m_pBtnShared, SIGNAL(clicked()), this, SLOT(Slot_SharedLiveUrl()));
      }
   }

   //最小化按钮
   m_pBtnMin = new TitleButton(this);
   if (m_pBtnMin) {
      m_pBtnMin->loadPixmap(":/sysButton/min_button");
      m_pBtnMin->setToolTip(QString::fromWCharArray(L"最小化"));
      ui.layout_title_btn->addWidget(m_pBtnMin);
      connect(m_pBtnMin, SIGNAL(clicked()), this, SLOT(Slot_btnMinClicked()));
   }
   //全屏
   m_pBtnFullScreen = new TitleButton(this);
   if (m_pBtnFullScreen) {
      m_pBtnFullScreen->loadPixmap(":/sysButton/img/sysButton/fullbutton.png");
      m_pBtnFullScreen->setToolTip(QString::fromWCharArray(L"全屏"));
      ui.layout_title_btn->addWidget(m_pBtnFullScreen);
      connect(m_pBtnFullScreen, SIGNAL(clicked()), this, SLOT(Slot_btnFullScreenClicked()));
   }
   //关闭按钮
   m_pBtnClose = new TitleButton(this);
   if (m_pBtnClose) {
      m_pBtnClose->loadPixmap(":/sysButton/close_button");
      m_pBtnClose->setToolTip(QString::fromWCharArray(L"关闭窗口"));
      ui.layout_title_btn->addWidget(m_pBtnClose);
      connect(m_pBtnClose, SIGNAL(clicked()), this, SLOT(Slot_btnCloseClicked()));
   }
   //本地视频显示区
   m_pCenterVideoRender = new CVideoRenderWdg(this, WND_TYPE_CENTER);
   if (m_pCenterVideoRender) {
      connect(m_pCenterVideoRender, SIGNAL(applyFullScreen(CVideoRenderWdg*)), this, SLOT(OnVideoRenderFullScreen(CVideoRenderWdg*)));
      connect(m_pCenterVideoRender, SIGNAL(exitFullScreen(CVideoRenderWdg*)), this, SLOT(OnExitVideoRenderFullScreen(CVideoRenderWdg*)));
      connect(m_pCenterVideoRender, SIGNAL(Sig_MouseMoveInCenterRender()), this, SLOT(Slot_MouseEnterCtrlWdg()));
      m_pCenterVideoRender->EnableAll(true);
      m_centerRenderWidget = m_pCenterVideoRender;
   }
   m_pCenterVideoRender->hide();
   ui.layout_render->addWidget(m_centerRenderWidget);
   m_pUserMenu = new VhallMenu();
   if (m_pUserMenu) {
      connect(m_pUserMenu, SIGNAL(triggered(QAction *)), this, SLOT(Slot_MenuTriggered(QAction *)));
   }
   //混流管理对象
   connect(&m_StreamMixObj, SIGNAL(SigRemixEnd(bool)), this, SLOT(SlotRemixEnd(bool)));
   connect(&m_StreamMixObj, SIGNAL(SigTimeExpired()), this, SLOT(Slot_TimeExpired()));
   //麦克风音量控制
   m_pMicVolumn = new AudioSliderUI((QDialog *)this);
   m_pMicVolumn->Create();
   m_pMicVolumn->HideMuteBtn();
   m_pMicVolumn->HideSettingBtn();
   connect(m_pMicVolumn, SIGNAL(sigVolumnChanged(int)), this, SLOT(Slot_MicVolumnChanged(int)));
   //扬声器音量控制
   mPlayerVolumn = new AudioSliderUI((QDialog *)this);
   mPlayerVolumn->Create();
   mPlayerVolumn->HideMuteBtn();
   mPlayerVolumn->HideSettingBtn();
   connect(mPlayerVolumn, SIGNAL(sigVolumnChanged(int)), this, SLOT(Slot_PlayerVolumnChanged(int)));
   //主持离线事件
   m_pHostExitTimer = new QTimer(this);
   if (m_pHostExitTimer) {
      connect(m_pHostExitTimer, SIGNAL(timeout()), this, SLOT(Slot_HostOfflineEvent()));
   }
   mPhotoPixmap = QPixmap(150, 150);
   mPhotoPixmap.fill(Qt::red);
   SetCenterRenderViewType(RenderType_Video);
   m_pFadeOutTipWidget = new VHFadeOutTipWidget(this);
   m_pExitWaiting = new VhallWaiting(this);
   if (m_pExitWaiting) {
      m_pExitWaiting->SetCerclePos(55, 50);
      m_pExitWaiting->SetPixmap(":/interactivity/cb2");
      m_pExitWaiting->Append(":/interactivity/c1");
      m_pExitWaiting->Append(":/interactivity/c2");
      m_pExitWaiting->Append(":/interactivity/c3");
      m_pExitWaiting->Append(":/interactivity/c4");
      m_pExitWaiting->Append(":/interactivity/c5");
      m_pExitWaiting->Append(":/interactivity/c6");
      m_pExitWaiting->Append(":/interactivity/c7");
      m_pExitWaiting->Append(":/interactivity/c8");
      m_pExitWaiting->Append(":/interactivity/c9");
      m_pExitWaiting->Append(":/interactivity/c10");
      m_pExitWaiting->Append(":/interactivity/c11");
      m_pExitWaiting->Append(":/interactivity/c12");
   }
   m_pStartWaiting = new VhallWaiting(this);
   if (m_pStartWaiting) {
      m_pStartWaiting->SetCerclePos(55, 50);
      m_pStartWaiting->SetPixmap(":/interactivity/img/interactivity/startLive.png");
      m_pStartWaiting->Append(":/interactivity/c1");
      m_pStartWaiting->Append(":/interactivity/c2");
      m_pStartWaiting->Append(":/interactivity/c3");
      m_pStartWaiting->Append(":/interactivity/c4");
      m_pStartWaiting->Append(":/interactivity/c5");
      m_pStartWaiting->Append(":/interactivity/c6");
      m_pStartWaiting->Append(":/interactivity/c7");
      m_pStartWaiting->Append(":/interactivity/c8");
      m_pStartWaiting->Append(":/interactivity/c9");
      m_pStartWaiting->Append(":/interactivity/c10");
      m_pStartWaiting->Append(":/interactivity/c11");
      m_pStartWaiting->Append(":/interactivity/c12");
   }
   TRACE6("VhallInteractive::VhallInteractive\n");
   //屏幕共享时候  右上角 显示麦上主持和嘉宾的视频列表浮框
   m_pFloatRenderWdg = new CFloatRenderWdg();
   if (m_pFloatRenderWdg) {
      m_pFloatRenderWdg->hide();
   }
   m_pHideToolWidgetTimer = new QTimer(this);
   if (m_pHideToolWidgetTimer) {
      connect(m_pHideToolWidgetTimer, SIGNAL(timeout()), this, SLOT(Slot_OnHideToolWidget()));
   }

   //全屏共享 上部 工具栏
   m_pScreenShareToolWgd = new CScreenShareToolWgd();
   if (m_pScreenShareToolWgd) {
      m_pScreenShareToolWgd->hide();
      connect(m_pScreenShareToolWgd, SIGNAL(Sig_ClickedMicBtn()), this, SLOT(Slot_MicButtonClicked()));
      connect(m_pScreenShareToolWgd, SIGNAL(Sig_ClickedStopShareBtn()), this, SLOT(Slot_BtnScreenClicked()));
      connect(m_pScreenShareToolWgd, SIGNAL(Sig_ClickedCameraBtn()), this, SLOT(Slot_CameraBtnClicked()));
      connect(m_pScreenShareToolWgd, SIGNAL(Sig_ClickedPlayerBtn()), this, SLOT(Slot_PlayerButtonClicked()));
      connect(m_pScreenShareToolWgd, SIGNAL(Sig_ClickedStartLiveBtn()), this, SLOT(Slot_BtnStreamClicked()));
   }

   //普通屏幕显示 底部工具栏
   m_pInterActiveCtrlWdg = new CInterActiveCtrlWdg(this);
   if (m_pInterActiveCtrlWdg) {
      m_pAnimation = new QPropertyAnimation(m_pInterActiveCtrlWdg, "pos");
      ui.horizontalLayout->addWidget(m_pInterActiveCtrlWdg);
      connect(m_pInterActiveCtrlWdg, SIGNAL(Sig_OnClickedCameraCtrlBtn()), this, SLOT(Slot_CameraBtnClicked()));
      connect(m_pInterActiveCtrlWdg, SIGNAL(Sig_OnClickedMicCtrlBtn()), this, SLOT(Slot_MicButtonClicked()));
      connect(m_pInterActiveCtrlWdg, SIGNAL(Sig_OnClickedPlayerCtrlBtn()), this, SLOT(Slot_PlayerButtonClicked()));
      connect(m_pInterActiveCtrlWdg, SIGNAL(Sig_OnClickedScreenShareCtrlBtn()), this, SLOT(Slot_BtnScreenClicked()));
      connect(m_pInterActiveCtrlWdg, SIGNAL(Sig_OnClickedStartLiveCtrlBtn()), this, SLOT(Slot_BtnStreamClicked()));
      connect(m_pInterActiveCtrlWdg, SIGNAL(Sig_OnClickedLeaveActiveBtn()), this, SLOT(Slot_BtnLeaveLiveClicked()));
      connect(m_pInterActiveCtrlWdg, SIGNAL(Sig_OnClickedShowCameraListBtn()), this, SLOT(Slot_ShowCameraList()));
      connect(m_pInterActiveCtrlWdg, SIGNAL(Sig_OnClickedShowMicListBtn()), this, SLOT(Slot_ShowMicList()));
      connect(m_pInterActiveCtrlWdg, SIGNAL(Sig_OnClickedShowPlayerBtn()), this, SLOT(Slot_ShowSpeakerList()));
      connect(m_pInterActiveCtrlWdg, SIGNAL(Sig_OnMicBtnEnter()), this, SLOT(Slot_MicButtonEnter()));
      connect(m_pInterActiveCtrlWdg, SIGNAL(Sig_OnMicBtnLeave()), this, SLOT(Slot_MicButtonLeave()));
      connect(m_pInterActiveCtrlWdg, SIGNAL(Sig_OnPlayerBtnEnter()), this, SLOT(Slot_PlayerButtonEnter()));
      connect(m_pInterActiveCtrlWdg, SIGNAL(Sig_OnPlayerBtnLeave()), this, SLOT(Slot_PlayerButtonLeave()));
      connect(m_pInterActiveCtrlWdg, SIGNAL(Sig_MouseEnterCtrlWdg()), this, SLOT(Slot_MouseEnterCtrlWdg()));
      connect(m_pInterActiveCtrlWdg, SIGNAL(sig_OnPlayFileClicked()), this, SLOT(Slot_OnPlayFileClicked()));
      connect(m_pInterActiveCtrlWdg, &CInterActiveCtrlWdg::SigCurRecordState, this, &VhallInteractive::slotCurRecordState);
      connect(m_pInterActiveCtrlWdg, &CInterActiveCtrlWdg::SigStopLive, this, &VhallInteractive::slotStopLive);
   }

   m_pExternalCaptureTimer = new QTimer(this);
   if (m_pExternalCaptureTimer) {
      connect(m_pExternalCaptureTimer, SIGNAL(timeout()), this, SLOT(Slot_OnSendExternalCapture()));
   }

   connect(ui.renderTypeWdg, SIGNAL(Sig_MouseMoveInCenterNoticeWdg()), this, SLOT(Slot_MouseEnterCtrlWdg()));
   connect(ui.renderTypeWdg, SIGNAL(Sig_ContextMenuEvent()), this, SLOT(Slot_ContextCenterNoticeMenuEvent()));
   connect(ui.renderTypeWdg, SIGNAL(Sig_MouseDoubleClicked()), this, SLOT(Slot_SetFullWnd()));
   connect(&mTimeExpriseTimer, SIGNAL(timeout()), this, SLOT(Slot_OnTimeExprise()));
   connect(&mCheckMixStreamServerTimer, SIGNAL(timeout()), this, SLOT(Slot_OnCheckMixStreamServer())); 

   mHttpNetWork = new HttpNetWork(this);
   if (mHttpNetWork) {
      connect(mHttpNetWork, SIGNAL(HttpNetworkGetFinished(QByteArray, int)), this, SLOT(Slot_OnCheckMixStreamServerFinished(QByteArray, int)));
   }
   connect(&mCheckRenderStream, SIGNAL(timeout()), this, SLOT(Slot_OnCheckRenderStream()));
}

void VhallInteractive::SetVedioPlayUi(QWidget* pVedioPlayUI) {
   mpVedioPlayUi = pVedioPlayUI;
   if (mpVedioPlayUi) {
      m_pVedioPlayUiAnmt = new QPropertyAnimation(mpVedioPlayUi, "pos");
   }
}

VhallInteractive::~VhallInteractive() {
   mCheckMixStreamServerTimer.stop();
   if (mHttpNetWork) {
      delete mHttpNetWork;
      mHttpNetWork = NULL;
   }
   TRACE6("Enter Function:%s \n", __FUNCTION__);
   if (m_pCenterVideoRender) {
      disconnect(m_pCenterVideoRender, SIGNAL(applyFullScreen(CVideoRenderWdg*)), this, SLOT(OnVideoRenderFullScreen(CVideoRenderWdg*)));
      disconnect(m_pCenterVideoRender, SIGNAL(exitFullScreen(CVideoRenderWdg*)), this, SLOT(OnExitVideoRenderFullScreen(CVideoRenderWdg*)));
      disconnect(m_pCenterVideoRender, SIGNAL(Sig_MouseMoveInCenterRender()), this, SLOT(Slot_MouseEnterCtrlWdg()));
   }
   if (m_pExternalCaptureTimer) {
      m_pExternalCaptureTimer->stop();
      delete m_pExternalCaptureTimer;
      m_pExternalCaptureTimer = NULL;
   }
   if (mAlertDlg) {
      mAlertDlg->reject();
   }
   if (m_pHostExitTimer) {
      m_pHostExitTimer->stop();
      m_pHostExitTimer = NULL;
   }
   if (m_pAnimation) {
      m_pAnimation->stop();
      delete m_pAnimation;
      m_pAnimation = NULL;
   }
   if (m_pVedioPlayUiAnmt&&m_pVedioPlayUiAnmt->state()) {
      m_pVedioPlayUiAnmt->stop();
      m_pVedioPlayUiAnmt->deleteLater();
      m_pVedioPlayUiAnmt = NULL;
   }
   if (m_pUserMenu) {
      delete m_pUserMenu;
      m_pUserMenu = NULL;
   }
   if (m_pScreenShareToolWgd) {
      delete m_pScreenShareToolWgd;
      m_pScreenShareToolWgd = NULL;
   }
   if (m_pFloatRenderWdg) {
      delete m_pFloatRenderWdg;
      m_pFloatRenderWdg = NULL;
   }
   if (m_pFadeOutTipWidget) {
      delete m_pFadeOutTipWidget;
      m_pFadeOutTipWidget = NULL;
   }
   TRACE6("Leave Function:%s \n", __FUNCTION__);
}

enum PullStreamOperator {
   PullStream_None,
   PullStream_Start,
   PullStream_Stop
};

enum RemixStreamOperator {
   RemixStream_None,
   RemixStream_Sure,
   RemixStream_Cancel
};

typedef struct StatusSwitch_st {
   RemixStreamOperator remix;
   PullStreamOperator pullStream;
}StatusSwitch;

const static StatusSwitch StatusMechineOperators[4][4] = {
   { { RemixStream_None, PullStream_None }, { RemixStream_Sure, PullStream_Start }, { RemixStream_Sure, PullStream_Start }, { RemixStream_None, PullStream_None } },
   { { RemixStream_Cancel, PullStream_Stop }, { RemixStream_Sure, PullStream_None }, { RemixStream_None, PullStream_None }, { RemixStream_Sure, PullStream_None } },
   { { RemixStream_Cancel, PullStream_Stop }, { RemixStream_None, PullStream_None }, { RemixStream_Sure, PullStream_None }, { RemixStream_Sure, PullStream_None } },
   { { RemixStream_None, PullStream_None }, { RemixStream_Sure, PullStream_None }, { RemixStream_Sure, PullStream_None }, { RemixStream_Sure, PullStream_None } },
};

int VhallInteractive::GetCurrentStatus() {
   //判断当前成员麦克风状态。
   int nSpeakUser = 0;
   QMap<QString, bool> micStateMap = m_pSpeakerList->GetAllOpenMicUsers();
   if (m_desktopStreamId == "") {
      if (m_CameraListMap.count() == 0 && micStateMap.count() == 0) {
         return 0;
      } else {
         return 1;
      }
   } else {
      if (m_CameraListMap.count() == 0 && micStateMap.count() == 0) {
         return 2;
      }
   }
   return 3;
}

void VhallInteractive::Init(int appid, int bizid) {
   m_appid = appid;
   m_bizid = bizid;
   m_StreamMixObj.Init(appid);
}

void VhallInteractive::Reset(bool bIsActiveDisconnect) {
   mCheckMixStreamServerTimer.stop();
   TRACE6("Enter Function:%s \n", __FUNCTION__);
   if (mThreadPullReply) {
      mThreadPullReply->abort();
      mThreadPullReply->deleteLater();
      mThreadPullReply = NULL;
   }
   if (m_pExternalCaptureTimer) {
      m_pExternalCaptureTimer->stop();
   }
   m_nMixStreamStatus = 0;
   m_desktopStreamId = "";
   m_removeStreamID = "";
   m_CameraListMap.clear();
   m_pAnimation->stop();
   if (NULL != m_pVedioPlayUiAnmt &&m_pVedioPlayUiAnmt->state()) {
      m_pVedioPlayUiAnmt->stop();
   }

   m_SDK->ResetDeviceState();
   if (m_SDK->GetStreamState()) {
      m_SDK->StopPushStream();
      if (m_pInterActiveCtrlWdg) {
         m_pInterActiveCtrlWdg->OnStartLive(false);
      }
      if (m_pScreenShareToolWgd) {
         m_pScreenShareToolWgd->SetStartLive(false);
      }
      m_pUserMananger->NotifyHostPushState();
      ThirdPullStop(m_StreamId);
      TRACE6("%s ThirdPullStop\n", __FUNCTION__);
   }
   if (m_bIsFullScreen) {
      ResetToNormalSize();
      VH::CComPtr<IVhallRightExtraWidgetLogic> pVhallRightExtraWidget = NULL;
      DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IVhallRightExtraWidget, IID_IVhallRightExtraWidgetLogic, pVhallRightExtraWidget);
      if (pVhallRightExtraWidget) {
         pVhallRightExtraWidget->ShowExtraWidget(isVisible());
      }
      m_bIsFullScreen = false;
   }
   mPhotoPixmap = QPixmap();
   m_pFloatRenderWdg->ClearRenderWidgets();
   m_pHorWidget->Clear();
   m_pSpeakerList->OnLeaveActiveSuc(true);
   m_pSpeakerList->Clear();
   m_pSpeakerList->SetPushState(false);
   TRACE6("%s m_SDK->Logout()\n", __FUNCTION__);
   mbMaster = false;
   m_streamStatus = false;
   m_bScreenStatus = false;
   ReCheckScreenBtnStatus();
   m_selectedWidget = NULL;
   m_bIsUserSharing = false;
   m_SharingUser = "";
   if (m_startLiveTimer.isActive()) {
      m_startLiveTimer.stop();
      TRACE6("%s,%s,%d:mTimer.Stop\n", __FILE__, __FUNCTION__, __LINE__);
   }
   m_startLiveDateTime.setTime_t(0);
   m_bOpenCameraState = true;
   TRACE6("Leave Function:%s \n", __FUNCTION__);
}

void VhallInteractive::SetStreamId(bool master, QString streamId) {
   this->mbMaster = master;
   //处理嘉宾端是否隐藏成员列表
   QString toolConfPath = CPathManager::GetAppDataPath() + QString::fromStdWString(VHALL_TOOL_CONFIG);
   int hideMemberList = ConfigSetting::ReadInt(toolConfPath, GROUP_DEFAULT, KEY_VHALL_HIDE_MEMBER_LIST, 0);
   if (hideMemberList == 1 && !mbMaster) {
      ui.widget_4->hide();
      ui.widget_6->hide();
   }
   this->m_StreamId = streamId;
   if (m_pSpeakerList != NULL) {
      m_pSpeakerList->SetRole(master);
   }
   if (m_pInterActiveCtrlWdg) {
      m_pInterActiveCtrlWdg->InitBtnState(master);
   }
   if (!master) {
      SetCenterRenderViewType(RenderType_Wait);
      m_SDK->StopPushStream();
   }
   m_StreamMixObj.SetStreamid(streamId);
}
void VhallInteractive::SetUserId(QString userId) {
   m_UserId = userId;
   if (m_pSpeakerList) {
      m_pSpeakerList->SetUserId(userId);
   }
}
void VhallInteractive::SetThirdUrl(QString url) {
   if (url.left(4) != "http") {
      this->m_ThirdPullUrl = "http://" + url;
   } else {
      this->m_ThirdPullUrl = url;
   }
}

bool VhallInteractive::nativeEvent(const QByteArray &eventType, void *message, long *result) {
   MSG* msg = reinterpret_cast<MSG*>(message);
   if (!this->isHidden() && msg->hwnd == (HWND)this->winId()) {
      if (WM_SYSCOMMAND == msg->message) {
         if (SC_CLOSE == msg->wParam) {
            Slot_btnCloseClicked();
            return true;
         }
      }
   }
   if (WM_SETCURSOR == msg->message) {
      MouseMovedExtraWidget();
   }
   return QWidget::nativeEvent(eventType, message, result);
}

void VhallInteractive::closeEvent(QCloseEvent *e) {
   e->ignore();
   QWidget::closeEvent(e);
}

void VhallInteractive::ResetExternalWidget(QWidget* w) {
   TRACE6("%s start\n", __FUNCTION__);
   if (w == NULL || m_pExWidget != NULL) {
      TRACE6("%s w == NULL\n", __FUNCTION__);
      return;
   }

   ui.layout_right->addWidget(w);
   this->m_pExWidget = w;
   this->m_pExWidget->installEventFilter(this);
   SetShrink(false);
   TRACE6("%s reset ok\n", __FUNCTION__);
}

void VhallInteractive::AddRightExWidget(QWidget *w) {
   if (NULL == w || m_pExWidget != NULL) {
      TRACE6("%s NULL == w\n", __FUNCTION__);
      return;
   }

   m_pExWidget = w;
   ui.layout_right->addWidget(w);
   m_pExWidget->show();
   ui.widget_right->show();
}
void VhallInteractive::RemoveRightExWidget(QWidget *parent) {
   TRACE6("%s start\n", __FUNCTION__);
   if (!m_pExWidget) {
      TRACE6("%s return\n", __FUNCTION__);
      return;
   }
   ui.layout_right->removeWidget(m_pExWidget);
   m_pExWidget->setParent(parent);
   m_pExWidget = NULL;
   TRACE6("%s end\n", __FUNCTION__);

}
void VhallInteractive::AddOnlineList(QWidget *w) {
   if (!w) {
      return;
   }

   ui.layout_left_bottom->addWidget(w);
   m_pOnlineList = w;
   m_pOnlineList->show();
}
void VhallInteractive::RemoveOnlineList() {
   ui.layout_left_bottom->removeWidget(m_pOnlineList);
}

void VhallInteractive::Slot_SharedLiveUrl() {
   TRACE6("%s\n", __FUNCTION__);
   VhallSharedQr qr(this);
   qr.CenterWindow(this);
   QString toolConfPath = CPathManager::GetAppDataPath() + QString::fromStdWString(VHALL_TOOL_CONFIG);
   QString domain = ConfigSetting::ReadString(toolConfPath, GROUP_DEFAULT, KEY_VHALL_ShareURL, 0);
   qr.Shared(domain,m_StreamId);
}

void VhallInteractive::Slot_btnCloseClicked() {
   TRACE6("Enter Function:%s \n", __FUNCTION__);
   QJsonObject body;
   SingletonMainUIIns::Instance().reportLog(L"CloseInteraction", eLogRePortK_InteractionWdgClose, body);
   TRACE6("%s\n", __FUNCTION__);
   bool bStreamStart = m_SDK->GetStreamState();
   if (bStreamStart && mbMaster) {
      //AlertDlg dlg(QString::fromWCharArray(L"正在进行直播，请先结束直播再关闭直播界面"), true, this);
      ReleaseAlert();
      mAlertDlg = new AlertDlg(QString::fromWCharArray(L"正在进行直播，请先结束直播再关闭直播界面"), true, this);
      mAlertDlg->SetYesBtnText(QString::fromWCharArray(L"结束直播"));
      mAlertDlg->CenterWindow(ui.widget_center);
      QJsonObject body;
      if (mAlertDlg->exec() == QDialog::Accepted) {
         body["as"] = "OK";
         ResetMixStream();
         ExitToActiveListWnd();
      } else {
         body["as"] = "Cancel";
      }
      SingletonMainUIIns::Instance().reportLog(L"CloseInteraction_StopLive", eLogRePortK_InteractionWdgClose_StopLive, body);
   } else {
      ReleaseAlert();
      mAlertDlg = new AlertDlg(QString::fromWCharArray(L"是否退出当前直播"), true, this);
      //AlertDlg dlg(QString::fromWCharArray(L"是否退出当前直播"), true, this);
      mAlertDlg->CenterWindow(ui.widget_center);
      if (mAlertDlg->exec() == QDialog::Accepted) {
         ExitToActiveListWnd();
         SingletonMainUIIns::Instance().reportLog(L"CloseInteraction_Quit", eLogRePortK_InteractionWdg_Quit, body);
      }
   }
   TRACE6("Leave Function:%s \n", __FUNCTION__);
}

void VhallInteractive::Slot_btnMinClicked() {
   this->showNormal();
   this->showMinimized();
   QJsonObject body;
   SingletonMainUIIns::Instance().reportLog(L"Interaction_Min",eLogRePortK_InteractionWdgMin, body);
}

void VhallInteractive::Slot_ShowSpeakerList() {
   m_pPlayerList->Clear();
   if (m_SDK) {
      int count = m_SDK->GetSpeakerCount();
      const char *currentSpeakerId = m_SDK->GetCurrentSpeakerId();
      for (int i = 0; i < count; i++) {
         const char *name = m_SDK->GetSpeakerName(i);
         const char *id = m_SDK->GetSpeakerId(i);
         DeviceInfo info;
         info.m_sDeviceType = TYPE_COREAUDIO;
         QString::fromLocal8Bit(name).toWCharArray(info.m_sDeviceName);
         QString::fromLocal8Bit(name).toWCharArray(info.m_sDeviceDisPlayName);
         QString::fromLocal8Bit(id).toWCharArray(info.m_sDeviceID);
         m_pPlayerList->AddItemInterActivityItem(QString::fromLocal8Bit(name), info, i);
         if (currentSpeakerId != NULL) {
            if (strcmp(currentSpeakerId, id) == 0) {
               m_pPlayerList->SetChecked(info);
            }
         }
      }
   }

   m_pPlayerList->Show();
   QPoint p = m_pInterActiveCtrlWdg->GetPlayListBtnGlobalPos();
   int x = p.x() - m_pPlayerList->width() / 2 + m_pInterActiveCtrlWdg->GetPlayListBtnWidth() / 2;
   int y = p.y() - m_pPlayerList->height() - 8;
   m_pPlayerList->move(x, y);
}

void VhallInteractive::Slot_ShowMicList() {
   if (!IsEnableClicked()) {
      return;
   }
   m_pMicList->Clear();
   if (m_SDK) {
      int count = m_SDK->GetMicCount();
      const char *currentMicId = m_SDK->GetCurrentMicId();
      for (int i = 0; i < count; i++) {
         const char *name = m_SDK->GetMicName(i);
         const char *id = m_SDK->GetMicId(i);
         DeviceInfo info;
         info.m_sDeviceType = TYPE_COREAUDIO;

         QString::fromLocal8Bit(name).toWCharArray(info.m_sDeviceName);
         QString::fromLocal8Bit(name).toWCharArray(info.m_sDeviceDisPlayName);
         QString::fromLocal8Bit(id).toWCharArray(info.m_sDeviceID);
         m_pMicList->AddItemInterActivityItem(QString::fromLocal8Bit(name), info, i);
         if (currentMicId != NULL) {
            if (strcmp(currentMicId, id) == 0) {
               m_pMicList->SetChecked(info);
            }
         }
      }
   }

   m_pMicList->Show();
   QPoint p = m_pInterActiveCtrlWdg->GetMicListBtnGlobalPos();
   int x = p.x() - m_pMicList->width() / 2 + m_pInterActiveCtrlWdg->GetMicListBtnWidth() / 2;
   int y = p.y() - m_pMicList->height() - 8;
   m_pMicList->move(x, y);
}

void VhallInteractive::Slot_ShowCameraList() {
   if (!IsEnableClicked()) {
      return;
   }
   m_pCameraList->Clear();
   if (m_SDK) {
      int count = m_SDK->GetCameraCount();
      const char *currentCameraId = m_SDK->GetCurrentCameraId();
      for (int i = 0; i < count; i++) {
         const char *name = m_SDK->GetCameraName(i);
         const char *id = m_SDK->GetCameraId(i);
         DeviceInfo info;
         info.m_sDeviceType = TYPE_DSHOW_VIDEO;

         QString::fromLocal8Bit(name).toWCharArray(info.m_sDeviceName);
         QString::fromLocal8Bit(name).toWCharArray(info.m_sDeviceDisPlayName);
         QString::fromLocal8Bit(id).toWCharArray(info.m_sDeviceID);
         m_pCameraList->AddItemInterActivityItem(QString::fromLocal8Bit(name), info, i);
         if (currentCameraId != NULL) {
            if (strcmp(currentCameraId, id) == 0) {
               m_pCameraList->SetChecked(info);
            }
         }
      }
   }

   m_pCameraList->Show();
   QPoint p = m_pInterActiveCtrlWdg->GetCameraListBtnGlobalPos();
   int x = p.x() - m_pCameraList->width() / 2 + m_pInterActiveCtrlWdg->GetCameraListBtnWidth() / 2;
   int y = p.y() - m_pCameraList->height() - 8;
   m_pCameraList->move(x, y);
}

void VhallInteractive::Slot_ScreenShared(bool ok) {
   QJsonObject body;
   if (ok) {
      body["ac"] = "Open";
      TRACE6("%s OpenExternalCapture\n", __FUNCTION__);
      //m_SDK->CloseExternalCapture();
      //m_pExternalCaptureTimer->stop();
      m_SDK->OpenScreen(NULL, 5);
   } else {
      body["ac"] = "Close";
      m_SDK->CloseScreen();
   }
   SingletonMainUIIns::Instance().reportLog(L"Interaction_DesktopShare", eLogRePortK_Interaction_DesktopShare, body);
}

void VhallInteractive::SetInterActivitySDK(IInterActivitySDK *m) {
   this->m_SDK = m;
}

void VhallInteractive::SetInterActivityUsers(IInterAcivtityUser *m) {
   this->m_pUserMananger = m;
}

void VhallInteractive::Slot_BtnScreenClicked() {
   if (!IsEnableClicked()) {
      return;
   }

   if (IsPlayMediaFile()) {
      AlertDlg alert(ALREADY_PLAY_MEDIA_FILE, false, NULL);
      alert.CenterWindow(ui.widget_center);
      alert.exec();
      return;
   }

   bool currentScreenStatus = m_SDK->GetScreenStatus();
   if (m_bIsUserSharing && !currentScreenStatus) {
      AlertDlg alert(ALREADY_SCREEN_SHARE, false, NULL);
      alert.CenterWindow(ui.widget_center);
      alert.exec();
      return;
   }
   
   TRACE6("%s currentScreenStatus:%d\n", __FUNCTION__, currentScreenStatus);
   Slot_ScreenShared(!currentScreenStatus);
   SingletonMainUIIns::Instance().PostCRMessage(MSG_OBSCONTROL_DESKTOPSHARE, NULL, 0);
}

void VhallInteractive::Slot_BtnLeaveLiveClicked() {
   qDebug() << "VhallInteractive::Slot_BtnLeaveLiveClicked() ";
   Slot_btnCloseClicked();
}

void VhallInteractive::Slot_BtnStreamClicked() {
   TRACE6("Enter Function:%s \n", __FUNCTION__);
   VH::CComPtr<IVhallRightExtraWidgetLogic> pVhallRightExtraWidget = NULL;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IVhallRightExtraWidget, IID_IVhallRightExtraWidgetLogic, pVhallRightExtraWidget);
   if (!pVhallRightExtraWidget) {
      TRACE6("Enter Function:%s !pVhallRightExtraWidget\n", __FUNCTION__);
      return;
   }
   QJsonObject body;
   body["ac"] = m_SDK->GetStreamState() ? "StopStream" : "StartStream";
   SingletonMainUIIns::Instance().reportLog(L"Interaction_StartOrStopStream", eLogRePortK_Interaction_StartOrStopStream, body);

   STRU_OBSCONTROL_PUBLISH loPublish;
   loPublish.m_bInteractive = true;
   loPublish.m_bExit = false;
   QEventLoop loop;
   if (!m_SDK->GetStreamState()) {
      TRACE6("Enter Function:%s m_SDK->GetStreamState() = false\n", __FUNCTION__);
      if (m_pStartWaiting) {
         m_pStartWaiting->Show();
      }
      if (!pVhallRightExtraWidget->StartWebnair()) {
         wchar_t msg[1024] = { 0 };
         pVhallRightExtraWidget->GetLastError(msg);
         QString errMsg = QString::fromWCharArray(msg);
         if (errMsg == "") {
            errMsg = QString::fromWCharArray(L"开始活动失败，请稍后重试");
         }

         int code = pVhallRightExtraWidget->GetLastCode();
         if (code == 1001) {
            QString domain = ConfigSetting::ReadString(CPathManager::GetConfigPath(), GROUP_DEFAULT, KEY_URL_DOMAIN, "");
            QString url = domain + "/account/center";
            AlertDlg tip(errMsg, true, this);
            tip.CenterWindow(this);
            tip.SetYesBtnText(QString::fromWCharArray(L"前往购买"));
            tip.SetNoBtnText(QString::fromWCharArray(L"取消"));
            if (tip.exec() == QDialog::Accepted) {
               QDesktopServices::openUrl(url);
            }
         } else {
            FadeOutTip(errMsg);
         }
         if (m_pStartWaiting) {
            m_pStartWaiting->Close();
         }
         TRACE6("Enter Function:%s m_SDK->GetStreamState() = false close ok\n", __FUNCTION__);
         return;
      }

      TRACE6("VhallInteractive::Slot_BtnStreamClicked() StartPushStream\n");
      m_SDK->StartPushStream();
      m_pUserMananger->SetHasPushStream(true);
      if (m_pInterActiveCtrlWdg) {
         m_pInterActiveCtrlWdg->OnStartLive(true);
      }
      if (m_pScreenShareToolWgd) {
         m_pScreenShareToolWgd->SetStartLive(true);
      }
      m_startLiveTimer.start(1000);
      TRACE6("%s,%s,%d:mTimer.Start\n", __FILE__, __FUNCTION__, __LINE__);
      loPublish.m_bIsStartPublish = true;
      OpenExternalCapture();
      m_MixStreamUrl = "rtmp://" + QString::number(m_bizid) + ".liveplay.myqcloud.com/live/" + m_StreamId + STREAM_TAIL;
      ThirdPullStart(m_StreamId, m_MixStreamUrl);
      m_pUserMananger->NotifyHostPushState();//广播推流
      m_streamStatus = true;
      if (mbMaster && !IsExistScreenShare() && (m_CameraListMap.size() == 0 || (m_CameraListMap.size() == 1 && !m_SDK->GetCurrentCameraState()))) {
         TRACE6("%s OpenExternalCapture\n", __FUNCTION__);
         // m_SDK->OpenExternalCapture();
      }
      TRACE6("%s Goto ReMixStream\n", __FUNCTION__);
      ReMix();
      if (m_pStartWaiting) {
         m_pStartWaiting->Close();
      }
      m_pUserMananger->NotifySelfMediaState();
   } else {
      if (!m_streamStatus) {
         TRACE6("m_streamStatus = false\n");
         return;
      }
      if (GetRecordState() == eRecordState_Stop) {
         m_pExitWaiting->Show();
      }
      int iResult = pVhallRightExtraWidget->StopWebnair();
      if (eStopWebNair_Fail == iResult) {
         AlertDlg tip(QString::fromWCharArray(L"当前网络异常，结束直播失败，请重试"), false, this);
         tip.CenterWindow(ui.widget_center);
         tip.exec();
         m_streamStatus = true;
         m_pExitWaiting->Close();
         return;
      } else if (eStopWebNair_StopRecordFail == iResult) {
         m_pExitWaiting->Close();
         return;
      }
      m_pExitWaiting->Close();
      m_streamStatus = false;
      TRACE6("VhallInteractive::Slot_BtnStreamClicked() StopPushStream\n");

      if (m_pInterActiveCtrlWdg) {
         m_pInterActiveCtrlWdg->OnStartLive(false);
      }
      if (m_pScreenShareToolWgd) {
         m_pScreenShareToolWgd->SetStartLive(false);
      }
      m_startLiveTimer.stop();
      TRACE6("%s,%s,%d:mTimer.Stop\n", __FILE__, __FUNCTION__, __LINE__);
      m_startLiveDateTime.setTime_t(0);
      ResetMixStream();
      m_SDK->StopPushStream();
      loPublish.m_bServerPlayback = true;
      loPublish.m_bIsStartPublish = false;
      ThirdPullStop(m_StreamId);
      m_pUserMananger->NotifyHostPushState();
      m_pUserMananger->ClearGuestUser(false);
   }
   TRACE6("Leave Function:%s \n", __FUNCTION__);
}

void VhallInteractive::Slot_GuesetUserItemClicked(QString id) {
   TRACE6("VhallInteractive Slot_GuesetUserItemClicked %s", id.toStdString().c_str());
   if (m_pUserMananger) {
      m_pUserMananger->RemoveActiveMember(id.toStdString().c_str());
   }
}

void VhallInteractive::Slot_CameraItemClicked(bool bCheck, DeviceInfo info) {
   qDebug() << "VhallInteractive::SlotItemClicked" << bCheck << QString::fromWCharArray(info.m_sDeviceID);
   QString id = QString::fromWCharArray(info.m_sDeviceID);
   if (bCheck) {
      TRACE6("%s CloseExternalCapture\n", __FUNCTION__);
      m_SDK->CloseExternalCapture();
      m_pExternalCaptureTimer->stop();
      m_SDK->OpenCamera(id.toUtf8().data());
      TRACE6("%s,%s,%d:  OpenSpeaker failed\n", __FILE__, __FUNCTION__, __LINE__);
   } else {
      m_LastSelectCameraID = m_SDK->GetCurrentCameraId();
      this->m_SDK->CloseCamera();
   }
}
void VhallInteractive::Slot_SpeakerItemClicked(bool bCheck, DeviceInfo info) {
   qDebug() << "VhallInteractive::Slot_SpeakerItemClicked" << bCheck << QString::fromWCharArray(info.m_sDeviceID);
   QString id = QString::fromWCharArray(info.m_sDeviceID);
   if (bCheck) {
      this->m_SDK->OpenSpeaker(id.toUtf8().data());
   } else {
      this->m_SDK->CloseSpeaker();
   }
}

void VhallInteractive::Slot_MicItemClicked(bool bCheck, DeviceInfo info) {
   qDebug() << "VhallInteractive::SlotMicItemClicked" << bCheck << QString::fromWCharArray(info.m_sDeviceID);
   QString id = QString::fromWCharArray(info.m_sDeviceID);
   if (bCheck) {
      this->m_SDK->OpenMic(id.toUtf8().data());
      TRACE6("SlotMicItemClicked OpenMic\n");
   } else {
      m_LastSelectMicID = m_SDK->GetCurrentMicId();
      this->m_SDK->CloseMic();
      TRACE6("SlotMicItemClicked CloseMic\n");
   }
}
void VhallInteractive::Timeout() {
   if (m_startLiveDateTime.toTime_t() == 0) {
      m_startLiveDateTime = QDateTime::currentDateTime();
   }

   unsigned long long diff = QDateTime::currentDateTime().toTime_t() - m_startLiveDateTime.toTime_t();
   int h = diff / 3600;
   int m = diff % 3600 / 60;
   int s = diff % 60;
   char cmd[128] = { 0 };
   sprintf(cmd, "%02d:%02d:%02d", h, m, s);
   if (m_pInterActiveCtrlWdg) {
      m_pInterActiveCtrlWdg->OnSetLiveTime(cmd);
   }
}

void VhallInteractive::OnScreenExit(const char *id) {
   EnableRenderWidgetScreen(id, false);
   m_desktopStreamId = "";
   m_removeStreamID = "";
   if (mbMaster && !IsExistScreenShare() && m_CameraListMap.size() == 0 && m_SDK->GetStreamState()) {
      TRACE6("%s OpenExternalCapture\n", __FUNCTION__);
   }
   TRACE6("%s Goto ReMixStream\n", __FUNCTION__);
   ReMix();
}

void VhallInteractive::OnCameraExit(const char *id,bool bRemix) {
   EnableRenderWidgetCamera(id, false);
}

void VhallInteractive::OnScreenEnter(const char *id) {
   QString tstreamId = "8136_" + GetTencentStreamId(m_StreamId, strlen(id) == 0 ? m_UserId : QString(id), "aux");
   m_desktopStreamId = tstreamId;
   m_removeStreamID = "8136_" + GetTencentStreamId(m_StreamId, strlen(id) == 0 ? m_UserId : QString(id), "main");
   EnableRenderWidgetScreen(id, true);
   TRACE6("%s Goto ReMixStream\n", __FUNCTION__);
   ReMix();
}

void VhallInteractive::OnCameraEnter(const char *id) {
   EnableRenderWidgetCamera(id, true);
   TRACE6("%s Goto ReMixStream\n",__FUNCTION__);
}

void VhallInteractive::ReMix() {
   TRACE6("Enter Function:%s \n", __FUNCTION__);
   if (m_nMixStreamStatus < 0) {
      m_nMixStreamStatus = 0;
   }

   if (m_nMixStreamStatus > 3) {
      m_nMixStreamStatus = 3;
   }

   int currentStatus = GetCurrentStatus();
   StatusSwitch statusSwitch = StatusMechineOperators[m_nMixStreamStatus][currentStatus];
   m_nLastMixStreamStatus = m_nMixStreamStatus;
   m_nMixStreamStatus = currentStatus;

   if (!mbMaster) {
      return;
   }

   QSet<QString> cameraList;
   QSet<QString> mixMicList;

   //更新当前需要混音用户的流id.
   QMap<QString, bool> allSpeakerMic = m_pSpeakerList->GetAllOpenMicUsers();
   QMap<QString, bool>::iterator iterAllSpeaker = allSpeakerMic.begin();
   while (iterAllSpeaker != allSpeakerMic.end()) {
      QString tstreamId = "8136_" + GetTencentStreamId(m_StreamId, iterAllSpeaker.key(), "main");
      mixMicList.insert(tstreamId);
      iterAllSpeaker++;
   }

   QMap<QString, bool>::iterator iter = m_CameraListMap.begin();
   while (iter != m_CameraListMap.end()) {
      cameraList.insert(iter.key());
      iter++;
   }
   if (statusSwitch.remix == RemixStream_Sure && m_SDK->GetStreamState()) {
      ////屏幕共享的时候，不再混合自己摄像头那一路的音频。
      if (!m_desktopStreamId.isEmpty()) {
         QSet<QString>::iterator iter = cameraList.find(m_removeStreamID);
         if (iter != cameraList.end()) {
            cameraList.erase(iter);
         }

         QSet<QString>::iterator iterMic = mixMicList.find(m_removeStreamID);
         if (iterMic != mixMicList.end()) {
            mixMicList.erase(iterMic);
         }
      }

      //mixMicList 只保留没有开启摄像头，但是开启了麦克风的用户。
      QSet<QString>::iterator iterCamera = cameraList.begin();
      while (iterCamera != cameraList.end()) {
         QSet<QString>::iterator iterMic = mixMicList.find(*iterCamera);
         if (iterMic != mixMicList.end()) {
            mixMicList.erase(iterMic);
         }
         iterCamera++;
      }
      ReCheckMixUser(cameraList, mixMicList);
      m_StreamMixObj.RemixStream(m_desktopStreamId, cameraList, mixMicList);
   } else if (statusSwitch.remix == RemixStream_Cancel && m_SDK->GetStreamState()) {
      if (m_desktopStreamId.isEmpty() && cameraList.size() == 0 && !m_SDK->GetScreenStatus()) {
         m_StreamMixObj.CancelRemix();
      }
   }
   TRACE6("Leave Function:%s \n", __FUNCTION__);
}

void VhallInteractive::Render(VHVideoFrame frame) {
   CVideoRenderWdg *render = GetVideoRender(frame.identifier);
   if (render) {
      render->SetLastRenderTime();
      SetMixStreamUserID(frame.identifier);
      if ((frame.src_type == 1 && render->GetPauseState()) || frame.externalData) {
         return;
      }

      if (frame.src_type == ilive::VIDEO_SRC_TYPE_CAMERA) {
         bool ret = render->DoRender(frame);
         if (this->m_selectedWidget == NULL) {
            this->m_selectedWidget = render;
            ResetCenterRenderMemberInfo(render);
         }
      }

      switch (frame.src_type) {
      case ilive::VIDEO_SRC_TYPE_CAMERA:{
         if (render == this->m_selectedWidget && !m_bIsUserSharing && !m_SDK->GetScreenStatus() && m_SDK->GetPlayFileState() == ilive::E_PlayMediaFileStop && !mbIsUserPlayMediaFile) {
            CVideoRenderWdg *centerRender = dynamic_cast<CVideoRenderWdg *>(m_centerRenderWidget);
            if (centerRender) {
               if (RenderType_Video != mRenderType) {
                  SetCenterRenderViewType(RenderType_Video);
               }
               centerRender->DoRender(frame);
            }
         }
      }
      break;
      case ilive::VIDEO_SRC_TYPE_SCREEN:{
         if (!m_SDK->GetScreenStatus() && m_bIsUserSharing) {
            CVideoRenderWdg *centerRender = dynamic_cast<CVideoRenderWdg *>(m_centerRenderWidget);
            if (centerRender) {
               if (RenderType_Video != mRenderType) {
                  SetCenterRenderViewType(RenderType_Video);
               }
               centerRender->DoRender(frame);
            } else {
               SetCenterRenderViewType(RenderType_Screen);
            }
         }
      }
      break;
      case ilive::VIDEO_SRC_TYPE_MEDIA:{
         CVideoRenderWdg *centerRender = dynamic_cast<CVideoRenderWdg *>(m_centerRenderWidget);
         if (centerRender) {
            if (RenderType_Video != mRenderType) {
               SetCenterRenderViewType(RenderType_Video);
            }
            centerRender->DoRender(frame);
         }
      }
      break;
      default:
         break;
      }
   }
}

QString VhallInteractive::GetTencentStreamId(QString streamId, QString userId, QString type/*main | aux*/) {
   QCryptographicHash md(QCryptographicHash::Md5);
   md.addData(QString(streamId + "_" + userId + "_" + type).toUtf8());
   return md.result().toHex();
}

void VhallInteractive::ThirdPullStart(QString streamId, QString url) {
   if (!mbMaster) {
      return;
   }

   if (mThreadPullReply) {
      mThreadPullReply->abort();
      mThreadPullReply->deleteLater();
      mThreadPullReply = NULL;
   }
   TRACE6("VhallInteractive::ThirdPullStart %s,%s\n", streamId.toLocal8Bit().data(), url.toLocal8Bit().data());
   QJsonObject obj;
   obj["sessionid"] = "vhall_inter_" + streamId;
   obj["command"] = "start";
   obj["vhallid"] = streamId;
   obj["3rdstream"] = url;
   obj["type"] = "pull";
   obj["ishelper"] = "1";


   QNetworkRequest request;
   request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
   request.setUrl(QUrl(m_ThirdPullUrl));
   QJsonDocument doc;
   doc.setObject(obj);

   TRACE6("VhallInteractive::ThirdPullStart put %s,%s\n", m_ThirdPullUrl.toLocal8Bit().data(), doc.toJson().data());
   QEventLoop loop;
   mThreadPullReply = m_ThirdPullManager.put(request, doc.toJson());
   connect(&m_ThirdPullManager, SIGNAL(finished(QNetworkReply *)), &loop, SLOT(quit()));
   loop.exec();

   mCheckMixStreamServerTimer.start(MAX_CHECK_MIX_STREAM_SERVER_TIMEOUT);
   mCheckRenderStream.start(MAX_CHECK_MIX_STREAM_SERVER_TIMEOUT);
}
void VhallInteractive::ThirdPullStop(QString streamId) {
   if (!mbMaster) {
      return;
   }

   if (mThreadPullReply) {
      mThreadPullReply->abort();
      mThreadPullReply->deleteLater();
      mThreadPullReply = NULL;
   }

   TRACE6("VhallInteractive::ThirdPullStop %s\n", streamId.toLocal8Bit().data());
   QJsonObject obj;
   obj["sessionid"] = "vhall_inter_" + streamId;
   obj["command"] = "stop";

   QNetworkRequest request;
   request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
   request.setUrl(QUrl(m_ThirdPullUrl));
   QJsonDocument doc;
   doc.setObject(obj);

   TRACE6("VhallInteractive::ThirdPullStop put %s,%s\n", m_ThirdPullUrl.toLocal8Bit().data(), doc.toJson().data());

   QEventLoop loop;
   mThreadPullReply = m_ThirdPullManager.put(request, doc.toJson());
   connect(&m_ThirdPullManager, SIGNAL(finished(QNetworkReply *)), &loop, SLOT(quit()));
   loop.exec();

   mCheckMixStreamServerTimer.stop();
   mCheckRenderStream.stop();
}

void VhallInteractive::ThirdStreamFinished(QNetworkReply *r) {
   QByteArray ba = r->readAll();
   r->deleteLater();
   mThreadPullReply = NULL;
   TRACE6("VhallInteractive::ThirdStreamFinished \n");
}

void VhallInteractive::OnPushStreamSuccess(const char *url) {
   mSDKRet = true;
   m_MixStreamUrl = "rtmp://" + QString::number(m_bizid) + ".liveplay.myqcloud.com/live/" + m_StreamId;
   qDebug() << "VhallInteractive::OnPushStreamSuccess" << m_MixStreamUrl;
   emit this->SigSDKReturn();
}

void VhallInteractive::OnPushStreamFailed() {
   TRACE6("Enter Function:%s \n", __FUNCTION__);
   mSDKRet = false;
   emit this->SigSDKReturn();
}

void VhallInteractive::Slot_JoinActivity() {
   TRACE6("Enter Function:%s \n", __FUNCTION__);
   m_pUserMananger->SendJoinRequest(m_userHeadUrl.toStdString().c_str());
}

void VhallInteractive::Slot_CancelJoinActivity() {
   TRACE6("Enter Function:%s \n", __FUNCTION__);
   m_pUserMananger->SendCancelJoinRequest();
}

void VhallInteractive::Slot_HostInviteJoin() {
   TRACE6("Enter Function:%s \n", __FUNCTION__);
   if (m_pInviteJoinWidget && m_pInviteJoinWidget->isHidden()) {
      m_pInviteJoinWidget->CenterWindow(this);
      m_pInviteJoinWidget->exec();
   }
}

void VhallInteractive::Slot_GuestAgreeHostInvite() {
   emit Sig_GuestAgreeHostInvite();
   if (m_pInviteJoinWidget) {
      m_pInviteJoinWidget->close();
   }
}

void VhallInteractive::Slot_GuestRefuseHostInvite() {
   TRACE6("Enter Function:%s \n", __FUNCTION__);
   //拒绝主播上麦要向主播回复C2C拒绝消息。
   m_pUserMananger->SendRefuseInvite();
   if (m_pInviteJoinWidget) {
      m_pInviteJoinWidget->close();
   }
}

void VhallInteractive::SetUserName(QString &userName) {
   if (m_userInfo) {
      m_userInfo->SlotSetUserName(userName);
   }

   CVideoRenderWdg *render = GetVideoRender(this->m_UserId);
   if (render) {
      render->SetUserName(userName);
   }
}

void VhallInteractive::SetUserImage(QString &userImage) {
   m_userHeadUrl = userImage;
   if (m_userInfo) {
      m_userInfo->SetUserImage(userImage);
   }
   if (m_pUserMananger) {
      m_pUserMananger->SetUserHeadImageUrl(userImage.toStdString().c_str());
   }

   CVideoRenderWdg *render = GetVideoRender(this->m_UserId);
   if (render) {
      render->SetHeadImageUrl(m_userHeadUrl);
   }
}

void VhallInteractive::Show() {
   TRACE6("Enter Function:%s \n", __FUNCTION__);
   if (true) {
      if (m_pInterActiveCtrlWdg) {
         m_pInterActiveCtrlWdg->InitBtnState(mbMaster);
      }
      m_pSpeakerList->SetPushState(false);
      if (mbMaster) {
         m_pHorWidget->Clear();
         ReOpenCamera();
         ReOpenMic();
			ReOpenSpeaker();
         m_pHostExitTimer->stop();
         m_pSpeakerList->Append(m_UserId, m_UserId, true);
         bool bCameraOpen = m_SDK->GetCurrentCameraId() != NULL ? true : false;
         bool bMicOpen = m_SDK->GetCurrentMicId() != NULL ? true : false;
         m_pSpeakerList->SetCameraIcon(m_UserId, bCameraOpen);
         m_pSpeakerList->SetMicIcon(m_UserId, bMicOpen);
         TRACE6("%s Goto ReMixStream\n", __FUNCTION__);
         ReMix();
         CreateRenderWidget(m_UserId.toStdString().c_str());
         CVideoRenderWdg *render = GetVideoRender(m_UserId);
         if (render) {
            render->SetHeadImageUrl(m_userHeadUrl);
            m_selectedWidget = render;
            ResetCenterRenderMemberInfo(render);
         }
      } else {
         if (m_SDK->GetCurrentCameraState()) {
            m_SDK->CloseCamera();
         } else {
            OnDeviceCallBack(ilive::E_CloseCamera, 0, NULL);
         }
         if (m_SDK->GetCurrentMicState()) {
            m_SDK->CloseMic();
         } else {
            OnDeviceCallBack(ilive::E_CloseMic, 0, NULL);
         }
         m_SDK->SetPlayerVolumn(0);
         m_SDK->CloseSpeaker();
         ReCheckPlayerBtnStatus();
         SendUserJoinRoomState(AVIMCMD_EnterLive);
      }
   }
   if (m_toolChatBtn) {
      m_toolChatBtn->show();
   }
   OpenExternalCapture();
   this->show();
   this->activateWindow();
   QRect r = QApplication::desktop()->availableGeometry();
   QPoint pos = QPoint((r.width() - this->width()) / 2, (r.height() - this->height()) / 2);
   this->move(pos);
   TRACE6("Leave Function:%s \n", __FUNCTION__);
}

void VhallInteractive::Hide() {
   this->hide();
}

void VhallInteractive::Slot_WidgetClicked(QWidget *w) {
   TRACE6("Enter Function:%s \n", __FUNCTION__);
   if (m_SDK->GetScreenStatus()) {
      SetCenterRenderViewType(RenderType_Screen);
      return;
   }
   if (m_bIsUserSharing || mbIsUserPlayMediaFile || m_pUserMananger->IsLockUserView() || IsPlayMediaFile()) {
      return;
   }
   SelectRender(w);
   TRACE6("Leave Function:%s \n", __FUNCTION__);
}

void VhallInteractive::OnMemStatusChanged(int event_id, const char * identifier) {
   TRACE6("VhallInteractive::OnMemStatusChanged %d user:%s\n", event_id, identifier);
   switch (event_id) {
   case ilive::EVENT_ID_ENDPOINT_EXIT:{
      if (m_UserId.compare(QString(identifier)) == 0) {
         RemoveUserCameraState(QString(identifier));
         emit SigMasterDisConnect();
      }
   }
   break;
   case ilive::EVENT_ID_ENDPOINT_NO_CAMERA_VIDEO:{
      if (identifier) {
         CVideoRenderWdg *render = GetVideoRender(QString(identifier));
         if (render != NULL) {
            render->Clear(true);
         }
      }
   }
   break;
   case ilive::EVENT_ID_ENDPOINT_HAS_CAMERA_VIDEO:{
      OnCameraEnter(identifier);
   }
   break;
   case ilive::EVENT_ID_ENDPOINT_HAS_SCREEN_VIDEO:{
      OnScreenEnter(identifier);
   }
   break;
   case ilive::EVENT_ID_ENDPOINT_NO_AUDIO:{
      if (QString(identifier).compare(m_UserId) == 0) {
         //如果设备列表没有设备，目前麦克风有选中的设备，暂时认为是将设备拔出。
         if (m_SDK->GetMicCount() == 0 && m_SDK->GetCurrentMicId()) {
            m_LastSelectMicID = m_SDK->GetCurrentMicId();
            this->m_SDK->CloseMic();
         }
      }
   }
   break;
   case ilive::EVENT_ID_ENDPOINT_HAS_MEDIA_VIDEO:{
      RoomHasMediaFileStream(identifier);
   }
   break;
   case ilive::EVENT_ID_ENDPOINT_NO_MEDIA_VIDEO:{
      RoomNoMediaFileStream(identifier);
   }
   break;
   default:
      break;
   }
   TRACE6("Leave Function:%s \n", __FUNCTION__);
}

void VhallInteractive::SetTopWidgetShowState() {
   TRACE6("Enter Function:%s \n", __FUNCTION__);
   if (m_pHorWidget && !m_bIsFullScreen) {
      //一个人时显示大画面，一个人以上即显示大画面又显示小画面。
      int nUserCount = m_pHorWidget->GetRenderCount();
      if (nUserCount >= 2) {
         ui.widget_top->show();
         if (IsExistScreenShare() || IsPlayMediaFile()) {
            ShowAllTopUserWnd();
         }
      }
      else if (nUserCount == 1 && (IsExistScreenShare() || IsPlayMediaFile())) {
         ui.widget_top->show();
         ShowAllTopUserWnd();
      }
      else if (m_pHorWidget->GetRenderCount() <= 1 && !IsExistScreenShare() && !IsPlayMediaFile()) {
         ui.widget_top->hide();
      }
   }
   TRACE6("Leave Function:%s \n", __FUNCTION__);
}

void VhallInteractive::CreateRenderWidget(const char *id) {
   TRACE6("Enter Function:%s \n", __FUNCTION__);
   QString identifier = id;
   if (identifier == "") {
      identifier = m_UserId;
      TRACE6("Slot_ActiveMemberJoin identifier == "";\n");
   }
   if (!m_pHorWidget) {
      TRACE6("%s m_pHorWidget == null \n", __FUNCTION__);
      return;
   }
   QWidget *w = m_pHorWidget->GetRenderWidget(identifier);
   if (!w) {
      TRACE6("%s new render\n", __FUNCTION__);
      VhallRenderWidget *wRender = new VhallRenderWidget();
      CVideoRenderWdg *render = new CVideoRenderWdg();
      if (render) {
         render->SetID(QString::fromStdString(id));
         connect(render, SIGNAL(Sig_DrawUserImage(QString)), this, SLOT(Slot_OnRenderDrawImage(QString)));
         render->Clear(true);
         TRACE6("CreateRenderWidget connect\n");
      }
      wRender->AddWidget(render);
      m_pHorWidget->Append(identifier, wRender);
      if (!m_selectedWidget) {
         m_selectedWidget = render;
         ResetCenterRenderMemberInfo(render);
      }
      
      //只有一个人时，隐藏当前成员在小窗中的画面，只显示在大画面中。
      TRACE6("%s m_pHorWidget->GetRenderCount() %d \n", __FUNCTION__, m_pHorWidget->GetRenderCount());
      if (m_pHorWidget->GetRenderCount() == 1) {
         ChangeCenterRendUser(QString(), id);
      }
      if (m_bIsFullScreen || (m_bIsFullScreen && IsExistScreenShare()) || m_SDK->GetScreenStatus() || (m_bIsFullScreen && IsPlayMediaFile())) {
         TRACE6("%s m_bIsFullScreen || IsExistScreenShare()\n", __FUNCTION__);
         MoveRenderFromTopWndToFloatWnd();
      }

      SetTopWidgetShowState();
      TRACE6("%s new render ok\n", __FUNCTION__);
   }
   m_pSpeakerList->Append(identifier, identifier, mbMaster ? (identifier == m_UserId) : false);
   TRACE6("Leave Function:%s \n", __FUNCTION__);
}

void VhallInteractive::RemoveRenderWidget(const char *id) {
   TRACE6("Enter Function:%s \n", __FUNCTION__);
   QString identifier = id;
   if (identifier.isEmpty()) {
      identifier = m_UserId;
   }

   //提前与m_pHorWidget->Remove(identifier);
   if (m_pFloatRenderWdg) {
      m_pFloatRenderWdg->RemoveRender(QString(id));
   }

   TRACE6("VhallInteractive::RemoveRenderWidget %s", id);
   QWidget *w = m_pHorWidget->GetRenderWidget(identifier);
   if (w) {
      VhallRenderWidget *wRender = dynamic_cast<VhallRenderWidget *>(w);
      if (wRender != NULL) {
         QWidget *renderWidget = wRender->GetWidget();
         if (renderWidget == this->m_selectedWidget) {
            this->m_selectedWidget = NULL;
            this->m_centerRenderWidget->repaint();
         }
      }
   }
   m_pHorWidget->Remove(identifier);
   m_pSpeakerList->Remove(identifier);
   OnCameraExit(id);
   SetMixStreamUserID(id,false);
   TRACE6("%s Goto ReMixStream\n", __FUNCTION__);
   if (id && QString(id).compare(m_UserId) == 0) {
      TRACE6("VhallInteractive::RemoveRenderWidge  remvew myself\n");
      m_pSpeakerList->OnLeaveActiveSuc(true);
      m_SDK->StopPushStream();
   }
   if (m_pUserMananger->GetLockUserView().compare(QString(id).toStdString()) == 0) {
      m_pUserMananger->SetLockUserView("");
   }
   SetTopWidgetShowState();
   TRACE6("Leave Function:%s \n", __FUNCTION__);
}

void VhallInteractive::EnableRenderWidgetCamera(const char *id, bool bEnable) {
   QString identifier = id;
   if (identifier == "") {
      identifier = m_UserId;
   }

   CVideoRenderWdg *render = GetVideoRender(identifier);
   if (render) {
      render->EnableCamera(bEnable);
   }

   if (m_pSpeakerList) {
      m_pSpeakerList->SetCameraIcon(id, bEnable);
   }
}

void VhallInteractive::EnableRenderWidgetScreen(const char *id, bool bEnable) {
   QString identifier = id;
   if (identifier == "") {
      identifier = m_UserId;
   }

   CVideoRenderWdg *render = GetVideoRender(identifier);
   if (render) {
      render->EnableScreen(bEnable);
      render->SetScreenEnableIcon(bEnable);
   }
}

CVideoRenderWdg *VhallInteractive::GetVideoRender(VhallHorListWidgetKey id) {
   QString identifier = id;
   if (identifier == "") {
      identifier = m_UserId;
   }

   CVideoRenderWdg *render = NULL;
   QWidget *w = m_pHorWidget->GetRenderWidget(identifier);
   if (w) {
      VhallRenderWidget *wRender = dynamic_cast<VhallRenderWidget *>(w);
      if (wRender) {
         render = dynamic_cast<CVideoRenderWdg *>(wRender->GetWidget());
      }
   }
   return render;
}

void VhallInteractive::Slot_MenuTriggered(QAction *action) {
   qDebug() << "VhallInteractive::Slot_MenuTriggered";
   if (action == m_pActionMic) {
      Slot_DealMic(true);
   } else if (action == m_pActionCamera) {
      Slot_DealCamera(true);
   } else if (action == m_pLockView) {
      Slot_DealLockView(true);
   } else if (action == m_pActionInviteUser) {
      Slot_DealInviteUser(true);
   }
}

void VhallInteractive::ShowRightMouseClickedUserMenu(const QString& id, const QString& role, const int type,const bool centerWnd) {
   if (role.compare("host") == 0 || !mbMaster) {
      return;
   }
   //存在插播和屏幕共享时，大画面右键属性被屏蔽。
   if ((IsPlayMediaFile() || IsExistScreenShare()) && centerWnd) {
      return;
   }

   m_pUserMenu->clear();
   m_pActionMic = NULL;
   m_pActionCamera = NULL;
   m_pActionInviteUser = NULL;
   m_pLockView = NULL;

   if (m_pUserMenu) {
      if (type == SELECT_FROM_USER_LIST) {
         m_pActionInviteUser = new QAction(m_pUserMenu);
         if (m_pActionInviteUser) {
            connect(m_pActionInviteUser, SIGNAL(triggered(bool)), this, SLOT(Slot_DealInviteUser(bool)));
         }
         if (m_pUserMananger && m_pUserMananger->IsVhallLiveLoginUser(id.toStdString().c_str())) {
            m_selectUserID = id;
            if (m_pUserMananger->IsAllowInviteJoinActive(id.toStdString().c_str()) && m_SDK->GetStreamState()) {
               m_pActionInviteUser->setText(INVITE_USER_JOIN);
               m_pUserMenu->addAction(m_pActionInviteUser);
               m_pUserMenu->SetPixmap(":/interactivity/menuAskSpeak", 39, 26, 30, 30);
               m_pUserMenu->show();
               m_pUserMenu->move(QPoint(QCursor::pos().x() - 39, QCursor::pos().y() - 13));
            }
         }
      } else if (type == SELECT_FROM_RENDER_WND && mbMaster) {
         m_pActionMic = new QAction(m_pUserMenu);
         if (m_pActionMic) {
            connect(m_pActionMic, SIGNAL(triggered(bool)), this, SLOT(Slot_DealMic(bool)));
         }
         m_pActionCamera = new QAction(m_pUserMenu);
         if (m_pActionCamera) {
            connect(m_pActionCamera, SIGNAL(triggered(bool)), this, SLOT(Slot_DealCamera(bool)));
         }
         m_pLockView = new QAction(m_pUserMenu);
         if (m_pLockView) {
            connect(m_pLockView, SIGNAL(triggered(bool)), this, SLOT(Slot_DealLockView(bool)));
         }

         m_selectUserID = id;
         bool  bIsOpenMic, bIsOpenCamera, bIsOpenDesktopSharing;
         m_pUserMananger->GetActiveUserState(id.toStdString().c_str(), bIsOpenMic, bIsOpenCamera, bIsOpenDesktopSharing);
         if (bIsOpenMic) {
            m_pActionMic->setText(CLOSE_MIC);
         } else {
            m_pActionMic->setText(OPEN_MIC);
         }

         if (bIsOpenCamera) {
            m_pActionCamera->setText(CLOSE_CAMERA);
         } else {
            m_pActionCamera->setText(OPEN_CAMERA);
         }

         if (m_pUserMananger->IsLockUserView() && m_pUserMananger->GetLockUserView() == id.toStdString()) {
            m_pLockView->setText(UNLOCK_USER_VIEW);
         } else {
            m_pLockView->setText(LOCK_USER_VIEW);
         }

         m_pUserMenu->addAction(m_pActionCamera);
         m_pUserMenu->addAction(m_pActionMic);
         m_pUserMenu->addAction(m_pLockView);
         m_pUserMenu->SetPixmap(":/interactivity/menuCloseCamera", 35, 22, 25, 25);

         m_pUserMenu->show();
         m_pUserMenu->move(QPoint(QCursor::pos().x() - 38, QCursor::pos().y() - 14));
      }
   }
}

void VhallInteractive::Slot_DealMic(bool) {
   TRACE6("Enter Function:%s \n", __FUNCTION__);
   if (m_selectUserID.compare(m_UserId) == 0 && mbMaster) {
      if (m_pActionMic->text().compare(CLOSE_MIC) == 0) {
         m_LastSelectMicID = m_SDK->GetCurrentMicId();
         m_SDK->CloseMic();
      } else {
         ReOpenMic();
      }
   } else {
      if (m_pActionMic->text().compare(CLOSE_MIC) == 0) {
         QVariantMap varmap;
         varmap.insert("userAction", (int)AVIMCMD_Multi_Interact_CloseMic);
         QJsonDocument doc;
         doc.setObject(QJsonObject::fromVariantMap(varmap));
         string json = QString(doc.toJson()).toStdString();
         m_SDK->SendC2CCustomCmd(m_selectUserID.toStdString().c_str(), json.c_str());
         TRACE6("%s AVIMCMD_Multi_Interact_CloseMic\n", __FUNCTION__);
      } else {
         QVariantMap varmap;
         varmap.insert("userAction", (int)AVIMCMD_Multi_Interact_OpenMic);
         QJsonDocument doc;
         doc.setObject(QJsonObject::fromVariantMap(varmap));
         string json = QString(doc.toJson()).toStdString();
         m_SDK->SendC2CCustomCmd(m_selectUserID.toStdString().c_str(), json.c_str());
         TRACE6("%s AVIMCMD_Multi_Interact_OpenMic\n", __FUNCTION__);
      }
   }
   TRACE6("Leave Function:%s \n", __FUNCTION__);
}

void VhallInteractive::Slot_DealCamera(bool) {
   TRACE6("Enter Function:%s \n", __FUNCTION__);
   if (m_selectUserID.compare(m_UserId) == 0 && mbMaster) {
      if (m_pActionCamera->text().compare(CLOSE_CAMERA) == 0) {
         m_LastSelectCameraID = m_SDK->GetCurrentCameraId();
         m_SDK->CloseCamera();
      } else {
         ReOpenCamera();
      }
   } else {
      if (m_pActionCamera->text().compare(CLOSE_CAMERA) == 0) {
         QVariantMap varmap;
         varmap.insert("userAction", (int)AVIMCMD_Multi_Interact_CloseCamera);
         QJsonDocument doc;
         doc.setObject(QJsonObject::fromVariantMap(varmap));
         string json = QString(doc.toJson()).toStdString();
         m_SDK->SendC2CCustomCmd(m_selectUserID.toStdString().c_str(), json.c_str());
         TRACE6("%s AVIMCMD_Multi_Interact_CloseCamera\n", __FUNCTION__);
      } else {
         QVariantMap varmap;
         varmap.insert("userAction", (int)AVIMCMD_Multi_Interact_OpenCamera);
         QJsonDocument doc;
         doc.setObject(QJsonObject::fromVariantMap(varmap));
         string json = QString(doc.toJson()).toStdString();
         m_SDK->SendC2CCustomCmd(m_selectUserID.toStdString().c_str(), json.c_str());
         TRACE6("%s AVIMCMD_Multi_Interact_OpenCamera\n", __FUNCTION__);
      }
   }
   ReCheckCameraBtnStatus();
   TRACE6("Leave Function:%s \n", __FUNCTION__);
}

void VhallInteractive::Slot_DealInviteUser(bool) {
   TRACE6("%s Slot_DealInviteUser\n", __FUNCTION__);
   if (m_pActionInviteUser && m_SDK && m_pActionInviteUser->text().compare(INVITE_USER_JOIN) == 0) {
      if (m_pUserMananger && m_pUserMananger->IsMemberEnableInvite(m_selectUserID.toStdString().c_str())) {
         QJsonObject body;
         body["id"] = m_selectUserID;
         SingletonMainUIIns::Instance().reportLog(L"Interaction_InvitationOnLine", eLogRePortK_Interaction_InvitationOnLine, body);
         m_pUserMananger->SendInviteGuestJoin(m_selectUserID.toStdString().c_str());
      }
   }
}

void VhallInteractive::Slot_OnlineEvent(bool bOnline, QString userId, QString userRole, QString name, int type) {
   TRACE6("%s bOnline:%d, id:%s, userRole:%s, name:%s synType:%d\n", __FUNCTION__, 
	   bOnline, userId.toStdString().c_str(), userRole.toStdString().c_str(), name.toStdString().c_str(), type);
   if (!isVisible() || type == e_RQ_UserOnlineList) {
      TRACE6("%s e_RQ_UserOnlineList type:%d\n", __FUNCTION__, type);
      return;
   }   

   if (!bOnline && userRole.compare(QString("guest")) == 0) {
      if (m_SharingUser.compare(userId) == 0 && m_bIsUserSharing) { //屏幕共享者离开
         TRACE6("%s sharing user exit\n", __FUNCTION__);
         Slot_ActiveMemberJoin(userId, false, "");
      }
      else if (mstrPlayMediaUserID.compare(userId) == 0) {  //插播视频用户离线
         RoomNoMediaFileStream(userId.toStdString().c_str());
      }
   }

   if (mbMaster) {
      if (bOnline) { //主持人收到用户上线通知，发送推流状态。
         m_pUserMananger->NotifyHostPushState();
         m_pUserMananger->HostDealEndpointEnterRoom();
      }
      else {
         if (userRole.compare(QString("guest")) == 0) {  //主持人收到嘉宾退出
            Slot_GuestExitRoom(userId, name);
            m_pUserMananger->GuestOffLine(userId.toStdString().c_str());
         }
      }
   }
   else {
      if (bOnline) {
         if (userRole.compare("host") == 0) { //主持在线
            m_pHostExitTimer->stop();
         }
      }
   }
   TRACE6("%s end\n", __FUNCTION__);
}

void VhallInteractive::Slot_KikoutEvent(bool bKikout, QString userId, QString userRole) {
   if (bKikout && mbMaster) {
      m_pUserMananger->KickOutUser(userId.toStdString().c_str());
      if (m_pUserMananger->GetLockUserView().compare(userId.toStdString()) == 0) {
         m_pUserMananger->SetLockUserView("");
      }
   }
}

void VhallInteractive::Slot_GuestJoinActiveSuc(bool suc) {
   TRACE6("Slot_GuestJoinActiveSuc 1\n");
   if (m_pSpeakerList && !this->mbMaster && !suc) {
      TRACE6("Slot_GuestJoinActiveSuc requeset err;\n");
      m_pSpeakerList->OnJoinActiveSuc(suc);
      FadeOutTip(QStringLiteral("上麦请求失败"));
   }

   if (suc) {
      TRACE6("Slot_GuestJoinActiveSuc requeset succ;\n");
      m_SDK->StartPushStream();
      if (m_bOpenCameraState) {
         ReOpenCamera();
      }
      if (m_bOpenMicState) {
         ReOpenMic();
      }
      ReOpenSpeaker();
   }
}

void VhallInteractive::Slot_GuestLeavActiveSuc(bool suc) {
   TRACE6("Slot_GuestLeavActiveSuc ;\n");
   if (m_pSpeakerList && !this->mbMaster && !suc) {
      TRACE6("OnLeaveActiveSuc err ;\n");
      m_pSpeakerList->OnLeaveActiveSuc(suc);
      FadeOutTip(QStringLiteral("下麦请求失败"));
   }
   if (m_SDK && suc) {
      TRACE6("ResetAuthToWatcherUI ;\n");
      ResetAuthToWatcherUI();
      m_pSpeakerList->OnLeaveActiveSuc(suc);
      m_SDK->StopPushStream();
   }
}
void VhallInteractive::SlotRemixEnd(bool ok) {
   TRACE6("%s ok:%d\n", __FUNCTION__, ok);
   if (ok) {
      m_nLastMixStreamStatus = m_nMixStreamStatus;
   } else {
      m_nMixStreamStatus = m_nLastMixStreamStatus;
      QTimer::singleShot(4000, this, SLOT(ReMix()));
   }
}

void VhallInteractive::Slot_ActiveMemberJoin(QString id, bool joined, QString headUrl) {
   TRACE6("Enter Function:%s \n", __FUNCTION__);
   if (joined) {
      if (!mbMaster && id.compare(m_UserId) == 0) {
         TRACE6("%s myself in\n", __FUNCTION__);
         m_pSpeakerList->OnJoinActiveSuc(true);
         m_SDK->StartPushStream();
      }
      CreateRenderWidget(id.toStdString().c_str());
      CVideoRenderWdg *render = GetVideoRender(id);
      if (render) {
         render->SetHeadImageUrl(headUrl);
         QString name = id;
         VH::CComPtr<IVhallRightExtraWidgetLogic> pVhallRightExtraWidget = NULL;
         DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IVhallRightExtraWidget, IID_IVhallRightExtraWidgetLogic, pVhallRightExtraWidget);
         if (pVhallRightExtraWidget != NULL) {
            wchar_t wname[512] = { 0 };
            //if (pVhallRightExtraWidget->GetUserName(id.toUtf8().data(), wname)) 
			{
               name = QString::fromWCharArray(wname);
            }
         }
         render->SetUserName(name);
      }
      m_pSpeakerList->Append(id, id, mbMaster);
      TRACE6("%s done joined\n", __FUNCTION__);
   } else {//离开
      if (id.isEmpty()) {
         id = m_UserId;
      }
      //如果插播用户下线、被踢出、下麦结束插播。
      if (mstrPlayMediaUserID.compare(id) == 0 && IsPlayMediaFile()) {
         SingletonMainUIIns::Instance().PostCRMessage(MSG_MAINUI_VEDIOPLAY_STOPPLAY, NULL, 0);
      }
      TRACE6("VhallInteractive::Slot_ActiveMemberJoin RemoveRenderWidget member %s", id.toStdString().c_str());
      //如果移除的用户为大画面用户，重置大画面图像。
      bool bResetCenterWnd = false;
      CVideoRenderWdg *render = GetVideoRender(id);
      if (render != NULL && m_selectedWidget != NULL && m_selectedWidget == render) {
         bResetCenterWnd = true;
      }
      //如果移除的用户不为锁定用户时，锁定用户不为空，不重置大画面。
      if (!m_pUserMananger->GetLockUserView().empty() && m_pUserMananger->GetLockUserView().compare(id.toStdString()) != 0) {
         TRACE6("!m_pUserMananger->GetLockUserView().empty() %s\n", m_pUserMananger->GetLockUserView().c_str());
         QWidget* widget = m_pHorWidget->GetRenderWidget(QString::fromStdString(m_pUserMananger->GetLockUserView()));
         if (widget != NULL) {
            SelectRender(widget);
         }
         bResetCenterWnd = false;
      }
      if (m_pUserMananger->GetLockUserView().compare(id.toStdString()) == 0) {
         m_pUserMananger->SetLockUserView("");
      }
      m_pSpeakerList->Remove(id);
      RemoveRenderWidget(id.toStdString().c_str());
      if (m_SharingUser.compare(id) == 0 && m_bIsUserSharing) {
         UserStopShareScreen(id);
         bResetCenterWnd = true;
      }
      
      if (!mbMaster && id.compare(m_UserId) == 0) {
         m_pSpeakerList->OnLeaveActiveSuc(true);
         m_SDK->StopPushStream();
      }

      if (bResetCenterWnd) {
         QString id = mbMaster ? m_UserId : QString(m_pUserMananger->GetHostID());
         QWidget* widget = m_pHorWidget->GetRenderWidget(id);
         if (widget != NULL) {
            SelectRender(widget);
         }
      }

      ResetFloatWndState();
   }
   TRACE6("Leave Function:%s \n", __FUNCTION__);
}
//主播开始推流，开始推流 / 停止推流
void VhallInteractive::Slot_HostStartPush(bool push) {
   if (push) {
      if (m_bIsUserSharing && !m_SDK->GetScreenStatus()) {
         m_selectedWidget = GetVideoRender(m_SharingUser);
         if (m_selectedWidget) {
            CVideoRenderWdg *render = dynamic_cast<CVideoRenderWdg *>(m_selectedWidget);
            ResetCenterRenderMemberInfo(render);
         }
         SetCenterRenderViewType(RenderType_Video);
         if (!mbMaster) {
            if (!m_startLiveTimer.isActive()) {
               m_startLiveTimer.start(1000);
               TRACE6("%s,%s,%d:mTimer.Start\n", __FILE__, __FUNCTION__, __LINE__);
            }
         }
      } else if (m_SDK->GetScreenStatus()) {
         SetCenterRenderViewType(RenderType_Screen);
         if (!mbMaster) {
            if (!m_startLiveTimer.isActive()) {
               m_startLiveTimer.start(1000);
               TRACE6("%s,%s,%d:mTimer.Start\n", __FILE__, __FUNCTION__, __LINE__);
            }
         }
      } else if (mRenderType != RenderType_Photo) {
         SetCenterRenderViewType(RenderType_Video);
         if (!mbMaster) {
            if (!m_startLiveTimer.isActive()) {
               m_startLiveTimer.start(1000);
               TRACE6("%s,%s,%d:mTimer.Start\n", __FILE__, __FUNCTION__, __LINE__);
            }
         }
      } else {
         if (!mbMaster) {
            if (!m_startLiveTimer.isActive()) {
               m_startLiveTimer.start(1000);
               TRACE6("%s,%s,%d:mTimer.Start\n", __FILE__, __FUNCTION__, __LINE__);
            }
         }
      }

      m_pHostExitTimer->stop();
      m_pSpeakerList->SetPushState(true);
      m_bStreamPushState = true;
   } else {
      if (!mbMaster) {
         if (m_startLiveTimer.isActive()) {
            TRACE6("%s,%s,%d:mTimer.Stop\n", __FILE__, __FUNCTION__, __LINE__);
            m_startLiveTimer.stop();
            m_startLiveDateTime.setTime_t(0);
         }
      }
      SetCenterRenderViewType(RenderType_Finished);
      if (!mbMaster) {
         if (m_SDK->GetScreenStatus()) {
            Slot_ScreenShared(false);
         }
         OnCloseDevice();
         ResetStateFromFullOrShare();
         m_SDK->StopPushStream();
         m_pSpeakerList->OnLeaveActiveSuc(true);
         m_pHorWidget->Clear();
         m_pSpeakerList->Clear();
         m_selectedWidget = NULL;
         m_SDK->SetPlayerVolumn(0);
         m_SDK->CloseSpeaker();
         ReCheckPlayerBtnStatus();
         m_bIsUserSharing = false;
         m_SharingUser = "";
      }
      m_pSpeakerList->SetPushState(false);
      m_bStreamPushState = false;
   }
}

void VhallInteractive::Slot_HostNotifyGuestCloseCamera(bool bClose) {
   TRACE6("Slot_HostNotifyGuestCloseCamera \n");
   if (bClose && m_SDK->GetCurrentCameraId() != NULL) {
      m_LastSelectCameraID = m_SDK->GetCurrentCameraId();
      m_SDK->CloseCamera();
      TRACE6("Slot_HostNotifyGuestCloseCamera close \n");
   } else {
      ReOpenCamera();
      TRACE6("Slot_HostNotifyGuestCloseCamera open\n ");
   }
}

void VhallInteractive::Slot_HostNotifyGuestCloseMic(bool bClose) {
   TRACE6("Slot_HostNotifyGuestCloseMic \n");
   if (bClose && m_SDK->GetCurrentMicState()) {
      m_LastSelectMicID = m_SDK->GetCurrentMicId();
      m_SDK->CloseMic();
      TRACE6("Slot_HostNotifyGuestCloseMic close\n");
   } else {
      ReOpenMic();
      TRACE6("Slot_HostNotifyGuestCloseMic open\n");
   };
}

void VhallInteractive::Slot_DealLockView(bool) {
   if (m_pLockView->text().compare(LOCK_USER_VIEW) == 0) {
      m_pUserMananger->SetLockUserView(m_selectUserID.toStdString().c_str());
   } else {
      m_pUserMananger->SetLockUserView(NULL);
   }
}

void VhallInteractive::Slot_GuestRecvLockView(QString id, bool bLock) {
   if (!id.isEmpty() && bLock) {
      TRACE6("Slot_GuestRecvLockView \n");
      QWidget *w = m_pHorWidget->GetRenderWidget(id);
      if (w != NULL) {
         if (m_SDK->GetScreenStatus()) {
            SetCenterRenderViewType(RenderType_Screen);
            TRACE6("SetCenterRenderViewType(RenderType_Screen) \n");
            return;
         }
         if (m_bIsUserSharing || IsPlayMediaFile()) {
            return;
         }
         SelectRender(w);
      }
   }
}

void VhallInteractive::Slot_RecvCloseScreenNotify(QString hostID) {
   if (m_SDK && m_SDK->GetScreenStatus()) {
      int ret = this->m_SDK->CloseScreen();
      m_bScreenStatus = false;
      ReCheckScreenBtnStatus();
      m_selectedWidget = GetVideoRender(m_pUserMananger->GetHostID());
      if (m_selectedWidget) {
         CVideoRenderWdg *render = dynamic_cast<CVideoRenderWdg *>(m_selectedWidget);
         ResetCenterRenderMemberInfo(render);
      }
      QVariantMap varmap;
      varmap.insert(UserAction, (int)AVIMCMD_Multi_Interact_HostNotifyCloseScreenResp);
      QJsonDocument doc;
      doc.setObject(QJsonObject::fromVariantMap(varmap));
      string json = QString(doc.toJson()).toStdString();
      m_SDK->SendC2CCustomCmd(hostID.toStdString().c_str(), json.c_str());
   }
}

void VhallInteractive::Slot_RecvCloseScreenResp() {
   TRACE6("VhallInteractive::Slot_RecvCloseScreenResp\n");
   if (m_SDK  && mbMaster && !m_SDK->GetScreenStatus()) {
      Slot_ScreenShared(true);
   }
}
void VhallInteractive::RefreshOnlineList() {
   qDebug() << "VhallInteractive::RefreshOnlineList()";
   if (m_pSpeakerList) {
      m_pSpeakerList->RefreshOnlineList();
   }
   if (m_pHorWidget) {
      m_pHorWidget->RefreshOnlineList();
   }
}

void VhallInteractive::Slot_UpdateMemberMediaState(QString id, bool bMicOpen, bool bCameraOpen, bool bShareOpen) {
   if (!id.isEmpty()) {
      TRACE6("%s user:%s bMicOpen:%d bCameraOpen:%d  bShareOpen:%d bShareOpen\n", __FUNCTION__, id.toStdString().c_str(), bMicOpen, bCameraOpen, bShareOpen);
      if (mbMaster && m_pSpeakerList->IsExistSpeak(id)) {
         m_pSpeakerList->SetCameraIcon(id, bCameraOpen);
         bool bMicStateChange = m_pSpeakerList->SetMicIcon(id, bMicOpen);
         //if (bMicStateChange) {
         //   //如果麦克风状态改变了，也重新进行混流处理。
         //   TRACE6("%s Goto ReMixStream\n", __FUNCTION__);
         //   ReMix();
         //}
      } else if (!mbMaster && m_pSpeakerList->IsExistSpeak(id)) {
         m_pSpeakerList->SetCameraIcon(id, bCameraOpen);
         m_pSpeakerList->SetMicIcon(id, bMicOpen);
      }

      if (bShareOpen && m_SharingUser.compare(id) != 0) {
         if ((!mbMaster && m_bStreamPushState) || mbMaster) {
            OnScreenEnter(id.toStdString().c_str());
            if (m_SDK->GetStreamState() && m_pExternalCaptureTimer) {
               //m_SDK->CloseExternalCapture();
               TRACE6("%s OpenExternalCapture\n", __FUNCTION__);
               //m_pExternalCaptureTimer->stop();
            }
            m_bIsUserSharing = true;
            m_SharingUser = QString(id.toStdString().c_str());
            if (m_SharingUser.compare(m_UserId) != 0 && !m_bIsFullScreen) {
               OnVideoRenderFullScreen(m_pCenterVideoRender);
               ChangeCenterRendUser(m_pCenterVideoRender->GetID(), QString());
            } else if (m_SharingUser.compare(m_UserId) != 0 && m_bIsFullScreen) {
               MoveRenderFromTopWndToFloatWnd();
            }
            SetCenterRenderViewType(RenderType_Video);
            if (m_pCenterVideoRender) {
               m_pCenterVideoRender->Clear(true);
            }
         }
      } else if (m_SharingUser.compare(id) == 0 && !bShareOpen) {
         UserStopShareScreen(id);
      }
      SetCameraState(id, bCameraOpen);
      CVideoRenderWdg *render = GetVideoRender(id);
      if (render != NULL) {
         render->SetMicDisableIcon(!bMicOpen);
         render->SetScreenEnableIcon(bShareOpen);
         if (!bCameraOpen) {
            render->pauseRender();
            EnableRenderWidgetCamera(id.toStdString().c_str(), false);
            render->Clear(true);
            CVideoRenderWdg *slectRender = dynamic_cast<CVideoRenderWdg *>(m_selectedWidget);
            if (slectRender == render && !m_bIsUserSharing && !m_SDK->GetScreenStatus()) {
               QString filePath = render->GetUserImageFilePath();
               QFile imageFile(filePath);
               if (imageFile.open(QIODevice::ReadOnly)) {
                  QByteArray array = imageFile.readAll();
                  mPhotoPixmap.loadFromData(array);
               }
               SetCenterRenderViewType(RenderType_Photo);
            }
         } else {
            OnCameraEnter(id.toStdString().c_str());
            render->recoverRender();
         }
         CVideoRenderWdg *selectRender = dynamic_cast<CVideoRenderWdg *>(m_selectedWidget);
         if (selectRender == render) {
            ResetCenterRenderMemberInfo(selectRender);
         }
      }

      if (QString(id).compare(m_UserId) == 0) {
         //如果设备列表没有设备，摄像头有选中的设备，暂时认为是将设备拔出。
         if (m_SDK->GetCameraCount() == 0 && m_SDK->GetCurrentCameraId() != NULL) {
            m_LastSelectCameraID = m_SDK->GetCurrentCameraId();
            m_bOpenCameraState = false;
            m_pInterActiveCtrlWdg->OnOpenCamera(false);
         }
      }
   }
}

void VhallInteractive::slotChatClicked() {
   if (NULL != m_pExWidget&&m_pExWidget->isVisible()) {
      m_pExWidget->hide();
   } else if (NULL != m_pExWidget) {
      m_pExWidget->show();
   }
}

void VhallInteractive::SetCenterRenderViewType(const int type) {
   qDebug() << "VhallInteractive::SetCenterRenderViewType " << type;
   QPixmap pixmap;
   switch (type) {
   case RenderType_Video:
      if (this->geometry() != QApplication::desktop()->availableGeometry() && m_pHorWidget->GetRenderCount() >= 2 && !m_bIsFullScreen) {
         ui.widget_top->show();
      }
      ui.renderTypeWdg->hide();
      m_centerRenderWidget->show();
      ui.layout_render->setContentsMargins(0, 0, 0, 0);
      break;
   case RenderType_Wait:
      ui.widget_top->hide();
      ui.renderTypeWdg->show();
      ui.renderTypeWdg->SetNoticeType(RenderType_Wait);
      m_centerRenderWidget->hide();
      break;
   case RenderType_Finished:
      ui.widget_top->hide();
      ui.renderTypeWdg->show();
      ui.renderTypeWdg->SetNoticeType(RenderType_Finished);
      m_centerRenderWidget->hide();
      if (!mbMaster) {
         m_startLiveTimer.stop();
         TRACE6("%s,%s,%d:mTimer.Stop\n", __FILE__, __FUNCTION__, __LINE__);
      }
      break;
   case RenderType_MasterExit:
      ui.widget_top->hide();
      ui.renderTypeWdg->show();
      ui.renderTypeWdg->SetNoticeType(RenderType_MasterExit);
      m_centerRenderWidget->hide();
      if (!mbMaster) {
         m_startLiveTimer.stop();
         TRACE6("%s,%s,%d:mTimer.Stop\n", __FILE__, __FUNCTION__, __LINE__);
      }
      break;
   case RenderType_Screen:
      if (this->geometry() != QApplication::desktop()->availableGeometry() && m_pHorWidget->GetRenderCount() >= 2 && !m_bIsFullScreen) {
         ui.widget_top->show();
      }
      ui.renderTypeWdg->show();
      ui.renderTypeWdg->SetNoticeType(RenderType_Screen);
      m_centerRenderWidget->hide();
      break;
   case RenderType_Photo:
      if (this->geometry() != QApplication::desktop()->availableGeometry() && m_pHorWidget->GetRenderCount() >= 2 && !m_bIsFullScreen) {
         ui.widget_top->show();
      }
       
      if (IsPlayMediaFile()) {
         return;
      }
      ui.renderTypeWdg->show();
      ui.renderTypeWdg->SetPhotoPixmap(mPhotoPixmap);
      ui.renderTypeWdg->SetNoticeType(RenderType_Photo);
      m_centerRenderWidget->hide();
      break;
   default:
      break;
   }
   mRenderType = type;
}

void VhallInteractive::Slot_HostExitRoom() {
   if (mAlertDlg) {
      mAlertDlg->reject();
   }
   if (m_pHostExitTimer->isActive()) {
      return;
   }
   m_pHostExitTimer->stop();
   if (m_SDK) {
      ResetAuthToWatcherUI();
   }
   TRACE6("Slot_HostExitRoom \n");
   m_pSpeakerList->SetPushState(false);
   SetCenterRenderViewType(VhallInteractionRenderType::RenderType_MasterExit);
   ResetStateFromFullOrShare();
   if (m_pHorWidget) {
      m_pHorWidget->Clear();
      m_selectedWidget = NULL;
   }
   if (m_pSpeakerList) {
      m_pSpeakerList->Clear();
      m_pSpeakerList->OnLeaveActiveSuc(true);
      m_SDK->StopPushStream();
   }
   m_pHostExitTimer->start(MAX_HOST_OFFLINE_TIMEOUT);
}

void VhallInteractive::ReOpenMic() {
   bool bHasLastDev = false;
   int count = m_SDK->GetMicCount();
   for (int i = 0; i < count; i++) {
      const char *id = m_SDK->GetMicId(i);
      if (id != NULL && m_LastSelectMicID.compare(QString(id)) == 0) {
         bHasLastDev = true;
         break;
      }
   }

   if (bHasLastDev) {
      m_SDK->OpenMic(m_LastSelectMicID.toStdString().c_str());
      //m_bOpenMicState = true;
   } else {
      int count = m_SDK->GetMicCount();
      if (count > 0) {
         const char *id = m_SDK->GetMicId(0);
         if (id != NULL) {
            m_SDK->OpenMic(id);
         }
      }
   }
}

void VhallInteractive::ReOpenSpeaker() {
   bool bHasLastDev = false;
   int count = m_SDK->GetSpeakerCount();
   for (int i = 0; i < count; i++) {
      const char *id = m_SDK->GetSpeakerId(i);
      if (id != NULL && m_LastSelectSpeakerID.compare(QString(id)) == 0) {
         bHasLastDev = true;
         break;
      }
   }

   if (bHasLastDev) {
      m_SDK->OpenSpeaker(m_LastSelectSpeakerID.toStdString().c_str());
   } else {
      int count = m_SDK->GetSpeakerCount();
      if (count > 0) {
         const char *id = m_SDK->GetSpeakerId(0);
         if (id != NULL) {
            m_SDK->OpenSpeaker(id);
         }
      }
   }
}

void VhallInteractive::SetShrink(bool ok) {
   if (ok) {
      ui.widget_right->hide();
      if (!m_bIsFullScreen) {
         this->setMinimumWidth(MAINWINDGETWIDTH);
         this->setMaximumWidth(MAINWINDGETWIDTH);
         this->setMinimumWidth(MAINWINDGETWIDTH);
         this->setMaximumWidth(MAINWINDGETWIDTH);
      }
      m_toolChatBtn->changeImage(":/extern/08.png");
   } else {
      if (!m_bIsFullScreen) {
         TRACE6("%s widget_right->show()\n",__FUNCTION__);
         ui.widget_right->show();
         m_pExWidget->show();
         this->setMinimumWidth(MAINWINDGETWIDTH + 300);
         this->setMaximumWidth(MAINWINDGETWIDTH + 300);
         this->setMinimumWidth(MAINWINDGETWIDTH + 300);
         this->setMaximumWidth(MAINWINDGETWIDTH + 300);
      }
      m_toolChatBtn->changeImage(":/extern/09.png");
   }
   if (!m_bIsFullScreen) {
      QRect rect = this->geometry();
      rect.setWidth(!ok ? MAINWINDGETWIDTH + 300 : MAINWINDGETWIDTH);
      this->setGeometry(rect);
      this->resize(rect.size());
   }
}

void VhallInteractive::MouseMovedExtraWidget() {
   if (!m_toolChatBtn || m_bIsFullScreen) {
      return;
   }

   QPoint cursorPos = this->mapFromGlobal(this->cursor().pos());
   if (cursorPos.x() < MAINWINDGETWIDTH - 40 || cursorPos.x() > MAINWINDGETWIDTH) {
      m_toolChatBtn->hide();
   } else {
      m_toolChatBtn->show();
      m_toolChatBtn->raise();
      m_toolChatBtn->move(ui.widgetRender->width() - m_toolChatBtn->width(), (ui.widgetRender->height() - m_toolChatBtn->height()) / 2);
   }
}

void VhallInteractive::ReOpenCamera() {
   bool bHasLastDev = false;
   int count = m_SDK->GetCameraCount();
   for (int i = 0; i < count; i++) {
      const char *id = m_SDK->GetCameraId(i);
      if (id != NULL && m_LastSelectCameraID.compare(QString(id)) == 0) {
         bHasLastDev = true;
         break;
      }
   }

   if (bHasLastDev) {
      m_SDK->CloseExternalCapture();
      TRACE6("%s CloseExternalCapture\n", __FUNCTION__);
      m_pExternalCaptureTimer->stop();
      m_SDK->OpenCamera(m_LastSelectCameraID.toStdString().c_str());
   } else {
      int count = m_SDK->GetCameraCount();
      if (count > 0) {
         const char *id = m_SDK->GetCameraId(0);
         if (id != NULL) {
            m_SDK->CloseExternalCapture();
            TRACE6("%s CloseExternalCapture\n", __FUNCTION__);
            m_pExternalCaptureTimer->stop();
            m_SDK->OpenCamera(id);
         }
      }
   }
}
void  VhallInteractive::FadeOutTip(QString str) {
   if (ui.widget_top->isHidden()) {
      m_pFadeOutTipWidget->SetSize(ui.widgetRender->width() + 1, 34);
      m_pFadeOutTipWidget->Tip(this,ui.widgetRender->mapTo(this, QPoint(-1, -1)), str);
   } else {
      m_pFadeOutTipWidget->SetSize(ui.widget_top->width() + 1, 34);
      m_pFadeOutTipWidget->Tip(this,ui.widget_top->mapTo(this, QPoint(-1, -1)), str);
   }
   m_pFadeOutTipWidget->rePos(this);
}

void VhallInteractive::ReCheckCameraBtnStatus() {
   if (m_SDK->GetCurrentCameraId() == NULL) {
      if (m_pInterActiveCtrlWdg) {
         m_pInterActiveCtrlWdg->OnOpenCamera(false);
      }
   } else {
      if (m_pInterActiveCtrlWdg) {
         m_pInterActiveCtrlWdg->OnOpenCamera(true);
      }
   }
}

void VhallInteractive::Slot_HostNotifyAllActiveMember(QString activeMembersJson) {
   QList<QString> activeUsers;
   QByteArray byte_array = activeMembersJson.toUtf8();
   QJsonArray json_array = QJsonDocument::fromJson(byte_array).array();
   for (int i = 0; i < json_array.size(); i++) {
      QString userid;
      QJsonObject obj = json_array.at(i).toObject();
      QVariantMap varmap = obj.toVariantMap();
      if (varmap.contains(MemberID)) {
         userid = varmap.value(MemberID).toString();
         activeUsers.push_back(userid);
      }
   }

   if (m_pSpeakerList) {
      QList<QString> speakers = m_pSpeakerList->GetSpeakListUsers();
      for (int i = 0; i < speakers.size(); i++) {
         bool bFind = false;
         for (int j = 0; j < activeUsers.size(); j++) {
            if (speakers[i] == activeUsers[j]) {
               bFind = true;
            }
         }
         if (!bFind) {
            TRACE6("Slot_HostNotifyAllActiveMember remove speakers %s\n", speakers[i].toStdString().c_str());
            m_pSpeakerList->Remove(speakers[i]);
            TRACE6("%s Goto ReMixStream\n", __FUNCTION__);
            ReMix();
            if (speakers[i].compare(m_UserId) == 0) {
               m_pSpeakerList->OnLeaveActiveSuc(true);
               m_SDK->StopPushStream();
            }
            if (m_pUserMananger->GetLockUserView().compare(m_UserId.toStdString()) == 0) {
               m_pUserMananger->SetLockUserView("");
            }
         }
      }
   }

   if (m_pHorWidget) {
      QList<QString> renders = m_pHorWidget->GetRenderMembers();
      for (int i = 0; i < renders.size(); i++) {
         bool bFind = false;
         for (int j = 0; j < activeUsers.size(); j++) {
            if (renders[i] == activeUsers[j]) {
               bFind = true;
            }
         }
         if (!bFind) {
            TRACE6("Slot_HostNotifyAllActiveMember remove renders %s\n", renders[i].toStdString().c_str());
            if (m_pFloatRenderWdg) {
               m_pFloatRenderWdg->RemoveRender(renders[i]);
               ResetFloatWndState();
            }
            m_pHorWidget->Remove(renders[i]);
         }
      }
   }
}

void VhallInteractive::Slot_GuestSendJoinActiveMsgErr() {
   if (m_pSpeakerList && !this->mbMaster) {
      TRACE6("Slot_GuestSendJoinActiveMsgErr m_pSpeakerList->OnJoinActiveSuc(false)\n");
      m_pSpeakerList->OnJoinActiveSuc(false);
   }
   if (m_SDK) {
      RemoveRenderWidget(m_UserId.toStdString().c_str());
      ResetFloatWndState();
      m_SDK->StopPushStream();
   }
   if (!mbMaster) {
      FadeOutTip(QStringLiteral("网络异常，请求发送失败"));
   }
}

void VhallInteractive::Slot_GuestRefuseHostResp(QString sender) {
   VH::CComPtr<IVhallRightExtraWidgetLogic> pVhallRightExtraWidget = NULL;
   QString name = sender;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IVhallRightExtraWidget, IID_IVhallRightExtraWidgetLogic, pVhallRightExtraWidget);
   if (pVhallRightExtraWidget != NULL) {
      wchar_t wname[512] = { 0 };
      //if (pVhallRightExtraWidget->GetUserName(sender.toUtf8().data(), wname))
	  {
         name = QString::fromWCharArray(wname);
      }
   }
   FadeOutTip(QStringLiteral("%1拒绝了您的邀请").arg(name));
}

void VhallInteractive::Slot_HostOfflineEvent() {
   if (m_pHostExitTimer) {
      m_pHostExitTimer->stop();
   }
   emit SigMasterExitRoom();
}


void VhallInteractive::Slot_CameraBtnClicked() {
   QJsonObject body;
   if (!IsEnableClicked()) {
      return;
   }
   if (!m_SDK->GetCurrentCameraState()) {
      ReOpenCamera();
      body["vb"] = "Open";
   } else {
      m_LastSelectCameraID = m_SDK->GetCurrentCameraId();
      m_SDK->CloseCamera();
      body["vb"] = "Close";
   }

   SingletonMainUIIns::Instance().reportLog(L"Interaction_Camera_op", eLogRePortK_Interaction_Camera, body);
}
void VhallInteractive::Slot_PlayerButtonClicked() {
   if (!m_SDK->GetCurrentPlayerState()) {
      return;
   }
   qDebug() << "VhallInteractive::Slot_SpeakerButtonClicked";
   mPlayerVolumn->StopClose();
   QJsonObject body;

   int v = m_SDK->GetPlayerVolumn();
   if (v < 0) {
      v = 0;
      body["vb"] = "Close";
   } else if (v > 100) {
      v = 100;
      body["vb"] = "Open";
   }
   if (!m_bIsUserSharing && !m_SDK->GetScreenStatus()) {
      mPlayerVolumn->Show(v, m_pInterActiveCtrlWdg->GetPlayBtnGlobalPos());
   }

   SingletonMainUIIns::Instance().reportLog(L"Interaction_Player", eLogRePortK_Interaction_Player, body);
   m_SDK->SetPlayerMutex();
   ReCheckPlayerBtnStatus();
}

void VhallInteractive::Slot_PlayerButtonEnter() {
   qDebug() << "VhallInteractive::Slot_SpeakerButtonEnter";
   mPlayerVolumn->StopClose();
   bool isOpenPlayer = m_SDK->GetCurrentPlayerState();
   int v = m_SDK->GetPlayerVolumn();
   if (v < 0) {
      v = 0;
   } else if (v > 100) {
      v = 100;
   }
   if (!isOpenPlayer) {
      v = 0;
   }
   qDebug() << "VhallInteractive::Slot_SpeakerButtonEnter m_pSpeakerVolumn" << v;
   if (!m_bIsUserSharing && !m_SDK->GetScreenStatus()) {
      mPlayerVolumn->Show(v, m_pInterActiveCtrlWdg->GetPlayBtnGlobalPos());
   }
}
void VhallInteractive::Slot_PlayerButtonLeave() {
   qDebug() << "VhallInteractive::Slot_SpeakerButtonLeave";
   mPlayerVolumn->WillClose(200);
}
void VhallInteractive::Slot_PlayerVolumnChanged(int volume) {
   if (m_SDK->GetSpeakerCount() == 0) {
      return;
   }
   if (!m_SDK->GetCurrentPlayerState()) {
      ReOpenSpeaker();
   }
   if (m_SDK) {
      m_SDK->SetPlayerVolumn(volume);
   }
   ReCheckPlayerBtnStatus();
}

void VhallInteractive::ReCheckPlayerBtnStatus() {
   int v = m_SDK->GetPlayerVolumn();
   if (v == 0) {
      if (m_pScreenShareToolWgd) {
         m_pScreenShareToolWgd->SetPlayerState(false);
      }
      if (m_pInterActiveCtrlWdg) {
         m_pInterActiveCtrlWdg->OnOpenPlayer(false);
      }
   } else {
      if (m_pScreenShareToolWgd) {
         m_pScreenShareToolWgd->SetPlayerState(true);
      }
      if (m_pInterActiveCtrlWdg) {
         m_pInterActiveCtrlWdg->OnOpenPlayer(true);
      }
   }
   mPlayerVolumn->SetVolumn(v);
}

void VhallInteractive::Slot_MicButtonClicked() {
   if (!IsEnableClicked()) {
      return;
   }
   CloseMicAndSetVolume();
}

void VhallInteractive::Slot_MicButtonEnter() {
   if (!mbMaster && !m_SDK->GetStreamState()) {
      return;
   }
   m_pMicVolumn->StopClose();
   int v = m_SDK->GetMicVolumn();
   if (v < 0) {
      v = 0;
   } else if (v > 100) {
      v = 100;
   }
   if (!m_bIsUserSharing && !m_SDK->GetScreenStatus()) {
      m_pMicVolumn->Show(v, m_pInterActiveCtrlWdg->GetMicBtnGlobalPos());
   }
}

void VhallInteractive::Slot_MicButtonLeave() {
   m_pMicVolumn->WillClose(200);
}

void VhallInteractive::ReCheckMicBtnStatus() {
   int v = m_SDK->GetMicVolumn();
   if (v == 0) {
      if (m_pInterActiveCtrlWdg) {
         m_pInterActiveCtrlWdg->OnOpenMic(false);
      }
   } else {
      if (m_pInterActiveCtrlWdg) {
         m_pInterActiveCtrlWdg->OnOpenMic(true);
      }
   }
   m_pMicVolumn->SetVolumn(v);
}
void VhallInteractive::ReCheckScreenBtnStatus() {
   if (!m_bScreenStatus) {
      if (m_pInterActiveCtrlWdg) {
         m_pInterActiveCtrlWdg->OnOpenScreenShare(false);
      }
      SetCenterRenderViewType(RenderType_Video);
   } else {
      if (m_pInterActiveCtrlWdg) {
         m_pInterActiveCtrlWdg->OnOpenScreenShare(true);
      }
   }
}

void VhallInteractive::Slot_MicVolumnChanged(int v) {
   if (m_SDK) {
      m_SDK->SetMicVolumn(v);
   }
   if (v == 0) {
      m_LastSelectMicID = m_SDK->GetCurrentMicId();
      m_SDK->CloseMic();
   } else if (v > 0) {
      if (!m_SDK->GetCurrentMicState()) {
         ReOpenMic();
      }
   }
}

bool VhallInteractive::IsEnableClicked() {
   if (!mbMaster && !m_SDK->GetStreamState()) {
      FadeOutTip(QStringLiteral("进行上麦操作后可使用此功能"));
      return false;
   }
   return true;
}

void SendExitRoomErr(int code, const char* desc, void* data) {
   TRACE6("guest send exit room err\n");
}

void VhallInteractive::Slot_GuestExitRoom(QString id, QString name) {
   QString noticeName = id;
   if (!name.isEmpty()) {
      noticeName = name;
   }
   RemoveUserCameraState(id);
   FadeOutTip(QStringLiteral("嘉宾%1已退出直播").arg(noticeName));
}

void VhallInteractive::Slot_SendMsgErr(QString data) {
   if (!data.isEmpty()) {
      FadeOutTip(data);
   } else {
      FadeOutTip(QStringLiteral("网络异常，请求发送失败"));
   }
}

void VhallInteractive::Slot_HostKickOutUser() {
   ResetAuthToWatcherUI();
   ExitToActiveListWnd(reason_kickOut);
}

void VhallInteractive::ResetAuthToWatcherUI() {
   m_LastSelectCameraID = m_SDK->GetCurrentCameraId();
   m_LastSelectMicID = m_SDK->GetCurrentMicId();
   if (m_SDK->GetScreenStatus()) {
      m_SDK->CloseScreen();
   }
   if (m_SDK->GetCurrentMicState()) {
      m_SDK->CloseMic();
   }
   if (m_SDK->GetCurrentCameraState()) {
      m_SDK->CloseCamera();
   }
   m_SDK->StopPushStream();
}

void VhallInteractive::Slot_OnRenderDrawImage(QString id) {
   //如果当前小窗窗口已经停止摄像头，则画头像。
   CVideoRenderWdg *render = GetVideoRender(id);
   if (render->GetPauseState()) {
      render->Clear(true);
   }
   //如果小窗与选中的窗体相同，则更新大窗内效果。
   CVideoRenderWdg *selectRender = dynamic_cast<CVideoRenderWdg *>(m_selectedWidget);
   if (render && selectRender && selectRender == render) {
      QWidget *w = m_pHorWidget->GetRenderWidget(id);
      if (w) {
         Slot_WidgetClicked(w);
      }
   }
}
void VhallInteractive::OnVideoRenderFullScreen(CVideoRenderWdg* pRender) {
   if (pRender != NULL && !pRender->IsShowFull()) {
      ui.renderTypeWdg->SetIsFullState(true);
      m_pInterActiveCtrlWdg->SetFullState(true);
      pRender->SetFullState(true);
      ui.widget_top->hide();
      ui.widget_title->hide();
      ui.widget->hide();
      ui.widget_right->hide();
      ui.widget_tool->hide();
      if (m_pHideToolWidgetTimer) {
         m_pHideToolWidgetTimer->stop();
         m_pHideToolWidgetTimer->start(HIDCTRLWDG_TIMEOUT);
      }
      m_bIsFullScreen = true;
      ResetToFullScreenSize();
      MoveRenderFromTopWndToFloatWnd();
      ui.horizontalLayout->removeWidget(m_pInterActiveCtrlWdg);
      this->setFocus();
   }
   if (m_pCenterVideoRender) {
      m_pCenterVideoRender->Clear(true);
   }
}
void VhallInteractive::OnExitVideoRenderFullScreen(CVideoRenderWdg* pRender, bool bIsLiveStop /*= false*/) {
   TRACE6("Enter Function:%s \n", __FUNCTION__);
   if (pRender != NULL && pRender->IsShowFull()) {
      ui.renderTypeWdg->SetIsFullState(false);
      m_pInterActiveCtrlWdg->SetFullState(false);
      if (NULL != m_pAnimation && m_pAnimation->state()) {
         m_pAnimation->stop();
      }

      if (NULL != m_pVedioPlayUiAnmt && m_pVedioPlayUiAnmt->state()) {
         m_pVedioPlayUiAnmt->stop();
      }

      m_pInterActiveCtrlWdg->ShowTop(false);
      ui.horizontalLayout->addWidget(m_pInterActiveCtrlWdg);
      MoveRenderFromFloatWndToTopWnd();
      //其他人屏幕共享或插播视频时，退出全屏在顶端要显示小窗
      if ((m_bIsUserSharing && !m_SDK->GetScreenStatus()) || IsPlayMediaFile()) {
         ChangeCenterRendUser(m_pCenterVideoRender->GetID(), QString());
         ui.widget_top->show();
      }

      if (m_pHideToolWidgetTimer) {
         m_pHideToolWidgetTimer->stop();
      }
      pRender->SetFullState(false);
      m_bIsFullScreen = false;
      if (!bIsLiveStop) {
         SetTopWidgetShowState();
      }
      ui.widget_title->show();
      ui.widget->show();
      ui.widget_right->show();
      ui.widget_tool->show();

      ResetToNormalSize();
      SetShrink(false);
   }
   if (m_pCenterVideoRender && IsExistScreenShare()) {
      m_pCenterVideoRender->Clear(true);
   }
   TRACE6("Leave Function:%s \n", __FUNCTION__);
}

void VhallInteractive::SelectRender(QWidget *w) {
   qDebug() << "Slot_WidgetClicked";
   QJsonObject body;

   VhallRenderWidget *vw = dynamic_cast<VhallRenderWidget *>(w);
   if (vw) {
      QString showCenterUserID, hideUserID;
      if (m_selectedWidget) {
         CVideoRenderWdg *render = dynamic_cast<CVideoRenderWdg *>(m_selectedWidget);
         if (render) {
            showCenterUserID = render->GetID();
         }
      }
      m_selectedWidget = vw->GetWidget();
      CVideoRenderWdg *render = dynamic_cast<CVideoRenderWdg *>(m_selectedWidget);
      if (render) {
         hideUserID = render->GetID();
         body["id"] = hideUserID;
         ChangeCenterRendUser(showCenterUserID, hideUserID);
         ResetCenterRenderMemberInfo(render);

         QString headImage = render->GetUserImageUrl();
         CVideoRenderWdg *centerRender = dynamic_cast<CVideoRenderWdg *>(m_centerRenderWidget);
         if (centerRender) {
            bool bLoadSuc = false;
            QString filePath = render->GetUserImageFilePath();
            QFile imageFile(filePath);
            if (imageFile.open(QIODevice::ReadOnly)) {
               QByteArray array = imageFile.readAll();
               bLoadSuc = mPhotoPixmap.loadFromData(array);
            }
         }
         //如果选中的用户没有摄像头或者视频流接收超时。大图显示选中者头像
         if (!GetCameraState(hideUserID) || !render->IsFrameRender()) {
            SetCenterRenderViewType(RenderType_Photo);
         } else {
            SetCenterRenderViewType(RenderType_Video);
         }
      }

      SingletonMainUIIns::Instance().reportLog(L"HorListWidget_clicked", eLogRePortK_Interaction_HorListWidget_clicked, body);
   }
}

void VhallInteractive::Slot_ChangetoLiveGuest() {
   TRACE6("%s\n", __FUNCTION__);
   OnOpenDevice();
}

void VhallInteractive::Slot_ChangetoWatcherGuest() {
   TRACE6("%s\n", __FUNCTION__);
   OnCloseDevice();
}

bool VhallInteractive::GetIsMaster() {
   return mbMaster;
}

void VhallInteractive::ExitToActiveListWnd(int reason) {
   STRU_MAINUI_CLICK_CONTROL loControl;
   loControl.m_eType = control_CloseApp;
   loControl.m_dwExtraData = (DWORD)this;
   loControl.m_eReason = (enum_exit_reason)reason;
   TRACE6("%s exit reason:%d\n", __FUNCTION__, reason);
   SingletonMainUIIns::Instance().PostCRMessage(MSG_MAINUI_CLICK_CONTROL, &loControl, sizeof(STRU_MAINUI_CLICK_CONTROL));

   if (!mbMaster) {
      m_pHostExitTimer->stop();
      SendUserJoinRoomState(AVIMCMD_ExitLive);
   } 
}
void VhallInteractive::CloseWaiting() {
   m_pExitWaiting->Close();
}

void VhallInteractive::ShowWaiting() {
   m_pExitWaiting->Show();
}

void VhallInteractive::SendUserJoinRoomState(int state) {
   TRACE6("%s state %d\n", __FUNCTION__, state);
   if (state == AVIMCMD_EnterLive) {
      QVariantMap varmap;
      varmap.insert("userAction", (int)AVIMCMD_EnterLive);
      varmap.insert("actionParam", m_UserId);
      QJsonDocument doc;
      doc.setObject(QJsonObject::fromVariantMap(varmap));
      string json = QString(doc.toJson()).toStdString();
      m_SDK->SendGroupCustomCmd(json.c_str());
   } else if (state == AVIMCMD_ExitLive) {
      QVariantMap varmap;
      varmap.insert("userAction", (int)AVIMCMD_ExitLive);
      varmap.insert("actionParam", m_UserId);
      QJsonDocument doc;
      doc.setObject(QJsonObject::fromVariantMap(varmap));
      string json = QString(doc.toJson()).toStdString();
      m_SDK->SendGroupCustomCmd(json.c_str());
   } else if (state == AVIMCMD_Host_Leave) {
      QVariantMap varmap;
      varmap.insert("userAction", (int)AVIMCMD_Multi_Interact_HostNotifyExitRoom);
      QJsonDocument doc;
      doc.setObject(QJsonObject::fromVariantMap(varmap));
      string json = QString(doc.toJson()).toStdString();
      m_SDK->SendGroupCustomCmd(json.c_str());
   }
}

void VhallInteractive::OnDeviceCallBack(int devOperateType, int retCode, void* data) {
   TRACE6(" VhallInteractive::OnDeviceCallBack devOperateType :%d code :%d \n", devOperateType, retCode);
   m_pUserMananger->NotifySelfMediaState();
   //腾讯互动错误码：https://github.com/zhaoyang21cn/iLiveSDK_Android_Suixinbo/blob/master/doc/ILiveSDK/avsdkErr.md
   //1003 已处于所要状态
   switch (devOperateType) {
   case ilive::E_DeviceOperationType::E_OpenCamera:{
      if (retCode == 0 || retCode == AV_ERR_HAS_IN_THE_STATE || retCode == ilive::ERR_ALREADY_STATE) {
         m_LastSelectCameraID = m_SDK->GetCurrentCameraId();
         m_bOpenCameraState = true;
         ReCheckCameraBtnStatus();
         if (m_pScreenShareToolWgd) {
            m_pScreenShareToolWgd->SetCameraState(true);
         }
         if (m_SDK->GetExternalCaptureState()) {
            m_SDK->CloseExternalCapture();
            TRACE6("%s CloseExternalCapture\n", __FUNCTION__);
         }
         m_pExternalCaptureTimer->stop();
      }
      else if (retCode == AV_ERR_EXCLUSIVE_OPERATION) {
         m_pExternalCaptureTimer->stop();
         m_SDK->CloseExternalCapture();
         TRACE6("%s CloseExternalCapture\n", __FUNCTION__);
      }
      else if (retCode != 0) {
         FadeOutTip(QString::fromWCharArray(L"打开摄像头失败，请重试或重启客户端。错误码:%1").arg(retCode));
      }
   }
   break;
   case ilive::E_DeviceOperationType::E_CloseCamera:{
      if (retCode == 0 || retCode == AV_ERR_HAS_IN_THE_STATE || retCode == ilive::ERR_ALREADY_STATE) {
         m_bOpenCameraState = false;
         ReCheckCameraBtnStatus();
         if (m_pScreenShareToolWgd) {
            m_pScreenShareToolWgd->SetCameraState(false);
         }
         m_SDK->OpenExternalCapture();
      }
   }
   break;
   case ilive::E_OpenExternalCapture:{
      if (retCode == 0 || retCode == AV_ERR_HAS_IN_THE_STATE || retCode == ilive::ERR_ALREADY_STATE) {
         m_pExternalCaptureTimer->stop();
         m_pExternalCaptureTimer->start(100); //发送自定义采集帧率 10fps
      } else if (retCode == AV_ERR_EXCLUSIVE_OPERATION) {
         TRACE6("%s E_OpenExternalCapture AV_ERR_EXCLUSIVE_OPERATION\n",__FUNCTION__);
      } 
   }
   break;
   case ilive::E_CloseExternalCapture:{
      if (retCode == 0 || retCode == AV_ERR_HAS_IN_THE_STATE || retCode == ilive::ERR_ALREADY_STATE) {
         m_pExternalCaptureTimer->stop();
      }
   }
   break;
   case ilive::E_DeviceOperationType::E_OpenMic:{
      if (retCode == 0 || retCode == AV_ERR_HAS_IN_THE_STATE || retCode == ilive::ERR_ALREADY_STATE) {
         m_LastSelectMicID = m_SDK->GetCurrentMicId();
         m_pInterActiveCtrlWdg->SetMicListBtnEnable(true);
         m_SDK->SetMicVolumn(100);
         m_bOpenMicState = true;
         ReCheckMicBtnStatus();
         if (m_pScreenShareToolWgd) {
            m_pScreenShareToolWgd->SetMicState(true);
         }
      } else if (retCode != 0) {
         FadeOutTip(QString::fromWCharArray(L"打开麦克风失败，请重试或重启客户端。错误码:%1").arg(retCode));
      }
      OpenExternalCapture();
   }
   break;
   case ilive::E_DeviceOperationType::E_CloseMic:{
      if (retCode == 0 || retCode == AV_ERR_HAS_IN_THE_STATE || retCode == ilive::ERR_ALREADY_STATE) {
         m_pInterActiveCtrlWdg->SetMicListBtnEnable(false);
         m_SDK->SetMicVolumn(0);
         m_bOpenMicState = false;
         ReCheckMicBtnStatus();
         if (m_pScreenShareToolWgd) {
            m_pScreenShareToolWgd->SetMicState(false);
         }
      }
      OpenExternalCapture();
   }
   break;
   case ilive::E_DeviceOperationType::E_OpenPlayer:{
      if (retCode == 0 || retCode == AV_ERR_HAS_IN_THE_STATE || retCode == ilive::ERR_ALREADY_STATE) {
         m_LastSelectSpeakerID = m_SDK->GetCurrentSpeakerId();
         m_SDK->SetPlayerVolumn(100);
         if (m_pScreenShareToolWgd) {
            m_pScreenShareToolWgd->SetPlayerState(true);
         }
         if (m_pInterActiveCtrlWdg) {
            m_pInterActiveCtrlWdg->OnOpenPlayer(true);
         }
         if (m_pScreenShareToolWgd) {
            m_pScreenShareToolWgd->SetPlayerState(true);
         }
      } else if (retCode != 0) {
         FadeOutTip(QString::fromWCharArray(L"打开扬声器失败，请重试或重启客户端。错误码:%1").arg(retCode));
      }
   }
   break;
   case ilive::E_DeviceOperationType::E_ClosePlayer:{
      if (retCode == 0 || retCode == AV_ERR_HAS_IN_THE_STATE || retCode == ilive::ERR_ALREADY_STATE) {
         if (m_pScreenShareToolWgd) {
            m_pScreenShareToolWgd->SetPlayerState(false);
         }
         if (m_pInterActiveCtrlWdg) {
            m_pInterActiveCtrlWdg->OnOpenPlayer(false);
         }
         if (m_pScreenShareToolWgd) {
            m_pScreenShareToolWgd->SetPlayerState(false);
         }
      }
   }
   break;
   case ilive::E_DeviceOperationType::E_OpenScreenShare:{
      if (retCode == 0) {
         SetCenterRenderViewType(RenderType_Screen);
         m_bScreenStatus = true;
         ReCheckScreenBtnStatus();
         QRect rect = QApplication::desktop()->availableGeometry();
         if (m_pScreenShareToolWgd) {
            m_pScreenShareToolWgd->SetHostUser(mbMaster);
            m_pScreenShareToolWgd->move((rect.width() - m_pScreenShareToolWgd->width()) / 2, 0);
            m_pScreenShareToolWgd->show();
         }
         MoveRenderFromTopWndToFloatWnd();
         this->hide();
      } else if (retCode == AV_ERR_RESOURCE_IS_OCCUPIED) {
         AlertDlg alert(ALREADY_SCREEN_SHARE, false, NULL);
         alert.CenterWindow(ui.widget_center);
         alert.exec();
      } else if (retCode == AV_ERR_EXCLUSIVE_OPERATION) {
         AlertDlg alert(ALREADY_PLAY_MEDIA_FILE, false, NULL);
         alert.CenterWindow(ui.widget_center);
         alert.exec();
      }
   }
   break;
   case ilive::E_DeviceOperationType::E_CloseScreenShare:{
      if (retCode == 0 || retCode == AV_ERR_HAS_IN_THE_STATE) {
         UserStopShareScreen(m_UserId);
      }
   }
   break;
   case ilive::E_ClosePlayMediaFile:{
      if (!IsPlayMediaFile()) {
         SetTopWidgetShowState();
         m_desktopStreamId.clear();
      }
   }
   break;
   case ilive::E_OpenPlayMediaFile:{
      if (retCode == 0) {
         return;
      } else if (retCode == AV_ERR_RESOURCE_IS_OCCUPIED) {
         SingletonMainUIIns::Instance().PostCRMessage(MSG_MAINUI_VEDIOPLAY_STOPPLAY, NULL, 0);
         AlertDlg alert(ALREADY_SCREEN_SHARE, false, NULL);
         alert.CenterWindow(ui.widget_center);
         alert.exec();
      } else if (retCode == AV_ERR_EXCLUSIVE_OPERATION) {
         VH::CComPtr<IVedioPlayLogic> pVedioPlayLogic = NULL;
         DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IMainUI, IID_IVedioPlayLogic, pVedioPlayLogic, return);
         if (pVedioPlayLogic) {
            pVedioPlayLogic->StopAdmin(true);
            pVedioPlayLogic->ForceHide(true);
            pVedioPlayLogic->StopPlayFile();
         }
         AlertDlg alert(ALREADY_SCREEN_SHARE, false, NULL);
         alert.CenterWindow(ui.widget_center);
         alert.exec();
      }
   }
   break;
   default:
      break;
   }
}

void VhallInteractive::OnCloseDevice() {
   m_LastSelectCameraID = m_SDK->GetCurrentCameraId();
   m_LastSelectMicID = m_SDK->GetCurrentMicId();
   m_SDK->CloseMic();
   m_SDK->CloseCamera();
}

void VhallInteractive::OnOpenDevice() {
   ReOpenCamera(); 
   //如果已经存储插播视频的情况，新上麦用户不能打开麦克风。
   if (!IsPlayMediaFile()) {
      ReOpenMic();
   }
}

void VhallInteractive::ChangeCenterRendUser(QString showUser, QString hideUser) {
   if (showUser.compare(hideUser) == 0) {
      return;
   } else {
      if (m_pHorWidget) {
         m_pHorWidget->ChangeCenterRender(showUser, hideUser);
         if (IsPlayMediaFile() && !m_bIsFullScreen) {
            ShowAllTopUserWnd();
         }
      }
   }
}

void VhallInteractive::ResetCenterRenderMemberInfo(CVideoRenderWdg* selectRender) {
   if (selectRender != NULL && m_pCenterVideoRender != NULL) {
      QString memberID = selectRender->GetID();
      TRACE6("%s. memberID %s\n", __FUNCTION__, memberID.toStdString().c_str());
      m_pCenterVideoRender->SetID(memberID);
   }
}

void VhallInteractive::Slot_OnHideToolWidget() {
   QRect rect = QApplication::desktop()->screenGeometry();
   if (m_pAnimation) {
      m_pAnimation->stop();
      m_pAnimation->setDuration(2000);
      m_pAnimation->setStartValue(QPoint(0, rect.height() - m_pInterActiveCtrlWdg->height()));
      m_pAnimation->setEndValue(QPoint(0, rect.height()));
      m_pAnimation->start();
   }
   if (m_pVedioPlayUiAnmt) {
      if (m_pVedioPlayUiAnmt->state()) {
         m_pVedioPlayUiAnmt->stop();
      }

      m_pVedioPlayUiAnmt->setDuration(2000);
      m_pVedioPlayUiAnmt->setStartValue(QPoint(0, rect.height() - m_pInterActiveCtrlWdg->height() - 5));
      m_pVedioPlayUiAnmt->setEndValue(QPoint(0, rect.height() - 5));
      m_pVedioPlayUiAnmt->start();
   }
   m_bIsHiddingCtrlWdg = true;
   if (m_pHideToolWidgetTimer) {
      m_pHideToolWidgetTimer->stop();
   }
}

void VhallInteractive::keyPressEvent(QKeyEvent *event) {
   switch (event->key()) {
   case Qt::Key_Escape:
      if (m_bIsFullScreen) {
         TRACE6("%s Key_Escape\n", __FUNCTION__);
         OnExitVideoRenderFullScreen(m_pCenterVideoRender);
      }
      break;
   default:
      break;
   }
   QWidget::keyPressEvent(event);
}

void VhallInteractive::MoveRenderFromTopWndToFloatWnd() {
   TRACE6("Enter Function:%s \n", __FUNCTION__);
   int nCount = 0;
   if (m_pHorWidget) {
      const QMap<QString, QWidget*> renderWidgetsMap = m_pHorWidget->GetRenderWidgetsMap();
      QMap<QString, QWidget*>::const_iterator iter = renderWidgetsMap.begin();
      nCount = renderWidgetsMap.count();
      TRACE6("%s TotalCount:%d", __FUNCTION__, nCount);
      while (iter != renderWidgetsMap.end()) {
         QString id = iter.key();
         m_pFloatRenderWdg->AppendRender(id, iter.value());
         if (IsExistScreenShare() || IsPlayMediaFile()) {
            iter.value()->show();
            TRACE6("%s MoveRenderFromFloatWndToTopWnd iter.value()->show() 1 id:%s \n", __FUNCTION__, id.toStdString().c_str());
         } else if (iter.key().compare(m_pCenterVideoRender->GetID()) == 0) {
            iter.value()->hide();
            nCount--;
            TRACE6("%s MoveRenderFromFloatWndToTopWnd iter.value()->hide() 2 id:%s \n", __FUNCTION__, id.toStdString().c_str());
         } else {
            iter.value()->show();
            TRACE6("%s MoveRenderFromFloatWndToTopWnd iter.value()->show() 3 id:%s \n", __FUNCTION__, id.toStdString().c_str());
         }
         iter++;
      }
   }
   ResetFloatWndState();

   const QMap<QString, QWidget*> renderWidgetsMap = m_pFloatRenderWdg->GetRenderWidgetsMap();
   QMap<QString, QWidget*>::const_iterator iter = renderWidgetsMap.begin();
   nCount = renderWidgetsMap.count();
   TRACE6("%s TotalCount:%d", __FUNCTION__, nCount);
   while (iter != renderWidgetsMap.end()) {
      CVideoRenderWdg *render = NULL;
      VhallRenderWidget *wRender = dynamic_cast<VhallRenderWidget *>(iter.value());
      if (wRender) {
         render = dynamic_cast<CVideoRenderWdg *>(wRender->GetWidget());
         if (render && !GetCameraState(render->GetID())) {
            render->Clear(true);
         }
      }
      iter++;
   }
   TRACE6("Leave Function:%s \n", __FUNCTION__);
}

void VhallInteractive::MoveRenderFromFloatWndToTopWnd() {
   TRACE6("Enter Function:%s \n", __FUNCTION__);
   m_pFloatRenderWdg->hide();
   if (m_pHorWidget && m_pCenterVideoRender) {
      QString centerRenderID = m_pCenterVideoRender->GetID();
      TRACE6("%s centerRenderID: %s\n", __FUNCTION__, centerRenderID.toStdString().c_str());
      const QMap<QString, QWidget*> renderWidgetsMap = m_pFloatRenderWdg->GetRenderWidgetsMap();
      QMap<QString, QWidget*>::const_iterator iter = renderWidgetsMap.begin();
      while (iter != renderWidgetsMap.end()) {
         QString id = iter.key();
         m_pHorWidget->Append(id, iter.value());
         if (id.compare(centerRenderID) == 0) {
            iter.value()->hide();
            TRACE6("%s hide render id: %s\n", __FUNCTION__, id.toStdString().c_str());
         } else {
            iter.value()->show();
            TRACE6("%s show render id: %s\n", __FUNCTION__, id.toStdString().c_str());
         }
         iter++;
      }
      m_pFloatRenderWdg->ClearRenderWidgets();
   }
   TRACE6("Leave Function:%s \n", __FUNCTION__);
}


bool VhallInteractive::IsExistScreenShare() {
   if (m_bIsUserSharing || m_SDK->GetScreenStatus()) {
      return true;
   }
   return false;
}

void VhallInteractive::showEvent(QShowEvent *event) {
   m_pInterActiveCtrlWdg->setRecordBtnShow(mpMainUILogic->GetDisplayCutRecord());
   this->setAttribute(Qt::WA_Mapped);
   QWidget::showEvent(event);
}

void VhallInteractive::Slot_MouseEnterCtrlWdg() {
   if (m_pHideToolWidgetTimer && m_bIsFullScreen  && !m_SDK->GetScreenStatus()) {
      if (m_bIsHiddingCtrlWdg) {
         m_pAnimation->stop();
         m_pAnimation->setDuration(900);
         QRect rect = QApplication::desktop()->screenGeometry();
         m_pAnimation->setStartValue(QPoint(0, m_pInterActiveCtrlWdg->pos().ry()));
         m_pAnimation->setEndValue(QPoint(0, rect.height() - m_pInterActiveCtrlWdg->height()));
         m_pAnimation->start();

         if (m_pVedioPlayUiAnmt) {
            if (m_pVedioPlayUiAnmt->state()) {
               m_pVedioPlayUiAnmt->stop();
            }
            m_pVedioPlayUiAnmt->setDuration(900);
            m_pVedioPlayUiAnmt->setStartValue(QPoint(0, m_pInterActiveCtrlWdg->pos().ry() - 5));
            m_pVedioPlayUiAnmt->setEndValue(QPoint(0, rect.height() - m_pInterActiveCtrlWdg->height() - 5));
            m_pVedioPlayUiAnmt->start();
         }

         m_bIsHiddingCtrlWdg = false;
      }
      m_pHideToolWidgetTimer->stop();
      m_pHideToolWidgetTimer->start(HIDCTRLWDG_TIMEOUT);
   }
}

void VhallInteractive::Slot_btnFullScreenClicked() {
   if (m_pCenterVideoRender) {
      OnVideoRenderFullScreen(m_pCenterVideoRender);
      QJsonObject body;
      SingletonMainUIIns::Instance().reportLog(L"Interaction_Max", eLogRePortK_InteractionWdgMax, body);
   }
}

void VhallInteractive::ResetStateFromFullOrShare() {
   if (m_bIsFullScreen && !IsExistScreenShare()) {
      OnExitVideoRenderFullScreen(m_pCenterVideoRender, true);
   } else if (m_bIsFullScreen && IsExistScreenShare()) {
      if (m_pScreenShareToolWgd) {
         m_pScreenShareToolWgd->hide();
      }
      OnExitVideoRenderFullScreen(m_pCenterVideoRender, true);
   } else if (!m_bIsFullScreen && IsExistScreenShare()) {
      if (m_pScreenShareToolWgd) {
         m_pScreenShareToolWgd->hide();
      }
      if (m_pFloatRenderWdg) {
         m_pFloatRenderWdg->hide();
         m_pFloatRenderWdg->ClearRenderWidgets();
      }
   }
}

void VhallInteractive::ResetToNormalSize() {
   this->setMinimumWidth(MAINWINDGETWIDTH + 300);
   this->setMaximumWidth(MAINWINDGETWIDTH + 300);

   QRect rect;
   rect.setWidth(MAINWINDGETWIDTH + 300);
   rect.setHeight(MAINWINDMINHEIGHT);
   this->setGeometry(rect);

   QRect r = QApplication::desktop()->availableGeometry();
   QPoint pos = QPoint((r.width() - (MAINWINDGETWIDTH + 300)) / 2, (r.height() - MAINWINDMINHEIGHT) / 2);
   this->move(pos);
   m_pFadeOutTipWidget->SetSize(730, 34);
   m_pFadeOutTipWidget->rePos(this);
   VH::CComPtr<IVedioPlayLogic> pVedioPlayLogic;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IMainUI, IID_IVedioPlayLogic, pVedioPlayLogic, return, ASSERT(FALSE));
   if (pVedioPlayLogic != NULL) {
      pVedioPlayLogic->ReposVedioPlay(true);
      pVedioPlayLogic->ResetPlayUiSize(730);
   }
}

void VhallInteractive::ResetToFullScreenSize() {
   QRect r = QApplication::desktop()->screenGeometry();
   this->setGeometry(r);
   this->setMinimumWidth(r.width() + 1);
   this->setMaximumWidth(r.width() + 1);
   QRect ctrlWdgRect = m_pInterActiveCtrlWdg->geometry();
   ctrlWdgRect.setWidth(r.width());
   m_pInterActiveCtrlWdg->setGeometry(ctrlWdgRect);
   m_pInterActiveCtrlWdg->ShowTop(true);
   m_pInterActiveCtrlWdg->raise();
   m_pInterActiveCtrlWdg->show();
   QRect rect = QApplication::desktop()->screenGeometry();
   m_pInterActiveCtrlWdg->move(QPoint(0, rect.height() - m_pInterActiveCtrlWdg->height()));
   m_pFadeOutTipWidget->SetSize(r.width(), 34);
   m_pFadeOutTipWidget->rePos(this);
   m_pFadeOutTipWidget->move(0, 0);
   VH::CComPtr<IVedioPlayLogic> pVedioPlayLogic;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IMainUI, IID_IVedioPlayLogic, pVedioPlayLogic, return, ASSERT(FALSE));
   if (pVedioPlayLogic != NULL) {
      pVedioPlayLogic->ReposVedioPlay(true);
      pVedioPlayLogic->ResetPlayUiSize(r.width());
   }
}

void VhallInteractive::OnRoomDisconnect() {
  // TRACE6("%s \n");
   STRU_MAINUI_CLICK_CONTROL loControl;
   loControl.m_eType = control_CloseApp;
   loControl.m_dwExtraData = (DWORD)this;
   loControl.m_bIsRoomDisConnect = true;

   SingletonMainUIIns::Instance().PostCRMessage(
      MSG_MAINUI_CLICK_CONTROL, &loControl, sizeof(STRU_MAINUI_CLICK_CONTROL));

   if (!mbMaster) {
      m_pHostExitTimer->stop();
   }
}

void VhallInteractive::SetActiveTitle(const QString& name, const QString& streamID) {
   ui.label_activeName->clear();
   ui.label_activeName->setStyleSheet("QLabel#label_activeName:QToolTip{color:rgb(128,128,128);}");
   QString activeName = name;
   QString toolTips = QString("%1(ID:%2)").arg(activeName, streamID);
   ui.label_activeName->setToolTip(toolTips);

   if (activeName.length() > 12) {
      activeName = activeName.left(12);
      activeName.append("...");
   }
   QString title = QString("%1(ID:%2)").arg(activeName, streamID);
   ui.label_activeName->setText(title);
}

void VhallInteractive::Slot_OnSendExternalCapture() {
   if (m_SDK) {
      m_SDK->SendExternalFrame("ExternalCapture.bmp", m_UserId.toStdString().c_str());
   }
}

void VhallInteractive::resizeEvent(QResizeEvent *event) {
   if (m_pFadeOutTipWidget && !m_pFadeOutTipWidget->isHidden()) {
      m_pFadeOutTipWidget->resize(ui.widget_center->width(), 34);
      FadeOutTip("");
   }
   return QWidget::resizeEvent(event);
}


bool VhallInteractive::GetCameraState(QString& id) {
   QMap<QString, bool>::iterator iter = m_userCameraStateMap.find(id);
   if (iter != m_userCameraStateMap.end()) {
      return iter.value();
   }
   return false;
}

void VhallInteractive::SetCameraState(QString& id, bool hasCamera) {
   m_userCameraStateMap[id] = hasCamera;
}

void VhallInteractive::RemoveUserCameraState(QString& id) {
   QMap<QString, bool>::iterator iter = m_userCameraStateMap.find(id);
   if (iter != m_userCameraStateMap.end()) {
      m_userCameraStateMap.erase(iter);
   }
}

void VhallInteractive::UserStopShareScreen(const QString &id) {
   TRACE6("Enter Function:%s \n", __FUNCTION__);
   m_bIsUserSharing = false;
   m_SharingUser = "";
   OnScreenExit(id.toStdString().c_str());
   if (m_pScreenShareToolWgd && !m_pScreenShareToolWgd->isHidden()) {
      m_pScreenShareToolWgd->hide();
   }
   if (id.compare(m_UserId) == 0) {
      if (m_bIsFullScreen) {
         MoveRenderFromTopWndToFloatWnd();
      } else {
         MoveRenderFromFloatWndToTopWnd();
         if (m_pHorWidget && m_pHorWidget->GetRenderCount() == 1 && !ui.widget_top->isHidden()) {
            ui.widget_top->hide();
         }
      }
      QString id = QString::fromStdString(m_pUserMananger->GetLockUserView());
      if (id.isEmpty()) {
         id = mbMaster ? m_UserId : m_pUserMananger->GetHostID();
         TRACE6("%s close screen get id:%s\n", __FUNCTION__, id.toStdString().c_str());
      }
      if (!id.isEmpty()) {
         QWidget* widget = m_pHorWidget->GetRenderWidget(id);
         if (widget) {
            SelectRender(widget);
         }
      }
      m_bScreenStatus = false;
      if (!m_bScreenStatus) {
         if (m_pInterActiveCtrlWdg) {
            m_pInterActiveCtrlWdg->OnOpenScreenShare(false);
         }
      } else {
         if (m_pInterActiveCtrlWdg) {
            m_pInterActiveCtrlWdg->OnOpenScreenShare(true);
         }
      }
      this->show();
   } else {
      if (m_bIsFullScreen) {
         OnExitVideoRenderFullScreen(m_pCenterVideoRender);
      } else if (!m_bIsFullScreen) {
         ChangeCenterRendUser(QString(), m_pCenterVideoRender->GetID());
      }
      if (!m_pUserMananger->GetLockUserView().empty()) {
         TRACE6("!m_pUserMananger->GetLockUserView().empty() %s\n", m_pUserMananger->GetLockUserView().c_str());
         QWidget* widget = m_pHorWidget->GetRenderWidget(QString::fromStdString(m_pUserMananger->GetLockUserView()));
         if (widget != NULL) {
            SelectRender(widget);
         }
      }
      if (m_pHorWidget->GetRenderCount() <= 1) {
         ui.widget_top->hide();
      }
      if (mbMaster && !IsExistScreenShare() && (m_CameraListMap.size() == 0 || (m_CameraListMap.size() == 1 && !m_SDK->GetCurrentCameraState())) && m_SDK->GetStreamState()) {
         TRACE6("%s OpenExternalCapture\n", __FUNCTION__);
         //m_SDK->OpenExternalCapture();
      }
   }
   TRACE6("Leave Function:%s \n", __FUNCTION__);
}

void VhallInteractive::Slot_ShowChatWdg() {
   QJsonObject body;
   if (ui.widget_right->isHidden()) {//显示
      SetShrink(false);
      body["ms"] = "Show";
   } else {//隐藏
      SetShrink(true);
      body["ms"] = "Hide";
   }

   SingletonMainUIIns::Instance().reportLog(L"Interaction_DisplayOrHideChat", eLogRePortK_Interaction_RightExtraDisplayOrHide, body);
}

void VhallInteractive::ResetMixStream() {
   QString tstreamId = "8136_" + GetTencentStreamId(m_StreamId, m_UserId, "main");
   QMap<QString, bool>::iterator iter = m_CameraListMap.begin();
   while (iter != m_CameraListMap.end()) {
      if (iter.key().compare(tstreamId) != 0) {
         iter = m_CameraListMap.erase(iter);
      } else {
         iter++;
      }
   }
   m_desktopStreamId = "";
   m_removeStreamID = "";
   QString stopStreamID = m_StreamId + STREAM_TAIL;
   m_StreamMixObj.StopMixStream(stopStreamID);
}

void VhallInteractive::mouseMoveEvent(QMouseEvent *event) {
   if (m_bTitlePressed) {
      int dx = this->cursor().pos().x() - this->pressPoint.x();
      int dy = this->cursor().pos().y() - this->pressPoint.y();
      this->move(this->startPoint.x() + dx, this->startPoint.y() + dy);

      if (m_pFadeOutTipWidget) {
         m_pFadeOutTipWidget->rePos(this);
      }
      MouseMovedExtraWidget();
   } else if (ui.renderTypeWdg->rect().contains(event->pos()) && m_bIsFullScreen) {
      Slot_MouseEnterCtrlWdg();
   }
}

void VhallInteractive::mousePressEvent(QMouseEvent *event) {
   if (ui.widget_title && ui.widget_title->rect().contains(event->pos())) {
      this->pressPoint = this->cursor().pos();
      this->startPoint = this->pos();
      m_bTitlePressed = true;
   } else if (ui.renderTypeWdg->rect().contains(event->pos()) && m_bIsFullScreen) {
      Slot_MouseEnterCtrlWdg();
   }
}

void VhallInteractive::mouseReleaseEvent(QMouseEvent *event) {
   m_bTitlePressed = false;
   if (ui.renderTypeWdg->rect().contains(event->pos()) && m_bIsFullScreen) {
      Slot_MouseEnterCtrlWdg();
   }
}

void VhallInteractive::Slot_ContextCenterNoticeMenuEvent() {
   if (m_pCenterVideoRender && m_pCenterVideoRender->isHidden() && mbMaster) {
      QString id = m_pCenterVideoRender->GetID();
      if (!id.isEmpty()) {
         ShowRightMouseClickedUserMenu(id, "guest", SELECT_FROM_RENDER_WND, true);
      }
   }
}

void VhallInteractive::Slot_SetFullWnd() {
   if (m_pCenterVideoRender) {
      if (m_bIsFullScreen) {
         OnExitVideoRenderFullScreen(m_pCenterVideoRender);
      } else {
         OnVideoRenderFullScreen(m_pCenterVideoRender);
      }
   }
}

void VhallInteractive::ShowLiveTool(bool ok) {
   if (!m_pInterActiveCtrlWdg) {
      return;
   }
   m_pInterActiveCtrlWdg->ShowLiveTool(ok);
}


bool VhallInteractive::IsShareScreen() {
   if (m_pScreenShareToolWgd && !m_pScreenShareToolWgd->isHidden()) {
      return true;
   }
   return false;
}

void VhallInteractive::slotCurRecordState(int curRecordState) {
   if (eRecordState_Stop == m_pInterActiveCtrlWdg->GetRecordState()) {//当前是停止状态, 点击按钮就开始录制
      if (m_SDK&&!m_SDK->GetStreamState()) {//未开启推流
         FadeOutTip(QString::fromWCharArray(L"请先开始直播，再开启录制"));
         return;
      } else {//开始录制
         commitRecord(eRecordReTyp_Start);
      }
   } else {//其它状态 弹出操作选择对话
      if (NULL == mPRecordDlg) {
         mPRecordDlg = new CRecordDlg(this);
         mPRecordDlg->Create();
         connect(mPRecordDlg, &CRecordDlg::sigClicked, this, &VhallInteractive::slotRecordStateChange);
      }
      mPRecordDlg->show();
      QPoint p = m_pInterActiveCtrlWdg->GetRecordBtnPos();
      int x = p.x() - mPRecordDlg->width() / 2 + m_pInterActiveCtrlWdg->GetRecordBtnWidth() / 2;
      int y = p.y() - mPRecordDlg->height() - 8;
      mPRecordDlg->move(x, y);
   }
}

void VhallInteractive::slotRecordStateChange(int iSate) {
   switch (iSate) {
   case eCurOp_Recovery://开始/恢复录制
      commitRecord(eRecordReTyp_Start);
   break;
   default:{//暂停/停止录制
      if (eCurOp_Stop == iSate) {//停止录制
         commitRecord(eRecordReTyp_Stop);
      } else {
         commitRecord(eRecordReTyp_Suspend);
      }
   }
   break;
   }
}

void VhallInteractive::slotStopLive() {
   //如果目前的状态是没有录制或者已经停止录制，则不再触发提交停止录制请求。
   //if (GetRecordState() != eRecordState_Stop) {
   //slotRecordStateChange(eCurOp_Stop);
   //}
}

void VhallInteractive::commitRecord(const int eRequestType) {
   if (m_pInterActiveCtrlWdg->IsRecordBtnhide()) {
      return;
   }

   static QString strURL = ConfigSetting::ReadString(CPathManager::GetConfigPath(), GROUP_DEFAULT, KEY_URL_DOMAIN, "");
   PublishInfo loStreamInfo;
   VH::CComPtr<ICommonData> pCommonData;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
   if (pCommonData) {
      pCommonData->GetStreamInfo(loStreamInfo);
   }

   VH::CComPtr<IVhallRightExtraWidgetLogic> pVhallRightExtraWidget = NULL;
   QString strToken;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IVhallRightExtraWidget, IID_IVhallRightExtraWidgetLogic, pVhallRightExtraWidget);
   if (pVhallRightExtraWidget) {
      char *pMsgToken = pVhallRightExtraWidget->GetMsgToken();
      if (pMsgToken) {
         strToken = QString(pMsgToken);
         delete[]pMsgToken;
      }
   }
   wchar_t *format = L"%s/api/client/v1/clientapi/cut-record?webinar_id=%s&status=%d&send=1&token=%s";// + m_token";

   STRU_HTTPCENTER_HTTP_RS loRQ;
   QString strStreamName = QString(loStreamInfo.mStreamName.c_str());
   swprintf_s(loRQ.m_wzRequestUrl, DEF_MAX_HTTP_URL_LEN, format,
              reinterpret_cast<const wchar_t *>(strURL.utf16()),
              //HLS_POINT_RECORD,
              reinterpret_cast<const wchar_t *>(strStreamName.utf16()),
              eRequestType,
              reinterpret_cast<const wchar_t *>(strToken.utf16())
              );

   SingletonMainUIIns::Instance().PostCRMessage(MSG_MAINUI_HTTP_TASK, &loRQ, sizeof(loRQ));
}

void VhallInteractive::SetPointRecordUrl(QString strUrl) {
   mStrPointRecordUrl = strUrl;
}

void VhallInteractive::SetRecordState(const int iState) {
   m_pInterActiveCtrlWdg->SetRecordState(iState);
   if (NULL != mPRecordDlg) {
      mPRecordDlg->SetRecordState(iState);
   }
}

void VhallInteractive::SetMainUILogic(MainUILogic* pMainUILogic) {
   mpMainUILogic = pMainUILogic;
}

int VhallInteractive::GetRecordState() {
   int iState = eRecordState_Stop;
   if (NULL != m_pInterActiveCtrlWdg) {
      iState = m_pInterActiveCtrlWdg->GetRecordState();
   }
   return iState;
}

void VhallInteractive::Slot_OnPlayFileClicked() {
   if (!IsEnableClicked()) {
      return;
   }
   if (IsExistScreenShare()) {
      AlertDlg alert(ALREADY_SCREEN_SHARE, false, NULL);
      alert.CenterWindow(ui.widget_center);
      alert.exec();
      return;
   } else if (IsPlayMediaFile() && !IsPlayUiShow()) {
      AlertDlg alert(ALREADY_PLAY_MEDIA_FILE, false, NULL);
      alert.CenterWindow(ui.widget_center);
      alert.exec();
      return;
   }
   STRU_MAINUI_CLICK_CONTROL loControl;
   loControl.m_eType = (enum_control_type)(control_VideoSrc);
   loControl.m_dwExtraData = eLiveType_TcActive;
   SingletonMainUIIns::Instance().PostCRMessage(MSG_MAINUI_CLICK_CONTROL, &loControl, sizeof(STRU_MAINUI_CLICK_CONTROL));
}

QWidget* VhallInteractive::GetRenderWidget() {
   return ui.widget_center;
}

int VhallInteractive::GetMediaPlayUIWidth() {
	return ui.widget_center->width();
}

void VhallInteractive::moveEvent(QMoveEvent *event) {
   VH::CComPtr<IVedioPlayLogic> pVedioPlayLogic;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IMainUI, IID_IVedioPlayLogic, pVedioPlayLogic, return, ASSERT(FALSE));
   if (pVedioPlayLogic != NULL) {
      pVedioPlayLogic->ReposVedioPlay(true);
   }
}

bool VhallInteractive::IsPlayMediaFile() {
   VH::CComPtr<IVedioPlayLogic> pVedioPlayLogic;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IMainUI, IID_IVedioPlayLogic, pVedioPlayLogic, return false, ASSERT(FALSE));
   int nPlayFileState = m_SDK->GetPlayFileState();
   bool bIsShowPlayUI = pVedioPlayLogic->IsPlayMediaFileUIShown();
   if ((nPlayFileState == ilive::E_PlayMediaFilePlaying || nPlayFileState == ilive::E_PlayMediaFilePause) && bIsShowPlayUI) {
      return true;
   } else if (mbIsUserPlayMediaFile) {
      return true;
   }
   return false;
}

bool VhallInteractive::IsPlayUiShow() {
   VH::CComPtr<IVedioPlayLogic> pVedioPlayLogic;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IMainUI, IID_IVedioPlayLogic, pVedioPlayLogic, return false, ASSERT(FALSE));
   bool bIsShowPlayUI = pVedioPlayLogic->IsPlayMediaFileUIShown();
   if (bIsShowPlayUI) {
      return true;
   }
   return false;
}

void VhallInteractive::OnPlayMaideExit(const char* identifier) {
   //主持人打开插播文件。
   if (mbMaster) {
      m_desktopStreamId.clear();
      m_removeStreamID.clear();
      TRACE6("%s Goto ReMixStream\n", __FUNCTION__);
      ReMix();
   }

   QString id = QString::fromStdString(m_pUserMananger->GetLockUserView());
   if (id.isEmpty()) {
      id = mbMaster ? m_UserId : m_pUserMananger->GetHostID();
      TRACE6("%s close screen get id:%s\n", __FUNCTION__, id.toStdString().c_str());
   }
   if (!id.isEmpty()) {
      QWidget* widget = m_pHorWidget->GetRenderWidget(id);
      if (widget) {
         SelectRender(widget);
      }
   } 
   if (m_pCenterVideoRender) {
      ChangeCenterRendUser(QString(), m_pCenterVideoRender->GetID());
   }
   if (m_pInterActiveCtrlWdg) {
      m_pInterActiveCtrlWdg->update();
   }
}

void VhallInteractive::OnPlayMediaEnter(const char* id) {
   //主持人打开插播文件。
   if (mbMaster) {
      m_desktopStreamId = "8136_" + GetTencentStreamId(m_StreamId, strlen(id) == 0 ? m_UserId : QString(id), "aux");
      m_removeStreamID = "8136_" + GetTencentStreamId(m_StreamId, strlen(id) == 0 ? m_UserId : QString(id), "main");
      TRACE6("%s Goto ReMixStream\n", __FUNCTION__);
      ReMix();
   }

   ChangeCenterRendUser(m_pCenterVideoRender->GetID(), QString());
   if (m_bIsFullScreen && mbMaster) {
      MoveRenderFromTopWndToFloatWnd();
   }
   if (m_pFloatRenderWdg && !m_pFloatRenderWdg->isHidden() && m_bIsFullScreen) {
      m_pFloatRenderWdg->ResetWndSize(true);
   }
}

void VhallInteractive::CloseMicAndSetVolume() {
   m_pMicVolumn->StopClose();
   int v = m_SDK->GetMicVolumn();
   if (v < 0) {
      v = 0;
   } else if (v > 100) {
      v = 100;
   }
   if (!m_bIsUserSharing && !m_SDK->GetScreenStatus() && !IsPlayMediaFile()) {
      m_pMicVolumn->Show(v, m_pInterActiveCtrlWdg->GetMicBtnGlobalPos());
   }
   QJsonObject body;
   if (m_SDK->GetCurrentMicState()) {
      m_LastSelectMicID = m_SDK->GetCurrentMicId();
      m_SDK->CloseMic();
      body["vb"] = "Close";
   } else {
      ReOpenMic();
      body["vb"] = "Open";
   }
   SingletonMainUIIns::Instance().reportLog(L"Interaction_Mic", eLogRePortK_Interaction_Mic, body);
}

void VhallInteractive::RoomNoMediaFileStream(const char* identifier) {
   TRACE6("%s enter: id:%s\n", __FUNCTION__, identifier);
   mbIsUserPlayMediaFile = false;
   mstrPlayMediaUserID.clear();
   SetTopWidgetShowState();
   OnPlayMaideExit(identifier);
   ResetFloatWndState();
   mLastPlayMediaFileEndTime = QDateTime::currentDateTime().toTime_t();
   TRACE6("%s leave: id:%s\n", __FUNCTION__, identifier);
}

void VhallInteractive::RoomHasMediaFileStream(const char* identifier) {
   TRACE6("%s enter: id:%s\n", __FUNCTION__, identifier);
   //当登陆到互动界面时。有可能会收到媒体流状态，UI判断下如果主持人没有开始直播并且没有开始插播，则不出来。
   if (mbMaster && !m_SDK->GetStreamState()) {
      VH::CComPtr<IVedioPlayLogic> pVedioPlayLogic;
      DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IMainUI, IID_IVedioPlayLogic, pVedioPlayLogic, return, ASSERT(FALSE));
      bool bIsShowPlayUI = pVedioPlayLogic->IsPlayMediaFileUIShown();
      if (!bIsShowPlayUI) {   
         TRACE6("%s return: id:%s\n", __FUNCTION__, identifier);
         return;
      }
   }
   mbIsUserPlayMediaFile = true;
   mstrPlayMediaUserID = QString(identifier);
   SetTopWidgetShowState();   
   if (m_bIsFullScreen) {
      ResetFloatWndState();
   }
   OnPlayMediaEnter(identifier);
   CVideoRenderWdg *centerRender = dynamic_cast<CVideoRenderWdg *>(m_centerRenderWidget);
   centerRender->Clear(true);
   QDateTime currentDt = QDateTime::currentDateTime();
   unsigned int currentTime = currentDt.toTime_t();
   //为了防止上一次文件播放完毕紧接着播放新的文件导致关闭了麦克风，添加时间控制（2秒）。
   if (m_SDK->GetCurrentMicState() && currentTime - mLastPlayMediaFileEndTime > 2) {
      m_LastSelectMicID = m_SDK->GetCurrentMicId();
      m_SDK->CloseMic();
   }
   TRACE6("%s leave: id:%s\n", __FUNCTION__, identifier);
}

void VhallInteractive::ReleaseAlert() {
   if (mAlertDlg) {
      mAlertDlg->reject();
      mAlertDlg->deleteLater();
      mAlertDlg = NULL;
   }
}

void VhallInteractive::ShowAllTopUserWnd() {
   if (m_pHorWidget) {
      const QMap<QString, QWidget*> renderWidgetsMap = m_pHorWidget->GetRenderWidgetsMap();
      QMap<QString, QWidget*>::const_iterator iter = renderWidgetsMap.begin();
      while (iter != renderWidgetsMap.end()) {
         iter.value()->show();
         iter++;
      }
   }
}

void VhallInteractive::ResetFloatWndState() {
   TRACE6("%s enter\n", __FUNCTION__);
   int nShowCount = 0;
   if (m_pFloatRenderWdg) {
      m_pFloatRenderWdg->hide();
      nShowCount = m_pFloatRenderWdg->ResetWndSize(IsExistScreenShare() || IsPlayMediaFile());
      TRACE6("%s nShowCount:%d\n", __FUNCTION__, nShowCount);
      if (nShowCount == 0 && !IsExistScreenShare()) {
         m_pFloatRenderWdg->hide();
         TRACE6("%s float wnd hide\n", __FUNCTION__);
      } else if(nShowCount > 0){
         if ((IsExistScreenShare() && m_SDK->GetScreenStatus()) || IsPlayMediaFile()) {
            QString showID = mbMaster ? m_UserId : QString(m_pUserMananger->GetHostID());
            ChangeCenterRendUser(showID, "");
            m_pFloatRenderWdg->show();
            TRACE6("%s float wnd show\n", __FUNCTION__);
            m_pFloatRenderWdg->SetStayTop();
            QRect r = QApplication::desktop()->availableGeometry();
            int nInitX = r.width() - 280;
            int nInitY = 40;
            m_pFloatRenderWdg->move(nInitX, nInitY);
         }
      }
   }
   TRACE6("%s leave\n", __FUNCTION__);
}

void VhallInteractive::DealSelectMediaFile() {
   if (m_bIsFullScreen) {
      Slot_MouseEnterCtrlWdg();
   }
}

void VhallInteractive::OpenExternalCapture() {
   if (m_SDK && m_SDK->GetCameraCount() == 0 && !m_SDK->GetExternalCaptureState()) {
      m_SDK->OpenExternalCapture();
   }
}

void VhallInteractive::OnDeviceDetectCallback() {
   OpenExternalCapture();
}

void VhallInteractive::Slot_OnCheckMixStreamServer() {
   QString requestUrl = m_ThirdPullUrl + "vhall_inter_" + m_StreamId;
   mHttpNetWork->HttpNetworkGet(requestUrl);
}

void VhallInteractive::Slot_OnCheckMixStreamServerFinished(QByteArray replay, int replayCode) {
   QByteArray replyData = replay;
   if (!replyData.isEmpty()) {
      //除code = 100, 101, 102表示当前拉流状态正常其他皆为异常，需要重试或重启动
      QJsonDocument doc = QJsonDocument::fromJson(replyData);
      QJsonObject obj = doc.object();
      if (obj.contains("code")) {
         int status = obj["status"].toInt();
         QString code = obj["code"].toString();
         int errCode = code.toInt();
         if (errCode != 100 && errCode != 101 && errCode != 102) {
            TRACE6("%s  errCode is %d\n", __FUNCTION__, errCode);
            emit SigCheckMixStreamServerErr();
         }
         TRACE6("%s reply:%s \n", __FUNCTION__, replyData.toStdString().c_str());
      }
   }
}

void VhallInteractive::Slot_TimeExpired() {
   QString tip = QStringLiteral("请及时校准电脑设备时钟,否则影响观众端收看直播");
   FadeOutTip(tip);
   if (!mTimeExpriseTimer.isActive()) {
      mTimeExpriseTimer.stop();
      mTimeExpriseTimer.start(MAX_CHECK_MIX_STREAM_SERVER_TIMEOUT);
   }
}

void VhallInteractive::Slot_OnTimeExprise() {
   mTimeExpriseTimer.stop();
   emit SigCheckMixStreamServerErr();
}

void VhallInteractive::Slot_OnCheckRenderStream() {
   bool bReMix = false;
   const QMap<QString, QWidget*> renderWidgetsMap = m_pHorWidget->GetRenderWidgetsMap();
   QMap<QString, QWidget*>::const_iterator iterRender = renderWidgetsMap.begin();
   while (iterRender != renderWidgetsMap.end()) {
      CVideoRenderWdg *render = NULL;
      VhallRenderWidget *wRender = dynamic_cast<VhallRenderWidget *>(iterRender.value());
      if (wRender) {
         render = dynamic_cast<CVideoRenderWdg *>(wRender->GetWidget());
         bool isMixing = render->IsMixStream();
         QString id = render->GetID();
         unsigned int curTime = QDateTime::currentDateTime().toTime_t();
         unsigned int lastRenderTime = render->GetLastRenderTime();
         if (lastRenderTime + 30 < curTime) {
            render->SetMixStream(false);
            //如果30秒之内没有视频,并且之前参与了混流则重新混流。
            if (isMixing) {
               bReMix = true;
            }
         }
         else {
            render->SetMixStream(true);
            //如果之前没参与混流，如果有视频流了则重新混流。
            if (!isMixing) {
               bReMix = true;
            }
         }
      }
      iterRender++;
   }
   if (bReMix) {
      ReMix();
   }
}

void VhallInteractive::ReCheckMixUser(QSet<QString>& cameraList, QSet<QString>& micList) {
   const QMap<QString, QWidget*> renderWidgetsMap = m_pHorWidget->GetRenderWidgetsMap();
   QMap<QString, QWidget*>::const_iterator iterRender = renderWidgetsMap.begin();
   while (iterRender != renderWidgetsMap.end()) {
      CVideoRenderWdg *render = NULL;
      VhallRenderWidget *wRender = dynamic_cast<VhallRenderWidget *>(iterRender.value());
      if (wRender) {
         render = dynamic_cast<CVideoRenderWdg *>(wRender->GetWidget());
         QString id = render->GetID();
         QString streamId = "8136_" + GetTencentStreamId(m_StreamId, id, "main");
         unsigned int curTime = QDateTime::currentDateTime().toTime_t();
         unsigned int lastRenderTime = render->GetLastRenderTime();
         if (!render->IsMixStream()) {
            QSet<QString>::iterator iterCamera = cameraList.begin();
            QSet<QString>::iterator iterMic = micList.begin();
            while (iterCamera != cameraList.end()) {
               if ((*iterCamera).compare(streamId) == 0 ) {
                  iterCamera = cameraList.erase(iterCamera);
               }
               else  {
                  iterCamera++;
               }
            }
            while (iterMic != micList.end()) {
               if ((*iterMic).compare(streamId) == 0) {
                  iterMic = micList.erase(iterMic);
               } else {
                  iterMic++;
               }
            }
         }
      }
      iterRender++;
   }
}

void VhallInteractive::SetMixStreamUserID(const char* id,bool add) {
   if (mbMaster) {
      QString tstreamId = "8136_" + GetTencentStreamId(m_StreamId, strlen(id) == 0 ? m_UserId : QString(id), "main");
      QMap<QString, bool>::iterator iter = m_CameraListMap.find(tstreamId);
      if (iter == m_CameraListMap.end() && add) {
         m_CameraListMap[tstreamId] = true;
         ReMix();
         TRACE6("%s add\n",__FUNCTION__);
      }
      if (iter != m_CameraListMap.end() && !add) {
         m_CameraListMap.erase(iter);
         ReMix();
         TRACE6("%s del\n", __FUNCTION__);
      }
   }
}


