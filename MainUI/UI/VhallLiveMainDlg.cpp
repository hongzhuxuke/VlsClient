#include "VhallLiveMainDlg.h"
#include "stdafx.h"
#include <windows.h>
#include <dbt.h> 
#include <Usbiodef.h>
#include <QVBoxLayout>
#include <QIcon>
#include <QPushButton>
#include <QApplication>
#include <QDesktopWidget>
#include <QDebug>
#include <QPropertyAnimation>
//#include <QWebEngineSettings>
//#include <QWebEngineProfile>
#include <QDesktopServices>
#include <winuser.h>
#include "CameraTeachTips.h"
#include "VhallLivePlugInUnitDlg.h"
#include "InteractAPIManager.h"
#include "TitleBarWidget.h"
#include "RenderWnd.h"
#include "ToolBarWidget.h"
#include "LiveContentCtrl.h"
#include "Msg_MainUI.h"
#include "Msg_OBSControl.h"
#include "ICommonData.h"
#include "AlertTipsDlg.h"
#include "ToolButton.h"
#include "vhfadeouttipwidget.h"
#include "VHDialog.h"
#include "ConfigSetting.h"
#include "vhallsharedqr.h"
#include "vhallteaching.h"
#include "pathManager.h"
#include "pub.Struct.h"
#include "pub.Const.h"
#include "RecordDlg.h"
#include "PerformanceStatistics.h"
#include "LiveToolDlg.h"
#include "TabWidget/CTabWidget.h"
#include "ShowTipsWdg.h"
#include "IWinPCapData.h"


float MicDBtoLog(float db) {
   /* logarithmic scale for audio meter */
   return -log10(0.0f - (db - 6.0f));
}

#define VOL_MIN -96
#define VOL_MAX 0
#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif
#ifndef min
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif
float mic_minLinear = MicDBtoLog(VOL_MIN);
float mic_maxLinear = MicDBtoLog(VOL_MAX);

VhallLiveMainDlg::VhallLiveMainDlg(QWidget *parent)
   : CAliveDlg(parent) {
   /*
   **计算方法如下：
   **  1、标题栏固定40px,工具栏固定56px，中间区不固定，但是必须16：9（经计算为896x504）
   **  2、主窗口添加了1px的边框，计算时，高宽均有2px的影响
   **  3、右边栏，需高度一致
   */
   ui.setupUi(this);
   setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowSystemMenuHint | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint);
   wstring confPath = GetAppDataPath() + VHALL_TOOL_CONFIG;
   QString qsConfPath = qsConfPath.fromStdWString(confPath);
   QString vhallLive = ConfigSetting::ReadString(qsConfPath, GROUP_DEFAULT, KEY_VHALL_LIVE, VHALL_LIVE_TEXT);
   this->setWindowTitle(vhallLive);
   this->setFocusPolicy(Qt::ClickFocus);

   m_pVoiceTranslateWdg = new CVoiceTranslateWdg(this);
   if (m_pVoiceTranslateWdg) {
      m_pVoiceTranslateWdg->RegisterMainUI(this);
      m_pVoiceTranslateWdg->hide();
   }

   //左侧工具栏信号槽关联
   connect(ui.widget_leftTool, SIGNAL(sig_OnClickedDoc()), this, SLOT(slot_OnClickedDoc()));
   connect(ui.widget_leftTool, &LeftToolWdg::sig_OnClickedWhiteboard, this, &VhallLiveMainDlg::slot_ClickedWhiteboard);
   connect(ui.widget_leftTool, SIGNAL(sig_OnClickedDesktop()), this, SLOT(slot_OnClickedDesktop()));
   connect(ui.widget_leftTool, &LeftToolWdg::sig_OnClickedPluginUrl, this, &VhallLiveMainDlg::slot_OnClickedPluginUrl);
   connect(ui.widget_leftTool, &LeftToolWdg::sigSharedBtnClicked, this, &VhallLiveMainDlg::slot_OnShared);
   connect(ui.widget_leftTool, &LeftToolWdg::sigMemberBtnClicked, this, &VhallLiveMainDlg::slotMemberListClicked);
   connect(&mDocLoadTimer, SIGNAL(timeout()), this, SLOT(slot_LoadUrlCloseTool()));

   connect(ui.tabWidget->mpMemberListDlg, &MemberListWdg::signal_Refresh, this, &VhallLiveMainDlg::signal_MemberListRefresh);
   connect(ui.tabWidget->mpMemberListDlg, &MemberListWdg::sig_OperationUser, this, &VhallLiveMainDlg::sig_OperationUser);
   connect(ui.tabWidget->mpMemberListDlg, &MemberListWdg::sig_ReqApointPageUserList, this, &VhallLiveMainDlg::sig_ReqApointPageUserList);
   connect(ui.tabWidget->mpMemberListDlg, &MemberListWdg::sig_Hide, this, &VhallLiveMainDlg::slotMemberClose);

   //顶层title
   if (!ui.widget_title->Create(true)) {
      ASSERT(FALSE);
   }
   ui.widget_title->SetControlRightWidget(true);
   ui.widget_title->SetParentWidget(this);
   connect(ui.widget_title, SIGNAL(sigMousePressed()), this, SLOT(TitleBarPressed()));
   connect(ui.widget_title, &TitleBarWidget::sigRefresh, this, &VhallLiveMainDlg::slot_OnFresh);
   connect(ui.widget_title, &TitleBarWidget::sigMaxClicked, this, &VhallLiveMainDlg::slotMaxClicke);
   connect(ui.widget_title, &TitleBarWidget::sigCopy, this, &VhallLiveMainDlg::Slot_Copy);
   connect(ui.widget_title, SIGNAL(sigMin()), this, SLOT(slot_ClickedMin()));
   connect(ui.widget_title, SIGNAL(sigMouseMoved()), this, SLOT(slot_mouseMoved()));
   

   connect(ui.widget_bottomTool, SIGNAL(sigMousePressed()), this, SLOT(TitleBarPressed()));


   mLiveContentCtrlPtr = new LiveContentCtrl(this);
   if (mLiveContentCtrlPtr) {
      mLiveContentCtrlPtr->hide();
      connect(mLiveContentCtrlPtr, SIGNAL(sig_ChangeView()), this, SLOT(slot_OnChangeView()));
      connect(mLiveContentCtrlPtr, &LiveContentCtrl::sig_MicClicked, ui.widget_ToolBarWidget, &ToolBarWidget::micBtnClicked);
      connect(ui.widget_ToolBarWidget, &ToolBarWidget::sigMicStateChange, mLiveContentCtrlPtr, &LiveContentCtrl::slot_SetMicState);
   }

   connect(ui.tabWidget, &CTabWidget::sig_DragMember, this, &VhallLiveMainDlg::slot_changed);
   connect(ui.tabWidget, &CTabWidget::sig_JsCallOnOffLine, this, &VhallLiveMainDlg::JsCallQtMsg);
   connect(ui.tabWidget, &CTabWidget::sig_FlashJsCallUserOnLine, this, &VhallLiveMainDlg::JSCallQtUserOnline);
   connect(ui.tabWidget, &CTabWidget::sig_FlashJsCallUserOffLine, this, &VhallLiveMainDlg::JSCallQtUserOffline);
   this->installEventFilter(this);

   mNormalRect = this->geometry();
   if (QApplication::desktop()->availableGeometry().width() < mNormalRect.width() ||
      QApplication::desktop()->availableGeometry().height() < mNormalRect.height()) {
      this->setGeometry(QApplication::desktop()->availableGeometry());
      mNormalRect = QApplication::desktop()->availableGeometry();
   }
   ui.widget_leftTool->setEnablePluginBtn(false);
   VH::CComPtr<IMainUILogic> pMainUILogic = NULL;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IMainUI, IID_IMainUILogic, pMainUILogic);
   if (pMainUILogic) {
      pMainUILogic->SetEnabledInteractive(false);
   }

   //setMinimumSize(QSize((1280), (640)));
   //setMaximumSize(QSize((1920), (1080)));

}

VhallLiveMainDlg::~VhallLiveMainDlg() {
   mbIsDestory = true;
   removeEventFilter(this);

   mDocLoadTimer.stop();
   ui.tabWidget->Destroy();

   if (mpMicVolumeTimerPtr) {
      mpMicVolumeTimerPtr->stop();
      delete mpMicVolumeTimerPtr;
      mpMicVolumeTimerPtr = nullptr;
   }

   if (NULL != m_pContentWidget) {
      m_pContentWidget->removeEventFilter(this);
      m_pContentWidget->Destroy();
      delete m_pContentWidget;
      m_pContentWidget = NULL;
   }
   if (NULL != ui.widget_ToolBarWidget){
      ui.widget_ToolBarWidget->Destroy();
      delete ui.widget_ToolBarWidget;
      ui.widget_ToolBarWidget = NULL;
   }

   ReleaseDocWebView();
   DestroyUI();

   if (m_pVoiceTranslateWdg) {
      delete m_pVoiceTranslateWdg;
      m_pVoiceTranslateWdg = NULL;
   }
   TRACE6("%s ReleaseChatWebView\n", __FUNCTION__);
}


void VhallLiveMainDlg::InitUI() {
   mShowTipsWdgPtr = new ShowTipsWdg(this);
   if (mShowTipsWdgPtr) {
      mShowTipsWdgPtr->hide();
      connect(mShowTipsWdgPtr, SIGNAL(sig_changeProfile(int)), this, SLOT(slot_changeProfile(int)));
      connect(mShowTipsWdgPtr, SIGNAL(sig_removeItem()), this, SLOT(slot_removeTipsItem()));
   }

   mpMicVolumeTimerPtr = new QTimer(this);
   if (mpMicVolumeTimerPtr) {
      connect(mpMicVolumeTimerPtr, SIGNAL(timeout()), this, SLOT(slot_OnMicVolumeShow()));
      mpMicVolumeTimerPtr->start(500);
   }

   mPerformanceStatisticsPtr = new PerformanceStatistics();
   if (mPerformanceStatisticsPtr) {
      mPerformanceStatisticsPtr->Start();
   }
   mFullDocWdg = new FullDocWdg();
   if (mFullDocWdg) {
      connect(mFullDocWdg, SIGNAL(sig_PressKeyEsc()), this, SLOT(slot_PressKeyEsc()));
      mFullDocWdg->hide();
   }

   if (m_suspendTipTimer == nullptr) {
      m_suspendTipTimer = new QTimer(this);
      connect(m_suspendTipTimer, SIGNAL(timeout()), this, SLOT(closeSuspendTip()));
   }

   if (m_suspendTipButton == nullptr) {
      m_suspendTipButton = new QPushButton(NULL);
      m_suspendTipButton->setWindowFlags(Qt::FramelessWindowHint | Qt::Tool);
      m_suspendTipButton->setAttribute(Qt::WA_TranslucentBackground);
      m_suspendTipButton->setAutoFillBackground(false);
      m_suspendTipButton->installEventFilter(this);
      connect(m_suspendTipButton, SIGNAL(clicked()), this, SLOT(closeSuspendTip()));
   }

   if (m_suspendTipCloseTimer == nullptr) {
      m_suspendTipCloseTimer = new QTimer(this);
      connect(m_suspendTipCloseTimer, SIGNAL(timeout()), this, SLOT(DoSuspendClose()));
   }
}

