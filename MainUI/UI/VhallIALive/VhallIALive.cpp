#include "VhallIALive.h"
#include "vhAliveinteraction.h"
#include "CInterActiveCtrlWdg.h"
#include <Windows.h>
#include <string>
#include <ostream>
#include <VhallIALiveSettingDlg.h>
#include <QDateTime>
#include <QDesktopServices>
#include <QCoreApplication>
#include <QDesktopWidget>
#include <QGuiApplication>
#include <QScreen> 
#include <QtMath>
#include <QFileInfo>
#include <QTimer> 
#include <QMutexLocker> 
#include "vhproxytest.h"
#include "vhallsharedqr.h"
#include "DebugTrace.h"
#include "vhdesktopcameralist.h"
#include "ConfigSetting.h"
#include "pathmanager.h"
#include "AlertTipsDlg.h"
#include "vhallwaiting.h"
#include "ICommonData.h"
#include "Msg_VhallRightExtraWidget.h"
#include "CRPluginDef.h"
#include "MainUIIns.h"
#include "Msg_MainUI.h"
#include "VhallRenderWdg.h"
#include "vhfadeouttipwidget.h"
#include "InviteJoinWidget.h"
#include "Msg_OBSControl.h"
#include "Msg_OBSControl.h"
#include "IOBSControlLogic.h"
#include "suspensionDlg.h"
#include "vlive_def.h"   
#include "pub.Struct.h"
#include "ApplyJoinWdg.h"
#include "VSSAPIManager.h"
#include "MemberParam.h"
#include "InteractAPIManager.h"
#include "vhdesktopsharingui.h"
#include "PerformanceStatistics.h"
#include "IWinPCapData.h"
#include "pathManage.h"
#include <psapi.h>

using namespace vlive;

VhallIALive::VhallIALive(bool showTeaching, QWidget *parent)
   : CAliveDlg(parent)
   , mpHttpResponseTips(NULL)
   , mCurrentLayout(-1)
   , bFirstConnected(true)
   , mCPUCheckTimeOut(MIN_CPU_CHECK_TIME)
   , mIsShowTeachingWidget(showTeaching)
{
   ui.setupUi(this);
   setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowSystemMenuHint | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint);
   ui.widget_title->SetTitleType(eVhallIALive);
   ui.widget_leftTool->setEnablePluginBtn(false);
   this->setFocusPolicy(Qt::ClickFocus);
   ui.widget_2->hide();
   connect(ui.widget_title, &VhallLiveTitle::sig_ClickedClose, this, &VhallIALive::slot_OnClose);
   connect(ui.widget_title, &VhallLiveTitle::sig_ClickedFresh, this, &VhallIALive::slot_OnFresh);
   connect(ui.widget_title, &VhallLiveTitle::sig_ClickedFullSize, this, &VhallIALive::slot_OnFullSize);
   connect(ui.widget_title, &VhallLiveTitle::sig_ClickedMinSize, this, &VhallIALive::slot_OnMinSize);
   connect(ui.widget_title, &VhallLiveTitle::sig_ClickedSetting, this, &VhallIALive::slot_OnSetting);
   connect(ui.widget_title, &VhallLiveTitle::sig_ClickedShare, this, &VhallIALive::slot_OnShare);
   connect(ui.widget_title, &VhallLiveTitle::sig_OnCopy, this, &VhallIALive::slot_OnCopy);
   connect(ui.widget_title, &VhallLiveTitle::Sig_OnClickedStartLiveCtrlBtn, this, &VhallIALive::slot_BtnStreamClicked);
   connect(ui.widget_title, &VhallLiveTitle::sigToSpeakClicked, this, &VhallIALive::slot_OnToSpeakClicked);
   connect(ui.widget_title, &VhallLiveTitle::sigCancelClicked, this, &VhallIALive::slot_CancleApplyToSpeak);
   connect(ui.widget_title, &VhallLiveTitle::sigRecordType, this, &VhallIALive::slot_commitRecord);
   connect(ui.widget_title, &VhallLiveTitle::sigFadeOutTip, this, &VhallIALive::slotFadOutTips);
   connect(ui.widget_title, &VhallLiveTitle::sigMouseMoved, this, &VhallIALive::slotMouseMoved);
   connect(ui.widget_title, &VhallLiveTitle::sigApplyTimeout, this, &VhallIALive::slotApplyTimeout);
   
   connect(&mSetMainViewTimer, &QTimer::timeout, this, &VhallIALive::slot_SetMainViewTimeout);
   connect(&mSetDeskTopMainViewTimer, &QTimer::timeout, this, &VhallIALive::slot_SetDesktopMainViewTimeout);
   connect(&mSetMediaFileMainViewTimer, &QTimer::timeout, this, &VhallIALive::slot_SetMediaFileMainViewTimeout);

   connect(&mRePublishLocalStreamTimer, &QTimer::timeout, this, &VhallIALive::slot_PublishLocalStreamTimeout);
   connect(&mRePublishDesktopStreamTimer, &QTimer::timeout, this, &VhallIALive::slot_PublishDesktopStreamTimeout);
   connect(&mRePublishMediaFileStreamTimer, &QTimer::timeout, this, &VhallIALive::slot_PublishMediaFileStreamTimeout);
   connect(&mReSetLocalCapture, &QTimer::timeout, this, &VhallIALive::slot_ReSetLocalCaptureTimeout);
   connect(&mCheckSubScribeStream, &QTimer::timeout, this, &VhallIALive::slot_CheckSubScribeTimeout);
   connect(&mReSetLayOutTimer, &QTimer::timeout, this, &VhallIALive::slot_ReSetLayoutTimeout);
   mReSetLayOutTimer.setSingleShot(true);

   int bg_head = 10;
   int bg_body = 30;
   int bg_tail = 10;
   QPixmap bgPixmap = QPixmap(":/interactivity/db");

   mpExitWaiting = new VhallWaiting(this);
   if (mpExitWaiting) {
      mpExitWaiting->SetPixmap(":/interactivity/cb");
      mpExitWaiting->Append(":/interactivity/c1");
      mpExitWaiting->Append(":/interactivity/c2");
      mpExitWaiting->Append(":/interactivity/c3");
      mpExitWaiting->Append(":/interactivity/c4");
      mpExitWaiting->Append(":/interactivity/c5");
      mpExitWaiting->Append(":/interactivity/c6");
      mpExitWaiting->Append(":/interactivity/c7");
      mpExitWaiting->Append(":/interactivity/c8");
      mpExitWaiting->Append(":/interactivity/c9");
      mpExitWaiting->Append(":/interactivity/c10");
      mpExitWaiting->Append(":/interactivity/c11");
      mpExitWaiting->Append(":/interactivity/c12");
   }
   mpStartWaiting = new VhallWaiting(this);
   if (mpStartWaiting) {
      mpStartWaiting->SetCerclePos(55, 50);
      mpStartWaiting->SetPixmap(":/interactivity/img/interactivity/startLive.png");
      mpStartWaiting->Append(":/interactivity/c1");
      mpStartWaiting->Append(":/interactivity/c2");
      mpStartWaiting->Append(":/interactivity/c3");
      mpStartWaiting->Append(":/interactivity/c4");
      mpStartWaiting->Append(":/interactivity/c5");
      mpStartWaiting->Append(":/interactivity/c6");
      mpStartWaiting->Append(":/interactivity/c7");
      mpStartWaiting->Append(":/interactivity/c8");
      mpStartWaiting->Append(":/interactivity/c9");
      mpStartWaiting->Append(":/interactivity/c10");
      mpStartWaiting->Append(":/interactivity/c11");
      mpStartWaiting->Append(":/interactivity/c12");
   }
   connect(&mCheckSpeakUserTimeOut, &QTimer::timeout, this, &VhallIALive::slot_CheckSpeakUserTimeOut);
   connect(&mRoomReConnectTimer, &QTimer::timeout, this, &VhallIALive::slot_RoomReConnectTimer);
   mCheckSpeakUserTimeOut.setSingleShot(true);
   SetMainViewBackGroud(MAIN_VIEW_IS_NOT_START);

   //全屏共享 上部 工具栏
   m_pScreenShareToolWgd = new VHDesktopSharingUI(this);
   if (m_pScreenShareToolWgd) {
      m_pScreenShareToolWgd->SetLiveState(false);
      m_pScreenShareToolWgd->hide();
      m_pScreenShareToolWgd->SetPlayerShow(false);
      connect(m_pScreenShareToolWgd, &VHDesktopSharingUI::sigToStopDesktopSharing, this, &VhallIALive::slot_BtnScreenClicked);
      connect(m_pScreenShareToolWgd, &VHDesktopSharingUI::sigCameraClicked, this, &VhallIALive::slot_CameraBtnClicked);
      connect(m_pScreenShareToolWgd, &VHDesktopSharingUI::sigPlayer, this, &VhallIALive::slot_PlayerButtonClicked);
      connect(m_pScreenShareToolWgd, &VHDesktopSharingUI::sigSettingClicked, this, &VhallIALive::slot_OnSetting);
      connect(m_pScreenShareToolWgd, &VHDesktopSharingUI::sigLiveClicked, this, &VhallIALive::slot_BtnStreamClicked);
      connect(m_pScreenShareToolWgd, &VHDesktopSharingUI::sigMicClicked, this, &VhallIALive::slot_MicButtonClicked);
      connect(m_pScreenShareToolWgd, &VHDesktopSharingUI::sigChatClicked, ui.tabWidget, &CTabWidget::slotChatClicked);
      connect(m_pScreenShareToolWgd, &VHDesktopSharingUI::sigMemberClicked, this, &VhallIALive::slot_MemberList);
      connect(m_pScreenShareToolWgd, &VHDesktopSharingUI::sigInteractiveClicked, this, &VhallIALive::slot_LiveToolFromDesktopClicked);
      connect(m_pScreenShareToolWgd, &VHDesktopSharingUI::sigShareSelect, this, &VhallIALive::slot_OnShareSelect);
      connect(m_pScreenShareToolWgd, &VHDesktopSharingUI::sigFloatSwitchBtn, this, &VhallIALive::slot_OnFloatWndSwitch);
      
   }
   if (mDesktopCameraRenderWdgPtr == nullptr) {
      mDesktopCameraRenderWdgPtr = new DesktopCameraRenderWdg();
      connect(mDesktopCameraRenderWdgPtr, SIGNAL(sig_closeDesktopCameraRender()), this, SLOT(slot_closeDesktopCameraRender()));
   }
   ui.listWidgetMainView->hide();
   VH::CComPtr<ICommonData> pCommonData;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
   ClientApiInitResp respInitData;
   pCommonData->GetInitApiResp(respInitData);
   ui.widget_leftTool->ShowSharedButton(true);
   ui.widget_leftTool->ShowInsertVedio();
   if (respInitData.player == FLASH_LIVE_CREATE_TYPE) { 
      ui.widget_leftTool->ShowWhiteboardBtn(false);
      connect(ui.widget_leftTool, &LeftToolWdg::sigMemberBtnClicked, this, &VhallIALive::slot_MemberList);
   }
   else {  
      connect(ui.tabWidget, &CTabWidget::sig_DragMember, this, &VhallIALive::slot_changed);
      connect(ui.widget_leftTool, &LeftToolWdg::sig_OnClickedWhiteboard, this, &VhallIALive::slot_ClickedWhiteboard);
   }

   if (respInitData.role_name.compare(USER_HOST) == 0) {
      ui.widget_title->ShowStartLiveBtn(true);
      ui.widget_title->setRecordBtnShow(eDispalyCutRecord_Show);
   }
   else {
      ui.widget_title->ShowStartLiveBtn(false);
      ui.widget_title->setRecordBtnShow(eDispalyCutRecord_Hide);
   }

   connect(ui.widget_leftTool, &LeftToolWdg::sigClicked, this, &VhallIALive::slot_BtnScreenClicked);
   connect(ui.widget_leftTool, SIGNAL(sig_OnClickedDoc()), this, SLOT(slot_OnClickedDoc()));
   connect(ui.widget_leftTool, &LeftToolWdg::sig_OnClickedPluginUrl, this, &VhallIALive::slot_OnClickedPluginUrl);
   connect(ui.widget_leftTool, &LeftToolWdg::sig_InsertVedio, this, &VhallIALive::slot_OnPlayFileClicked);
   connect(ui.widget_leftTool, &LeftToolWdg::sigSharedBtnClicked, this, &VhallIALive::slot_OnShare);
   connect(ui.tabWidget, &CTabWidget::sig_JsCallOnOffLine, this, &VhallIALive::JsCallQtMsg);
   connect(ui.tabWidget, &CTabWidget::sig_FlashJsCallUserOnLine, this, &VhallIALive::JSCallQtUserOnline);
   connect(ui.tabWidget, &CTabWidget::sig_FlashJsCallUserOffLine, this, &VhallIALive::JSCallQtUserOffline);

   mIsFullSize = false;
   mNormalRect = this->geometry();
   QRect screen = QApplication::desktop()->screenGeometry(this);
   if (this->width() > screen.width()|| this->height() > screen.height()) {
      InitMaxSize();
   }

   ResetListWidgetSmallViewSize();
   mDocLoadingTimer = new QTimer(this);
   if (mDocLoadingTimer) {
      connect(mDocLoadingTimer, SIGNAL(timeout()), this, SLOT(slot_DocLoading()));
   }
   mDocLoadEndTimer = new QTimer(this);
   if (mDocLoadEndTimer) {
      connect(mDocLoadEndTimer, SIGNAL(timeout()), this, SLOT(slot_DocLoadEnd()));
      mDocLoadEndTimer->setSingleShot(true);
   }

   mPlayUIHideTimer = new QTimer(this);
   if (mPlayUIHideTimer) {
      mPlayUIHideTimer->setSingleShot(true);
      connect(mPlayUIHideTimer, SIGNAL(timeout()), this, SLOT(slot_HidePlayUI()));
   }

   mShowTipsWdgPtr = new ShowTipsWdg(this);
   if (mShowTipsWdgPtr) {
      mShowTipsWdgPtr->hide();
      connect(mShowTipsWdgPtr, SIGNAL(sig_removeItem()), this, SLOT(slot_ReSetTipsPos()));
      connect(mShowTipsWdgPtr, SIGNAL(sig_removeItem()), this, SLOT(slot_closeItem()));
      connect(mShowTipsWdgPtr, SIGNAL(sig_changeProfile(int)), this, SLOT(slot_changeProfile(int)));
   }

   mDesktopShowTipsWdgPtr = new ShowTipsWdg();
   if (mDesktopShowTipsWdgPtr) {
      mDesktopShowTipsWdgPtr->hide();
      connect(mDesktopShowTipsWdgPtr, SIGNAL(sig_removeItem()), this, SLOT(slot_ReSetTipsPos()));
      connect(mDesktopShowTipsWdgPtr, SIGNAL(sig_removeItem()), this, SLOT(slot_closeItem()));
      connect(mDesktopShowTipsWdgPtr, SIGNAL(sig_changeProfile(int)), this, SLOT(slot_changeProfile(int)));
      mDesktopShowTipsWdgPtr->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint | Qt::Tool | Qt::WindowStaysOnTopHint);
   }

   mPerformanceStatisticsPtr = new PerformanceStatistics();
   ui.widget_bottomTool->ShowPushStreamFps(false);
   ui.widget_bottomTool->ShowPushStreamBitRate(false);
   ui.widget_bottomTool->ShowPushStreamLostPacket(false);
   mpWCapStatTimerPtr = new QTimer(this);
   if (mpWCapStatTimerPtr) {
      connect(mpWCapStatTimerPtr, &QTimer::timeout, this, &VhallIALive::slot_OnNetWordDataStat);
      mpWCapStatTimerPtr->start(1000);
   }
   QString confPath = CPathManager::GetConfigPath();
   QString localProfile = ConfigSetting::writeValue(confPath, GROUP_DEFAULT, KEY_LOCAL_VIDEO_PROFILE, PROFILE_INDEX_STANDER);
   int openSubDoubleStream = ConfigSetting::ReadInt(confPath, GROUP_DEFAULT, SUB_DOUBLE_STREAM, 1);
   mbIsOpenSubDoubleStream = openSubDoubleStream == 1 ? true : false;

   QString upAddr = ConfigSetting::ReadString(CPathManager::GetToolConfigPath(), GROUP_DEFAULT, KEY_UP_ADDR, "");
   if (!upAddr.isEmpty()) {
      vlive::GetWebRtcSDKInstance()->SetDebugLogAddr(upAddr.toStdString());
   }
   mFullDocWdg = new FullDocWdg();
   if (mFullDocWdg) {
      connect(mFullDocWdg, SIGNAL(sig_PressKeyEsc()), this, SLOT(slot_PressKeyEsc()));
      mFullDocWdg->hide();
   }
   mConfigBroadCast = new VhallConfigBroadCast(this);
   TRACE6("%s end\n", __FUNCTION__);
}

VhallIALive::~VhallIALive(){
   disconnect(ui.widget_title, &VhallLiveTitle::sig_ClickedClose, this, &VhallIALive::slot_OnClose);
   if (mFullDocWdg) {
      delete mFullDocWdg;
      mFullDocWdg = nullptr;
   }
   if (mDesktopShareSelectWdg) {
      delete mDesktopShareSelectWdg;
      mDesktopShareSelectWdg = nullptr;
   }
   if (mDesktopCameraRenderWdgPtr) {
      delete mDesktopCameraRenderWdgPtr;
      mDesktopCameraRenderWdgPtr = nullptr;
   }
   if (mPerformanceStatisticsPtr) {
      mPerformanceStatisticsPtr->Stop();
      delete mPerformanceStatisticsPtr;
      mPerformanceStatisticsPtr = nullptr;
   }
   if (mpWCapStatTimerPtr) {
      mpWCapStatTimerPtr->stop();
   }

   ui.widget_title->StartTimeClock(FALSE);
   ui.widget_title->ReInit();
   if (mShowTipsWdgPtr) {
      mShowTipsWdgPtr->hide();
   }
   if (mPlayUIHideTimer) {
      mPlayUIHideTimer->stop();
   }
   if (mDocCefWidget) {
      mDocCefWidget->hide();
      ui.verticalLayout_mainWebView->removeWidget(mDocCefWidget);
      delete mDocCefWidget;
      mDocCefWidget = nullptr;
   }
   ui.tabWidget->Destroy();
   ConfigSetting::writeValue(CPathManager::GetConfigPath(), GROUP_DEFAULT, PUSH_DESKTOP_CAPTURE_OPEN, 0);
   mCheckSubScribeStream.stop();
   mSetMainViewTimer.stop();
   mSetDeskTopMainViewTimer.stop();
   mSetMediaFileMainViewTimer.stop();
   mRePublishLocalStreamTimer;
   mRePublishDesktopStreamTimer.stop();
   mRePublishMediaFileStreamTimer.stop();
   mReSetLocalCapture.stop();
   GetWebRtcSDKInstance()->DisConnetWebRtcRoom();
   mbIsDelete = true;
   TRACE6("%s start\n", __FUNCTION__);
   RemoveAllRender();
   if (mpExitWaiting) {
      delete mpExitWaiting;
      mpExitWaiting = NULL;
   }
   if (mpStartWaiting) {
      delete mpStartWaiting;
      mpStartWaiting = NULL;
   }
   mRoomReConnectTimer.stop();
   mCheckSpeakUserTimeOut.stop();
   if (mpVedioPlayUi) {
      delete mpVedioPlayUi;
      mpVedioPlayUi = NULL;
   }
   if (mMediaFilePlayWdg) {
      delete mMediaFilePlayWdg;
      mMediaFilePlayWdg = NULL;
   }
   if (mDesktopWdg) {
      delete mDesktopWdg;
      mDesktopWdg = NULL;
   }
   if (mpInviteJoinWdg) {
      delete mpInviteJoinWdg;
      mpInviteJoinWdg = NULL;
   }
   if (mpSettingDlg) {
      delete mpSettingDlg;
      mpSettingDlg = NULL;
   }

   if (m_pScreenShareToolWgd) {
      delete m_pScreenShareToolWgd;
      m_pScreenShareToolWgd = NULL;
   }
   ClearSpeakerUser();
   ReleaseDocWebView();
   if (NULL != mpHttpResponseTips){
      delete mpHttpResponseTips;
      mpHttpResponseTips = NULL;
   }
   if (mActiveParam.mbIsH5Live) {
      DestoryPaasSDKInstance();
   }
   else {
      DestroyWebRtcSDKInstance();
   }
   ClearApplyJoinMap();
   TRACE6("%s end\n", __FUNCTION__);
}

void VhallIALive::SetMainViewBackGroud(int state) {
   TRACE6("%s state:%d\n", __FUNCTION__, state);
   switch (state)
   {
      case MAIN_VIEW_IS_LIVING: {
         ui.widget_liveEnd->hide();
         ui.widget_liveNotStart->hide();
         break;
      }
      case MAIN_VIEW_IS_NOT_START: {
         ui.widget_liveEnd->hide();
         ui.widget_liveNotStart->show();
         break;
      }
      case MAIN_VIEW_IS_END: {
         ui.widget_liveEnd->show();
         ui.widget_liveNotStart->hide();
         break;
      }
      default:
         break;
   }
   TRACE6("%s end\n", __FUNCTION__);
}

void VhallIALive::ShowApplyJoinWnd() {
   QMap<QString, ApplyJoinWdg*>::iterator it = mApplyJoinWdg.begin();
   while (it != mApplyJoinWdg.end()) {
      it.value()->show();
      it++;
   }
}

void VhallIALive::ClearApplyJoinMap(){
   QStringList strList = mApplyJoinWdg.keys();
   ApplyJoinWdg * temp = NULL;
   for (int i = strList.count() - 1; i >= 0; i--) {
      temp = mApplyJoinWdg[strList[i]];
      if (NULL != temp) {
         delete temp;
         temp = NULL;
      }
   }
}

void VhallIALive::slot_DestroyApplyWdg(QString strId){
   ApplyJoinWdg * pWdg = NULL;
   pWdg = mApplyJoinWdg[strId];
   if (NULL != pWdg) {
      pWdg->hide();
      pWdg->deleteLater();
      pWdg = NULL;
   }
}

void VhallIALive::OnDevicelistNotify(const QList<VhallLiveDeviceInfo>& micList, const QList<VhallLiveDeviceInfo>& cameraList, const QList<VhallLiveDeviceInfo>& playerList, int type) {
   TRACE6("%s mic Num:%d camera Num:%d player Num:%d\n", __FUNCTION__, micList.size(), cameraList.size(), playerList.size());
   mMicList.clear();
   mCameraList.clear();
   mPlayerList.clear();
   for (int i = 0; i < micList.size(); i++) {
      VhallLiveDeviceInfo devInfo = micList.at(i);
      mMicList.push_back(devInfo);
   }
   for (int i = 0; i < cameraList.size(); i++) {
      VhallLiveDeviceInfo devInfo = cameraList.at(i);
      mCameraList.push_back(devInfo);
   }
   for (int i = 0; i < playerList.size(); i++) {
      VhallLiveDeviceInfo devInfo = playerList.at(i);
      mPlayerList.push_back(devInfo);
   }

   InteractAPIManager apiManager(mpMainUILogic);
   if (micList.size() > 0 && cameraList.size() > 0) {
      apiManager.HttpSendSetDevice(1);//1可上麦
   }
   else {
      apiManager.HttpSendSetDevice(2); //2不可上麦
   }

   switch (type) {
   case DevRequest_GetDevList: {
      QCoreApplication::postEvent(mpMainUILogic, new QEvent(CustomEvent_SelectGetDev));
      break;
   }
   case DevRequest_StartLive: {
      QCoreApplication::postEvent(mpMainUILogic, new QEvent(CustomEvent_StartLiveGetDev));
      break;
   }
   case DevRequest_InitSDK: {
      QCoreApplication::postEvent(mpMainUILogic, new QEvent(CustomEvent_InitVhallSDK));
      break;
   }
   default:
      break;
   }
}

void VhallIALive::OnDevChanged(const QString &cameraDevId, const int cameraIndex, const QString &micDevId, const int micIndex, const QString &playerDevId, const int playerIndex) {
   CustomDevChangedEvent* event = new CustomDevChangedEvent(CustomEvent_DevChanged, cameraDevId, cameraIndex, micDevId, micIndex, playerDevId, playerIndex);
   QCoreApplication::postEvent(mpMainUILogic, event);
}

void VhallIALive::OnChangeLayOutMode(LAYOUT_MODE mode) {
   mCurrentMode = GetCurrentLayoutMode(mode);
   SetLayOutMode(mCurrentMode);
}

void VhallIALive::Init(MainUILogic *obj, VHPaasSDKCallback* callback) {
   mpMainUILogic = obj;
   callbackReciver = callback;
   ui.widget_leftTool->CheckStartMode();
   //初始化设置用户信息。
   VH::CComPtr<ICommonData> pCommonData;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
   ClientApiInitResp respInitData;
   pCommonData->GetInitApiResp(respInitData);
   StartLiveUrl startLiveData;
   pCommonData->GetStartLiveUrl(startLiveData);
   std::wstring appVersion;
   pCommonData->GetCurVersion(appVersion);
   VSSRoomAttributes attributes;
   pCommonData->GetVSSRoomAttributes(attributes);
   if (respInitData.player == FLASH_LIVE_CREATE_TYPE) {
      QString chatUrl = respInitData.web_chat_url + "&version=" + QString::fromStdWString(appVersion) + "&webinar_id=" + startLiveData.stream_name;//访问连接  和 流Id
      mActiveParam.chatUrl = chatUrl;
      mActiveParam.handsUp = respInitData.handsUp;
      mActiveParam.mainShow = respInitData.mainShow;
      mActiveParam.watchLayout = respInitData.watchLayout;
      mActiveParam.definition = respInitData.definition;
      mActiveParam.speaker_list = respInitData.speaker_list;
      mActiveParam.join_uid = respInitData.join_uid;//saas对应的用户id
      mActiveParam.bIsLiving = respInitData.webinar_type == 1 ? true : false;  //判断活动是否还在直播中 data["webinar_type"].toInt() == 1 ? true : false;  //判断活动是否还在直播中
      //文档区域的url需要替换。
      QString docUrl = mActiveParam.chatUrl;
      mActiveParam.pluginUrl = docUrl.replace("chat.html", "doc.html");
   }
   else {
      VSSGetRoomInfo roomInfo;
      pCommonData->GetVSSRoomBaseInfo(roomInfo);
      mActiveParam.chatUrl = respInitData.web_chat_url + QString("&hide=1");
      mActiveParam.pluginUrl = respInitData.web_doc_url + QString("&hide=1");
      mActiveParam.handsUp = attributes.is_handsup;
      mActiveParam.mainShow = attributes.main_screen;
      mActiveParam.watchLayout = attributes.layout.isEmpty() ? "float" : attributes.layout;
      mActiveParam.definition = attributes.definition.isEmpty() ? "STR_VIDEO_PROFILE_480P_1" : attributes.definition;
      mActiveParam.speaker_list = attributes.speaker_list;
      mActiveParam.join_uid = roomInfo.third_party_user_id;//paas对应的用户id ,使用third_user_id,互动流中的id
      mActiveParam.bIsLiving = roomInfo.status == 1 ? true : false;
   }

   mActiveParam.bExitToLivelist = !pCommonData->GetJoinActiveState();
   mActiveParam.user_id = respInitData.user_id;
   mActiveParam.role_name = respInitData.role_name;
   mActiveParam.device_audio = respInitData.device_audio;
   mActiveParam.device_video = respInitData.device_video;
   mActiveParam.currPresenter = respInitData.curr_presenter.isEmpty() ? mActiveParam.mainShow : respInitData.curr_presenter;
   mActiveParam.bizRole = respInitData.bizRole;
   mActiveParam.reportUrl = respInitData.reportUrl;
   mActiveParam.against_url = respInitData.against_url;
   mActiveParam.room_token = respInitData.room_token;
   mActiveParam.invite_list = respInitData.invite_list;
   mActiveParam.thirdPushStreamUrl = respInitData.thirdStreamToken;
   mActiveParam.msgToken = startLiveData.msg_token;
   mActiveParam.userNickName = respInitData.nick_name;
   mActiveParam.imageUrl = respInitData.avatar;
   mActiveParam.streamName = startLiveData.stream_name;
   mActiveParam.webinarName = respInitData.webinar_name;
   mActiveParam.live_time = respInitData.live_time;
   mActiveParam.vss_token = respInitData.vss_token;
   mActiveParam.mbIsH5Live = respInitData.player == FLASH_LIVE_CREATE_TYPE ? false : true;
   mActiveParam.vss_room_id = respInitData.vss_room_id;
   TRACE6("%s mActiveParam.mainShow %s current user role_name:%s mActiveParam.handsUp:%d  mActiveParam.bIsLiving %d\n", __FUNCTION__, mActiveParam.mainShow.toStdString().c_str(), respInitData.role_name.toStdString().c_str(), mActiveParam.handsUp, mActiveParam.bIsLiving);
   //当前用户是主持人
   if (mActiveParam.role_name.compare(USER_HOST) == 0) {
      ui.widget_title->ShowStartLiveBtn(true);
      m_pScreenShareToolWgd->SetLiveShow(true);
      ui.tabWidget->mpMemberListDlg->InitLiveTypeInfo(true,true);
   }
   else {
      ui.tabWidget->mpMemberListDlg->InitLiveTypeInfo(true, false);
      ui.widget_title->ShowStartLiveBtn(false);
      m_pScreenShareToolWgd->SetLiveShow(false);
      //只有以非主持人身份进入的用户，才创建被邀请界面。
      mpInviteJoinWdg = new InviteJoinWidget(this);
      if (mpInviteJoinWdg) {
         mpInviteJoinWdg->InitTimeOut(10);
         mpInviteJoinWdg->hide();
         connect(mpInviteJoinWdg, &InviteJoinWidget::Sig_GuestAgreeHostInvite, this, &VhallIALive::slot_GuestAgreeHostInvite);
         connect(mpInviteJoinWdg, &InviteJoinWidget::Sig_GuestRefuseHostInvite, this, &VhallIALive::slot_GuestRefuseHostInvite);
      }
   }

   ui.widget_title->SetLiveInfo(mActiveParam.streamName, mActiveParam.webinarName, mActiveParam.userNickName);
   createMemberListDlg();
   if (ui.tabWidget->mpMemberListDlg) {
      ui.tabWidget->mpMemberListDlg->setHansUpStates(eHandsUp_open == mActiveParam.handsUp);
   }

   if (!mpSettingDlg) {
      bool isHost = mActiveParam.role_name.compare(USER_HOST) == 0 ? true : false;
      mpSettingDlg = new VhallIALiveSettingDlg(this, isHost, "");
      if (mpSettingDlg) {
         connect(mpSettingDlg, &VhallIALiveSettingDlg::sig_PlayerBtnClicked, this, &VhallIALive::slot_PlayerButtonClicked);
         connect(mpSettingDlg, &VhallIALiveSettingDlg::sigVolumnChanged, this, &VhallIALive::slot_PlayerVolumnChanged);
         connect(mpSettingDlg, &VhallIALiveSettingDlg::sigProfileChanged, this, &VhallIALive::slot_SettingDlgProfileChanged);
         mpSettingDlg->OnOpenPlayer(false);
         mpSettingDlg->RegisterListener(this);
         mpSettingDlg->RegisterPlayMediaFileObj(this);
         //默认选择
         mpSettingDlg->slot_SelectTiledMode();
         if (mActiveParam.role_name.compare(USER_HOST) == 0) {
            mActiveParam.watchLayout = "tiled";
            mpSettingDlg->SetLayOutMode(GetLayOutMode(mActiveParam.watchLayout));
         }
      }
      wstring wzGurVersion;
      VH::CComPtr<ICommonData> pCommonData;
      DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
      if (pCommonData) {
         pCommonData->GetCurVersion(wzGurVersion);
         if (mpSettingDlg) {
            mpSettingDlg->SerVersion(QString::fromStdWString(wzGurVersion.c_str()));
         }
      }
      if (mActiveParam.mainShow != mActiveParam.join_uid) {
         ConfigSetting::writeValue(CPathManager::GetConfigPath(), GROUP_DEFAULT, PUSH_DESKTOP_CAPTURE_OPEN, 0);
         mpSettingDlg->ShowDesktopCaptrue(false);
      }
   }
   mDeviceGetThread.SetSubscribe(this);
   mDeviceGetThread.SetRequestType(DevRequest_InitSDK);
   CheckGuestUserManagerPermisson();
   InitRenderView(mActiveParam);  //初始化当前房间内麦上成员信息
   InitVhallSDK();   //创建room，注册监听，进行connect
   InitMainPlugin(mActiveParam.pluginUrl);
   ui.tabWidget->InitChatWebView(mActiveParam.chatUrl);

   InitLeftBtnEnableState(mActiveParam.mainShowRole == USER_USER);
   ui.widget_leftTool->SetDocBtnClicked(true);
}

void VhallIALive::slot_commitRecord(const int& eRequestType)
{
   static QString strURL = ConfigSetting::ReadString(CPathManager::GetToolConfigPath(), GROUP_DEFAULT, KEY_URL_DOMAIN, domainUrl);
   QString format = QString("%1/api/client/v1/clientapi/cut-record?webinar_id=%2&status=%3&send=0&token=%4")
      .arg(strURL.toStdString().c_str()).arg(mActiveParam.streamName.toStdString().c_str())
      .arg(eRequestType).arg(mActiveParam.msgToken.toStdString().c_str());// + m_token";

   QObject* obj = mpMainUILogic;
   TRACE6("%s url:%s\n", __FUNCTION__, format.toStdString().c_str());
   HTTP_GET_REQUEST httpRequest = VSSAPIManager::GetHttpRequest(format.toStdString());
   httpRequest.SetHttpPost(true);
   GetHttpManagerInstance()->HttpGetRequest(httpRequest, [eRequestType, obj](const std::string& msg, int code, const std::string userData) {
      VhallIALive::commitRecordRS(msg, code, userData, eRequestType, obj);
      TRACE6("%s code:%d msg:%s\n", __FUNCTION__, code, msg.c_str());
   });
}

void VhallIALive::commitRecordRS(const std::string& msg, int code, const std::string userData, const int& eRequestType, QObject* obj)
{
   QJsonObject ObjValue = CPathManager::GetJsonObjectFromString(QString::fromStdString(msg));
   QString strCode = CPathManager::GetObjStrValue(ObjValue, "code");
   if (strCode.compare("200") == 0) {
      if (ObjValue.value("data").isObject()) {
         QJsonObject objData = ObjValue.value("data").toObject();
         QString webinar_id = CPathManager::GetObjStrValue(objData, "webinar_id");
         int status = CPathManager::GetObjStrValue(objData, "status").toInt();
         RecordEvent* customEvent = new RecordEvent(CustomEvent_RecordEvent, status);
         if (customEvent && obj) {
            QCoreApplication::postEvent(obj, customEvent);
         }
      }
   }
   else
   {
      QString msg = CPathManager::GetObjStrValue(ObjValue, "msg");
      RecordEventErr* customEvent = new RecordEventErr(CustomEvent_RecordEventErr, msg);
      if (customEvent && obj) {
         QCoreApplication::postEvent(obj, customEvent);
      }
   }
}

LAYOUT_MODE VhallIALive::GetLayOutMode(QString& strLayoutMode) {
   if (strLayoutMode.compare("float") == 0) {
      return LAYOUT_MODE_FLOAT;
   }
   else if (strLayoutMode.compare("tiled") == 0) {
      return LAYOUT_MODE_TILED;
   }
   else if (strLayoutMode.compare("grid") == 0) {
      return LAYOUT_MODE_GRID;
   }
   return LAYOUT_MODE_TILED;
}

void VhallIALive::ResetListWidgetSmallViewSize() {
   if (ui.horizontalLayout_smallRender->count() == 0) {
      ui.widget_2->hide();
      ui.widget_3->hide();
   }
   else if(!mbIsFullDoc){
      ui.widget_2->show();
      ui.widget_3->show();
   }

   QApplication::postEvent(this,new QEvent(CustomEvent_ReSizeCenterRender));
   slot_ReSetTipsPos();
}

bool VhallIALive::ParamRtcInfoToUserInfo(QString user_data, VhallAudienceUserInfo& info) {
    bool bRet = false;
   //obj["join_uid"] = mActiveParam.join_uid;
   //obj["join_uname"] = mActiveParam.userNickName;
   //obj["join_role"] = mActiveParam.role_name == USER_HOST ? QString("host") : QString("guest");
   //obj["avatar"] = QString("http:") + mActiveParam.imageUrl;
   //obj["nickName"] = mActiveParam.userNickName;
   //obj["role"] = mActiveParam.role_name == USER_HOST ? QString("host") : QString("guest");
   //obj["role"] = mActiveParam.role_name == USER_HOST ? QString("1") : QString("4");
   QJsonParseError json_error;
   QJsonDocument parse_doucment = QJsonDocument::fromJson(QByteArray(user_data.toStdString().c_str(), user_data.toStdString().length()), &json_error);
   if (json_error.error == QJsonParseError::NoError) {
      if (parse_doucment.isObject()) {
         QJsonObject obj = parse_doucment.object();
         if (obj.contains("join_uid")) {
            info.userId = obj["join_uid"].toString().toStdWString();
            bRet = true;
         }
         if (obj.contains("join_uname")) {
            info.userName = obj["join_uname"].toString().toStdWString();
            bRet = true;
         }
         if (obj.contains("nickName")) {
            info.userName = obj["nickName"].toString().toStdWString();
            bRet = true;
         }
         if (obj.contains("join_role")) {
            info.role = obj["join_role"].toString().toStdWString();
            bRet = true;
         }
         if (obj.contains("role")) {
            QString role_name = obj["role"].toString();
            if (role_name == "1") {
               info.role = QString::fromStdString(USER_HOST).toStdWString();
            }
            else if (role_name == "2") {
               info.role = QString::fromStdString(USER_USER).toStdWString();
            }
            else if (role_name == "3") {
               info.role = QString::fromStdString(USER_ASSISTANT).toStdWString();
            }
            else if (role_name == "4") {
               info.role = QString::fromStdString(USER_GUEST).toStdWString();
            }
            else {
               info.role = obj["role"].toString().toStdWString();
            }
            bRet = true;
         }
      }
   }
   return bRet;
}

void VhallIALive::ParamJsonToUserInfo(std::string json, QString& uid, QString& nickname, QString role) {
   QJsonParseError json_error;
   QJsonDocument parse_doucment = QJsonDocument::fromJson(QByteArray(json.c_str(), json.length()), &json_error);
   if (json_error.error == QJsonParseError::NoError){
      if (parse_doucment.isObject()){
         QJsonObject obj = parse_doucment.object();
         if (obj.contains(WEBRTC_JOIN_UID)){
            QJsonValue uid_value = obj.take(WEBRTC_JOIN_UID);
            if (uid_value.isString()){
               uid = uid_value.toString();
            }
         }
         if (obj.contains(WEBRTC_JOIN_UNAME)){
            QJsonValue uname_value = obj.take(WEBRTC_JOIN_UNAME);
            if (uname_value.isString()){
               nickname = uname_value.toString();
            }
         }
         if (obj.contains(WEBRTC_JOIN_UROLE)){
            QJsonValue role_value = obj.take(WEBRTC_JOIN_UROLE);
            if (role_value.isString()){
               role = role_value.toString();
            }
         }
      }
   }
}

QString VhallIALive::GetLayoutStringName(LayoutMode mode) {
   QString layout;
   if (mActiveParam.mbIsH5Live) {
      switch (mode) {
         case CANVAS_LAYOUT_PATTERN_GRID_1: {
            layout = "CANVAS_LAYOUT_PATTERN_GRID_1";
            break;
         }
         case CANVAS_LAYOUT_PATTERN_GRID_2_H: {
            layout = "CANVAS_LAYOUT_PATTERN_GRID_2_H";
            break;
         }
         case CANVAS_LAYOUT_PATTERN_GRID_3_E: {
            layout = "CANVAS_LAYOUT_PATTERN_GRID_3_E";
            break;
         }
         case CANVAS_LAYOUT_PATTERN_GRID_3_D: {
            layout = "CANVAS_LAYOUT_PATTERN_GRID_3_D";
            break;
         }
         case CANVAS_LAYOUT_PATTERN_GRID_4_M: {
            layout = "CANVAS_LAYOUT_PATTERN_GRID_4_M";
            break;
         }
         case CANVAS_LAYOUT_PATTERN_GRID_5_D: {
            layout = "CANVAS_LAYOUT_PATTERN_GRID_5_D";
            break;
         }
         case CANVAS_LAYOUT_PATTERN_GRID_6_E: {
            layout = "CANVAS_LAYOUT_PATTERN_GRID_6_E";
            break;
         }
         case CANVAS_LAYOUT_PATTERN_FLOAT_6_5D: {
            layout = "CANVAS_LAYOUT_PATTERN_FLOAT_6_5D";
            break;
         }
         case CANVAS_LAYOUT_PATTERN_TILED_6_1T5D: {
            layout = "CANVAS_LAYOUT_PATTERN_TILED_6_1T5D";
            break;
         }
      }
   }
   else {
      switch (mode) {
      case CANVAS_LAYOUT_PATTERN_GRID_1:
      case CANVAS_LAYOUT_PATTERN_GRID_2_H:
      case CANVAS_LAYOUT_PATTERN_GRID_3_E:
      case CANVAS_LAYOUT_PATTERN_GRID_3_D:
      case CANVAS_LAYOUT_PATTERN_GRID_4_M:
      case CANVAS_LAYOUT_PATTERN_GRID_5_D:
      case CANVAS_LAYOUT_PATTERN_GRID_6_E: {
         layout = "grid";
         break;
      }
      case CANVAS_LAYOUT_PATTERN_FLOAT_6_5D: {
         layout = "float";
         break;
      }
      case CANVAS_LAYOUT_PATTERN_TILED_6_1T5D: {
         layout = "tiled";
         break;
      }
      }
   }
   TRACE6("%s layout:%s\n", __FUNCTION__, layout.toStdString().c_str());
   return layout;
}

void VhallIALive::UpdataCloseCamera() {
   TRACE6("%s enter\n", __FUNCTION__);
   if (!mCurCameraDevID.isEmpty()) {
      HttpSendDeviceStatus(mActiveParam.join_uid, CAMERA_DEVICE, 0);
      ChangeLocalCameraDevState(false, mActiveParam.join_uid);
      TRACE6("%s mCurCameraDevID.clear()\n", __FUNCTION__);
   }
}

void VhallIALive::SetLayOutMode(LayoutMode mode) {
   QObject* recvObj = mpMainUILogic;
   mCurrentMode = mode;
   TRACE6("%s mode %d\n", __FUNCTION__, mode);
   int nRet = vlive::GetWebRtcSDKInstance()->SetConfigBroadCastLayOut(mode, [&, recvObj, mode](const std::string& result, const std::string& msg, int errorCode)->void {
      TRACE6("%s SetConfigBroadCastLayOut result:%s\n", __FUNCTION__, result.c_str());
      QString resultMsg = QString::fromStdString(result);
      if (recvObj) {
         CustomRecvMsgEvent *msgEvent = new CustomRecvMsgEvent(CustomEvent_ConfigBroadLayOut, resultMsg);
         if (msgEvent) {
            msgEvent->mode = mode;
            QCoreApplication::postEvent(recvObj, msgEvent);
         }
      }
   });
   if (nRet == VhallLive_ROOM_DISCONNECT) {
      mReSetLayOutTimer.stop();
      mReSetLayOutTimer.start(2000);
   }
   QString strLayout = GetLayoutStringName(mCurrentMode);
   HttpSendMixLayOutAndProfile(strLayout, mRemoteVideoProfile);
}

LayoutMode  VhallIALive::GetCurrentLayoutMode(LAYOUT_MODE type) {
   switch (type) {
   case LAYOUT_MODE_GRID: {
      mCurrentLayout = LAYOUT_MODE_GRID;
      mCurrentMode = GetGridUserSize();        //均分模式根据上麦人数来获取。
      TRACE6("%s config LAYOUT_MODE_GRID userSize:%d\n", __FUNCTION__, mCurrentMode);
      break;
   }
   case LAYOUT_MODE_FLOAT: {
      mCurrentLayout = LAYOUT_MODE_FLOAT;
      mCurrentMode = CANVAS_LAYOUT_PATTERN_FLOAT_6_5D;
      break;
   }
   case LAYOUT_MODE_TILED: {
      mCurrentLayout = LAYOUT_MODE_TILED;
      mCurrentMode = CANVAS_LAYOUT_PATTERN_TILED_6_1T5D;
      break;
   }
   case LAYOUT_MODE_ONLY: {
      mCurrentLayout = LAYOUT_MODE_ONLY;
      mCurrentMode = CANVAS_LAYOUT_PATTERN_GRID_1;
      break;
   }
   default:
      break;
   }
   return mCurrentMode;
}

void VhallIALive::SetChangeRemoteDefinition() {
   if (mpSettingDlg) {
      LAYOUT_MODE layoutMode = LAYOUT_MODE_TILED;
      if (mpSettingDlg) {
         layoutMode = mpSettingDlg->GetLayOutMode();
      }
      mCurrentMode = GetCurrentLayoutMode(layoutMode);
      QString strLayout = GetLayoutStringName(mCurrentMode);
      HttpSendMixLayOutAndProfile(strLayout, mRemoteVideoProfile);
   }
}

void VhallIALive::HandleGetDeskTopSuc()
{
   mbEnableStopDesktopStream = true;
   if (mActiveParam.bIsLiving){ //如果开始直播
      slot_PublishDesktopStreamTimeout();
   }
   OpenScreenShare();
}

//桌面共享失败
void VhallIALive::HandleGetDeskTopErr(){
   mbEnableStopDesktopStream = true;
}

void VhallIALive::HandleSetDesktopMainView(QString streamId) {
   QObject* recvObj = mpMainUILogic;
   mCurDeskTopStreamID = streamId;
   TRACE6("%s setMixLayoutMainScreen StreamID:%s\n", __FUNCTION__, mCurDeskTopStreamID.toStdString().c_str());
   if (mActiveParam.role_name == USER_HOST) {
      int bRet = GetWebRtcSDKInstance()->SetMixLayoutMainView(streamId.toStdString(), [&, recvObj](const std::string& result, const std::string& msg, int errorCode)->void {
         if (errorCode == 20013) {
            //混流房间找不到该成员
            return;
         }
         //如果设置布局成功
         QString errMsg = QString::fromStdString(result);
         if (recvObj) {
            QCoreApplication::postEvent(recvObj, new CustomRecvMsgEvent(CustomEvent_ConfigDesktopMainView, errMsg));
         }
      });
   }
}

void VhallIALive::HandlePushDeskTopSuc(QString& dataMsg, bool mbVideo){
   InteractAPIManager apiManager(mpMainUILogic);
   apiManager.HttpSendChangeWebWatchLayout(1);
   if (mDesktopWdg) {
      mDesktopWdg->setUpdatesEnabled(false);
   }
   HandleSetDesktopMainView(dataMsg);
}

void VhallIALive::JsCallQtOpenUrl(QString url) {
   TRACE6("%s start\n", __FUNCTION__);
   QDesktopServices::openUrl(url);
}

void VhallIALive::JSCallQtUserOnline(QString param) {
   TRACE6("%s start\n", __FUNCTION__);
   QCoreApplication::postEvent(mpMainUILogic, new CustomRecvMsgEvent(CustomEvent_JSCallUserOnLine, param), Qt::LowEventPriority);
}

void VhallIALive::JSCallQtUserOffline(QString param) {
   TRACE6("%s start\n", __FUNCTION__);
   QCoreApplication::postEvent(mpMainUILogic, new CustomRecvMsgEvent(CustomEvent_JSCallUserOffLine, param), Qt::LowEventPriority);
}

void VhallIALive::SetSpeakUserOffLine(const QString &uid, bool offline) {
   if (IsExistRenderWnd(uid)) {
      TRACE6("%s uid:%s offline:%d\n", __FUNCTION__, uid.toStdString().c_str(), offline);
      if (mActiveParam.role_name == USER_HOST) {
         if (offline) {
            //主持人检测到有用户下麦，启动定时器去检测。
            mCheckSpeakUserTimeOut.stop();
            mCheckSpeakUserTimeOut.start(CHECK_OFFLINE_TIMEOUT);
            mSpeakUserCheckMap.insert(std::map<QString, bool>::value_type(uid, offline));
         }
      }
      else if (mActiveParam.role_name == USER_GUEST) {
         if (!offline && mActiveParam.join_uid == uid) {
            //如果嘉宾检测到自己上线了，并且还在麦上。拉取上麦列表，检测自己是否还在麦上。
            GetSpeakUserList();
         }
      }
   }
}

void VhallIALive::RemoveCheckSpeakUser(const QString& uid) {
   std::map<QString, bool>::iterator iter = mSpeakUserCheckMap.find(uid);
   if (iter != mSpeakUserCheckMap.end()) {
      mSpeakUserCheckMap.erase(iter);
   }
}

bool VhallIALive::IsInSpeakList(std::vector<QString>& speaklist) {
   std::map<QString, VhallRenderWdg*>::iterator iter = mRenderMap.begin();
   while (iter != mRenderMap.end()) {
      bool isInSpeakList = false; //渲染窗口没有在上麦列表中找到
      for (int i = 0; i < speaklist.size(); i++) {
         QString join_id = speaklist.at(i);
         if (join_id == iter->first) {
            isInSpeakList = true;
         }
      }
      if (!isInSpeakList && mActiveParam.role_name == USER_HOST) {
         TRACE6("%s host remove %s\n", __FUNCTION__, iter->first.toStdString().c_str());
         slot_ClickedNotSpeak(iter->first);
      }
      else if (!isInSpeakList && mActiveParam.role_name == USER_GUEST) {
         TRACE6("%s guest not find user %s\n", __FUNCTION__, iter->first.toStdString().c_str());
      }
      iter++;
   }

   if (mbIsGuestSpeaking && mActiveParam.role_name == USER_GUEST) {
      bool isInSpeakList = false;
      for (int i = 0; i < speaklist.size(); i++) {
         QString join_id = speaklist.at(i);
         if (join_id == mActiveParam.join_uid) {
            TRACE6("%s find user in speaklist \n", __FUNCTION__);
            isInSpeakList = true;
            break;
         }
      }
      if (!isInSpeakList) {
         TRACE6("%s can not find user in speaklist,so change to no speak \n", __FUNCTION__);
         vlive::GetWebRtcSDKInstance()->StopLocalCapture();
         RemoveRemoteUser(mActiveParam.join_uid);
         ResetPlayMediaFileAndDesktopShare();
         ResetToNospeakState();
         ui.widget_title->SetToSpeakBtnState(true);
      }
   }
   return true;
}


void VhallIALive::slot_CheckSpeakUserTimeOut() {
   CheckSpeakUserOffLine();
}

void VhallIALive::CheckSpeakUserOffLine() {
   TRACE6("%s \n", __FUNCTION__);
   if (mActiveParam.role_name == USER_HOST) {
      std::map<QString, bool>::iterator iter = mSpeakUserCheckMap.begin();
      while (iter != mSpeakUserCheckMap.end()) {
         QString id = iter->first;
         TRACE6("%s id:%s  \n", __FUNCTION__, id.toStdString().c_str());
         //主持人检测，当网页强制关闭或者助手强制关闭没有发送下麦时的处理。
         if (id != mActiveParam.join_uid && IsExistRenderWnd(id)) {
            slot_ClickedNotSpeak(id);
         }
         iter++;
      }
      mSpeakUserCheckMap.clear();
   }
   TRACE6("%s end \n", __FUNCTION__);
}

void VhallIALive::slot_RoomReConnectTimer() {
   TRACE6("%s reconnect room\n", __FUNCTION__);
   mRoomReConnectTimer.stop();
}

void VhallIALive::JSCallQtPageReady() {
   TRACE6("%s \n", __FUNCTION__);
   QString qtCallJsMethod = GetDocSetMainViewJson(mActiveParam.mainShow, mActiveParam.role_name.compare(USER_USER) == 0 ? "*setMainShow" : "*setMainSpeaker");
   if (!qtCallJsMethod.isEmpty() && mDocCefWidget) {
      mDocCefWidget->AppCallJsFunction(qtCallJsMethod);
   }
}

void VhallIALive::slotFadOutTips(const QString& strMsg){
   FadeOutTip(strMsg, TipsType_Error);
}

void VhallIALive::HandsUpTimeout(QString user_id) {
   if (ui.tabWidget->mpMemberListDlg && !user_id.isEmpty()) {
      VhallAudienceUserInfo user;
      user.userId = user_id.toStdWString();
      ui.tabWidget->mpMemberListDlg->addRaiseHandsMember(user, false);
   }
   else {
      ui.tabWidget->mpMemberListDlg->CancelHandsUpIcon();
   }
}

void VhallIALive::slotApplyTimeout() {
   HandsUpTimeout(mActiveParam.join_uid);
}

void VhallIALive::slotMouseMoved() {
   if (mIsFullSize) {
      QRect rect = QApplication::desktop()->availableGeometry(this);
      ui.widget_title->SetMaxIcon(!mIsFullSize);
      mIsFullSize = false;
      QPoint pos = QCursor::pos();
      int moveX = (rect.width() - mNormalRect.width()) / 2;
      int moveY = (rect.height() - mNormalRect.height()) / 2;
      int iWidth = mNormalRect.width();
      int iHeight = mNormalRect.height();

      mNormalRect.setX(pos.x() - 200);
      mNormalRect.setY(pos.y());
      mNormalRect.setWidth(iWidth);
      mNormalRect.setHeight(iHeight);

      this->setFixedHeight(iHeight);
      this->setFixedWidth(iWidth);

      this->setGeometry(mNormalRect);
      ui.widget_title->SetPressedPoint(QPoint(200, pos.y()));
      TRACE6("%s moveX:%d moveY:%d  rect.x():%d ", __FUNCTION__, moveX, moveY, rect.x());
   }
}

void VhallIALive::slot_RunningState(int cpu, int netSend, int netRecv) {
   double lost = GetWebRtcSDKInstance()->GetPushDesktopVideoLostRate();
   if (m_pScreenShareToolWgd) {
      m_pScreenShareToolWgd->SetPushStreamLost(lost);
   }
   ui.widget_bottomTool->SetCPU(cpu);
   mCPUValue = cpu;
   //CPU达到80%且持续，进行提示
   if (cpu >= CPU_CHECK_VALUE) {
      if (!mbIsNoticeCPUHigh) {
         mLastCPUCheckTime = QDateTime::currentDateTime().toTime_t();
         mbIsNoticeCPUHigh = true;
      }
      uint cur = QDateTime::currentDateTime().toTime_t();
      if (cur - mLastCPUCheckTime >= mCPUCheckTimeOut) {
         //如果没有上麦不提醒
         if (!GetRenderWnd(mActiveParam.join_uid)) {
            FadeOutTip(DEVICE_CPU_HIGH, TipsType_SystemBusy);
            mCPUCheckTimeOut = MAX_CPU_CHECK_TIME;
         }
         else {
            slot_OnCPUCheckTimeout();
         }
         mLastCPUCheckTime = QDateTime::currentDateTime().toTime_t();
      }
   }
   else {
      mbIsNoticeCPUHigh = false;
      mLastCPUCheckTime = QDateTime::currentDateTime().toTime_t();
   }
}

void VhallIALive::slot_closeItem() {
   //1、直播进行中，已提示1次，且用户未做任何操作，或用户点击【关闭】【知道了】，再次满足触发提示条件时，延时5分钟再次进行提示
   //2、直播进行中，已提示1次，且用户选择【切换】，再次满足触发提示条件，延时5分钟再次进行提示
   //3、当用户调整画质档位后，在当前活动上再次发起新的活动，此活动的【设置】中，依旧是默认推荐的标清画质（不记录用户的操作行为）
   if (mActiveParam.bIsLiving) {
      mCPUCheckTimeOut = MAX_CPU_CHECK_TIME;
   }
   mLastCPUCheckTime = QDateTime::currentDateTime().toTime_t();
   mbIsNoticeCPUHigh = false;
}

void VhallIALive::slot_changeProfile(int curProfile) {
   if (curProfile == TipsType_ChangeProfile_Standard) {
      QString confPath = CPathManager::GetConfigPath();
      QString localProfile = ConfigSetting::writeValue(confPath, GROUP_DEFAULT, KEY_LOCAL_VIDEO_PROFILE, PROFILE_INDEX_STANDER);
      mpSettingDlg->ResetProfile(1);
      StartLocalCapture();
      if (GetWebRtcSDKInstance()->IsCapturingStream(VHStreamType_MediaFile)) {
         long long seek = GetWebRtcSDKInstance()->MediaFileGetCurrentDuration();
         StartPlayMediaStream(true);
      }
      //标清画质切换中
      if (mShowTipsWdgPtr) {
         mShowTipsWdgPtr->ChangeProfileStatus(TipsType_ChangeProfile_Standard, LEVEL_TO_STA);
      }
   }
   else if (curProfile == TipsType_ChangeProfile_Fluency) {
      //流畅画质切换中
      QString confPath = CPathManager::GetConfigPath();
      QString localProfile = ConfigSetting::writeValue(confPath, GROUP_DEFAULT, KEY_LOCAL_VIDEO_PROFILE, PROFILE_INDEX_FLUENCY);
      mpSettingDlg->ResetProfile(2);
      StartLocalCapture();
      if (GetWebRtcSDKInstance()->IsCapturingStream(VHStreamType_MediaFile)) {
         StartPlayMediaStream(true);
      }
      if (mShowTipsWdgPtr) {
         mShowTipsWdgPtr->ChangeProfileStatus(TipsType_ChangeProfile_Fluency, LEVEL_TO_FLU);
      }
   }
   mbIsNoticeCPUHigh = false;
}

void VhallIALive::slot_OnCPUCheckTimeout() {
   TRACE6("%s\n", __FUNCTION__);
   QString confPath = CPathManager::GetConfigPath();
   QString localProfile = ConfigSetting::ReadString(confPath, GROUP_DEFAULT, KEY_LOCAL_VIDEO_PROFILE, PROFILE_INDEX_STANDER);
   if (localProfile == PROFILE_INDEX_HIGH) {
      FadeOutTip(CPU_HIGH_PUSH_LEVEL_STD, TipsType_ChangeProfile_Standard);
   }
   else if (localProfile == PROFILE_INDEX_STANDER) {
      FadeOutTip(CPU_HIGH_PUSH_LEVEL_FLU, TipsType_ChangeProfile_Fluency);
   }
   else {
      FadeOutTip(CPU_HIGH_CLOSE_OTHER, TipsType_SystemBusy);
   }
   mCPUCheckTimeOut = MAX_CPU_CHECK_TIME;
}

void VhallIALive::logCPUAndMemory() {
   double cpuf = (double)mCPUValue;
   int nMemTotal = 0;
   int nMemUsed = 0;
   GetSysMemory(nMemTotal, nMemUsed);
   HANDLE handle = GetCurrentProcess();
   PROCESS_MEMORY_COUNTERS pmc;
   GetProcessMemoryInfo(handle, &pmc, sizeof(pmc));
   int currentMemory = pmc.PagefileUsage / 1024 / 1024;
   STRU_MAINUI_LOG log;
   swprintf_s(log.m_wzRequestUrl, DEF_MAX_HTTP_URL_LEN, L"key=basic_info&cpu=%f&mem_phy=%d&mem_cur=%d&mem_vls=%d&k=%d",
      cpuf,//Cpu使用率
      nMemTotal,//内存总量
      nMemUsed,//内存使用量
      currentMemory,//当前分页项目
      eLogRePortK_BasicInfo);
   QJsonObject body;
   body["cu"] = cpuf;   	//Cpu使用率
   body["mus"] = nMemTotal;	//内存总量
   body["muu"] = nMemUsed;	//内存使用量
   body["crm"] = nMemUsed;	//当前分页项目
   QString json = CPathManager::GetStringFromJsonObject(body);
   strcpy_s(log.m_wzRequestJson, json.mid(0, DEF_MAX_HTTP_URL_LEN).toLatin1().data());
   SingletonMainUIIns::Instance().PostCRMessage(MSG_MAINUI_LOG, &log, sizeof(STRU_MAINUI_LOG));
}

void VhallIALive::slot_OnNetWordDataStat(){
   if (mPerformanceStatisticsPtr) {
      slot_RunningState(mPerformanceStatisticsPtr->GetCPUValue(),0,0);
   }
   mNetDevDataCount++;
   if (mNetDevDataCount >= 15) {
      logCPUAndMemory();
      mNetDevDataCount = 0;
   }
   if (m_pScreenShareToolWgd && !m_pScreenShareToolWgd->isHidden()) {
      double lost = GetWebRtcSDKInstance()->GetPushDesktopVideoLostRate();
      if (lost >= 0.0) {
         m_pScreenShareToolWgd->SetPushStreamLost(lost);
      }
      int level = GetWebRtcSDKInstance()->GetAudioLevel(mActiveParam.join_uid.toStdWString());
      level = level / 100;
      m_pScreenShareToolWgd->SetMicVol(level);
   }
}

void VhallIALive::JsCallQtMsg(QString param) {
   VH::CComPtr<ICommonData> pCommonData;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
   UserOnLineParam lineParam;
   pCommonData->ParamToLineParam(param, lineParam);

   VhallAudienceUserInfo Info;
   Info.role = lineParam.userRole.toStdWString();
   Info.userId = lineParam.userId.toStdWString();
   Info.userName = lineParam.userName.toStdWString();
   Info.gagType = lineParam.is_banned == true ? VhallShowType_Allow : VhallShowType_Hide;
   Info.headImage = lineParam.headImage.replace("https", "http").toStdWString();
   Info.mDevType = lineParam.mDevType;
   Info.miUserCount = lineParam.mUV;

   if (lineParam.type == "disable" || lineParam.type == "permit") {
      Event oEvent;
      oEvent.m_eventType = e_eventType_msg;
      oEvent.m_eMsgType = lineParam.type == "disable" ? e_RQ_UserProhibitSpeakOneUser : e_RQ_UserAllowSpeakOneUser;
      oEvent.m_oUserInfo = Info;
      QJsonObject eventJson = VHAliveInteraction::MakeEventObject(&oEvent);
      if (mpMainUILogic) {
         QCoreApplication::postEvent(mpMainUILogic, new CustomSocketIOMsg(CustomEvent_CustomRecvSocketIOMsgEvent, eventJson), Qt::LowEventPriority);
      }
   } 
   else if (lineParam.type == "docFullscreen") {
      if (lineParam.is_full) {
         ui.verticalLayout_mainWebView->removeWidget(mDocCefWidget);
         QRect screen_geometry_rect = QApplication::desktop()->screenGeometry(this);
         mFullDocWdg->setGeometry(screen_geometry_rect);  
         mFullDocWdg->InsertDoc(mDocCefWidget);
         mFullDocWdg->show();
      }
      else {
         mFullDocWdg->RemoveDoc(mDocCefWidget);
         mFullDocWdg->repaint();
         mFullDocWdg->hide();
         ui.verticalLayout_mainWebView->addWidget(mDocCefWidget);
         show();
         raise();
         setFocus();
         activateWindow();
      }
   }
   else if (lineParam.type == "esc") {
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
}

void VhallIALive::keyPressEvent(QKeyEvent *event) {
   switch (event->key())
   {
   case Qt::Key_Escape:
      break;
   default:
      QWidget::keyPressEvent(event);
   }
}

void VhallIALive::ConfigRoomMainView(const QString& uid) {//主画面ID
   QString streamID;
   if (uid.compare(mActiveParam.join_uid) == 0) {
      streamID = mLocalStreamID;
   }
   else {
      streamID = QString::fromStdString(GetWebRtcSDKInstance()->GetUserStreamID(uid.toStdWString(), vlive::VHStreamType_AVCapture));
   }
   TRACE6("%s  streamID:%s\n", __FUNCTION__, streamID.toStdString().c_str());
   if (!streamID.isEmpty()) {
      if (mActiveParam.role_name == USER_HOST && mMediaFilePlayWdg == NULL && mDesktopWdg == NULL && m_pScreenShareToolWgd && m_pScreenShareToolWgd->isHidden()) {
         QObject *recvObj = mpMainUILogic;
         TRACE6("%s setMixLayoutMainScreen streamID:%s\n", __FUNCTION__, streamID.toStdString().c_str());
         int bRet = GetWebRtcSDKInstance()->SetMixLayoutMainView(streamID.toStdString(), [&, recvObj](const std::string& result, const std::string& msg, int errorCode)->void {
            if (errorCode == 20013) {
               //混流房间找不到该成员
               return;
            }
            //如果设置布局成功
            QString errMsg = QString::fromStdString(result);
            if (recvObj) {
               QCoreApplication::postEvent(recvObj, new CustomRecvMsgEvent(CustomEvent_ConfigMainView, errMsg));
            }
         });
      }
   }
   TRACE6("%s uid:%s\n", __FUNCTION__, uid.toStdString().c_str());
}

void VhallIALive::slot_MicVolumnChanged(int volume) {
   if (!mCurMicDevID.isEmpty()) {
      GetWebRtcSDKInstance()->SetCurrentMicVol(volume);
      mMicVolume = volume;
      if (mMicVolume == 0) {
         ChangeLocalMicDevState(false, mActiveParam.join_uid);
      }
      else {
         ChangeLocalMicDevState(true, mActiveParam.join_uid);
      }
   }
}

void VhallIALive::slot_PlayerVolumnChanged(int volume) {
   if (!mCurPlayerID.isEmpty()) {
      GetWebRtcSDKInstance()->SetCurrentPlayVol(volume);
      mPlayerVolume = volume;
      if (mPlayerVolume == 0) {
         GetWebRtcSDKInstance()->SetCurrentPlayVol(mPlayerVolume);
         mbIsPlayerOpen = false;
      }
      else {
         mbIsPlayerOpen = true;
      }
      mpSettingDlg->OnOpenPlayer(mbIsPlayerOpen);
      if (NULL != m_pScreenShareToolWgd){
         m_pScreenShareToolWgd->SetPlayerState(mbIsPlayerOpen);
      }
   }
}

void VhallIALive::PushMsgEvent(int type, QString data, bool bVideo) {
   STRU_VHALL_ACTIVE_EVENT activeEvent;
   activeEvent.mbHasVideo = bVideo;
   activeEvent.mEventType = type;
   strcpy_s(activeEvent.mEventData, data.toStdString().c_str());
   SingletonMainUIIns::Instance().PostCRMessage(MSG_MAINUI_VHALL_ACTIVE_EVENT, &activeEvent, sizeof(STRU_VHALL_ACTIVE_EVENT));
}

void VhallIALive::HandleCustomEvent(QEvent* event) {
   if (event) {
      int type = event->type();
      TRACE6("%s enter type:%d\n", __FUNCTION__, type);
      if (mbExitRoom && type == CustomEvent_CloseExit) {
         HandleCloseAndExit();
      }
      else if (!mbExitRoom) {
         switch (type)
         {
         case CustomEvent_GetLocalStreamSuc: {
            HandleGetLocalStream(event);
            break;
         }
         case CustomEvent_PushLocalStreamSuc: {
            HandlePushLocalStreamSuc(event);
            break;
         }
         case RoomEvent_SetMainView: {
            HandlePaasSetMainView(event);
            break;
         }
         case CustomEvent_VSSGetKickedList:
         case CustomEvent_VSSGetBannedList:
         case CustomEvent_VSSGetOnlineList:
         case CustomEvent_VSSGetSpecialList: {
            break;
         }
         case CustomEvent_GetLocalStreamErr: {
            mbEnableStopStream = true;
            TRACE6("%s CustomEvent_GetLocalStreamErr mbEnableStopStream %d\n", __FUNCTION__, mbEnableStopStream);
            FadeOutTip(OPEN_CAMERA_ERR, TipsType_Error);
            UpdataCloseCamera();
            mbHasVideoFrame = false;
            break;
         }
         case CustomEvent_OpenCameraErrMsgEvent: {
            mbEnableStopStream = true;
            FadeOutTip(OPEN_CAMERA_ERR, TipsType_Error);
            UpdataCloseCamera();
            break;
         }
         case CustomEvent_OpenMicErrMsgEvent: {
            mbEnableStopStream = true;
            FadeOutTip(OPEN_MIC_ERR, TipsType_Error);
            UpdataCloseCamera();
            break;
         }
         case CustomEvent_AudioCaptureErr: {
            FadeOutTip(CAPTURE_MIC_ERR, TipsType_Error);
            break;
         }
         case CustomEvent_RecvMsg: {
            CustomRecvMsgEvent* msgEvent = (CustomRecvMsgEvent*)event;
            if (msgEvent) {
               QString msg = msgEvent->msg;
               if (mDocCefWidget) {
                  mDocCefWidget->AppCallJsFunction(msg);
               }
               ui.tabWidget->RunJavaScrip(msg);
            }
            break;
         }
         case CustomEvent_StartLiveGetDev: {
            StartLiveWithOpenDevice();
            break;
         }
         case CustomEvent_InitVhallSDK: {    //如果主持人崩溃了再次进入互动，置状态为已经开始直播。当链接房间成时，根据mbIsHostStartLiving自动推流。
            HandleInitVhallSDK();
            break;
         }
         case CustomEvent_SelectGetDev: {
            if (mpSettingDlg) {
               mpSettingDlg->SetLastLayoutMode();
               mpSettingDlg->SetCurrentDeviceInfo(mMicList, mCurMicDevID, mCameraList, mCurCameraDevID, mPlayerList, mCurPlayerID);               
               if (m_pScreenShareToolWgd && !m_pScreenShareToolWgd->isHidden()) {
                  ConfigSetting::GetInScreenPos(m_pScreenShareToolWgd, mpSettingDlg);
               }
               else {
                  ConfigSetting::GetInScreenPos(this, mpSettingDlg);
               }
               mpSettingDlg->show();
               mpSettingDlg->raise();
            }
            break;
         }
         case CustomEvent_ConnectRoomSuc: {//连接媒体房间成功
            HandleConnectRoomSuc();
            break;
         }
         case CustomEvent_ConnectRoomErr: { //连接媒体房间失败
            HandleConnectRoomErr();
            break;
         }
         case CustomEvent_STREAM_REMOVED: {
            HandleStreamRemoved(event);
            break;
         }
         case CustomEvent_RenderView:
         case CustomEvent_SubScribSuc: {
            HandleSubScribSuc(event);
            break;
         }
         case CustomEvent_STREAM_FAILED: {
            HandleStreamFailed(event);
            break;
         }
         case CustomEvent_GetLocalFileStreamSuc: {
            HandlePlayMediaFileSuc();
            break;
         }
         case CustomEvent_PushMediaFileStreamSuc: {
            HandlePushMediaFileStreamSuc(event);
            break;
         }
         case CustomEvent_ConfigBoradCast: {
            HandleConfigBoradCast(event);
            break;
         }
         case CustomEvent_ConfigBoradCastOnlyOne: {
            HandleConfigBoradCastOnlyOne(event);
            break;
         }
         case CustomEvent_GetDeskTopStreamSuc: {
            HandleGetDeskTopSuc();
            break;
         }
         case CustomEvent_GetDeskTopStreamErr: {
            HandleGetDeskTopErr();
            break;
         }
         case CustomEvent_PushDeskTopStreamSuc: {
            CustomRecvMsgEvent* msgEvent = (CustomRecvMsgEvent*)event;
            if (msgEvent) {
               HandlePushDeskTopSuc(msgEvent->msg, msgEvent->mbHasVideo);
            }
            break;
         }
         case CustomEvent_ConfigBroadLayOut: {
            HandleLayOutMode(event);
            break;
         }
         case CustomEvent_ConfigMediaFileMainView: {
            HandleConfigMediaFileMainView(event);
            break;
         }
         case CustomEvent_ConfigMainView: {
            HandleConfigMainView(event);
            break;
         }
         case CustomEvent_UnPushMediaFileStream: {
            HandleUnPushMediaFileStream(event);
            break;
         }
         case CustomEvent_SubStream: {
            HandleSubStream(event);
            break;
         }
         case CustomEvent_ConfigDesktopMainView: {
            HandleConfigDesktopMainView(event);
            break;
         }
         case CustomEvent_HostExitRoom: {
            HandleHostExitRoom();
            break;
         }
         case CustomEvent_UnPushDeskTopStream: {
            break;
         }
         case CustomEvent_RePublishLocalStream: {
            HandleRePublishLocalStream();
            break;
         }
         case CustomEvent_RePublishDeskTopStream: {
            HandleRePublishDesktopStream();
            break;
         }
         case CustomEvent_RePublishMediaStream: {
            HandleRePublishMediaStream();
            break;
         }
         case CustomEvent_ReSubScribCameraStream: {
            HandleReSubScribeLocalStream();
            break;
         }
         case CustomEvent_ReSubScribDesktopStream: {
            HandleReSubScribeDesktopStream();
            break;
         }
         case CustomEvent_ReSubScribMediaStream: {
            HandleReSubScribeMediaStream();
            break;
         }
         case CustomEvent_GetLocalFileStreamErr: {
            FadeOutTip(OPEN_FILE_ERR, TipsType_Error);
            TRACE6("%s CustomEvent_GetLocalFileStreamErr\n", __FUNCTION__);
            if (mbIsPlayMediaFile) {
               TRACE6("%s MSG_MAINUI_VEDIOPLAY_STOPPLAY\n", __FUNCTION__);
               SingletonMainUIIns::Instance().PostCRMessage(MSG_MAINUI_VEDIOPLAY_STOPPLAY, NULL, 0);
               StopPlayMediaFile();
            }
            break;
         }
         case CustomEvent_PushStreamReConnect: {
            HandlePushStreamReConnect(event);
            break;
         }
         case CustomEvent_RoomReconnecting: {
            FadeOutTip(RTC_CONNECT_ERR, TipsType_Error, 5000);
            break;
         }
         case CustomEvent_LostStream: {
            FadeOutTip(SOURCE_LOST, TipsType_Error);
            mReSetLocalCapture.start(2000);
            mReSetLocalCapture.setSingleShot(true);
            break;
         }
         case CustomEvent_VIDEO_CAPTURE_ERROR: {
            FadeOutTip(VIDEO_CAPTURE_ERROR, TipsType_Error);
            break;
         }
         case CustomEvent_AudioDevRemoved: {
            FadeOutTip(AUDIO_DEV_RREMOVED, TipsType_Error);
            break;
         }
         case CustomEvent_DevChanged: {
            HandleDeviceEvent(event);
            break;
         }
         case CustomEvent_OnRecvHttpResponse: {
            HandleOnRecvHttpResponse(event);
            break;
         }
         case CustomEvent_RoomReconnected: {
            HandleRoomReconnected();
            break;
         }
         default:
            customEvent(event);
            break;
         }
      }
      TRACE6("%s leave type:%d\n", __FUNCTION__, type);
   }
}

void VhallIALive::HandleRoomReconnected() {
   FadeOutTip(RTC_CONNECT_OK, TipsType_Success);
   TRACE6("%s end\n", __FUNCTION__);
}

void VhallIALive::HandleConfigMainView(QEvent* event) {
   TRACE6("%s \n", __FUNCTION__);
   if (event) {
      CustomRecvMsgEvent* eventMsg = (CustomRecvMsgEvent*)event;
      if (eventMsg) {
         TRACE6("%s eventMsg.msg :%s\n", __FUNCTION__, eventMsg->msg.toStdString().c_str());
      }
      if (eventMsg->msg.compare("success") != 0 && mMediaFilePlayWdg == NULL && mDesktopWdg == NULL 
            && !GetWebRtcSDKInstance()->IsCapturingStream(VHStreamType_Desktop) 
            && !GetWebRtcSDKInstance()->IsCapturingStream(VHStreamType_MediaFile)
            && GetWebRtcSDKInstance()->IsCapturingStream(VHStreamType_AVCapture)) {
         mSetMainViewTimer.start(2000);
         mSetMainViewTimer.setSingleShot(true);
      }
      else {
         mSetMainViewTimer.stop();
      }
   }
}

void VhallIALive::slot_SetMainViewTimeout() {
   TRACE6("%s setMixLayoutMainScreen mainShow:%s\n", __FUNCTION__, mActiveParam.mainShow.toStdString().c_str());
   ConfigRoomMainView(mActiveParam.mainShow);
}

void VhallIALive::slot_SetDesktopMainViewTimeout() {
   TRACE6("%s setMixLayoutMainScreen mCurDeskTopStreamID:%s\n", __FUNCTION__, mCurDeskTopStreamID.toStdString().c_str());
   if (mActiveParam.role_name == USER_HOST) {
      QObject* recvObj = mpMainUILogic;
      GetWebRtcSDKInstance()->SetMixLayoutMainView(mCurDeskTopStreamID.toStdString(), [&, recvObj](const std::string& result, const std::string &msg, int errorCode) -> void {
         if (errorCode == 20013) {
            //混流房间找不到该成员
            return;
         }
         QString errMsg = QString::fromStdString(result);
         if (recvObj) {
            QCoreApplication::postEvent(recvObj, new CustomRecvMsgEvent(CustomEvent_ConfigDesktopMainView, errMsg));
         }
      });
   }
}

void VhallIALive::slot_SetMediaFileMainViewTimeout() {
   TRACE6("%s setMixLayoutMainScreen mCurPlayMediaFileStreamID:%s\n", __FUNCTION__, mCurPlayMediaFileStreamID.toStdString().c_str());
   if (mActiveParam.role_name == USER_HOST) {
      QObject* recvObj = mpMainUILogic;
      GetWebRtcSDKInstance()->SetMixLayoutMainView(mCurPlayMediaFileStreamID.toStdString(), [&, recvObj](const std::string &result, const std::string &msg, int errorCode) -> void {
         if (errorCode == 20013) {
            //混流房间找不到该成员
            return;
         }
         QString errMsg = QString::fromStdString(result);
         if (recvObj) {
            QCoreApplication::postEvent(recvObj, new CustomRecvMsgEvent(CustomEvent_ConfigMediaFileMainView, errMsg));
         }
      });
   }
}
void VhallIALive::slot_PublishLocalStreamTimeout() {
   mRePublishLocalStreamTimer.stop();
   TRACE6("%s %d\n", __FUNCTION__, mActiveParam.bIsLiving);
   if (mActiveParam.bIsLiving) {
      int nRet = GetWebRtcSDKInstance()->StartPushLocalStream();
      TRACE6("%s %d\n", __FUNCTION__, nRet);
      if (nRet == VhallLive_ROOM_DISCONNECT && mActiveParam.bIsLiving) {
         //网络连接异常，启动定时器尝试重新推流。
         mRePublishLocalStreamTimer.start(3000);
         mRePublishLocalStreamTimer.setSingleShot(true);
      }
   }
}

void VhallIALive::slot_PublishDesktopStreamTimeout() {
   TRACE6("%s %d\n", __FUNCTION__, mActiveParam.bIsLiving);
   mRePublishDesktopStreamTimer.stop();
   if (mActiveParam.bIsLiving) {
      int nRet = GetWebRtcSDKInstance()->StartPushDesktopStream();
      TRACE6("%s %d\n", __FUNCTION__, nRet);
      if (nRet == VhallLive_ROOM_DISCONNECT && mActiveParam.bIsLiving) {
         TRACE6("%s mRePublishDesktopStreamTimer start\n", __FUNCTION__);
         //网络连接异常，启动定时器尝试重新推流。
         mRePublishDesktopStreamTimer.start(3000);
         mRePublishDesktopStreamTimer.setSingleShot(true);
      }
   }
}

void VhallIALive::slot_PublishMediaFileStreamTimeout() {
   TRACE6("%s %d\n", __FUNCTION__, mActiveParam.bIsLiving);
   mRePublishMediaFileStreamTimer.stop();
   if (mActiveParam.bIsLiving) {
      int nRet = GetWebRtcSDKInstance()->StartPushMediaFileStream();
      TRACE6("%s %d\n", __FUNCTION__, nRet);
      if (nRet == VhallLive_ROOM_DISCONNECT && mActiveParam.bIsLiving) {
         //网络连接异常，启动定时器尝试重新推流。
         mRePublishMediaFileStreamTimer.start(3000);
         mRePublishMediaFileStreamTimer.setSingleShot(true);
      }
   }
}

void VhallIALive::slot_ReSetLocalCaptureTimeout() {
   HandleLostLocalCameraStream(true);
}

void VhallIALive::slot_CheckSubScribeTimeout() {
   GetSpeakUserList();
}

void VhallIALive::slot_ReSetLayoutTimeout() {
   if (mActiveParam.role_name == USER_HOST) {
      SetLayOutMode(mCurrentMode);
   }
}

void VhallIALive::HandleVSSSetMainScreenResp(int libCurlCode, std::string msg, std::string receive_join_id, std::string join_id, QString roleName) {
   if (libCurlCode != 0) {
      FadeOutTip(HTTP_SEND_TIMEOUT, TipsType_Error);
   }
   else {
      int nCode = 0;
      std::string respMsg;
      InteractAPIManager::ParamHttpResp(msg, nCode, respMsg);
      if (nCode != 200) {
         FadeOutTip(QString::fromStdString(respMsg), TipsType_Error);
      }
      else {
         if (mActiveParam.mbIsH5Live) {
            //QString streamID;
            //if (QString::fromStdString(receive_join_id).compare(mActiveParam.join_uid) == 0) {
            //   streamID = mLocalStreamID;
            //}
            //else {
            //   streamID = QString::fromStdString(GetWebRtcSDKInstance()->GetUserStreamID(QString::fromStdString(receive_join_id).toStdWString(), vlive::VHStreamType_AVCapture));
            //}
            //if (!streamID.isEmpty()) {
            //   vlive::GetPaasSDKInstance()->SetMainView(streamID.toStdString());
            //}
            ChangeMainView(QString::fromStdString(receive_join_id));
            ConfigRoomMainView(QString::fromStdString(receive_join_id));

            if (roleName == USER_GUEST || roleName == USER_HOST) {
               InteractAPIManager apiManager(mpMainUILogic);
               apiManager.HttpSendSetDocPermission(QString::fromStdString(receive_join_id));
            }
         }
         ResetPlayMediaFileAndDesktopShare();
      }
   }
}

void VhallIALive::HandleVSSAgreeApply(int libCurlCode, std::string msg, std::string recv_user_id, std::string join_id) {
   if (libCurlCode != 0) {
      FadeOutTip(HTTP_SEND_TIMEOUT, TipsType_Error);
   }
   else {
      int nCode = 0;
      std::string respMsg;
      InteractAPIManager::ParamHttpResp(msg, nCode, respMsg);
      if (nCode == 30007) {
         return;
      }
      if (nCode != 200) {
         FadeOutTip(QString::fromStdString(respMsg), TipsType_Error);
      }
      else {
         if (mActiveParam.mbIsH5Live) {
            vlive::GetPaasSDKInstance()->AuditInavPublish(recv_user_id, AuditPublish_Accept);
         }
      }
   }
}

void VhallIALive::HandleVSSRejectApply(int libCurlCode, std::string msg, std::string recv_user_id, std::string join_id) {
   if (libCurlCode != 0) {
      FadeOutTip(HTTP_SEND_TIMEOUT, TipsType_Error);
   }
   else {
      int nCode = 0;
      std::string respMsg;
      InteractAPIManager::ParamHttpResp(msg, nCode, respMsg);
      if (nCode == 30007) {
         return;
      }
      if (nCode != 200) {
         FadeOutTip(QString::fromStdString(respMsg), TipsType_Error);
      }
      else {
         if (ui.tabWidget->mpMemberListDlg) {
            ui.tabWidget->mpMemberListDlg->UpdataOnLineUserHandsUpState(QString::fromStdString(recv_user_id), false);
         }
         if (mActiveParam.mbIsH5Live) {
            vlive::GetPaasSDKInstance()->AuditInavPublish(recv_user_id, AuditPublish_Refused);
         }
      }
   }
}

void VhallIALive::HandleRejectInviteResp(int libCurlCode, std::string msg, std::string recv_user_id, std::string join_id) {
   if (libCurlCode != 0) {
      FadeOutTip(HTTP_SEND_TIMEOUT, TipsType_Error);
   }
   else {
      int nCode = 0;
      std::string respMsg;
      InteractAPIManager::ParamHttpResp(msg, nCode, respMsg);
      if (nCode != 200) {
         FadeOutTip(QString::fromStdString(respMsg), TipsType_Error);
      }
      else {

      }
   }
}

void VhallIALive::HandleVSSSendInvite(int libCurlCode, std::string msg, std::string recv_user_id, std::string join_id) {
   if (libCurlCode != 0) {
      FadeOutTip(HTTP_SEND_TIMEOUT, TipsType_Error);
   }
   else {
      int nCode = 0;
      std::string respMsg;
      InteractAPIManager::ParamHttpResp(msg, nCode, respMsg);
      if (nCode != 200) {
         FadeOutTip(QString::fromStdString(respMsg), TipsType_Error);
      }
      else {
         if (mActiveParam.mbIsH5Live) {
            //调用paas-sdk
            vlive::GetPaasSDKInstance()->AskforInavPublish(recv_user_id);
         }
      }
   }
}

void VhallIALive::HandleVSSSetStream(int libCurlCode, std::string msg, std::string layout, std::string profile) {
   if (libCurlCode != 0) {
      if (mActiveParam.mbIsH5Live) {
         QCoreApplication::postEvent(mpMainUILogic, new CustomRecvMsgEvent(CustomEvent_ConfigBoradCast, "failed"));
      }
   }
   else {
      int nCode = 0;
      std::string respMsg;
      InteractAPIManager::ParamHttpResp(msg, nCode, respMsg);
      if (nCode != 200) {
         FadeOutTip(QString::fromStdString(respMsg), TipsType_Error);
         if (mActiveParam.mbIsH5Live) {
            QCoreApplication::postEvent(mpMainUILogic, new CustomRecvMsgEvent(CustomEvent_ConfigBoradCast, "failed"));
         }
      }
      else {
         if (mActiveParam.mbIsH5Live) {
            QCoreApplication::postEvent(mpMainUILogic, new CustomRecvMsgEvent(CustomEvent_ConfigBoradCast, "success"));
         }
      }
   }
}

void VhallIALive::HandlePaasSDKFailedEvent(int libCurlCode, std::string msg, int eventCode) {
   switch (eventCode) {
      case RoomEvent_Login: {
         FadeOutTip(QString::fromStdString(msg), TipsType_Error);
         break;
      }
      case RoomEvent_Start_PublishInavAnother: {
         InitBroadCast();
         break;
      }
   }
}

void VhallIALive::HandlePaasSDKConnectSuc(std::string msg, int eventCode) {
   switch (eventCode) {
      case RoomEvent_Login: {
         emit signal_MemberListRefresh();
         break;
      }     
      case RoomEvent_Start_PublishInavAnother: {
         TRACE6("RoomEvent_Start_PublishInavAnother success");
         break;
      }
   }
}

void VhallIALive::HandleDevCheck() {
   mbIsDeviceChecking = true;
   AlertTipsDlg tip(CHECK_DEVICE_RE_NOTICE, true, this);
   tip.CenterWindow(this);
   tip.SetYesBtnText(TO_CHECK_DEV);
   tip.SetNoBtnText(NO_CHECK_DEV);
   if (tip.exec() == QDialog::Accepted) {
      mDeviceTestingWdg = new DeviceTestingWdg(eLiveType_VhallActive,this);
      mDeviceTestingWdg->SetRole(mActiveParam.role_name == USER_HOST ? true : false);
      mDeviceTestingWdg->StartPreView();
      mDeviceTestingWdg->raise();
      int nRet = mDeviceTestingWdg->exec();
      if (mActiveParam.role_name != USER_HOST) {
         if (mpInviteJoinWdg && mpInviteJoinWdg->GetLeftTime() != -1) {
            ConfigSetting::MoveWindow(mpInviteJoinWdg,this->isVisible() ? this : nullptr);
            mpInviteJoinWdg->exec();
         }
      }
   }
   else {
      AlertTipsDlg tip_notice(CHECK_DEVICE_NOTICE, true, this);
      tip_notice.CenterWindow(this);
      tip_notice.SetYesBtnText(TO_CHECK_DEV);
      tip_notice.SetNoBtnText(QString::fromWCharArray(L"不检测"));
      if (tip_notice.exec() == QDialog::Accepted) {
         mDeviceTestingWdg = new DeviceTestingWdg(eLiveType_VhallActive,this);
         mDeviceTestingWdg->SetRole(mActiveParam.role_name == USER_HOST ? true : false);
         mDeviceTestingWdg->raise();
         mDeviceTestingWdg->StartPreView();
         mDeviceTestingWdg->exec();
      }
      if (mActiveParam.role_name != USER_HOST) {
         if (mpInviteJoinWdg && mpInviteJoinWdg->GetLeftTime() != -1) {
            ConfigSetting::MoveWindow(mpInviteJoinWdg, this->isVisible() ? this : nullptr);
            mpInviteJoinWdg->exec();
         }
      }
   }
   mbIsDeviceChecking = false;
}

void VhallIALive::HandleVSSGetRoomBaseInfo(int libCurlCode, std::string msg) {
   if (libCurlCode != 0) {
      FadeOutTip(RTC_CONNECT_INIT_ERR, TipsType_Error);
   }
   else {
      int nCode = 0;
      std::string respMsg;
      InteractAPIManager::ParamHttpResp(msg, nCode, respMsg);
      if (nCode != 200) {
         QString errMsg = QString::fromStdString(respMsg);
         if (errMsg == "") {
            FadeOutTip(RTC_CONNECT_INIT_ERR, TipsType_Error);
         }
         FadeOutTip(errMsg, TipsType_Error);
      }
      else {
         int nCode = 0;
         std::string live_room_id;
         QString respMsg;
         QJsonParseError json_error;
         QJsonArray speak_array;
         int arraySize = 0;
         QString name;
         QJsonDocument parse_doucment = QJsonDocument::fromJson(QByteArray(msg.c_str()), &json_error);
         if (json_error.error == QJsonParseError::NoError) {
            if (parse_doucment.isObject()) {
               QJsonObject obj = parse_doucment.object();
               if (obj.contains("data") && obj["data"].isObject()) {
                  QJsonObject data = obj["data"].toObject();
                  if (data.contains("paas_access_token") && data["paas_access_token"].isString()) {
                     mActiveParam.pass_access_token = data["paas_access_token"].toString();
                  }
                  if (data.contains("inav_id") && data["inav_id"].isString()) {
                     mActiveParam.paas_inav_room_id = data["inav_id"].toString();
                  }
                  if (data.contains("room_id") && data["room_id"].isString()) {
                     live_room_id = data["room_id"].toString().toStdString();
                  }
                  if (data.contains("account_id")) {
                     if (data["account_id"].isString()) {
                        mActiveParam.paas_account_id = data["account_id"].toString();
                     }
                     else {
                        mActiveParam.paas_account_id = QString::number(data["account_id"].toInt());
                     }
                  }
                  if (data.contains("app_id") && data["app_id"].isString()) {
                     mActiveParam.pass_app_id = data["app_id"].toString();
                  }

                  std::string channel_id;
                  if (data.contains("channel_id") && data["channel_id"].isString()) {
                     channel_id = data["channel_id"].toString().toStdString();
                  }
                  std::string context = MainUILogic::GetPaasSDKJoinContext();
                  vlive::GetPaasSDKInstance()->LoginRoom(mActiveParam.pass_access_token.toStdString(), mActiveParam.pass_app_id.toStdString(), mActiveParam.paas_account_id.toStdString(), mActiveParam.paas_inav_room_id.toStdString(), live_room_id, channel_id, context);
               }
            }
         }
      }
   }
}

void VhallIALive::HandleVSSStartLive(int libCurlCode, std::string msg) {
   if (mpStartWaiting) {
      mpStartWaiting->Close();
   }
   if (libCurlCode != 0) {
      FadeOutTip(HTTP_SEND_TIMEOUT, TipsType_Error);
   }
   else {
      int nCode = 0;
      std::string respMsg;
      InteractAPIManager::ParamHttpResp(msg, nCode, respMsg);
      if (nCode != 200) {
         QString errMsg = QString::fromStdString(respMsg);
         if (errMsg == "") {
            errMsg = START_LIVE_ERR;
         }
         FadeOutTip(errMsg, TipsType_Error);
         if (nCode == 10001 || nCode == 10002 || nCode == 10003 || nCode == 10004) {
            QString domain = ConfigSetting::ReadString(CPathManager::GetToolConfigPath(), GROUP_DEFAULT, KEY_URL_DOMAIN, domainUrl);
            QString url = domain + "/account/center";
            AlertTipsDlg tip(errMsg, true, this);
            tip.CenterWindow(this);
            tip.SetYesBtnText(TO_BUY);
            tip.SetNoBtnText(CANCEL);
            if (tip.exec() == QDialog::Accepted) {
               QDesktopServices::openUrl(url);
            }
         }
         return;
      }
      else {
         //开始直播成功时记录状态位1。结束直播时恢复状态为0 ，表示直播过程中无异常
         ConfigSetting::writeValue(CPathManager::GetConfigPath(), GROUP_DEFAULT, KEY_CRASH_STATE, 1);
         TRACE6("VhallInteractive::Slot_BtnStreamClicked() StartPushStream\n");
         HandleStartLiving();
      }
   }
   ui.widget_title->StartTimeClock(TRUE);
   if (m_pScreenShareToolWgd) {
      m_pScreenShareToolWgd->SetStartLiveBtnEnable(true);
   }
}

void VhallIALive::HandleStopLive(int libCurlCode, std::string msg) {
   if (mpExitWaiting) {
      mpExitWaiting->Close();
   }
   if (libCurlCode != 0) {
      FadeOutTip(HTTP_SEND_TIMEOUT, TipsType_Error);
   }
   else {
      int nCode = 0;
      std::string respMsg;
      InteractAPIManager::ParamHttpResp(msg, nCode, respMsg);
      HandleHttpResponseStopwebinarr("", QString::fromStdString(respMsg), nCode, "", true);
      mIsCallStartMixStream = false;
      if (ui.tabWidget->mpMemberListDlg) {
         ui.tabWidget->mpMemberListDlg->setHansUpStates(false);
      }
   }
   ui.widget_title->StartTimeClock(FALSE);
   if (m_pScreenShareToolWgd) {
      m_pScreenShareToolWgd->SetStartLiveBtnEnable(true);
   }
}

void VhallIALive::HandleVSSAgreeInvite(int libCurlCode, std::string msg, std::string uid, bool agree) {
   if (libCurlCode != 0) {
      FadeOutTip(HTTP_SEND_TIMEOUT, TipsType_Error);
   }
   else {
      int nCode = 0;
      std::string respMsg;
      InteractAPIManager::ParamHttpResp(msg, nCode, respMsg);
      if (nCode != 200) {
         FadeOutTip(QString::fromStdString(respMsg), TipsType_Error);
      }
      else {
         if (agree) {
            LocalUserToSpeak();
            TRACE6("%s LocalUserToSpeak();\n", __FUNCTION__);
         }
      }
   }
}

void VhallIALive::HandleChatMsg(CustomOnHttpResMsgEvent* customEvent) {
   if (customEvent) {
      VhallAudienceUserInfo Info;
      Info.role = customEvent->mRoleName.toStdWString();
      Info.userId = customEvent->mRecvUserId.toStdWString();
      Info.userName = customEvent->mNickName.toStdWString();
      Info.gagType = customEvent->is_banned == true ? VhallShowType_Allow : VhallShowType_Hide;
      Info.miUserCount = customEvent->mUv;
      Info.mDevType = customEvent->devType;
      if (customEvent->mMsg == "Join") {
         if (NULL != ui.tabWidget->mpMemberListDlg) {
            ui.tabWidget->mpMemberListDlg->NtfUserOnline(Info);
            //操作对象:嘉宾	已上麦	举手（实心红色）	下麦、设为主讲人、禁言、踢出
            if (mActiveParam.role_name == USER_HOST) {
               ui.tabWidget->mpMemberListDlg->ShowSetmainSpeaker(mActiveParam.mainShow);
            }
         }
         SetSpeakUserOffLine(QString::fromStdWString(Info.userId), false);
         RemoveCheckSpeakUser(QString::fromStdWString(Info.userId));
         mCheckSpeakUserTimeOut.stop();
         mCheckSpeakUserTimeOut.start(CHECK_OFFLINE_TIMEOUT);
      }
      else if (customEvent->mMsg == "Leave") {
         VH::CComPtr<ICommonData> pCommonData;
         DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
         ClientApiInitResp info;
         pCommonData->GetInitApiResp(info);
         if (NULL != ui.tabWidget->mpMemberListDlg && info.user_id != Info.userId) {           
            ui.tabWidget->mpMemberListDlg->NtfUserOffLine(QString::fromStdWString(Info.userId), Info.miUserCount);
            SetSpeakUserOffLine(QString::fromStdWString(Info.userId), true);
         }
         TRACE6("%s Info.userId:%s\n", __FUNCTION__, QString::fromStdWString(Info.userId).toStdString().c_str());
         mCheckSpeakUserTimeOut.stop();
         mCheckSpeakUserTimeOut.start(CHECK_OFFLINE_TIMEOUT);
      }
   }
}

void VhallIALive::HandleVSSHandsUpRespParam(int libCurlCode, std::string msg, std::string) {
   if (libCurlCode != 0) {
      FadeOutTip(HTTP_SEND_TIMEOUT, TipsType_Error);
   }
   else {
      int nCode = 0;
      std::string respMsg;
      InteractAPIManager::ParamHttpResp(msg, nCode, respMsg);
      if (nCode != 200) {
         FadeOutTip(QString::fromStdString(respMsg), TipsType_Error);
      }
      else {
         if (mActiveParam.mbIsH5Live) {
            vlive::GetPaasSDKInstance()->ApplyInavPublish();
         }
         else {
            HandleHttpResponseHandsup("", QString::fromStdString(respMsg), nCode, mActiveParam.join_uid, true);
         }
      }
   }
}

void VhallIALive::HandleVSSCancelHandsUpRespParam(int libCurlCode, std::string msg, std::string)
{
   if (libCurlCode != 0) {
      FadeOutTip(HTTP_SEND_TIMEOUT, TipsType_Error);
   }
   else {
      int nCode = 0;
      std::string respMsg;
      InteractAPIManager::ParamHttpResp(msg, nCode, respMsg);
      if (nCode != 200) {
         FadeOutTip(QString::fromStdString(respMsg), TipsType_Error);
      }
      else {
         ui.widget_title->stopHandsUpCountDown();
      }
   }
}

void VhallIALive::HandleVSSChangeToSSpeakResp(int libCurlCode, std::string msg, std::string uid) {
   if (libCurlCode != 0) {
      FadeOutTip(HTTP_SEND_TIMEOUT, TipsType_Error);
   }
   else {
      int nCode = 0;
      std::string respMsg;
      InteractAPIManager::ParamHttpResp(msg, nCode, respMsg);
      if (nCode != 200) {
         FadeOutTip(QString::fromStdString(respMsg), TipsType_Error);
      }
      else {
         HandleHttpResponseChangeToSpeak("", QString::fromStdString(respMsg), nCode, mActiveParam.join_uid, true);
      }
   }
}

void VhallIALive::HandleVSSNoSpeakResp(int libCurlCode, std::string msg, std::string uid, bool bShowNotice) {
   if (libCurlCode != 0) {
      FadeOutTip(HTTP_SEND_TIMEOUT, TipsType_Error);
   }
   else {
      int nCode = 0;
      std::string respMsg;
      InteractAPIManager::ParamHttpResp(msg, nCode, respMsg);
      if (bShowNotice) {
         if (nCode != 200) {
            FadeOutTip(QString::fromStdString(respMsg), TipsType_Error);
         }
         else {
            HandleHttpResponseNotSpeak("", QString::fromStdString(respMsg), nCode, QString::fromStdString(uid), bShowNotice);
         }
      }
   }
}

void VhallIALive::HandleSetHandlsUpMsg(CustomOnHttpResMsgEvent* customEvent) {
   if (customEvent->mCode != 0) {
      FadeOutTip(HTTP_SEND_TIMEOUT, TipsType_Error);
   }
   else {
      QJsonParseError json_error;
      QJsonArray speak_array;
      int arraySize = 0;
      QString name;
      QJsonDocument parse_doucment = QJsonDocument::fromJson(QByteArray(customEvent->mMsg.toStdString().c_str()), &json_error);
      if (json_error.error == QJsonParseError::NoError) {
         if (parse_doucment.isObject()) {
            QJsonObject obj = parse_doucment.object();
            int nCode = 0;
            QString respMsg;
            if (obj.contains("code")) {
               nCode = obj["code"].toInt();
            }
            if (obj.contains("msg")) {
               respMsg = obj["msg"].toString();
            }
            if (nCode == 200) {
               ui.tabWidget->mpMemberListDlg->DealSwitchHansUp(customEvent->is_open);
               mActiveParam.handsUp = customEvent->is_open ? 1 : 0;
            }
            else {
               FadeOutTip(respMsg, TipsType_Error);
            }
         }
      }
   }
}

void VhallIALive::HandleVSSSendMsg(int libCurlCode, std::string msg, std::string, std::string) {
   if (libCurlCode != 0) {
      FadeOutTip(HTTP_SEND_TIMEOUT, TipsType_Error);
   }
   else {
      QJsonParseError json_error;
      QJsonArray speak_array;
      int arraySize = 0;
      QString name;
      QJsonDocument parse_doucment = QJsonDocument::fromJson(QByteArray(msg.c_str()), &json_error);
      if (json_error.error == QJsonParseError::NoError) {
         if (parse_doucment.isObject()) {
            QJsonObject obj = parse_doucment.object();
            int nCode = 0;
            QString respMsg;
            if (obj.contains("code")) {
               nCode = obj["code"].toInt();
            }
            if (obj.contains("msg")) {
               respMsg = obj["msg"].toString();
            }
            if (nCode != 200) {
               FadeOutTip(respMsg, TipsType_Error);
            }
         }
      }
   }
}

void VhallIALive::CheckHostShowUpperInMemberList() {
   if (mbIsHostSetGuestToSpeaker && mActiveParam.role_name == USER_HOST && mActiveParam.bIsLiving) {
      VhallRenderWdg* render = GetRenderWnd(mActiveParam.join_uid);
      if (render) {
         if (render->IsTeacherPos()) {
            ui.tabWidget->mpMemberListDlg->SetSelfUpperWheat(false);  //找到了在大窗中，隐藏上下麦
            ui.tabWidget->mpMemberListDlg->SetEnableWheat(false);

            render->ShowSetKickOutBtn(false);
         }
         else {
            ui.tabWidget->mpMemberListDlg->SetSelfUpperWheat(false);   //找到了在小窗中，显示下麦
            ui.tabWidget->mpMemberListDlg->SetEnableWheat(true);

            render->ShowSetKickOutBtn(true);
         }
      }
      else {
         //没找到，显示上麦
         ui.tabWidget->mpMemberListDlg->SetSelfUpperWheat(true);
         ui.tabWidget->mpMemberListDlg->SetEnableWheat(true);
      }
   }
   else {
      ui.tabWidget->mpMemberListDlg->SetEnableWheat(false);
   }
}

void VhallIALive::ProhibiteLedPutDownWheat(const QString& id)
{
   if (mActiveParam.join_uid.compare(id) == 0 && mActiveParam.role_name != USER_HOST){//被禁言的是本人切正在推流
      bProhibit = true;
      if (mbIsGuestSpeaking || IsExistRenderWnd(mActiveParam.join_uid)) {
         TRACE6("%s mbIsGuestSpeaking:%d id:%s\n", __FUNCTION__, mbIsGuestSpeaking, id.toStdString().c_str());
         HttpSendNotSpeak(mActiveParam.join_uid, false);//发送下麦请求
      }
      if (mpInviteJoinWdg) {
         mpInviteJoinWdg->close();
      }
   }
   else if (mActiveParam.role_name == USER_HOST) {
      std::map<QString, VhallRenderWdg*>::iterator iter = mRenderMap.find(id);
      if (iter != mRenderMap.end()) {
         if (iter->second->GetUserRole() == USER_USER) {
            HttpSendNotSpeak(id, false);//发送下麦请求
         }
      }
   }
}

void VhallIALive::HandleRecvSocketIOMsgEvent(Event &event) {
   TRACE6("%s enter event.m_eMsgType:%d\n", __FUNCTION__, event.m_eMsgType);
   if (!mbExitRoom){
      if (event.m_bSendMsg == 1) {
         int nCode = 200;
         std::string errMsg;
         InteractAPIManager::ParamHttpResp(event.context.toStdString(), nCode, errMsg);
         if (nCode != 200) {
            FadeOutTip(QString::fromStdString(errMsg).isEmpty() ? DO_ERR : QString::fromStdString(errMsg), TipsType_Error);
         }
         return;
      }

      switch (event.m_eMsgType)
      {
      case e_RQ_UserProhibitSpeakOneUser://单个用户禁言
      {
         if (ui.tabWidget->mpMemberListDlg) {
            ui.tabWidget->mpMemberListDlg->RecvUserListMsg(event);
         }
         ProhibiteLedPutDownWheat(QString::fromStdWString(event.m_oUserInfo.userId));
         break;
      }
      case e_RQ_UserProhibitSpeakAllUser:
      {
         if (ui.tabWidget->mpMemberListDlg) {
            ui.tabWidget->mpMemberListDlg->RecvUserListMsg(event);
         }
         if (mActiveParam.role_name.compare(USER_HOST) != 0)
            ProhibiteLedPutDownWheat(mActiveParam.join_uid);
         break;
      }
      case e_RQ_ReleaseAnnouncement: {
         HandleRecvAnnouncement(event);
         break;
      }
      case e_RQ_UserAllowSpeakOneUser://单个用户取消禁言
      {
         if (ui.tabWidget->mpMemberListDlg) {
            ui.tabWidget->mpMemberListDlg->RecvUserListMsg(event);
         }
         if (mActiveParam.join_uid.compare(event.m_oUserInfo.userId) == 0)//被禁言的是本人切正在推流
         {
            bProhibit = false;
         }
         break;
      }
      case e_RQ_UserAllowSpeakAllUser:
      {
         if (ui.tabWidget->mpMemberListDlg) {
            ui.tabWidget->mpMemberListDlg->RecvUserListMsg(event);
         }
         bProhibit = false;
         break;
      }
      case e_RQ_SearchOnLineMember:
      case e_RQ_UserAllowJoinOneUser://允许单个用户加入	
      case e_Ntf_UserOffline:
      case e_RQ_UserOnlineList://在线列表
      case e_RQ_UserProhibitSpeakList://禁言列表
      case e_RQ_UserKickOutList://踢出列表	
      case e_RQ_UserSpecialList: //特殊用户列表 主持人、嘉宾、助理
      case e_RQ_replyInvite_Error: {
         if (ui.tabWidget->mpMemberListDlg) {
            ui.tabWidget->mpMemberListDlg->RecvUserListMsg(event);
         }
         //如果支持人已经把嘉宾设置成主讲人
         if (e_RQ_UserSpecialList == event.m_eMsgType || e_RQ_UserOnlineList == event.m_eMsgType) {
            CheckHostShowUpperInMemberList();
            //操作对象:嘉宾	已上麦	举手（实心红色）	下麦、设为主讲人、禁言、踢出
            if (mActiveParam.role_name == USER_HOST) {
               ui.tabWidget->mpMemberListDlg->ShowSetmainSpeaker(mActiveParam.mainShow);
            }
         }
         break;
      }
      case e_Ntf_UserOnline: {
         break;
      }
      case e_RQ_setDefinition: {
         break;
      }
      case e_RQ_switchHandsup_open://举手开关-开
      case e_RQ_switchHandsup_close://举手开关-关
      {
         if (mActiveParam.role_name.compare(USER_HOST) == 0 && ui.tabWidget->mpMemberListDlg)
            ui.tabWidget->mpMemberListDlg->RecvUserListMsg(event);
      }
      break;
      case e_RQ_handsUp:{ //举手
         if (((mActiveParam.role_name.compare(USER_HOST) == 0 || mActiveParam.role_name.compare(USER_GUEST) == 0) && QString::fromStdWString(event.m_oUserInfo.role).compare(USER_USER) == 0) ||
            (mActiveParam.role_name.compare(USER_GUEST) == 0 && QString::fromStdWString(event.m_oUserInfo.role).compare(USER_GUEST) == 0)) {//观众 + 嘉宾-嘉宾 --- 举手
            if (ui.tabWidget->mpMemberListDlg) {
               ui.tabWidget->mpMemberListDlg->addRaiseHandsMember(event.m_oUserInfo, true, false);
               if (ui.tabWidget->mpMemberListDlg->isHidden()) {
                  ui.tabWidget->SetHandsUpState(eHandsUpState_existence);
                  ui.widget_leftTool->SetEixtUnseeHandsUpList(true);
               }
            }
         } 
         else if (mActiveParam.role_name.compare(USER_HOST) == 0){  //主持人收到嘉宾申请上麦
            QString strId = QString::fromStdWString(event.m_oUserInfo.userId);
            QString strNickName = QString::fromStdWString(event.m_oUserInfo.userName);
            QMap<QString, ApplyJoinWdg*>::iterator it = mApplyJoinWdg.find(strId);
            ApplyJoinWdg* pWdg = NULL;
            if (it != mApplyJoinWdg.end()) { 
               pWdg = it.value();
            }
            if (NULL == pWdg){
               pWdg = new ApplyJoinWdg(this);
               pWdg->SetUid(strId, strNickName);
               mApplyJoinWdg[strId] = pWdg;
               connect(pWdg, &ApplyJoinWdg::sig_AgreeUpper, this, &VhallIALive::slotAgreeUpper);
               connect(pWdg, &ApplyJoinWdg::sig_RejectUpper, this, &VhallIALive::slot_RejectSpeak);
            }
            if (NULL != pWdg){
               pWdg->InitTimeOut(30);
               if (mbIsDeviceChecking) {
                  pWdg->hide();
               }
               else {
                  ConfigSetting::MoveWindow(pWdg, this->isVisible() ? this : nullptr);
                  pWdg->show();
               }
            }
            if (ui.tabWidget->mpMemberListDlg) {
               ui.tabWidget->mpMemberListDlg->UpdataOnLineUserHandsUpState(strId, true);
            }          
         }
      }
      break;
      case e_RQ_CancelHandsUp: {
         if (ui.tabWidget->mpMemberListDlg) {
            ui.tabWidget->mpMemberListDlg->UpdataOnLineUserHandsUpState(QString::fromStdWString(event.m_oUserInfo.userId), false);
         }

         if (mActiveParam.role_name.compare(USER_HOST) == 0){//主持人 收到 嘉宾申请上麦
            QString strId = QString::fromStdWString(event.m_oUserInfo.userId);
            QString strNickName = QString::fromStdWString(event.m_oUserInfo.userName);
            QMap<QString, ApplyJoinWdg*>::iterator it = mApplyJoinWdg.find(strId);
            if (it != mApplyJoinWdg.end()) {
               ApplyJoinWdg* pWdg = NULL;
               pWdg = it.value();
               if (NULL != pWdg){
                  delete pWdg;
                  mApplyJoinWdg.erase(it);
               }
            }
         }
         break;
      }
      case e_RQ_UserKickOutOneUser: { //踢出单个用户
         HandleKickOutUser(event);
         break;
      }
      case e_RQ_addSpeaker: {	 //有用户上麦。
         HandleAddSpeaker(event);
         break;
      }
      case e_RQ_agreeSpeaker: {  //主持人同意我上麦
         HandleAgreeSpeaker(event);
         break;
      }
      case e_RQ_notSpeak: { //有用户下麦。
         HandleNoSpeaker(event);
         break;
      }
      case e_RQ_switchDevice_close: {	  //设备开关-关闭
         HandleSwitchDeviceClose(event);
         break;
      }
      case e_RQ_switchDevice_open: { //设备开关-开
         HandleSwitchDeviceOpen(event);
         break;
      }
      case e_RQ_sendInvite: {	//收到邀请上麦消息，需要判断是否是自己并且自己的身份不是主持。
         TRACE6("bProhibit :%d", bProhibit);
         if (!bProhibit && QString::fromStdWString(event.m_oUserInfo.userId).compare(mActiveParam.join_uid) == 0 && mActiveParam.role_name.compare(USER_HOST) != 0) {
            if (!GetWebRtcSDKInstance()->HasAudioDev() && !GetWebRtcSDKInstance()->HasVideoDev()) {
               FadeOutTip(CHECK_DEVICE_ERR, TipsType_Error);
               TRACE6("%s HasVideoOrAudioDev\n", __FUNCTION__);
               return;
            }
            RecvHostInviteMsg();
         }
         break;
      }
      case e_RQ_setMainShow:
      case e_RQ_setMainSpeaker: { //设置主讲人。 //主画面
         HandleSwitchMainView(event);
         break;
      }
      case e_RQ_setPublishStart: {
         HandleStartPush(event);
         break;
      }
      case e_RQ_setOver: {
         HandleOverPush(event);
         break;
      }
      case e_RQ_replyInvite_Fail: {
         if (mActiveParam.role_name.compare(USER_HOST) == 0) {
            FadeOutTip(QString::fromStdWString(event.m_oUserInfo.userName) + REJUEST_SPEAK, TipsType_Error);
            if (ui.tabWidget->mpMemberListDlg) {
               ui.tabWidget->mpMemberListDlg->RecvUserListMsg(event);
            }
         }
         break;
      }
      case e_RQ_replyInvite_Suc: {
         if (mActiveParam.role_name.compare(USER_HOST) == 0) {
            FadeOutTip(QString::fromStdWString(event.m_oUserInfo.userName) + ACCEPT_SPEAK, TipsType_Normal);
            if (ui.tabWidget->mpMemberListDlg) {
               ui.tabWidget->mpMemberListDlg->RecvUserListMsg(event);
               if (mActiveParam.role_name == USER_HOST) {
                  ui.tabWidget->mpMemberListDlg->ShowSetmainSpeaker(mActiveParam.mainShow);
               }
            }
         }
         break;
      }
      case e_RQ_RejectSpeak://拒绝申请上麦
      {
         QString strId = QString::fromStdWString(event.m_oUserInfo.userId);
         if (strId.compare(mActiveParam.join_uid) == 0)
         {
            ui.widget_title->stopHandsUpCountDown();
            //提示拒绝上麦
            FadeOutTip(HOST_REJECT_SPEAK, TipsType_Error);
         }
      }
      break;
      default:
         break;
      }

   }
   TRACE6("%s leave event.m_eMsgType:%d\n", __FUNCTION__, event.m_eMsgType);
}

long long VhallIALive::GetShowUserMember(){
   int count = 0;
   if (NULL != ui.tabWidget->mpMemberListDlg) {
      count = ui.tabWidget->mpMemberListDlg->GetShowUserMember();
   }
   return count;
}

void VhallIALive::HandleJoinMember(VhallAudienceUserInfo* Info) {
   TRACE6("%s\n", __FUNCTION__);
   if (Info == NULL) {
      return;
   }
   if (Info) {
      if (NULL != ui.tabWidget->mpMemberListDlg) {
         ui.tabWidget->mpMemberListDlg->NtfUserOnline(*Info);
         //操作对象:嘉宾已上麦	举手（实心红色下麦、设为主讲人、禁言、踢出
         //if (mActiveParam.role_name == USER_HOST) {
         //   ui.tabWidget->mpMemberListDlg->ShowSetmainSpeaker(mActiveParam.mainShow);
         //}
      }
      SetSpeakUserOffLine(QString::fromStdWString(Info->userId), false);
      RemoveCheckSpeakUser(QString::fromStdWString(Info->userId));
   }
   TRACE6("%s end\n", __FUNCTION__);
}

void VhallIALive::HandleLeaveMember(VhallAudienceUserInfo* Info) {
   TRACE6("%s\n", __FUNCTION__);
   if (Info == NULL) {
      return;
   }
   if (Info) {
      VH::CComPtr<ICommonData> pCommonData;
      DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
      ClientApiInitResp info;
      pCommonData->GetInitApiResp(info);
      if (NULL != ui.tabWidget->mpMemberListDlg && Info->userId != info.user_id) {
         ui.tabWidget->mpMemberListDlg->NtfUserOffLine(QString::fromStdWString(Info->userId), Info->miUserCount);
         SetSpeakUserOffLine(QString::fromStdWString(Info->userId), true);
      }
      TRACE6("%s Info.userId:%s\n", __FUNCTION__, QString::fromStdWString(Info->userId).toStdString().c_str());
   }
   TRACE6("%s end\n", __FUNCTION__);
}

void VhallIALive::HandleLostLocalCameraStream(bool restart) {
   TRACE6("%s enter\n", __FUNCTION__);
   if (!mCurCameraDevID.isEmpty()) {
      HttpSendDeviceStatus(mActiveParam.join_uid, CAMERA_DEVICE, 0);
      ChangeLocalCameraDevState(false, mActiveParam.join_uid);
      TRACE6("%s mCurCameraDevID. %s\n", __FUNCTION__, mCurCameraDevID.toStdString().c_str());
      if (restart) {
         mCameraList.clear();
         bool bFindDev = false;
         std::list<vhall::VideoDevProperty> cameraList;
         TRACE6("%s GetCameraDevices\n", __FUNCTION__);
         GetWebRtcSDKInstance()->GetCameraDevices(cameraList);
         TRACE6("%s GetCameraDevices %d\n", __FUNCTION__, cameraList.size());
         std::list<vhall::VideoDevProperty>::iterator it = cameraList.begin();
         while (it != cameraList.end()) {
            VhallLiveDeviceInfo info(QString::fromStdString(it->mDevId), QString::fromStdString(it->mDevName), it->mIndex);
            mCameraList.push_back(info);
            if (mCurCameraDevID == QString::fromStdString(it->mDevId)) {
               bFindDev = true;
            }
            it++;
         }
         if (!bFindDev) {
            mCurCameraDevID.clear();
            mCurrentCameraIndex = 0;
            TRACE6("%s no devices\n", __FUNCTION__);
         }
         else {
            StartLiveWithOpenDevice();
         }
      }
   }
   TRACE6("%s leave\n", __FUNCTION__);
}

void VhallIALive::HandleStopLiving() {
   TRACE6("VhallInteractive::HandleStopLiving() start\n");
   if (!IsMainRenderNormalPos()) {
      slot_SwitchMainViewLayout();
   }
   if (mpExitWaiting) {
      mpExitWaiting->Close();
   }
   if (mConfigBroadCast) {
      mConfigBroadCast->SetNeedConfigBroadCast(true);
      TRACE6("%s SetNeedConfigBroadCast true\n", __FUNCTION__);
   }
   mbIsHostUnPublished = false;
   mbIsHostSetGuestToSpeaker = false;
   mLocalStreamID = "";
   TRACE6("%s mbIsHostSetGuestToSpeaker false\n", __FUNCTION__);
   CheckHostShowUpperInMemberList();
   if (mpSettingDlg) {
      mpSettingDlg->SetStartLiving(false);
   }
   TRACE6("%s SetStreamButtonStatus false\n", __FUNCTION__);
   ui.widget_title->SetStreamButtonStatus(false);
   ui.widget_title->SetLivingState(false);
   m_pScreenShareToolWgd->SetStreamButtonStatus(false);
   RemoveAllRender();
   ResetToNospeakState();
   mStartLiveTimer.stop();
   mStartLiveDateTime.setTime_t(0);
   SetMainViewBackGroud(MAIN_VIEW_IS_END);
   if (mpExitWaiting) {
      mpExitWaiting->Close();
   }
   mActiveParam.bIsLiving = false;
   mActiveParam.mainShow = mActiveParam.join_uid;
   mActiveParam.currPresenter = mActiveParam.join_uid;
   mActiveParam.mainShowRole = USER_HOST;

   ResetPlayMediaFileAndDesktopShare();
   STRU_MAINUI_LOG log;
   wstring uploadData = L"key=stream_start&k=%d&tv=rtp";
   swprintf_s(log.m_wzRequestUrl, DEF_MAX_HTTP_URL_LEN, uploadData.c_str(), eLogRePortK_StopStream);
   SingletonMainUIIns::Instance().PostCRMessage(MSG_MAINUI_LOG, &log, sizeof(STRU_MAINUI_LOG));
   TRACE6("VhallInteractive::HandleStopLiving() end\n");
}

void VhallIALive::HandleStartLiving() {
   TRACE6("%s SetStreamButtonStatus true\n", __FUNCTION__);
   ui.widget_title->SetStreamButtonStatus(true);
   ui.widget_title->SetLivingState(true);
   ui.widget_title->StartWithActiveLive(0);
   m_pScreenShareToolWgd->SetStreamButtonStatus(true);
   //如果登录用户是主画面，自己的画面先显示在主画面区域
   if (mActiveParam.join_uid.compare(mActiveParam.mainShow) == 0) {
      AppendRenderUser(mActiveParam.join_uid, mActiveParam.role_name, mActiveParam.userNickName, true, true, true, false);
   }
   if (mActiveParam.role_name == USER_HOST) {
      mStartLiveDateTime.setTime_t(0);
      mStartLiveTimer.start(1000);  
      InitBroadCast();
   }
   //mbIsHostStartLiving = true;
   if (mpStartWaiting) {
      mpStartWaiting->Close();
   }
   TRACE6("%s Getlocalstream mbEnableStopStream %d\n", __FUNCTION__, mbEnableStopStream);
   StartLiveWithOpenDevice();
   SetMainViewBackGroud(MAIN_VIEW_IS_LIVING);
   mActiveParam.bIsLiving = true;
   if (mDesktopCameraRenderWdgPtr) {
      mDesktopCameraRenderWdgPtr->SetCurrentIndex(1);
   }

   if (mbIsPlayMediaFile) {
      mbIsPushMediaErr = true;
      slot_PublishMediaFileStreamTimeout();
   }
   if (m_pScreenShareToolWgd->isVisible()) {
      slot_PublishDesktopStreamTimeout();
   }
   if (mDocCefWidget) {
      mDocCefWidget->AppCallJsFunction("qtCallJsStartLive(1)");
      TRACE6("%s qtCallJsStartLive\n", __FUNCTION__);
   }
   STRU_MAINUI_LOG log;
   swprintf_s(log.m_wzRequestUrl, DEF_MAX_HTTP_URL_LEN, L"key=stream_start&k=%d&tv=true", eLogRePortK_StartStream);
   StreamStatus status;
   memset(&status, 0, sizeof(StreamStatus));
   QJsonObject body;
   body["tv"] = "true";   
   QString json = CPathManager::GetStringFromJsonObject(body);
   strcpy_s(log.m_wzRequestJson, json.mid(0, DEF_MAX_HTTP_URL_LEN).toLatin1().data());
   SingletonMainUIIns::Instance().PostCRMessage(MSG_MAINUI_LOG, &log, sizeof(STRU_MAINUI_LOG));
   //主持人开始直播
   mActiveParam.mainShow = mActiveParam.join_uid;
   mActiveParam.currPresenter = mActiveParam.join_uid;
   mActiveParam.mainShowRole = USER_HOST;
   InitLeftBtnEnableState();
   if (mpSettingDlg)
      mpSettingDlg->SetStartLiving(true);
}

void VhallIALive::HandlePushStreamReConnect(QEvent* event) {
   TRACE6("%s\n", __FUNCTION__);
   if (event == NULL) {
      return;
   }
}

void VhallIALive::HandleReSubScribeLocalStream() {
   TRACE6("%s\n", __FUNCTION__);
   std::map<QString, VhallRenderWdg*>::iterator iter = mRenderMap.find(mActiveParam.join_uid);
   if (iter != mRenderMap.end()) {
      iter->second->SetEnableUpdateRendWnd(true);
   }
}

void VhallIALive::HandleReSubScribeDesktopStream() {
   if (mDesktopWdg) {
      mDesktopWdg->setUpdatesEnabled(true);
   }
}

void VhallIALive::HandleReSubScribeMediaStream() {
   if (mMediaFilePlayWdg) {
      mMediaFilePlayWdg->setUpdatesEnabled(true);
   }
}

void VhallIALive::HandleRePublishLocalStream() {
   TRACE6("%s enter\n", __FUNCTION__);
   std::map<QString, VhallRenderWdg*>::iterator iter = mRenderMap.find(mActiveParam.join_uid);
   if (iter != mRenderMap.end()) {
      iter->second->SetEnableUpdateRendWnd(true);
   }
   TRACE6("%s leave\n", __FUNCTION__);
}

void VhallIALive::HandleRePublishDesktopStream() {
   if (mDesktopWdg) {
      TRACE6("%s setUpdatesEnabled\n", __FUNCTION__);
      mDesktopWdg->setUpdatesEnabled(true);
   }
   TRACE6("%s\n", __FUNCTION__);
}

void VhallIALive::HandleRePublishMediaStream() {
   if (mMediaFilePlayWdg) {
      TRACE6("%s setUpdatesEnabled\n", __FUNCTION__);
      mMediaFilePlayWdg->setUpdatesEnabled(true);
   }
   TRACE6("%s\n", __FUNCTION__);
}


void VhallIALive::HandleConnectRoomSuc() {
   TRACE6("%s start\n", __FUNCTION__);
   InitBroadCast();
   mIsCallStartMixStream = false;

   //此处是处理主持人连接媒体服务器断开之后，重连成功重新推流处理。
   if (mActiveParam.role_name == USER_HOST && mActiveParam.bIsLiving) {
      TRACE6("%s mActiveParam.role_name == USER_HOST && mbIsHostStartLiving\n", __FUNCTION__);
      GetWebRtcSDKInstance()->StopPushLocalStream();
      slot_PublishLocalStreamTimeout();
   }
   else if (mActiveParam.role_name != USER_HOST) {
      TRACE6("%s mActiveParam.role_name:%s\n", __FUNCTION__, mActiveParam.role_name.toStdString().c_str());
      if (mbInitInRoom && GetRenderWnd(mActiveParam.join_uid)) {
         mbInitInRoom = false;
         ui.widget_title->SetToSpeakBtnState(false);
         StartLiveWithOpenDevice();
         mbIsGuestSpeaking = true;
      }
      else if(mbIsGuestSpeaking){
         GetWebRtcSDKInstance()->StopPushLocalStream();
         slot_PublishLocalStreamTimeout();
      }
   }
   mCheckSpeakUserTimeOut.stop();
   mCheckSpeakUserTimeOut.start(CHECK_OFFLINE_TIMEOUT);
   if (GetWebRtcSDKInstance()->IsCapturingStream(VHStreamType_MediaFile)) {
      mRePublishMediaFileStreamTimer.start(3000);
      mRePublishMediaFileStreamTimer.setSingleShot(true);
   }
   if (GetWebRtcSDKInstance()->IsCapturingStream(VHStreamType_Desktop)) {
      mRePublishDesktopStreamTimer.start(3000);
      mRePublishDesktopStreamTimer.setSingleShot(true);
   }
   TRACE6("%s end\n", __FUNCTION__);
}

void VhallIALive::HandleConnectRoomErr() {
   if (mbCallRoomDisConnect) {
      return;
   }
   FadeOutTip(RTC_CONNECT_ERR, TipsType_Error, 5000);
   mRoomReConnectTimer.stop();
   mRoomReConnectTimer.start(MAX_ROOM_RECONNECT_TIMEOUT);
   TRACE6("%s mActiveParam.role_name:%s\n", __FUNCTION__, mActiveParam.role_name.toStdString().c_str());
   ResetPlayMediaFileAndDesktopShare();
}

void VhallIALive::HandleConfigDesktopMainView(QEvent* event) {
   TRACE6("%s\n", __FUNCTION__);
   if (event == NULL) {
      return;
   }
   CustomRecvMsgEvent* eventMsg = (CustomRecvMsgEvent*)event;
   if (eventMsg) {
      TRACE6("%s msg:%s\n", __FUNCTION__, eventMsg->msg.toStdString().c_str());
      if (eventMsg->msg.compare("success") != 0) {
         mSetDeskTopMainViewTimer.start(2000);
         mSetDeskTopMainViewTimer.setSingleShot(true);
      }
      else {
         mSetDeskTopMainViewTimer.stop();
         SetLayOutMode(CANVAS_LAYOUT_PATTERN_GRID_1);
      }
   }
}

void VhallIALive::HandleHostExitRoom() {
   TRACE6("%s enter\n", __FUNCTION__);
   mActiveParam.bIsLiving = false;
   if (mActiveParam.role_name.compare(USER_HOST) != 0) {
      if (mpSettingDlg) {
         ConfigSetting::writeValue(CPathManager::GetConfigPath(), GROUP_DEFAULT, PUSH_DESKTOP_CAPTURE_OPEN, 0);
         mpSettingDlg->ShowDesktopCaptrue(false);
      }
      ui.widget_title->SetToSpeakBtnState(true);
      mbIsGuestSpeaking = false;
      TRACE6("%s mbIsGuestSpeaking = false\n", __FUNCTION__);
      m_pScreenShareToolWgd->SetStreamButtonStatus(false);
      RemoveAllRender();
      ResetToNospeakState();
      mStartLiveTimer.stop();
      mStartLiveDateTime.setTime_t(0);
      ResetPlayMediaFileAndDesktopShare();
   }
   TRACE6("%s leave\n", __FUNCTION__);
}

void VhallIALive::HandleSubStream(QEvent* event) {
   TRACE6("%s\n", __FUNCTION__);
   if (event == NULL) {
      return;
   }
   CustomRecvMsgEvent* eventMsg = (CustomRecvMsgEvent*)event;
   if (eventMsg) {
      TRACE6("%s msg:%s\n", __FUNCTION__, eventMsg->msg.toStdString().c_str());
   }
}

void VhallIALive::HandleUnPushMediaFileStream(QEvent* event) {
   TRACE6("%s \n", __FUNCTION__);
}

void VhallIALive::HandleLayOutMode(QEvent* event) {
   TRACE6("%s\n", __FUNCTION__);
   if (event == NULL) {
      return;
   }
   CustomRecvMsgEvent* eventMsg = (CustomRecvMsgEvent*)event;
   if (eventMsg) {
      TRACE6("%s msg:%s\n", __FUNCTION__, eventMsg->msg.toStdString().c_str());
      if (eventMsg->msg.compare("success") != 0) {
         mReSetLayOutTimer.stop();
         mReSetLayOutTimer.start(2000);
      }
   }
}

void VhallIALive::HandleConfigMediaFileMainView(QEvent* event) {
   TRACE6("%s\n", __FUNCTION__);
   if (event == NULL) {
      return;
   }
   CustomRecvMsgEvent* eventMsg = (CustomRecvMsgEvent*)event;
   if (eventMsg) {
      TRACE6("%s msg:%s\n", __FUNCTION__, eventMsg->msg.toStdString().c_str());
      if (eventMsg->msg.compare("success") != 0) {
         mSetMediaFileMainViewTimer.start(2000);
         mSetMediaFileMainViewTimer.setSingleShot(true);
      }
      else {
         mSetMediaFileMainViewTimer.stop();
         SetLayOutMode(CANVAS_LAYOUT_PATTERN_GRID_1);
      }
   }
}

void VhallIALive::HandleConfigBoradCastOnlyOne(QEvent* event) {
   if (event == NULL) {
      return;
   }
   CustomRecvMsgEvent* eventMsg = (CustomRecvMsgEvent*)event;
   if (eventMsg) {
      TRACE6("%s msg:%s\n", __FUNCTION__, eventMsg->msg.toStdString().c_str());
      if (eventMsg->msg.compare("success") != 0) {
         Sleep(300);
         SetLayOutMode(CANVAS_LAYOUT_PATTERN_GRID_1);
      }
   }
}

void VhallIALive::HandleConfigBoradCast(QEvent *event) {
   TRACE6("%s \n", __FUNCTION__);
   if (event) {
      CustomRecvMsgEvent* eventMsg = (CustomRecvMsgEvent*)event;
      if (eventMsg) {
         TRACE6("%s eventMsg.msg:%s\n", __FUNCTION__, eventMsg->msg.toStdString().c_str());
         if (eventMsg->msg.compare("success") == 0) {
            TRACE6("%s set config borad cast success mainShow:%s\n", __FUNCTION__, mActiveParam.mainShow.toStdString().c_str());
            if (GetWebRtcSDKInstance()->IsPushingStream(VHStreamType_Desktop) || mDesktopWdg != nullptr ||
               GetWebRtcSDKInstance()->IsPushingStream(VHStreamType_MediaFile) || mMediaFilePlayWdg != nullptr) {
               return;
            }
            ConfigRoomMainView(mActiveParam.mainShow);
         }
         else {
            InitBroadCast(true);
         }
      }
   }
}

void VhallIALive::HandlePlayMediaFileSuc() {
   TRACE6("%s\n", __FUNCTION__);
   if (GetWebRtcSDKInstance()->GetCaptureStreamType(mActiveParam.join_uid.toStdWString(), VHStreamType_MediaFile, CaptureStreamAVType_Video)) {
      if (mMediaFilePlayWdg == NULL) {
         mMediaFilePlayWdg = new QWidget(this);
         TRACE6("%s create mMediaFilePlayWdg\n", __FUNCTION__);
      }
      if (mMediaFilePlayWdg) {
         if (mDocCefWidget) {
            mDocCefWidget->hide();
            ui.widget_docLoadingTips->hide();
         }
         mMediaFilePlayWdg->installEventFilter(this);
         ui.verticalLayout_mainWebView->addWidget(mMediaFilePlayWdg);
         HWND wnd = (HWND)(mMediaFilePlayWdg->winId());
         GetWebRtcSDKInstance()->StartRenderLocalStream(VHStreamType_MediaFile, wnd);
         if (mActiveParam.role_name == USER_HOST && !mActiveParam.bIsLiving) {
            VideoProfileIndex profile = VhallIALiveSettingDlg::GetPlayMediaFileProfile(mCurPlayFileName);
            GetWebRtcSDKInstance()->PlayFile(mCurPlayFileName.toStdString(), profile);
         }
         mMediaFilePlayWdg->setUpdatesEnabled(false);
         TRACE6("%s mMediaStream play\n", __FUNCTION__);
      }
   }
   else {
      StopShowMediaVideo();
   }
   slot_ClickedMic(mActiveParam.join_uid, true);
   FadeOutTip(PLAY_FILE_AND_CLOSE_MIC, TipsType_MediaFilePlay);
   mbIsPlayMediaFile = true;
   if (GetWebRtcSDKInstance()->IsWebRtcRoomConnected() && ((mActiveParam.bIsLiving && mActiveParam.role_name == USER_HOST) || (mActiveParam.role_name != USER_HOST && mbIsGuestSpeaking))) {
      slot_PublishMediaFileStreamTimeout();
      FadeOutTip(PLAY_FILE_LOADING, TipsType_Error);
   }
}

void VhallIALive::StopPushMeidaFileStream() {
   GetWebRtcSDKInstance()->StopPushMediaFileStream();
}

void VhallIALive::HandleSetMediaFileMainView(QString streamId) {
   mCurPlayMediaFileStreamID = streamId;
   TRACE6("%s setMixLayoutMainScreen mCurPlayMediaFileStreamID:%s\n", __FUNCTION__, mCurPlayMediaFileStreamID.toStdString().c_str());
   if (mActiveParam.role_name == USER_HOST) {
      QObject* recvObj = mpMainUILogic;
      GetWebRtcSDKInstance()->SetMixLayoutMainView(mCurPlayMediaFileStreamID.toStdString().c_str(), [&, recvObj](const std::string &result, const std::string &msg, int errorCode) -> void {
         if (errorCode == 20013) {
            //混流房间找不到该成员
            return;
         }
         QString errMsg = QString::fromStdString(result);
         if (recvObj) {
            QCoreApplication::postEvent(mpMainUILogic, new CustomRecvMsgEvent(CustomEvent_ConfigMediaFileMainView, errMsg));
         }
      });
   }
}

void VhallIALive::HandlePushMediaFileStreamSuc(QEvent* event) {
   TRACE6("%s\n", __FUNCTION__);
   if (event == NULL) {
      return;
   }
   CustomRecvMsgEvent* eventMsg = (CustomRecvMsgEvent*)event;
   if (mMediaFilePlayWdg) {
      mMediaFilePlayWdg->setUpdatesEnabled(false);
      HandleSetMediaFileMainView(eventMsg->msg);
      VideoProfileIndex profile = VhallIALiveSettingDlg::GetPlayMediaFileProfile(mCurPlayFileName);
      if (!mbIsPushMediaErr) {
         GetWebRtcSDKInstance()->PlayFile(mCurPlayFileName.toStdString(), profile);
      }
      mbIsPushMediaErr = false;
   }
   InteractAPIManager apiManager(this);
   apiManager.HttpSendChangeWebWatchLayout(1);
}

void VhallIALive::HandleGetLocalStream(QEvent* event) {
   if (event == NULL) {
      return;
   }
   CustomRecvMsgEvent *msgEvent = (CustomRecvMsgEvent*)event;
   TRACE6("%s CustomEvent_GetLocalStreamSuc mbEnableStopStream %d\n", __FUNCTION__, mbEnableStopStream);
   mbEnableStopStream = true;
   if (msgEvent){
      InteractAPIManager apiManager(mpMainUILogic);
      mbHasVideoFrame = msgEvent->mbHasVideo;
      if (mCurCameraDevID.length() > 0 || msgEvent->mbHasVideo) {
         HttpSendDeviceStatus(mActiveParam.join_uid, CAMERA_DEVICE, 1);//2：摄像头 1：开启
      }
      else {
         QString qsConfPath = CPathManager::GetConfigPath();
         int openPushPic = ConfigSetting::ReadInt(qsConfPath, GROUP_DEFAULT, PUSH_PIC_OPEN, 0);
         if (openPushPic == 1) {
            HttpSendDeviceStatus(mActiveParam.join_uid, CAMERA_DEVICE, 1);//2：摄像头 1：开启
         }
         else {
            HttpSendDeviceStatus(mActiveParam.join_uid, CAMERA_DEVICE, 0);//2：摄像头 0：关闭
         }
      }

      if (mActiveParam.mbIsH5Live) {
         vlive::GetPaasSDKInstance()->UserPublishCallback(PushStreamEvent_Upper);
      }
      VhallRenderWdg *render = GetRenderWnd(mActiveParam.join_uid);
      if (render) {
         if (msgEvent->mbHasVideo) {
            render->SetEnableUpdateRendWnd(false);
            render->SetViewState(RenderView_None);
         }
         else {
            render->SetViewState(RenderView_NoCamera);
         }
         render->show();
         if (mDesktopCameraRenderWdgPtr && !mDesktopCameraRenderWdgPtr->isHidden()) {
            TRACE6("%s mDesktopCameraRenderWdgPtr play\n", __FUNCTION__);
            mDesktopCameraRenderWdgPtr->SetCloseCamera(false);
            mDesktopCameraRenderWdgPtr->StartRender();
         }
         else {
            TRACE6("%s win play\n", __FUNCTION__);
            std::shared_ptr<vhall::VideoRenderReceiveInterface> videoReciver = std::shared_ptr<vhall::VideoRenderReceiveInterface>(render->GetVideoReciver());
            GetWebRtcSDKInstance()->StartRenderLocalStream(VHStreamType_AVCapture, videoReciver);
            render->SetVhallIALiveSettingDlg(mpSettingDlg);
         }
         TRACE6("%s play\n", __FUNCTION__);
      }
      else {
         TRACE6("%s can not find uid:%s\n", __FUNCTION__, mActiveParam.join_uid.toStdWString().c_str());
      }
   }
   if (GetWebRtcSDKInstance()->IsWebRtcRoomConnected()) {
      TRACE6("%s PublisLocalStream\n", __FUNCTION__);
      slot_PublishLocalStreamTimeout();
   }
   TRACE6("%s end\n", __FUNCTION__);
}

void VhallIALive::RoomMixStreamState(bool success) {
   if (success) {
      TRACE6("%s success end\n", __FUNCTION__);
   }
   else {
      InitBroadCast(true);
   }
}

void VhallIALive::HandleSetLayout(QEvent *event) {

}


void VhallIALive::InitBroadCast(bool call_back) {
   if (call_back && mConfigBroadCast) {
      mConfigBroadCast->SetNeedConfigBroadCast(true);
      TRACE6("%s SetNeedConfigBroadCast\n", __FUNCTION__);
   }
   TRACE6("%s call_back %d  mConfigBroadCast->IsNeedConfigBroadCast() %d\n", __FUNCTION__, call_back, mConfigBroadCast->IsNeedConfigBroadCast());
   if (mConfigBroadCast && mConfigBroadCast->IsNeedConfigBroadCast()) {
      TRACE6("%s IsNeedConfigBroadCast\n", __FUNCTION__);
      QObject* recvObj = mpMainUILogic;
      LAYOUT_MODE layoutMode = LAYOUT_MODE_TILED;
      if (mpSettingDlg) {
         layoutMode = mpSettingDlg->GetLayOutMode();
      }
      mCurrentMode = GetCurrentLayoutMode(layoutMode);
      TRACE6("%s mCurrentMode %d\n", __FUNCTION__, mCurrentMode);
      BroadCastVideoProfileIndex broadProfileIndex = VhallIALiveSettingDlg::GetBroadCastVideoProfileIndex();
      if (call_back) {
         mConfigBroadCast->HandleBroadCast(mCurrentMode, broadProfileIndex, false);
      }
      else {
         mConfigBroadCast->InitBroadCast(mCurrentMode, broadProfileIndex, recvObj);
      }
   }
}

void VhallIALive::HandlePaasSetMainView(QEvent* event) {
   CustomOnHttpResMsgEvent* eventMsg = (CustomOnHttpResMsgEvent*)event;
   if (eventMsg) {
      if (eventMsg->mMsg.compare("success") != 0) {
         TRACE6("%s err\n", __FUNCTION__);
         mSetDeskTopMainViewTimer.start(2000);
         mSetDeskTopMainViewTimer.setSingleShot(true);
      }
      else {
         TRACE6("%s success\n", __FUNCTION__);
         mSetDeskTopMainViewTimer.stop();
         SetLayOutMode(CANVAS_LAYOUT_PATTERN_GRID_1);
      }
   }
}

void VhallIALive::HandlePushLocalStreamSuc(QEvent* event) {
   TRACE6("%s\n", __FUNCTION__);
   if (event == NULL) {
      return;
   }
   CustomRecvMsgEvent* eventMsg = (CustomRecvMsgEvent*)event;
   if (eventMsg) {
      TRACE6("%s eventMsg->msg:%s\n", __FUNCTION__, eventMsg->msg.toStdString().c_str());
      mLocalStreamID = eventMsg->msg;
      TRACE6("%s ok mLocalStreamID:%s\n", __FUNCTION__, mLocalStreamID.toStdString().c_str());
      if (mActiveParam.role_name.compare(USER_HOST) == 0 && mMediaFilePlayWdg == NULL && mDesktopWdg == NULL && mActiveParam.mainShow == mActiveParam.join_uid 
         && m_pScreenShareToolWgd && m_pScreenShareToolWgd->isHidden()) {
         QObject* recvObj = mpMainUILogic;
         TRACE6("%s SetMixLayoutMainView %s\n", __FUNCTION__, mLocalStreamID.toStdString().c_str());
         int bRet = GetWebRtcSDKInstance()->SetMixLayoutMainView(mLocalStreamID.toStdString(), [&, recvObj](const std::string& result, const std::string& msg, int errorCode)->void {
            if (errorCode == 20013) {
               //混流房间找不到该成员
               return;
            }
            //如果设置布局成功
            QString errMsg = QString::fromStdString(result);
            if (recvObj) {
               TRACE6("%s SetMixLayoutMainView %s\n", __FUNCTION__, errMsg.toStdString().c_str());
               QCoreApplication::postEvent(recvObj, new CustomRecvMsgEvent(CustomEvent_ConfigMainView, errMsg));
            }
         });
      }
      else if (mActiveParam.role_name.compare(USER_GUEST) == 0) {
         GetSpeakUserList();
      }
   }
}

void VhallIALive::HandleStreamRemoved(QEvent* event) {
   TRACE6("%s\n", __FUNCTION__);
   if (event == NULL) {
      return;
   }
   CustomRecvMsgEvent* eventMsg = (CustomRecvMsgEvent*)event;
   if (eventMsg)
   {
      TRACE6("%s user stream remove uid:%s eventMsg->mStreamType:%d\n", __FUNCTION__, eventMsg->msg.toStdString().c_str(), eventMsg->mStreamType);
      if (eventMsg->mStreamType <= VHStreamType_AVCapture) {
         VhallRenderWdg* render = GetRenderWnd(eventMsg->msg);
         if (render) {
            TRACE6("%s find Render \n", __FUNCTION__);
            render->SetEnableUpdateRendWnd(true);
            render->update();
         }
      }
      else if (eventMsg->mStreamType == VHStreamType_MediaFile) {
         TRACE6("%s remove VHStreamType_MediaFile\n", __FUNCTION__);
         if (mMediaFilePlayWdg && !mbIsPlayMediaFile) {
            if (mDocCefWidget) {
               mDocCefWidget->show();
            }
            ui.verticalLayout_mainWebView->removeWidget(mMediaFilePlayWdg);
            delete mMediaFilePlayWdg;
            mMediaFilePlayWdg = NULL;
         }
         if (mActiveParam.role_name == USER_HOST) {
            LAYOUT_MODE layoutMode = LAYOUT_MODE_TILED;
            if (mpSettingDlg) {
               layoutMode = mpSettingDlg->GetLayOutMode();
            }
            mCurrentMode = GetCurrentLayoutMode(layoutMode);
            SetLayOutMode(mCurrentMode);
         }
      }
      else if (eventMsg->mStreamType == VHStreamType_Desktop)
      {
         TRACE6("%s remove VHStreamType_Desktop\n", __FUNCTION__);
         if (NULL != mDesktopWdg) {
            if (mDocCefWidget) {
               mDocCefWidget->show();
            }
            ui.verticalLayout_mainWebView->removeWidget(mDesktopWdg);
            delete mDesktopWdg;
            mDesktopWdg = NULL;
         }
         if (mActiveParam.role_name == USER_HOST) {
            LAYOUT_MODE layoutMode = LAYOUT_MODE_TILED;
            if (mpSettingDlg) {
               layoutMode = mpSettingDlg->GetLayOutMode();
            }
            mCurrentMode = GetCurrentLayoutMode(layoutMode);
            SetLayOutMode(mCurrentMode);
         }
      }
   }
   TRACE6("%s end\n", __FUNCTION__);
   mCheckSpeakUserTimeOut.setSingleShot(true);
   mCheckSpeakUserTimeOut.start(CHECK_OFFLINE_TIMEOUT);
}

void VhallIALive::HandleStreamFailed(QEvent* event) {
   TRACE6("%s\n", __FUNCTION__);
   if (event == NULL) {
      return;
   }
   CustomRecvMsgEvent* eventMsg = (CustomRecvMsgEvent*)event;
   if (eventMsg) {
      TRACE6("%s uid:%s\n", __FUNCTION__, eventMsg->msg.toStdString().c_str());
   }
   TRACE6("%s uid:%s  mActiveParam.join_uid:%s\n", __FUNCTION__, eventMsg->msg.toStdString().c_str(), mActiveParam.join_uid.toStdString().c_str());
   if (eventMsg->mStreamType == vlive::VHStreamType_AVCapture) {
      TRACE6("%s repush camera\n", __FUNCTION__);
      mRePublishLocalStreamTimer.start(3000);
      mRePublishLocalStreamTimer.setSingleShot(true);
      FadeOutTip(CAMERA_PUSH_STREAM_ERR, TipsType_Error);
   }
   else if (eventMsg->mStreamType == vlive::VHStreamType_MediaFile) {
      TRACE6("%s repush mediafile\n", __FUNCTION__);
      mbIsPushMediaErr = true;
      mRePublishMediaFileStreamTimer.start(3000);
      mRePublishMediaFileStreamTimer.setSingleShot(true);
      FadeOutTip(FILE_PUSH_STREAM_ERR, TipsType_Error);
   }
   else if (eventMsg->mStreamType == vlive::VHStreamType_Desktop) {
      TRACE6("%s repush desktop\n", __FUNCTION__);
      mRePublishDesktopStreamTimer.start(3000);
      mRePublishDesktopStreamTimer.setSingleShot(true);
      FadeOutTip(DESKTOP_PUSH_STREAM_ERR, TipsType_Error);
   }
}

void VhallIALive::HandleSubScribSuc(QEvent *eventMsg) {
   TRACE6("%s start\n", __FUNCTION__);
   if (eventMsg == NULL) {
      return;
   }
   mActiveParam.bIsLiving = true;
   CustomRecvMsgEvent* event = (CustomRecvMsgEvent*)eventMsg;
   //如果有流，先判断是否有窗口，如果没有窗口不显示。
   if (event) {
      TRACE6("%s to find uid:%s event->mbHasVideo:%d  mActiveParam.mainShow:%s\n", __FUNCTION__, event->msg.toStdString().c_str(), event->mbHasVideo, mActiveParam.mainShow.toStdString().c_str());
      if (event->mStreamType <= VHStreamType_AVCapture) {
         if (event->msg != mActiveParam.mainShow) {
            //不是主画面的用户，订阅摄像头的小流
            TRACE6("%s ChangeSubScribeUserSimulCast VHSimulCastType_SmallStream\n", __FUNCTION__);
            if (mbIsOpenSubDoubleStream) {
               TRACE6("%s ChangeSubScribeUserSimulCast VHSimulCastType_SmallStream id:%s\n", __FUNCTION__, event->msg.toStdString().c_str());
               GetWebRtcSDKInstance()->ChangeSubScribeUserSimulCast(event->msg.toStdWString(), VHStreamType_AVCapture, VHSimulCastType_SmallStream);
            }
         }
         VhallAudienceUserInfo user_info;
         ParamRtcInfoToUserInfo(event->mUserData, user_info);
         VhallRenderWdg *render = GetRenderWnd(event->msg);
         if (render) {
            TRACE6("%s find render and play\n", __FUNCTION__);
            render->SetUserStreamInfo(event->mStreamId, VHStreamType_AVCapture);
            render->SetUserCameraState(event->mbHasVideo);
            PlayRemoteUser(render, event->msg, event->mbHasVideo);
            render->ShowVideoLoading(false);
            render->show();
         }
         //else if (ParamRtcInfoToUserInfo(event->mUserData, user_info)) {
         //   user_info.userId = event->msg.toStdWString();
         //   AppendRenderUser(QString::fromStdWString(user_info.userId), QString::fromStdWString(user_info.role), QString::fromStdWString(user_info.userName),true,true, event->msg != mActiveParam.mainShow,true,true);
         //   VhallRenderWdg *new_render = GetRenderWnd(event->msg);
         //   if (new_render) {
         //      new_render->SetViewState(RenderView_None);
         //   }
         //   std::shared_ptr<vhall::VideoRenderReceiveInterface> videoReciver = std::shared_ptr<vhall::VideoRenderReceiveInterface>(new_render->GetVideoReciver());
         //   GetWebRtcSDKInstance()->StartRenderRemoteStream(event->msg.toStdWString(), VHStreamType_AVCapture, videoReciver);
         //}
         else {
            TRACE6("%s cant not find render and start get speaker list\n", __FUNCTION__);
            //当接收到互动流时没有预先准备好的窗口，此时需要查询上麦列表。如果不是上麦用户再次把用户踢下麦。
            TRACE6("%s start to get speaker list:%s\n", __FUNCTION__, mSubScribeUserID.toStdString().c_str());
            mSubScribeUserID = event->msg;
            mCheckSubScribeStream.setSingleShot(true);
            mCheckSubScribeStream.start(5000);
         }
         if (event->msg == mActiveParam.mainShow && mDesktopWdg == nullptr && mMediaFilePlayWdg == nullptr) {
            ConfigRoomMainView(event->msg);
         }
      }
      else if (event->mStreamType == VHStreamType_Desktop) {
         mCurDeskTopStreamID = event->mStreamId;
         if (mDesktopWdg == NULL) {
            mDesktopWdg = new QWidget(this);
         }
         if (mDesktopWdg) {
            if (mDocCefWidget) {
               mDocCefWidget->hide();
               ui.widget_docLoadingTips->hide();
            }
            ui.verticalLayout_mainWebView->addWidget(mDesktopWdg);
            HWND wnd = (HWND)(mDesktopWdg->winId());
            if (IsWindow(wnd)) {
               TRACE6("%s play remote mDesktopWdg\n", __FUNCTION__);
               mDesktopWdg->setUpdatesEnabled(false);
               GetWebRtcSDKInstance()->StartRenderRemoteStream(event->msg.toStdWString(), VHStreamType_Desktop, wnd);
            }
            else {
               TRACE6("%s CustomEvent_RenderView mDesktopWdg\n", __FUNCTION__);
               QCoreApplication::postEvent(mpMainUILogic, new CustomRecvMsgEvent(CustomEvent_RenderView, event->msg, event->mbHasVideo));
            }
            HandleSetDesktopMainView(mCurDeskTopStreamID);
         }
         TRACE6("%s can not find :%s\n", __FUNCTION__, event->msg.toStdString().c_str());
      }
      else if (event->mStreamType == VHStreamType_MediaFile) {
         mCurPlayMediaFileStreamID = event->mStreamId;
         if (GetWebRtcSDKInstance()->GetCaptureStreamType(event->msg.toStdWString(), VHStreamType_MediaFile, CaptureStreamAVType_Video)) {
            if (mMediaFilePlayWdg == NULL) {
               mMediaFilePlayWdg = new QWidget(this);
            }
         }
         else {
            StopShowMediaVideo();
         }

         if (mActiveParam.bIsLiving) {
            slot_ClickedMic(mActiveParam.join_uid, true);
            if (mDocCefWidget && mMediaFilePlayWdg) {
               mDocCefWidget->hide();
               ui.widget_docLoadingTips->hide();
            }
         }
         else {
            if (mMediaFilePlayWdg) {
               mMediaFilePlayWdg->hide();
            }
         }
         if (mMediaFilePlayWdg) {
            ui.verticalLayout_mainWebView->addWidget(mMediaFilePlayWdg);
            HWND wnd = (HWND)(mMediaFilePlayWdg->winId());
            if (IsWindow(wnd)) {
               TRACE6("%s play remote mMediaFilePlayWdg\n", __FUNCTION__);
               mMediaFilePlayWdg->setUpdatesEnabled(false);
               GetWebRtcSDKInstance()->StartRenderRemoteStream(event->msg.toStdWString(), VHStreamType_MediaFile, wnd);
            }
            else {
               TRACE6("%s CustomEvent_RenderView mMediaFilePlayWdg\n", __FUNCTION__);
               QCoreApplication::postEvent(mpMainUILogic, new CustomRecvMsgEvent(CustomEvent_RenderView, event->msg, event->mbHasVideo));
            }
            HandleSetMediaFileMainView(mCurPlayMediaFileStreamID);
            FadeOutTip(PLAY_FILE_AND_CLOSE_MIC, TipsType_MediaFilePlay);
         }
      }
   }
}

void VhallIALive::PlayRemoteUser(VhallRenderWdg* render, QString user_id, bool hasVideo) {
   HWND wnd = render->GetRenderWndID();
   if (render->GetUserCameraState()) {
      render->SetViewState(RenderView_None);
   }
   else {
      render->SetViewState(RenderView_NoCamera);
   }
   render->show();
   if (IsWindow(wnd)) {
      TRACE6("%s play remote camera hasVideo:%d\n", __FUNCTION__, hasVideo);
      std::shared_ptr<vhall::VideoRenderReceiveInterface> videoReciver = std::shared_ptr<vhall::VideoRenderReceiveInterface>(render->GetVideoReciver());
      GetWebRtcSDKInstance()->StartRenderRemoteStream(user_id.toStdWString(), VHStreamType_AVCapture, videoReciver);
   }
   else {
      TRACE6("%s CustomEvent_RenderView\n", __FUNCTION__);
      QCoreApplication::postEvent(mpMainUILogic, new CustomRecvMsgEvent(CustomEvent_RenderView, user_id, hasVideo));
   }
   TRACE6("%s find:%s  to play\n", __FUNCTION__, user_id.toStdString().c_str());
}

QWidget* VhallIALive::GetRenderWidget() {
   return ui.widget_bottomTool;
}

void VhallIALive::InitVhallSDK() {
   std::list<vhall::AudioDevProperty> playerList;
   GetWebRtcSDKInstance()->GetPlayerDevices(playerList);
   std::list<vhall::AudioDevProperty>::iterator iter = playerList.begin();
   while (iter != playerList.end()) {
      VhallLiveDeviceInfo devInfo(QString::fromStdWString(iter->mDevGuid), QString::fromStdWString(iter->mDevName), iter->mIndex);
      mPlayerList.push_back(devInfo);
      iter++;
   }
   SelectPlayOutDev(mPlayerList);

   QJsonObject obj;
   obj["join_uid"] = mActiveParam.join_uid;
   obj["join_uname"] = mActiveParam.userNickName;
   obj["join_role"] = mActiveParam.role_name == USER_HOST ? QString("host") : QString("guest");
   obj["avatar"] = QString("http:") + mActiveParam.imageUrl;
   obj["nickName"] = mActiveParam.userNickName;
   obj["role"] = mActiveParam.role_name == USER_HOST ? QString("host") : QString("guest");
   if (mActiveParam.mbIsH5Live) {
     // 1:老师 2 : 学员 3 : 助教 4 : 嘉宾 5 : 监课
      obj["role"] = mActiveParam.role_name == USER_HOST ? QString("1") : QString("4");
      QJsonDocument document = QJsonDocument(obj);
      QByteArray array = document.toJson();
      std::string userData = array.toStdString();
      vlive::GetPaasSDKInstance()->JoinRtcRoom(userData);
   }
   else {
      GetWebRtcSDKInstance()->InitSDK(callbackReciver);
      vlive::WebRtcRoomOption option;
      option.nWebRtcSdkReConnetTime = 10;
      option.strLogUpLogUrl = QString("http:%1").arg(mActiveParam.reportUrl).toStdWString();
      option.strRoomId = mActiveParam.streamName.toStdWString();
      option.strRoomToken = mActiveParam.room_token.toStdWString();
      option.strThirdPushStreamUrl = mActiveParam.thirdPushStreamUrl.toStdWString();
      option.strUserId = mActiveParam.join_uid.toStdWString();
      option.strBusinesstype = L"0";
      QString toolConfPath = CPathManager::GetToolConfigPath();
      int numBu = ConfigSetting::ReadInt(toolConfPath, GROUP_DEFAULT, KEY_VHALL_BU, 0);
      option.strBusinesstype = QString::number(numBu).toStdWString();
      QJsonDocument document = QJsonDocument(obj);
      QByteArray array = document.toJson();
      std::string userData = array.toStdString();
      option.userData = userData;
      vlive::GetWebRtcSDKInstance()->ConnetWebRtcRoom(option);
      TRACE6("%s upLogUrl:%s\n", __FUNCTION__, option.strLogUpLogUrl.c_str());
      TRACE6("%s strRoomToken:%s\n", __FUNCTION__, option.strRoomToken.c_str());
      TRACE6("%s strUserId:%s\n", __FUNCTION__, option.strUserId.c_str());
   }
   ShowCheckDev();
   TRACE6("%s end\n", __FUNCTION__);
}

void VhallIALive::InitMainPlugin(const QString& url) {
   if (mDocCefWidget == nullptr) {
      if (mMediaFilePlayWdg || mDesktopWdg) {
         ui.widget_docLoadingTips->hide();
      }
      else {
         if (mDocLoadingTimer) {
            mDocLoadingTimer->start(700);
         }
         ui.widget_docLoadingTips->show();
      }
      mDocCefWidget = new LibCefViewWdg(this);
      mDocCefWidget->InitLibCef();
      mDocCefWidget->Create(url);
      ui.verticalLayout_mainWebView->addWidget(mDocCefWidget);
   }
}

void VhallIALive::slot_DocWebloadFinished(bool) {

}

void VhallIALive::CreateDocWebView() {
   InitMainPlugin(mActiveParam.pluginUrl);
}

void VhallIALive::HandleInitVhallSDK() {
   ui.widget_title->SetLivingState(mActiveParam.bIsLiving);
   if (mActiveParam.bIsLiving && mActiveParam.role_name == USER_HOST) {
      VH::CComPtr<ICommonData> pCommonData;
      DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
      ClientApiInitResp respInitData;
      pCommonData->GetInitApiResp(respInitData);
      TRACE6("%s SetStreamButtonStatus true\n", __FUNCTION__);
      ui.widget_title->SetStreamButtonStatus(true);
      ui.widget_title->StartWithActiveLive(respInitData.live_time);
      m_pScreenShareToolWgd->SetStreamButtonStatus(true);
      if (mActiveParam.role_name == USER_HOST) {
         mStartLiveDateTime.setTime_t(0);
         mStartLiveTimer.start(1000);
      }
      if (mpSettingDlg) {
         mpSettingDlg->SetStartLiving(true);
      }
      SetMainViewBackGroud(MAIN_VIEW_IS_LIVING);
      mActiveParam.bIsLiving = true;
      if (mbEnableStopStream && !mbIsHostUnPublished) {
         StartLiveWithOpenDevice();
      }
      else {
         SelectPlayOutDev(mPlayerList);
      }
   }
   else {
      SelectPlayOutDev(mPlayerList);
   }
}

void VhallIALive::StartLiveWithOpenDevice() {
   TRACE6("%s \n", __FUNCTION__);
   InitSelectDev();
   //如果登录的是主持人，并且需要显示在主窗体中
   bool showMain = mActiveParam.mainShow.compare(mActiveParam.join_uid) == 0 ? true : false;
   TRACE6("%s show in mainview  mActiveParam.mainShow:%s\n", __FUNCTION__, mActiveParam.mainShow.toStdString().c_str());
   AppendRenderUser(mActiveParam.join_uid, mActiveParam.role_name, mActiveParam.userNickName, mbIsMicOpen, mbIsCameraOpen, showMain, false);
   StartLocalStream();
   if (mpSettingDlg) {
      mpSettingDlg->SetStartLiving(true);
   }
}

void VhallIALive::CheckGuestUserManagerPermisson() {
   VH::CComPtr<ICommonData> pCommonData;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
   ClientApiInitResp respInitData;
   pCommonData->GetInitApiResp(respInitData);
   std::list<QString>::iterator iter = respInitData.permission_data.begin();

   int iOpType = 0;
   if (mActiveParam.role_name == USER_GUEST) {
      setMemberAuthority(false);
      for (; iter != respInitData.permission_data.end(); iter++) {
         QString perMissionData = (*iter);
         if (respInitData.player == H5_LIVE_CREATE_TYPE) {
            if (perMissionData.trimmed().toInt() == eWebOpType_MemberManager) {
               setMemberAuthority(true);
               break;
            }
         }
         else {
            if ("members_manager" == perMissionData) {
               setMemberAuthority(true);
               break;
            }
         }
      }
   }
}

void VhallIALive::InitSelectDev() {
   TRACE6("%s \n", __FUNCTION__);
   if (!mCameraList.isEmpty()) {
      if (mCurCameraDevID.isEmpty()) {
         mCurCameraDevID = mCameraList.at(0).devId;
         TRACE6("%s mCurCameraDevID= %s\n", __FUNCTION__, mCurCameraDevID.toStdString().c_str());
      }
   }
   if (!mMicList.isEmpty()) {
      if (mCurMicDevID.isEmpty()) {
         mCurMicDevID = mMicList.at(0).devId;
      }
   }
   if (!mPlayerList.isEmpty()) {
      if (mCurPlayerID.isEmpty()) {
         mCurPlayerID = mPlayerList.at(0).devId;
      }
   }
   TRACE6("%s end\n", __FUNCTION__);
}


void VhallIALive::StartLocalStream() {
   TRACE6("%s mbEnableStopStream %d\n", __FUNCTION__, mbEnableStopStream);
   if (mCameraList.isEmpty() && mMicList.isEmpty()) {
      TRACE6("%s there are no devices\n", __FUNCTION__, mbEnableStopStream);
      return;
   }

   if (!mCameraList.isEmpty()) {
      bool bFindDev = false;
      mCurCameraDevID = ConfigSetting::ReadString(CPathManager::GetConfigPath(), GROUP_DEFAULT, CAPTURE_CAMERA_ID, "");
      for (int i = 0; i < mCameraList.size(); i++) {
         if (mCurCameraDevID == mCameraList.at(i).devId) {
            bFindDev = true;
            mCurrentCameraIndex = mCameraList.at(i).devIndex;
            break;
         }
      }
      if (!bFindDev) {
         mCurCameraDevID = mCameraList.at(0).devId;
         ConfigSetting::writeValue(CPathManager::GetConfigPath(), GROUP_DEFAULT, CAPTURE_CAMERA_ID, mCurCameraDevID);
         TRACE6("%s mCurCameraDevID= %s\n", __FUNCTION__, mCurCameraDevID.toStdString().c_str());
      }
   }

   SelectMicDev(mMicList);
   SelectPlayOutDev(mPlayerList);
   StartLocalCapture();
   TRACE6("%s end\n", __FUNCTION__);
}

void VhallIALive::StartLocalCapture() {
   TRACE6("%s\n", __FUNCTION__);
   QString qsConfPath = CPathManager::GetConfigPath();
   int openDoubleStream = ConfigSetting::ReadInt(qsConfPath, GROUP_DEFAULT, PUSH_DOUBLE_STREAM, 1);
   VideoProfileIndex profile = VhallIALiveSettingDlg::GetPushStreamProfileByStreamType(VHStreamType_AVCapture);
   if (mActiveParam.role_name == USER_HOST) {
      ////主持人单流
      openDoubleStream = 0;
   }
   else {
      openDoubleStream = 1;//嘉宾双流
   }

   int openPushPic = ConfigSetting::ReadInt(qsConfPath, GROUP_DEFAULT, PUSH_PIC_OPEN, 0);
   if (openPushPic == 1) {
      QString filePath = ConfigSetting::ReadString(qsConfPath, GROUP_DEFAULT, PUSH_PIC_PATH, "");
      QString defaultPic = ConfigSetting::ReadString(CPathManager::GetToolConfigPath(), GROUP_DEFAULT, DEFAULT_PIC_NAME, VALUE_PIC_NAME);
      QFileInfo  fileInfo(filePath);
      if (filePath.isEmpty() || !fileInfo.exists()) {
         filePath = QApplication::applicationDirPath() + "\\" + defaultPic;
      }
      filePath = filePath.replace("/", "\\");
      TRACE6("%s filePath :%s\n", __FUNCTION__, filePath.toStdString().c_str());
      int nRet = GetWebRtcSDKInstance()->StartLocalCapturePicture(filePath.toStdString(), VhallIALiveSettingDlg::GetPushPicStreamProfile(profile), /*openDoubleStream == 1 ? true : */false);
      if (nRet != 0) {
         FadeOutTip(CAPTURE_IMAGE_NOT_EXIST, TipsType_Error);
      }
      QJsonObject body;
      SingletonMainUIIns::Instance().reportLog(STR_VH_LIVE_OPERATE, eLogRePortK_VHSDK_RTC_PIC_PUSH, L"rtc_pic", body);
   }
   else {
      TRACE6("%s mCurCameraDevID:%s  mCurMicDevID:%s  mCurPlayerID:%s\n", __FUNCTION__, mCurCameraDevID.toStdString().c_str(), mCurMicDevID.toStdString().c_str(), mCurPlayerID.toStdString().c_str());
      int nRet = GetWebRtcSDKInstance()->StartLocalCapture(mCurCameraDevID.toStdString(), profile, openDoubleStream == 1 && mCurCameraDevID.length() > 0 ? true : false);
      if (nRet == VhallLive_OK) {
         TRACE6("%s StartLocalCapture ok\n", __FUNCTION__);
      }
   }
   TRACE6("%s end\n", __FUNCTION__);
}

void VhallIALive::SelectMicDev(QList<VhallLiveDeviceInfo> &micDev) {
   QString DesktopCaptureId;
   int enableDeskTopCaptrue = ConfigSetting::ReadInt(CPathManager::GetConfigPath(), GROUP_DEFAULT, PUSH_DESKTOP_CAPTURE_OPEN, 0);
   if (enableDeskTopCaptrue == 1) {
      DesktopCaptureId = ConfigSetting::ReadString(CPathManager::GetConfigPath(), GROUP_DEFAULT, PUSH_DESKTOP_CAPTURE_DEVID, "");
   }
   if (!micDev.isEmpty()) {
      bool bFindDev = false;
      mCurMicDevID = ConfigSetting::ReadString(CPathManager::GetConfigPath(), GROUP_DEFAULT, CAPTURE_MIC_ID, "");
      for (int i = 0; i < micDev.size(); i++) {
         if (mCurMicDevID == micDev.at(i).devId) {
            bFindDev = true;
            mCurrentMicIndex = micDev.at(i).devIndex;
            break;
         }
      }
      if (!bFindDev) {
         mCurMicDevID = micDev.at(0).devId;
         ConfigSetting::writeValue(CPathManager::GetConfigPath(), GROUP_DEFAULT, CAPTURE_MIC_ID, mCurMicDevID);
      }
      TRACE6("%s mCurrentMicIndex:%d\n", __FUNCTION__, mCurrentMicIndex);
      GetWebRtcSDKInstance()->SetUsedMic(mCurrentMicIndex, mCurMicDevID.toStdString(), DesktopCaptureId.toStdWString());
      TRACE6("%s SetUsedMic end\n", __FUNCTION__);
      if (!mCurMicDevID.isEmpty()) {
         GetWebRtcSDKInstance()->SetCurrentMicVol(100);
         mMicVolume = 100;
         ChangeLocalMicDevState(true, mActiveParam.join_uid);
      }
      else {
         GetWebRtcSDKInstance()->SetCurrentMicVol(0);
         mMicVolume = 0;
         ChangeLocalMicDevState(false, mActiveParam.join_uid);
      }
   }
   else {
      TRACE6("%s mCurrentMicInde -1\n", __FUNCTION__);
      GetWebRtcSDKInstance()->SetUsedMic(-1, mCurMicDevID.toStdString(), DesktopCaptureId.toStdWString());
   }
   TRACE6("%s end\n", __FUNCTION__);
}

void VhallIALive::SelectPlayOutDev(QList<VhallLiveDeviceInfo> &playOutDev) {
   TRACE6("%s \n", __FUNCTION__);
   if (!playOutDev.isEmpty()) {
      bool bFindDev = false;
      mCurPlayerID = ConfigSetting::ReadString(CPathManager::GetConfigPath(), GROUP_DEFAULT, PLAY_OUT_DEV_ID, "");
      for (int i = 0; i < playOutDev.size(); i++) {
         if (mCurPlayerID == playOutDev.at(i).devId) {
            bFindDev = true;
            mCurrentPlayerIndex = playOutDev.at(i).devIndex;
            break;
         }
      }

      if (!bFindDev) {
         mCurPlayerID = playOutDev.at(0).devId;
         ConfigSetting::writeValue(CPathManager::GetConfigPath(), GROUP_DEFAULT, PLAY_OUT_DEV_ID, mCurPlayerID);
      }
      TRACE6("%s SetUsedPlay\n", __FUNCTION__);
      GetWebRtcSDKInstance()->SetUsedPlay(mCurrentPlayerIndex, mCurPlayerID.toStdString());
      if (!mCurPlayerID.isEmpty()) {
         mpSettingDlg->OnOpenPlayer(true);
         if (NULL != m_pScreenShareToolWgd) {
            m_pScreenShareToolWgd->SetPlayerState(true);
         }
         GetWebRtcSDKInstance()->SetCurrentPlayVol(100);
         mPlayerVolume = 100;
         mbIsPlayerOpen = true;
      }
      else {
         mpSettingDlg->OnOpenPlayer(false);
         if (NULL != m_pScreenShareToolWgd) {
            m_pScreenShareToolWgd->SetPlayerState(false);
         }
         GetWebRtcSDKInstance()->SetCurrentPlayVol(0);
         mPlayerVolume = 0;
         mbIsPlayerOpen = false;
      }
   }
   else {
      TRACE6("%s SetUsedPlay\n", __FUNCTION__);
      GetWebRtcSDKInstance()->SetUsedPlay(-1, mCurPlayerID.toStdString());
   }
   TRACE6("%s end\n", __FUNCTION__);
}

void VhallIALive::HandleDownLoadHead(QEvent* event) {
   if (ui.tabWidget->mpMemberListDlg) {
      ui.tabWidget->mpMemberListDlg->HandleDownLoadHead(event);
   }
}

void VhallIALive::HandleDeviceEvent(QEvent* event) {
   if (event == NULL) {
      return;
   }
   CustomDevChangedEvent* devEvent = (CustomDevChangedEvent*)event;
   if (devEvent) {
      TRACE6("%s select device mbIsGuestSpeaking:%d  mbIsHostStartLiving:%d mActiveParam.role_name:%s\n", __FUNCTION__, mbIsGuestSpeaking, mActiveParam.bIsLiving, mActiveParam.role_name.toStdString().c_str());
      mCurCameraDevID = devEvent->cameraDevId;
      mCurMicDevID = devEvent->micDevId;
      mCurPlayerID = devEvent->playerDevId;
      mCurrentMicIndex = devEvent->micIndex;
      mCurrentCameraIndex = devEvent->cameraIndex;
      mCurrentPlayerIndex = devEvent->playerIndex;
   }
}

QString VhallIALive::GetJoinId(){
   return mActiveParam.join_uid;
}

QString VhallIALive::GetJoinRole()
{
   return mActiveParam.role_name;
}

QWidget* VhallIALive::GetShareUIWidget() {
   return m_pScreenShareToolWgd->isHidden() ? (QWidget*)this : (QWidget*)m_pScreenShareToolWgd;
}

void VhallIALive::HandleStartPush(Event &e) {
   if (mActiveParam.role_name.compare(USER_HOST) != 0) {
      SetMainViewBackGroud(MAIN_VIEW_IS_LIVING);
      if (mActiveParam.hostNickName.isEmpty()) {
         if (ui.tabWidget->mpMemberListDlg) {
            mActiveParam.hostNickName = ui.tabWidget->mpMemberListDlg->GetUserNickName(mActiveParam.hostJoinUid);
         }
      }
      AppendRenderUser(mActiveParam.hostJoinUid, USER_HOST, mActiveParam.hostNickName, true, true, true, false);
   }

   mActiveParam.bIsLiving = true;
   if (ui.tabWidget->mpMemberListDlg) {
      ui.tabWidget->mpMemberListDlg->RecvUserListMsg(e);
   }
   if (mActiveParam.role_name == USER_GUEST) {
      InitLeftBtnEnableState(false);
   }
   if (mMediaFilePlayWdg) {
      if (mDocCefWidget) {
         mDocCefWidget->hide();
         ui.widget_docLoadingTips->hide();
      }
      mMediaFilePlayWdg->show();
      slot_ClickedMic(mActiveParam.join_uid, true);
   }
   ui.widget_title->SetLivingState(true);
}

void VhallIALive::HandleOverPush(Event &e) {
   TRACE6("%s enter\n", __FUNCTION__);
   ClearSpeakerUser();
   ResetMemberState();
   if (mActiveParam.role_name.compare(USER_HOST) != 0) {
      SetMainViewBackGroud(MAIN_VIEW_IS_END);
      HandleHostExitRoom();
   }
   if (ui.tabWidget->mpMemberListDlg) {
      ui.tabWidget->mpMemberListDlg->RecvUserListMsg(e);
      ui.tabWidget->mpMemberListDlg->SetEnableWheat(false);
   }
   if (!IsMainRenderNormalPos()) {
      slot_SwitchMainViewLayout();
   }
   mActiveParam.bIsLiving = false;
   mActiveParam.mainShow = mActiveParam.role_name == USER_HOST ? mActiveParam.join_uid : mActiveParam.hostJoinUid;
   mActiveParam.currPresenter = mActiveParam.mainShow;
   mActiveParam.mainShowRole = USER_HOST;
   if (mActiveParam.role_name == USER_GUEST) {
      InitLeftBtnEnableState(false);
   }
   QString confPath = CPathManager::GetConfigPath();
   QString localProfile = ConfigSetting::writeValue(confPath, GROUP_DEFAULT, KEY_LOCAL_VIDEO_PROFILE, PROFILE_INDEX_STANDER);
   if (mpSettingDlg) {
      mpSettingDlg->ResetProfile(1);
   }
   ui.widget_title->SetLivingState(false);
   TRACE6("%s leave mActiveParam.mainShow:%s \n", __FUNCTION__, mActiveParam.mainShow.toStdString().c_str());
}

void VhallIALive::HandleSwitchMainView(Event &e) {
   if (QString::fromStdWString(e.m_oUserInfo.userId).isEmpty()) {
      return;
   }
   if (QString::fromStdWString(e.m_oUserInfo.userId).compare(mActiveParam.join_uid) == 0 && mActiveParam.role_name.compare(USER_HOST) != 0) {
      FadeOutTip(GET_SPEAK_AUTH, TipsType_OnlyOne_Context);
   }
   else if (mActiveParam.role_name.compare(USER_HOST) != 0 && mActiveParam.mainShow.compare(mActiveParam.join_uid) == 0) {
      FadeOutTip(LOST_SPEAK_AUTH, TipsType_Normal);
   }

   if (mActiveParam.mainShow != mActiveParam.join_uid && mbIsOpenSubDoubleStream) {
      TRACE6("%s ChangeSubScribeUserSimulCast VHSimulCastType_SmallStream id:%s\n", __FUNCTION__, mActiveParam.mainShow.toStdString().c_str());
      GetWebRtcSDKInstance()->ChangeSubScribeUserSimulCast(mActiveParam.mainShow.toStdWString(), VHStreamType_AVCapture, VHSimulCastType_SmallStream);
   }
   mActiveParam.mainShow = QString::fromStdWString(e.m_oUserInfo.userId);
   if (e.m_oUserInfo.userId != mActiveParam.join_uid  && mbIsOpenSubDoubleStream) {
      TRACE6("%s ChangeSubScribeUserSimulCast VHSimulCastType_BigStream id:%s\n", __FUNCTION__, mActiveParam.mainShow.toStdString().c_str());
      GetWebRtcSDKInstance()->ChangeSubScribeUserSimulCast(mActiveParam.mainShow.toStdWString(), VHStreamType_AVCapture, VHSimulCastType_BigStream);
   }

   if (mActiveParam.mainShow.compare(mActiveParam.join_uid) != 0) {
      ResetPlayMediaFileAndDesktopShare();
   }

   if(e.m_eMsgType == e_RQ_setMainSpeaker){ //设置主讲人。
      mActiveParam.currPresenter = QString::fromStdWString(e.m_oUserInfo.userId);
   }

   TRACE6("%s mActiveParam.mainShow %s \n", __FUNCTION__, mActiveParam.mainShow.toStdString().c_str());
   mActiveParam.mainShowRole = QString::fromStdWString(e.m_oUserInfo.role);
   ChangeMainView(QString::fromStdWString(e.m_oUserInfo.userId));
   //ConfigRoomMainView(QString::fromStdWString(e.m_oUserInfo.userId));

   QString docShowType = "*setMainSpeaker";
   //如果主持人或者嘉宾设置成功主画面或主讲人，显示文档控制栏
   if (mActiveParam.role_name.compare(USER_HOST) == 0 || mActiveParam.role_name.compare(USER_GUEST) == 0) {
      docShowType = "*setMainSpeaker";
   }
   else {
      docShowType = "*setMainShow";		//如果观众设置成功主画面不显示文档控制栏
   }
   //如果主持人把主画面设置给了一个普通用户，文档权限在主持人处。
   if (QString::fromStdWString(e.m_oUserInfo.role) == USER_USER && mActiveParam.role_name.compare(USER_HOST) == 0) {
      docShowType = "*setMainSpeaker";
      QString qtCallJsMethod = GetDocSetMainViewJson(mActiveParam.join_uid, docShowType);
      if (!qtCallJsMethod.isEmpty() && mDocCefWidget) {
         TRACE6("%s  USER_USER\n", __FUNCTION__);
         mDocCefWidget->AppCallJsFunction(qtCallJsMethod);
      }
   }
   else {
      QString qtCallJsMethod = GetDocSetMainViewJson(QString::fromStdWString(e.m_oUserInfo.userId), docShowType);
      if (!qtCallJsMethod.isEmpty() && mDocCefWidget) {
         TRACE6("%s  uid:%s\n", __FUNCTION__, QString::fromStdWString(e.m_oUserInfo.userId).toStdString().c_str());
         mDocCefWidget->AppCallJsFunction(qtCallJsMethod);
      }
   }
   InitLeftBtnEnableState(QString::fromStdWString(e.m_oUserInfo.role) == USER_USER);
   if (ui.tabWidget->mpMemberListDlg &&QString::fromStdWString(e.m_oUserInfo.role) != USER_USER) {
      ui.tabWidget->mpMemberListDlg->RecvUserListMsg(e);
   }
}

void VhallIALive::InitLeftBtnEnableState(bool mainIsUser) {
   if (!mActiveParam.bIsLiving) {
      // 直播未开始时，主持人端：桌面共享、插播、互动工具不可点击（置灰），文档、白板、分享可点击
      if (mActiveParam.role_name.compare(USER_HOST) == 0) {
         ui.widget_leftTool->setEnableDesktopBtn(true);
         ui.widget_leftTool->setEnablePluginBtn(false);
         ui.widget_leftTool->setEnableInsertVedio(true);
         ui.widget_leftTool->setEnableDocAndWhiteBtn(true);
      }
      else if (mActiveParam.role_name.compare(USER_GUEST) == 0) {
         ui.widget_leftTool->setEnableDesktopBtn(false);
         ui.widget_leftTool->setEnablePluginBtn(false);
         ui.widget_leftTool->setEnableInsertVedio(false);
         ui.widget_leftTool->setEnableDocAndWhiteBtn(false);
      }
   }
   else {
      if (mActiveParam.mainShow == mActiveParam.join_uid) {
         ui.widget_leftTool->setEnableDesktopBtn(true);
         ui.widget_leftTool->setEnablePluginBtn(true);
         ui.widget_leftTool->setEnableInsertVedio(true);
         ui.widget_leftTool->setEnableDocAndWhiteBtn(true);
      }
      else {
         // 主持人端 - 非主讲人：白板 / 文档 / 桌面共享 / 插播文件，不可点击（置灰），分享可点击
         // 嘉宾端 - 非主讲人：文档、白板、桌面共享、插播文件，置灰不可点，分享可点击
         if (mActiveParam.role_name.compare(USER_HOST) == 0) {
            if (mainIsUser) {
               ui.widget_leftTool->setEnableDesktopBtn(true);
               ui.widget_leftTool->setEnablePluginBtn(true);
               ui.widget_leftTool->setEnableInsertVedio(true);
               ui.widget_leftTool->setEnableDocAndWhiteBtn(true);
            }
            else {
               ui.widget_leftTool->setEnableDesktopBtn(false);
               ui.widget_leftTool->setEnablePluginBtn(false);
               ui.widget_leftTool->setEnableInsertVedio(false);
               ui.widget_leftTool->setEnableDocAndWhiteBtn(false);
            }
         }
         else if (mActiveParam.role_name.compare(USER_GUEST) == 0) {
            ui.widget_leftTool->setEnableDesktopBtn(false);
            ui.widget_leftTool->setEnablePluginBtn(false);
            ui.widget_leftTool->setEnableInsertVedio(false);
            ui.widget_leftTool->setEnableDocAndWhiteBtn(false);
         }
      }
   }
}

void VhallIALive::ResetPlayMediaFileAndDesktopShare() {
   //如果我自己下麦了，这个时候还在进行插播，那么取消插播。
   if (mbIsPlayMediaFile) {
      TRACE6("%s MSG_MAINUI_VEDIOPLAY_STOPPLAY\n", __FUNCTION__);
      SingletonMainUIIns::Instance().PostCRMessage(MSG_MAINUI_VEDIOPLAY_STOPPLAY, NULL, 0);
      StopPlayMediaFile();
   }
   //桌面共享中
   if ((m_pScreenShareToolWgd && !m_pScreenShareToolWgd->isHidden()) || GetWebRtcSDKInstance()->IsCapturingStream(VHStreamType_Desktop)) {
      StopDeskTopStream();
      CloseScreenShare();
   }
}

void VhallIALive::HandleKickOutUser(Event &event) {
   if (ui.tabWidget->mpMemberListDlg) {
      ui.tabWidget->mpMemberListDlg->RecvUserListMsg(event);
   }

   VH::CComPtr<ICommonData> pCommonData;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
   ClientApiInitResp respInitData;
   pCommonData->GetInitApiResp(respInitData);
   if (QString::fromStdWString(event.m_oUserInfo.userId).compare(mActiveParam.join_uid) == 0) {
      HttpSendNotSpeak(mActiveParam.join_uid, false);
      ExitRoom(EXIT_KICKOUT);
   }
   else if (respInitData.role_name == USER_HOST) {
      HttpSendNotSpeak(QString::fromStdWString(event.m_oUserInfo.userId), false);
   }
}

void VhallIALive::HandleRecvAnnouncement(Event &e) {
   if (e.m_oUserInfo.userId == mActiveParam.join_uid) {
      return;
   }
   QString msg = e.context;
   QString pushTime = e.pushTime;
   ui.tabWidget->AppendAnnouncement(msg, pushTime);
}

void VhallIALive::HandleSocketIOConeect() {
   if (mActiveParam.role_name == USER_GUEST) {
      SetSpeakUserOffLine(mActiveParam.join_uid, false);
      RemoveCheckSpeakUser(mActiveParam.join_uid);
   }
   else if (mActiveParam.role_name == USER_HOST && mActiveParam.bIsLiving) {
      GetSpeakUserList();
   }
}

void VhallIALive::HandleSendNotice(QEvent* event) {
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

void VhallIALive::HandleAddSpeaker(Event &e) {
   TRACE6("%s  uid:%s\n", __FUNCTION__, QString::fromStdWString(e.m_oUserInfo.userId).toStdString().c_str());
   if (QString::fromStdWString(e.m_oUserInfo.userId).compare(mActiveParam.join_uid) != 0) {//不是当前用户，则订阅远端流
      bool bShowInMainView = false;
      if (mActiveParam.mainShow == e.m_oUserInfo.userId || QString::fromStdWString(e.m_oUserInfo.role) == USER_HOST) {
         bShowInMainView = true;
      }
      if (QString::fromStdWString(e.m_oUserInfo.role) == USER_HOST) {
         mActiveParam.hostJoinUid = QString::fromStdWString(e.m_oUserInfo.userId);
      }
      AppendRenderUser(QString::fromStdWString(e.m_oUserInfo.userId), QString::fromStdWString(e.m_oUserInfo.role), QString::fromStdWString(e.m_oUserInfo.userName), true, true, bShowInMainView, false);
      VhallRenderWdg* render = GetRenderWnd(QString::fromStdWString(e.m_oUserInfo.userId));
      if (render) {
         HWND wnd = render->GetRenderWndID();
         TRACE6("%s  StartRenderRemoteStream\n", __FUNCTION__);
         std::shared_ptr<vhall::VideoRenderReceiveInterface> videoReciver = std::shared_ptr<vhall::VideoRenderReceiveInterface>(render->GetVideoReciver());
         bool bret = GetWebRtcSDKInstance()->StartRenderRemoteStream(e.m_oUserInfo.userId, VHStreamType_AVCapture, videoReciver);
         if (bret) {
            render->SetViewState(RenderView_NoJoin);
         }
      }
   }

   if (ui.tabWidget->mpMemberListDlg) {
      ui.tabWidget->mpMemberListDlg->RecvUserListMsg(e);
   }
   ResetGridLayOutMode();
   TRACE6("%s  end\n", __FUNCTION__);
}

void VhallIALive::HandleAgreeSpeaker(Event &e) {
   TRACE6("%s  uid:%s\n", __FUNCTION__, QString::fromStdWString(e.m_oUserInfo.userId).toStdString().c_str());
   if (QString::fromStdWString(e.m_oUserInfo.userId).compare(mActiveParam.join_uid) == 0) {
      if (!GetWebRtcSDKInstance()->HasAudioDev() && !GetWebRtcSDKInstance()->HasVideoDev()) {
         FadeOutTip(CHECK_DEVICE_ERR, 0);
         return;
      }
      LocalUserToSpeak();
   }
   TRACE6("%s  end\n", __FUNCTION__);
}

void VhallIALive::HandleNoSpeaker(Event& e) {
   TRACE6("%s uid:%s  current_id:%s\n", __FUNCTION__, QString::fromStdWString(e.m_oUserInfo.userId).toStdString().c_str(), mActiveParam.join_uid.toStdString().c_str());
   //如果直播未开始，或者主持人点击了结束直播。那么嘉宾端收到下麦消息不做处理。
   if (!mActiveParam.bIsLiving && mActiveParam.role_name != USER_HOST) {
      TRACE6("%s end return ; mActiveParam.bIsLiving %d, mActiveParam.role_name %s\n", __FUNCTION__, mActiveParam.bIsLiving, mActiveParam.role_name.mid(0, 20).toStdString().c_str());
      return;
   }

   if (QString::fromStdWString(e.m_oUserInfo.userId).compare(mActiveParam.join_uid) != 0) {
      RemoveRemoteUser(QString::fromStdWString(e.m_oUserInfo.userId));
      //如果主持人已经下麦
      TRACE6("%s mbIsHostUnPublished:%d mActiveParam.mainShow:%s role:%s\n", __FUNCTION__, mbIsHostUnPublished,mActiveParam.mainShow.toStdString().c_str(), QString::fromStdWString(e.m_oUserInfo.role).toStdString().c_str());
      if (mActiveParam.role_name == USER_HOST && mbIsHostUnPublished) {
         //检测到嘉宾下麦，就要判断是否下麦嘉宾是当前主讲人. 如果是当前主讲人,那么主持人就要上麦然后自己成为主讲人
         if (QString::fromStdWString(e.m_oUserInfo.userId) == mActiveParam.mainShow || mActiveParam.join_uid == mActiveParam.mainShow) {
            mbIsHostSetGuestToSpeaker = false;
            TRACE6("%s mbIsHostSetGuestToSpeaker false\n", __FUNCTION__);
            CheckHostShowUpperInMemberList();
            LocalUserToSpeak();
         }
      }
   }
   else {
      TRACE6("%s user:%s  myself:%s\n", __FUNCTION__, QString::fromStdWString(e.m_oUserInfo.userId).toStdString().c_str(), mActiveParam.join_uid.toStdString().c_str());
      ui.widget_title->SetToSpeakBtnState(true);
      ResetToNospeakState();
      if (mActiveParam.role_name == USER_GUEST) {
         InitLeftBtnEnableState(false);
      }
      mbIsGuestSpeaking = false;
      TRACE6("%s mbIsGuestSpeaking = false\n", __FUNCTION__);
      RemoveRemoteUser(QString::fromStdWString(e.m_oUserInfo.userId));
      //如果我自己下麦了，这个时候还在进行插播，那么取消插播。
      ResetPlayMediaFileAndDesktopShare();
      if (mActiveParam.mbIsH5Live) {
         vlive::GetPaasSDKInstance()->UserPublishCallback(PushStreamEvent_Lower);
      }
      if (mActiveParam.role_name == USER_HOST) {
         mbIsHostSetGuestToSpeaker = true;
         CheckHostShowUpperInMemberList();
      }
      FadeOutTip(NO_SPEAK_NOTIC, TipsType_None);
   }

   if (ui.tabWidget->mpMemberListDlg) {
      ui.tabWidget->mpMemberListDlg->RecvUserListMsg(e);
   }
   ResetGridLayOutMode();
   TRACE6("%s mActiveParam.mainShow:%s  QString::fromStdWString(e.m_oUserInfo.m_szUserID):%s\n", __FUNCTION__, mActiveParam.mainShow.toStdString().c_str(), QString::fromStdWString(e.m_oUserInfo.userId).toStdString().c_str());
   if (mActiveParam.role_name == USER_HOST) {
      if (mActiveParam.mainShow == e.m_oUserInfo.userId) {
         TRACE6("%s  SetUserMainView\n", __FUNCTION__);
         SetUserMainView(mActiveParam.join_uid, QString::fromStdWString(e.m_oUserInfo.role), DISABLE_NOTICE_ERR_MSG);
      }
   }
   TRACE6("%s  end\n", __FUNCTION__);
}

void  VhallIALive::ResetGridLayOutMode() {
   TRACE6("%s  start\n", __FUNCTION__);
   if (mActiveParam.role_name == USER_HOST) {
      LAYOUT_MODE layoutMode = LAYOUT_MODE_TILED;
      if (mpSettingDlg) {
         layoutMode = mpSettingDlg->GetLayOutMode();
      }
      //只有选择了均分布局，当用户有上下麦的时候才设置布局。
      if (layoutMode == LAYOUT_MODE_GRID) {
         bool isDesktop = IsPlayDesktopShare();
         TRACE6("%s LAYOUT_MODE_GRID isDesktop %d\n", __FUNCTION__, isDesktop);
         if (mMediaFilePlayWdg != nullptr) {
            TRACE6("%s play mediafile\n", __FUNCTION__);
         }
         if (!isDesktop && mMediaFilePlayWdg == nullptr) {
            TRACE6("%s not media share\n", __FUNCTION__);
            mCurrentMode = GetCurrentLayoutMode(layoutMode);
            SetLayOutMode(mCurrentMode);
         }
      }
   }
}

void VhallIALive::HandleSwitchDeviceClose(Event &e) {
   TRACE6("%s  uid:%s e.meDevice:%d\n", __FUNCTION__, QString::fromStdWString(e.m_oUserInfo.userId).toStdString().c_str(), e.meDevice);
   if (e.meDevice == eDevice_Mic) {	//麦克风关闭
      VhallRenderWdg* render = GetRenderWnd(QString::fromStdWString(e.m_oUserInfo.userId));
      if (render) {
         render->SetUserMicState(false);
      }
      //需要判断当前状态是否已经关闭。如果处于打开的状态，才进行关闭。
      if (QString::fromStdWString(e.m_oUserInfo.userId).compare(mActiveParam.join_uid) == 0 && mbIsMicOpen) {
         ChangeLocalMicDevState(false, mActiveParam.join_uid);
      }
   }
   else if (e.meDevice == eDevice_Camera) {
      VhallRenderWdg* render = GetRenderWnd(QString::fromStdWString(e.m_oUserInfo.userId));
      if (render) {
         render->SetUserCameraState(false);
      }
      //需要判断当前状态是否已经关闭。如果处于打开的状态，才进行关闭。
      if (QString::fromStdWString(e.m_oUserInfo.userId).compare(mActiveParam.join_uid) == 0 && mbIsCameraOpen) {
         ChangeLocalCameraDevState(false, mActiveParam.join_uid);
      }
   }
   TRACE6("%s  end\n", __FUNCTION__);
}

void VhallIALive::HandleSwitchDeviceOpen(Event& e) {
   TRACE6("%s  uid:%s e.meDevice:%d \n", __FUNCTION__, QString::fromStdWString(e.m_oUserInfo.userId).toStdString().c_str(), e.meDevice);
   if (e.meDevice == eDevice_Mic) {	//麦克风打开
      VhallRenderWdg* render = GetRenderWnd(QString::fromStdWString(e.m_oUserInfo.userId));
      if (render) {
         render->SetUserMicState(true);
      }
      //需要判断当前状态是否已经关闭。如果处于"打开"的状态，才进行打开。
      if (QString::fromStdWString(e.m_oUserInfo.userId).compare(mActiveParam.join_uid) == 0 && !mbIsMicOpen) {
         ChangeLocalMicDevState(true, mActiveParam.join_uid);
      }
   }
   else if (e.meDevice == eDevice_Camera) {
      VhallRenderWdg* render = GetRenderWnd(QString::fromStdWString(e.m_oUserInfo.userId));
      if (render) {
         render->SetUserCameraState(true);
      }
      //需要判断当前状态是否已经关闭。如果处于"打开"的状态，才进行打开。
      if (QString::fromStdWString(e.m_oUserInfo.userId).compare(mActiveParam.join_uid) == 0 && !mbIsCameraOpen) {
         ChangeLocalCameraDevState(true, mActiveParam.join_uid);
      }
   }
   TRACE6("%s  end\n", __FUNCTION__);
}

//添加远端视频成员窗体
int VhallIALive::AppendRenderUser(const QString& uid, const QString &role, const QString& nickName, const bool audio, const bool video, bool showInMainView, bool isInitSpeakList, bool show) {
   TRACE6("%s enter uid:%s role:%s nickname %s isInitSpeakList:%d\n", __FUNCTION__, uid.toStdString().c_str(), role.toStdString().c_str(), nickName.toStdString().c_str(), isInitSpeakList);
   //如果已经存在了窗口，不再创建。
   if (IsExistRenderWnd(uid) || uid.isEmpty()) {
      return -1;
   }
   RemoveCheckSpeakUser(uid);
   TRACE6("%s showInMainView %d  uid:%s -- mainView:%s\n", __FUNCTION__, showInMainView, uid.toStdString().c_str(), mActiveParam.mainShow.toStdString().c_str());
   VhallRenderWdg *renderView = new VhallRenderWdg(this);
   if (renderView) {
      renderView->InitUserInfo(uid, nickName, role, mActiveParam.role_name, showInMainView, mActiveParam.join_uid);
      if (isInitSpeakList) {
         renderView->SetUserCameraState(video);
         if (video) {
            renderView->SetViewState(RenderView_NoJoin);
         }
      }
      else {
         if (video) {
            renderView->SetViewState(RenderView_NoJoin);
         }
         else {
            renderView->SetViewState(RenderView_NoCamera);
         }
      }
      renderView->SetUserMicState(audio);
      connect(renderView, &VhallRenderWdg::sig_setInMainView, this, &VhallIALive::slot_setInMainView);
      connect(renderView, &VhallRenderWdg::sig_setToSpeaker, this, &VhallIALive::slot_setToSpeaker);
      connect(renderView, &VhallRenderWdg::sig_ClickedCamera, this, &VhallIALive::slot_ClickedCamera);
      connect(renderView, &VhallRenderWdg::sig_ClickedMic, this, &VhallIALive::slot_ClickedMic);
      connect(renderView, &VhallRenderWdg::sig_NotToSpeak, this, &VhallIALive::slot_ClickedNotSpeak);
      connect(renderView, &VhallRenderWdg::sig_SwitchView, this, &VhallIALive::slot_SwitchMainViewLayout);

      if (showInMainView && ui.listWidgetMainView->count() == 0) {
         QListWidgetItem *renderViewItem = new QListWidgetItem(ui.listWidgetMainView);
         ui.listWidgetMainView->setItemWidget(renderViewItem, renderView);
         renderView->SetTeacherPos(true);
         ui.listWidgetMainView->addItem(renderViewItem);
         if (!IsMainRenderNormalPos()) {
            renderView->SwitchCenterSize(ui.widget_5->width(), ui.widget_5->height());
         }
         else {
            renderView->ResetViewSize(true);
         }

         AddRenderToMap(uid, renderView);
         ui.listWidgetMainView->show();
         TRACE6("%s add to mainRendView\n", __FUNCTION__);
      }
      else {
         renderView->ResetViewSize(false);
         ui.horizontalLayout_smallRender->addWidget(renderView);
         renderView->SetTeacherPos(false);
         AddRenderToMap(uid, renderView);
      }
      TRACE6("%s add user %s\n", __FUNCTION__, uid.toStdString().c_str());
      if (show) {
         renderView->show();
      }
      else {
         renderView->hide();
      }
      TRACE6("%s find uid:%s nickName:%s role:%s showInMainView:%d\n", __FUNCTION__, uid.toStdString().c_str(), nickName.toStdString().c_str(), role.toStdString().c_str(), showInMainView);
      if (role == USER_HOST && mActiveParam.role_name != USER_HOST) {
         mActiveParam.bIsLiving = true;
      }
   }
   TRACE6("%s  ReSizeMediaCtrlWdg\n", __FUNCTION__);
   ReSizeMediaCtrlWdg();
   ResetListWidgetSmallViewSize();
   ui.tabWidget->mpMemberListDlg->SetUpperStatesById(uid, true);
   //操作对象:嘉宾	已上麦	举手（实心红色）	下麦、设为主讲人、禁言、踢出
   if (mActiveParam.role_name == USER_HOST) {
      ui.tabWidget->mpMemberListDlg->ShowSetmainSpeaker(mActiveParam.mainShow);
   }
   repaint();
   TRACE6("%s  end\n", __FUNCTION__);
   return 0;
}

VhallRenderWdg *VhallIALive::GetRenderWnd(const QString& uid) {
   std::map<QString, VhallRenderWdg*>::iterator iter = mRenderMap.find(uid);
   if (iter != mRenderMap.end()) {
      return iter->second;
   }
   return NULL;
}

LayoutMode VhallIALive::GetGridUserSize() {
   LayoutMode layoutMode = CANVAS_LAYOUT_PATTERN_GRID_1;
   int userSize = mRenderMap.size();
   switch (userSize) {
   case 1:
      layoutMode = CANVAS_LAYOUT_PATTERN_GRID_1;
      break;
   case 2:
      layoutMode = CANVAS_LAYOUT_PATTERN_GRID_2_H;
      break;
   case 3:
      layoutMode = CANVAS_LAYOUT_PATTERN_GRID_3_E;
      break;
   case 4:
      layoutMode = CANVAS_LAYOUT_PATTERN_GRID_4_M;
      break;
   case 5:
      layoutMode = CANVAS_LAYOUT_PATTERN_GRID_5_D;
      break;
   case 6:
      layoutMode = CANVAS_LAYOUT_PATTERN_GRID_6_E;
      break;
   default:
      break;
   }
   return layoutMode;
}

bool VhallIALive::IsExistRenderWnd(const QString& uid) {
   std::map<QString, VhallRenderWdg*>::iterator iter = mRenderMap.find(uid);
   if (iter != mRenderMap.end()) {
      return true;
   }
   return false;
}

void VhallIALive::StopStream(const QString& uid) {
   if (mActiveParam.join_uid == uid) {
      GetWebRtcSDKInstance()->StopPushLocalStream();
   }
}

void VhallIALive::AddRenderToMap(const QString& id, VhallRenderWdg*render) {
   std::map<QString, VhallRenderWdg*>::iterator iter = mRenderMap.find(id);
   if (iter == mRenderMap.end()) {
      mRenderMap.insert(make_pair(id, render));
      TRACE6("%s add render id:%s \n", __FUNCTION__, id.toStdString().c_str());
   }
}

void VhallIALive::RemoveRendFromMap(const QString& id) {
   std::map<QString, VhallRenderWdg*>::iterator iter = mRenderMap.find(id);
   if (iter != mRenderMap.end()) {
      TRACE6("%s enter uid:%s\n", __FUNCTION__, id.toStdString().c_str());
      if (iter->second != NULL) {
         QString id = iter->second->GetUserID();
         TRACE6("%s delete enter id:%s\n", __FUNCTION__, id.toStdString().c_str());
         VhallRenderWdg* tmp = iter->second;
         disconnect(tmp, &VhallRenderWdg::sig_setInMainView, this, &VhallIALive::slot_setInMainView);
         disconnect(tmp, &VhallRenderWdg::sig_setToSpeaker, this, &VhallIALive::slot_setToSpeaker);
         disconnect(tmp, &VhallRenderWdg::sig_ClickedCamera, this, &VhallIALive::slot_ClickedCamera);
         disconnect(tmp, &VhallRenderWdg::sig_ClickedMic, this, &VhallIALive::slot_ClickedMic);
         disconnect(tmp, &VhallRenderWdg::sig_NotToSpeak, this, &VhallIALive::slot_ClickedNotSpeak);
         disconnect(tmp, &VhallRenderWdg::sig_SwitchView, this, &VhallIALive::slot_SwitchMainViewLayout);
         delete tmp;
         tmp = NULL;
         mRenderMap.erase(iter);
      }
      TRACE6("%s end \n", __FUNCTION__);
   }
}

//删除远端视频窗体
void VhallIALive::RemoveRemoteUser(const QString& uid) {
   TRACE6("%s start uid:%s \n", __FUNCTION__, uid.toStdString().c_str());
   bool bFindInSamllViewList = false;
   if (IsExistRenderWnd(uid)) {
      RemoveCheckSpeakUser(uid);
      //从小画面先删除渲染窗体。
      for (int i = 0; i < ui.horizontalLayout_smallRender->count(); i++) {
         QLayoutItem *item = ui.horizontalLayout_smallRender->itemAt(i);
         if (item && item->widget()) {
            VhallRenderWdg* itemRender = dynamic_cast<VhallRenderWdg*>(item->widget());
            if (itemRender && itemRender->GetUserID() == uid) {
               itemRender->hide();
               StopStream(uid);
               bFindInSamllViewList = true; //小窗中找到了要删除的对象，不再主画面中再去查找
               ui.horizontalLayout_smallRender->removeWidget(itemRender);
               RemoveRendFromMap(uid);
               break;
            }
         }
      }
      if (!bFindInSamllViewList) {
         bool bIsHostUserDel = false;
         for (int i = 0; i < ui.listWidgetMainView->count(); i++) {
            QListWidgetItem *item = ui.listWidgetMainView->item(i);
            if (item) {
               VhallRenderWdg* itemRender = dynamic_cast<VhallRenderWdg*>(ui.listWidgetMainView->itemWidget(item));
               if (itemRender && itemRender->GetUserID() == uid) {
                  TRACE6("%s  delete enter listWidgetMainView\n", __FUNCTION__);
                  itemRender->hide();
                  StopStream(uid);
                  bIsHostUserDel = itemRender->IsHostRole();
                  QListWidgetItem *deleteItem = ui.listWidgetMainView->takeItem(i);
                  RemoveRendFromMap(uid);
                  delete deleteItem;
                  break;
                  TRACE6("%s  delete end listWidgetMainView\n", __FUNCTION__);
               }
            }
         }
         TRACE6("%s bIsHostUserDel:%d \n", __FUNCTION__, bIsHostUserDel);
         //如果在主画面中删除的是嘉宾，则找到主持人之后移动到主画面
         if (!bIsHostUserDel) {
            for (int i = 0; i < ui.horizontalLayout_smallRender->count(); i++) {
               QLayoutItem *item = ui.horizontalLayout_smallRender->itemAt(i);
               if (item && item->widget()) {
                  VhallRenderWdg* oldHostRender = dynamic_cast<VhallRenderWdg*>(item->widget());
                  if (oldHostRender && (oldHostRender->IsHostRole() || (oldHostRender->GetUserID() == mActiveParam.join_uid && mActiveParam.role_name == USER_HOST))) {
                     TRACE6("%s bIsHostUserDel delete enter listWidgetMainView\n", __FUNCTION__);
                     oldHostRender->hide();
                     ui.horizontalLayout_smallRender->removeWidget(oldHostRender);
                     VhallRenderWdg* newHostRender = CopyNewRender(oldHostRender, true);

                     QListWidgetItem *renderViewItem = new QListWidgetItem(ui.listWidgetMainView);
                     ui.listWidgetMainView->setItemWidget(renderViewItem, newHostRender);
                     ui.listWidgetMainView->addItem(renderViewItem);  
                     if (!IsMainRenderNormalPos()) {
                        newHostRender->SwitchCenterSize(ui.widget_5->width(), ui.widget_5->height());
                     }
                     else {
                        newHostRender->ResetViewSize(true);
                     }
                     newHostRender->SetTeacherPos(true);
                     RemoveRendFromMap(newHostRender->GetUserID());
                     AddRenderToMap(newHostRender->GetUserID(), newHostRender);

                     //处理新的大画面
                     QString id = newHostRender->GetUserID();
                     if (id == mActiveParam.join_uid) {
                        HWND wnd = newHostRender->GetRenderWndID();
                        std::shared_ptr<vhall::VideoRenderReceiveInterface> videoReciver = std::shared_ptr<vhall::VideoRenderReceiveInterface>(newHostRender->GetVideoReciver());
                        GetWebRtcSDKInstance()->StartRenderLocalStream(VHStreamType_AVCapture, videoReciver);
                        newHostRender->SetVhallIALiveSettingDlg(mpSettingDlg);
                     }
                     else {
                        HWND wnd = newHostRender->GetRenderWndID();
                        std::shared_ptr<vhall::VideoRenderReceiveInterface> videoReciver = std::shared_ptr<vhall::VideoRenderReceiveInterface>(newHostRender->GetVideoReciver());
                        GetWebRtcSDKInstance()->StartRenderRemoteStream(id.toStdWString(), VHStreamType_AVCapture, videoReciver);
                     }
                     TRACE6("%s bIsHostUserDel delete end listWidgetMainView\n", __FUNCTION__);
                     break;
                  }
               }
            }
         }
      }
   }
   ReSizeMediaCtrlWdg();
   ui.tabWidget->mpMemberListDlg->SetUpperStatesById(uid, false);
   //操作对象:嘉宾	已上麦	举手（实心红色）	下麦、设为主讲人、禁言、踢出
   if (mActiveParam.role_name == USER_HOST) {
      ui.tabWidget->mpMemberListDlg->ShowSetmainSpeaker(mActiveParam.mainShow);
   }
   ResetListWidgetSmallViewSize();
   TRACE6("%s end find uid:%s \n", __FUNCTION__, uid.toStdString().c_str());
}

bool VhallIALive::IsMainRenderNormalPos() {
   if (ui.verticalLayout_mainRendView->indexOf(ui.listWidgetMainView) >= 0) {
      return true;
   }
   return false;
}

void VhallIALive::ChangeMainView(QString uid) {
   TRACE6("%s Enter uid:%s  mActiveParam.join_uid:%s\n", __FUNCTION__, uid.toStdString().c_str(), mActiveParam.join_uid.toStdString().c_str());
   //操作对象:嘉宾	已上麦	举手（实心红色）	下麦、设为主讲人、禁言、踢出
   if ((mActiveParam.role_name == USER_HOST || mActiveParam.role_name == USER_GUEST) && mActiveParam.mainShowRole != USER_USER) {
      ui.tabWidget->mpMemberListDlg->ShowSetmainSpeaker(mActiveParam.mainShow);
   }
   if (uid == mActiveParam.join_uid) {
      if (mpSettingDlg) {
         mpSettingDlg->ShowDesktopCaptrue(true);
      }
   }
   else {
      if (mpSettingDlg) {
         int enableDeskTopCaptrue = ConfigSetting::ReadInt(CPathManager::GetConfigPath(), GROUP_DEFAULT, PUSH_DESKTOP_CAPTURE_OPEN, 0);
         if (enableDeskTopCaptrue == 1) {
            int nRet = GetWebRtcSDKInstance()->StartLocalCapturePlayer(L"", 0);
            vlive::GetWebRtcSDKInstance()->MuteAllSubScribeAudio(false);
         }
         ConfigSetting::writeValue(CPathManager::GetConfigPath(), GROUP_DEFAULT, PUSH_DESKTOP_CAPTURE_OPEN, 0);
         mpSettingDlg->ShowDesktopCaptrue(false);
      }
   }
   for (int i = 0; i < ui.listWidgetMainView->count(); i++) {
      QListWidgetItem *lastMainItem = ui.listWidgetMainView->item(i);
      if (lastMainItem) {
         VhallRenderWdg* lastMainRender = dynamic_cast<VhallRenderWdg*>(ui.listWidgetMainView->itemWidget(lastMainItem));
         if (lastMainRender && lastMainRender->GetUserID() == uid) {
            TRACE6("%s leave uid:%s\n", __FUNCTION__, uid.toStdString().c_str());
            return;
         }
         else if (lastMainRender)
         {
            lastMainRender->hide();
            QListWidgetItem *deleteItem = ui.listWidgetMainView->takeItem(i);
            VhallRenderWdg* newSmallRender = CopyNewRender(lastMainRender, false);
            QString lastRoleName = lastMainRender->GetUserRole();

            RemoveRendFromMap(newSmallRender->GetUserID());
            delete deleteItem;
            deleteItem = NULL;
            if (newSmallRender) {
               ui.horizontalLayout_smallRender->addWidget(newSmallRender);
               newSmallRender->SetTeacherPos(false);
               ResetListWidgetSmallViewSize();
               AddRenderToMap(newSmallRender->GetUserID(), newSmallRender);

               QString id = newSmallRender->GetUserID();
               if (id == mActiveParam.join_uid) {
                  HWND wnd = newSmallRender->GetRenderWndID();
                  std::shared_ptr<vhall::VideoRenderReceiveInterface> videoReciver = std::shared_ptr<vhall::VideoRenderReceiveInterface>(newSmallRender->GetVideoReciver());
                  GetWebRtcSDKInstance()->StartRenderLocalStream(VHStreamType_AVCapture, videoReciver);
                  newSmallRender->SetVhallIALiveSettingDlg(mpSettingDlg);
                  TRACE6("%s newSmallRender uid:%s\n", __FUNCTION__, id.toStdString().c_str());
               }
               else {
                  HWND wnd = newSmallRender->GetRenderWndID();
                  std::shared_ptr<vhall::VideoRenderReceiveInterface> videoReciver = std::shared_ptr<vhall::VideoRenderReceiveInterface>(newSmallRender->GetVideoReciver());
                  GetWebRtcSDKInstance()->StartRenderRemoteStream(id.toStdWString(), VHStreamType_AVCapture, videoReciver);
                  TRACE6("%s newSmallRender uid:%s\n", __FUNCTION__, id.toStdString().c_str());
               }
               TRACE6("%s lastRoleName :%s\n", __FUNCTION__, lastRoleName.toStdString().c_str());
               if (mActiveParam.role_name.compare(USER_HOST) == 0) {
                  if (lastRoleName == USER_HOST) {		 			//当主持人主画面切换到小窗，显示设置主区域按键
                     newSmallRender->ShowSetMainViewBtn(false);
                     newSmallRender->ShowSetSpeakBtn(true);
                  }
                  else if (lastRoleName.compare(USER_GUEST) == 0) {	//当前主画面是嘉宾，画面切换到小窗，显示设置为主讲人。
                     newSmallRender->ShowSetSpeakBtn(true);
                     newSmallRender->ShowSetMainViewBtn(false);
                  }
                  else if (lastRoleName.compare(USER_USER) == 0) {	//当前主画面是观众，画面切换到小窗，显示设置为主画面。
                     newSmallRender->ShowSetSpeakBtn(false);
                     newSmallRender->ShowSetMainViewBtn(true);
                  }
               }
            }
            break;
         }
      }
   }

   //如果主画面不是要切换成主画面的用户才进行切换。
   for (int i = 0; i < ui.horizontalLayout_smallRender->count(); i++) {
      QLayoutItem *lastSmallItem = ui.horizontalLayout_smallRender->itemAt(i);
      if (lastSmallItem && lastSmallItem->widget()) {
         VhallRenderWdg* lastSmallRender = dynamic_cast<VhallRenderWdg*>(lastSmallItem->widget());
         if (lastSmallRender) {
            QString id = lastSmallRender->GetUserID();
            //找到了要切换的用户
            if (id.compare(uid) == 0) {
               TRACE6("%s find uid:%s id:%s\n", __FUNCTION__, uid.toStdString().c_str(), id.toStdString().c_str());
               //把小窗的移除并设置大小
               lastSmallRender->hide();
               //移除
               ui.horizontalLayout_smallRender->removeWidget(lastSmallRender);
               //大小窗互换。
               VhallRenderWdg* newMainViewRender = CopyNewRender(lastSmallRender, true);
               RemoveRendFromMap(newMainViewRender->GetUserID());


               QListWidgetItem *newMainViewItem = new QListWidgetItem(ui.listWidgetMainView);
               newMainViewRender->ResetViewSize(true);
               ui.listWidgetMainView->setItemWidget(newMainViewItem, newMainViewRender);
               ui.listWidgetMainView->addItem(newMainViewItem);
               if (!IsMainRenderNormalPos()) {
                  newMainViewRender->SwitchCenterSize(ui.widget_5->width(), ui.widget_5->height());
               }
               else {
                  newMainViewRender->ResetViewSize(true);
               }
               newMainViewRender->SetTeacherPos(true);
               AddRenderToMap(newMainViewRender->GetUserID(), newMainViewRender);
               newMainViewRender->show();
               QString uid = newMainViewRender->GetUserID();
               ////处理新的小画面
               if (id == mActiveParam.join_uid) {
                  HWND wnd = newMainViewRender->GetRenderWndID();
                  std::shared_ptr<vhall::VideoRenderReceiveInterface> videoReciver = std::shared_ptr<vhall::VideoRenderReceiveInterface>(newMainViewRender->GetVideoReciver());
                  GetWebRtcSDKInstance()->StartRenderLocalStream(VHStreamType_AVCapture, videoReciver);
                  newMainViewRender->SetVhallIALiveSettingDlg(mpSettingDlg);
               }
               else {
                  HWND wnd = newMainViewRender->GetRenderWndID();
                  std::shared_ptr<vhall::VideoRenderReceiveInterface> videoReciver = std::shared_ptr<vhall::VideoRenderReceiveInterface>(newMainViewRender->GetVideoReciver());
                  GetWebRtcSDKInstance()->StartRenderRemoteStream(id.toStdWString(), VHStreamType_AVCapture, videoReciver);
               }

               TRACE6("%s mActiveParam.role_name:%s\n", __FUNCTION__, mActiveParam.role_name.toStdString().c_str());
               if (mActiveParam.role_name.compare(USER_HOST) == 0) {
                  if (newMainViewRender) {		 	////移动到大窗的用户是主持人自己不显示：下麦/设置主讲人/设置主区域
                     newMainViewRender->ShowSetMainViewBtn(false);
                     newMainViewRender->ShowSetSpeakBtn(false);
                  }
                  //大窗显示嘉宾，成员列表主持人显示上麦
                  if (newMainViewRender->GetUserRole() == USER_GUEST) {
                     mbIsHostSetGuestToSpeaker = true;
                     TRACE6("%s mbIsHostSetGuestToSpeaker true\n", __FUNCTION__);
                     CheckHostShowUpperInMemberList();
                  }
                  //大窗显示其它角色时，成员列表主持人不显示上下麦。
                  else {
                     mbIsHostSetGuestToSpeaker = false;
                     TRACE6("%s mbIsHostSetGuestToSpeaker false\n", __FUNCTION__);
                     CheckHostShowUpperInMemberList();
                  }
               }
               ResetListWidgetSmallViewSize();
               return;
            }
         }
      }
   }
}

VhallRenderWdg* VhallIALive::CopyNewRender(VhallRenderWdg* render, bool bIsMainView) {
   TRACE6("%s bIsMainView:%d\n", __FUNCTION__, bIsMainView);
   VhallRenderWdg *renderView = new VhallRenderWdg(this);
   if (renderView) {
      renderView->InitUserInfo(render->GetUserID(), render->GetNickName(), render->GetUserRole(), mActiveParam.role_name, bIsMainView, mActiveParam.join_uid);
      renderView->SetUserCameraState(render->GetUserCameraState());
      renderView->SetUserMicState(render->GetUserMicState());
      renderView->SetUserStreamInfo(render->GetStreamId(), VHStreamType_AVCapture);
      if (renderView->GetUserCameraState()) {
         renderView->SetViewState(RenderView_None);
      }
      else {
         renderView->SetViewState(RenderView_NoCamera);
      }
      connect(renderView, &VhallRenderWdg::sig_setInMainView, this, &VhallIALive::slot_setInMainView);
      connect(renderView, &VhallRenderWdg::sig_setToSpeaker, this, &VhallIALive::slot_setToSpeaker);
      connect(renderView, &VhallRenderWdg::sig_ClickedCamera, this, &VhallIALive::slot_ClickedCamera);
      connect(renderView, &VhallRenderWdg::sig_ClickedMic, this, &VhallIALive::slot_ClickedMic);
      connect(renderView, &VhallRenderWdg::sig_NotToSpeak, this, &VhallIALive::slot_ClickedNotSpeak);
      connect(renderView, &VhallRenderWdg::sig_SwitchView, this, &VhallIALive::slot_SwitchMainViewLayout);
      renderView->ResetViewSize(bIsMainView);
   }
   return renderView;
}

void VhallIALive::slot_ClickedCamera(QString uid, bool curState) {
   TRACE6("%s uid:%s  curState :%d\n", __FUNCTION__, uid.toStdString().c_str(), curState);
   TRACE6("%s mActiveParam.join_uid:%s  mCurCameraDevID :%s\n", __FUNCTION__, mActiveParam.join_uid.toStdString().c_str(),
      mCurCameraDevID.toStdString().c_str());
   if (uid.compare(mActiveParam.join_uid) == 0 && mCurCameraDevID.isEmpty()) {
      TRACE6("%s mActiveParam.join_uid:%s  mCurCameraDevID :%s\n", __FUNCTION__, mActiveParam.join_uid.toStdString().c_str(),mCurCameraDevID.toStdString().c_str());
      return;
   }
   QString confPath = CPathManager::GetConfigPath();
   QString domain = ConfigSetting::ReadString(CPathManager::GetToolConfigPath(), GROUP_DEFAULT, KEY_URL_DOMAIN, domainUrl);
   int type = curState == true ? 0 : 1;
   QJsonObject body;
   if (curState) {
      body["vb"] = "Close";
   }
   else {
      body["vb"] = "Open";
   }

   HttpSendDeviceStatus(uid, CAMERA_DEVICE, type);
   VhallRenderWdg* render = GetRenderWnd(uid);
   if (render) {
      render->SetUserCameraState(!curState);
   }
   if (uid.compare(mActiveParam.join_uid) == 0) {
      ChangeLocalCameraDevState(!curState, mActiveParam.join_uid);
   }
   TRACE6("%s ok\n", __FUNCTION__);
}

void VhallIALive::slot_SwitchMainViewLayout() {
   VhallRenderWdg* render = nullptr;
   if (ui.listWidgetMainView->count() > 0) {
      QListWidgetItem *deleteItem = ui.listWidgetMainView->item(0);
      render = (VhallRenderWdg*)ui.listWidgetMainView->itemWidget(deleteItem);

   }
   if (ui.verticalLayout_mainRendView->indexOf(ui.stackedWidget) >= 0) {
      ui.verticalLayout_mainRendView->addWidget(ui.listWidgetMainView );
      ui.horizontalLayout_2->addWidget(ui.stackedWidget);
      if (mDocCefWidget) {
         QString funPage = QString("QtCallJsChangeDocTool(%1)").arg(eDocToolCtrlType_OpenTool);
         mDocCefWidget->AppCallJsFunction(funPage);
      }
      if (render) {
         render->ResetViewSize(true);
      }
   }
   else {
      //将渲染窗移动到中间布局。
      ui.verticalLayout_mainRendView->addWidget(ui.stackedWidget);
      ui.horizontalLayout_2->addWidget(ui.listWidgetMainView);
      if (mDocCefWidget) {
         QString funPage = QString("QtCallJsChangeDocTool(%1)").arg(eDocToolCtrlType_CloseTool);
         mDocCefWidget->AppCallJsFunction(funPage);
      }
      if (render) {
         render->SwitchCenterSize(ui.widget_5->width(),ui.widget_5->height());
      }
   }
}

void VhallIALive::slot_ClickedNotSpeak(QString uid) {
   TRACE6("%s uid: %s\n", __FUNCTION__, uid.toStdString().c_str());
   //主持人点击自己下麦
   if (mActiveParam.role_name == USER_HOST && uid.compare(mActiveParam.join_uid) == 0) {
      HttpSendNotSpeak(uid, true);
   }
   //其它角色
   else if (uid.compare(mActiveParam.join_uid) == 0) {
      slot_OnToSpeakClicked();
   }
   else {
      HttpSendNotSpeak(uid, true);
   }
}

//邀请上麦  / 下麦
void VhallIALive::slot_Upper(const QString& strId, const bool& bOperation /*= true*/){
   if (bOperation){//邀请上麦
      if (ui.tabWidget->mpMemberListDlg && ui.tabWidget->mpMemberListDlg->IsUserHandsUp(strId)) {
         slotAgreeUpper(strId);
      }                                 
      else{
         HttpSendInvite(strId, true);
      }
   }
   else{//下麦
      HttpSendNotSpeak(strId, true);
   }
}

void VhallIALive::slotMemberClose(){
   if (nullptr != ui.tabWidget)
   {
      ui.tabWidget->setCurrentIndex(0);
      ui.tabWidget->GetBar()->setCurrentIndex(0);
   }
}

void VhallIALive::slot_HostPublish(const bool& publish) {
   TRACE6("%s\n", __FUNCTION__);
   //publish 为 true时 显示为上麦，此时主持人点击之后进行上麦操作
   if (publish) {
      LocalUserToSpeak();
   }
   else {
      slot_ClickedNotSpeak(mActiveParam.join_uid);
   }
}

void VhallIALive::slot_ClickedMic(QString uid, bool curState) {
   TRACE6("%s uid %s curState:%d\n", __FUNCTION__, uid.toStdString().c_str(), curState);
   if (uid.compare(mActiveParam.join_uid) == 0 && mCurMicDevID.isEmpty()) {
      return;
   }
   int type = curState == true ? 0 : 1;	//0关闭，1打开
   HttpSendDeviceStatus(uid, MIC_DEVICE, type);
   VhallRenderWdg* render = GetRenderWnd(uid);
   if (render) {
      render->SetUserMicState(!curState);
   }
   if (uid.compare(mActiveParam.join_uid) == 0) {
      ChangeLocalMicDevState(!curState, mActiveParam.join_uid);
   }
}

//设置主讲人,同时显示在主窗体
void VhallIALive::slot_setToSpeaker(QString uid, QString role) {
   if (GetWebRtcSDKInstance()->IsPushingStream(vlive::VHStreamType_MediaFile)) {
      AlertTipsDlg tip(CHANGE_MAIN_SPEAK_STOP_PLAY, true, this);
      tip.CenterWindow(this);
      tip.SetYesBtnText(CONTINUE);
      tip.SetNoBtnText(CANCEL);
      if (tip.exec() != QDialog::Accepted) {
         return;
      }
   }

   if (/*mMediaStream == NULL && mDesktopStream == NULL && */(mDesktopWdg != NULL || mMediaFilePlayWdg != NULL)) {
      AlertTipsDlg tip(CHANGE_MAIN_SPEAK_STOP_PLAY, true, this);
      tip.CenterWindow(this);
      tip.SetYesBtnText(CONTINUE);
      tip.SetNoBtnText(CANCEL);
      if (tip.exec() != QDialog::Accepted) {
         return;
      }
   }
   HttpSendSwitchSpeaker(uid, role, ENABLE_NOTICE_ERR_MSG);

}

void VhallIALive::SetUserMainView(const QString& uid, QString role, int enableNotice) {
   HttpSendSwitchSpeaker(uid, role, enableNotice);
}

//主持人切换主画面
void VhallIALive::slot_setInMainView(QString uid, QString userRole) {
   if (GetWebRtcSDKInstance()->IsCapturingStream(VHStreamType_Desktop) || GetWebRtcSDKInstance()->IsCapturingStream(VHStreamType_MediaFile)
      || mDesktopWdg != NULL || mMediaFilePlayWdg != NULL){
      AlertTipsDlg tip(CHANGE_MAIN_SPEAK_STOP_PLAY, true, this);
      tip.CenterWindow(this);
      tip.SetYesBtnText(CONTINUE);
      tip.SetNoBtnText(CANCEL);
      if (tip.exec() != QDialog::Accepted) {
         return;
      }
   }
   SetUserMainView(uid, userRole, ENABLE_NOTICE_ERR_MSG);
}

//设置用户麦克风状态。
void  VhallIALive::SetUserMicState(const QString& uid, bool open) {
   TRACE6("%s enter \n", __FUNCTION__);
   std::map<QString, VhallRenderWdg*>::iterator iter = mRenderMap.find(uid);
   if (iter != mRenderMap.end()) {
      TRACE6("%s enter uid:%s\n", __FUNCTION__, uid.toStdString().c_str());
      if (iter->second != NULL) {
         iter->second->SetUserMicState(open);
      }
   }
   TRACE6("%s leave \n", __FUNCTION__);
}

void  VhallIALive::SetUserCameraState(const QString& uid, bool open) {
   TRACE6("%s enter \n", __FUNCTION__);
   std::map<QString, VhallRenderWdg*>::iterator iter = mRenderMap.find(uid);
   if (iter != mRenderMap.end()) {
      TRACE6("%s enter uid:%s\n", __FUNCTION__, uid.toStdString().c_str());
      if (iter->second != NULL) {
         iter->second->SetUserCameraState(open);
      }
   }
   TRACE6("%s leave \n", __FUNCTION__);
}

void VhallIALive::PreviewCameraCallback(bool success, int code) {
   if (mDeviceTestingWdg && mbIsDeviceChecking) {
      mDeviceTestingWdg->HandlePreviewCamera(success, code);
   }
   if (mpSettingDlg) {
      mpSettingDlg->PreviewCameraCallback(success);
   }
}

void VhallIALive::CloseFloatChatWdg() {
   ui.tabWidget->slotCloseShareDlg();
}

void VhallIALive::StartDeskTopStream(int index) {
   TRACE6("%s index %d\n",__FUNCTION__, index);
   VideoProfileIndex profile = VhallIALiveSettingDlg::GetPushStreamProfileByStreamType(VHStreamType_Desktop);
   int nRet = GetWebRtcSDKInstance()->StartDesktopCapture(index, profile);
   if (nRet == vlive::VhallLive_OK) {
      TRACE6("%s nRet %d\n", __FUNCTION__, nRet);
      mbEnableStopDesktopStream = false;
   }
}

void VhallIALive::StopDeskTopStream()
{
   InteractAPIManager apiManager(mpMainUILogic);
   apiManager.HttpSendChangeWebWatchLayout(0);
   GetWebRtcSDKInstance()->StopDesktopCapture();
   LAYOUT_MODE layoutMode = LAYOUT_MODE_TILED;
   if (mpSettingDlg) {
      layoutMode = mpSettingDlg->GetLayOutMode();
   }
   mCurrentMode = GetCurrentLayoutMode(layoutMode);
   if (mActiveParam.role_name == USER_HOST) {
      SetLayOutMode(mCurrentMode);
   }
}

void VhallIALive::StopPublishDeskTopStream(){
   GetWebRtcSDKInstance()->StopPushDesktopStream();
}

void VhallIALive::createMemberListDlg(){
   if (ui.tabWidget->mpMemberListDlg) {
      ui.tabWidget->mpMemberListDlg->SetAlive(this);
      connect(ui.tabWidget->mpMemberListDlg, &MemberListWdg::signal_Refresh, this, &VhallIALive::signal_MemberListRefresh);
      connect(ui.tabWidget->mpMemberListDlg, &MemberListWdg::sigAllowRaiseHands, this, &VhallIALive::slot_AllowRaiseHands);
      connect(ui.tabWidget->mpMemberListDlg, &MemberListWdg::signal_Upper, this, &VhallIALive::slot_Upper);
      connect(ui.tabWidget->mpMemberListDlg, &MemberListWdg::sig_OperationUser, this, &VhallIALive::sig_OperationUser);
      connect(ui.tabWidget->mpMemberListDlg, &MemberListWdg::sig_AgreeUpper, this, &VhallIALive::slotAgreeUpper);
      connect(ui.tabWidget->mpMemberListDlg, &MemberListWdg::sig_ReqApointPageUserList, this, &VhallIALive::sig_ReqApointPageUserList);
      connect(ui.tabWidget->mpMemberListDlg, &MemberListWdg::sig_selfUpper, this, &VhallIALive::slot_HostPublish);
      connect(ui.tabWidget->mpMemberListDlg, &MemberListWdg::sig_Hide, this, &VhallIALive::slotMemberClose);
      connect(ui.tabWidget->mpMemberListDlg, &MemberListWdg::sig_setMainSpeaker, this, &VhallIALive::slot_setToSpeaker);
      if (mActiveParam.role_name.compare(USER_HOST) == 0)//主持
         ui.tabWidget->mpMemberListDlg->SetHansUpShow(true);
      else//非主持
         ui.tabWidget->mpMemberListDlg->SetHansUpShow(false);
   }
}

void VhallIALive::slot_changed(bool bShowMem){
   if (bShowMem){
      slot_MemberList();
   }
   else{
      if (ui.tabWidget->mpMemberListDlg && ui.tabWidget->mpMemberListDlg->isVisible()){
         ui.tabWidget->mpMemberListDlg->hide();
      }
   }

}

void VhallIALive::slot_MemberList(){
   TRACE6("%s\n", __FUNCTION__);
   if (NULL != ui.tabWidget->mpMemberListDlg){
      if (ui.tabWidget->mpMemberListDlg->isVisible()){
         ui.tabWidget->ShowMemberFromDesktop(false,QPoint());
      } else {
         QPoint pos;
         if (m_pScreenShareToolWgd && !m_pScreenShareToolWgd->isHidden()) { 
            QRect rect = QApplication::desktop()->availableGeometry(m_pScreenShareToolWgd);
            int ix = rect.x() + (rect.width() - ui.tabWidget->width()) / 2;
            int iy = rect.y() + (rect.height() - ui.tabWidget->height()) / 2;
            pos.setX(ix);
            pos.setY(iy);
         }
         else {
            QRect rect = QApplication::desktop()->availableGeometry(this);
            int ix = rect.x() + (rect.width() - ui.tabWidget->width()) / 2;
            int iy = rect.y() + (rect.height() - ui.tabWidget->height()) / 2;
            pos.setX(ix);
            pos.setY(iy);
         }
         ui.tabWidget->ShowMemberFromDesktop(true, pos);
         ui.widget_leftTool->SetEixtUnseeHandsUpList(false);
      }
      TRACE6("%s ui.tabWidget->mpMemberListDlg SHOW\n", __FUNCTION__);
   }
   else{
      TRACE6("%s ui.tabWidget->mpMemberListDlg is NULL\n", __FUNCTION__);
   }
}

void VhallIALive::slot_closeDesktopCameraRender() {
   VhallRenderWdg *render = GetRenderWnd(mActiveParam.join_uid);
   if (render) {
      HWND wnd = render->GetRenderWndID();
      if (IsWindow(wnd)) {
         std::shared_ptr<vhall::VideoRenderReceiveInterface> videoReciver = std::shared_ptr<vhall::VideoRenderReceiveInterface>(render->GetVideoReciver());
         GetWebRtcSDKInstance()->StartRenderLocalStream(VHStreamType_AVCapture, videoReciver);
         render->SetVhallIALiveSettingDlg(mpSettingDlg);
      }
   }
}

void VhallIALive::slot_LiveToolFromDesktopClicked() {
   VH::CComPtr<ICommonData> pCommonData;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
   ClientApiInitResp respInitData;
   pCommonData->GetInitApiResp(respInitData);
   if (respInitData.player == FLASH_LIVE_CREATE_TYPE) {
      STRU_MAINUI_CLICK_CONTROL loControl;
      loControl.m_eType = control_LiveTool;
      loControl.m_bIsReLoad = mbIsFirstLoad ? true : false;
      mbIsFirstLoad = false;
      SingletonMainUIIns::Instance().PostCRMessage(MSG_MAINUI_CLICK_CONTROL, &loControl, sizeof(STRU_MAINUI_CLICK_CONTROL));
   }
   else {
      emit sigInteractiveClicked(mbIsFirstLoad, 0, 0);
      mbIsFirstLoad = false;
   }
}

void VhallIALive::InitRenderView(const VhallActiveJoinParam &param) {
   QJsonParseError json_error;
   QJsonArray speak_array;
   int arraySize = 0;
   QString name;
   QJsonDocument parse_doucment = QJsonDocument::fromJson(QByteArray(param.speaker_list.toStdString().c_str()), &json_error);
   if (json_error.error == QJsonParseError::NoError) {
      if (parse_doucment.isArray()) {
         speak_array = parse_doucment.array();
         arraySize = speak_array.size();
      }
   }
   TRACE6("%s mActiveParam.currPresenter %s mActiveParam.mainShow %s\n", __FUNCTION__, mActiveParam.currPresenter.toStdString().c_str(), mActiveParam.mainShow.toStdString().c_str());

   if (mActiveParam.role_name.compare(USER_HOST) == 0) {
      mbInitLayOut = true;
      if (mConfigBroadCast && !mActiveParam.bIsLiving) {
         mConfigBroadCast->SetNeedConfigBroadCast(true);
         TRACE6("%s SetNeedConfigBroadCast true\n", __FUNCTION__);
      }
      else if (mConfigBroadCast && mActiveParam.bIsLiving && speak_array.size() == 1 && (mActiveParam.currPresenter == mActiveParam.join_uid || mActiveParam.mainShow == mActiveParam.join_uid || mActiveParam.mainShowRole == USER_HOST)) {
         mConfigBroadCast->SetNeedConfigBroadCast(true);
         TRACE6("%s SetNeedConfigBroadCast true\n", __FUNCTION__);
      }
      else if (mConfigBroadCast && mActiveParam.bIsLiving && speak_array.size() == 0) {
         mConfigBroadCast->SetNeedConfigBroadCast(true);
         TRACE6("%s SetNeedConfigBroadCast true\n", __FUNCTION__);
      }
   }

   VH::CComPtr<ICommonData> pCommonData;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
   ClientApiInitResp respInitData;
   pCommonData->GetInitApiResp(respInitData);

   TRACE6("%s arraySize:%d", __FUNCTION__, arraySize);
   //解析上麦列表中的用户。
   bool bFindHostIsInSpeakerList = false;
   bool bFindGuestInSpeakerPos = false;

   for (int i = 0; i < arraySize; i++) {
      QJsonObject userObj = speak_array[i].toObject();
      QString join_uid, nick_name, role_name;
      if (respInitData.player == FLASH_LIVE_CREATE_TYPE) {
         join_uid = userObj["join_uid"].toString();
         nick_name = userObj["nick_name"].toString();
         role_name = userObj["role_name"].toString();
      }
      else {
         join_uid = userObj["account_id"].toString();
         nick_name = userObj["nick_name"].toString();
         role_name = userObj["role_name"].toString();
         // （1:老师 2 : 学员 3 : 助教 4 : 嘉宾 5 : 监课）
         if (role_name == "1") {
            role_name = USER_HOST;
         }
         else if (role_name == "2") {
            role_name = USER_USER;
         }
         else if (role_name == "3") {
            role_name = USER_ASSISTANT;
         }
         else if (role_name == "4") {
            role_name = USER_GUEST;
         }
      }

      if (mActiveParam.currPresenter == join_uid) {
         mActiveParam.mainShowRole = role_name;
      }
   }


   for (int i = 0; i < arraySize; i++) {
      QJsonObject userObj = speak_array[i].toObject();
      QString join_uid, nick_name, role_name;
      if (respInitData.player == FLASH_LIVE_CREATE_TYPE) {
         join_uid = userObj["join_uid"].toString();
         nick_name = userObj["nick_name"].toString();
         role_name = userObj["role_name"].toString();
      }
      else {
         join_uid = userObj["account_id"].toString();
         nick_name = userObj["nick_name"].toString();
         role_name = userObj["role_name"].toString();
         // （1:老师 2 : 学员 3 : 助教 4 : 嘉宾 5 : 监课）
         if (role_name == "1") {
            role_name = USER_HOST;
         }
         else if (role_name == "2") {
            role_name = USER_USER;
         }
         else if (role_name == "3") {
            role_name = USER_ASSISTANT;
         }
         else if (role_name == "4") {
            role_name = USER_GUEST;
         }
      }
      if (role_name == USER_HOST) {
         mActiveParam.hostJoinUid = join_uid;
      }
      if (mActiveParam.currPresenter == join_uid) {
         mActiveParam.mainShowRole = role_name;
      }

      if (mActiveParam.mainShow == join_uid) {
         if (role_name == USER_GUEST) {
            bFindGuestInSpeakerPos = true;
            mbIsHostSetGuestToSpeaker = true;
            TRACE6("%s mbIsHostSetGuestToSpeaker true\n", __FUNCTION__);
         }
      }
      int nAudio = userObj["audio"].isString() ? userObj["audio"].toString().toInt() : userObj["audio"].toInt();
      int nVideo = userObj["video"].isString() ? userObj["video"].toString().toInt() : userObj["video"].toInt();
      //此判断是为了防止主持人端崩溃，再次进入时要先创建显示的用户窗体。
      if (mActiveParam.role_name.compare(USER_HOST) == 0 && mActiveParam.bIsLiving) {
         bool bShowMain = join_uid == mActiveParam.mainShow ? true : false;
         TRACE6("%s host join_uid:%s nick_name:%s role_name:%s nAudio:%d nVideo:%d\n", __FUNCTION__, join_uid.toStdString().c_str(), nick_name.toStdString().c_str(), role_name.toStdString().c_str(), nAudio, nVideo);
         AppendRenderUser(join_uid, role_name, nick_name, nAudio == 1 ? true : false, nVideo == 1 ? true : false, bShowMain, true);
         //主持人在上麦列表中
         if (join_uid == mActiveParam.join_uid) {
            bFindHostIsInSpeakerList = true;
         }
      }
      else if (mActiveParam.role_name.compare(USER_HOST) != 0 && mActiveParam.bIsLiving && join_uid != mActiveParam.join_uid) {
         bool bShowMain = join_uid == mActiveParam.mainShow ? true : false;
         TRACE6("%s guest join_uid:%s nick_name:%s role_name:%s nAudio:%d nVideo:%d\n", __FUNCTION__, join_uid.toStdString().c_str(), nick_name.toStdString().c_str(), role_name.toStdString().c_str(), nAudio, nVideo);
         AppendRenderUser(join_uid, role_name, nick_name, nAudio == 1 ? true : false, nVideo == 1 ? true : false, bShowMain, true);
      }
      else if (mActiveParam.role_name.compare(USER_HOST) != 0 && mActiveParam.bIsLiving && join_uid == mActiveParam.join_uid) {
         //HttpSendNotSpeak(mActiveParam.join_uid, false);
         bool bShowMain = join_uid == mActiveParam.mainShow ? true : false;
         TRACE6("%s guest join_uid:%s nick_name:%s role_name:%s nAudio:%d nVideo:%d\n", __FUNCTION__, join_uid.toStdString().c_str(), nick_name.toStdString().c_str(), role_name.toStdString().c_str(), nAudio, nVideo);
         AppendRenderUser(join_uid, role_name, nick_name, nAudio == 1 ? true : false, nVideo == 1 ? true : false, bShowMain, true);
         mbIsGuestSpeaking = true;
         mbInitInRoom = true;
         continue;
      }
      TRACE6("%s find uid:%s nickName:%s role:%s \n", __FUNCTION__, join_uid.toStdString().c_str(), nick_name.toStdString().c_str(), role_name.toStdString().c_str());
   }

   if (!mActiveParam.bIsLiving) {
      mActiveParam.hostJoinUid = mActiveParam.mainShow;
   }

   if (mActiveParam.role_name.compare(USER_HOST) != 0 && mActiveParam.bIsLiving) {
      SetMainViewBackGroud(MAIN_VIEW_IS_LIVING);
   }
   //主持人当前活动已经开播，但是上麦列表没有主持人，需要判断主持人什么情况下不能上麦。
   else if (mActiveParam.role_name.compare(USER_HOST) == 0 && mActiveParam.bIsLiving) {
      //不在上麦列表中，并且主画面是嘉宾 ，主持人不能上麦。
      if (!bFindHostIsInSpeakerList && bFindGuestInSpeakerPos) {
         TRACE6("%s bFindGuestInSpeakerPos:%d  bFindHostIsInSpeakerList %d", __FUNCTION__, bFindGuestInSpeakerPos, bFindHostIsInSpeakerList);
         mbIsHostUnPublished = true;
         TRACE6("%s mbIsHostUnPublished = true\n", __FUNCTION__);
      }
      CheckHostShowUpperInMemberList();
   }

   if (mActiveParam.role_name.compare(USER_HOST) == 0) {
      VhallRenderWdg* render = GetRenderWnd(mActiveParam.join_uid);
      if (!render) {
         //没找到，显示上麦
         ui.tabWidget->mpMemberListDlg->SetSelfUpperWheat(true);
         ui.tabWidget->mpMemberListDlg->SetEnableWheat(true);
         mbIsHostSetGuestToSpeaker = true;
      }
   }
}

void VhallIALive::slot_OnClose() {
   if (!mbEnableStopStream) {
      return;
   }
   TRACE6("%s ------------ \n", __FUNCTION__);
   ui.widget_title->SetEnableCloseBtn(false);
   if (mActiveParam.role_name.compare(USER_HOST) == 0) {
      if (mActiveParam.bIsLiving) {
         AlertTipsDlg tip(EXIT_ON_LIVING, true, nullptr);
         tip.CenterWindow(this);
         tip.SetYesBtnText(DETERMINE);
         tip.SetNoBtnText(CANCEL);
         if (tip.exec() == QDialog::Accepted) {
            if (mpExitWaiting) {
               mpExitWaiting->Show();
            }
            mbClickedCloseExit = true;
            HttpSendStopLive();
         }
         else {
            if (mpExitWaiting) {
               mpExitWaiting->Close();
            }
            ui.widget_title->SetEnableCloseBtn(true);
         }
      }
      else {
         AlertTipsDlg tip(IF_EXIT_LIVE, true, nullptr);
         tip.CenterWindow(this);
         tip.SetYesBtnText(DETERMINE);
         tip.SetNoBtnText(CANCEL);
         if (tip.exec() == QDialog::Accepted) {
            mbExitRoom = true;
            QCoreApplication::postEvent(mpMainUILogic, new QEvent(CustomEvent_CloseExit));
         }
         else {
            ui.widget_title->SetEnableCloseBtn(true);
         }
      }
   }
   else {
      AlertTipsDlg tip(IF_EXIT_LIVE, true, nullptr);
      tip.CenterWindow(this);
      tip.SetYesBtnText(DETERMINE);
      tip.SetNoBtnText(CANCEL);
      if (tip.exec() == QDialog::Accepted) {
         mbExitRoom = true;
         QCoreApplication::postEvent(mpMainUILogic, new QEvent(CustomEvent_CloseExit), Qt::LowEventPriority);
      }
      else {
         ui.widget_title->SetEnableCloseBtn(true);
      }
   }
   ui.widget_title->SetEnableCloseBtn(true);
}

void VhallIALive::HandleCloseAndExit() {
   mActiveParam.bIsLiving = false;
   if (mpSettingDlg) {
      mpSettingDlg->SetStartLiving(false);
   }
   TRACE6("%s stop stream and exit\n", __FUNCTION__);
   mStartLiveTimer.stop();
   mStartLiveDateTime.setTime_t(0);
   mbExitRoom = true;
   if (mActiveParam.role_name != USER_HOST && mbIsGuestSpeaking) {//如果正在进行上麦。
      HttpSendNotSpeak(mActiveParam.join_uid, false);;
   }
   ExitRoom();
}

void VhallIALive::slot_OnFresh() {
   CreateDocWebView();
   if (mDocCefWidget) {
      if (mMediaFilePlayWdg || mDesktopWdg) {
         ui.widget_docLoadingTips->hide();
      }
      else {
         if (mDocLoadingTimer) {
            mDocLoadingTimer->start(700);
         }

         ui.widget_docLoadingTips->show();
      }
      mDocCefWidget->LoadUrl(mActiveParam.pluginUrl);
   }
   ui.tabWidget->FreshWebView();
}

void VhallIALive::ReleaseDocWebView() {

}

void VhallIALive::AddWebViewFromPlguinDlg(LibCefViewWdg* webView) {
   if (webView) {
      ui.verticalLayout_mainWebView->addWidget(webView);
   }
}

bool VhallIALive::IsEnableClicked(){
   if (mActiveParam.role_name.compare(USER_HOST) != 0 && !mActiveParam.bIsLiving) {
      FadeOutTip(NEED_TO_SPEAK, TipsType_None);
      return false;
   }
   return true;
}

bool VhallIALive::IsExistScreenShare(){
   //判断当前屏幕共享是否共享中
   return false;
}

bool VhallIALive::IsPlayDesktopShare() {
   if (mDesktopWdg != NULL) {
      return true;
   }
   if (m_pScreenShareToolWgd && !m_pScreenShareToolWgd->isHidden()) {
      return true;
   }
   return false;
}

bool VhallIALive::IsPlayMediaFile(){
   VH::CComPtr<IVedioPlayLogic> pVedioPlayLogic;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IMainUI, IID_IVedioPlayLogic, pVedioPlayLogic, return false, ASSERT(FALSE));
   if (mbIsPlayMediaFile) {
      TRACE6("%s  mbIsPlayMediaFile \n", __FUNCTION__);
      return true;
   }
   else if (mMediaFilePlayWdg != NULL) {
      TRACE6("%s  mMediaFilePlayWdg \n", __FUNCTION__);
      return true;
   }
   return false;
}

bool VhallIALive::IsPlayUiShow(){
   VH::CComPtr<IVedioPlayLogic> pVedioPlayLogic;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IMainUI, IID_IVedioPlayLogic, pVedioPlayLogic, return false, ASSERT(FALSE));
   bool bIsShowPlayUI = pVedioPlayLogic->IsPlayMediaFileUIShown();
   if (bIsShowPlayUI) {
      return true;
   }
   return false;
}

void VhallIALive::ClearSpeakerUser() {
   mSpeakUserCheckMap.clear();
}

void VhallIALive::ResetMemberState() {
   ui.tabWidget->mpMemberListDlg->ResetSpeakState();
}

void VhallIALive::InitMaxSize() {
   QRect rect = QApplication::desktop()->availableGeometry(this);
   QRect screen_geometry_rect = QApplication::desktop()->screenGeometry(this);
   if (rect == screen_geometry_rect) {
      rect.setHeight(screen_geometry_rect.height() - 1);
   }
   else {
      this->setFixedHeight(rect.height());
      this->setFixedWidth(rect.width());
      mNormalRect = rect;
   }

   this->setGeometry(rect);
}

void VhallIALive::slot_OnFullSize() {
   QRect rect = QApplication::desktop()->availableGeometry(this);
   ui.widget_title->SetMaxIcon(!mIsFullSize);
   if (mIsFullSize) {
      mIsFullSize = false;
      int moveX = (rect.width() - mNormalRect.width()) / 2;
      int moveY = (rect.height() - mNormalRect.height()) / 2;
      int iWidth = mNormalRect.width();
      int iHeight = mNormalRect.height();

      mNormalRect.setX(rect.x() + moveX);
      mNormalRect.setY(moveY >= 0 ? moveY : 0);
      mNormalRect.setWidth(iWidth);
      mNormalRect.setHeight(iHeight);

      this->setFixedHeight(iHeight);
      this->setFixedWidth(iWidth);

      this->setGeometry(mNormalRect);
      TRACE6("%s moveX:%d moveY:%d  rect.x():%d ", __FUNCTION__, moveX, moveY, rect.x());
   }
   else {
      this->setFixedHeight(rect.height());
      this->setFixedWidth(rect.width());
      TRACE6("%s  width %d height %d", __FUNCTION__, rect.width(), rect.height());
      this->setGeometry(rect);
      TRACE6("%s moveX:%d moveY:%d width %d height %d", __FUNCTION__, rect.x(), rect.y(), rect.width(), rect.height());
      mIsFullSize = true;
   }

   if (mShowTipsWdgPtr) {
      mShowTipsWdgPtr->ResizeWidth(this->width() - ui.widget_leftTool->width() - ui.tabWidget->width());
      slot_ReSetTipsPos();
   }
   ui.widget_title->SetMaxIcon(mIsFullSize);
}

void VhallIALive::resizeEvent(QResizeEvent *event) {
   ReSizeMediaCtrlWdg();
   ResetMediaCtrlWdgPos();
   TRACE6("%s width %d  height %d", __FUNCTION__, width(), height());
   if (mShowTipsWdgPtr) {
       mShowTipsWdgPtr->setFixedWidth(this->width() - ui.widget_leftTool->width() - ui.tabWidget->width());
       slot_ReSetTipsPos(); 
   }
   if (!IsMainRenderNormalPos()) {
      VhallRenderWdg* render = nullptr;
      if (ui.listWidgetMainView->count() > 0) {
         QListWidgetItem *renderItem = ui.listWidgetMainView->item(0);
         render = (VhallRenderWdg*)ui.listWidgetMainView->itemWidget(renderItem);
         if (render) {
            render->SwitchCenterSize(ui.widget_5->width(), ui.widget_5->height());
         }
      }
   }
   QWidget::resizeEvent(event);
}

void VhallIALive::moveEvent(QMoveEvent *event) {
   ResetMediaCtrlWdgPos();
   slot_ReSetTipsPos();
   QWidget::moveEvent(event);
}

void VhallIALive::ResetMediaCtrlWdgPos() {
   if (mpVedioPlayUi) {
      QPoint pos = this->mapToGlobal(ui.widget_bottomTool->pos()) - QPoint(0, mpVedioPlayUi->height()) + QPoint(ui.widget_leftTool->width(), 0);
      mpVedioPlayUi->move(pos);
   }
}

void VhallIALive::ReSizeMediaCtrlWdg() {
   if (mpVedioPlayUi) {
      int width = GetMediaPlayUIWidth();
      mpVedioPlayUi->setFixedWidth(width);
      TRACE6("%s  mpVedioPlayUi->setFixedWidth(%d) \n", __FUNCTION__, width);
   }
}

void VhallIALive::slot_OnMinSize() {
   showMinimized();
}

void VhallIALive::slot_OnSetting() {
   TRACE6("%s  ---------------------------------- \n", __FUNCTION__);
   if (mpSettingDlg) {
      wstring wzGurVersion;
      VH::CComPtr<ICommonData> pCommonData;
      DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
      if (pCommonData) {
         pCommonData->GetCurVersion(wzGurVersion);
         mpSettingDlg->SerVersion(QString::fromStdWString(wzGurVersion.c_str()));
      }
      mDeviceGetThread.SetRequestType(DevRequest_GetDevList);
   }
}

void VhallIALive::slot_OnShare() {
   QString toolConfPath = CPathManager::GetAppDataPath() + QString::fromStdWString(VHALL_TOOL_CONFIG);
   QString domain = ConfigSetting::ReadString(toolConfPath, GROUP_DEFAULT, KEY_VHALL_ShareURL, "http://live.vhall.com");
   if (mpShareDlg == nullptr) {
      mpShareDlg = new VhallSharedQr(this);
   }
   if (mpShareDlg) {
      mpShareDlg->CenterWindow(this);
      mpShareDlg->Shared(domain, mActiveParam.streamName);
   }
}

void VhallIALive::slot_OnCopy() {
   FadeOutTip(COPY_SUCCESS, TipsType_Success);
}

void VhallIALive::slot_CameraBtnClicked() {
   TRACE6("%s enter mbIsCameraOpen:%d\n", __FUNCTION__, mbIsCameraOpen);
}

void VhallIALive::slot_MicButtonClicked() {
   if (!mCurMicDevID.isEmpty()) {
      TRACE6("%s enter mbIsMicOpen:%d\n", __FUNCTION__, mbIsMicOpen);
      slot_ClickedMic(mActiveParam.join_uid, mbIsMicOpen);
      mMicVolume = mbIsMicOpen ? 100 : 0;
      QJsonObject body;
      if (mbIsMicOpen)
         body["vb"] = "Close";
      else
         body["vb"] = "Open";
      SingletonMainUIIns::Instance().reportLog(STR_VH_LIVE_OPERATE, eLogRePortK_VHSDK_Mic, L"VHSDK_Mic", body);
   }
   TRACE6("%s leave\n", __FUNCTION__);
}

void VhallIALive::slot_PlayerButtonClicked() {
   if (!mCurPlayerID.isEmpty()) {
      TRACE6("%s enter mbIsPlayerOpen:%d\n", __FUNCTION__, mbIsPlayerOpen);
      mPlayerVolume = mbIsPlayerOpen ? 0 : 100;

      QJsonObject body;
      if (mbIsPlayerOpen){
         body["vb"] = "Close";
      }
      else{
         body["vb"] = "Open";
      }
      SingletonMainUIIns::Instance().reportLog(STR_VH_LIVE_OPERATE, eLogRePortK_VHSDK_Player, L"VHSDK_Player", body);
      GetWebRtcSDKInstance()->SetCurrentPlayVol(mPlayerVolume);
      mbIsPlayerOpen = !mbIsPlayerOpen;
      if (NULL != mpSettingDlg){
         mpSettingDlg->OnOpenPlayer(mbIsPlayerOpen);
         mpSettingDlg->setPlayerVolume(mPlayerVolume);
      }
      if (NULL != m_pScreenShareToolWgd){
         m_pScreenShareToolWgd->SetPlayerState(mbIsPlayerOpen);
      }
   }
   TRACE6("%s leave\n", __FUNCTION__);
}

bool GetScreenList(vector<VHD_WindowInfo>& allWindows) {
   BOOL enum_result = TRUE;

   for (int device_index = 0;; ++device_index) {
      DISPLAY_DEVICE device;
      device.cb = sizeof(device);
      enum_result = EnumDisplayDevices(NULL, device_index, &device, 0);

      // |enum_result| is 0 if we have enumerated all devices.
      if (!enum_result)
         break;

      // We only care about active displays.
      if (!(device.StateFlags & DISPLAY_DEVICE_ACTIVE))
         continue;

      DEVMODE device_mode;
      device_mode.dmSize = sizeof(device_mode);
      device_mode.dmDriverExtra = 0;
      BOOL result = EnumDisplaySettingsEx(device.DeviceName, ENUM_CURRENT_SETTINGS, &device_mode, 0);
      VHD_WindowInfo info;
      info.type = VHD_Desktop;
      RECT screenRect;
      screenRect.left = device_mode.dmPosition.x;
      screenRect.top = device_mode.dmPosition.y;
      screenRect.right = device_mode.dmPosition.x + device_mode.dmPelsWidth;
      screenRect.bottom = device_mode.dmPosition.y + device_mode.dmPelsHeight;
      info.rect = screenRect;
      info.screen_id = device_index;
      memcpy(info.name, device.DeviceName,sizeof(device.DeviceName));
      allWindows.push_back(info);
   }
   return true;
}

//桌面共享
void VhallIALive::slot_BtnScreenClicked() {
   TRACE6("%s \n", __FUNCTION__);
   if (!IsEnableUsing()) {
      return;
   }
   if (!GetWebRtcSDKInstance()->IsWebRtcRoomConnected()) {
      FadeOutTip(DISABLE_DESKTOP, TipsType_Error);
      return;
   }
   if (!IsEnalbePlayMedia()) {
      FadeOutTip(NOT_MAIN_SPEAKER, TipsType_Error);
      return;
   }

   if (IsPlayMediaFile() && !mbIsDeskTop) {
      AlertTipsDlg tip(CLOSE_FILE_TO_DESKTOP, true, this);
      tip.CenterWindow(this);
      if (tip.exec() == QDialog::Accepted) {
         SingletonMainUIIns::Instance().PostCRMessage(MSG_MAINUI_VEDIOPLAY_STOPPLAY, NULL, 0);
         slot_ClickedMic(mActiveParam.join_uid,false);
         GetWebRtcSDKInstance()->StopMediaFileCapture();
         mbIsPlayMediaFile = false;
         StopShowMediaVideo();
      }
      else {
         return;
      }
   }

   bool currentScreenStatus = true;
   if (!currentScreenStatus) {
      AlertTipsDlg alert(ALREADY_SCREEN_SHARE, false, NULL);
      alert.exec();
      return;
   }

   TRACE6("%s currentScreenStatus:%d mbIsDeskTop %d\n", __FUNCTION__, currentScreenStatus, mbIsDeskTop);
   QJsonObject body;
   if (mbIsDeskTop) { //桌面共享中
      if (!mbEnableStopDesktopStream) {
         TRACE6("%s mbEnableStopDesktopStream %d\n", __FUNCTION__, mbEnableStopDesktopStream);
         return;
      }
      if (GetWebRtcSDKInstance()->IsPushingStream(vlive::VHStreamType_Desktop)) {
         StopPublishDeskTopStream();
      }
      body["ac"] = "Close";
      StopDeskTopStream();
      CloseScreenShare();
      if (mActiveParam.mbIsH5Live) {
         emit sigToStopDesktopSharing();
      }
   }
   else {
      body["ac"] = "Open";
      int width = 0;
      int height = 0;
      vector<VHD_WindowInfo> allDesktop;
      GetScreenList(allDesktop);
      if (allDesktop.size() > 1) {
         if (ShowScreenSelectDlg(allDesktop,this) == QDialog::DialogCode::Rejected) {
            return;
         }
      }
      else {
         TRACE6("%s  StartDeskTopStream %d\n", __FUNCTION__, mbEnableStopDesktopStream);
         StartDeskTopStream(allDesktop.at(0).screen_id);
         if (m_pScreenShareToolWgd) {
            m_pScreenShareToolWgd->SetShowShareSelect(false);
            m_pScreenShareToolWgd->SetCurrentShareScreenInfo(allDesktop.at(0));
         }
         mCurrentCaptureScreenIndex = 0;
      }

   }
}


void VhallIALive::slot_OnShareSelect() {
   vector<VHD_WindowInfo> allDesktop;
   GetScreenList(allDesktop);
   if (allDesktop.size() > 1) {
      if (ShowScreenSelectDlg(allDesktop, m_pScreenShareToolWgd) == QDialog::DialogCode::Rejected) {
         return;
      }
   }
}

void VhallIALive::slot_OnFloatWndSwitch() {
   if (mDesktopCameraRenderWdgPtr) {
      if (mDesktopCameraRenderWdgPtr->isHidden()) {
         if (m_pScreenShareToolWgd) {
            VHD_WindowInfo win_info = m_pScreenShareToolWgd->GetCurrentShareWindowInfo();
            QRect screen_rect;
            QDesktopWidget* desktopWidget = QApplication::desktop();
            if (desktopWidget) {
               QList<QScreen *> screenlist = QGuiApplication::screens();
               for (int i = 0; i < screenlist.size(); i++) {
                  if (QString::fromStdWString(win_info.name) == screenlist.at(i)->name()) {
                     screen_rect = screenlist.at(i)->availableGeometry();
                     break;
                  }
               }
            }

            if (mDesktopCameraRenderWdgPtr) {
               bool isOpenCamera = false;
               VhallRenderWdg* render = GetRenderWnd(mActiveParam.join_uid);
               if (render) {
                  isOpenCamera = render->GetUserCameraState();
               }
               mDesktopCameraRenderWdgPtr->SetCloseCamera(!isOpenCamera);
               mDesktopCameraRenderWdgPtr->StartRender();
               mDesktopCameraRenderWdgPtr->ReSize();
               mDesktopCameraRenderWdgPtr->move(screen_rect.x() + screen_rect.width() - mDesktopCameraRenderWdgPtr->width() - 100, 100);
               if (!mActiveParam.bIsLiving) {
                  mDesktopCameraRenderWdgPtr->SetCurrentIndex(2);
               }
               mDesktopCameraRenderWdgPtr->show();
            }
            m_pScreenShareToolWgd->SetFloatWndBtnState(true);
         }
      }
      else {
         mDesktopCameraRenderWdgPtr->hide();
         m_pScreenShareToolWgd->SetFloatWndBtnState(false);
      }
   }
}


int VhallIALive::ShowScreenSelectDlg(vector<VHD_WindowInfo> &allDesktop,QWidget* reference) {
   TRACE6("%s screen count %d\n", __FUNCTION__, allDesktop.size());
   if (mDesktopShareSelectWdg == nullptr) {
      mDesktopShareSelectWdg = new DesktopShareSelectWdg();
   }
   if (!mDesktopShareSelectWdg->isHidden()) {
      return QDialog::DialogCode::Rejected;
   }
   mDesktopShareSelectWdg->InsertDesktopPreviewItem(allDesktop);
   int x = 0, y = 0;
   int screen_index = ConfigSetting::ContainsInDesktopIndex(reference);
   ConfigSetting::GetInScreenPos(screen_index, mDesktopShareSelectWdg, x, y);
   mDesktopShareSelectWdg->move(x, y);

   if (mDesktopShareSelectWdg->exec() == QDialog::DialogCode::Accepted) {
      int index = mDesktopShareSelectWdg->GetCurrentIndex();
      VHD_WindowInfo currentScreen = mDesktopShareSelectWdg->GetCurrentWindowInfo();
      StartDeskTopStream(index);
      mCurrentCaptureScreenIndex = index;
      TRACE6("%s select index %d\n", __FUNCTION__, index);
      if (m_pScreenShareToolWgd) {
         m_pScreenShareToolWgd->SetShowShareSelect(true);
         m_pScreenShareToolWgd->SetCurrentShareScreenInfo(currentScreen);
         if (!m_pScreenShareToolWgd->isHidden()) {
            m_pScreenShareToolWgd->Show();
         }
         if (mDesktopCameraRenderWdgPtr && !mDesktopCameraRenderWdgPtr->isHidden()) {
            bool isOpenCamera = false;
            QRect screen_rect;
            QDesktopWidget* desktopWidget = QApplication::desktop();
            if (desktopWidget) {
               QList<QScreen *> screenlist = QGuiApplication::screens();
               for (int i = 0; i < screenlist.size(); i++) {
                  if (QString::fromStdWString(currentScreen.name) == screenlist.at(i)->name()) {
                     screen_rect = screenlist.at(i)->availableGeometry();
                     break;
                  }
               }
            }

            mDesktopCameraRenderWdgPtr->ReSize();
            mDesktopCameraRenderWdgPtr->move(screen_rect.x() + screen_rect.width() - mDesktopCameraRenderWdgPtr->width() - 100, 100);
            mDesktopCameraRenderWdgPtr->show();
         }
      }

      delete mDesktopShareSelectWdg;
      mDesktopShareSelectWdg = nullptr;
      return QDialog::DialogCode::Accepted;
   }
   else {
      TRACE6("%s close\n", __FUNCTION__);
      delete mDesktopShareSelectWdg;
      mDesktopShareSelectWdg = nullptr;
      return QDialog::DialogCode::Rejected;
   }
}

void VhallIALive::showEvent(QShowEvent *event) {
   ui.tabWidget->SetMemberListCAliveDlg(this);
   VH::CComPtr<ICommonData> pCommonData;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
   ClientApiInitResp initApiResp;
   pCommonData->GetInitApiResp(initApiResp);
   if (pCommonData) {
      int startMode = pCommonData->GetStartMode();
      if (FLASH_LIVE_CREATE_TYPE == initApiResp.player){
         ui.tabWidget->SetShowTabBar(false);
      }
      else{
         ui.tabWidget->SetShowTabBar(true);
      }
   }

   if (nullptr != mPerformanceStatisticsPtr){
      mPerformanceStatisticsPtr->Start();
   }

   if (mTeachingWidget == nullptr && mIsShowTeachingWidget) {
      mTeachingWidget = new QWidget(this);
      if (mTeachingWidget) {
         mTeachingWidget->installEventFilter(this);
         mTeachingWidget->setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowSystemMenuHint | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint);
         mTeachingWidget->setFixedWidth(mNormalRect.width());
         mTeachingWidget->setFixedHeight(mNormalRect.height());
         mTeachingWidget->setStyleSheet("border-image: url(:/teaching/img/teaching/active_live_chat.png);");
         mTeachingWidget->move(this->pos());
         mTeachingWidget->show();
         mTeachingWidget->installEventFilter(this);
      }
   } 
   this->setAttribute(Qt::WA_Mapped);
   QWidget::showEvent(event);
}

void VhallIALive::hideEvent(QHideEvent *event) {
   if (mShowTipsWdgPtr) {
      mShowTipsWdgPtr->hide();
   }
}

void VhallIALive::closeEvent(QCloseEvent *event) {
   if (event) {
      event->ignore();
   }
   slot_OnClose();
}

int VhallIALive::InitMediaStream(const char *szMediaFile) {
   QFile file(szMediaFile);
   if (!file.exists()) {
      QCoreApplication::postEvent(mpMainUILogic, new QEvent(CustomEvent_GetLocalFileStreamErr));
      return -1;
   }

   mCurPlayFileName = szMediaFile;
   StartPlayMediaStream();
   TRACE6("%s StartPlayMediaStream\n", __FUNCTION__);
   return 0;
}

void VhallIALive::slot_SettingDlgProfileChanged() {
   if (GetWebRtcSDKInstance()->IsCapturingStream(VHStreamType_MediaFile)) {
      StartPlayMediaStream(true);
   }
   if (GetWebRtcSDKInstance()->IsCapturingStream(VHStreamType_Desktop)) {
      StartDeskTopStream(mCurrentCaptureScreenIndex);
   }
}

void VhallIALive::StartPlayMediaStream(bool changeProfile) {
   VideoProfileIndex profile = VhallIALiveSettingDlg::GetPlayMediaFileProfile(mCurPlayFileName);
   long long seekPos = 0;
   if (changeProfile) {
      GetWebRtcSDKInstance()->ChangeMediaFileProfile(profile);
   }
   else {
      if (GetWebRtcSDKInstance()->IsPushingStream(VHStreamType_MediaFile)) {
         TRACE6("Enter Function:%s VHStreamType_MediaFile IsPushingStream\n", __FUNCTION__);
         GetWebRtcSDKInstance()->PlayFile(mCurPlayFileName.toStdString(), profile);
      }
      else {
         TRACE6("Enter Function:%s InitMediaFile\n", __FUNCTION__);
         GetWebRtcSDKInstance()->InitMediaFile();
      }
   }
}

void VhallIALive::slot_BtnStreamClicked() {
   if (m_pScreenShareToolWgd) {
      m_pScreenShareToolWgd->SetStartLiveBtnEnable(false);
   }
   TRACE6("Enter Function:%s \n", __FUNCTION__);
   if (!mActiveParam.bIsLiving) { //开播
      QJsonObject body;
      body["ac"] = mActiveParam.bIsLiving ? "StopStream" : "StartStream";
      SingletonMainUIIns::Instance().reportLog(L"interaction_startorstopstream", eLogRePortK_Interaction_Start, mActiveParam.bIsLiving ? L"StopStream" : L"StartStream", body);
      if (!GetWebRtcSDKInstance()->IsWebRtcRoomConnected()) {
         QString msg;
         if (mActiveParam.role_name == USER_HOST) {
            msg = RTC_CONNECT_ERR_HOST_RETRY;
         }
         else {
            msg = RTC_CONNECT_ERR_GUEST_RETRY;
         }
         AlertTipsDlg tip(msg, false, this);
         tip.CenterWindow(this);
         tip.SetYesBtnText(DETERMINE);
         tip.exec();
         if (m_pScreenShareToolWgd) {
            m_pScreenShareToolWgd->SetStartLiveBtnEnable(true);
         }
         return;
      }

      if (!GetWebRtcSDKInstance()->HasAudioDev() && !GetWebRtcSDKInstance()->HasVideoDev()) {
         FadeOutTip(RTC_NO_DEVICE, 0);
         return;
      }
      TRACE6("Enter Function:%s m_SDK->GetStreamState() = false\n", __FUNCTION__);
      if (mpStartWaiting) {
         mpStartWaiting->Show();
      }
      HttpSendStartLive();
   }
   else {
      if (!mActiveParam.bIsLiving) {
         TRACE6("m_streamStatus = false\n");
         if (m_pScreenShareToolWgd) {
            m_pScreenShareToolWgd->SetStartLiveBtnEnable(true);
         }
         return;
      }
      TRACE6("%s mbEnableStopStream %d\n", __FUNCTION__, mbEnableStopStream);
      if (!mbEnableStopStream) {
         QString tips = DEV_IS_INIT;
         FadeOutTip(tips, TipsType_Normal);
         if (m_pScreenShareToolWgd) {
            m_pScreenShareToolWgd->SetStartLiveBtnEnable(true);
         }
         return;
      }
      if (mpExitWaiting) {
         mpExitWaiting->Show();
      }
      HttpSendStopLive();
   }
   TRACE6("Leave Function:%s \n", __FUNCTION__);
}

bool VhallIALive::IsLiveTimeShort() {
   long time = ui.widget_title->GetLiveTime();
   TRACE6("%s IsLiveTimeShort:%d\n", __FUNCTION__, time);
   if (time > 30) {
      return false;
   }
   return true;
}

void VhallIALive::HandleHttpResponseChangeToSpeak(const QString &url, const QString& msg, int code, const QString& uid, bool isNoticeMsg) {
   TRACE6("%s enter: msg:%s code:%d uid:%s\n", __FUNCTION__, msg.toStdString().c_str(), code, uid.toStdString().c_str());
   if (isNoticeMsg) {
      if (code != 200) {
         if (msg.isEmpty()) {
            FadeOutTip(HTTP_SEND_TIMEOUT, TipsType_Error);
         }
         else {
            FadeOutTip(msg, TipsType_Error);
         }
      }
      else {
         if (mActiveParam.role_name == USER_HOST) {
            //主持人上麦之后, 如果主讲人还是其它嘉宾，那么要在成员列表显示下麦，并显示在小窗；
            //如果主持人上麦，并且没有设置其它嘉宾为主讲人，则显示在主讲人位置
            bool showSpeakerPos = mActiveParam.mainShow == mActiveParam.join_uid ? true : false;
            if (mbIsHostSetGuestToSpeaker) {
               ui.tabWidget->mpMemberListDlg->SetSelfUpperWheat(false);
            }
            TRACE6("%s host change to speaker. mainView:%s\n", __FUNCTION__, mActiveParam.mainShow.toStdString().c_str());
            StartLiveWithOpenDevice();
            VhallRenderWdg* render = GetRenderWnd(mActiveParam.join_uid);
            if (render) {
               if (showSpeakerPos) {  //显示在主讲人区域
                  render->ShowSetKickOutBtn(false);
                  render->ShowSetMainViewBtn(false);
                  render->ShowSetSpeakBtn(false);
               }
               else {   //显示在嘉宾区域
                  render->ShowSetMainViewBtn(false);
                  render->ShowSetSpeakBtn(true);
                  render->ShowSetKickOutBtn(true);
               }
            }
            if (mActiveParam.role_name == USER_HOST) {
               ui.tabWidget->mpMemberListDlg->ShowSetmainSpeaker(mActiveParam.mainShow);
               ui.tabWidget->mpMemberListDlg->FlushBtnState(mActiveParam.join_uid);
            }
            mbIsHostUnPublished = false;
            TRACE6("%s mbIsHostUnPublished = false\n", __FUNCTION__);
         }
         else {
            TRACE6("%s send to speak suc\n", __FUNCTION__);
            ui.widget_title->SetToSpeakBtnState(false);
            StartLiveWithOpenDevice();
            mbIsGuestSpeaking = true;
            if (mActiveParam.role_name == USER_HOST) {
               mStartLiveDateTime.setTime_t(0);
               mStartLiveTimer.start(1000);
            }
            AppendRenderUser(mActiveParam.join_uid, mActiveParam.role_name, mActiveParam.userNickName, true, true, false, false);
         }
         TRACE6("%s tospeak suc\n", __FUNCTION__);
      }
   }
   TRACE6("%s end\n", __FUNCTION__);
}

void VhallIALive::ShowInteractiveTools(const int& x, const int& y)
{
   if (nullptr == mLiveToolDlg)
   {
      mLiveToolDlg = new LiveToolDlg(this);
      mLiveToolDlg->Create();
      connect(mLiveToolDlg, &LiveToolDlg::sigClicked, this, &VhallIALive::slotLiveToolChoise);
   }
   mLiveToolDlg->move(x, y);
   mLiveToolDlg->show();
}

void VhallIALive::slotLiveToolChoise(const int& iOpType) {
   QString funPage;
   switch (iOpType)
   {
   case eOpType_Questions: { //  问答 
      funPage = QString("QtCallFunctionPage(%1)").arg(eDocCtrlType_Questions);
   }
   break;
   case eOpType_Signin: { //  签到  
      funPage = QString("QtCallFunctionPage(%1)").arg(eDocCtrlType_Signin);
   }
   break;
   case eOpType_Luck: { // 抽奖
      funPage = QString("QtCallFunctionPage(%1)").arg(eDocCtrlType_Luck);
   }
   break;
   case eOpType_questionnaire:   { // 问卷 
      funPage = QString("QtCallFunctionPage(%1)").arg(eDocCtrlType_questionnaire);
   }
   break;
   case eOpType_envelopes:   { // 红包  
   }
   break;
   default:
      break;
   }

   if (mDocCefWidget && !funPage.isEmpty()) {
      mDocCefWidget->AppCallJsFunction(funPage);
   }
}

void VhallIALive::HandleHttpResponseNotSpeak(const QString &url, const QString& msg, int code, const QString& uid, bool bNoticeMsg) {
   //自己下麦请求回调。
   TRACE6("%s enter: msg:%s code:%d uid:%s\n", __FUNCTION__, msg.toStdString().c_str(), code, uid.toStdString().c_str());
   if (bNoticeMsg) {
      if (code != 200) {
         mbExitRoom = false;
         if (msg.isEmpty()) {
            FadeOutTip(NO_SPEAK_MEMBER_TIMEOUT, TipsType_Error);
         }
         else {
            FadeOutTip(msg, TipsType_Error);
         }
      }
      else {
         //控制自己下麦
         if (uid == mActiveParam.join_uid) {
            if (mActiveParam.role_name == USER_HOST) {
               mbIsHostUnPublished = true;
               TRACE6("%s mbIsHostUnPublished = true\n", __FUNCTION__);
            }
            ui.widget_title->SetToSpeakBtnState(true);
            ResetToNospeakState();
            mbIsGuestSpeaking = false;
            TRACE6("%s mbIsGuestSpeaking = false\n", __FUNCTION__);
            RemoveRemoteUser(mActiveParam.join_uid);
            TRACE6("%s off speak suc\n", __FUNCTION__);
            if (mbExitRoom){
               ExitRoom();
               mbExitRoom = false;
            }
            if (mActiveParam.role_name == USER_HOST) {
               ui.tabWidget->mpMemberListDlg->SetSelfUpperWheat(true);
            }
         }
         else {
            //控制别人下麦。
            RemoveRemoteUser(uid);
         }
         mbExitRoom = false;
      }
   }
   TRACE6("%s end\n", __FUNCTION__);
}

void VhallIALive::HandleHttpResponseStopwebinarr(const QString &url, const QString& msg, int code, const QString& uid, bool isNoticeMsg) {
   TRACE6("Enter Function:%s \n", __FUNCTION__);
   if (code != 200) {
      if (msg.isEmpty()) {
         FadeOutTip(STOP_LIVE_FAILED, TipsType_Error);
      }
      else {
         FadeOutTip(msg, TipsType_Error);
      }
   }
   else {
      //结束直播时恢复状态为0 ，表示直播过程中无异常
      ConfigSetting::writeValue(CPathManager::GetConfigPath(), GROUP_DEFAULT, KEY_CRASH_STATE, 0);
      VH::CComPtr<ICommonData> pCommonData;
      DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
      ClientApiInitResp respInitData;
      pCommonData->GetInitApiResp(respInitData);
      VSSGetRoomInfo roomInfo;
      pCommonData->GetVSSRoomBaseInfo(roomInfo);
      ui.widget_title->SetRecordState(eRecordState_Stop);
      HandleStopLiving();
      bool bIsShort = IsLiveTimeShort();
      mActiveParam.live_time = 0;
      mActiveParam.bIsLiving = false;
      mActiveParam.mainShow = mActiveParam.join_uid;
      mActiveParam.currPresenter = mActiveParam.join_uid;
      mActiveParam.mainShowRole = USER_HOST;
      InitLeftBtnEnableState();
      QString confPath = CPathManager::GetConfigPath();
      QString localProfile = ConfigSetting::writeValue(confPath, GROUP_DEFAULT, KEY_LOCAL_VIDEO_PROFILE, PROFILE_INDEX_STANDER);
      if (mpSettingDlg) {
         mpSettingDlg->ShowDesktopCaptrue(true);
         mpSettingDlg->SetStartLiving(false);
         mpSettingDlg->ResetProfile(1);
      }
      ui.widget_leftTool->slot_OnClickedDoc();
      if (bIsShort) {
         FadeOutTip(LIVE_TIME_TOO_SHORT, TipsType_Error);
         if (mbClickedCloseExit) {
            HandleCloseAndExit();
         }
      }
      else {
         TRACE6("Enter Function:%s !pVhallRightExtraWidget\n", __FUNCTION__);
         TRACE6("%s SetEnableCloseBtn(false)\n", __FUNCTION__);
         ui.widget_title->SetEnableCloseBtn(false);
         mpMainUILogic->CreateRecord(mbClickedCloseExit);
         ui.widget_title->SetEnableCloseBtn(true);
         TRACE6("%s SetEnableCloseBtn(true)\n", __FUNCTION__);
      }
      if (respInitData.player == H5_LIVE_CREATE_TYPE) {
         VSSGetRoomInfo roomInfo;
         pCommonData->GetVSSRoomBaseInfo(roomInfo);
         GetPaasSDKInstance()->StopPublishInavAnother(roomInfo.room_id.toStdString());
      }
      else {
         GetWebRtcSDKInstance()->StopBroadCast();
      }
   }
   if (mpExitWaiting) {
      mpExitWaiting->Close();
   }
   TRACE6("Leave Function:%s \n", __FUNCTION__);
}

void VhallIALive::HandleHttpResponseSpeakList(const QString &url, const QString& msg, int code, const QString& uid, bool isNoticeMsg, QString msgData, bool bHasVideo) {
   TRACE6("%s code:%d", __FUNCTION__, code);
   QJsonParseError json_error;
   QJsonArray speak_array;
   int arraySize = 0;
   QString name;
   bool bIsSpeakUser = false;
   TRACE6("%s check user mSubScribeUserID:%s\n", __FUNCTION__, mSubScribeUserID.toStdString().c_str());

   VH::CComPtr<ICommonData> pCommonData;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
   ClientApiInitResp respInitData;
   pCommonData->GetInitApiResp(respInitData);

   QJsonDocument parse_doucment = QJsonDocument::fromJson(QByteArray(msg.toStdString().c_str()), &json_error);
   if (json_error.error == QJsonParseError::NoError) {
      if (parse_doucment.isObject()) {
         QJsonObject obj = parse_doucment.object();
         if (obj.contains("data")) {
            if (mActiveParam.mbIsH5Live) {
               QJsonObject dataObj = obj["data"].toObject();
               if (dataObj.contains("speaker_list") && dataObj["speaker_list"].isArray()) {
                  speak_array = dataObj["speaker_list"].toArray();
                  arraySize = speak_array.size();
               }
            }
            else {
               speak_array = obj["data"].toArray();
               arraySize = speak_array.size();
            }
            std::vector<QString> speaklist;
            //解析上麦列表中的用户。
            for (int i = 0; i < arraySize; i++) {
               QJsonObject userObj = speak_array[i].toObject();
               QString join_uid, nick_name, role_name;
               if (respInitData.player == FLASH_LIVE_CREATE_TYPE) {
                  join_uid = userObj["join_uid"].toString();
                  nick_name = userObj["nick_name"].toString();
                  role_name = userObj["role_name"].toString();
               }
               else {
                  join_uid = userObj["account_id"].toString();
                  nick_name = userObj["nick_name"].toString();
                  role_name = userObj["role_name"].toString();
                  // （1:老师 2 : 学员 3 : 助教 4 : 嘉宾 5 : 监课）
                  if (role_name == "1") {
                     role_name = USER_HOST;
                  }
                  else if (role_name == "2") {
                     role_name = USER_USER;
                  }
                  else if (role_name == "3") {
                     role_name = USER_ASSISTANT;
                  }
                  else if (role_name == "4") {
                     role_name = USER_GUEST;
                  }
               }
               speaklist.push_back(join_uid);
               int nAudio = userObj["audio"].isString() ? userObj["audio"].toString().toInt() : userObj["audio"].toInt();
               int nVideo = userObj["video"].isString() ? userObj["video"].toString().toInt() : userObj["video"].toInt();
               TRACE6("%s join_uid:%s  nVideo %d", __FUNCTION__, join_uid.toStdString().c_str(), nVideo);
               VhallRenderWdg* existRender = GetRenderWnd(join_uid);
               if (existRender == nullptr) {
                  if (join_uid != mActiveParam.join_uid) {
                     AppendRenderUser(join_uid, role_name, nick_name, nAudio > 0 ? true : false, nVideo > 0 ? true : false, mActiveParam.mainShow == join_uid, false);
                     VhallRenderWdg *render = GetRenderWnd(join_uid);
                     if (render) {
                        HWND wnd = render->GetRenderWndID();
                        TRACE6("%s StartRenderRemoteStream join_uid:%s  ", __FUNCTION__, join_uid.toStdString().c_str());
                        std::shared_ptr<vhall::VideoRenderReceiveInterface> videoReciver = std::shared_ptr<vhall::VideoRenderReceiveInterface>(render->GetVideoReciver());
                        bool bRet = vlive::GetWebRtcSDKInstance()->StartRenderRemoteStream(join_uid.toStdWString(), vlive::VHStreamType::VHStreamType_AVCapture, videoReciver);
                        render->SetViewState(RenderView_None);
                     }
                  }
               }
               else {
                  if (existRender) {
                     PlayRemoteUser(existRender, join_uid, nVideo > 0 ? true : false);
                  }
               }
            }
            IsInSpeakList(speaklist);
         }
      }
   }
}

void VhallIALive::HandleHttpResponseHandsup(const QString &url, const QString& msg, int code, const QString& uid, bool isNoticeMsg) {
   TRACE6("Enter Function:%s \n", __FUNCTION__);
   //ui.widget_ctrl->EnableStartLive(true);
   if (code != 200) {
      if (msg.isEmpty()) {
         FadeOutTip(SPEAK_MSG_SEND_ERR, TipsType_Error);
      }
      else {
         FadeOutTip(msg, TipsType_Error);
      }
   }
   TRACE6("Leave Function:%s \n", __FUNCTION__);
}

void VhallIALive::HandleOnRecvHttpResponse(QEvent* event) {

}

QString VhallIALive::GetDocSetMainViewJson(const QString &uid, const QString& type) {
   QJsonObject cmdObj;
   QJsonObject valueObj;
   QJsonArray jsonArray;
   valueObj["type"] = type;// "*setMainSpeaker";
   valueObj["join_uid"] = uid;
   QJsonDocument parse_doucment;
   parse_doucment.setObject(valueObj);
   QString valueData = parse_doucment.toJson();
   cmdObj["key"] = "cmd";
   cmdObj["value"] = valueData;

   QJsonDocument doc;
   doc.setObject(cmdObj);
   QByteArray data = doc.toJson();

   TRACE6("%s data:%s\n", __FUNCTION__, data.data());
   QString base64EncodeData = data.toBase64();
   QString method = QString("qtCallJsSocketIoSendMsg('%1')").arg(base64EncodeData);
   return method;
}

bool VhallIALive::IsEnalbePlayMedia() {
   //自己是主讲人，且身份不是观众。
   if (mActiveParam.mainShow.compare(mActiveParam.join_uid) == 0 && mActiveParam.role_name.compare(USER_USER) != 0) {
      return true;
   }
   //当自己是主持人，并且当前共享的角色是普通用户，此时主持人有此权限。
   else if (mActiveParam.role_name.compare(USER_HOST) == 0 && mActiveParam.mainShow.compare(mActiveParam.join_uid) != 0 &&
      mActiveParam.mainShowRole.compare(USER_USER) == 0) {
      return true;
   }
   return false;
}

void VhallIALive::slot_OnPlayFileClicked() {
   if (!IsEnableUsing()) {
      return;
   }
   if (!GetWebRtcSDKInstance()->IsWebRtcRoomConnected()) {
      FadeOutTip(DISABLE_PLAY_FILE, TipsType_Error);
      return;
   }
   QJsonObject body;
   SingletonMainUIIns::Instance().reportLog(STR_VH_LIVE_OPERATE, eLogRePortK_MultiMedia_InsertVedio, L"InsertVedio", body);
   int enableDeskTopCaptrue = ConfigSetting::ReadInt(CPathManager::GetConfigPath(), GROUP_DEFAULT, PUSH_DESKTOP_CAPTURE_OPEN, 0);
   if (enableDeskTopCaptrue == 1) {
      QString msg = RTC_PLAY_FILE_NOTICE;
      AlertTipsDlg tip(msg, true, this);
      tip.CenterWindow(this);
      tip.SetYesBtnText(CONFIRM_CLOSE);
      tip.SetNoBtnText(NOT_CLOSE);
      if (tip.exec() == QDialog::Accepted) {
         TRACE6("%s Stop Local CapturePlayer mCurrentMicIndex:%d\n", __FUNCTION__, mCurrentMicIndex);
         ConfigSetting::writeValue(CPathManager::GetConfigPath(), GROUP_DEFAULT, PUSH_DESKTOP_CAPTURE_OPEN, 0);
         GetWebRtcSDKInstance()->StartLocalCapturePlayer(L"", 100);
         vlive::GetWebRtcSDKInstance()->MuteAllSubScribeAudio(false);
      }
   }

   //只有主讲人才能进行此操作。
   if (IsEnalbePlayMedia()) {
      STRU_MAINUI_CLICK_CONTROL loControl;
      loControl.m_eType = (enum_control_type)(control_VideoSrc);
      loControl.m_dwExtraData = eLiveType_VhallActive;
      SingletonMainUIIns::Instance().PostCRMessage(MSG_MAINUI_CLICK_CONTROL, &loControl, sizeof(STRU_MAINUI_CLICK_CONTROL));
   }
   else {
      FadeOutTip(NOT_MAIN_SPEAKER, TipsType_Error);
   }
}

void VhallIALive::SetVedioPlayUi(QWidget* pVedioPlayUI) {
   mpVedioPlayUi = pVedioPlayUI;
   if (mpVedioPlayUi) {
      ResetMediaCtrlWdgPos();
   }
}

bool VhallIALive::GetIsLiving(){
   return mActiveParam.bIsLiving;
}

QString VhallIALive::GetHostId(){
   return mActiveParam.role_name.compare(USER_HOST) == 0 ? mActiveParam.join_uid : mActiveParam.hostJoinUid;
}

void VhallIALive::OpenScreenShare(){
   int cur_enhance = ConfigSetting::ReadInt(CPathManager::GetConfigPath(), GROUP_DEFAULT, DESKTOP_ENHANCE, 0);
   GetWebRtcSDKInstance()->SetDesktopEdgeEnhance(cur_enhance <= 0 ? false : true);
   if (cur_enhance > 0) {
      QJsonObject body;
      SingletonMainUIIns::Instance().reportLog(STR_VH_LIVE_OPERATE, eLogRePortK_Desktop_Hance, L"desktop_enhance", body);
      TRACE6("%s DesktopEnhanceControlSwitch \n", __FUNCTION__);
   }
   mbIsDeskTop = true;
   this->hide();
   QRect rect = QApplication::desktop()->availableGeometry(this);
   if (m_pScreenShareToolWgd) {
      m_pScreenShareToolWgd->SetLiveShow(mActiveParam.role_name.compare(USER_HOST) == 0);
      m_pScreenShareToolWgd->Show();
      VHD_WindowInfo win_info = m_pScreenShareToolWgd->GetCurrentShareWindowInfo();
      if (mDesktopCameraRenderWdgPtr && mDesktopCameraRenderWdgPtr->isHidden()) {
         bool isOpenCamera = false;
         VhallRenderWdg* render = GetRenderWnd(mActiveParam.join_uid);
         if (render) {
            isOpenCamera = render->GetUserCameraState();
         }

         QRect screen_rect;
         QDesktopWidget* desktopWidget = QApplication::desktop();
         if (desktopWidget) {
            QList<QScreen *> screenlist = QGuiApplication::screens();
            for (int i = 0; i < screenlist.size(); i++) {
               if (QString::fromStdWString(win_info.name) == screenlist.at(i)->name()) {
                  screen_rect = screenlist.at(i)->availableGeometry();
                  break;
               }
            }
         }

         mDesktopCameraRenderWdgPtr->SetCloseCamera(!isOpenCamera);
         mDesktopCameraRenderWdgPtr->StartRender();
         mDesktopCameraRenderWdgPtr->ReSize();
         mDesktopCameraRenderWdgPtr->move(screen_rect.x() + screen_rect.width() - mDesktopCameraRenderWdgPtr->width() - 100, 100);
         mDesktopCameraRenderWdgPtr->show();
         mDesktopCameraRenderWdgPtr->ShowCloseBtn();
         if (!mActiveParam.bIsLiving) {
            mDesktopCameraRenderWdgPtr->SetCurrentIndex(2);
         }
         m_pScreenShareToolWgd->SetFloatWndBtnState(true);
      }
   }
}

void VhallIALive::CloseScreenShare(){
   mbIsDeskTop = false;
   if (mDesktopShowTipsWdgPtr) {
      mDesktopShowTipsWdgPtr->ClearAllTips();
      mDesktopShowTipsWdgPtr->hide();
   }
   m_pScreenShareToolWgd->Close();
   if (mDesktopCameraRenderWdgPtr) {
      mDesktopCameraRenderWdgPtr->hide();
      mDesktopCameraRenderWdgPtr->ReSize();
      slot_closeDesktopCameraRender();
   }
   this->show();
}

void  VhallIALive::JsCallAlert(QString url)
{
   TRACE6("%s\n", __FUNCTION__);
   AlertTipsDlg tip(url, false, NULL);
   tip.exec();
}

void VhallIALive::ReleaseMediaStream() {
   TRACE6("%s\n", __FUNCTION__);
   slot_ClickedMic(mActiveParam.join_uid, false);
   GetWebRtcSDKInstance()->StopMediaFileCapture();
   mbIsPlayMediaFile = false;
   InteractAPIManager apiManager(this);
   apiManager.HttpSendChangeWebWatchLayout(0);
}

void VhallIALive::StopPlayMediaFile() {
   ReleaseMediaStream();
   StopShowMediaVideo();
}

void VhallIALive::StopShowMediaVideo() {
   TRACE6("%s enter\n", __FUNCTION__);
   mbIsPlayMediaFile = false;
   mbIsPushMediaErr = false;
   if (mMediaFilePlayWdg) {
      if (mDocCefWidget) {
         mDocCefWidget->show();
      }
      ui.verticalLayout_mainWebView->removeWidget(mMediaFilePlayWdg);
      delete mMediaFilePlayWdg;
      mMediaFilePlayWdg = NULL;
      LAYOUT_MODE layoutMode = LAYOUT_MODE_TILED;
      if (mpSettingDlg) {
         layoutMode = mpSettingDlg->GetLayOutMode();
      }
      mCurrentMode = GetCurrentLayoutMode(layoutMode);
      if (mActiveParam.role_name == USER_HOST) {
         SetLayOutMode(mCurrentMode);
      }
  
   }
   TRACE6("%s leaveHandleConfigMediaFileMainView\n", __FUNCTION__);
}

int VhallIALive::GetMediaPlayUIWidth() {
   return ui.widget_bottomTool->width() - ui.widget_leftTool->width();
}

int VhallIALive::GetPlayFileState() {
   int nRet = -1;
   if (GetWebRtcSDKInstance()) {
      nRet = GetWebRtcSDKInstance()->MediaGetPlayerState();
      if (nRet == 7) {
         QCoreApplication::postEvent(mpMainUILogic, new QEvent(CustomEvent_GetLocalFileStreamErr));
      }
   }
   return nRet;
}

int  VhallIALive::GetPlayMediaFilePos(signed long long& n64Pos, signed long long& n64MaxPos) {
   if (GetWebRtcSDKInstance()) {
      n64Pos = GetWebRtcSDKInstance()->MediaFileGetCurrentDuration();
      n64MaxPos = GetWebRtcSDKInstance()->MediaFileGetMaxDuration();
   }
   return 0;
}

int VhallIALive::OpenPlayMediaFile(const char *szMediaFile) {
   return InitMediaStream(szMediaFile);
}

int VhallIALive::PausePlayMediaFile() {
   if (GetWebRtcSDKInstance()) {
      GetWebRtcSDKInstance()->MediaFilePause();
   }
   return 0;
}

int VhallIALive::ResumePlayMediaFile() {
   if (GetWebRtcSDKInstance()) {
      GetWebRtcSDKInstance()->MediaFileResume();
   }
   return 0;
}

int VhallIALive::SetPlayMediaFilePos(const signed long long& n64Pos) {
   if (GetWebRtcSDKInstance()) {
      GetWebRtcSDKInstance()->MediaFileSeek(n64Pos);
   }
   return 0;
}

void VhallIALive::SetPlayMeidaFileVolume(const int volume) {
   TRACE6("%s SetPlayMeidaFileVolume:%d\n", __FUNCTION__, volume);
   if (GetWebRtcSDKInstance()) {
      GetWebRtcSDKInstance()->MediaFileVolumeChange(volume);
   }
   return;
}


void VhallIALive::ShowPlayUIState(bool show) {
   VH::CComPtr<IVedioPlayLogic> pVedioPlayLogic = NULL;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IMainUI, IID_IVedioPlayLogic, pVedioPlayLogic, return);
   if (pVedioPlayLogic) {
      pVedioPlayLogic->ReposVedioPlay();
      pVedioPlayLogic->ShowPlayUI(show);
   }
}

void VhallIALive::slot_HidePlayUI() {
   ShowPlayUIState(false);
}

bool VhallIALive::eventFilter(QObject *obj, QEvent *e) {
   if (mMediaFilePlayWdg && mMediaFilePlayWdg == obj) {
      if (e->type() == QEvent::Enter) {
         ShowPlayUIState(true);
      }
      else if (e->type() == QEvent::Leave) {
         if (mPlayUIHideTimer) {
            mPlayUIHideTimer->start(2000);
         }
      }
   }
   if (mDocCefWidget && mDocCefWidget == obj) {
      if (e->type() == QEvent::Leave) {
         setCursor(Qt::ArrowCursor);
      }
   }

   if (mTeachingWidget && mTeachingWidget == obj && e->type() == QEvent::MouseButtonPress) {
      if (mTeachingPageIndex == 0) {
         mTeachingWidget->setStyleSheet("border-image: url(:/teaching/img/teaching/rtc_beauty.png);");
         mTeachingWidget->repaint();
      }
      else {
         mTeachingWidget->hide();
         mIsShowTeachingWidget = false;
         ShowCheckDev();
      }
      mTeachingPageIndex++;
   }
   return QWidget::eventFilter(obj, e);
}

void VhallIALive::ShowCheckDev() {
   if (mActiveParam.role_name == USER_HOST && !mActiveParam.bIsLiving && !mIsShowTeachingWidget) {
      QApplication::postEvent(this, new QEvent(CustomEvent_DEV_CHECK));
   }
   else if(!mbIsGuestSpeaking && !mIsShowTeachingWidget && mActiveParam.role_name == USER_GUEST){
      QApplication::postEvent(this, new QEvent(CustomEvent_DEV_CHECK));
   }
}

void VhallIALive::HandleMsgReConnect() {
   TRACE6("%s \n", __FUNCTION__);
   GetSpeakUserList();
   TRACE6("%s end\n", __FUNCTION__);
}

void VhallIALive::HandleSubScribedError(QEvent *event) {
   TRACE6("%s \n", __FUNCTION__);
   CustomRecvMsgEvent* msgEvent = dynamic_cast<CustomRecvMsgEvent*>(event);
   if (msgEvent) {
      TRACE6("%s\n", __FUNCTION__);
      if (msgEvent->mode == 50001) {
         //ssrc 统计异常的订阅。SDK已经移除了订阅的流处理。
         if (!msgEvent->mJoinUserId.isEmpty()) {
            TRACE6("%s mJoinUserId %s\n", __FUNCTION__, msgEvent->mJoinUserId.toStdString().c_str());
            if (ui.tabWidget->mpMemberListDlg) {
               QString name = ui.tabWidget->mpMemberListDlg->GetUserNickName(msgEvent->mJoinUserId);
               if (!name.isEmpty()) {
                  VhallRenderWdg* render = GetRenderWnd(msgEvent->mJoinUserId);
                  if (render) {
                     render->ShowVideoLoading(true);
                  }
               }
            }
         }
      }
      else if (msgEvent->mode == 1) {
         //stream_reconnect 订阅流
         if (!msgEvent->mJoinUserId.isEmpty()) {
            TRACE6("%s mJoinUserId %s\n", __FUNCTION__, msgEvent->mJoinUserId.toStdString().c_str());
            if (ui.tabWidget->mpMemberListDlg) {
               QString name = ui.tabWidget->mpMemberListDlg->GetUserNickName(msgEvent->mJoinUserId);
               if (!name.isEmpty()) {
                  QString tips = name + SUB_REMOTE_STREAM_ERR;
                  VhallRenderWdg* render = GetRenderWnd(msgEvent->mJoinUserId);
                  if (render) {
                     render->ShowVideoLoading(true);
                     if (render->GetSubStreamErrorCount() < 3) {
                        FadeOutTip(tips, TipsType_Error);
                     }
                     render->SetSubStreamError();
                  }
               }
            }
         }
      }
      TRACE6("%s SubScribeRemoteStream %s\n", __FUNCTION__, msgEvent->mStreamId.toStdString().c_str());
      vlive::GetWebRtcSDKInstance()->SubScribeRemoteStream(msgEvent->mStreamId.toStdString(), 3000);
   }
}

void VhallIALive::HandleRecord(QEvent* event) {
   if (CustomEvent_RecordEvent == event->type()){
      RecordEvent* cuEvent = dynamic_cast<RecordEvent*>(event);
      ui.widget_title->RecordStateSucce();
      return;
   }
   else if (CustomEvent_RecordEventErr == event->type()){
      RecordEventErr* cuEvent = dynamic_cast<RecordEventErr*>(event);
      FadeOutTip(cuEvent->msg, TipsType_Error);
   }
}

void VhallIALive::customEvent(QEvent *event) {
   if (event) {
      int typeValue = event->type();
      CustomOnHttpResMsgEvent *customEvent = dynamic_cast<CustomOnHttpResMsgEvent*>(event);
      switch (typeValue) {
         case CustomEvent_OnRoomFailedEvent: {
            TRACE6("%s CustomEvent_OnRoomFailedEvent\n", __FUNCTION__);
            HandlePaasSDKFailedEvent(customEvent->mCode, customEvent->mMsg.toStdString(), customEvent->mEventCode);
            TRACE6("%s CustomEvent_OnRoomFailedEvent end\n", __FUNCTION__);
            break;
         }
         case CustomEvent_CopyShareUrl: {
            FadeOutTip(COPY_SUCCESS, TipsType_Success);
            break;
         }
         case CustomEvent_OnRoomSuccessedEvent: {
            TRACE6("%s CustomEvent_OnRoomSuccessedEvent\n", __FUNCTION__);
            HandlePaasSDKConnectSuc(customEvent->mMsg.toStdString(), customEvent->mEventCode);
            TRACE6("%s CustomEvent_OnRoomSuccessedEvent end\n", __FUNCTION__);
            break;
         }
         case CustomEvent_VSSGetRoomBaseInfo: {
            TRACE6("%s CustomEvent_VSSGetRoomBaseInfo\n", __FUNCTION__);
            HandleVSSGetRoomBaseInfo(customEvent->mCode, customEvent->mMsg.toStdString());
            TRACE6("%s CustomEvent_VSSGetRoomBaseInfo end\n", __FUNCTION__);
            break;
         }
         case CustomEvent_DEV_CHECK: {
            HandleDevCheck();
            break;
         }
         case CustomEvent_ReSizeCenterRender: {
            if (!IsMainRenderNormalPos()) {
               for (int i = 0; i < ui.listWidgetMainView->count(); i++) {
                  QListWidgetItem *item = ui.listWidgetMainView->item(i);
                  if (item) {
                     VhallRenderWdg* itemRender = dynamic_cast<VhallRenderWdg*>(ui.listWidgetMainView->itemWidget(item));
                     if (itemRender) {
                        itemRender->SwitchCenterSize(ui.widget_5->width(), ui.widget_5->height());
                     }
                  }
               }
            }
            break;
         }
         case CustomEvent_VSSStartLiveResp: {
            TRACE6("%s CustomEvent_VSSStartLiveResp \n", __FUNCTION__);
            HandleVSSStartLive(customEvent->mCode, customEvent->mMsg.toStdString());
            TRACE6("%s CustomEvent_VSSStartLiveResp end\n", __FUNCTION__);
            break;
         }
         case CustomEvent_VSSGetRoomSpeakList: {
            TRACE6("%s CustomEvent_VSSGetRoomSpeakList \n", __FUNCTION__);
            HandleHttpResponseSpeakList("", customEvent->mMsg, customEvent->mCode, "", false, "", false);
            TRACE6("%s CustomEvent_VSSGetRoomSpeakList end\n", __FUNCTION__);
            break;
         }
         case CustomEvent_VSSStopLive: {
            TRACE6("%s CustomEvent_VSSStopLive \n", __FUNCTION__);
            HandleStopLive(customEvent->mCode, customEvent->mMsg.toStdString());
            TRACE6("%s CustomEvent_VSSStopLive end\n", __FUNCTION__);
            break;
         }
         case CustomEvent_VSSSetStream: {
            TRACE6("%s CustomEvent_VSSSetStream \n", __FUNCTION__);
            HandleVSSSetStream(customEvent->mCode, customEvent->mMsg.toStdString(), customEvent->mLayout.toStdString(), customEvent->mVideoProfile.toStdString());
            TRACE6("%s CustomEvent_VSSSetStream end\n", __FUNCTION__);
            break;
         }
         case CustomEvent_VSSSendInvite: {
            TRACE6("%s CustomEvent_VSSSetStream \n", __FUNCTION__);
            HandleVSSSendInvite(customEvent->mCode, customEvent->mMsg.toStdString(), customEvent->mRecvUserId.toStdString(), customEvent->mUid.toStdString());
            break;
         }
         case CustomEvent_VSSSetHandsUp: {
            HandleSetHandlsUpMsg(customEvent);
            break;
         }
         case CustomEvent_VSSRejectInviteResp: {
            HandleRejectInviteResp(customEvent->mCode, customEvent->mMsg.toStdString(), customEvent->mRecvUserId.toStdString(), customEvent->mUid.toStdString());
            break;
         }
         case CustomEvent_VSSAgreeApply: {
            HandleVSSAgreeApply(customEvent->mCode, customEvent->mMsg.toStdString(), customEvent->mRecvUserId.toStdString(), customEvent->mUid.toStdString());
            break;
         }
         case CustomEvent_StopCaptureMediaFile: {
            SingletonMainUIIns::Instance().PostCRMessage(MSG_MAINUI_VEDIOPLAY_STOPPLAY, NULL, 0);
            break;
         }
         case CustomEvent_VSSRejectApply: {
            HandleVSSRejectApply(customEvent->mCode, customEvent->mMsg.toStdString(), customEvent->mRecvUserId.toStdString(), customEvent->mUid.toStdString());
            break;
         }
         case CustomEvent_VSSSetMainScreenResp: {
            HandleVSSSetMainScreenResp(customEvent->mCode, customEvent->mMsg.toStdString(), customEvent->mRecvUserId.toStdString(), customEvent->mUid.toStdString(), customEvent->mRoleName);
            break;
         }
         case CustomEvent_VSSNoSpeakResp: {
            HandleVSSNoSpeakResp(customEvent->mCode, customEvent->mMsg.toStdString(), customEvent->mRecvUserId.toStdString(), customEvent->mbNoticeMsg);
            break;
         }
         case CustomEvent_VSSSpeak: {
            HandleVSSChangeToSSpeakResp(customEvent->mCode, customEvent->mMsg.toStdString(), customEvent->mUid.toStdString());
            break;
         }
         case CustomEvent_VSSApply: {
            HandleVSSHandsUpRespParam(customEvent->mCode, customEvent->mMsg.toStdString(), "");
            break;
         }
         case CustomEvent_VSSCancleApply: {
            HandleVSSCancelHandsUpRespParam(customEvent->mCode, customEvent->mMsg.toStdString(), "");
            break;
         }
         case CustomEvent_VSSAgreeInvite: {
            HandleVSSAgreeInvite(customEvent->mCode, customEvent->mMsg.toStdString(), customEvent->mRecvUserId.toStdString(), customEvent->mIsAgree);
            break;
         }
         case CustomEvent_ChatMsg: {
            HandleChatMsg(customEvent);
            break;
         }
         case CustomEvent_LibCefMsg: {
            CustomRecvMsgEvent* cus_event = dynamic_cast<CustomRecvMsgEvent*>(event);
            if (cus_event) {
               JsCallQtMsg(cus_event->msg);
            }
            break;
         }
      }
   }
}

void VhallIALive::OnRecvMsg(std::string fun_name, std::string cus_msg) {
   CustomRecvMsgEvent* event = new CustomRecvMsgEvent(CustomEvent_LibCefMsg, QString::fromStdString(cus_msg));
   QApplication::postEvent(this, event);
}

void VhallIALive::OnWebViewDestoryed() {

}

void VhallIALive::OnWebViewLoadEnd() {
   VH::CComPtr<ICommonData> pCommonData;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
   ClientApiInitResp initResp;
   pCommonData->GetInitApiResp(initResp);
   if (mDocLoadingTimer) {
      mDocLoadingTimer->stop();
   }
   if (mDocLoadEndTimer) {
      mDocLoadEndTimer->start(2000);
   }
   
   ui.widget_docLoadingTips->hide();
   if (!mActiveParam.bIsLiving && initResp.player == H5_LIVE_CREATE_TYPE) {
      ui.widget_leftTool->slot_OnClickedDoc();
   }
}

void VhallIALive::OnTitleChanged(const int id, std::string title_name) {
   VH::CComPtr<ICommonData> pCommonData;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
   ClientApiInitResp initResp;
   pCommonData->GetInitApiResp(initResp);
   QString title = QString::fromStdString(title_name);
   if (initResp.player == FLASH_LIVE_CREATE_TYPE && (title.contains("flash", Qt::CaseInsensitive) || title.contains("player", Qt::CaseInsensitive))) {
      if (mDocCefWidget && !mDocCefWidget->isHidden()) {
         mDocCefWidget->LoadUrl(mActiveParam.pluginUrl);
      }
   }
}

void VhallIALive::HttpSendStartLive() {
   InteractAPIManager apiManager(mpMainUILogic);
   apiManager.HttpSendStartLive(eLiveType_VhallActive);
}

void VhallIALive::HttpSendStopLive() {
   InteractAPIManager apiManager(mpMainUILogic);
   apiManager.HttpSendStopLive(eLiveType_VhallActive);
}

//下麦
void VhallIALive::HttpSendNotSpeak(QString uid, bool showNotice) {
   InteractAPIManager apiManager(mpMainUILogic);
   apiManager.HttpSendNotSpeak(uid, showNotice);
}

//上麦
void VhallIALive::LocalUserToSpeak() {
   InteractAPIManager apiManager(mpMainUILogic);
   apiManager.HttpSendSpeak();
}

//邀请上麦
void VhallIALive::HttpSendInvite(QString uid, bool showNotice) {
   InteractAPIManager apiManager(mpMainUILogic);
   apiManager.HttpSendInvite(uid, showNotice);
}

//同意邀请上麦
void VhallIALive::slot_GuestAgreeHostInvite() {
   if (vlive::GetWebRtcSDKInstance()->IsWebRtcRoomConnected()) {
      InteractAPIManager apiManager(mpMainUILogic);
      apiManager.HttpSendAgreeInvite();
   }
   else {
      FadeOutTip(RTC_CONNECT_ERR_GUEST_RETRY, TipsType_Error);
      InteractAPIManager apiManager(mpMainUILogic);
      apiManager.HttpSendRejectInvite();
   }
   if (mpInviteJoinWdg) {
      mpInviteJoinWdg->accept();
   }
}

//拒绝邀请上麦  
void VhallIALive::slot_GuestRefuseHostInvite() {
   InteractAPIManager apiManager(mpMainUILogic);
   apiManager.HttpSendRejectInvite();
   if (mpInviteJoinWdg) {
      mpInviteJoinWdg->accept();
   }
}

//申请上麦
void VhallIALive::ApplyToSpeak() {
   InteractAPIManager apiManager(mpMainUILogic);
   apiManager.HttpSendApplyToSpeak();
}

//主持人同意上麦
void VhallIALive::slotAgreeUpper(const QString& strId) {
   InteractAPIManager apiManager(mpMainUILogic);
   apiManager.HttpSendAgreeToSpeak(strId);
}

//主持人拒绝用户上麦
void VhallIALive::slot_RejectSpeak(const QString& strId) {
   InteractAPIManager apiManager(mpMainUILogic);
   apiManager.HttpSendRejectSpeak(strId);
}

//允许举手开关
void VhallIALive::slot_AllowRaiseHands(const bool& checked) {
   InteractAPIManager apiManager(mpMainUILogic);
   apiManager.HttpSendAllowRaiseHands(checked);
}

//切换主讲人
void VhallIALive::HttpSendSwitchSpeaker(QString receive_uid, QString role, int enableNotice) {
   InteractAPIManager apiManager(mpMainUILogic);
   apiManager.HttpSendSwitchSpeaker(receive_uid, role, enableNotice);
}

//设备操作
void VhallIALive::HttpSendDeviceStatus(QString receiveJoinId, int device, int type) {
   InteractAPIManager apiManager(mpMainUILogic);
   apiManager.HttpSendDeviceStatus(receiveJoinId, device, type);
   if (mDesktopCameraRenderWdgPtr && device == 2) {
      mDesktopCameraRenderWdgPtr->SetCloseCamera(type == 0 ? true : false);
   }
}

void VhallIALive::HttpSendMixLayOutAndProfile(QString strLayout, QString videoProfile) {
   InteractAPIManager apiManager(mpMainUILogic);
   apiManager.HttpSendMixLayOutAndProfile(strLayout, videoProfile);
}

void VhallIALive::GetSpeakUserList() {
   InteractAPIManager apiManager(mpMainUILogic);
   apiManager.HttpSendGetSpeakUserList();
}

void VhallIALive::ResetToNospeakState() {
   if (mpSettingDlg) {
      mpSettingDlg->SetStartLiving(false);
   }
   vlive::GetWebRtcSDKInstance()->StopLocalCapture();
   mbIsMicOpen = false;
   mbIsCameraOpen = false;
   mbIsPlayerOpen = false;
   if (NULL != m_pScreenShareToolWgd) {
      m_pScreenShareToolWgd->SetMicMute(true);
      m_pScreenShareToolWgd->SetCameraState(false);
      if (mDesktopCameraRenderWdgPtr) {
         mDesktopCameraRenderWdgPtr->SetCloseCamera(true);
      }
      m_pScreenShareToolWgd->SetPlayerState(false);
   }
   mpSettingDlg->OnOpenPlayer(false);
   mCurCameraDevID.clear();
   TRACE6("%s mCurCameraDevID.clear()\n", __FUNCTION__);
}

bool VhallIALive::IsEnableUsing() {
   if (mActiveParam.role_name.compare(USER_HOST) != 0 && !mActiveParam.bIsLiving) {
      FadeOutTip(LIVE_NOT_START, TipsType_Error);
      return false;
   }
   return true;
}

//嘉宾自己进行，上麦/下麦操作
void VhallIALive::slot_OnToSpeakClicked() {
   TRACE6("%s enter\n", __FUNCTION__);
   if (!GetWebRtcSDKInstance()->HasAudioDev() && !GetWebRtcSDKInstance()->HasVideoDev() && !ui.widget_title->GetToSpeakBtnState()) {
      FadeOutTip(CHECK_DEVICE_ERR, TipsType_Error);
      ui.widget_title->stopHandsUpCountDown();
      TRACE6("%s HasVideoOrAudioDev\n", __FUNCTION__);
      return;
   }

   if (IsEnableUsing()) {
      if (ui.widget_title->GetToSpeakBtnState()) {	 	//下麦操作
         HttpSendNotSpeak(mActiveParam.join_uid, false);
      }
      else {
         //申请上麦举手
         if (!bProhibit){ //判断自己没有被禁言
            if (vlive::GetWebRtcSDKInstance()->IsWebRtcRoomConnected()) {
               ui.widget_title->startToSpeakCoutDown();
               ApplyToSpeak();
            }
            else {
               ui.widget_title->stopHandsUpCountDown();
               FadeOutTip(RTC_CONNECT_ERR_GUEST_RETRY, TipsType_Error);
            }
         }
      }
   }
   TRACE6("%s leave\n", __FUNCTION__);
}

void VhallIALive::slot_CancleApplyToSpeak(){
   TRACE6("%s enter\n", __FUNCTION__);
   InteractAPIManager apiManager(mpMainUILogic);
   apiManager.HttpSendCancelApplyToSpeak();
   TRACE6("%s leave\n", __FUNCTION__);
}

void  VhallIALive::FadeOutTip(QString str, int type, int showTime /*= 4000*/) {
   if (this->isHidden() && mDesktopShowTipsWdgPtr && m_pScreenShareToolWgd && !m_pScreenShareToolWgd->isHidden()) {
      mDesktopShowTipsWdgPtr->setFixedWidth(this->width() - ui.widget_leftTool->width() - ui.tabWidget->width());
      mDesktopShowTipsWdgPtr->AddTips(str, this->width() - ui.widget_leftTool->width() - ui.tabWidget->width(), showTime, (TipsType)type);
      if (!isMinimized()) {
         mDesktopShowTipsWdgPtr->show();
      }
      QRect rect = QApplication::desktop()->availableGeometry(m_pScreenShareToolWgd);
      mDesktopShowTipsWdgPtr->move(rect.x() + (rect.width() - mDesktopShowTipsWdgPtr->width()) / 2,200);
   }
   else if (mShowTipsWdgPtr) {
      mShowTipsWdgPtr->setFixedWidth(this->width() - ui.widget_leftTool->width() - ui.tabWidget->width());
      mShowTipsWdgPtr->AddTips(str, this->width() - ui.widget_leftTool->width() - ui.tabWidget->width(), showTime, (TipsType)type);
      if (!isMinimized()) {
         mShowTipsWdgPtr->show();
      }
      slot_ReSetTipsPos();
   }
}

void VhallIALive::slot_ReSetTipsPos() {
   if (mShowTipsWdgPtr) {
      QPoint pos = ui.widget_4->mapToGlobal(QPoint(0, 0));//ui.stackedWidget->pos();
      mShowTipsWdgPtr->move(pos);
   }
}

void  VhallIALive::ExitRoom(int reason) {
   vlive::GetWebRtcSDKInstance()->StopLocalCapture();
   mbExitRoom = true;
   TRACE6("%s ------------ \n", __FUNCTION__);
   if (mpVedioPlayUi) {
      mpVedioPlayUi = NULL;
   }
   if (mpExitWaiting) {
      mpExitWaiting->Show();
   }
   if (NULL != ui.tabWidget->mpMemberListDlg) {
      ui.tabWidget->mpMemberListDlg->clearData();
   }
   RemoveAllRender();
   TRACE6("%s enter\n", __FUNCTION__);

   TRACE6("%s start DisConnect\n", __FUNCTION__);
   if (GetWebRtcSDKInstance()) {
      GetWebRtcSDKInstance()->DisConnetWebRtcRoom();
      mbCallRoomDisConnect = true;
   }
   TRACE6("%s end DisConnect\n", __FUNCTION__);
   hide();
   if (mpExitWaiting) {
      mpExitWaiting->Close();
   }

   if (reason == EXIT_KICKOUT) {
      AlertTipsDlg tip(KICK_OUT_LIVE, false, NULL);
      QCoreApplication::postEvent(mpMainUILogic, new CustomDeleteLiveMsgEvent(CustomEvent_DeleteVhallLive, mActiveParam.bExitToLivelist, reason));
      tip.exec();
   }
   else {
      QCoreApplication::postEvent(mpMainUILogic, new CustomDeleteLiveMsgEvent(CustomEvent_DeleteVhallLive, mActiveParam.bExitToLivelist, reason));
   }
   TRACE6("%s leave\n", __FUNCTION__);
}

void VhallIALive::RemoveAllRender() {
   TRACE6("%s enter\n", __FUNCTION__);
   GetWebRtcSDKInstance()->StopPushLocalStream();
   GetWebRtcSDKInstance()->StopRecvAllRemoteStream();
   ui.listWidgetMainView->hide();
   while (ui.horizontalLayout_smallRender->count() > 0) {
      ui.horizontalLayout_smallRender->takeAt(0);
   }

   while (ui.listWidgetMainView->count() > 0) {
      TRACE6("%s ui.listWidgetMainView->count():%d\n", __FUNCTION__, ui.listWidgetMainView->count());
      QListWidgetItem *deleteItem = ui.listWidgetMainView->takeItem(0);
      delete deleteItem;
   }

   std::map<QString, VhallRenderWdg*>::iterator iter = mRenderMap.begin();
   while (iter != mRenderMap.end()) {
      if (iter->second != NULL) {
         QString id = iter->second->GetUserID();
         TRACE6("%s delete enter id:%s\n", __FUNCTION__, id.toStdString().c_str());
         VhallRenderWdg* tmp = iter->second;
         disconnect(tmp, &VhallRenderWdg::sig_setInMainView, this, &VhallIALive::slot_setInMainView);
         disconnect(tmp, &VhallRenderWdg::sig_setToSpeaker, this, &VhallIALive::slot_setToSpeaker);
         disconnect(tmp, &VhallRenderWdg::sig_ClickedCamera, this, &VhallIALive::slot_ClickedCamera);
         disconnect(tmp, &VhallRenderWdg::sig_ClickedMic, this, &VhallIALive::slot_ClickedMic);
         disconnect(tmp, &VhallRenderWdg::sig_NotToSpeak, this, &VhallIALive::slot_ClickedNotSpeak);
         disconnect(tmp, &VhallRenderWdg::sig_SwitchView, this, &VhallIALive::slot_SwitchMainViewLayout);
         delete tmp;
         tmp = NULL;
         mRenderMap.erase(iter);
         iter = mRenderMap.begin();
      }
   }
   ui.listWidgetMainView->clear();
   ReSizeMediaCtrlWdg();
   ResetListWidgetSmallViewSize();
   TRACE6("%s  end\n", __FUNCTION__);
}

void VhallIALive::ChangeLocalMicDevState(bool open, const QString& uid) {
   TRACE6("%s open:%d\n", __FUNCTION__, open);
   mbIsMicOpen = open;
   if (GetWebRtcSDKInstance()) {
      if (open) {
         GetWebRtcSDKInstance()->OpenMic(); //设置开启状态，则本地应该为非静音
         FadeOutTip(MIC_ALREADY_OPEN, TipsType_MIC_NOTICE,2000);
      }
      else {
         GetWebRtcSDKInstance()->CloseMic(); //设置开启状态，则本地应该为非静音
         FadeOutTip(MIC_ALREADY_CLOSE, TipsType_MIC_NOTICE, 2000);
      }
   }
   if (NULL != m_pScreenShareToolWgd) {
      m_pScreenShareToolWgd->SetMicMute(!mbIsMicOpen);
   }
   VhallRenderWdg* render = GetRenderWnd(uid);
   if (render) {
      render->SetUserMicState(mbIsMicOpen);
   }
}

void VhallIALive::ChangeLocalCameraDevState(bool open, const QString & uid) {
   TRACE6("%s open:%d\n", __FUNCTION__, open);
   mbIsCameraOpen = open;
   if (GetWebRtcSDKInstance()) {
      if (open) {
         GetWebRtcSDKInstance()->OpenCamera(); //设置开启状态，则本地应该为非静音
         FadeOutTip(CAMERA_ALREADY_OPEN, TipsType_MIC_NOTICE, 2000);
      }
      else {
         GetWebRtcSDKInstance()->CloseCamera(); //设置开启状态，则本地应该为非静音
         FadeOutTip(CAMERA_ALREADY_CLOSE, TipsType_MIC_NOTICE, 2000);
      }
   }
   if (NULL != m_pScreenShareToolWgd) {
      m_pScreenShareToolWgd->SetCameraState(mbIsCameraOpen);
   }
   if (mDesktopCameraRenderWdgPtr) {
      mDesktopCameraRenderWdgPtr->SetCloseCamera(!mbIsCameraOpen);
   }

   VhallRenderWdg* render = GetRenderWnd(uid);
   if (render) {
      render->SetUserCameraState(mbIsCameraOpen);
      if (mbIsCameraOpen) {
         render->SetEnableUpdateRendWnd(false);
      }
      else {
         render->SetEnableUpdateRendWnd(true);
      }
   }
}

void VhallIALive::RecvHostInviteMsg() {
   if (mbIsDeviceChecking) {
      if (mpInviteJoinWdg) {
         mpInviteJoinWdg->StartTimer();
      }
   }
   else {
      if (mpInviteJoinWdg) {
         ConfigSetting::MoveWindow(mpInviteJoinWdg, this->isVisible() ? this : nullptr);
         mpInviteJoinWdg->exec();
      }
   }
}

QString VhallIALive::GetStreamName() {
   return mActiveParam.streamName.trimmed();
}

QString VhallIALive::GetMainViewID() { 
   return mActiveParam.mainShow;
}

QString VhallIALive::GetMainSpeakerID() {
   return mActiveParam.currPresenter;
}

QString VhallIALive::GetMainViewRole() { 
   return mActiveParam.mainShowRole; 
}

bool VhallIALive::IsInteractiveHandsUp() {
   return mActiveParam.handsUp == 1 ? true : false;
}

void VhallIALive::slot_OnClickedDoc() {
   VH::CComPtr<ICommonData> pCommonData;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
   ClientApiInitResp respInitData;
   pCommonData->GetInitApiResp(respInitData);
   if (respInitData.player != FLASH_LIVE_CREATE_TYPE) {  //flash活动
      if (!IsEnalbePlayMedia()) {
         FadeOutTip(NOT_MAIN_SPEAKER, TipsType_Error);
         return;
      }
      if (mDocCefWidget) {
         QString funPage = QString("QtCallFunctionPage(%1)").arg(eDocCtrlType_Doc);
         mDocCefWidget->AppCallJsFunction(funPage);
      }
   }

   if (!IsMainRenderNormalPos()) {
      slot_SwitchMainViewLayout();
   }
}

void VhallIALive::slot_ClickedWhiteboard() {
   if (!IsEnalbePlayMedia()) {
      FadeOutTip(NOT_MAIN_SPEAKER, TipsType_Error);
      return;
   }
   if (mDocCefWidget) {
      QString funPage = QString("QtCallFunctionPage(%1)").arg(eDocCtrlType_Broad);
      mDocCefWidget->AppCallJsFunction(funPage);
   }
   if (!IsMainRenderNormalPos()) {
      slot_SwitchMainViewLayout();
   }
}

void VhallIALive::slot_OnClickedPluginUrl(const int& iX, const int& iy) {
   STRU_MAINUI_CLICK_CONTROL loControl;
   loControl.m_eType = control_LiveTool;
   loControl.m_globalX = iX;
   loControl.m_globalY = iy;
   loControl.m_bIsReLoad = mbIsFirstLoad ? true : false;
   mbIsFirstLoad = false;
   SingletonMainUIIns::Instance().PostCRMessage(MSG_MAINUI_CLICK_CONTROL, &loControl, sizeof(STRU_MAINUI_CLICK_CONTROL));
}

void VhallIALive::slot_DocLoadEnd() {
   if (!IsMainRenderNormalPos()) {
      if (mDocCefWidget) {
         QString funPage = QString("QtCallJsChangeDocTool(%1)").arg(eDocToolCtrlType_CloseTool);
         mDocCefWidget->AppCallJsFunction(funPage);
      }
   }
   //if (mActiveParam.bIsLiving) {
   //   if (ui.widget_leftTool->IsClickedWB()) {
   //      if (mDocCefWidget) {
   //         QString funPage = QString("QtCallFunctionPage(%1)").arg(eDocCtrlType_Broad);
   //         mDocCefWidget->AppCallJsFunction(funPage);
   //      }
   //   }
   //   else {
   //      if (mDocCefWidget) {
   //         QString funPage = QString("QtCallFunctionPage(%1)").arg(eDocCtrlType_Doc);
   //         mDocCefWidget->AppCallJsFunction(funPage);
   //      }
   //   }
   //}
}

void VhallIALive::slot_DocLoading() {
   QString tips = LOADING1;
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
   ui.label_docLoadingTips->setText(tips);
}

void VhallIALive::slot_PressKeyEsc() {
   TRACE6("%s", __FUNCTION__);
   QString funPage = QString("QtCallFunctionPage(%1)").arg(eDocCtrlType_DocNormal_Size);
   if (mDocCefWidget) {
      mDocCefWidget->AppCallJsFunction(funPage);
   }
}