void VhallLiveMainDlg::DestroyUI() {
   if (mTeachingWidget) {
      mTeachingWidget->removeEventFilter(this);
      delete mTeachingWidget;
      mTeachingWidget = nullptr;
   }
   if (mPRecordDlg) {
      mPRecordDlg->Destroy();
      delete mPRecordDlg;
      mPRecordDlg = nullptr;
   }
   if (mLiveToolDlg) {
      delete mLiveToolDlg;
      mLiveToolDlg = nullptr;
   }
   if (mPlayUIHideTimer) {
      mPlayUIHideTimer->stop();
      delete mPlayUIHideTimer;
      mPlayUIHideTimer = nullptr;
   }
   if (mDocLoadingTimer) {
      mDocLoadingTimer->stop();
      delete mDocLoadingTimer;
      mDocLoadingTimer = nullptr;
   }
   if (mShowTipsWdgPtr) {
      delete mShowTipsWdgPtr;
      mShowTipsWdgPtr = nullptr;
   }
   if (mCameraTeachTips) {
      delete mCameraTeachTips;
      mCameraTeachTips = nullptr;
   }
   if (mpVhallSharedQr) {
      delete mpVhallSharedQr;
      mpVhallSharedQr = nullptr;
   }
   //if (NULL != ui.tabWidget->mpMemberListDlg) {
   //   delete ui.tabWidget->mpMemberListDlg;
   //   ui.tabWidget->mpMemberListDlg = NULL;
   //}
   if (mPerformanceStatisticsPtr) {
      mPerformanceStatisticsPtr->Stop();
      delete mPerformanceStatisticsPtr;
      mPerformanceStatisticsPtr = nullptr;
   }

   if (mFullDocWdg) {
      delete mFullDocWdg;
      mFullDocWdg = nullptr;
   }
   if (m_suspendTipTimer) {
      m_suspendTipTimer->stop();
      delete m_suspendTipTimer;
      m_suspendTipTimer = nullptr;
   }
   if (m_suspendTipButton) {
      delete m_suspendTipButton;
      m_suspendTipButton = nullptr;
   }
   if (m_suspendTipCloseTimer) {
      delete m_suspendTipCloseTimer;
      m_suspendTipCloseTimer = nullptr;
   }
}

void VhallLiveMainDlg::Destroy() {

}

void VhallLiveMainDlg::CallDocWebViewtartLive() {
   if (mDocCefWidget) {
      VH::CComPtr<ICommonData> pCommonData;
      DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
      ClientApiInitResp initResp;
      pCommonData->GetInitApiResp(initResp);
      if (initResp.player == FLASH_LIVE_CREATE_TYPE) {
         mDocCefWidget->AppCallJsFunction(QString("qtCallJsStartLive(1)"));
      }
      else {
         mDocCefWidget->AppCallJsFunction(QString("QtCallFunctionPage(1)"));
      }
      ui.widget_leftTool->ReInitDocAndWhiteBoradSheet();
      ui.widget_leftTool->SetDocBtnClicked(true);
      TRACE6("%s qtCallJsStartLive\n", __FUNCTION__);
   }
}

void VhallLiveMainDlg::loadFinished(bool ok) {
   
}

void VhallLiveMainDlg::CreateDocWebView() {
   if (mDocCefWidget == nullptr) {
      mDocCefWidget = new LibCefViewWdg(this);
      if (mDocCefWidget) {
         mDocCefWidget->InitLibCef();
         if (mSmallWdgType == SmallWndType_Render) {
            ui.horizontalLayout_docWebView->addWidget(mDocCefWidget);
         }
         else if (mSmallWdgType == SmallWndType_Doc) {
            ui.horizontalLayout_smallRender->addWidget(mDocCefWidget);
         }
         ui.widget_docLoading->show();
         mDocCefWidget->Create(mDocUrl);
      }

      if (mDocLoadingTimer == nullptr) {
         mDocLoadingTimer = new QTimer(this);
         if (mDocLoadingTimer) {
            connect(mDocLoadingTimer, SIGNAL(timeout()), this, SLOT(slot_DocLoading()));
         }
      }
      if (mDocLoadingTimer) {
         mDocLoadingTimer->start(700);
      }
   }
   else {
      if (mSmallWdgType == SmallWndType_Render) {
         ui.widget_docLoading->show();
      }
      else if (mSmallWdgType == SmallWndType_Doc) {
         ui.widget_docLoading->hide();
      }
      if (mDocLoadingTimer) {
         mDocLoadingTimer->start(700);
      }
   }
}

void VhallLiveMainDlg::LoadDocUrl(const QString& url) {
   mDocUrl = url;
   if (mDocCefWidget && !ui.widget_smallRender->isHidden()){
      mDocCefWidget->LoadUrl(mDocUrl.toStdString().c_str());
   }
}

void VhallLiveMainDlg::slot_LoadUrlCloseTool() {
   if (mLoadCount == 6) {
      mDocLoadTimer.stop();
      mLoadCount = 0;
   }
   mLoadCount++;

   VH::CComPtr<ICommonData> pCommonData;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
   ClientApiInitResp initResp;
   pCommonData->GetInitApiResp(initResp);
   if (initResp.player == H5_LIVE_CREATE_TYPE) {
      if (ui.widget_leftTool->IsClickedWB()) {
         QString funPage = QString("QtCallFunctionPage(%1)").arg(eDocCtrlType_Broad);
         mDocCefWidget->AppCallJsFunction(funPage);
      }
   }

   if (mSmallWdgType == SmallWndType_Doc && mDocCefWidget) {
      QString funPage = QString("QtCallJsChangeDocTool(%1)").arg(eDocToolCtrlType_CloseTool);
      mDocCefWidget->AppCallJsFunction(funPage);
   }
}

void VhallLiveMainDlg::slot_OnChangeView() {
   if (mSmallWdgType == SmallWndType_Doc) {
      mLiveContentCtrlPtr->SetMicShow(true);
      //当前小窗显示文档，点击切换要把文档放在大窗
      ui.stackedWidget->setCurrentIndex(CenterPageIndex_Doc);
      if (mDocCefWidget) {
         ui.horizontalLayout_docWebView->addWidget(mDocCefWidget);
         QString funPage = QString("QtCallJsChangeDocTool(%1)").arg(eDocToolCtrlType_OpenTool);
         mDocCefWidget->AppCallJsFunction(funPage);
      }
      if (m_pContentWidget) {
         m_pContentWidget->SetRenderPos(false);
         ui.horizontalLayout_smallRender->addWidget(m_pContentWidget);
      }
      mSmallWdgType = SmallWndType_Render;
      slot_LeaveRenderEvent();
   }
   else if (mSmallWdgType == SmallWndType_Render) {
      mLiveContentCtrlPtr->SetMicShow(false);
      //当前小窗显示视频，点击切换要把视频放在大窗
      ui.stackedWidget->setCurrentIndex(CenterPageIndex_Render);
      if (m_pContentWidget) {
         m_pContentWidget->SetRenderPos(true);
         ui.horizontalLayout_centerRender->addWidget(m_pContentWidget);
      }
      if (mDocCefWidget) {
         ui.horizontalLayout_smallRender->addWidget(mDocCefWidget);
         QString funPage = QString("QtCallJsChangeDocTool(%1)").arg(eDocToolCtrlType_CloseTool);
         mDocCefWidget->AppCallJsFunction(funPage);
      }
      mSmallWdgType = SmallWndType_Doc;
   }
}


void VhallLiveMainDlg::slot_DocLoading() {
   QString tips = LOADING3;
   mDocLoadingPointCount++;
   if (mDocLoadingPointCount == 1) {
      tips = LOADING1;
   }
   else if (mDocLoadingPointCount == 2) {
      tips = LOADING2;
   }
   else {
      tips = LOADING3;
      mDocLoadingPointCount = 0;
   }
   ui.label_docLoading->setText(tips);
}

void VhallLiveMainDlg::slot_changed(bool bShowMem)
{
   if (bShowMem)
   {
      slotMemberListClicked();
   }
   else
   {
      if (ui.tabWidget->mpMemberListDlg && ui.tabWidget->mpMemberListDlg->isVisible())
      {
         ui.tabWidget->mpMemberListDlg->hide();
      }
   }
}

void VhallLiveMainDlg::slotMemberClose()
{
   if (nullptr != ui.tabWidget)
   {
      ui.tabWidget->setCurrentIndex(0);
      ui.tabWidget->GetBar()->setCurrentIndex(0);
   }
}

void VhallLiveMainDlg::InitChatWebView(const QString& strUrl)
{
   if (ui.tabWidget)
   {
      ui.tabWidget->InitChatWebView(strUrl);
      ui.widget_title->ShowRefreshButton();
   }
}

void VhallLiveMainDlg::hide() {
   if (mLiveContentCtrlPtr) {
      mLiveContentCtrlPtr->hide();
   }
   if (ui.tabWidget->mpMemberListDlg) {
      ui.tabWidget->mpMemberListDlg->hide();
   }
   ui.tabWidget->slotCloseShareDlg();
   ui.tabWidget->HideImagePrivew();
   CAliveDlg::hide();
}

bool VhallLiveMainDlg::Create(MainUILogic* pLogic) {
   m_pLogic = pLogic;
   //创建ContentWidget
   m_pContentWidget = new RenderWnd(this);
   if (NULL == m_pContentWidget || !m_pContentWidget->Create()) {
      ASSERT(FALSE);
      return false;
   }
   connect(m_pContentWidget, SIGNAL(sig_enterEvent(bool)), this, SLOT(slot_showChangeViewCtrl(bool)));
   connect(m_pContentWidget, SIGNAL(sig_LeaveEvent()), this, SLOT(slot_LeaveRenderEvent()));
   ui.horizontalLayout_centerRender->addWidget(m_pContentWidget);

   if (NULL == ui.widget_ToolBarWidget || !ui.widget_ToolBarWidget->Create()) {
      ASSERT(FALSE);
      return false;
   }
   ui.widget_ToolBarWidget->setLiveMainDlg(this);
   connect(ui.widget_ToolBarWidget, &ToolBarWidget::sigMemberListClicked, this, &VhallLiveMainDlg::slotMemberListClicked);
   connect(ui.widget_ToolBarWidget, &ToolBarWidget::sigClickedCamera, this, &VhallLiveMainDlg::slotClickedCamera);

   //注册设备插拔检测
   DEV_BROADCAST_DEVICEINTERFACE NotificationFilter;
   ZeroMemory(&NotificationFilter, sizeof(NotificationFilter));
   NotificationFilter.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
   NotificationFilter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
   // assume we want to be notified with USBSTOR  
   // to get notified with all interface on XP or above  
   // ORed 3rd param with DEVICE_NOTIFY_ALL_INTERFACE_CLASSES and dbcc_classguid will be ignored  
   NotificationFilter.dbcc_classguid = GUID_DEVINTERFACE_USB_DEVICE; //只注册USB设备
   HDEVNOTIFY hDevNotify = RegisterDeviceNotification((HWND)this->winId(), &NotificationFilter, DEVICE_NOTIFY_WINDOW_HANDLE);
   if (!hDevNotify) {
      // error handling...  
      return FALSE;
   }

   ui.widget_title->installEventFilter(this);
   m_pContentWidget->installEventFilter(this);
   ui.widget_ToolBarWidget->installEventFilter(this);
   m_suspendCloseCount = 0;
   return true;
}

void VhallLiveMainDlg::SetRenderWidgetUpdate(bool enable_updata) {
   if (m_pContentWidget) {
      m_pContentWidget->setUpdatesEnabled(enable_updata);
   }
}

void VhallLiveMainDlg::slot_OnClickedDoc() {
   slotClickedCamera();
   bool bShowWebView = false;
   if (mDocCefWidget == nullptr) {
      mSmallWdgType = SmallWndType_Render;
      CreateDocWebView();
   }
   else {
      bShowWebView = true;
   }

   mSmallWdgType = SmallWndType_Render;
   if (m_pContentWidget) {
      ui.widget_smallRender->show();
      m_pContentWidget->SetRenderPos(false);                                                  
      ui.horizontalLayout_smallRender->addWidget(m_pContentWidget);
   }
   if (mDocCefWidget) {
      ui.horizontalLayout_docWebView->addWidget(mDocCefWidget);
      ui.stackedWidget->setCurrentIndex(CenterPageIndex_Doc);
      QString funPage = QString("QtCallFunctionPage(%1)").arg(eDocCtrlType_Doc);
      mDocCefWidget->AppCallJsFunction(funPage);
      funPage = QString("QtCallJsChangeDocTool(%1)").arg(eDocToolCtrlType_OpenTool);
      mDocCefWidget->AppCallJsFunction(funPage);
   }
}

void VhallLiveMainDlg::slot_ClickedWhiteboard(){
   slotClickedCamera();
   if (mDocCefWidget == nullptr) {
      CreateDocWebView();
   }

   mSmallWdgType = SmallWndType_Render;
   if (m_pContentWidget) {
      ui.widget_smallRender->show();
      m_pContentWidget->SetRenderPos(false);
      ui.horizontalLayout_smallRender->addWidget(m_pContentWidget);
   }
   if (mDocCefWidget) {
      ui.stackedWidget->setCurrentIndex(CenterPageIndex_Doc);
      ui.horizontalLayout_docWebView->addWidget(mDocCefWidget);

      QString funPage = QString("QtCallFunctionPage(%1)").arg(eDocCtrlType_Broad);
      mDocCefWidget->AppCallJsFunction(funPage);
      funPage = QString("QtCallJsChangeDocTool(%1)").arg(eDocToolCtrlType_OpenTool);
      mDocCefWidget->AppCallJsFunction(funPage);
   }
}

void VhallLiveMainDlg::slot_OnClickedPluginUrl(const int& iX, const int& iY) {
   STRU_MAINUI_CLICK_CONTROL loControl;
   loControl.m_eType = control_LiveTool;
   loControl.m_globalX = iX;
   loControl.m_globalY = iY;
   loControl.m_bIsReLoad = mbIsFirstLoad ? true : false;
   mbIsFirstLoad = false;
   SingletonMainUIIns::Instance().PostCRMessage(MSG_MAINUI_CLICK_CONTROL, &loControl, sizeof(STRU_MAINUI_CLICK_CONTROL));
}

void VhallLiveMainDlg::ShowPlayUIState(bool show) {
   VH::CComPtr<IVedioPlayLogic> pVedioPlayLogic = NULL;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IMainUI, IID_IVedioPlayLogic, pVedioPlayLogic, return);
   if (pVedioPlayLogic) {
      pVedioPlayLogic->ReposVedioPlay();
      //pVedioPlayLogic->ShowPlayUI(show);
   }
}

void VhallLiveMainDlg::slot_HidePlayUI() {
   ShowPlayUIState(false);
}

void VhallLiveMainDlg::slot_LeaveRenderEvent() {
   if (mPlayUIHideTimer == nullptr) {
      mPlayUIHideTimer = new QTimer(this);
      if (mPlayUIHideTimer) {
         mPlayUIHideTimer->setSingleShot(true);
         connect(mPlayUIHideTimer, SIGNAL(timeout()), this, SLOT(slot_HidePlayUI()));
      }
   }
   if (mPlayUIHideTimer) {
      mPlayUIHideTimer->start(2000);
   }
   ShowPlayUIState(false);
}

void VhallLiveMainDlg::slot_showChangeViewCtrl(bool isCenterPos) {
   if (!isCenterPos) {
      if (mLiveContentCtrlPtr) {
         QPoint pos = ui.widget_smallRender->pos();
         mLiveContentCtrlPtr->move(pos);
         mLiveContentCtrlPtr->show();
      }
   }
   else {
      ShowPlayUIState(true);
   }
}

void VhallLiveMainDlg::slot_OnShared() {
   TRACE6("%s\n", __FUNCTION__);
   VH::CComPtr<ICommonData> pCommonData;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
   PublishInfo oStreamInfo;
   TRACE6("%s pCommonData->GetStreamInfo 1\n", __FUNCTION__);
   pCommonData->GetStreamInfo(oStreamInfo);
   TRACE6("%s pCommonData->GetStreamInfo 2\n", __FUNCTION__);

   QString toolConfPath = CPathManager::GetAppDataPath() + QString::fromStdWString(VHALL_TOOL_CONFIG);
   QString domain = ConfigSetting::ReadString(toolConfPath, GROUP_DEFAULT, KEY_VHALL_ShareURL, 0);
   QString url = domain + "/" + oStreamInfo.mStreamName.c_str();

   STRU_MAINUI_LOG log;
   swprintf_s(log.m_wzRequestUrl, DEF_MAX_HTTP_URL_LEN,
      L"key=%s&k=%d",
      L"livelistshare",
      eLogRePortK_LiveListShare);

   QJsonObject body;
   body["url"] = url;   	//分享链接

   QString json = CPathManager::GetStringFromJsonObject(body);
   strcpy_s(log.m_wzRequestJson, json.mid(0, DEF_MAX_HTTP_URL_LEN).toLatin1().data());
   SingletonMainUIIns::Instance().PostCRMessage(MSG_MAINUI_LOG, &log, sizeof(STRU_MAINUI_LOG));

   if (mpVhallSharedQr == nullptr) {
      mpVhallSharedQr = new VhallSharedQr(this);
   }

   if (nullptr != mpVhallSharedQr) {
      mpVhallSharedQr->CenterWindow(this);
      mpVhallSharedQr->Shared(domain, QString::fromStdString(oStreamInfo.mStreamName));
   }
}

void VhallLiveMainDlg::ShowSuspendTip(bool bShow) {
   if (!m_suspendTipButton) {
      return;
   }

   if (m_bShowSuspendTipForceHide == true) {
      m_suspendTipButton->hide();
      return;
   }

   if (!ui.widget_title) {
      return;
   }

   if (!m_pContentWidget) {
      return;
   }

   int x = this->x();
   int y = this->y();
   m_suspendTipButton->setStyleSheet("border-image:url(:/toolTip/img/toolTip/suspendTip.png)");
   m_suspendTipButton->resize(400, 152);

   x += this->width() / 2;
   x -= m_suspendTipButton->width() / 2;

   y += ui.widget_title->height();
   y += m_pContentWidget->height() / 2;
   y -= m_suspendTipButton->height() / 2;

   m_suspendTipButton->move(x, y);

   if (bShow&&m_bShowSuspendTip && !m_bForceHide) {
      if (m_suspendTipButton->isHidden()) {
         if (!this->isHidden()) {
            m_suspendTipButton->show();
            m_suspendTipButton->raise();
         }
      }
   }
   else {
      m_suspendTipButton->hide();
   }
}

void VhallLiveMainDlg::ShowSuspend(int msec) {
   if (m_bTipsClose) {
      m_suspendTipTimer->stop();
      return;
   }
   m_bShowSuspendTip = true;
   if (m_suspendTipButton) {
      m_suspendTipButton->setWindowOpacity(1.0f);
   }
   ShowSuspendTip(true);
   m_suspendTipTimer->start(msec);
}
void VhallLiveMainDlg::SetTipForceHide(bool bHide) {
   m_bForceHide = bHide;
}

void VhallLiveMainDlg::DoSuspendClose() {
   m_suspendCloseCount++;
   if (m_suspendCloseCount >= 10) {
      if (m_suspendTipCloseTimer) {
         m_suspendTipCloseTimer->stop();
      }

      m_bShowSuspendTip = false;
      ShowSuspendTip(false);
      m_suspendCloseCount = 0;
      return;
   }

   if (m_suspendTipButton) {
      m_suspendTipButton->setWindowOpacity((100 - 10 * m_suspendCloseCount) / 100.0f);
   }
}

void VhallLiveMainDlg::TitleBarPressed() {
   m_bTitleBarPressed = true;
   TRACE6("%s\n", __FUNCTION__);
}

void VhallLiveMainDlg::closeSuspendTip() {
   if (m_suspendTipTimer) {
      m_suspendTipTimer->stop();
   }

   if (!m_suspendTipButton) {
      return;
   }

   m_suspendCloseCount = 0;
   if (m_bTipsClose) {
      return;
   }
   m_suspendTipCloseTimer->start(100);
}

void VhallLiveMainDlg::Slot_Copy() {
   FadeOutTip(COPY_SUCCESS, TipsType_Success);
}

void VhallLiveMainDlg::slotMaxClicke()
{
   QRect rect = QApplication::desktop()->availableGeometry(this);
   if (mbIsMaxSize) { //显示正常态
      mbIsMaxSize = false;
      int moveX = 0;
      int moveY = 0;
      moveX = (rect.width() - mNormalRect.width()) / 2;
      moveY = (rect.height() - mNormalRect.height()) / 2;
      int iWidth = mNormalRect.width();
      int iHeight = mNormalRect.height();

      mNormalRect.setX(rect.x() + moveX);
      mNormalRect.setY(moveY >= 0 ? moveY : 0);
      mNormalRect.setWidth(iWidth);
      mNormalRect.setHeight(iHeight);
      this->setGeometry(mNormalRect);
      TRACE6("%s moveX:%d moveY:%d  rect.x():%d ", __FUNCTION__, moveX, moveY, rect.x());
   }
   else { //显示最大化状态
      QRect screen_geometry_rect = QApplication::desktop()->screenGeometry(this);
      if (rect == screen_geometry_rect) {
         rect.setHeight(screen_geometry_rect.height() - 1);
      }
      this->setGeometry(rect);
      setFocus();
      TRACE6("%s moveX:%d moveY:%d", __FUNCTION__, rect.x(), rect.y());
      mbIsMaxSize = true;
   }
   MoveCameraTips();
   ReSetTipsPos();
   ui.widget_title->SetMaxIcon(mbIsMaxSize);

   VH::CComPtr<IVedioPlayLogic> pVedioPlayLogic = NULL;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IMainUI, IID_IVedioPlayLogic, pVedioPlayLogic, return);
   if (pVedioPlayLogic) {
      pVedioPlayLogic->ReposVedioPlay();
   }
}

void VhallLiveMainDlg::slotLiveToolChoise(const int& iOpType) {
   QString funPage;
   switch (iOpType)
   {
   case eOpType_Questions://  问答      
   {
      funPage = QString("QtCallFunctionPage(%1)").arg(eDocCtrlType_Questions);
   }
   break;
   case eOpType_Signin://  签到   
   {
      funPage = QString("QtCallFunctionPage(%1)").arg(eDocCtrlType_Signin);
   }
   break;
   case eOpType_Luck:// 抽奖    
   {
      funPage = QString("QtCallFunctionPage(%1)").arg(eDocCtrlType_Luck);
   }
   break;
   case eOpType_questionnaire:// 问卷    
   {
      funPage = QString("QtCallFunctionPage(%1)").arg(eDocCtrlType_questionnaire);
   }
   break;
   case eOpType_envelopes:// 红包     
   {

   }
   break;
   default:
      break;
   }

   if (mDocCefWidget) {
      slot_OnClickedDoc();
      mDocCefWidget->AppCallJsFunction(funPage);
   }
}

void* VhallLiveMainDlg::GetRenderWidget() {
   if (NULL == m_pContentWidget) {
      ASSERT(FALSE);
      return NULL;
   }
   return m_pContentWidget;
}

void* VhallLiveMainDlg::GetRenderPos() {
   return ui.stackedWidget;
}

int  VhallLiveMainDlg::GetMediaPlayUIWidth() {
   if (NULL == m_pContentWidget) {
      return 0;
   }
   return m_pContentWidget->width();
}

void VhallLiveMainDlg::StartTimeClock(bool bStart) {
   if (NULL != ui.widget_title) {
      ui.widget_title->StartTimeClock(bStart);
      if (!bStart) {
         ui.widget_title->StartWithActiveLive(0);
      }
   }
}
void VhallLiveMainDlg::ClearTimeClock() {
   if (NULL != ui.widget_title) {
      ui.widget_title->ClearTimeClock();
   }
}

void VhallLiveMainDlg::SetStreamButtonStatus(bool isActive) {
   if (ui.widget_title) {
      ui.widget_title->SetStreamButtonStatus(isActive);
      if (nullptr != ui.widget_ToolBarWidget)
      {
         ui.widget_ToolBarWidget->SetStreamButtonStatus(ui.widget_title->isStreamStatusActive());
      }
   }
}

void VhallLiveMainDlg::mouseMoveEvent(QMouseEvent *e) {
   qDebug() << "CMainWidget::mouseMoveEvent";
   QDialog::mouseMoveEvent(e);
}


bool VhallLiveMainDlg::nativeEvent(const QByteArray &eventType, void *message, long *result) {
   Q_UNUSED(eventType);
   MSG* msg = reinterpret_cast<MSG*>(message);
   /*if (WM_WINDOWPOSCHANGED == msg->message) {
      if (!IsIconic((HWND)this->winId()) && IsWindowVisible((HWND)this->winId())) {
         if (!TitleBarWidget::MouseIsPressed()) {
            VH::CComPtr<IMainUILogic> pMainUILogic = NULL;
            DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IMainUI, IID_IMainUILogic, pMainUILogic);
            if (pMainUILogic) {
               QWidget *liveTool = (QWidget *)pMainUILogic->GetLiveToolWidget();
               if (qApp->focusWidget() == liveTool) {
                  if (liveTool != NULL) {
                     liveTool->raise();
                  }
               }
               else if (qApp->focusWidget() == this && !pMainUILogic->IsShareDesktop()) {
                  ActiveExtraWidget();
               }
               else if (pMainUILogic->IsShareDesktop()) {
                  hide();
               }
            }
         }
      }
   }*/

   if (WM_ACTIVATE == msg->message&&msg->hwnd == (HWND)this->winId()) {
      switch (msg->wParam) {
      case WA_ACTIVE:
      case WA_CLICKACTIVE:
         ActiveExtraWidget();
         break;
      default:
         break;
      }
   }

   if (!this->isHidden() && msg->hwnd == (HWND)this->winId()) {
      if (WM_SYSCOMMAND == msg->message) {
         if (SC_RESTORE == msg->wParam) {
            ActiveExtraWidget();
         }
         else if (SC_CLOSE == msg->wParam) {
            STRU_MAINUI_CLICK_CONTROL loControl;
            loControl.m_eType = control_CloseApp;
            loControl.m_dwExtraData = (DWORD)this;
            SingletonMainUIIns::Instance().PostCRMessage(MSG_MAINUI_CLICK_CONTROL, &loControl, sizeof(STRU_MAINUI_CLICK_CONTROL));
         }
      }
   }

   ProcessExternalMsg(msg->message, (void*)msg->lParam);
   if (WM_ACTIVATE == msg->message) {
      bool widgetIsShow = false;
      DWORD hwndProcess;
      if (GetWindowThreadProcessId(msg->hwnd, &hwndProcess)) {
         if (GetCurrentThreadId() == hwndProcess) {
            widgetIsShow = true;
         }
      }
      if (widgetIsShow) {
         ReposVedioPlayUI(true);
         ReposVoiceTranslateWdg(true);
         ShowSuspendTip(true);
      }
   }
   else if (msg->message == WM_NCACTIVATE) {
      if (msg->hwnd == (HWND)this->winId()) {
         WINDOWPLACEMENT param;
         GetWindowPlacement(msg->hwnd, &param);
         if (param.showCmd == 2) {
            ReposVedioPlayUI(false);
            ReposVoiceTranslateWdg(false);
            ShowSuspendTip(false);
         }
      }
   }
   //else if (WM_WINDOWPOSCHANGED == msg->message) {
   //   if (msg->hwnd == (HWND)this->winId()) {
   //      WINDOWPLACEMENT param;
   //      GetWindowPlacement(msg->hwnd, &param);
   //      if (param.showCmd == 1) {
   //         ReposVedioPlayUI(true);
   //         ReposVoiceTranslateWdg(true);
   //         ShowSuspendTip(true);
   //      }
   //   }
   //   return QWidget::nativeEvent(eventType, message, result);
   //}
   else if (WM_DEVICECHANGE == msg->message) {
      ProcessDeviceChgMsg(msg->wParam, msg->lParam);
      //return QWidget::nativeEvent(eventType, message, result);
   }
  /* else if (WM_NCCALCSIZE == msg->message) {
      WINDOWPLACEMENT ws;
      ws.length = sizeof(WINDOWPLACEMENT);
      GetWindowPlacement((HWND)this->winId(), &ws);

      VH::CComPtr<IVedioPlayLogic> pVedioPlayLogic;
      DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IMainUI, IID_IVedioPlayLogic, pVedioPlayLogic, return true);
      if (ws.showCmd == 2) {
         pVedioPlayLogic->ForceHide(true);
         m_bShowSuspendTipForceHide = true;
         ShowSuspendTip(false);
      }
      else {
         pVedioPlayLogic->ForceHide(false);
         m_bShowSuspendTipForceHide = false;
         ShowSuspendTip(true);
      }
   }*/
   return QDialog::nativeEvent(eventType, message, result);
}

void VhallLiveMainDlg::resizeEvent(QResizeEvent *event) {
   VH::CComPtr<IVedioPlayLogic> pVedioPlayLogic = NULL;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IMainUI, IID_IVedioPlayLogic, pVedioPlayLogic, return);
   if (pVedioPlayLogic) {
      pVedioPlayLogic->ReposVedioPlay();
   }

   if (mShowTipsWdgPtr) {
      mShowTipsWdgPtr->setFixedWidth(ui.stackedWidget->width());
      ReSetTipsPos();
   }

   VH::CComPtr<IMainUILogic> pMainUILogic;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IMainUI, IID_IMainUILogic, pMainUILogic, return, ASSERT(FALSE));
   if (ui.stackedWidget && m_pVoiceTranslateWdg && pMainUILogic && pMainUILogic->IsOpenVoiceTranslateFunc() && !pMainUILogic->IsShareDesktop()) {
      m_pVoiceTranslateWdg->show();
      m_pVoiceTranslateWdg->raise();
      QPoint pos = this->mapToGlobal(ui.stackedWidget->pos());
      int posWidth = (ui.stackedWidget->width() - m_pVoiceTranslateWdg->width()) / 2;
      int posHeight = (ui.stackedWidget->height() - m_pVoiceTranslateWdg->height());
      pos = pos + QPoint(posWidth, posHeight);
      m_pVoiceTranslateWdg->move(pos);
   }
   QSize size = this->size();
}

void VhallLiveMainDlg::changeEvent(QEvent *e) {
   QWidget::changeEvent(e);
   if (e->type() == QEvent::WindowStateChange) {
      if (this->windowState() & Qt::WindowMinimized) {
         if (m_pVoiceTranslateWdg) {
            m_pVoiceTranslateWdg->hide();
         }
      }
   }
   if (!m_bShow) {
      return;
   }
   TRACE6("%s  m_pLiveMainDlg->show()\n", __FUNCTION__);
   if (!IsShareScreen()) {
      show();
   }
   ShowSuspendTip(true);
   QDialog::changeEvent(e);
}

void VhallLiveMainDlg::leaveEvent(QEvent *e) {
   QWidget::leaveEvent(e);
   QDialog::leaveEvent(e);
}

void VhallLiveMainDlg::Hide() {
   TRACE6("%s\n", __FUNCTION__);
   if (m_pVoiceTranslateWdg) {
      m_pVoiceTranslateWdg->hide();
   }
   if (mLiveContentCtrlPtr) {
      mLiveContentCtrlPtr->hide();
   }
   CAliveDlg::hide();
}

void VhallLiveMainDlg::HideLogo(bool bHide) {
   if (ui.widget_title) {
      ui.widget_title->HideLogo(bHide);
   }
}

void VhallLiveMainDlg::showEvent(QShowEvent *e) {
   if (m_pLogic && ui.widget_title) {
      ui.widget_title->setRecordBtnShow(m_pLogic->GetDisplayCutRecord());
   }
   ui.tabWidget->SetMemberListCAliveDlg(this);
   bool enable = true;
   VH::CComPtr<IMainUILogic> pMainUILogic;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IMainUI, IID_IMainUILogic, pMainUILogic, return, ASSERT(FALSE));

   if (ui.stackedWidget && m_pVoiceTranslateWdg && pMainUILogic && pMainUILogic->IsOpenVoiceTranslateFunc() && !pMainUILogic->IsShareDesktop()) {
      m_pVoiceTranslateWdg->show();
      m_pVoiceTranslateWdg->raise();
      QPoint pos = this->mapToGlobal(ui.stackedWidget->pos());
      int posWidth = (ui.stackedWidget->width() - m_pVoiceTranslateWdg->width()) / 2;
      int posHeight = (ui.stackedWidget->height() - m_pVoiceTranslateWdg->height());
      pos = pos + QPoint(posWidth, posHeight);
      m_pVoiceTranslateWdg->move(pos);
   }

   VH::CComPtr<ICommonData> pCommonData;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
   ClientApiInitResp initApiResp;
   pCommonData->GetInitApiResp(initApiResp);
   // 1 flash 2 h5
   if (FLASH_LIVE_CREATE_TYPE == initApiResp.player) {
      ui.widget_leftTool->ShowWhiteboardBtn(false);
   }
   else {
      ui.widget_leftTool->ShowWhiteboardBtn();
   }
   ui.widget_leftTool->ShowMemberButoon(false);
   if (mbShowTips) {
      if (mCameraTeachTips == nullptr) {
         mCameraTeachTips = new CameraTeachTips(this);
      }
      if (mCameraTeachTips && ((mTeachingWidget && mTeachingWidget->isHidden()) || mTeachingWidget == nullptr)) {
         mCameraTeachTips->show();
         mbShowTips = false;
         MoveCameraTips();
      }
   }
   if (mTeachingWidget && mTeachingWidget->isVisible()) {
      mTeachingWidget->move(0, 0);
      QApplication::postEvent(this,new QEvent(CustomEvent_ShowTeaching));
   }
   this->setAttribute(Qt::WA_Mapped);
   QWidget::showEvent(e);
}

void VhallLiveMainDlg::SetTeachingPage(bool showTeaching) {
   VH::CComPtr<ICommonData> pCommonData;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
   ClientApiInitResp initApiResp;
   pCommonData->GetInitApiResp(initApiResp);
   if (initApiResp.player == FLASH_LIVE_CREATE_TYPE) {
      if (initApiResp.live_time <= 0) {
         ShowCheckDev();
      }
      return;
   }
   if (showTeaching) {
      if (mTeachingWidget == nullptr) {
         mTeachingWidget = new QWidget(this);
      }
      if (mCameraTeachTips) {
         mCameraTeachTips->hide();
      }
      if (mTeachingWidget) {
         mTeachingWidget->setFixedWidth(mNormalRect.width());
         mTeachingWidget->setFixedHeight(mNormalRect.height());
         mTeachingWidget->setStyleSheet("border-image: url(:/teaching/img/teaching/video_live_chat.png);");
         mTeachingWidget->move(0, 0);
         mTeachingWidget->show();
         mTeachingWidget->installEventFilter(this);
      }
   }
   else if (initApiResp.live_time <= 0) {
      ShowCheckDev();
   }
}

void VhallLiveMainDlg::HandleDownLoadHead(QEvent* event) {
   if (ui.tabWidget->mpMemberListDlg) {
      ui.tabWidget->mpMemberListDlg->HandleDownLoadHead(event);
   }
}

void VhallLiveMainDlg::StopCPUState() {
   if (mPerformanceStatisticsPtr) {
      mPerformanceStatisticsPtr->Stop();
      disconnect(this, SIGNAL(sig_RunningState(int, int, int)), this, SLOT(slot_RunningState(int, int, int)));
   }
}

void VhallLiveMainDlg::ClickedDocBtn() {
   VH::CComPtr<ICommonData> pCommonData;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
   if (eStartMode_flash == pCommonData->GetStartMode()) {
      return;
   }
   ui.widget_leftTool->slot_OnClickedDoc();
}

void VhallLiveMainDlg::MoveCameraTips() {
   if (mCameraTeachTips && !mCameraTeachTips->isHidden() && !mbIsDestory) {
      QPoint pos = ui.stackedWidget->mapToGlobal(QPoint(0, 0));
      mCameraTeachTips->move(pos + QPoint(20, ui.widget_ToolBarWidget->height()));
   }
}

void VhallLiveMainDlg::closeEvent(QCloseEvent *event) {
   if (event) {
      event->ignore();
   }
}

void VhallLiveMainDlg::showWidget() {
   ui.widget_title->setRecordBtnShow(true);
   TRACE6("%s\n", __FUNCTION__);
   if (!IsShareScreen()) {
      this->show();
      raise();
   }
   TRACE6("%s show end\n", __FUNCTION__);
   this->update();
}

void VhallLiveMainDlg::SetStreamStatus(StreamStatus* status) {
   if (status == nullptr) {
      return;
   }
   if (m_bIsStreamStart) {
      ui.widget_bottomTool->SetPushStreamBitRate(status->bytesSpeed);
      ui.widget_bottomTool->SetPushStreamLostPacket(status->droppedFrames, status->sumFrames);
   }
   else {
      ui.widget_bottomTool->SetPushStreamBitRate(0);
      ui.widget_bottomTool->SetPushStreamLostPacket(0, 0);
   }
}

float VhallLiveMainDlg::GetPushStreamLostRate() {
   return ui.widget_bottomTool->GetPushStreamLostRate();
}

void VhallLiveMainDlg::ShowNormal() {
   if (isHidden()) {
      show();
   }
   showWidget();
}

void VhallLiveMainDlg::slotClickedCamera() {
   if (mCameraTeachTips) {
      mCameraTeachTips->hide();
      delete mCameraTeachTips;
      mCameraTeachTips = nullptr;
   }
}

bool VhallLiveMainDlg::eventFilter(QObject *obj, QEvent *e) {
   if (this == obj) {
      if (e->type() == QEvent::FocusIn && !IsShareScreen()) {
         this->raise();
      }
   }
   else if (mDocCefWidget && mDocCefWidget == obj) {
      if (e->type() == QEvent::Enter && mSmallWdgType == SmallWndType_Doc) {
         slot_showChangeViewCtrl(false);
      }
      else if (e->type() == QEvent::Leave) {
         setCursor(Qt::ArrowCursor);
      }
   }

   if (mTeachingWidget && mTeachingWidget == obj && e->type() == QEvent::MouseButtonPress) {
      if (mTeachingPageIndex == 0) {
         mTeachingWidget->setStyleSheet("border-image: url(:/teaching/img/teaching/live_beauty.png);");
         mTeachingWidget->repaint();
      }
      else {
         mTeachingWidget->hide();
         if (mCameraTeachTips) {
            mCameraTeachTips->show();
            MoveCameraTips();
         }
         ShowCheckDev();
      }
      mTeachingPageIndex++;
   }
   MoveCameraTips();
   return QWidget::eventFilter(obj, e);
}

void VhallLiveMainDlg::ActiveExtraWidget() {
   VH::CComPtr<ISettingLogic> pSettingLogic = nullptr;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IMainUI, IID_ISettingLogic, pSettingLogic, return, ASSERT(FALSE));
   if (pSettingLogic != nullptr) {
      pSettingLogic->ActiveSettingUI();
   }
}

void VhallLiveMainDlg::ReActive() {
   if (!GetWindowLong((HWND)this->winId(), GWL_EXSTYLE& WS_EX_TOPMOST)) {
      return;
   }
   if (!IsShareScreen()) {
      raise();
   }
}

void VhallLiveMainDlg::RemoveImageWidget(QWidget *w) {
   if (!ui.widget_title) {
      return;
   }
   ui.widget_title->RemoveImageWidget(w);
}

void VhallLiveMainDlg::SetTipsClose(bool bClose) {
   m_bTipsClose = bClose;
   if (m_suspendTipButton) {
      m_suspendTipButton->close();
   }
}

void VhallLiveMainDlg::moveEvent(QMoveEvent *e) {
   QWidget::moveEvent(e);
   if (m_bTitleBarPressed) {
      m_bTitleBarPressed = false;
      emit sigMove();
   }
   MoveCameraTips();
   ReSetTipsPos();
   if (nullptr != mLiveContentCtrlPtr && mLiveContentCtrlPtr->isVisible()) {
      QPoint pos = ui.widget_smallRender->pos();
      mLiveContentCtrlPtr->move(pos);
   }

   if (ui.stackedWidget && m_pVoiceTranslateWdg) {
      m_pVoiceTranslateWdg->raise();
      QPoint pos = this->mapToGlobal(ui.stackedWidget->pos());
      int posWidth = (ui.stackedWidget->width() - m_pVoiceTranslateWdg->width()) / 2;
      int posHeight = (ui.stackedWidget->height() - m_pVoiceTranslateWdg->height());
      pos = pos + QPoint(posWidth, posHeight);
      m_pVoiceTranslateWdg->move(pos);
   }
   VH::CComPtr<IVedioPlayLogic> pVedioPlayLogic;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IMainUI, IID_IVedioPlayLogic, pVedioPlayLogic, return, ASSERT(FALSE));
   if (pVedioPlayLogic != nullptr) {
      pVedioPlayLogic->ReposVedioPlay(/*true*/);
   }
   QDialog::moveEvent(e);
}

void VhallLiveMainDlg::EnableShow() {
   this->m_bShow = true;
}

void VhallLiveMainDlg::showMinized() {
   this->showMinimized();
}

void VhallLiveMainDlg::ProcessExternalMsg(unsigned int uiMsg, void* param) {
   switch (uiMsg) {
   case WM_WINDOWPOSCHANGED: {
      ShowSuspendTip(true);
      break;
   }
                             break;
   case WM_USER_MODIFYWINDOW: {        //修改窗口
      STRU_MAINUI_CLICK_CONTROL loControl;
      loControl.m_eType = control_WindowSrc;
      SingletonMainUIIns::Instance().PostCRMessage(MSG_MAINUI_CLICK_CONTROL, &loControl, sizeof(STRU_MAINUI_CLICK_CONTROL));
      VH::CComPtr<ICommonData> pCommonData;
      DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
      pCommonData->ReportEvent("rjys05");
      break;
   }
   case WM_USER_MODIFYTEXT: {        //修改文本
      VH::CComPtr<IOBSControlLogic> pObsControlLogic = NULL;
      DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IOBSControl, IID_IOBSControlLogic, pObsControlLogic, return, ASSERT(FALSE));
      STRU_OBSCONTROL_TEXT* pTextFormat = (STRU_OBSCONTROL_TEXT*)param;
      STRU_OBSCONTROL_TEXT loTextFormat = *pTextFormat;
      pObsControlLogic->MemoryFree(pTextFormat);
      SingletonMainUIIns::Instance().PostCRMessage(MSG_MAINUI_MODIFY_TEXT, &loTextFormat, sizeof(STRU_OBSCONTROL_TEXT));
      break;
   }
   case WM_USER_MODIFYIMAGE: {       //修改图片
      VH::CComPtr<IOBSControlLogic> pObsControlLogic = NULL;
      DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IOBSControl, IID_IOBSControlLogic, pObsControlLogic, return, ASSERT(FALSE));
      STRU_OBSCONTROL_IMAGE *pModify = (STRU_OBSCONTROL_IMAGE*)param;
      STRU_OBSCONTROL_IMAGE loModify = *pModify;
      pObsControlLogic->MemoryFree(pModify);
      SingletonMainUIIns::Instance().PostCRMessage(MSG_MAINUI_MODIFY_IMAGE, &loModify, sizeof(STRU_OBSCONTROL_IMAGE));
      break;
   }
   case WM_USER_MODIFYCAMERA: {        //修改区域
      VH::CComPtr<ICommonData> pCommonData;
      DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
      pCommonData->ReportEvent("sxsb06");
      break;
   }
   case WM_USER_DELETE_CAMERA: {
      VH::CComPtr<ICommonData> pCommonData;
      DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
      pCommonData->ReportEvent("sxsb05");
      break;
   }
   case WM_USER_DELETE_TEXT: {
      break;
   }
   case WM_USER_DELETE_PIC: {
      break;
   }
   case WM_USER_DELETE_MONITOR: {
      VH::CComPtr<ICommonData> pCommonData;
      DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
      pCommonData->ReportEvent("zmys03");
      break;
   }
   case WM_USER_DELETE_REGION: {
      VH::CComPtr<ICommonData> pCommonData;
      DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
      pCommonData->ReportEvent("qyys01");
      break;
   }
   case WM_USER_DELETE_WINDOWS: {
      VH::CComPtr<ICommonData> pCommonData;
      DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
      pCommonData->ReportEvent("rjys04");
      break;
   }
   case WM_USER_DELETE_MEDIA: {
      VH::CComPtr<ICommonData> pCommonData;
      DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
      pCommonData->ReportEvent("crsp06cbt05");
      break;
   }
   default:
      break;
   }
}

void VhallLiveMainDlg::UpdateStreamStateUI(QString qStrStreamState, int iStreamCnt) {
   if (NULL != ui.widget_ToolBarWidget) {
      ui.widget_ToolBarWidget->UpdateStreamStateUI(qStrStreamState, iStreamCnt);
   }
}

void VhallLiveMainDlg::ReSetTipsPos() {
   QPoint pos = ui.stackedWidget->mapToGlobal(QPoint(0, 0));
   if (mShowTipsWdgPtr) {
      mShowTipsWdgPtr->move(pos);
   }
}

void VhallLiveMainDlg::ReposVedioPlayUI(bool bShow) {
   //重设插播窗口位置
   VH::CComPtr<IVedioPlayLogic> pVedioPlayLogic = nullptr;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IMainUI, IID_IVedioPlayLogic, pVedioPlayLogic, return);
   if (pVedioPlayLogic) {
      pVedioPlayLogic->ReposVedioPlay(/*bShow*/);
   }
}

void VhallLiveMainDlg::ProcessDeviceChgMsg(WPARAM wparam, LPARAM lparam) {
   if (DBT_DEVICEARRIVAL == wparam || DBT_DEVICEREMOVECOMPLETE == wparam) {          //插入
      PDEV_BROADCAST_DEVICEINTERFACE pHdr = (PDEV_BROADCAST_DEVICEINTERFACE)lparam;
      if (pHdr) {
         STRU_MAINUI_DEVICE_CHANGE loDeviceChg;
         wcscpy(loDeviceChg.m_wzDeviceID, pHdr->dbcc_name);
         loDeviceChg.m_bAdd = DBT_DEVICEARRIVAL == wparam ? TRUE : FALSE;
         SingletonMainUIIns::Instance().PostCRMessage(
            MSG_MAINUI_DEVICE_CHANGE, &loDeviceChg, sizeof(STRU_MAINUI_DEVICE_CHANGE));
      }
   }
}

void VhallLiveMainDlg::CenterWindow(QWidget* parent) {
   int x = 0;
   int y = 0;
   if (NULL == parent) {
      const QRect rect = QApplication::desktop()->availableGeometry();
      x = rect.left() + (rect.width() - width()) / 2;
      y = rect.top() + (rect.height() - height()) / 2;
   }
   else {
      QPoint point(0, 0);
      point = parent->mapToGlobal(point);
      x = point.x() + (parent->width() - width()) / 2;
      y = point.y() + (parent->height() - height()) / 2;
   }
   move(x, y);
}

void VhallLiveMainDlg::SetStartLiveState(bool start) {
   if (ui.widget_ToolBarWidget) {
      ui.widget_ToolBarWidget->SetStartLiveState(start);
   }
   ui.widget_leftTool->CheckStartMode();
   if (start) {
      //开始直播显示
      m_pContentWidget->SetRenderPos(true);
      ui.widget_smallRender->hide();
      ui.horizontalLayout_centerRender->addWidget(m_pContentWidget);
      ui.stackedWidget->setCurrentIndex(CenterPageIndex_Render);
      mSmallWdgType = SmallWndType_None;
      VH::CComPtr<ICommonData> pCommonData;
      DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
      if (pCommonData && pCommonData->GetStartMode() == eStartMode_flash) {
         ui.widget_title->ShowRefreshButton(false);
         ui.widget_title->ShowWebinarName(false);
         int width = ui.tabWidget->width();
         int mainWidth = this->width();
         int newWidth = mainWidth - width;
         this->setFixedWidth(newWidth);
         ui.tabWidget->hide();
      }
   }
   else {
      if (mFullDocWdg) {
         mFullDocWdg->hide();
      }
      if (mPerformanceStatisticsPtr) {
         mPerformanceStatisticsPtr->Stop();
      }
      mNetDevDataCount = 0;
   }
}

//麦克风静音
void VhallLiveMainDlg::SetMicMute(bool bMute) {
   if (!ui.widget_ToolBarWidget) {
      return;
   }
   ui.widget_ToolBarWidget->SetMicMute(bMute);
}

//扬声器静音
void VhallLiveMainDlg::SetSpeakerMute(bool bMute) {
   if (!ui.widget_ToolBarWidget) {
      return;
   }
   ui.widget_ToolBarWidget->SetSpeakerMute(bMute);
}

bool VhallLiveMainDlg::IsDesktopShow() {
   if (m_pLogic) {
      return m_pLogic->IsDesktopShow();
   }
   return false;
}

void VhallLiveMainDlg::SetDesktopSharing(bool bShow) {
   if (!ui.widget_ToolBarWidget) {
      return;
   }
   ui.widget_leftTool->SetDesktopSharing(bShow);
}

void VhallLiveMainDlg::HandleDevCheck() {
   //mbIsDeviceChecking = true;
   AlertTipsDlg tip(CHECK_DEVICE_RE_NOTICE, true, this);
   tip.CenterWindow(this);
   tip.SetYesBtnText(TO_CHECK_DEV);
   tip.SetNoBtnText(NO_CHECK_DEV);
   if (tip.exec() == QDialog::Accepted) {
      mDeviceTestingWdg = new DeviceTestingWdg(eLiveType_Live, this);
      mDeviceTestingWdg->SetRole(true);
      mDeviceTestingWdg->StartPreView();
      mDeviceTestingWdg->raise();
      mDeviceTestingWdg->exec();
   }
   else {
      AlertTipsDlg tip_notice(CHECK_DEVICE_NOTICE, true, this);
      tip_notice.CenterWindow(this);
      tip_notice.SetYesBtnText(TO_CHECK_DEV);
      tip_notice.SetNoBtnText(QString::fromWCharArray(L"不检测"));
      if (tip_notice.exec() == QDialog::Accepted) {
         mDeviceTestingWdg = new DeviceTestingWdg(eLiveType_Live, this);
         mDeviceTestingWdg->SetRole(true);
         mDeviceTestingWdg->raise();
         mDeviceTestingWdg->StartPreView();
         mDeviceTestingWdg->exec();
      }

   }
   //mbIsDeviceChecking = false;
}

void VhallLiveMainDlg::FadeOutTip(QString str, int type, int showTime /*= 3000*/) {
   if (mShowTipsWdgPtr) {
      mShowTipsWdgPtr->setFixedWidth(ui.stackedWidget->width());
      if (!isMinimized()) {
         mShowTipsWdgPtr->show();
      }
      mShowTipsWdgPtr->AddTips(str, ui.stackedWidget->width(), showTime, (TipsType)type);
      ReSetTipsPos();
   }
}

void VhallLiveMainDlg::slot_removeTipsItem() {
   ReSetTipsPos();
}

void VhallLiveMainDlg::BeHide() {
   if (m_suspendTipButton) {
      delete m_suspendTipButton;
      m_suspendTipButton = NULL;
   }
}

void VhallLiveMainDlg::ReInit() {
   if (ui.widget_title) {
      ui.widget_title->ReInit();
   }
   if (ui.widget_leftTool) {
      ui.widget_leftTool->ReInitDocAndWhiteBoradSheet();
   }
   m_bIsStreamStart = false;
   mbShowTips = true;
   this->setGeometry(mNormalRect);
   mbIsMaxSize = false;
}

void VhallLiveMainDlg::ToExit() {
   emit this->exitClicked(true);
}

void VhallLiveMainDlg::SetStreamStatus(bool bStart) {
   m_bIsStreamStart = bStart;
   VH::CComPtr<IMainUILogic> pMainUILogic = NULL;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IMainUI, IID_IMainUILogic, pMainUILogic);
   if (bStart) {
      ui.widget_bottomTool->SetPushStreamFps(1);
      ui.widget_leftTool->setEnablePluginBtn(true);
      if (pMainUILogic) {
         pMainUILogic->SetEnabledInteractive(true);
      }
   }
   else {
      ui.widget_bottomTool->SetPushStreamFps(0);
      ui.widget_bottomTool->SetPushStreamBitRate(0);
      ui.widget_bottomTool->SetPushStreamLostPacket(0, 0);
      ui.widget_leftTool->setEnablePluginBtn(false);
      //ui.widget_leftTool->slot_OnClickedDoc();
      if (pMainUILogic) {
         pMainUILogic->SetEnabledInteractive(false);
      }       
      //if (mpDocWebEngineView != nullptr && (mSmallWdgType == SmallWndType_Render || mSmallWdgType == SmallWndType_Doc)) {
      //   ui.widget_leftTool->ReInitDocAndWhiteBoradSheet();
      //   ui.widget_leftTool->SetDocBtnClicked(true);
      //}
   }
   VH::CComPtr<ICommonData> pCommonData;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
   if (pCommonData) {
      pCommonData->SetPublishState(bStart);
   }
   int state = bStart == true ? 1 : 0;
   SingletonMainUIIns::Instance().PostCRMessage(MSG_MAINUI_START_LIVE_STATUS, &state, sizeof(int));
}

void VhallLiveMainDlg::SetWebinarName(QString name, QString id) {
   if (ui.widget_title) {
      ui.widget_title->SetWebinarName(name, id);
   }
}

void VhallLiveMainDlg::ClearWebinarName() {
   if (ui.widget_title) {
      ui.widget_title->ClearWebinarName();
   }
}

void VhallLiveMainDlg::ClearMemberList() {
   if (NULL != ui.tabWidget->mpMemberListDlg) {
      ui.tabWidget->mpMemberListDlg->setUserCount(-1);
      ui.tabWidget->mpMemberListDlg->clearData();
   }
}

void VhallLiveMainDlg::ShowVoiceTranslateWdg(bool show) {
   if (m_pVoiceTranslateWdg) {
      if (show) {
         ui.widget_ToolBarWidget->SetVoiceTranslateBtnState(true);
         m_pVoiceTranslateWdg->show();
         m_pVoiceTranslateWdg->raise();
         if (ui.stackedWidget && m_pVoiceTranslateWdg) {
            QPoint pos = this->mapToGlobal(ui.stackedWidget->pos());
            int posWidth = (ui.stackedWidget->width() - m_pVoiceTranslateWdg->width()) / 2;
            int posHeight = (ui.stackedWidget->height() - m_pVoiceTranslateWdg->height());
            pos = pos + QPoint(posWidth, posHeight);
            m_pVoiceTranslateWdg->move(pos);
         }
      }
      else {
         m_pVoiceTranslateWdg->hide();
         ui.widget_ToolBarWidget->SetVoiceTranslateBtnState(false);
      }
   }
}

void VhallLiveMainDlg::SetVoiceTransitionText(QString text) {
   if (m_pVoiceTranslateWdg) {
      m_pVoiceTranslateWdg->SetVoiceTranslateText(text);
   }
}

void VhallLiveMainDlg::hideEvent(QHideEvent *event) {
   if (m_pVoiceTranslateWdg) {
      m_pVoiceTranslateWdg->hide();
   }
   if (mCameraTeachTips) {
      mCameraTeachTips->hide();
   }
}

void VhallLiveMainDlg::customEvent(QEvent *event) {
   if (event) {
      int typeValue = event->type();
      switch (typeValue) {
      case CustomEvent_CopyShareUrl: {
         FadeOutTip(COPY_SUCCESS, TipsType_Success);
         break;
      }
      case CustomEvent_LibCefMsg:{
         CustomRecvMsgEvent* cus_event = dynamic_cast<CustomRecvMsgEvent*>(event);
         if (cus_event) {
            JsCallQtMsg(cus_event->msg);
         }
         break;
      }
      case CustomEvent_ShowTeaching: {
         if (mTeachingWidget && mTeachingWidget->isVisible()) {
            mTeachingWidget->move(0,0);
         }
         break;
      }
      case CustomEvent_DEV_CHECK: {
         HandleDevCheck();
         break;
      }
      default:
         break;
      }
   }
}

void VhallLiveMainDlg::SetVoiceTransitionFontSize(int size) {
   if (m_pVoiceTranslateWdg) {
      m_pVoiceTranslateWdg->ResetFontSize(size);
   }
}

void VhallLiveMainDlg::ReposVoiceTranslateWdg(bool bShow) {
   if (ui.stackedWidget && m_pVoiceTranslateWdg && bShow) {
      m_pVoiceTranslateWdg->raise();
      QPoint pos = this->mapToGlobal(ui.stackedWidget->pos());
      int posWidth = (ui.stackedWidget->width() - m_pVoiceTranslateWdg->width()) / 2;
      int posHeight = (ui.stackedWidget->height() - m_pVoiceTranslateWdg->height());
      pos = pos + QPoint(posWidth, posHeight);
      m_pVoiceTranslateWdg->move(pos);
   }
}

bool VhallLiveMainDlg::IsShareScreen() {
   VH::CComPtr<IMainUILogic> pMainUILogic = NULL;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IMainUI, IID_IMainUILogic, pMainUILogic);
   if (pMainUILogic && pMainUILogic->IsShareDesktop()) {
      return true;
   }
   return false;
}

bool VhallLiveMainDlg::IsRecordShow(){
   if (!mPRecordDlg) {
      return false;
   }
   return !mPRecordDlg->isHidden();
}

void VhallLiveMainDlg::SetRecordState(const int& eState)
{
   if (ui.widget_title) {
      ui.widget_title->SetRecordState(eState);
      if (NULL == mPRecordDlg) {
         mPRecordDlg = new CRecordDlg(this);
         if (NULL == mPRecordDlg || !mPRecordDlg->Create()) {
            ASSERT(FALSE);
            return;
         }
      }
      mPRecordDlg->SetRecordState(eState);
   }
}

void VhallLiveMainDlg::ShowRecordDlg()
{
   if (NULL == mPRecordDlg)
   {
      mPRecordDlg = new CRecordDlg(this);
      if (NULL == mPRecordDlg || !mPRecordDlg->Create()) {
         ASSERT(FALSE);
         return;
      }
   }
   QPoint Pos = ui.widget_title->RecordPos();
   mPRecordDlg->move(Pos.x() - 11, Pos.y() + ui.widget_title->height());
   mPRecordDlg->show();
}

int VhallLiveMainDlg::GetRecordState() {
   int iState = eRecordState_Stop;
   if (NULL != ui.widget_title) {
      iState = ui.widget_title->GetRecordState();
   }
   return iState;
}

bool VhallLiveMainDlg::IsRecordBtnhide() {
   bool bHide = true;
   if (NULL != ui.widget_title) {
      bHide = ui.widget_title->IsRecordBtnhide();
   }
   return bHide;
}

void VhallLiveMainDlg::ToExitAppWithAnimation() {
   if (ui.widget_ToolBarWidget) {
      ui.widget_ToolBarWidget->setMinimumHeight(0);
   }
   if (ui.widget_title) {
      ui.widget_title->setMinimumHeight(0);
   }
   if (m_pContentWidget) {
      m_pContentWidget->setMinimumHeight(0);
   }
   setMinimumSize(0, 0);
   QPropertyAnimation* closeAnimation = new QPropertyAnimation(this, "geometry");
   closeAnimation->setStartValue(geometry());
   closeAnimation->setEndValue(QRect(geometry().x(), geometry().y() /*+ height() / 2*/, width(), 0));
   closeAnimation->setDuration(150);
   connect(closeAnimation, SIGNAL(finished()), this, SLOT(slot_CloseAndExitApp()));
   closeAnimation->start(QAbstractAnimation::DeleteWhenStopped);
}

void VhallLiveMainDlg::slot_CloseAndExitApp() {
   TRACE6("%s\n", __FUNCTION__);
   hide();
   emit sig_ExitApp();
}

void VhallLiveMainDlg::slotMemberListClicked()
{
   TRACE6("%s\n", __FUNCTION__);
   createMemberListDlg();
   if (NULL != ui.tabWidget->mpMemberListDlg) {
      ui.tabWidget->mpMemberListDlg->SetCurrentListTab(eTabMember_onLine);
      ui.tabWidget->mpMemberListDlg->show(QApplication::desktop()->availableGeometry(this));
      ui.tabWidget->mpMemberListDlg->raise();
      TRACE6("%s ui.tabWidget->mpMemberListDlg SHOW\n", __FUNCTION__);
   }
   else {
      TRACE6("%s ui.tabWidget->mpMemberListDlg is NULL\n", __FUNCTION__);
   }
}

void VhallLiveMainDlg::createMemberListDlg()
{
   if (ui.tabWidget->mpMemberListDlg) {
      ui.tabWidget->mpMemberListDlg->SetAlive(this);
      ui.tabWidget->mpMemberListDlg->SetHansUpShow(false);
      //connect(ui.tabWidget->mpMemberListDlg, &MemberListWdg::signal_Refresh, this, &VhallLiveMainDlg::signal_MemberListRefresh);
      //connect(ui.tabWidget->mpMemberListDlg, &MemberListWdg::sig_OperationUser, this, &VhallLiveMainDlg::sig_OperationUser);
      //connect(ui.tabWidget->mpMemberListDlg, &MemberListWdg::sig_ReqApointPageUserList, this, &VhallLiveMainDlg::sig_ReqApointPageUserList);
      //connect(ui.tabWidget->mpMemberListDlg, &MemberListWdg::sig_Hide, this, &VhallLiveMainDlg::slotMemberClose);
   }
}

bool  VhallLiveMainDlg::GetIsLiving() {
   bool bIsLiving = false;
   return bIsLiving;
}

QString VhallLiveMainDlg::GetJoinId() {
   return mstrUser_id;
}

QString VhallLiveMainDlg::GetJoinRole() {
   return mstrRole;
}

QString VhallLiveMainDlg::GetHostId() {
   return mstrUser_id;
}

void VhallLiveMainDlg::SetStreamId(const QString &streamId) {
   mStreamId = streamId;
}

void VhallLiveMainDlg::GetRenderPos(int& x, int& y) {
   x = ui.stackedWidget->pos().x();
   y = ui.stackedWidget->pos().y() + ui.widget_ToolBarWidget->height();
}

void VhallLiveMainDlg::HandleJoinMember(VhallAudienceUserInfo* user_info) {
   if (NULL != ui.tabWidget->mpMemberListDlg) {
      ui.tabWidget->mpMemberListDlg->NtfUserOnline(*user_info);
   }
}

long long VhallLiveMainDlg::GetShowUserMember(){
   int count = 0;
   if (NULL != ui.tabWidget->mpMemberListDlg) {
      count = ui.tabWidget->mpMemberListDlg->GetShowUserMember();
   }
   return count;
}

void VhallLiveMainDlg::HandleLeaveMember(VhallAudienceUserInfo* user_info) {
   VH::CComPtr<ICommonData> pCommonData;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
   ClientApiInitResp info;
   pCommonData->GetInitApiResp(info);
   if (NULL != ui.tabWidget->mpMemberListDlg && info.user_id != user_info->userId) {
      ui.tabWidget->mpMemberListDlg->NtfUserOffLine(QString::fromStdWString(user_info->userId), user_info->miUserCount);
   }
}

void VhallLiveMainDlg::StartLiveWithTime(int liveTime) {
   SetStreamButtonStatus(true);
   ui.widget_title->StartWithActiveLive(liveTime);
}

QPoint VhallLiveMainDlg::GetTipPoint(/*int& iWidth*/) {
   QPoint refPos;
   refPos = ui.stackedWidget->pos();
   refPos = mapToGlobal(refPos);
   if (ui.page_render->isActiveWindow()) {
      refPos += QPoint(0, ui.widget_ToolBarWidget->height());
   }
   return refPos;
}

long VhallLiveMainDlg::GetStartTime() {
   return ui.widget_title->GetLiveStartTime();
}

void VhallLiveMainDlg::AddVoiceTranslate() {
   ui.widget_ToolBarWidget->AddVoiceTranslate();
}

void VhallLiveMainDlg::ShowInteractiveTools(const int& x, const int& y) {
   if (nullptr == mLiveToolDlg) {
      mLiveToolDlg = new LiveToolDlg(this);
      mLiveToolDlg->Create();
      connect(mLiveToolDlg, &LiveToolDlg::sigClicked, this, &VhallLiveMainDlg::slotLiveToolChoise);
   }
   mLiveToolDlg->move(x, y);
   mLiveToolDlg->show();
}

void VhallLiveMainDlg::RemoveVoiceTranslate() {
   ui.widget_ToolBarWidget->RemoveVoiceTranslate();
}

bool VhallLiveMainDlg::IsFirstLoadTool() {
   return mbIsFirstLoad;
}

QString VhallLiveMainDlg::GetStreamName() {
   return mStreamId;
}

bool VhallLiveMainDlg::IsExistRenderWnd(const QString& uid) {
   return false;
}

void VhallLiveMainDlg::HandleChatMsg(QEvent* event) {
   CustomOnHttpResMsgEvent *customEvent = dynamic_cast<CustomOnHttpResMsgEvent*>(event);
   if (customEvent) {
      VhallAudienceUserInfo Info;
      Info.role = customEvent->mRoleName.toStdWString();
      Info.userId = customEvent->mRecvUserId.toStdWString();
      Info.userName = customEvent->mNickName.toStdWString();
      Info.gagType = customEvent->is_banned == true ? VhallShowType_Allow : VhallShowType_Hide;
      Info.miUserCount = customEvent->mUv;
      if (customEvent->mMsg == "Join") {
         createMemberListDlg();
         if (NULL != ui.tabWidget->mpMemberListDlg) {
            ui.tabWidget->mpMemberListDlg->NtfUserOnline(Info);
         }
      }
      else if (customEvent->mMsg == "Leave") {
         VH::CComPtr<ICommonData> pCommonData;
         DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
         ClientApiInitResp info;
         pCommonData->GetInitApiResp(info);
         createMemberListDlg();
         if (NULL != ui.tabWidget->mpMemberListDlg && Info.userId != info.user_id) {
            ui.tabWidget->mpMemberListDlg->NtfUserOffLine(QString::fromStdWString(Info.userId), Info.miUserCount);
         }
         TRACE6("%s Info.userId:%s\n", __FUNCTION__, QString::fromStdWString(Info.userId).toStdString().c_str());
      }
   }
}

void VhallLiveMainDlg::HandleRecvSocketIOMsgEvent(Event &event) {
   TRACE6("%s enter event.m_eMsgType:%d\n", __FUNCTION__, event.m_eMsgType);
   createMemberListDlg();
   if (event.m_bSendMsg == 1) {
      int nCode = 200;
      std::string errMsg;
      InteractAPIManager::ParamHttpResp(event.context.toStdString(), nCode, errMsg);
      if (nCode != 200) {
         FadeOutTip(errMsg.length() == 0 ? DO_ERR : QString::fromStdString(errMsg), TipsType_Error);
      }
      return;
   }
   switch (event.m_eMsgType){
      case e_RQ_UserProhibitSpeakOneUser://单个用户禁言
      case e_RQ_UserAllowSpeakOneUser://单个用户取消禁言
      case e_RQ_UserAllowJoinOneUser://允许单个用户加入	
      case e_RQ_UserOnlineList://在线列表
      case e_RQ_SearchOnLineMember:
      case e_RQ_UserProhibitSpeakList://禁言列表
      case e_RQ_UserKickOutList://踢出列表	
      case e_RQ_UserSpecialList: //特殊用户列表 主持人、嘉宾、助理
      case e_RQ_CancelHandsUp://取消举手
      case e_RQ_replyInvite_Error: {
         if (ui.tabWidget->mpMemberListDlg) {
            ui.tabWidget->mpMemberListDlg->RecvUserListMsg(event);
         }
         break;
      }
      case e_RQ_UserKickOutOneUser: { //踢出单个用户
         if (ui.tabWidget->mpMemberListDlg) {
            ui.tabWidget->mpMemberListDlg->RecvUserListMsg(event);
         }
         break;
      }
      case e_RQ_ReleaseAnnouncement: {
         HandleRecvAnnouncement(event);
         break;
      }
      default:
         break;
   }
   TRACE6("%s leave event.m_eMsgType:%d\n", __FUNCTION__, event.m_eMsgType);
}

void VhallLiveMainDlg::HandleRecvAnnouncement(Event &e) {
   QString msg = e.context;
   QString pushTime = e.pushTime;
   ui.tabWidget->AppendAnnouncement(msg, pushTime);
}

void VhallLiveMainDlg::ShowCheckDev() {
   QApplication::postEvent(this, new QEvent(CustomEvent_DEV_CHECK));
}

void VhallLiveMainDlg::HandleSendNotice(QEvent* event) {
   CustomOnHttpResMsgEvent* customEvent = dynamic_cast<CustomOnHttpResMsgEvent*>(event);
   if (customEvent) {
      int libCode = 0;
      std::string msg;
      InteractAPIManager::ParamHttpResp(customEvent->mMsg.toStdString(), libCode, msg);
      if (libCode != 200) {
         FadeOutTip(QString::fromStdString(msg), TipsType_Error);
      }
      else {
         ui.tabWidget->AppendAnnouncement(customEvent->mData, QString());
      }
   }
}

void VhallLiveMainDlg::CloseFloatChatWdg() {
   ui.tabWidget->slotCloseShareDlg();
}

void VhallLiveMainDlg::JSCallQtUserOnline(QString param) {
   TRACE6("%s start\n", __FUNCTION__);
   VhallAudienceUserInfo Info;
   QString strEvent = AnalysisOnOffLine(param, &Info);
   createMemberListDlg();
   if (NULL != ui.tabWidget->mpMemberListDlg) {
      ui.tabWidget->mpMemberListDlg->NtfUserOnline(Info);
   }
}

void VhallLiveMainDlg::JsCallQtMsg(QString param) {
   TRACE6("%s \n", __FUNCTION__);
   VH::CComPtr<ICommonData> pCommonData;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
   UserOnLineParam lineParam;
   pCommonData->ParamToLineParam(param, lineParam);

   VhallAudienceUserInfo Info;
   Info.role = lineParam.userRole.toStdWString();
   Info.userId = lineParam.userId.toStdWString();
   Info.userName = lineParam.userName.toStdWString();
   Info.gagType = lineParam.is_banned == true ? VhallShowType_Allow : VhallShowType_Hide;
   Info.headImage = lineParam.headImage.replace("https","http").toStdWString();
   Info.mDevType = lineParam.mDevType;
   Info.miUserCount = lineParam.mUV;

   if (lineParam.type == "disable" || lineParam.type == "permit") {
      Event oEvent;
      oEvent.m_eventType = e_eventType_msg;
      oEvent.m_eMsgType = lineParam.type == "disable" ? e_RQ_UserProhibitSpeakOneUser : e_RQ_UserAllowSpeakOneUser;
      oEvent.m_oUserInfo = Info;
      oEvent.m_oUserInfo.userName = Info.userName;
      QJsonObject eventJson = VHAliveInteraction::MakeEventObject(&oEvent);
      if (m_pLogic) {
         QCoreApplication::postEvent(m_pLogic, new CustomSocketIOMsg(CustomEvent_CustomRecvSocketIOMsgEvent, eventJson), Qt::LowEventPriority);
      }
   } else if (lineParam.type == "docFullscreen") {                    
      if (lineParam.is_full) {
         ui.horizontalLayout_docWebView->removeWidget(mDocCefWidget);
         QRect screen_geometry_rect = QApplication::desktop()->screenGeometry(this);
         mFullDocWdg->setGeometry(screen_geometry_rect);
         mFullDocWdg->InsertDoc(mDocCefWidget);
         mFullDocWdg->show();
         mFullDocWdg->repaint();         
      }
      else {
         mFullDocWdg->RemoveDoc(mDocCefWidget);
         mFullDocWdg->repaint();
         mFullDocWdg->hide();
         ui.horizontalLayout_docWebView->addWidget(mDocCefWidget);
         show();
         raise();
         setFocus();
         activateWindow();
      }
   }
   else if (lineParam.type == "docmouseenter") {
      if (lineParam.is_full) {
         if (!ui.widget_smallRender->isHidden() && mSmallWdgType == SmallWndType_Doc) {
            if (mLiveContentCtrlPtr) {
               QPoint pos = ui.widget_smallRender->pos();
               mLiveContentCtrlPtr->move(pos);
               mLiveContentCtrlPtr->show();
            }
         }
      }
   }
   else if (lineParam.type == "esc" && mSmallWdgType == SmallWndType_Render) {
      slot_PressKeyEsc();
   }
   else if (lineParam.type == "page") {
      if (lineParam.pageIndex == 1) {
         ui.widget_leftTool->SetDocBtnSelect();
      }
      else {
         ui.widget_leftTool->SetWBBtnSelect();
      }
   }
   TRACE6("%s end\n", __FUNCTION__);
}

void VhallLiveMainDlg::keyPressEvent(QKeyEvent *event) {
   switch (event->key())
   {
   case Qt::Key_Escape:
      break;
   default:
      //if (mpDocWebEngineView) {
      //   mpDocWebEngineView->setFocus();
      //   mpDocWebEngineView->raise();
      //   mpDocWebEngineView->activateWindow();
      //}
      QWidget::keyPressEvent(event);
   }
}

void VhallLiveMainDlg::JSCallQtUserOffline(QString param) {
   TRACE6("%s start\n", __FUNCTION__);
   VhallAudienceUserInfo Info;
   QString strEvent = AnalysisOnOffLine(param, &Info);
   createMemberListDlg();
   VH::CComPtr<ICommonData> pCommonData;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
   ClientApiInitResp info;
   pCommonData->GetInitApiResp(info);
   if (NULL != ui.tabWidget->mpMemberListDlg && info.user_id != Info.userId) {
      ui.tabWidget->mpMemberListDlg->NtfUserOffLine(QString::fromStdWString(Info.userId), Info.miUserCount);
   }
}

void VhallLiveMainDlg::JsCallQtOpenUrl(QString url) {
   TRACE6("%s start\n", __FUNCTION__);
   QDesktopServices::openUrl(url);
}

void VhallLiveMainDlg::slot_OnMicVolumeShow() {
   float audioMag = 0.0f;
   float audioPeak = 0.0f;
   float audioMax = 0.0f;

   float workVol, workMax, rmsScale, maxScale, peakScale;
   float curTopVolume;

   VH::CComPtr<IOBSControlLogic> pOBSControlLogic;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IOBSControl, IID_IOBSControlLogic, pOBSControlLogic, return);
   if (pOBSControlLogic) {
      pOBSControlLogic->GetAudioMeter(audioMag, audioPeak, audioMax);
      curTopVolume = audioMag;
      workVol = min(VOL_MAX, max(VOL_MIN, curTopVolume)); // Bound volume levels
      workMax = min(VOL_MAX, max(VOL_MIN, audioMax));
      rmsScale = (MicDBtoLog(workVol) - mic_minLinear) / (mic_maxLinear - mic_minLinear); // Convert dB to logarithmic then to linear scale [0, 1]
      maxScale = (MicDBtoLog(workMax) - mic_minLinear) / (mic_maxLinear - mic_minLinear);
      peakScale = (MicDBtoLog(audioPeak) - mic_minLinear) / (mic_maxLinear - mic_minLinear);
   }
   mMicVolume = rmsScale * 100;
   ui.widget_bottomTool->SetVolume(rmsScale * 100);
   if (mDeviceTestingWdg && !mDeviceTestingWdg->isHidden()) {
      mDeviceTestingWdg->SetMicVolume(mMicVolume);
   }

   if (mPerformanceStatisticsPtr) {
      ui.widget_bottomTool->SetCPU(mPerformanceStatisticsPtr->GetCPUValue());
      mNetDevDataCount++;
      if (mNetDevDataCount >= 20) {
         ui.widget_ToolBarWidget->logCPUAndMemory(mPerformanceStatisticsPtr->GetCPUValue());
         mNetDevDataCount = 0;
      }
   }
}


int  VhallLiveMainDlg::GetMixAudioVolume() {
   return mMicVolume;
}
void VhallLiveMainDlg::slot_RunningState(int cpu, int netSend, int netRecv) {
   ////实时更新音量图标

   ui.widget_bottomTool->SetCPU(cpu);
   ui.widget_ToolBarWidget->logCPUAndMemory(cpu);
   mCPUValue = cpu;
}

void VhallLiveMainDlg::slot_PressKeyEsc() {
   TRACE6("%s",__FUNCTION__);
   QString funPage = QString("QtCallFunctionPage(%1)").arg(eDocCtrlType_DocNormal_Size);
   if (mDocCefWidget) {
      mDocCefWidget->AppCallJsFunction(funPage);
   }
}

void VhallLiveMainDlg::slot_changeProfile(int type) {
   if (type == TipsType_Bitrate_Down) {
      VH::CComPtr<IOBSControlLogic> pObsControlLogic = NULL;
      DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IOBSControl, IID_IOBSControlLogic, pObsControlLogic, return, ASSERT(FALSE));
      if (pObsControlLogic) {
         pObsControlLogic->RateControlSwitch(false);
      }
   }
}

void VhallLiveMainDlg::slot_ClickedMin() {
   //this->showNormal();
   this->showMinized();
}

void VhallLiveMainDlg::slot_mouseMoved() {
   if (mbIsMaxSize) { //显示正常态
      QRect rect = QApplication::desktop()->availableGeometry(this);
      mbIsMaxSize = false;
      int moveX = 0;
      int moveY = 0;

      QPoint pos = QCursor::pos();
      moveX = (rect.width() - mNormalRect.width()) / 2;
      int iWidth = mNormalRect.width();
      int iHeight = mNormalRect.height();

      mNormalRect.setX(pos.x() - 200);
      mNormalRect.setY(pos.y());
      mNormalRect.setWidth(iWidth);
      mNormalRect.setHeight(iHeight);
      this->setGeometry(mNormalRect);
      TRACE6("%s moveX:%d moveY:%d  rect.x():%d ", __FUNCTION__, pos.x(), pos.y(), rect.x());
      ui.widget_title->SetPressedPoint(QPoint(200, pos.y()));
   }
}

void VhallLiveMainDlg::OnRecvMsg(std::string fun_name, std::string cus_msg) {
   CustomRecvMsgEvent* event = new CustomRecvMsgEvent(CustomEvent_LibCefMsg, QString::fromStdString(cus_msg));
   QApplication::postEvent(this, event);
}

void VhallLiveMainDlg::OnWebViewDestoryed() {

}

void VhallLiveMainDlg::OnTitleChanged(const int id, std::string title_name) {
   VH::CComPtr<ICommonData> pCommonData;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
   ClientApiInitResp initResp;
   pCommonData->GetInitApiResp(initResp);
   QString title = QString::fromStdString(title_name);
   if (initResp.player == FLASH_LIVE_CREATE_TYPE && (title.contains("flash", Qt::CaseInsensitive) || title.contains("player", Qt::CaseInsensitive))) {
      LoadDocUrl(mDocUrl);
   }
}

void VhallLiveMainDlg::OnWebViewLoadEnd() {
   ui.widget_docLoading->hide();
   mDocLoadTimer.stop();
   mLoadCount = 0;
   VH::CComPtr<ICommonData> pCommonData;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
   ClientApiInitResp initResp;
   pCommonData->GetInitApiResp(initResp);
   if (initResp.player == H5_LIVE_CREATE_TYPE) {
      mDocLoadTimer.start(500);
      //if (ui.widget_leftTool->IsClickedWB()) {
      //   QString funPage = QString("QtCallFunctionPage(%1)").arg(eDocCtrlType_Broad);
      //   mDocCefWidget->AppCallJsFunction(funPage);
      //}
   }
   //if (mSmallWdgType == SmallWndType_Render) {
   //   slot_OnClickedDoc();
   //}
   //else 
   if (mSmallWdgType == SmallWndType_Doc) {
      QString funPage = QString("QtCallFunctionPage(%1)").arg(eDocCtrlType_Doc);
      mDocCefWidget->AppCallJsFunction(funPage);
      mDocLoadTimer.start(500);
   }

}

void VhallLiveMainDlg::ReleaseDocWebView() {
   if (mDocCefWidget) {
      delete mDocCefWidget;
      mDocCefWidget = nullptr;
   }
}

void VhallLiveMainDlg::ReleaseChatWebView() {
   ui.tabWidget->ReleaseChatWebView();
}

void VhallLiveMainDlg::slotMemberClicked() {
   VH::CComPtr<IMainUILogic> pMainUILogic;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IMainUI, IID_IMainUILogic, pMainUILogic, return, ASSERT(FALSE));

   QWidget* pMainUI = NULL;
   pMainUILogic->GetShareUIWidget((void**)&pMainUI, eLiveType_Live);
   if (NULL != ui.tabWidget->mpMemberListDlg) {
      if (ui.tabWidget->mpMemberListDlg->isVisible()) {
         ui.tabWidget->ShowMemberFromDesktop(false,QPoint());
      }
      else {
         QPoint pos;
         QRect rect = QApplication::desktop()->availableGeometry(pMainUI);
         int ix = rect.x() + (rect.width() - ui.tabWidget->width()) / 2;
         int iy = rect.y() + (rect.height() - ui.tabWidget->height()) / 2;
         pos.setX(ix);
         pos.setY(iy);
         ui.tabWidget->ShowMemberFromDesktop(true,pos);
         //ui.tabWidget->mpMemberListDlg->show(QApplication::desktop()->availableGeometry(pMainUI));
      }
   }
}

void VhallLiveMainDlg::slot_OnFresh() {
   if (nullptr != ui.tabWidget) {
      ui.tabWidget->FreshWebView();
   }
   LoadDocUrl(mDocUrl);
   if (!m_bIsStreamStart) {
      VH::CComPtr<ICommonData> pCommonData;
      DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
      ClientApiInitResp initResp;
      pCommonData->GetInitApiResp(initResp);
      if (initResp.player == H5_LIVE_CREATE_TYPE && !ui.widget_smallRender->isHidden()) {
         ui.widget_leftTool->ReInitDocSelectStatus();
      }
   }
}

void VhallLiveMainDlg::slotChatClicked() {
   TRACE6("%s \n", __FUNCTION__);
   if (nullptr != ui.tabWidget)
      ui.tabWidget->slotChatClicked();
}

void VhallLiveMainDlg::HandleCustomEvent(QEvent* event) {
   if (event) {
      CustomRecvMsgEvent* msgEvent = (CustomRecvMsgEvent*)event;
      if (msgEvent) {
         QString msg = msgEvent->msg;
         if (nullptr != ui.tabWidget) {
            ui.tabWidget->RunJavaScrip(msg);
         }
         if (mDocCefWidget) {
            mDocCefWidget->AppCallJsFunction(msg);
         }
      }
   }
}

void VhallLiveMainDlg::HideCaptureSrcUi() {
   ui.widget_leftTool->SetDesktopShow(false);
}