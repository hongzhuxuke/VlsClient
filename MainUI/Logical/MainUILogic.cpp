#include "StdAfx.h"
#include "MainUILogic.h"
#include "VhallLiveMainDlg.h"
#include "FullScreenLabel.h"
#include "AlertTipsDlg.h"
#include <QTimer>
#include <QApplication>
#include <QProcess>
#include <QDesktopWidget>
#include <QtNetwork/QNetworkInterface>
#include <QProcess>
#include <QCryptographicHash>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDateTime>
#include <QSettings>
#include <QDesktopServices>
#include "VhallIALive.h"
#include "httptaskworkthread.h"
#include "ConfigSetting.h"
//#include "CheckVersionDlg.h"
#include "CaptureSrcUI.h"
#include "MultiMediaUI.h"
#include "AudioSliderUI.h"
#include "Msg_OBSControl.h"
#include "IOBSControlLogic.h"
#include "Msg_CommonToolKit.h"
#include "IDeviceManager.h"
#include "IDShowPlugin.h"
#include "IDeckLinkDevice.h"
#include "ICommonData.h"
#include "UACUtility.h"
#include "vhdesktopsharingui.h"
#include "UpLoader.h"
#include "vhproxytest.h"
#include "vhallsharedqr.h"
#include "vhalljoinactivity.h"
#include "pathManager.h"
#include "pathManage.h"
#include "Logging.h"
#include "pub.Const.h"
#include "pub.Struct.h"
#include "IPluginWebServer.h"
#include "PluginDataProcessor.h"
#include "vhallloginwidget.h"
#include "pub.Struct.h"
#include "VHHttp.h"
#include <QScreen>
#include "VhallNetWorkInterface.h"
#include "VSSAPIManager.h"
#include "MemberParam.h"
#include "SocketMsgParam.h"
#include "WebRtcSDKInterface.h"
#include "VHPaasInteractionRoom.h"
#include "InteractAPIManager.h"
#include "DesktopShareSelectWdg.h"
#include "lib/rapidjson/include/rapidjson/rapidjson.h"
#include "lib/rapidjson/include/rapidjson/stringbuffer.h"
#include "lib/rapidjson/include/rapidjson/writer.h"
#include "lib/rapidjson/include/rapidjson/reader.h"
#include "lib/rapidjson/include/rapidjson/document.h"
using namespace rapidjson;
using namespace std;

#define FONT_COLOR_GREEN         "#28e56b"
#define FONT_COLOR_WHITE         "#e5e5e5"
#define FONT_COLOR_RED           "#e33836"
#define FONT_COLOR_YELLOW        "#FFA500"
#define FONT_COLOR_RED_LIMIT     2/3
#define FONT_COLOR_YELLOW_LIMIT  1/4
#define LIVE_SIGNAL_COUNT_SECONDS 5
#define SAMPLECOUNT 10
#define MAX_RETRY_TIME  2

#define webinar_award   "webinar_award"
#define survey          "survey"
#define personal_chat   "personal_chat"
#define sign_in         "sign_in"

Logger *gLogger = NULL;
#define CREATE_RECORD_TIME  30000//30秒内服务器不会生成录制文件

CPluginDataProcessor* MainUILogic::m_pPluginDataProcessor = NULL;
unsigned long long os_gettime_ns(void);



void createFile(QString filePath, QString fileName)
{
   QDir tempDir;
   //临时保存程序当前路径
   QString currentDir = tempDir.currentPath();
   //如果filePath路径不存在，创建它
   if (!tempDir.exists(filePath))
   {
      qDebug() << QObject::tr("不存在该路径") << endl;
      tempDir.mkpath(filePath);
   }
   QFile *tempFile = new QFile;
   //将程序的执行路径设置到filePath下
   tempDir.setCurrent(filePath);
   qDebug() << tempDir.currentPath();
   //检查filePath路径下是否存在文件fileName,如果停止操作。
   if (tempFile->exists(fileName))
   {
      qDebug() << QObject::tr("文件存在");
      return;
   }
   //此时，路径下没有fileName文件，使用下面代码在当前路径下创建文件
   tempFile->setFileName(fileName);
   if (!tempFile->open(QIODevice::WriteOnly | QIODevice::Text))
   {
      qDebug() << QObject::tr("打开失败");
   }
   tempFile->close();
   //将程序当前路径设置为原来的路径
   tempDir.setCurrent(currentDir);
   qDebug() << tempDir.currentPath();
}

MainUILogic::MainUILogic(void)
   : m_pLiveMainDlg(NULL)
   , m_pFullScreenLabel(NULL)
   , m_lRefCount(1)
   , m_bMuteMic(FALSE)
   , m_bMuteSpeaker(FALSE)
   , m_pCaptureSrcUI(NULL)
   , mIDisplyCutRecord(eDispalyCutRecord::eDispalyCutRecord_Hide) {
   msConfPath = CPathManager::GetConfigPath();
   gLogger = new Logger(VH_LOG_DIR L"MainUILogic.log", USER);
   memset(m_wzStreamId, 0, sizeof(m_wzStreamId));
   connect(this, SIGNAL(SigRecvAnnouncement(QString, QString)), this, SLOT(SlotRecvAnnouncement(QString, QString)));
   m_strLogReportUrl = ConfigSetting::ReadString(CPathManager::GetToolConfigPath(), GROUP_DEFAULT, KEY_LOGREPORT_URL, defaultLogReportURL);

   QDesktopWidget* desktopWidget = QApplication::desktop();
   QList<QScreen *> screenlist = QGuiApplication::screens();
   for (int i = 0; i < screenlist.size(); i++) {
      QScreen * curQScreen = screenlist.at(i);
      if (curQScreen) {
         connect(curQScreen, SIGNAL(physicalSizeChanged(const QSizeF &)), this, SLOT(slot_physicalSizeChanged(const QSizeF &)));
         connect(curQScreen, SIGNAL(physicalDotsPerInchChanged(qreal)), this, SLOT(slot_physicalDotsPerInchChanged(qreal)));
         connect(curQScreen, SIGNAL(logicalDotsPerInchChanged(qreal)), this, SLOT(slot_logicalDotsPerInchChanged(qreal)));
      }
   }

   mThreadToopTask = new ThreadToopTask(this);
}

void MainUILogic::slot_physicalSizeChanged(const QSizeF &) {
   if (m_pLiveMainDlg) {
      m_pLiveMainDlg->repaint();
   }
}

void MainUILogic::slot_physicalDotsPerInchChanged(qreal) {
   if (m_pLiveMainDlg) {
      m_pLiveMainDlg->repaint();
   }
}
void MainUILogic::slot_logicalDotsPerInchChanged(qreal) {
   if (m_pLiveMainDlg) {
      m_pLiveMainDlg->repaint();
   }
}

void MainUILogic::myfunction() {

}

MainUILogic::~MainUILogic(void) {
   ReleaseCommonData();
   if (gLogger) {
      delete gLogger;
      gLogger = NULL;
   }
   if (mDesktopWdg) {
      delete mDesktopWdg;
      mDesktopWdg = nullptr;
   }
   if (mpVhallSharedQr != nullptr){
      delete mpVhallSharedQr;
      mpVhallSharedQr = nullptr;
   }
   if (m_pJoinActivity != NULL) {
      delete m_pJoinActivity;
      m_pJoinActivity = NULL;
   }
   TRACE6("%s delete ClearHttpTaskThread end\n", __FUNCTION__);
}

void MainUILogic::SetTipForceHide(bool bHide) {
   if (m_pLiveMainDlg) {
      m_pLiveMainDlg->SetTipForceHide(bHide);
   }
}

void MainUILogic::SetDesktopSharingUIShow(bool bShow) {
   TRACE6("%s bShow %d\n", __FUNCTION__, bShow);
   VH::CComPtr<ISettingLogic> pSettingLogic;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IMainUI, IID_ISettingLogic, pSettingLogic, return);
   if (!m_pDesktopSharingUI) {
      return;
   }
   if (bShow) {
      TRACE6("%s m_pDesktopSharingUI Show\n", __FUNCTION__);
      m_pDesktopSharingUI->Show();
      pSettingLogic->SetDesktopShare(true);
      createLiveMainDlg();
   }
   else {
      TRACE6("%s m_pDesktopSharingUI Close\n", __FUNCTION__);
      m_pDesktopSharingUI->Close();
      VH::CComPtr<ISettingLogic> pSettingLogic;
      DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IMainUI, IID_ISettingLogic, pSettingLogic, return, ASSERT(FALSE));
      pSettingLogic->UnlockVideo(NULL);
      m_pDesktopTip->Stop();
      pSettingLogic->SetDesktopShare(false);
      emit this->sigMainWidgetShowNormal();
   }

   if (m_pCaptureSrcUI) {
      m_pCaptureSrcUI->SetDesktopShareStatus(bShow);
   }
   m_pLiveMainDlg->SetDesktopSharing(bShow);
}

void MainUILogic::GetDesktopSharingUIWinId(HWND *pHwnd) {
   if (!pHwnd) {
      return;
   }
   if (!m_pDesktopSharingUI) {
      *pHwnd = NULL;
   }
   else {
      *pHwnd = (HWND)m_pDesktopSharingUI->winId();
   }
}

void MainUILogic::SlotSetMicMute(bool bMute) {
   if (!m_pLiveMainDlg) {
      return;
   }
   m_pLiveMainDlg->SetMicMute(bMute);
   m_pDesktopSharingUI->SetMicMute(bMute);
}

void MainUILogic::SetMicMute(bool bMute) {
   emit this->SigSetMicMute(bMute);
}

void MainUILogic::SetSpeakerMute(bool bMute) {
   if (!m_pLiveMainDlg) {
      return;
   }
   m_pLiveMainDlg->SetSpeakerMute(bMute);
}

void MainUILogic::AddImageWidget(void *w) {
   if (!m_pLiveMainDlg) {
      return;
   }
}
void MainUILogic::RemoveImageWidget(void *w) {
   if (!m_pLiveMainDlg) {
      return;
   }
   m_pLiveMainDlg->RemoveImageWidget((QWidget *)w);
}
int MainUILogic::GetVHDialogShowCount() {
   return VHDialog::VHDialogShowCount();
}

void MainUILogic::FadoutTip(wchar_t *t) {
   QString str = QString::fromWCharArray(t);
   if (!m_pLiveMainDlg || str.isEmpty()) {
      return;
   }
   if (!m_showMainWidget && m_pDesktopTip) {
      m_pDesktopTip->Tip(str);
   }
   else if (!m_pLiveMainDlg->isHidden()) {
      m_pLiveMainDlg->FadeOutTip(str, TipsType_Error);
   }
   else if (m_pDesktopSharingUI && !m_pDesktopSharingUI->isHidden() && m_showMainWidget && m_pLiveMainDlg->isHidden() && m_pDesktopTip) {
      m_pDesktopTip->Tip(str);
   }
}

void MainUILogic::SlotAreaModify(int left, int top, int right, int height) {
   VH::CComPtr<IOBSControlLogic> pObsControlLogic = NULL;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IOBSControl, IID_IOBSControlLogic, pObsControlLogic, return, ASSERT(FALSE));
   pObsControlLogic->ModifyAreaShared(left, top, right, height);
   TRACE6("MainUILogic::SlotAreaModify left:%d top:%d right:%d height:%d\n", left, top, right, height);
}

void MainUILogic::Slot_CloseAreaShare() {
   VH::CComPtr<IOBSControlLogic> pObsControlLogic = NULL;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IOBSControl, IID_IOBSControlLogic, pObsControlLogic, return, ASSERT(FALSE));
   pObsControlLogic->ClearSourceByType(SRC_MONITOR_AREA);
   if (m_pAreaMask) {
      delete m_pAreaMask;
      m_pAreaMask = nullptr;
   }
   if (m_pLiveMainDlg && m_pLiveMainDlg->isMinimized()) {
      m_pLiveMainDlg->showNormal();
   }
}

void MainUILogic::UpdateVolumn(int v) {

}

bool MainUILogic::ISHasActivityList() {
   return m_bHasList;
}

void * MainUILogic::GetLiveToolWidget() {
   return m_pLivePlugInUnitDlg;
}


bool MainUILogic::SetEnabledInteractive(const bool& bEnabled/* = false*/)
{
   if (NULL != m_pDesktopSharingUI)
   {
      m_pDesktopSharingUI->setEnablePlguin(bEnabled);
   }
   return true;
}

unsigned long long GetCurAppVersion(QString file, QString &currentVersion) {
   unsigned long long 				_i64Ver = 0;
   DWORD				dwVerHnd = 0;
   VS_FIXEDFILEINFO*	pFileInfo = NULL;
   unsigned int		dwInfoSize = 0;
   char	*			pVersionInfo = NULL;
   wchar_t  exeFile[1024] = { 0 };
   file.toWCharArray(exeFile);
   DWORD				dwVerInfoSize = GetFileVersionInfoSizeW(exeFile, &dwVerHnd);

   if (dwVerInfoSize) {
      pVersionInfo = new char[dwVerInfoSize];

      GetFileVersionInfoW(exeFile, dwVerHnd, dwVerInfoSize, pVersionInfo);
      VerQueryValue(pVersionInfo, (LPTSTR)("\\"), (void**)&pFileInfo, &dwInfoSize);

      currentVersion = QString("%1.%2.%3.%4")
         .arg(HIWORD(pFileInfo->dwFileVersionMS))
         .arg(LOWORD(pFileInfo->dwFileVersionMS))
         .arg(HIWORD(pFileInfo->dwFileVersionLS))
         .arg(LOWORD(pFileInfo->dwFileVersionLS));

      _i64Ver = pFileInfo->dwProductVersionMS;
      _i64Ver = _i64Ver << 32;
      _i64Ver |= pFileInfo->dwProductVersionLS;

      delete[] pVersionInfo;
      pVersionInfo = NULL;
   }
   return _i64Ver;
}

BOOL MainUILogic::Create() {
   do {
      TRACE6("MainUILogic::Create()\n");
      WCHAR szApp[1024 * 4] = L"";
      GetModuleFileNameW(NULL, szApp, 1024 * 4);
      GetCurAppVersion(QString::fromStdWString(szApp), mCurrentVersion);
      //初始化分辨率等信息
      Optimization();
      TRACE6("Optimization\n");
      InitConfig();
      TRACE6("InitConfig\n");

     //创建此文件，安装时inno setup检测没有此文件，安装winpcap.exe
      createFile(QString::fromStdWString(GetAppDataPath()), "winpcap.ini");

      //直播列表 创建活动列表
      m_pLiveListWdg = new VhallLiveListViewDlg();
      if (NULL == m_pLiveListWdg || !m_pLiveListWdg->Create()) {
         TRACE6("MainUILogic::Create() new VhallWebView(NULL) m_pActivityList Failed!\n");
         break;
      }
      m_pLiveListWdg->setWindowTitle(LIVE_LIST_NAME);
      connect(m_pLiveListWdg, SIGNAL(exitClicked(bool)), this, SLOT(AppExit(bool)));
      connect(m_pLiveListWdg, SIGNAL(sig_ExitAppDirect()), this, SLOT(Slot_HandleAppExit()));
      connect(m_pLiveListWdg, SIGNAL(SigTest()), this, SLOT(JoinActivity()));

      //createLiveMainDlg();
      TRACE6("createLiveMainDlg\n");
      //创建直播插件
      m_pLivePlugInUnitDlg = new VhallLivePlugInUnitDlg();
      if (NULL == m_pLivePlugInUnitDlg || !m_pLivePlugInUnitDlg->Create()) {
         TRACE6("MainUILogic::Create() new VhallWebView(NULL) m_pLiveTool Failed!\n");
         ASSERT(FALSE);
      }
      m_pLivePlugInUnitDlg->SetWindowTitle(LIVE_TOOL_NAME);
      connect(m_pLivePlugInUnitDlg, SIGNAL(SigClose()), this, SLOT(Slot_ClosePluginDlg()));

      VH::CComPtr<ICommonData> pCommonData;
      DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return false, ASSERT(FALSE));
      if (pCommonData) {
         pCommonData->SetMainUiLoginObj(this);
      }
      m_pPluginDataProcessor = new CPluginDataProcessor();
      if (!m_pPluginDataProcessor) {
         TRACE6("%s new m_pPluginDataProcessor err\n", __FUNCTION__);
      }

      m_pDesktopTip = new VHDesktopTip(NULL);
      m_pDesktopTip->SetPixmap(":/desktopSharing/img/desktopSharing/tipbackground.png");

      connect(this, SIGNAL(SigSetMicMute(bool)), this, SLOT(SlotSetMicMute(bool)));
      connect(&m_restartTimer, SIGNAL(timeout()), this, SLOT(SlotResetTimeOut()));
      connect(this, SIGNAL(SigAskForOpenWebPage(QString, QString, QString, QString, QString)), this, SLOT(SlotAskForOpenWebPage(QString, QString, QString, QString, QString)));

      m_pWaiting = new VhallWaiting(m_pLiveListWdg);
      if (m_pWaiting) {
         m_pWaiting->SetPixmap(":/interactivity/cb");
         m_pWaiting->Append(":/interactivity/c1");
         m_pWaiting->Append(":/interactivity/c2");
         m_pWaiting->Append(":/interactivity/c3");
         m_pWaiting->Append(":/interactivity/c4");
         m_pWaiting->Append(":/interactivity/c5");
         m_pWaiting->Append(":/interactivity/c6");
         m_pWaiting->Append(":/interactivity/c7");
         m_pWaiting->Append(":/interactivity/c8");
         m_pWaiting->Append(":/interactivity/c9");
         m_pWaiting->Append(":/interactivity/c10");
         m_pWaiting->Append(":/interactivity/c11");
         m_pWaiting->Append(":/interactivity/c12");
         connect(m_pWaiting, SIGNAL(SigShowWaiting()), this, SLOT(Slot_DisableWebListDlg()));
         connect(m_pWaiting, SIGNAL(SigCloseWating()), this, SLOT(Slot_EnableWebListDlg()));
      }

      if (m_pLiveListWdg) {
         m_pLiveListWdg->SetWaiting(m_pWaiting);
      }
      TRACE6("create end\n");
      mCefObj = new LibCefViewWdg(nullptr);
      if (mCefObj) {
         mCefObj->InitLibCef();
      }
      return TRUE;
   } while (FALSE);
   return FALSE;
}

void MainUILogic::Destroy() {
   StopInteraction();
   //为文档插件创建http server
   if (m_pPluginHttpServer != NULL) {
      int nRet = m_pPluginHttpServer->StopHttpServer();
      TRACE6("%s stop http server:%d\n", __FUNCTION__, nRet);
      delete m_pPluginHttpServer;
      m_pPluginHttpServer = NULL;
   }
   TRACE6("%s delete m_pPluginHttpServer end\n", __FUNCTION__);

   if (m_pJoinActivity != NULL) {
      delete m_pJoinActivity;
      m_pJoinActivity = NULL;
   }
   TRACE6("%s delete mDesktopShareChatParentWdg end\n", __FUNCTION__);
   if (m_pPluginDataProcessor) {
      m_pPluginDataProcessor->ClearAllData();
      delete m_pPluginDataProcessor;
      m_pPluginDataProcessor = NULL;
   }
   TRACE6("%s delete m_pPluginDataProcessor end\n", __FUNCTION__);
   if (NULL != m_pDesktopTip) {
      delete m_pDesktopTip;
      m_pDesktopTip = NULL;
   }
   TRACE6("%s delete m_pDesktopTip end\n", __FUNCTION__);
   if (NULL != m_pAreaMask) {
      delete m_pAreaMask;
      m_pAreaMask = NULL;
   }
   TRACE6("%s delete m_pAreaMask end\n", __FUNCTION__);
   //销毁共享源窗口
   //if (NULL != m_pCaptureSrcUI) {
   //   m_pCaptureSrcUI->Destroy();
   //   delete m_pCaptureSrcUI;
   //   m_pCaptureSrcUI = NULL;
   //}
   TRACE6("%s delete m_pCaptureSrcUI end\n", __FUNCTION__);
   //销毁关于对话框
   //if (NULL != m_pFullScreenLabel) {
   //   m_pFullScreenLabel->Destroy();
   //   delete m_pFullScreenLabel;
   //   m_pFullScreenLabel = NULL;
   //}
   TRACE6("%s delete m_pFullScreenLabel end\n", __FUNCTION__);
   //销毁桌面分享UI
   if (NULL != m_pDesktopSharingUI) {
      delete m_pDesktopSharingUI;
      m_pDesktopSharingUI = NULL;
   }
   TRACE6("%s delete m_pDesktopSharingUI end\n", __FUNCTION__);
   //销毁直播插件
   if (NULL != m_pLivePlugInUnitDlg) {
      delete m_pLivePlugInUnitDlg;
      m_pLivePlugInUnitDlg = NULL;
   }
   TRACE6("%s delete m_pLivePlugInUnitDlg end\n", __FUNCTION__);
   //销毁主界面
   if (NULL != m_pLiveMainDlg) {
      m_pLiveMainDlg->StopCPUState();
      delete m_pLiveMainDlg;
      m_pLiveMainDlg = NULL;
   }

   TRACE6("%s delete m_pLiveMainDlg end\n", __FUNCTION__);
   //销毁活动列表
   if (NULL != m_pLiveListWdg) {
      m_pLiveListWdg->Destory();
      delete m_pLiveListWdg;
      m_pLiveListWdg = NULL;
   }
   TRACE6("%s delete m_pLiveListWdg end\n", __FUNCTION__);
   if (mLoginDlg) {
      delete mLoginDlg;
      mLoginDlg = NULL;
   }
   ClearHttpTaskThread();
   if (mThreadToopTask) {
      mThreadToopTask->ClearAllWork();
   }
   TRACE6("%s delete ClearHttpTaskThread end\n", __FUNCTION__);
}

HRESULT STDMETHODCALLTYPE MainUILogic::QueryInterface(REFIID riid, void ** appvObject) {
   if (riid == IID_VHIUnknown) {
      *appvObject = (VH_IUnknown*)this;
      AddRef();
      return CRE_OK;
   }
   else if (IID_IMainUILogic == riid) {
      *appvObject = (IMainUILogic*)this;
      AddRef();
      return CRE_OK;
   }
   return CRE_NOINTERFACE;
}

ULONG STDMETHODCALLTYPE MainUILogic::AddRef(void) {
   return ::InterlockedIncrement(&m_lRefCount);
}

ULONG STDMETHODCALLTYPE MainUILogic::Release(void) {
   return ::InterlockedDecrement(&m_lRefCount);
}

void STDMETHODCALLTYPE MainUILogic::GetMainUIWidget(void** apMainUIWidget, int type) {
   if (NULL != m_pLiveMainDlg && type == eLiveType_Live) {
      *apMainUIWidget = m_pLiveMainDlg;
   }
   else if (NULL != mVhallIALive && type == eLiveType_VhallActive) {
      *apMainUIWidget = mVhallIALive;
   }
}

void STDMETHODCALLTYPE MainUILogic::GetShareUIWidget(void** apShareUIWidget, int type) {
   VH::CComPtr<ICommonData> pCommonData;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
   ClientApiInitResp initResp;
   pCommonData->GetInitApiResp(initResp);
   if (initResp.selectLiveType == eLiveType_Live) {
      if (m_pDesktopSharingUI->isHidden()) {
         *apShareUIWidget = m_pLiveMainDlg;
      }
      else {
         *apShareUIWidget = m_pDesktopSharingUI;
      }
   }
   else {
      *apShareUIWidget = mVhallIALive == nullptr ? nullptr : mVhallIALive->GetShareUIWidget();
   }
}

void STDMETHODCALLTYPE MainUILogic::GetContentWidget(void** apContentWidget, int type) {
   if (NULL != m_pLiveMainDlg && type == eLiveType_Live) {
      *apContentWidget = m_pLiveMainDlg->GetRenderWidget();
   }
   else if (NULL != mVhallIALive && type == eLiveType_VhallActive) {
      *apContentWidget = mVhallIALive->GetRenderWidget();
   }
}

void STDMETHODCALLTYPE MainUILogic::GetRenderPos(int& x, int& y) {
   if (m_pLiveMainDlg) {
      m_pLiveMainDlg->GetRenderPos(x, y);
   }
}

void STDMETHODCALLTYPE MainUILogic::GetLiveActiveMediaPlayBasePos(void** apContentWidget, int type) {
   if (NULL != m_pLiveMainDlg && type == eLiveType_Live) {
      *apContentWidget = m_pLiveMainDlg->GetRenderPos();
   }
   else if (NULL != mVhallIALive && type == eLiveType_VhallActive) {
      *apContentWidget = mVhallIALive->GetRenderWidget();
   }
}


int STDMETHODCALLTYPE MainUILogic::GetMediaPlayUIWidth(int type) {
   int width = 0;
   if (NULL != m_pLiveMainDlg && type == eLiveType_Live) {
      width = m_pLiveMainDlg->GetMediaPlayUIWidth();
   }
   else if (NULL != mVhallIALive && type == eLiveType_VhallActive) {
      width = mVhallIALive->GetMediaPlayUIWidth();
   }
   return width;
}

void STDMETHODCALLTYPE MainUILogic::GetHwnd(void** apMsgWnd, void** apRenderWnd) {
   if (NULL != m_pLiveMainDlg) {
      *apMsgWnd = (void*)m_pLiveMainDlg->winId();
      *apRenderWnd = (void*)mRenderWidgetID;
   }
}

void MainUILogic::DealMessage(DWORD adwMessageID, void* apData, DWORD adwLen) {
   switch (adwMessageID) {
      //显示Widget
   case MSG_MAINUI_WIDGET_SHOW:
      DealShowWidget(apData, adwLen);
      break;
   case MSG_MAINUI_START_LIVE_PRIVATE: {
      DealPrivateStart(apData, adwLen);
      break;
   }
   case MSG_MAINUI_CLICK_CONTROL:
      DealClickControl(apData, adwLen);
      break;
   case MSG_VHALLRIGHTEXTRAWIDGET_CREATE:
      DealExtraWidgetCreate(apData, adwLen);
      break;
   case MSG_MAINUI_DELETE_MONITOR: {
      SetDesktopSharingUIShow(false);
      break;
   }
   //处理源相关
   case MSG_MAINUI_OBSCONTROL_PROCESSSRC: {
      DealProcessSrc(apData, adwLen);
      break;
   }
   case MSG_VHALLRIGHTEXTRAWIDGET_ACTIVE:
      DealExtraWidgetActive(apData, adwLen);
      break;
   case MSG_MAINUI_DO_CLOSE_MAINWINDOW:
      DealDoCloseMainWindow(apData, adwLen);
      break;
   case MSG_MAINUI_PUSH_STREAM_STATUS:
      DealPushStreamStatus(apData, adwLen);
      break;
   case MSG_OBSCONTROL_SHAREREGION:
      DealShareRegion(apData, adwLen);
      break;
   case MSG_MAINUI_HIDE_AREA:
      DealHideShareRegion(apData, adwLen);
      break;
   case MSG_OBSCONTROL_STREAM_NOTIFY:
      DealStreamNotify(apData, adwLen);
      break;
   case MSG_OBSCONTROL_PUSH_STREAM_BITRATE_DOWN: {
      DealPushStreamBitRateDown();
      break;
   }
   case MSG_COMMONDATA_DATA_INIT:
      DealInitCommonData(apData, adwLen);
      break;
   case MSG_MAINUI_LOG:
      DealLog(apData, adwLen);
      break;
   case MSG_VHALLRIGHTEXTRAWIDGET_END_REFRESH:
      DealEndFefresh(apData, adwLen);
      break;
   case MSG_VHALLRIGHTEXTRAWIDGET_RIGHT_SYNC_USERLIST:
      break;
   case MSG_OBSCONTROL_VOICE_TRANSITION:
      DealVoiceTransition(apData, adwLen);
      break;
   case MSG_OBSCONTROL_STREAM_PUSH_SUCCESS:
      DealStreamPushSuccess();
      break;
      //录制状态改变
   case MSG_MAINUI_RECORD_CHANGE:
      DealRecordChange(apData, adwLen);
      break;
   case MSG_HTTPCENTER_HTTP_RS:
      DeaPointRecordRs(apData, adwLen);
      break;
   case MSG_MAINUI_HTTP_TASK:
      DealHttpTask(apData, adwLen);
      break;
   case MSG_VHALLRIGHTEXTRAWIDGET_INITCOMMONINFO: {//初始化通用信息
      DealInitCommonInfo(apData, adwLen);
      break;
   }
   default:
      break;
   }
}

void MainUILogic::SlotResetTimeOut() {
   TRACE6("MainUILogic::SlotResetTimeOut() Reset Strean\n");

   QString msg = LIVE_RTMP_REPUSH;
   FadoutTip((wchar_t*)msg.toStdWString().c_str());

   m_restartTimer.stop();
   //网络异常重推流，重新选择线路。
   VH::CComPtr<ISettingLogic> pSettingLogic;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IMainUI, IID_ISettingLogic, pSettingLogic, return);
   pSettingLogic->ReSelectPushStreamLine();
   PublishStream(1, true);
}

void MainUILogic::DealInitCommonData(void* apData, DWORD adwLen) {
   PublishInfo loStreamInfo;
   VH::CComPtr<ICommonData> pCommonData;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
   pCommonData->GetStreamInfo(loStreamInfo);
   if (loStreamInfo.m_bHideLogo && NULL != m_pLiveMainDlg) {
      m_pLiveMainDlg->HideLogo(true);
   }
   QString::fromStdString(loStreamInfo.mStreamName).toWCharArray(m_wzStreamId);
   
}

void MainUILogic::UploadInitInfo() {
   VH::CComPtr<ICommonData> pCommonData;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
   //--------------------------------------------------------------------------
   //cpu
   wchar_t cpuInfo[256] = { 0 };
   QString key = "HKEY_LOCAL_MACHINE\\HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0";
   QSettings *pReg = new QSettings(key, QSettings::NativeFormat);
   if (NULL != pReg) {
      pReg->value("ProcessorNameString").toString().toWCharArray(cpuInfo);
      delete pReg;
      pReg = NULL;
   }

   key = "HKEY_LOCAL_MACHINE\\HARDWARE\\DESCRIPTION\\System";
   pReg = new QSettings(key, QSettings::NativeFormat);
   if (NULL != pReg) {
      pReg->value("Systembiosversion").toString().toWCharArray(cpuInfo);
      delete pReg;
      pReg = NULL;
   }

   //screen
   wchar_t sys_res[32] = { 0 };
   int aScreenWidth = GetSystemMetrics(SM_CXVIRTUALSCREEN);
   int aScreenHeight = GetSystemMetrics(SM_CYVIRTUALSCREEN);
   swprintf_s(sys_res, 32, L"%04dx%04d", aScreenWidth, aScreenHeight);

   int outputWidth = 0;
   int outputHeight = 0;
   OBSOutPutInfo outputInfo;
   if (pCommonData) {
      pCommonData->GetOutputInfo(&outputInfo);
      outputWidth = outputInfo.m_outputSize.cx;
      outputHeight = outputInfo.m_outputSize.cy;
   }

   int iStartMode = pCommonData->GetStartMode();
   wchar_t version[64] = { 0 };
   wstring wversion;
   pCommonData->GetCurVersion(wversion);
   wcscpy_s(version, _countof(version), wversion.c_str());

   QString cupInfoq = QString::fromWCharArray(cpuInfo);
   cupInfoq = cupInfoq.replace(" ", "-");
   cupInfoq.toWCharArray(cpuInfo);

   QString cardDetails = UilityTool::GetCardInfo();

   OSVERSIONINFO osver = { sizeof(OSVERSIONINFO) };
   GetVersionEx(&osver);
   QString strOS = QSysInfo::prettyProductName().simplified();
   QString sb = QSysInfo::buildAbi().simplified();
   QString machineUniqueId = QSysInfo::machineUniqueId().simplified();
   strOS = strOS.replace(" ", "_");
   wchar_t osInfo[256] = { 0 };
   wchar_t cardInfo[256] = { 0 };
   strOS.toWCharArray(osInfo);
   cardDetails.toWCharArray(cardInfo);

   //------Log-----
   STRU_MAINUI_LOG log;

   swprintf_s(log.m_wzRequestUrl, DEF_MAX_HTTP_URL_LEN, L"key=init_info&sys_res=%s&start_mode=0&mac_id=%s&version=%s&k=%d&CPU=%s&OS=%s&sb=%s&w=%d&h=%d&card=%s",
      sys_res,
      machineUniqueId.toStdWString().c_str(),
      version,
      eLogRePortK_HostConfig,
      cpuInfo,
      osInfo,
      sb.toStdWString().c_str(),
      outputWidth,
      outputHeight, 
      cardInfo);

   QJsonObject body;
   body["pr"] = cupInfoq;  //处理器
   body["os"] = strOS;	//操作系统名称
   body["sb"] = QSysInfo::buildAbi().simplified();   	//位数
   body["did"] = machineUniqueId;
   body["card"] = cardDetails;
   QString json = CPathManager::GetStringFromJsonObject(body);
   strcpy_s(log.m_wzRequestJson, json.mid(0, DEF_MAX_HTTP_URL_LEN).toLatin1().data());
   SingletonMainUIIns::Instance().PostCRMessage(MSG_MAINUI_LOG, &log, sizeof(STRU_MAINUI_LOG));
}

//处理日志信息
void MainUILogic::DealLog(void *apData, DWORD adwLen) {
   DEF_CR_MESSAGE_DATA_DECLARE_CA(STRU_MAINUI_LOG, loMessage, return);
   QString logIdBase = ConfigSetting::ReadString(msConfPath, GROUP_DEFAULT, LOG_ID_BASE, "0");
   
   rapidjson::Document document;
   document.SetObject();
   rapidjson::Document::AllocatorType& allocator = document.GetAllocator();

   std::string data;
   rapidjson::Document doc;
   doc.Parse<0>(loMessage.m_wzRequestJson);
   if (doc.IsObject()) {
      StringBuffer buffer;
      rapidjson::Writer<StringBuffer> writer(buffer);
      doc.Accept(writer);
      data = buffer.GetString();
   }

   QJsonObject body = CPathManager::GetJsonObjectFromString(loMessage.m_wzRequestJson);
   QStringList strList = body.keys();
   for (int i = 0; i < strList.size(); i++) {
      QString key = strList[i];
      QJsonValue value = body.value(strList[i]);
      if (value.isString()) {
         rapidjson::Value contextStr(StringRef(value.toString().toStdString().c_str()));
         document.AddMember(StringRef(key.toStdString().c_str()), contextStr, allocator);
      }
      else {
         rapidjson::Value process_type(value.toInt());
         document.AddMember(StringRef(key.toStdString().c_str()), process_type, allocator);
      }
   }
   wstring version;
   VH::CComPtr<ICommonData> pCommonData;
   ClientApiInitResp initResp;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
   if (pCommonData) {
      pCommonData->GetCurVersion(version);
      pCommonData->GetInitApiResp(initResp);
   }

   DWORD processId = GetCurrentProcessId();//当前进程id  
   body["aid"] = QString::fromStdWString(m_wzStreamId);
   body["pid"] = QString::number(processId);
   body["pf"] = QString::number(6);
   body["uid"] = logIdBase;
   body["bu"] = QString::number(0);
   body["v"] = QString::fromStdWString(version);
   if (mSessionID.isEmpty()) {
      mSessionID = QUuid::createUuid().toString();
   }

   QString json_data = QJsonDocument(body).toJson();
   wchar_t *format = L"%s?%s&v=%s&id=%s&s=%s&pf=6&bu=0&aid=%s&streamid=%s&pt=%d&uid=%s&token=%s";
   STRU_HTTPCENTER_HTTP_RQ loRQ;

   wchar_t token[DEF_MAX_HTTP_URL_LEN] = { 0 };
   if (QString::fromWCharArray(m_wzStreamId).isEmpty()) {
      PublishInfo loStreamInfo;
      pCommonData->GetStreamInfo(loStreamInfo);
      if(loStreamInfo.mStreamName.length() > 0){
         QString::fromStdString(loStreamInfo.mStreamName).toWCharArray(m_wzStreamId);
      }
      else {
         ClientApiInitResp initInfo;
         pCommonData->GetInitApiResp(initInfo);
         if (initInfo.webinar_name.length() > 0) {
            initInfo.webinar_name.toWCharArray(m_wzStreamId);
         }
      }
   }

   WCHAR wzRequestUrl[DEF_MAX_HTTP_URL_LEN + 1] = {0};
   QByteArray text(json_data.toUtf8());
   QString::fromUtf8(text.toBase64()).toWCharArray(token);
   QString session = QUuid::createUuid().toString();
   swprintf_s(wzRequestUrl, DEF_MAX_HTTP_URL_LEN, format,
      reinterpret_cast<const wchar_t *>(m_strLogReportUrl.utf16()),
      loMessage.m_wzRequestUrl,
      version.c_str(),
      QString(logIdBase).toStdWString().c_str(),
      mSessionID.toStdWString().c_str(),
      wcslen(m_wzStreamId) > 0 ? m_wzStreamId : L"0",
      wcslen(m_wzStreamId) > 0 ? m_wzStreamId : L"0",
      initResp.player,
      QString(logIdBase).toStdWString().c_str(),
      token);

   HTTP_GET_REQUEST request(QString::fromWCharArray(wzRequestUrl).toStdString());
   //request.SetHttpPost(true);
   request.mbIsNeedSyncWork = false;
   request.mbIsDownLoadFile = true;
   GetHttpManagerInstance()->HttpGetRequest(request, [&](const std::string& msg, int code, const std::string userData) {

   });
}
void MainUILogic::DealEndFefresh(void *apData, DWORD adwLen) {
   if (this->m_pLiveListWdg) {
      this->m_pWaiting->Close();
   }
}

void MainUILogic::DealStreamNotify(void* apData, DWORD adwLen) {
   if (m_pLiveMainDlg && m_pLiveMainDlg->isHidden() && m_pDesktopSharingUI && m_pDesktopSharingUI->isHidden()) {
      return;
   }
   this->FadoutTip((wchar_t *)apData);
}

void MainUILogic::DealPushStreamBitRateDown() {
   if (m_pLiveMainDlg) {
      m_pLiveMainDlg->FadeOutTip(BITRATE_DOWN_TIPS, TipsType_Bitrate_Down);
   }
}

void MainUILogic::DealHideShareRegion(void* apData, DWORD adwLen) {
   if (!m_pAreaMask) {
      return;
   }
   delete m_pAreaMask;
   m_pAreaMask = nullptr;
}

void MainUILogic::DealExtraWidgetCreate(void* apData, DWORD adwLen) {
   if (!m_pLiveMainDlg) {
      return;
   }

   m_pLiveMainDlg->SetTipsClose(true);
}

void MainUILogic::DealProcessSrc(void* apData, DWORD adwLen) {
   DEF_CR_MESSAGE_DATA_DECLARE_CA(int, loMessage, return);
   if (loMessage == SRC_WINDOWS || loMessage == SRC_MONITOR_AREA) {
      InteractAPIManager api(this);
      api.HttpSendChangeWebWatchLayout(0);
   }
}

void MainUILogic::DealExtraWidgetActive(void* apData, DWORD adwLen) {
   if (!m_pLiveMainDlg) {
      return;
   }
   if (!m_pLiveMainDlg->isHidden()) {
      m_pLiveMainDlg->ReActive();
   }
}

QString GetLeft3Version(QString version) {
   QStringList vl = version.split('.');
   if (vl.count() == 4) {
      QString v = vl[0] + "." + vl[1] + "." + vl[2];
      return v;
   }
   return version;
}

void MainUILogic::DealPrivateStart(void* apData, DWORD adwLen) {
   VH::CComPtr<ICommonData> pCommonDataInfo;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonDataInfo, return, ASSERT(FALSE));
   QString startParam = pCommonDataInfo->GetPrivateStartParam();
   TRACE6("%s startParam:%s\n", __FUNCTION__, startParam.toStdString().c_str());
   StartLive(startParam);
   UploadInitInfo();
}

void MainUILogic::DealShowWidget(void* apData, DWORD adwLen) {
   QString version;
   VH::CComPtr<ICommonData> pCommonDataInfo;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonDataInfo, return, ASSERT(FALSE));
   DEF_CR_MESSAGE_DATA_DECLARE_CA(STRU_MAINUI_WIDGET_SHOW, loMessage, return);
   switch (loMessage.m_eType) {
      case widget_show_type_Main: {          //显示主界面
         TRACE6("%s widget_show_type_Main\n", __FUNCTION__);
         if (loMessage.pgNameCore) {
            m_pgNameCore = loMessage.pgNameCore;
         }
         m_bLoadExtraRightWidget = loMessage.bLoadExtraRightWidget;
         ClientApiInitResp initResp;
         pCommonDataInfo->GetInitApiResp(initResp);
         QString toolUrl = initResp.interact_plugins_url + "&version=" + mCurrentVersion + "&webinar_id=" + mStartParam.streamName;//访问连接  和 流Id
         if (m_pLiveMainDlg && m_bLoadExtraRightWidget) {
            m_pLiveMainDlg->ClearWebinarName();
            QString name = QString::fromStdWString(loMessage.m_webinarName);
            QString id = QString::fromStdWString(loMessage.m_streamName);
            if (!name.isEmpty() && !id.isEmpty()) {
               m_pLiveMainDlg->SetWebinarName(name, id);
            }
            if (initResp.player == H5_LIVE_CREATE_TYPE) {
               //h5活动加载 paas文档
               m_pLiveMainDlg->LoadDocUrl(initResp.web_doc_url + QString("&hide=1"));
               m_pLiveMainDlg->InitChatWebView(initResp.web_chat_url + QString("&hide=1"));
               toolUrl = initResp.web_doc_url + QString("&hide=1");
            }
            else {
               QString chatUrl = initResp.web_chat_url + "&version=" + mCurrentVersion + "&webinar_id=" + QString::fromWCharArray(loMessage.m_streamName);//访问连接  和 流Id
               m_pLiveMainDlg->InitChatWebView(chatUrl);
               m_pLiveMainDlg->LoadDocUrl(chatUrl.replace("chat.html", "doc.html"));
               m_pLiveMainDlg->CreateDocWebView();
               TRACE6("%s chatUrl %s\n", __FUNCTION__, chatUrl.toLatin1().data());
            }
         }
         else if (!m_bLoadExtraRightWidget) {
            StartLiveUrl liveparam;
            pCommonDataInfo->GetStartLiveUrl(liveparam);
            memset(&mStartParam, 0, sizeof(VHStartParam));
            mStartParam.bLoadExtraRightWidget = false;
            strcpy(mStartParam.streamToken, liveparam.stream_token.toStdString().c_str());
            strcpy(mStartParam.streamName, liveparam.stream_name.toStdString().c_str());
            strcpy(mStartParam.msgToken, liveparam.msg_token.toStdString().c_str());
            mStartParam.bHideLogo = liveparam.hide_logo;
            mStartParam.bConnectToVhallService = false;
            createLiveMainDlg();
            UploadInitInfo();
         }
         if (m_pLivePlugInUnitDlg && m_bLoadExtraRightWidget) {
            SetEnabledLiveListWdg(true);
            m_pLivePlugInUnitDlg->InitPluginUrl(toolUrl, this);
            TRACE6("%s InitPluginUrl toolUrl:%s\n", __FUNCTION__, toolUrl.toStdString().c_str());
         }
         
         QApplication::postEvent(this,new QEvent(CustomEvent_StartCreateOBS));
         TRACE6("StartLive pObsControlLogic Create");
         break;
      }                
      case widget_show_type_Activites:    //显示活动列表
         TRACE6("%s widget_show_type_Activites %d\n", __FUNCTION__, mbIsPwdLogin);
         if (loMessage.pgNameCore) {
            m_pgNameCore = loMessage.pgNameCore;
         }
         HidePluginDataDig();
         m_bHasList = true;
         m_domain = ConfigSetting::ReadString(CPathManager::GetToolConfigPath(), GROUP_DEFAULT, KEY_URL_DOMAIN, domainUrl);
         ShowActivityList(!mbIsPwdLogin);
         TRACE6("%s widget_show_type_Activites end\n", __FUNCTION__);
         break;
      case widget_show_type_logIn: {
         HandleShowLoginDlg(loMessage);
         break;
      }
      default:
         break;
   }
}

void MainUILogic::DealClickControl(void* apData, DWORD adwLen) {
   DEF_CR_MESSAGE_DATA_DECLARE_CA(STRU_MAINUI_CLICK_CONTROL, loMessage, return);
   //desktop Exclude others
   VH::CComPtr<IOBSControlLogic> pObsControlLogic = NULL;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IOBSControl, IID_IOBSControlLogic, pObsControlLogic, return, ASSERT(FALSE));

   VH::CComPtr<ICommonData> pCommonData;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));

   bool isHasMonitor = pObsControlLogic->IsHasSource(SRC_MONITOR);
   switch (loMessage.m_eType) {
      //最小化
   case control_Minimize:
      MinimizeApp((void *)loMessage.m_dwExtraData);
      break;
      //关闭客户端
   case control_CloseApp:
      CloseApp((void *)loMessage.m_dwExtraData, loMessage.m_bIsRoomDisConnect, loMessage.m_eReason);
      break;
      //共享源窗口
   case control_CaptureSrc:
      ShareSrcUI(loMessage.m_globalX, loMessage.m_globalY);
      TRACE6("[BP] MainUILogic::DealClickControl control_CaptureSrc\n");
      break;
      //区域共享
   case control_RegionShare:
      if (isHasMonitor) {
         return;
      }
      ShareRegion();
      TRACE6("[BP] MainUILogic::DealClickControl control_RegionShare\n");
      break;
      //桌面共享
   case control_MonitorSrc:
      ShareDesktop();
      TRACE6("[BP] MainUILogic::DealClickControl control_MonitorSrc\n");
      break;
   case control_Record: {
      //录制（录制中/录制暂停）
      if (m_pLiveMainDlg && !pObsControlLogic->IsStartStream()) {
         m_pLiveMainDlg->FadeOutTip(RECORED_AFTER_LIVING, TipsType_Error);
         return;
      }
      ShowRecordDlg();
      TRACE6("[BP] MainUILogic::DealClickControl control_Record\n");
   }
                        break;
   case control_AddCamera:
      if (m_pLiveMainDlg&&isHasMonitor) {
         m_pLiveMainDlg->FadeOutTip(SHAREING_ADDSOURCE, TipsType_Error);
      }
      TRACE6("[BP] MainUILogic::DealClickControl control_AddCamera\n");
      break;
      //推流
   case control_StartLive:
      TRACE6("control_StartLive\n");
      PublishStream(loMessage.m_dwExtraData, false);
      if (loMessage.m_dwExtraData == 1) {
         m_pLiveMainDlg->ClearTimeClock();
         if (m_pLivePlugInUnitDlg) {
            m_pLivePlugInUnitDlg->StartLive(true);
         }
         if (pCommonData->GetStartMode() == eStartMode_flash) {
            if (m_pLiveMainDlg) {
               m_pLiveMainDlg->SetStreamStatus(true);
               m_pLiveMainDlg->SetStreamButtonStatus(true);
               m_pLiveMainDlg->closeSuspendTip();
               m_pLiveMainDlg->StartTimeClock(TRUE);
            }
            if (m_pDesktopSharingUI) {
               m_pDesktopSharingUI->SetStreamButtonStatus(true);
            }
            if (m_pWaiting) {
               m_pWaiting->Close();
            }
            if (m_pStartWaiting) {
               m_pStartWaiting->Close();
            }
         }
         TRACE6("[BP] MainUILogic::DealClickControl control_StartLive [start]\n");
      }
      else {
         TRACE6("[BP] MainUILogic::DealClickControl control_StartLive [stop]\n");
         if (pCommonData->GetStartMode() == eStartMode_flash) {
            HandleStopUIState();
         }
      }
      TRACE6("control_StartLive end\n");
      break;
   case control_VoiceTranslate:
      DealVoiceTranslate();
      break;
      //直播插件
   case control_LiveTool:{
      ReloadPlguinUrl(loMessage.m_bIsReLoad);
   }
   break;
   default:
      break;
   }
}

void MainUILogic::OnRecvPushStreamErrorEvent() {
   QApplication::postEvent(this,new QEvent(CustomEvent_PushStreamError));
}

void MainUILogic::ReloadPlguinUrl(bool isReLoad) {
   if (m_pLivePlugInUnitDlg) {
      m_pLivePlugInUnitDlg->CenterWindow(NULL);
      if (m_pLivePlugInUnitDlg->isMinimized()) {
         m_pLivePlugInUnitDlg->showNormal();
      }
      else {
         VH::CComPtr<ICommonData> pCommonData;
         DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
         ClientApiInitResp respInitData;
         pCommonData->GetInitApiResp(respInitData);
         if (respInitData.selectLiveType == eLiveType_Live && m_pDesktopSharingUI) {
            m_pLivePlugInUnitDlg->show(QApplication::desktop()->availableGeometry(m_pDesktopSharingUI));
         }
         else if (mVhallIALive) {
            m_pLivePlugInUnitDlg->show(QApplication::desktop()->availableGeometry(mVhallIALive));
         }
      }
      m_pLivePlugInUnitDlg->raise();
      m_pLivePlugInUnitDlg->setFocus();
      if (isReLoad) {
         m_pLivePlugInUnitDlg->ReloadPluginUrl();
      }
   }
}

void MainUILogic::ShowActivityList(bool bShow) {
   if (NULL != m_pLiveListWdg) {
      TRACE6("%s %d\n", __FUNCTION__, bShow);
      VH::CComPtr<ICommonData> pCommonData;
      DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
      TRACE6("%s SetAudioCapture %d\n", __FUNCTION__, bShow);
      QString qsConfPath = CPathManager::GetConfigPath();
      int is_http_proxy = ConfigSetting::ReadInt(qsConfPath, GROUP_DEFAULT, PROXY_OPEN, 0);
      if (is_http_proxy) {
         QString host = ConfigSetting::ReadString(qsConfPath, GROUP_DEFAULT, PROXY_HOST, "");
         int port = ConfigSetting::ReadInt(qsConfPath, GROUP_DEFAULT, PROXY_PORT, 80);
         QString usr = ConfigSetting::ReadString(qsConfPath, GROUP_DEFAULT, PROXY_USERNAME, "");
         QString pwd = ConfigSetting::ReadString(qsConfPath, GROUP_DEFAULT, PROXY_PASSWORD, "");
         m_pLiveListWdg->SetHttpProxyInfo(true, host, QString::number(port), usr, pwd);
      }
      else {
         m_pLiveListWdg->SetHttpProxyInfo(false);
      }
      QString vhToolConfPath = QString::fromStdWString(GetAppDataPath() + VHALL_TOOL_CONFIG);
      QString vhallLive = ConfigSetting::ReadString(vhToolConfPath, GROUP_DEFAULT, KEY_VHALL_LIVE, VHALL_LIVE_TEXT);
      m_pLiveListWdg->setWindowTitle(vhallLive);
      m_pLiveListWdg->Load(pCommonData->GetLoginRespInfo().list_url + "&version=" + mCurrentVersion, this);
      TRACE6("%s Load %d\n", __FUNCTION__, bShow);
      m_pLiveListWdg->SetUserImage(pCommonData->GetLoginRespInfo().avatar);
      m_pLiveListWdg->SetUserName(pCommonData->GetLoginRespInfo().nick_name);
      m_pLiveListWdg->CenterWindow(NULL);
      TRACE6("%s CenterWindow %d\n", __FUNCTION__, bShow);
      if (bShow) {
         m_pLiveListWdg->show();
         TRACE6("%s m_pLiveListWdg  show\n", __FUNCTION__);
         m_pLiveListWdg->activateWindow();
         TRACE6("%s m_pLiveListWdg  activateWindow\n", __FUNCTION__);
         //m_pLiveListWdg->SlotRefresh();
         int crashState = ConfigSetting::ReadInt(CPathManager::GetConfigPath(), GROUP_DEFAULT, KEY_CRASH_STATE, 0);
         if (crashState == 1) {
            uint curTime = QDateTime::currentDateTime().toTime_t();
            uint crahsTime = ConfigSetting::ReadInt(msConfPath, GROUP_DEFAULT, KEY_CRASH_TIME, 0);
            if (curTime - crahsTime < 300) {
               //当用户在助手闪退之后的5分钟内，再次启动助手，进行弹窗提示，弹窗居中显示在直播列表页
               QString liveUrl = ConfigSetting::ReadString(CPathManager::GetConfigPath(), GROUP_DEFAULT, KEY_CRASH_LIVE_URL, "");
               QString liveName = ConfigSetting::ReadString(CPathManager::GetConfigPath(), GROUP_DEFAULT, KEY_CRASH_LIVE_NAME, "");
               if (!liveUrl.isEmpty() && !liveName.isEmpty()) {
                  TRACE6("%s StartLive liveUrl:%s\n", __FUNCTION__, liveUrl.toStdString().c_str());
                  if (liveName.length() > 8) {
                     liveName = liveName.mid(0, 8) + QString("...");
                  }
                  AlertTipsDlg tip(CRASH_START.arg(liveName), true, m_pLiveListWdg);
                  tip.CenterWindow(m_pLiveListWdg);
                  if (tip.exec() == QDialog::Accepted) {
                     StartLive(liveUrl);
                  }
               }
            }
         }
      }
   }
   VH::CComPtr<IOBSControlLogic> pObsControlLogic = NULL;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IOBSControl, IID_IOBSControlLogic, pObsControlLogic, return, ASSERT(FALSE));
   pObsControlLogic->SetAudioCapture(false);
   TRACE6("%s end %d\n", __FUNCTION__, bShow);
}

QJsonObject AnalysisUrl(QString url) {
   QJsonObject obj;
   QStringList l = url.split("?");
   if (l.count() == 2) {
      l = l[1].split("&");
      for (int i = 0; i < l.count(); i++) {
         QStringList v = l[i].split("=");
         if (v.count() == 2) {
            obj[v[0]] = v[1];
         }
      }
   }

   return obj;
}
void MainUILogic::JsCallQtJoinActivity() {
   JoinActivity();
}
void MainUILogic::SlotToJoinActivity(QString id, QString nick, QString password, bool isPwdLogin) {
   VH::CComPtr<ICommonData> pCommonData;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
   ToJoinActivityByLogin(id, nick, password);
   if (isPwdLogin) {
      pCommonData->SetJoinActiveFromLoginPage(true);
   }
   else {
      pCommonData->SetJoinActiveFromLoginPage(false);
   }
}

//加入活动
void MainUILogic::JoinActivity() {
   HandleJoinAcivity(false);
}

void MainUILogic::StartLive(QString url) {
   mILiveType = eLiveType_Live;
   m_pWaiting->Show();
   TRACE6("MainUILogic::StartLive %s\n", url.toLocal8Bit().data());
   SetEnabledLiveListWdg(false);

   /*解析url 记录当前参数*/
   QJsonDocument doc;
   QJsonObject tobj = AnalysisUrl(url);

   StartLiveUrl liveUrl;
   liveUrl.stream_token = tobj["stream_token"].toString();
   liveUrl.stream_name = tobj["stream_name"].toString();
   liveUrl.msg_token = tobj["msg_token"].toString();
   liveUrl.hide_logo = tobj["hide_logo"].toString() != "0" ? true : false;

   //保存活动url和活动名称
   ConfigSetting::writeValue(CPathManager::GetConfigPath(), GROUP_DEFAULT, KEY_CRASH_LIVE_URL, url);

   VH::CComPtr<ICommonData> pCommonData;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
   pCommonData->SetStartLiveUrl(liveUrl);
   pCommonData->SetJoinActiveFromLoginPage(false);
   memset(&mStartParam, 0, sizeof(VHStartParam));
   mStartParam.bLoadExtraRightWidget = true;
   strcpy(mStartParam.streamToken, tobj["stream_token"].toString().toUtf8().data());
   strcpy(mStartParam.streamName, tobj["stream_name"].toString().toUtf8().data());
   strcpy(mStartParam.msgToken, tobj["msg_token"].toString().toUtf8().data());
   mStartParam.bHideLogo = (tobj["hide_logo"].toString() != "0");
   mStartParam.bConnectToVhallService = false;
   DoHideLogo(liveUrl.hide_logo);
   wstring appPath = GetAppPath();
   QString domain = ConfigSetting::ReadString(CPathManager::GetToolConfigPath(), GROUP_DEFAULT, KEY_URL_DOMAIN, domainUrl);
   //init  请求初始化客户端 参数
   QString initUrl = domain + "/api/client/v1/clientapi/init?secure=0&token=" + QString(liveUrl.msg_token) + QString("&version=%1").arg(mCurrentVersion);
   //代理配置
   QString configPath = CPathManager::GetConfigPath();
   QString proxyHost;
   QString proxyUsr;
   QString proxyPwd;
   int proxyPort;
   int is_http_proxy = ConfigSetting::ReadInt(configPath, GROUP_DEFAULT, PROXY_OPEN, 0);
   if (is_http_proxy) {
      TRACE6("%s is_http_proxy:%d\n", __FUNCTION__, is_http_proxy);
      proxyHost = ConfigSetting::ReadString(configPath, GROUP_DEFAULT, PROXY_HOST, "");
      proxyPort = ConfigSetting::ReadInt(configPath, GROUP_DEFAULT, PROXY_PORT, 80);
      proxyUsr = ConfigSetting::ReadString(configPath, GROUP_DEFAULT, PROXY_USERNAME, "");
      proxyPwd = ConfigSetting::ReadString(configPath, GROUP_DEFAULT, PROXY_PASSWORD, "");
   }

   QObject *recvObj = this;
   HTTP_GET_REQUEST request(initUrl.toStdString());
   GetHttpManagerInstance()->HttpGetRequest(request, [&, recvObj](const std::string& msg, int code, const std::string userData) {
      QCoreApplication::postEvent(recvObj, new QHttpResponeEvent(CustomEvent_StartLive, code, QString::fromStdString(msg)));
   });
}

void MainUILogic::JsCallQtStartLive(QString msg) {
   TRACE6("%s msg:%s m_bEnableStartLive:%d\n", __FUNCTION__, msg.toStdString().c_str(), m_bEnableStartLive);
   //如果用户点击了开播，将异常状态清0
   ConfigSetting::writeValue(CPathManager::GetConfigPath(), GROUP_DEFAULT, KEY_CRASH_STATE, 0);
   uint curTime = QDateTime::currentDateTime().toTime_t();
   ConfigSetting::writeValue(msConfPath, GROUP_DEFAULT, KEY_CRASH_TIME, curTime);
   //为了防止从活动列表快速点击开播两次返回两次回调，再此处进行判断，屏蔽第一次点击之后的问题开播处理。
   if (m_bEnableStartLive) {
      //测试启动助手 
      m_bEnableStartLive = false;
      StartLive(msg);
   }
}

void MainUILogic::ShareUrl(QString url) {
   QJsonObject body;
   body["url"] = url;//分享连接
   SingletonMainUIIns::Instance().reportLog(STR_VH_LIVE_OPERATE, eLogRePortK_LiveListShare, L"Share", body);
   JsQtDebug("MainUILogic::ShareUrl:" + url);
   if (mpVhallSharedQr == nullptr) {
      mpVhallSharedQr = new VhallSharedQr();
   }
   mpVhallSharedQr->CenterWindow(m_pLiveListWdg);
   int index = url.lastIndexOf("/");
   QString domain = url.mid(0, index);
   QString streamId = url.mid(index + 1, url.length());
   mpVhallSharedQr->Shared(domain, streamId);
}

void MainUILogic::JsCallQtDebug(QString msg) {
}

void MainUILogic::JsQtDebug(QString msg) {
}

void MainUILogic::JsCallQtStartVoiceLive(QString url) {
   AlertTipsDlg tip(VOICE_LIVE, true, m_pLiveListWdg);
   tip.CenterWindow(m_pLiveListWdg);
   tip.SetYesBtnText(START_USER_WEB);
   tip.SetNoBtnText(NOT_USER_WEB);
   if (tip.exec() == QDialog::Accepted) {
      QDesktopServices::openUrl(url);
   }
}

void MainUILogic::JsCallQtOpenUrl(QString url) {
   VH::CComPtr<ICommonData> pCommonData;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
   if (pCommonData) {
      QString domain = ConfigSetting::ReadString(CPathManager::GetToolConfigPath(), GROUP_DEFAULT, KEY_URL_DOMAIN, domainUrl);
      LoginRespInfo loginInfo = pCommonData->GetLoginRespInfo();
      QString createLiveUrl = QString("http:") + loginInfo.sso_url + "&after_login=" + url;
      TRACE6("%s createLiveUrl:%s\n", __FUNCTION__, createLiveUrl.toStdString().c_str());
      //免登陆创建直播
      QDesktopServices::openUrl(createLiveUrl);
   }
}

void MainUILogic::JSCallQtResize() {
   if (m_pLivePlugInUnitDlg) {
      m_pLivePlugInUnitDlg->JSCallQtResize();
   }
}

void MainUILogic::JsCallQtShareTo(QString msg) {
   //测试分享
   qDebug() << "MainUILogic::JSSendShared " << msg;
   ShareUrl(msg);
}

void MainUILogic::ShowMainWidget() {
   VH::CComPtr<ICommonData> pCommonData;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
   ClientApiInitResp respInitData;
   pCommonData->GetInitApiResp(respInitData);
   if(m_pLiveMainDlg == nullptr){
      createLiveMainDlg();
   }
   if (NULL != m_pLiveMainDlg) {
      TRACE6("%s \n", __FUNCTION__);
      m_showMainWidget = true;
      m_pLiveMainDlg->CenterWindow(NULL);
      m_pLiveMainDlg->SetStartLiveState(true);
      if (m_bShowTeaching) {
         m_pLiveMainDlg->SetTeachingPage(true);
      }
      else {
         m_pLiveMainDlg->SetTeachingPage(false);
      }
      m_bShowTeaching = false;
      m_pLiveMainDlg->InitUI();
      m_pLiveMainDlg->show();
      ConfigSetting::MoveWindow(m_pLiveMainDlg);
      QString dpi = ConfigSetting::ReadString(CPathManager::GetConfigPath(), GROUP_DEFAULT, DESKTOP_DIP, "1");
      if (!dpi.isEmpty() && dpi == "1") {
         double screen_dipSize = ConfigSetting::GetCurrentScreenDPISize(CPathManager::GetConfigPath(), m_pLiveMainDlg);
         if (screen_dipSize > 1.0) {
            m_pLiveMainDlg->slotMaxClicke();
         }
      }

      TRACE6("%s  m_pLiveMainDlg->show()\n", __FUNCTION__);
      if (m_pDesktopSharingUI) {
         m_pDesktopSharingUI->SetShowChat(m_bLoadExtraRightWidget);
      }

      bool bIsLiving = false;
      if (respInitData.player == FLASH_LIVE_CREATE_TYPE) {
         bIsLiving = respInitData.webinar_type == 1 ? true : false;
         InitFlashDocWebServer();
      }
      else {
         VSSGetRoomInfo roomInfo;
         pCommonData->GetVSSRoomBaseInfo(roomInfo);
         bIsLiving = roomInfo.status == 1 ? true : false;
      }

      if (bIsLiving) {
         m_pLiveMainDlg->StartLiveWithTime(respInitData.live_time);
         PublishStream(1, false, true);
      }
   }
}

void MainUILogic::MinimizeApp(void *parent) {
   if (parent == m_pLiveMainDlg) {
      if (NULL != m_pLiveMainDlg) {
         m_pLiveMainDlg->showNormal();
         m_pLiveMainDlg->showMinized();
      }
   }
   else if (parent == m_pLiveListWdg) {
      if (NULL != m_pLiveListWdg) {
         m_pLiveListWdg->showNormal();
         m_pLiveListWdg->showMinimized();
      }
   }
}
void MainUILogic::DealDoCloseMainWindow(void* apData, DWORD adwLen) {
   TRACE6("MainUILogic::DealDoCloseMainWindow\n");
   VH::CComPtr<ICommonData> pCommonData;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
   ClientApiInitResp respInitData;
   pCommonData->GetInitApiResp(respInitData);
   if (respInitData.player == H5_LIVE_CREATE_TYPE) {
      TRACE6("%s DestoryPaasSDKInstance\n",__FUNCTION__);
      vlive::DestroyWebRtcSDKInstance();
      TRACE6("%s DestoryPaasSDKInstance\n", __FUNCTION__);
      vlive::DestoryPaasSDKInstance();
      TRACE6("%s DestoryPaasSDKInstance end\n", __FUNCTION__);
   }
   if (m_bHasList && m_bShowList && m_pLiveListWdg != NULL) {
      TRACE6("MainUILogic::DealDoCloseMainWindow ShowActiveList \n");
      ShowActivityList(!mbIsPwdLogin);
      m_pLiveListWdg->SlotRefresh();
      StopInteraction();
      if (m_pLivePlugInUnitDlg) {
         m_pLivePlugInUnitDlg->hide();
      }
   }
   else {
      //处理flahs启动时，应用程序从主窗口退出。
      if (m_pLiveMainDlg) {
         m_pLiveMainDlg->ToExitAppWithAnimation();
      }
      else {
         PostEventToExitApp();
         TRACE6("MainUILogic::DealDoCloseMainWindow Choose to close Application. \n");
      }
   }
}

void MainUILogic::PostEventToExitApp() {
   VH::CComPtr<ICommonData> pCommonData;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
   if (pCommonData) {
      QObject* obj = pCommonData->GetRuntimeInstanceObj();
      if (obj) {
         QApplication::postEvent(obj,new QEvent(CustomEvent_AppExit));
      }
   }
}

void MainUILogic::DealPushStreamStatus(void* apData, DWORD adwLen) {
   if (adwLen != sizeof(StreamStatus)) {
      return;
   }
   StreamStatus *status = (StreamStatus *)apData;
   if (m_pLiveMainDlg) {
      m_pLiveMainDlg->SetStreamStatus(status);
      if (m_pDesktopSharingUI && !m_pDesktopSharingUI->isHidden()) {
         float rate = m_pLiveMainDlg->GetPushStreamLostRate();
         m_pDesktopSharingUI->SetPushStreamLost(rate / 100.0);
         int vol = m_pLiveMainDlg->GetMixAudioVolume();
         m_pDesktopSharingUI->SetMicVol(vol);
      }
   }
}

void MainUILogic::DealShareRegion(void* apData, DWORD adwLen) {
   if (adwLen != sizeof(STRU_OBSCONTROL_SHAREREGION)) {
      return;
   }

   if (m_pAreaMask == nullptr) {
      //区域共享
      m_pAreaMask = new VHAreaMark(NULL);
      if (NULL == m_pAreaMask) {
         ASSERT(FALSE);
      }
      connect(m_pAreaMask, SIGNAL(SigAreaChanged(int, int, int, int)), this, SLOT(SlotAreaModify(int, int, int, int)));
      connect(m_pAreaMask, SIGNAL(SigCloseAreaShare()), this, SLOT(Slot_CloseAreaShare()));
   }

   STRU_OBSCONTROL_SHAREREGION *ShareRegion = (STRU_OBSCONTROL_SHAREREGION *)apData;
   m_pAreaMask->ShowShared(
      ShareRegion->m_rRegionRect.left,
      ShareRegion->m_rRegionRect.top,
      ShareRegion->m_rRegionRect.right,
      ShareRegion->m_rRegionRect.bottom);

   if (m_pLiveMainDlg) {
      m_pLiveMainDlg->showMinimized();
   }

   InteractAPIManager api(this);
   api.HttpSendChangeWebWatchLayout(1);
}

void MainUILogic::CloseApp(void *parent, bool bIsActiveDisconnect/* = false*/, int reason) {
   TRACE6("%s bIsActiveDisconnect:%d reason:%d\n", __FUNCTION__, bIsActiveDisconnect, reason);
   m_bEnableStartLive = true;
   if (parent == m_pLiveListWdg) {
      TRACE6("%s\n", __FUNCTION__);
      HandleAppExit();
   }
   else {
      HandleCloseMainLiveWnd();
   }
   return;
}

void MainUILogic::ShareRegion() {
   TRACE6("%s enter\n", __FUNCTION__);
   if (m_pFullScreenLabel == nullptr) {
      m_pFullScreenLabel = new FullScreenLabel(m_pLiveMainDlg);
      if (NULL == m_pFullScreenLabel || !m_pFullScreenLabel->Create()) {
         ASSERT(FALSE);
      }
      TRACE6("create m_pFullScreenLabel\n");
   }

   if (NULL != m_pFullScreenLabel) {
      QScreen * primaryScreen = QGuiApplication::primaryScreen();
      if (primaryScreen) {
         TRACE6("%s screen\n", __FUNCTION__);
         QPixmap pixmap = primaryScreen->grabWindow(0);
         if (!pixmap.isNull()) {
            TRACE6("%s create pixmap\n", __FUNCTION__);
            m_pFullScreenLabel->SetFullScreenPixmap(pixmap);
            m_pFullScreenLabel->showFullScreen();
            m_pFullScreenLabel->raise();
         }
      }
   }
   TRACE6("%s leave\n", __FUNCTION__);
}

void MainUILogic::ShareDesktop() {
   VH::CComPtr<IOBSControlLogic> pObsControlLogic = NULL;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IOBSControl, IID_IOBSControlLogic, pObsControlLogic, return, ASSERT(FALSE));
   TRACE6("%s start\n", __FUNCTION__);
   bool isHasMonitor = pObsControlLogic->IsHasSource(SRC_MEDIA_OUT);
   if (m_pLiveMainDlg && isHasMonitor) {
      AlertTipsDlg tip(CLOSE_FILE_TO_DESKTOP, true, m_pLiveMainDlg);
      tip.CenterWindow(m_pLiveMainDlg);
      if (tip.exec() == QDialog::Accepted) {
         pObsControlLogic->MediaStop();
         StopPlayMediaFile();
      }
      else {
         TRACE6("%s play media file\n", __FUNCTION__);
         return;
      }
   }
   TRACE6("%s VHD_Window_Enum_init\n", __FUNCTION__);
   VHD_Window_Enum_init(VHD_Desktop);
   TRACE6("%s VHD_Window_Enum_init end\n", __FUNCTION__);
   vector<VHD_WindowInfo> allDesktop = VHD_DesktopWindowInfo();
   if (allDesktop.size() > 1) {
      TRACE6("%s allDesktop.size() > 1\n", __FUNCTION__);
      if (ShowScreenSelectDlg(allDesktop, m_pLiveMainDlg) == QDialog::DialogCode::Rejected) {
         TRACE6("%s Rejected\n", __FUNCTION__);
         return;
      }
   }
   else if (allDesktop.size() == 1) {
      TRACE6("%s allDesktop.size() == 1\n", __FUNCTION__);
      if (m_pDesktopSharingUI) {
         m_pDesktopSharingUI->SetShowShareSelect(false);
         m_pDesktopSharingUI->SetCurrentShareScreenInfo(allDesktop.at(0));
         m_pDesktopSharingUI->Show();
         VH::CComPtr<ISettingLogic> pSettingLogic;
         DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IMainUI, IID_ISettingLogic, pSettingLogic, return);
         pSettingLogic->SetDesktopShare(true);
      }
      StartToCaptureDesktop(allDesktop.at(0));
      TRACE6("%s only one screen\n", __FUNCTION__);
   }

   if (!pObsControlLogic->IsHasSource(SRC_MONITOR)) {
      TRACE6("%s IsHasSource\n", __FUNCTION__);
      pObsControlLogic->EnterSharedDesktop();
   }
   TRACE6("%s leave\n", __FUNCTION__);
}

int MainUILogic::ShowScreenSelectDlg(vector<VHD_WindowInfo>& allDesktop,QWidget* reference) {
   TRACE6("%s screen count %d\n", __FUNCTION__, allDesktop.size());
   if (mDesktopWdg == nullptr) {
      mDesktopWdg = new DesktopShareSelectWdg();
   }
   if (!mDesktopWdg->isHidden()) {
      return QDialog::DialogCode::Rejected;
   }
   mDesktopWdg->InsertDesktopPreviewItem(allDesktop);
   ConfigSetting::MoveWindow(mDesktopWdg, reference);
   if (mDesktopWdg->exec() == QDialog::DialogCode::Accepted) {
      int index = mDesktopWdg->GetCurrentIndex();
      VHD_WindowInfo currentScreen = mDesktopWdg->GetCurrentWindowInfo();
      StartToCaptureDesktop(currentScreen);
      if (m_pDesktopSharingUI) {
         m_pDesktopSharingUI->SetShowShareSelect(true);
         m_pDesktopSharingUI->SetCurrentShareScreenInfo(currentScreen);
         m_pDesktopSharingUI->Show();
         VH::CComPtr<ISettingLogic> pSettingLogic;
         DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IMainUI, IID_ISettingLogic, pSettingLogic, return 0);
         pSettingLogic->SetDesktopShare(true);
         //pSettingLogic->ReSetCameraWndPos(QString::fromStdWString(currentScreen.name));
      }
      TRACE6("%s select index %d\n", __FUNCTION__, index);
      delete mDesktopWdg;
      mDesktopWdg = nullptr;
      return QDialog::DialogCode::Accepted;
   }
   else {
      TRACE6("%s close\n", __FUNCTION__);
      delete mDesktopWdg;
      mDesktopWdg = nullptr;
      return QDialog::DialogCode::Rejected;
   }
}

void MainUILogic::StartToCaptureDesktop(VHD_WindowInfo windowInfo) {
   VHD_Window_Enum_final();
   int captureType = ConfigSetting::ReadInt(msConfPath, GROUP_DEFAULT, KEY_DESKCTOP_CAPTURE_TYPE, 0);
   int enhance_state = ConfigSetting::ReadInt(msConfPath, GROUP_DEFAULT, DESKTOP_ENHANCE, 0);
   windowInfo.captureType = captureType == 0 ? VHD_Capture_BitBlt : VHD_Capture_DX;
   windowInfo.isOpenEnhance = enhance_state > 0 ? true : false;
   TRACE6("%s select desktop type %d\n", __FUNCTION__, captureType);
   SingletonMainUIIns::Instance().PostCRMessage(MSG_OBSCONTROL_DESKTOPSHARE, &windowInfo, sizeof(VHD_WindowInfo));
   if (enhance_state > 0) {
      QJsonObject body;
      SingletonMainUIIns::Instance().reportLog(STR_VH_LIVE_OPERATE, eLogRePortK_Desktop_Hance, L"desktop_enhance", body);
      TRACE6("%s DesktopEnhanceControlSwitch \n", __FUNCTION__);
   }
   VH::CComPtr<ICommonData> pCommonData;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
   pCommonData->ReportEvent("zmys00");
   m_pLiveMainDlg->Hide();

   InteractAPIManager apiManager(this);
   apiManager.HttpSendChangeWebWatchLayout(1);
   TRACE6("%s mainDlg hide\n", __FUNCTION__);
}

void MainUILogic::InitConfig() {
   QString savePath = ConfigSetting::ReadString(msConfPath, GROUP_DEFAULT, KEY_RECORD_PATH, "");
   int iQuality = ConfigSetting::ReadInt(msConfPath, GROUP_DEFAULT, KEY_PUBLISH_QUALITY, 2);
   VH::CComPtr<ICommonData> pCommonData;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
   pCommonData->SetOutputInfo(&GOutputInfo[iQuality]);
}

void MainUILogic::StopDesktopSharing() {
   TRACE6("%s\n", __FUNCTION__);
   VH::CComPtr<ICommonData> pCommonData;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
   ClientApiInitResp respInitData;
   pCommonData->GetInitApiResp(respInitData);

   pCommonData->ReportEvent("zmys02");
   if (respInitData.selectLiveType == eLiveType_Live) {
      VH::CComPtr<IOBSControlLogic> pObsControlLogic = NULL;
      DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IOBSControl, IID_IOBSControlLogic, pObsControlLogic, return, ASSERT(FALSE));
      pObsControlLogic->ClearSourceByType(SRC_MONITOR);
      SetDesktopSharingUIShow(false);
      SingletonMainUIIns::Instance().PostCRMessage(MSG_OBSCONTROL_DESKTOPSHARE, NULL, 0);
   }
   if (m_pLivePlugInUnitDlg) {
      m_pLivePlugInUnitDlg->close();
   }

   VH::CComPtr<ISettingLogic> pSettingLogic;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IMainUI, IID_ISettingLogic, pSettingLogic, return);
   pSettingLogic->SetDesktopShare(false);
   pSettingLogic->SetDesktopShareState(false);

   InteractAPIManager apiManager(this);
   apiManager.HttpSendChangeWebWatchLayout(0);
   QJsonObject body;
   MainUIIns::reportLog(STR_VH_LIVE_OPERATE, eLogRePortK_DesktopShare_StopDeskTopShare, L"StopDeskTopShare", body);
   TRACE6("%s end\n", __FUNCTION__);
}

void MainUILogic::MainWidgetMove() {
   VH::CComPtr<IVedioPlayLogic> pVedioPlayLogic = NULL;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IMainUI, IID_IVedioPlayLogic, pVedioPlayLogic, return);
   if (pVedioPlayLogic) {
      pVedioPlayLogic->StopAdmin();
   }
}



void MainUILogic::PublishStream(DWORD dwPublishState, bool mediaCoreStreamErr, bool isAlreadyLive) {
   TRACE6("%s \n", __FUNCTION__);
   if (NULL == m_pLiveMainDlg) {
      ASSERT(FALSE);
      if (m_pDesktopSharingUI) {
         m_pDesktopSharingUI->SetStartLiveBtnEnable(true);
      }
      return;
   }

   VH::CComPtr<ICommonData> pCommonData = NULL;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData);

   if (m_restartTimer.isActive()) {
      m_restartTimer.stop();
   }

   if (dwPublishState == 0 && pCommonData != nullptr && pCommonData->GetPublishState() == false) {
      TRACE6("%s stop again\n", __FUNCTION__);
      if (m_pDesktopSharingUI) {
         m_pDesktopSharingUI->SetStartLiveBtnEnable(true);
      }
      return;
   }

   TRACE6("MainUILogic::PublishStream  dwPublishState %d\n", dwPublishState);
   QString streamId = "";//流名
   QString streamName = "";//流名与用户ID的组合   
   QString webinar_type = "";//flash传递参数，是否为多嘉宾活动 ismix 
   QString accesstoken = "";//flash传递参数
   QString domain = "";
   QString role = "";
   QString userId = "";
   int protocolType;

   PublishInfo loStreamInfo;
   if (pCommonData) {
      pCommonData->GetStreamInfo(loStreamInfo);
      streamId = QString::fromStdString(loStreamInfo.mStreamName);//流名
      webinar_type = loStreamInfo.mWebinarType;//flash传递参数，是否为多嘉宾活动 ismix 
      accesstoken = loStreamInfo.mAccesstoken;//flash传递参数
      protocolType = loStreamInfo.nProtocolType;
      role = loStreamInfo.mRole;

      if (webinar_type == "2") {
         if (role == "host") {
            streamName = streamId + "@host@";//流名与用户ID的组合   
         }
         else {
            streamName = streamId + "@" + loStreamInfo.mUserId + "@";//流名与用户ID的组合   
         }
      }
      else {
         streamName = streamId;
      }

      domain = loStreamInfo.mScheduler;
      userId = loStreamInfo.mUserId;
   }

   QString client_type = "2";// 客户端类型 2 代表助手
   //请求调度
   QString url = QString("http:%1/api/dispatch_publish?webinar_id=%2&stream_name=%3&webinar_type=%4&accesstoken=%5&client_type=%6&protocol_type=%7")
      .arg(domain)
      .arg(streamId)
      .arg(streamName)
      .arg(webinar_type)
      .arg(accesstoken)
      .arg(client_type)
      .arg(protocolType);

   bool bScheduling = true;
   QByteArray ba;
   if (!domain.isEmpty()) {
      if (!mediaCoreStreamErr) {
         InteractAPIManager api(this);
         api.HttpSendDispatchPublish(url, dwPublishState, mediaCoreStreamErr, isAlreadyLive);
         return;
      }
   }
   HandlePushStreamState(bScheduling, dwPublishState, mediaCoreStreamErr, isAlreadyLive);
}

void MainUILogic::HandlePushStreamState(bool bScheduling, int dwPublishState, bool mediaCoreStreamErr, bool isAlreadyLive) {
   int iResult = -1;
   bool bIsStartPush = (dwPublishState == 1 ? true : false);          //1代表开始推流 0代表结束推流
   if (bIsStartPush && !mediaCoreStreamErr) {   //开始推流,并且不是底层推流失败导致的重推。
      VH::CComPtr<ICommonData> pCommonData;
      DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
      pCommonData->SetPublishState(bIsStartPush);
      if (m_bLoadExtraRightWidget && !isAlreadyLive) {

         VH::CComPtr<ISettingLogic> pSettingLogic;
         DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IMainUI, IID_ISettingLogic, pSettingLogic, return);
         if (pSettingLogic) {
            if (!pSettingLogic->IsHasAudioDevice()) {
               if (m_pLiveMainDlg) {
                  m_pLiveMainDlg->FadeOutTip(LIVE_NO_DEVICE, TipsType_SystemBusy);
               }
               return;
            }
         }
         if (m_pStartWaiting) {
            m_pStartWaiting->Show();
         }
         InteractAPIManager api(this);
         api.HttpSendStartLive(eLiveType_Live, bScheduling);
         QJsonObject body;
         body["ls"] = "Start";
         SingletonMainUIIns::Instance().reportLog(STR_VH_LIVE_OPERATE, eLogRePortK_StartLive, L"Live_StartLive", body);
      }
      else {
         if (m_pLiveMainDlg) {
            m_pLiveMainDlg->SetStreamStatus(true);
         }
         if (m_pDesktopSharingUI) {
            m_pDesktopSharingUI->setEnablePlguin(true);
            m_pDesktopSharingUI->SetStreamButtonStatus(true);
         }
         HandlePushStream(bIsStartPush, mediaCoreStreamErr, bScheduling);
      }
   }
   else if (bIsStartPush && (isAlreadyLive || mediaCoreStreamErr)) {//重推流
      HandlePushStream(bIsStartPush, mediaCoreStreamErr, bScheduling);
   }
   else if (!bIsStartPush) {      //停止推流
      if (m_bLoadExtraRightWidget) {
         StopWebnair(false);
      }
   }

   TRACE6("%s MonitorReportPublish\n", __FUNCTION__);
   //监控上报(开始/结束推流)
   MonitorReportPublish(bIsStartPush);
   TRACE6("%s end\n", __FUNCTION__);
}

void MainUILogic::MonitorReportPublish(BOOL bStartPublish) {

}

static bool have_clockfreq = false;
static LARGE_INTEGER clock_freq;

static inline unsigned long long get_clockfreq(void) {
   if (!have_clockfreq)
      QueryPerformanceFrequency(&clock_freq);
   return clock_freq.QuadPart;
}

unsigned long long os_gettime_ns(void) {
   LARGE_INTEGER current_time;
   double time_val;

   QueryPerformanceCounter(&current_time);
   time_val = (double)current_time.QuadPart;
   time_val *= 1000000000.0;
   time_val /= (double)get_clockfreq();

   return (unsigned long long)time_val;
}
void MainUILogic::ShareSrcUI(int x, int y) {
   if (m_pCaptureSrcUI == nullptr) {
      //共享源窗口
      m_pCaptureSrcUI = new CaptureSrcUI((QDialog*)m_pLiveMainDlg);
      if (NULL == m_pCaptureSrcUI || !m_pCaptureSrcUI->Create()) {
         return;
      }
      connect(m_pCaptureSrcUI, SIGNAL(sig_hideCaptureSrcUi()), this, SLOT(slot_hideCaptureSrcUi()));
   }
   m_pCaptureSrcUI->show();
   m_pCaptureSrcUI->move(x, y);
}

void MainUILogic::ShowRecordDlg() {
   if (NULL == m_pLiveMainDlg) {
      ASSERT(FALSE);
      return;
   }
   else {
      m_pLiveMainDlg->ShowRecordDlg();
   }
}

bool MainUILogic::IsDesktopShow() {
   if (!m_pCaptureSrcUI) {
      return false;
   }
   return !m_pCaptureSrcUI->isHidden();
}

void MainUILogic::UpLoadEvents() {
   VH::CComPtr<ICommonData> pCommonData;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));

   VHJson::Value valueeventid;
   VHJson::Value jsonRoot;
   VHJson::Value jsonItem;
   valueeventid["eventid"] = pCommonData->GetEvents();
   wstring sCurVeison;
   pCommonData->GetCurVersion(sCurVeison);
   jsonItem["version"] = QString::fromStdWString(sCurVeison).toStdString();
   jsonItem["event"].append(valueeventid);
   jsonRoot["helper"] = jsonItem;
   string sInfo = jsonRoot.toStyledString();

   CUpLoader oUpLoader;
   oUpLoader.UpLoadInfo("http://datacollect.vhall.com:7880/helperevent", (char*)sInfo.c_str());
}

//收到websocket消息
void MainUILogic::RecvWebsocketMsg(char *msg) {
   JsQtDebug("[WebSocket]" + QString(msg));
   QString msgStr = msg;
   msgStr.replace("\\", "\\\\");
   msgStr.replace("\"", "\\\"");
   QString method = QString("qtCallJsWebSocketSendMsg(\"%1\")").arg(msgStr);
   this->m_pLivePlugInUnitDlg->executeJSCode(method);
   QCoreApplication::postEvent(this, new CustomRecvMsgEvent(CustomEvent_RecvMsg, method));
}

//收到socketIO消息
void MainUILogic::RecvSocketIOMsg(char *msg) {
   if (msg == nullptr) {
      return;
   }
   QByteArray text(msg);
   string base64EncodeData = text.toBase64().toStdString();
   QString method = QString("qtCallJsSocketIoSendMsg('%1')").arg(QString::fromStdString(base64EncodeData));
   if (m_pLivePlugInUnitDlg) {
      m_pLivePlugInUnitDlg->executeJSCode(method);
   }
   QCoreApplication::postEvent(this, new CustomRecvMsgEvent(CustomEvent_RecvMsg, method));
}

void MainUILogic::AppExit(bool bRestart) {
   TRACE6("MainUILogic::AppExit.\n");
   if (m_pLiveMainDlg) {
      m_pLiveMainDlg->hide();
      m_pLiveMainDlg->SetStartLiveState(false);
   }

   HidePluginDataDig();
   if (m_pLiveListWdg) {
      m_pLiveListWdg->hide();
   }

   if (m_pgNameCore) {
      if (*m_pgNameCore) {
         CloseHandle(*m_pgNameCore);
         *m_pgNameCore = NULL;
      }
   }

   if (bRestart) {
      QCoreApplication::postEvent(this, new QEvent(CustomEvent_ShowLoginDlg));
   }
   return;
}
//设置logo是否隐藏
void MainUILogic::DoHideLogo(bool bHide) {
   VH::CComPtr<IOBSControlLogic> pObsControlLogic = NULL;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IOBSControl, IID_IOBSControlLogic, pObsControlLogic, return, ASSERT(FALSE));
   if (pObsControlLogic) {
      pObsControlLogic->DoHideLogo(bHide);
   }
}
void MainUILogic::AskForOpenWebPage(QString sureBtn, QString cancelBtn,
   QString title, QString content, QString url) {
   emit this->SigAskForOpenWebPage(sureBtn, cancelBtn,
      title, content, url);
}
void MainUILogic::SlotAskForOpenWebPage(QString sureBtn, QString cancelBtn, QString title, QString content, QString url) {
   AlertTipsDlg tip(content, true, m_pLiveMainDlg);
   tip.CenterWindow(m_pLiveMainDlg);
   tip.SetYesBtnText(sureBtn);
   tip.SetNoBtnText(cancelBtn);
   if (tip.exec() == QDialog::Accepted) {
      QDesktopServices::openUrl(url);
   }
}

void MainUILogic::KitoutEvent(bool bkitout, wchar_t *userId, wchar_t *role) {
   emit SigKikoutEvent(bkitout, QString::fromWCharArray(userId), QString::fromWCharArray(role));
}

void MainUILogic::OnlineEvent(bool bonline, wchar_t *userId, wchar_t *role, wchar_t* name, int synType) {
   emit SigOnlineEvent(bonline, QString::fromWCharArray(userId), QString::fromWCharArray(role), QString::fromWCharArray(name), synType);
}

void MainUILogic::SlotCloseJoinActivityWnd() {
   SetEnabledLiveListWdg(true);
   if (mLoginDlg) {
      mLoginDlg->setEnabled(true);
   }
}

bool MainUILogic::IsShowMainWidget() {
   return m_showMainWidget;
}

void MainUILogic::customEvent(QEvent *event) {
   if (event) {
      int typeValue = event->type();
      TRACE6("%s typeValue %d enter\n",__FUNCTION__, typeValue);
      switch (typeValue) {
      case CustomEvent_StartLive: {
         HandleStartLive(event);
         break;
      }
      case CustomEvent_DownLoadFile: {
         HandleDownLoadHead(event);
         break;
      }
      case CustomEvent_WebSocketReconnecting: { 
         HandleWebSocketReconnecting();
         break;
      }
      case CustomEvent_CreateObsLogic: {
         HandleCreateObsLogic();
         break;
      }
      case CustomEvent_StartCreateOBS: {
         HandleStartCreateOBS();
         break;
      }
      case CustomEvent_RecordEvent:
      case CustomEvent_RecordEventErr:
         HandleRecord(event);
         break;
      case CustomEvent_JoinActivityDirect: {
         HandleJoinActiveDirect(event);
         break;
      }
      case CustomEvent_PushStreamError: {
         HandleStartRepushStream();
         break;
      }
      case CustomEvent_JoinActivityInit: {
         HandleJoinActivityInit(event);
         break;
      }
      case CustomEvent_JoinActivityByPwd: {
         HandleJoinActivityByPwd(event);
         break;
      }
      case CustomEvent_CreateRecord: {
         HandleCreateRecord(event);
         break;
      }
      case CustomEvent_Default_Record: {
         HandleDefaultRecord(event);
         break;
      }
      case CustomEvent_ShowMainWdg: {
         HandleShowMainWdg();
         break;
      }
      case CustomEvent_StopWebniar: {
         HandleStopWebniar(event);
         break;
      }
      case CustomEvent_SettingGetCameraDevSuc:
      case CustomEvent_SettingGetCameraDevErr: {
         if (mVhallIALive) {
            CustomRecvMsgEvent* msg_event = dynamic_cast<CustomRecvMsgEvent*>(event);
            if (msg_event) {
               mVhallIALive->PreviewCameraCallback(typeValue == CustomEvent_SettingGetCameraDevSuc ? true : false, msg_event->msg.toInt());
            }
         }
         break;
      }
      case CustomEvent_StartWebniar: {
         HandleStartWebniar(event);
         break;
      }
      case CustomEvent_VSSGetRoomAttributes: {
         HandleVSSGetRoomAttributes(event);
         break;
      }
      case CustomEvent_VSSGetPushInfo: {
         HandleVSSGetPushInfo(event);
         break;
      }
      case CustomEvent_Dispatch_Publish: {
         HandleDispatchPublish(event);
         break;
      }
      case CustomEvent_UploadLiveType: {
         HandleUploadLiveType(event);
         break;
      }
      case CustomEvent_ShowLoginDlg: {
         STRU_MAINUI_WIDGET_SHOW param;
         HandleShowLoginDlg(param);
         break;
      }
      case CustomEvent_VSSGetRoomBaseInfo: {
         HandleVSSGetRoomBaseInfo(event);
         break;
      }
      case CustomEvent_ExitApp: {
         if (m_pLiveListWdg) {
            m_pLiveListWdg->ToExitAppWithAnimation();
         }
         break;
      }
      case CustomEvent_RecvVssSocketIoMsg: {
         HandleRecvPaasSocketIOMsg(event);
         break;
      }
      case CustomEvent_CustomRecvSocketIOMsgEvent: {
         HandleRecvCustomRecvSocketIOMsgEvent(event);
         break;
      }
      case CustomEvent_DeleteVhallLive: {
         CustomDeleteLiveMsgEvent* delEvent = (CustomDeleteLiveMsgEvent*)event;
         if (delEvent) {
            HandleCloseVhallActive(delEvent->mbExit, delEvent->mReason);
         }
         break;
      }
      case CustomEvent_RecvMsg: {
         if (m_pLiveMainDlg) {
            m_pLiveMainDlg->HandleCustomEvent(event);
         }
         if (mVhallIALive) {
            mVhallIALive->HandleCustomEvent(event);
         }
         break;
      }
      case CustomEvent_OnRoomSuccessedEvent: {
         HandleRoomSuccessEvent(event);
         break;
      }
      case CustomEvent_OnRoomFailedEvent: {
         HandleConnectFailedEvent(event);
         break;
      }
      case CustomEvent_SendNotice: {
         HandleSendNotice(event);
         break;
      }
      case CustomEvent_SocketIOConnect: {
         HandleSocketIOConeect();
         break;
      }
      case CustomEvent_RemoteStreamAdd: {
         HandleRemoteStreamAdd(event);
         break;
      }
      case CustomEvent_SubScribedError: {
         HandleSubScribedError(event);
         break;
      }
      case CustomEvent_JsCallQtStartLive: {
         CustomRecvMsgEvent* cus_msg = dynamic_cast<CustomRecvMsgEvent*>(event);
         if (cus_msg) {
            JsCallQtStartLive(cus_msg->msg);
         }
         break;
      }
      case CustomEvent_JsCallQtShareTo: {
         CustomRecvMsgEvent* cus_msg = dynamic_cast<CustomRecvMsgEvent*>(event);
         if (cus_msg) {
            JsCallQtShareTo(cus_msg->msg);
         }
         break;
      }
      case CustomEvent_JsCallQtJoinActivity: {
         CustomRecvMsgEvent* cus_msg = dynamic_cast<CustomRecvMsgEvent*>(event);
         if (cus_msg) {
            JsCallQtJoinActivity();
         }
         break;
      }
      case CustomEvent_JsCallQtOpenUrl: {
         CustomRecvMsgEvent* cus_msg = dynamic_cast<CustomRecvMsgEvent*>(event);
         if (cus_msg) {
            JsCallQtOpenUrl(cus_msg->msg);
         }
         break;
      }
      case CustomEvent_JsCallQtStartVoiceLive: {
         CustomRecvMsgEvent* cus_msg = dynamic_cast<CustomRecvMsgEvent*>(event);
         if (cus_msg) {
            JsCallQtStartVoiceLive(cus_msg->msg);
         }
         break;
      }
      case CustomEvent_ChatMsg: {
         VH::CComPtr<ICommonData> pCommonData;
         DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
         ClientApiInitResp respInitData;
         pCommonData->GetInitApiResp(respInitData);
         if (respInitData.selectLiveType == eLiveType_Live && m_pLiveMainDlg) {
            m_pLiveMainDlg->HandleChatMsg(event);
         }
         if (mVhallIALive) {
            mVhallIALive->HandleCustomEvent(event);
         }
         break;
      }
      default: {
         //处理互动消息。 注意消息ID处理范围。
         if (typeValue >= CustomEvent_SelectGetDev && typeValue <= CustomEvent_END && mVhallIALive) {
            mVhallIALive->HandleCustomEvent(event);
         }
      }
      }
      TRACE6("%s typeValue %d Leave\n", __FUNCTION__, typeValue);
   }
}

void MainUILogic::HandleRecvCustomRecvSocketIOMsgEvent(QEvent *msgEvent) {
   CustomSocketIOMsg* socketIOMsg = dynamic_cast<CustomSocketIOMsg*>(msgEvent);
   Event event;
   if (socketIOMsg) {
      if (!AnalysisEventObject(socketIOMsg->recvObj, event)) {
         return;
      }
   }
   VH::CComPtr<ICommonData> pCommonData;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
   ClientApiInitResp respInitData;
   pCommonData->GetInitApiResp(respInitData);

   if (event.m_eMsgType == e_RQ_flashMsg && respInitData.player == FLASH_LIVE_CREATE_TYPE) {
      RecvSocketIOMsg((char*)event.context.toStdString().c_str());
   }
   else if (event.m_eMsgType == e_RQ_question_answer_open) {
      QDesktopServices::openUrl(respInitData.web_question_url);
   }
   else {
      if (eLiveType_VhallActive == mILiveType) {
         if (mVhallIALive) {
            mVhallIALive->HandleRecvSocketIOMsgEvent(event);
            if (event.m_eMsgType == e_RQ_setOver && m_pLivePlugInUnitDlg) {
               m_pLivePlugInUnitDlg->hide();
            }
         }
      }
      else {
         if (m_pLiveMainDlg) {
            m_pLiveMainDlg->HandleRecvSocketIOMsgEvent(event);
            if (event.m_eMsgType == e_RQ_setOver && m_pLivePlugInUnitDlg) {
               m_pLivePlugInUnitDlg->hide();
            }
         }
      }
   }
}

void* MainUILogic::GetCAliveDlg() {
   VH::CComPtr<ICommonData> pCommonData;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return nullptr, ASSERT(FALSE));
   ClientApiInitResp respInitData;
   pCommonData->GetInitApiResp(respInitData);
   if (respInitData.selectLiveType == eLiveType_Live) {
      return m_pLiveMainDlg;
   }
   else {
      return mVhallIALive;
   }
}

void MainUILogic::DealStreamPushSuccess() {
   m_bStartPushStreamSuccess = true;
   if (m_pPluginDataProcessor) {
      TRACE6("%s DealStreamPushSuccess\n", __FUNCTION__);
      m_pPluginDataProcessor->SetPushStreamState(true);
   }

   VH::CComPtr<ISettingLogic> pSettingLogic;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IMainUI, IID_ISettingLogic, pSettingLogic, return);
   pSettingLogic->PushStreamLineSuccess();
}

void MainUILogic::OnHttpRequestData(const char* data, int dataLen) {
   if (m_pPluginDataProcessor != NULL) {
      m_pPluginDataProcessor->OnRecvHttpData(data, dataLen);
   }
}

void MainUILogic::slot_MainDlg() {
   QCoreApplication::postEvent(this, new QEvent(CustomEvent_ShowMainWdg));
}

void MainUILogic::Slot_ClosePluginDlg() {
   if (m_pPluginDataProcessor && !m_pPluginDataProcessor->HasPushStreamState()) {
      m_pPluginDataProcessor->ClearAllData();
   }
   if (m_pLivePlugInUnitDlg) {
      m_pLivePlugInUnitDlg->hide();
   }
}

void MainUILogic::HidePluginDataDig() {
   if (m_pLivePlugInUnitDlg) {
      m_pLivePlugInUnitDlg->hide();
      m_pLivePlugInUnitDlg->Destory();
   }
   if (m_pPluginDataProcessor) {
      m_pPluginDataProcessor->ClearAllData();
   }
}

void STDMETHODCALLTYPE MainUILogic::SetHttpProxy(const bool enable /*= false*/, const char* ip /*= NULL*/, const char* port /*= NULL*/, const char* userName /*= NULL*/, const char* pwd /*= NULL*/) {
   if (enable && m_pLiveListWdg) {
      m_pLiveListWdg->SetHttpProxyInfo(enable, ip, port, userName, pwd);
   }
   else if (!enable) {
      m_pLiveListWdg->SetHttpProxyInfo(enable);
   }

   if (mAliveInteraction) {
      mAliveInteraction->SetHttpProxy(enable, ip, port, userName, pwd);
   }
}

void * STDMETHODCALLTYPE MainUILogic::GetParentWndForTips() {
   if (m_pLiveMainDlg) {
      if (m_pLiveMainDlg->isHidden()) {
         return NULL;
      }
      else {
         return (QWidget*)(m_pLiveMainDlg);
      }
   }
   return NULL;
}

void STDMETHODCALLTYPE MainUILogic::EnableVoiceTranslate(bool enable /*= false*/) {
   m_bEnableVoiceTranslate = enable;
}

bool STDMETHODCALLTYPE MainUILogic::GetVoiceTranslate() {
   return m_bEnableVoiceTranslate;
}

void STDMETHODCALLTYPE MainUILogic::OpenVoiceTranslateFun(bool open /*= false*/) {
   m_bOpenVoiceTranslateSelect = open;
   QString qsConfPath = CPathManager::GetConfigPath();
   ConfigSetting::writeValue(qsConfPath, GROUP_DEFAULT, KEY_OPEN_VT, m_bOpenVoiceTranslateSelect == true ? 1 : 0);
}

bool STDMETHODCALLTYPE MainUILogic::IsOpenVoiceTranslateFunc() {
   return m_bOpenVoiceTranslateSelect;
}

void MainUILogic::DealVoiceTranslate() {
   bool bEnableVT = false;
   VH::CComPtr<IMainUILogic> pMainUILogic;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IMainUI, IID_IMainUILogic, pMainUILogic, return, ASSERT(FALSE));
   if (pMainUILogic && pMainUILogic->GetVoiceTranslate() && pMainUILogic->IsOpenVoiceTranslateFunc() && m_pLiveMainDlg) {
      m_pLiveMainDlg->FadeOutTip(OPEN_VOICE_TRANSLATE, TipsType_Success, 2000);
      bEnableVT = true;
   }
   else if (pMainUILogic && pMainUILogic->GetVoiceTranslate() && !pMainUILogic->IsOpenVoiceTranslateFunc() && m_pLiveMainDlg) {
      m_pLiveMainDlg->FadeOutTip(CLOSE_VOICE_TRANSLATE, TipsType_Success, 2000);
      bEnableVT = false;
   }
   TRACE6("%s bEnableVT:%d\n", __FUNCTION__, bEnableVT);
   StartVoiceTranslate(bEnableVT);
}

void MainUILogic::StartVoiceTranslate(bool open) {
   if (m_pLiveMainDlg) {
      m_pLiveMainDlg->ShowVoiceTranslateWdg(open);
   }
   STRU_VT_INFO info;
   QString qsConfPath = CPathManager::GetConfigPath();
   info.fontSize = ConfigSetting::ReadInt(qsConfPath, GROUP_DEFAULT, KEY_VT_FONT_SIZE, 15);
   QString currentLan = ConfigSetting::ReadString(qsConfPath, GROUP_DEFAULT, KEY_VT_LANGUAGE, QString::fromStdWString(TextLanguage_Mandarin));
   if (currentLan.compare(QString::fromStdWString(TextLanguage_Mandarin)) == 0) {
      info.lan = LAN_TYPE_PTH;
   }
   else if (currentLan.compare(QString::fromStdWString(TextLanguage_Cantonese)) == 0) {
      info.lan = LAN_TYPE_YU;
   }
   else if (currentLan.compare(QString::fromStdWString(TextLanguage_Lmz)) == 0) {
      info.lan = LAN_TYPE_SC;
   }
   info.bEnable = open;
   //设置回显字体大小
   if (m_pLiveMainDlg) {
      m_pLiveMainDlg->SetVoiceTransitionFontSize(info.fontSize);
      m_pLiveMainDlg->SetVoiceTransitionText(QString());
   }
   SingletonMainUIIns::Instance().PostCRMessage(MSG_OBSCONTROL_ENABLE_VT, &info, sizeof(STRU_VT_INFO));
}

void MainUILogic::DealVoiceTransition(void *apData, DWORD adwLen) {
   DEF_CR_MESSAGE_DATA_DECLARE_CA(VOICE_TRANS_MSG, loMessage, return);
   if (m_pLiveMainDlg) {
      QString text = QString::fromWCharArray(loMessage.data);
      if (!m_pDesktopSharingUI->isVisible() && IsOpenVoiceTranslateFunc() && m_pLiveMainDlg->isVisible() && !m_pLiveMainDlg->isMinimized()) {
         m_pLiveMainDlg->SetVoiceTransitionText(text);
      }
   }
}

void MainUILogic::DealRecordChange(void* apData, DWORD adwLen) {
   DEF_CR_MESSAGE_DATA_DECLARE_CA(RECORD_STATE_CHANGE, loMessage, return);
   if (m_pLiveMainDlg) {
      m_pLiveMainDlg->SetRecordState(loMessage.iState);
      if (loMessage.iState == eRecordState_Stop && !IsRecordBtnhide() && loMessage.nLiveTime > 30) {
         m_pLiveMainDlg->FadeOutTip(QString::fromUtf8("直播结束后，打点录制的直播将自动生成回放"), TipsType_Normal);
      }
   }
}

void STDMETHODCALLTYPE MainUILogic::SetVoiceTranslateFontSize(int size) {
   if (m_pLiveMainDlg) {
      m_pLiveMainDlg->SetVoiceTransitionFontSize(size);
   }
}

void MainUILogic::SetEnabledLiveListWdg(bool enable) {
   if (m_pLiveListWdg) {
      m_pLiveListWdg->setEnabled(enable);
      m_bEnableStartLive = enable;
   }
}

void MainUILogic::Slot_DisableWebListDlg() {
   SetEnabledLiveListWdg(false);
}

void MainUILogic::Slot_EnableWebListDlg() {
   SetEnabledLiveListWdg(true);
}

void MainUILogic::slotCameraClicked() {
   mbCameraListShow = !mbCameraListShow;
   if (!mbCameraListShow) {
      return;
   }

   STRU_MAINUI_CLICK_CONTROL loControl;
   loControl.m_eType = control_AddCamera;
   QPoint globalPoint = m_pDesktopSharingUI->GetCameraPos();
   loControl.m_globalX = globalPoint.x() /*+ m_pCameraButton->width() / 2*/;
   loControl.m_globalY = globalPoint.y() + m_pDesktopSharingUI->GetCameraHeight();
   loControl.m_dwExtraData = mbCameraListShow ? 1 : 0;
   SingletonMainUIIns::Instance().PostCRMessage(MSG_MAINUI_DESKTOP_SHOW_CAMERALIST, &loControl, sizeof(STRU_MAINUI_CLICK_CONTROL));
}

void MainUILogic::slotSettingClicked() {
   SingletonMainUIIns::Instance().PostCRMessage(MSG_MAINUI_DESKTOP_SHOW_SETTING, NULL, 0);
}

void MainUILogic::slotLiveClicked() {
   TRACE6("%s Desktop\n", __FUNCTION__);
   if (m_pDesktopSharingUI) {
      STRU_MAINUI_CLICK_CONTROL loControl;
      loControl.m_eType = control_StartLive;
      loControl.m_dwExtraData = m_pDesktopSharingUI->GetStreamStatus() ? 0 : 1;
      m_pDesktopSharingUI->SetStartLiveBtnEnable(false);
      //QJsonObject body;
      //body["ls"] = QString(loControl.m_dwExtraData ? "Start" : "Stop");//开启/关闭操作
      //SingletonMainUIIns::Instance().reportLog(STR_VH_LIVE_OPERATE, eLogRePortK_DesktopShare_StartLive, L"StartLive", body);
      SingletonMainUIIns::Instance().PostCRMessage(MSG_MAINUI_CLICK_CONTROL, &loControl, sizeof(STRU_MAINUI_CLICK_CONTROL));
      if (loControl.m_dwExtraData == 0) {
         StopDesktopSharing();
      }
   }
   TRACE6("%s Desktop end\n", __FUNCTION__);
}

void MainUILogic::slotMicClicked() {
   STRU_MAINUI_MUTE loMute;
   loMute.m_eType = mute_Mic;
   loMute.m_bMute = !m_pDesktopSharingUI->GetMicMute();
   QJsonObject body;
   body["vb"] = QString(loMute.m_bMute ? "Open" : "Close");//开启/关闭操作
   SingletonMainUIIns::Instance().reportLog(STR_VH_LIVE_OPERATE, eLogRePortK_DesktopShare_Mic, L"Mic", body);
   SingletonMainUIIns::Instance().PostCRMessage(MSG_MAINUI_MUTE, &loMute, sizeof(loMute));
}

void MainUILogic::slot_OnShareSelect() {
   VHD_Window_Enum_init(VHD_Desktop);
   vector<VHD_WindowInfo> allDesktop = VHD_DesktopWindowInfo();
   if (allDesktop.size() > 1) {
      if (ShowScreenSelectDlg(allDesktop, m_pDesktopSharingUI) == QDialog::DialogCode::Rejected) {
         TRACE6("%s Rejected\n", __FUNCTION__);
         return;
      }
   }
   TRACE6("%s accept\n", __FUNCTION__);
}

void MainUILogic::slot_OnClickedPluginUrlFromDesktop(bool isReload, const int& iX, const int& iY) {
   VH::CComPtr<ICommonData> pCommonData;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
   ClientApiInitResp respInitData;
   pCommonData->GetInitApiResp(respInitData);
   if (respInitData.selectLiveType == eLiveType_Live) {
      if (m_pLiveMainDlg) {
         isReload = m_pLiveMainDlg->IsFirstLoadTool();
      }
   }
   if (respInitData.player == FLASH_LIVE_CREATE_TYPE) {
      ReloadPlguinUrl(isReload);
   }
   else {
      if (respInitData.selectLiveType == eLiveType_Live) {
         if (m_pLiveMainDlg && m_pLivePlugInUnitDlg) {
            if (m_pLivePlugInUnitDlg->isVisible())
               m_pLivePlugInUnitDlg->hide();
            else
               m_pLivePlugInUnitDlg->show(QApplication::desktop()->availableGeometry(m_pDesktopSharingUI));
         }
      }
      else {
         if (mVhallIALive && m_pLivePlugInUnitDlg) {
            if (m_pLivePlugInUnitDlg->isVisible())
               m_pLivePlugInUnitDlg->hide();
            else
               m_pLivePlugInUnitDlg->show(QApplication::desktop()->availableGeometry(mVhallIALive));
         }
      }
      if (isReload) {
         m_pLivePlugInUnitDlg->ReloadPluginUrl();
      }
   }
}

bool STDMETHODCALLTYPE MainUILogic::IsShareDesktop() {
   if (m_pDesktopSharingUI && m_pDesktopSharingUI->isHidden()) {
      return false;
   }
   else if (m_pDesktopSharingUI && !m_pDesktopSharingUI->isHidden()) {
      return true;
   }
   return false;
}

void MainUILogic::HandleAppExit(bool  isNotice) {
   TRACE6("%s isNotice:%d \n", __FUNCTION__, isNotice);
   if (isNotice) {
      wstring confPath = GetAppDataPath() + VHALL_TOOL_CONFIG;
      QString qsConfPath = qsConfPath.fromStdWString(confPath);
      QString vhallHelper = ConfigSetting::ReadString(qsConfPath, GROUP_DEFAULT, KEY_VHALL_HELPER, VHALL_LIVE_HELPER);
      AlertTipsDlg alert(ASK_EXIT + vhallHelper, true, nullptr);
      alert.CenterWindow(m_pLiveListWdg);
      if (alert.exec() != QDialog::Accepted) {
         return;
      }
      if (m_pLiveListWdg) {
         m_pLiveListWdg->ReleaseWebEngineView();
      }
      QCoreApplication::postEvent(this, new QEvent(CustomEvent_ExitApp));
      return;
   }
   PostEventToExitApp();
}

void MainUILogic::HandleCloseMainLiveWnd() {
   TRACE6("%s \n", __FUNCTION__);
   //是否正在直播
   PublishInfo loStreamInfo;
   VH::CComPtr<ICommonData> pCommonData;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
   bool bIsCurPublishing = false;
   if (pCommonData) {
      pCommonData->GetStreamInfo(loStreamInfo);
      bIsCurPublishing = pCommonData->GetPublishState();
   }

   //是否在设置中默认勾选回放？
   VH::CComPtr<ISettingLogic> pSettingLogic;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IMainUI, IID_ISettingLogic, pSettingLogic, return, ASSERT(FALSE));
   bool is_Setting_Default_Playback = pSettingLogic->GetIsServerPlayback();

   //退出框文本内容
   QString qsContent;
   QString qsSureBtn = DETERMINE;
   wstring confPath = GetAppDataPath() + VHALL_TOOL_CONFIG;
   QString qsConfPath = qsConfPath.fromStdWString(confPath);
   QString vhallHelper = ConfigSetting::ReadString(qsConfPath, GROUP_DEFAULT, KEY_VHALL_HELPER, VHALL_LIVE_HELPER);
   if (bIsCurPublishing) {
      if (m_bHasList) {
         qsContent = LIVING_CLOSE;
         qsSureBtn = STOP_LIVING;
      }
      else {
         qsContent = LIVING_CLOSE_APP + vhallHelper + QString("?");
      }
   }
   else {
      if (m_bHasList) {
         qsContent = EXIT_LIVING_PAGE;
      }
      else {

         qsContent = ASK_EXIT + vhallHelper;
      }
   }
   TRACE6("MainUILogic::CloseApp \n");
   int iResult = -1;
   //退出框操作
   int execRet = -1;
   AlertTipsDlg *alertDlg = NULL;
   if (m_bLoadExtraRightWidget && bIsCurPublishing) {
      alertDlg = new AlertTipsDlg(qsContent, true, m_pLiveMainDlg);
      alertDlg->CenterWindow(m_pLiveMainDlg);
      alertDlg->SetYesBtnText(qsSureBtn);
      execRet = alertDlg->exec();
      if (execRet == QDialog::Accepted) {
         if (m_bLoadExtraRightWidget) {
            ShowExitWaiting();
            StopWebnair(true);
            if (alertDlg) {
               delete alertDlg;
               alertDlg = nullptr;
            }
            return;
         }
      }
   }
   else {
      alertDlg = new AlertTipsDlg(qsContent, true, m_pLiveMainDlg);
      alertDlg->raise();
      execRet = alertDlg->exec();
   }
   if (m_pLiveMainDlg) {
      m_pLiveMainDlg->BeHide();
   }
   if (execRet == QDialog::Accepted) {
      CloseTurnToLiveListDlg();
   }

   if (alertDlg) {
      delete alertDlg;
      alertDlg = nullptr;
   }
}

void MainUILogic::ShowExitWaiting() {
   if (m_pExitWaiting) {
      m_pExitWaiting->Show();
   }
}
void MainUILogic::CloseExitWaiting() {
   if (m_pExitWaiting) {
      m_pExitWaiting->Close();
   }
}

void MainUILogic::CloseTurnToLiveListDlg() {
   TRACE6("CloseTurnToLiveListDlg\n");
   HidePluginDataDig();
   if (IsOpenVoiceTranslateFunc()) {
      StartVoiceTranslate(false);
      EnableVoiceTranslate(false);
   }
   if (m_pLiveMainDlg) {
      m_pLiveMainDlg->hide();
      m_pLiveMainDlg->ReleaseDocWebView();
      m_pLiveMainDlg->ReleaseChatWebView();
      m_pLiveMainDlg->ClearWebinarName();
      m_pLiveMainDlg->ClearMemberList();
   }
   
   VH::CComPtr<IOBSControlLogic> pObsControlLogic = NULL;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IOBSControl, IID_IOBSControlLogic, pObsControlLogic, return, ASSERT(FALSE));
   if (pObsControlLogic) {
      TRACE6("%s MediaStop start\n", __FUNCTION__);
      pObsControlLogic->MediaStop();
      TRACE6("%s MediaStop end\n", __FUNCTION__);
      InteractAPIManager apiManager(this);
      apiManager.HttpSendChangeWebWatchLayout(0);
      StopPlayMediaFile();
      TRACE6("%s StopPlayMediaFile end\n", __FUNCTION__);
   }
   SingletonMainUIIns::Instance().PostCRMessage(MSG_MAINUI_CLOSE_SYS_SETTINGWND, NULL, 0);
   if (m_pAreaMask) {
      delete m_pAreaMask;
      m_pAreaMask = nullptr;
   }

   pObsControlLogic->ClearAllSource(true);
   if (m_bHasList && m_pLiveMainDlg) {
      m_pLiveMainDlg->SetStartLiveState(false);
   }
   TRACE6("%s m_pLiveMainDlg hide\n", __FUNCTION__);

   VH::CComPtr<ICommonData> pCommonData;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
   pCommonData->ReportEvent("gb00");

   SingletonMainUIIns::Instance().PostCRMessage(MSG_MAINUI_DO_CLOSE_MAINWINDOW, NULL, 0);

   if (m_pLiveMainDlg) {
      m_pLiveMainDlg->StartTimeClock(FALSE);
      m_pLiveMainDlg->SetStreamButtonStatus(false);
   }
   if (m_pDesktopSharingUI) {
      m_pDesktopSharingUI->SetStreamButtonStatus(false);
   }

   if (m_pLiveMainDlg) {
      m_pLiveMainDlg->ReInit();
      m_pLiveMainDlg->DestroyUI();
   }

   if (m_pDesktopSharingUI) {
      delete m_pDesktopSharingUI;
      m_pDesktopSharingUI = nullptr;
   }
   if (m_pExitWaiting) {
      delete m_pExitWaiting;
      m_pExitWaiting = nullptr;
   }
   if (m_pStartWaiting) {
      delete m_pStartWaiting;
      m_pStartWaiting = nullptr;
   } 
   if (m_pLiveMainDlg) {
      m_pLiveMainDlg->StopCPUState();
      delete m_pLiveMainDlg;
      m_pLiveMainDlg = nullptr;
   }
   m_pCaptureSrcUI = nullptr;
   m_pFullScreenLabel = nullptr;
   if (mThreadToopTask) {
      mThreadToopTask->ClearAllWork();
   }
   TRACE6("CloseTurnToLiveListDlg end\n");
   VH::CComPtr<IDeviceManager> pDeviceManager;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IOBSControl, IID_IDeviceManager, pDeviceManager, return, ASSERT(FALSE));
   pDeviceManager->Destroy();
   TRACE6("pDeviceManager Destroy\n");
   pObsControlLogic->SetAudioCapture(false);
   pObsControlLogic->Destroy();
   TRACE6("pObsControlLogic Destroy\n");
   VH::CComPtr<ISettingLogic> pSettingLogic;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IMainUI, IID_ISettingLogic, pSettingLogic, return);
   pSettingLogic->Destroy();
   TRACE6("pSettingLogic Destroy\n");
}

void MainUILogic::SetDefaultRecord(bool closeMainUi) {
   TRACE6("%s\n", __FUNCTION__);
   QString domain = ConfigSetting::ReadString(CPathManager::GetToolConfigPath(), GROUP_DEFAULT, KEY_URL_DOMAIN, domainUrl);
   QString url = m_msgUrl + "createrecord?token=" + m_msgToken;
   TRACE6("%s url:%s\n", __FUNCTION__, url.toStdString().c_str());
   //代理配置
   QString configPath = CPathManager::GetConfigPath();
   QString proxyHost;
   QString proxyUsr;
   QString proxyPwd;
   int proxyPort;
   int is_http_proxy = ConfigSetting::ReadInt(configPath, GROUP_DEFAULT, PROXY_OPEN, 0);
   if (is_http_proxy) {
      TRACE6("%s is_http_proxy:%d\n", __FUNCTION__, is_http_proxy);
      proxyHost = ConfigSetting::ReadString(configPath, GROUP_DEFAULT, PROXY_HOST, "");
      proxyPort = ConfigSetting::ReadInt(configPath, GROUP_DEFAULT, PROXY_PORT, 80);
      proxyUsr = ConfigSetting::ReadString(configPath, GROUP_DEFAULT, PROXY_USERNAME, "");
      proxyPwd = ConfigSetting::ReadString(configPath, GROUP_DEFAULT, PROXY_PASSWORD, "");
   }

   QObject *recvObj = this;
   HTTP_GET_REQUEST request(url.toStdString());
   GetHttpManagerInstance()->HttpGetRequest(request, [&, recvObj, closeMainUi](const std::string& msg, int code, const std::string userData) {
      TRACE6("%s msg %s\n", __FUNCTION__, msg.c_str());
      QHttpResponeEvent *custom_event = new QHttpResponeEvent(CustomEvent_CreateRecord, code, QString::fromStdString(msg));
      if (custom_event) {
         custom_event->mIsClose = closeMainUi;
         QCoreApplication::postEvent(recvObj, custom_event);
      }
   });
   TRACE6("%s end\n", __FUNCTION__);
}

void MainUILogic::HandleRecvPaasSocketIOMsg(QEvent *event) {
   CustomOnHttpResMsgEvent* recvMsg = dynamic_cast<CustomOnHttpResMsgEvent*>(event);
   if (recvMsg) {
      if (recvMsg->mEventCode == SocketIOMsgType_FLASH) {

      }
      else {
         Event msgEvent;
         SocketMsgParam mSocketMsgParam;
         mSocketMsgParam.SocketIOMsgParamToEvent(recvMsg->mMsg.toStdString(), &msgEvent);
      }
   }
}

void MainUILogic::ShowLoginDlg(bool* exist) {
   STRU_MAINUI_WIDGET_SHOW  param;
   param.bExit = exist;
   HandleShowLoginDlg(param);
}

void MainUILogic::HandleShowLoginDlg(const STRU_MAINUI_WIDGET_SHOW& param) {
   WCHAR szApp[1024 * 4] = L"";
   GetModuleFileNameW(NULL, szApp, 1024 * 4);
   GetCurAppVersion(QString::fromStdWString(szApp), mCurrentVersion);
   m_bEnableStartLive = true;

   TRACE6("create m_pDesktopSharingUI\n");
   if (mLoginDlg == NULL) {
      mLoginDlg = new VhallLoginWidget;
      if (mLoginDlg) {
         connect(mLoginDlg, SIGNAL(sig_ExitApp()), this, SLOT(Slot_ExitApp()));
         connect(mLoginDlg, SIGNAL(sig_Login()), this, SLOT(Slot_LoginApp()));
         connect(mLoginDlg, SIGNAL(sig_PwdLogin()), this, SLOT(Slot_PwdLogin()));
      }
   }
   TRACE6("create mLoginDlg\n");
   if (mLoginDlg) {
      mLoginDlg->SetVersion(mCurrentVersion);
      TRACE6("mLoginDlg->show()\n");
      mLoginDlg->show();
      TRACE6("mLoginDlg->raise()\n");
      mLoginDlg->raise();
      TRACE6("mLoginDlg->activateWindow()\n");
      mLoginDlg->activateWindow();
      TRACE6("mLoginDlg->EnableClickedExitApp()\n");
      mLoginDlg->EnableClickedExitApp();
      ConfigSetting::MoveWindow(mLoginDlg,nullptr);
   }
   TRACE6("MainUILogic::HandleShowLoginDlg\n");
}

void MainUILogic::Slot_ExitApp() {
   TRACE6("%s\n",__FUNCTION__);
   HandleAppExit(false);
}

void MainUILogic::Slot_LoginApp() {
   VH::CComPtr<ICommonData> pCommonData;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
   STRU_MAINUI_WIDGET_SHOW loShow;
   m_domain = ConfigSetting::ReadString(CPathManager::GetToolConfigPath(), GROUP_DEFAULT, KEY_URL_DOMAIN, domainUrl);
   m_bShowTeaching = ConfigSetting::ReadInt(CPathManager::GetToolConfigPath(), GROUP_DEFAULT, KEY_SHOW_TEACHING, 1) == 1 ? true : false;
   ConfigSetting::writeValue(CPathManager::GetToolConfigPath(), GROUP_DEFAULT, KEY_SHOW_TEACHING, 0);
   if (mLoginDlg) {
      mLoginDlg->hide();
      mbIsPwdLogin = false;
   }
   TRACE6("%s mbIsPwdLogin %d\n", __FUNCTION__, mbIsPwdLogin);
   loShow.m_eType = widget_show_type_Activites;
   SingletonMainUIIns::Instance().PostCRMessage(MSG_MAINUI_WIDGET_SHOW, &loShow, sizeof(STRU_MAINUI_WIDGET_SHOW));
   //如果启动方式不是桌面启动，则启动时便需要初始化底层设备
   int iStartMode = 0;
   if (pCommonData) {
      iStartMode = pCommonData->GetStartMode();
   }
   UploadInitInfo();
   //if (iStartMode != eStartMode_desktop) {
   //   SingletonMainUIIns::Instance().PostCRMessage(MSG_COMMONDATA_DATA_INIT, NULL, 0);
   //}
}

void MainUILogic::Slot_PwdLogin() {
   HandleJoinAcivity(true);
   mbIsPwdLogin = true;
   TRACE6("%s mbIsPwdLogin %d\n", __FUNCTION__, mbIsPwdLogin);
}

void MainUILogic::slot_hideCaptureSrcUi() {
   if (m_pLiveMainDlg) {
      m_pLiveMainDlg->HideCaptureSrcUi();
   }
}

void MainUILogic::HandleJoinAcivity(bool bPwdLogin) {
   qDebug() << "MainUILogic::JoinActivity()";
   SetEnabledLiveListWdg(false);
   if (m_pJoinActivity == NULL) {
      m_pJoinActivity = new VhallJoinActivity();
      if (m_pJoinActivity) {
         connect(m_pJoinActivity, SIGNAL(SigJoinActivity(QString, QString, QString, bool)), this, SLOT(SlotToJoinActivity(QString, QString, QString, bool)));
         connect(m_pJoinActivity, SIGNAL(SigClose()), this, SLOT(SlotCloseJoinActivityWnd()));
         connect(this, SIGNAL(SigJoinActivetyEnd(bool, QString)), m_pJoinActivity, SLOT(SlotJoinActivityEnd(bool, QString)));
      }
   }

   if (NULL != m_pJoinActivity) {
      m_pJoinActivity->SetPwdLogin(bPwdLogin);
      m_pJoinActivity->setDefultNick(QString());
      m_pJoinActivity->CenterWindow(bPwdLogin == true ? mLoginDlg : (QWidget*)m_pLiveListWdg);
      m_pJoinActivity->show();
      if (mLoginDlg) {
         mLoginDlg->setEnabled(false);
      }
   }
}

void MainUILogic::ToJoinActivityDirect(QString id, QString nick, QString password, bool isPwdLogin, QString token) {
   VH::CComPtr<ICommonData> pCommonData = NULL;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData);
   if (m_pPluginHttpServer && !nick.isEmpty()) {
      char *encodeUrl = m_pPluginHttpServer->HttpUrlEncode(nick.toStdString().c_str(), nick.toStdString().length(), false);
      if (encodeUrl != NULL) {
         nick = QString(encodeUrl);
         free(encodeUrl);
         encodeUrl = NULL;
      }
   }
   QString url = m_domain + "/api/vhallassistant/webinar/join-webinar?secure=0&webinar_id=" + id + "&nick_name=" + nick + "&password=" + password + "&params_verify_token=" + token;
   TRACE6("%s url:%s\n", __FUNCTION__, url.toStdString().c_str());
   QString configPath = CPathManager::GetConfigPath();
   QString proxyHost;
   QString proxyUsr;
   QString proxyPwd;
   int proxyPort;
   int is_http_proxy = ConfigSetting::ReadInt(configPath, GROUP_DEFAULT, PROXY_OPEN, 0);
   if (is_http_proxy) {
      TRACE6("%s is_http_proxy:%d\n", __FUNCTION__, is_http_proxy);
      proxyHost = ConfigSetting::ReadString(configPath, GROUP_DEFAULT, PROXY_HOST, "");
      proxyPort = ConfigSetting::ReadInt(configPath, GROUP_DEFAULT, PROXY_PORT, 80);
      proxyUsr = ConfigSetting::ReadString(configPath, GROUP_DEFAULT, PROXY_USERNAME, "");
      proxyPwd = ConfigSetting::ReadString(configPath, GROUP_DEFAULT, PROXY_PASSWORD, "");
   }
   QObject *recvObj = this;
   HTTP_GET_REQUEST request(url.toStdString());
   GetHttpManagerInstance()->HttpGetRequest(request, [&, id, nick, password, isPwdLogin, recvObj](const std::string& msg, int code, const std::string userData) {
      QCoreApplication::postEvent(recvObj, new QHttpJoinActiveResponeEvent(CustomEvent_JoinActivityDirect, code, QString::fromStdString(msg), id, nick, password, isPwdLogin));
   });
   //模式选择日志上报
   QJsonObject body;
   body["id"] = id;
   body["nick"] = nick;
   SingletonMainUIIns::Instance().reportLog(STR_VH_LIVE_OPERATE, eLogRePortK_JoinActivity, L"JoinActivity", body);
}

void MainUILogic::ReleaseCommonData() {
   TRACE6("%s \n", __FUNCTION__);
   if (m_pCommonInfo) {
      delete m_pCommonInfo;
      m_pCommonInfo = NULL;
   }
   TRACE6("%s end\n", __FUNCTION__);
}

bool MainUILogic::ToJoinActivityByLogin(QString roomID, QString nickName, QString roomPwd) {
   if (m_pPluginHttpServer && !nickName.isEmpty()) {
      char *encodeUrl = m_pPluginHttpServer->HttpUrlEncode(nickName.toStdString().c_str(), nickName.toStdString().length(), false);
      if (encodeUrl != NULL) {
         nickName = QString(encodeUrl);
         free(encodeUrl);
         encodeUrl = NULL;
      }
   }
   QString configPath = CPathManager::GetConfigPath();
   m_domain = ConfigSetting::ReadString(CPathManager::GetToolConfigPath(), GROUP_DEFAULT, KEY_URL_DOMAIN, domainUrl);
   QString clientUnique = ConfigSetting::ReadString(configPath, GROUP_DEFAULT, USER_PWD_LOGIN_CLIENT_UNIQUE, "");
   QString url = m_domain + "/api/vhallassistant/auth/passwordlogin?webinar_id=%0&password=%1&nick_name=%2&client_unique=%3";
   url = url.arg(roomID).arg(roomPwd).arg(nickName).arg(clientUnique);

   QString proxyHost;
   QString proxyUsr;
   QString proxyPwd;
   int proxyPort;
   int is_http_proxy = ConfigSetting::ReadInt(configPath, GROUP_DEFAULT, PROXY_OPEN, 0);
   if (is_http_proxy) {
      TRACE6("%s is_http_proxy:%d\n", __FUNCTION__, is_http_proxy);
      proxyHost = ConfigSetting::ReadString(configPath, GROUP_DEFAULT, PROXY_HOST, "");
      proxyPort = ConfigSetting::ReadInt(configPath, GROUP_DEFAULT, PROXY_PORT, 80);
      proxyUsr = ConfigSetting::ReadString(configPath, GROUP_DEFAULT, PROXY_USERNAME, "");
      proxyPwd = ConfigSetting::ReadString(configPath, GROUP_DEFAULT, PROXY_PASSWORD, "");
   }
   bool isPwdLogin = true;
   QObject *recvObj = this;
   HTTP_GET_REQUEST request(url.toStdString(), "");
   GetHttpManagerInstance()->HttpGetRequest(request, [&, roomID, nickName, roomPwd, isPwdLogin, recvObj](const std::string& msg, int code, const std::string userData) {
      if (recvObj) {
         QCoreApplication::postEvent(recvObj, new QHttpJoinActiveResponeEvent(CustomEvent_JoinActivityByPwd, code, QString::fromStdString(msg), roomID, nickName, roomPwd, isPwdLogin));
      }
   });
   return true;
}

bool STDMETHODCALLTYPE MainUILogic::IsInteractive() {
   return mIsInteractive;
}

QString STDMETHODCALLTYPE MainUILogic::StrPointURL() {
   QString domain = ConfigSetting::ReadString(CPathManager::GetToolConfigPath(), GROUP_DEFAULT, KEY_URL_DOMAIN, domainUrl);
   VH::CComPtr<ICommonData> pCommonData;
   ClientApiInitResp respInitData;
   pCommonData->GetInitApiResp(respInitData);
   return respInitData.vod_create_url;//打点录制连接
}

void MainUILogic::DeaPointRecordRs(void* apData, DWORD adwLen) {
   DEF_CR_MESSAGE_DATA_DECLARE_CA(STRU_HTTPCENTER_HTTP_RS, loMessage, return);
   std::map<DWORD, HttpTaskWorkThread*>::iterator iter = mHttpTaskThreadList.find(loMessage.m_uuid);
   if (iter != mHttpTaskThreadList.end()) {
      if (iter->second != NULL && iter->second->isFinished()) {
         delete iter->second;
         mHttpTaskThreadList.erase(iter);
         iter = mHttpTaskThreadList.begin();
         TRACE6("%s delete HttpTaskWorkThread\n", __FUNCTION__);
      }
   }

   QString strReqUrl = QString::fromWCharArray(loMessage.m_wzRequestUrl);
   qDebug() << strReqUrl << endl;
   int iPos = strReqUrl.indexOf("cut-record");
   if (iPos >= 0) {//判断是  直播录制
      QJsonObject jeson = CPathManager::GetJsonObjectFromString(QString::fromWCharArray(loMessage.m_wzUrlData));
      int iCode = jeson["code"].toInt();
      QString strMsg = jeson["msg"].toString();
      if (200 == iCode) {
         QJsonObject datajeson = jeson["data"].toObject();
         QString strSend = datajeson["send"].toString();
         if (1 == strSend.toInt())//  0 表示非互动/直播  1表示互动
         {
            int istatus = datajeson["status"].toString().toInt();
            int iLocalRecordState = eRecordState_Stop;
            switch (istatus) {
            case eRecordReTyp_Start: {
               iLocalRecordState = eRecordState_Recording;
            }
                                     break;
            case eRecordReTyp_Suspend: {
               iLocalRecordState = eRecordState_Suspend;
            }
                                       break;
            default: {//eRecordReTyp_Stop =3,			//结束
               iLocalRecordState = eRecordState_Stop;
            }
                     break;
            }
            if (iLocalRecordState == eRecordState_Stop && !IsRecordBtnhide()) {
               m_pLiveMainDlg->FadeOutTip(STOP_LIVE_RECORED, TipsType_Normal);
            }
         }
      }
      else {
         wchar_t strw[1024] = { 0 };
         strMsg.toWCharArray(strw);
         FadoutTip(strw);
         TRACE6("%ws:%ws\n", strw, loMessage.m_wzRequestUrl);
      }
   }
   //微吼互动直播，设备操作上报事件回调处理。
   else if (loMessage.bActiveApi) {
      TRACE6("%s OnRecvHttpResponse start\n", __FUNCTION__);
      if (QString::fromWCharArray(loMessage.m_wzUrlData).isEmpty()) {
         QString strUid = QString::fromStdString(loMessage.m_uid);
         bool bNotice = loMessage.m_dwPluginId == ENABLE_NOTICE_ERR_MSG ? true : false;
         QCoreApplication::postEvent(this, new CustomOnHttpResMsgEvent(CustomEvent_OnRecvHttpResponse, strReqUrl, "", -1, strUid, bNotice));
      }
      else {
         if (strReqUrl.indexOf("speaker-list") > 0) {
            QJsonObject jeson = CPathManager::GetJsonObjectFromString(QString::fromWCharArray(loMessage.m_wzUrlData));
            QString strCode = CPathManager::GetObjStrValue(jeson, "code");
            int nCode = strCode.toInt();
            QString strMsg = jeson["msg"].toString();
            QString strMsgData = QString::fromWCharArray(loMessage.m_wzUrlData);
            QString strUid = QString::fromStdString(loMessage.m_uid);
            bool bNotice = loMessage.m_dwPluginId == ENABLE_NOTICE_ERR_MSG ? true : false;
            QCoreApplication::postEvent(this, new CustomOnHttpResMsgEvent(CustomEvent_OnRecvHttpResponse, strReqUrl, strMsg, nCode, strUid, bNotice, strMsgData, loMessage.bHasVideo));
         }
         else {
            QJsonObject jeson = CPathManager::GetJsonObjectFromString(QString::fromWCharArray(loMessage.m_wzUrlData));
            QString strCode = CPathManager::GetObjStrValue(jeson, "code");
            int nCode = strCode.toInt();
            QString strMsg = jeson["msg"].toString();
            QString strUid = QString::fromStdString(loMessage.m_uid);
            bool bNotice = loMessage.m_dwPluginId == ENABLE_NOTICE_ERR_MSG ? true : false;
            QCoreApplication::postEvent(this, new CustomOnHttpResMsgEvent(CustomEvent_OnRecvHttpResponse, strReqUrl, strMsg, nCode, strUid, bNotice));
         }
      }
      TRACE6("%s OnRecvHttpResponse end\n", __FUNCTION__);
   }
}

bool STDMETHODCALLTYPE MainUILogic::IsStopRecord() {
   bool isStoRecord = true;
   if (NULL != m_pLiveMainDlg) {
      isStoRecord = (eRecordState_Stop == m_pLiveMainDlg->GetRecordState());
   }
   return isStoRecord;
}

void STDMETHODCALLTYPE MainUILogic::SetRecordState(const int iState) {
   if (NULL != m_pLiveMainDlg) {
      m_pLiveMainDlg->SetRecordState(iState);
   }
}

void MainUILogic::DealHttpTask(void* apData, DWORD adwLen) {
   DEF_CR_MESSAGE_DATA_DECLARE_CA(STRU_HTTPCENTER_HTTP_RS, loMessage, return);
   TRACE6("%s new HttpTaskWorkThread\n", __FUNCTION__);
   QString urlStr = QString::fromWCharArray(loMessage.m_wzRequestUrl);
   QString configPath = CPathManager::GetConfigPath();
   QString proxyHost;
   QString proxyUsr;
   QString proxyPwd;
   int proxyPort;
   int is_http_proxy = ConfigSetting::ReadInt(configPath, GROUP_DEFAULT, PROXY_OPEN, 0);
   if (is_http_proxy) {
      TRACE6("%s is_http_proxy:%d\n", __FUNCTION__, is_http_proxy);
      proxyHost = ConfigSetting::ReadString(configPath, GROUP_DEFAULT, PROXY_HOST, "");
      proxyPort = ConfigSetting::ReadInt(configPath, GROUP_DEFAULT, PROXY_PORT, 80);
      proxyUsr = ConfigSetting::ReadString(configPath, GROUP_DEFAULT, PROXY_USERNAME, "");
      proxyPwd = ConfigSetting::ReadString(configPath, GROUP_DEFAULT, PROXY_PASSWORD, "");
   }
   TRACE6("%s url:%s", __FUNCTION__, urlStr.toStdString().c_str());
   HTTP_GET_REQUEST request(urlStr.toStdString());
   GetHttpManagerInstance()->HttpGetRequest(request, [&, loMessage](const std::string& msg, int code, const std::string userData) {
      if (code == 0) {
         QString qStrResult = QString::fromStdString(msg);
         STRU_HTTPCENTER_HTTP_RS aoRs;
         aoRs = loMessage;
         aoRs.m_bIsSuc = TRUE;
         aoRs.m_i64UserData = code;
         wstring strResult = qStrResult.mid(0, DEF_MAX_HTTP_URL_LEN).toStdWString();
         wcscpy_s(aoRs.m_wzUrlData, strResult.c_str());
         SingletonMainUIIns::Instance().PostCRMessage(MSG_HTTPCENTER_HTTP_RS, &aoRs, sizeof(aoRs));
      }
      else {
         STRU_HTTPCENTER_HTTP_RS aoRs;
         aoRs = loMessage;
         aoRs.m_bIsSuc = TRUE;
         aoRs.m_i64UserData = -1;
         SingletonMainUIIns::Instance().PostCRMessage(MSG_HTTPCENTER_HTTP_RS, &aoRs, sizeof(aoRs));
      }
   });
}

void MainUILogic::HandleRoomSuccessEvent(QEvent* event) {
   CustomOnHttpResMsgEvent* msgEvent = dynamic_cast<CustomOnHttpResMsgEvent*>(event);
   switch (msgEvent->mEventCode)
   {
   case RoomEvent_Login: {
      TRACE6("%s RoomEvent_Login\n", __FUNCTION__);
      HandleStartTurnToLiveMainUI(true);
      TRACE6("%s SlotOnConnected\n", __FUNCTION__);
      SlotOnConnected();
      TRACE6("%s RoomEvent_Loginenc\n", __FUNCTION__);
      break;
   }
   case RoomEvent_Start_PublishInavAnother: {
      if (mVhallIALive) {
         mVhallIALive->RoomMixStreamState(true);
      }
      break;
   }
   case RoomEvent_Stop_PublishInavAnother: {
      break;
   }
   case RoomEvent_OnNetWork_Reconnect: {
      HandleMsgReConnect();
      break;
   }
   case RoomEvent_SetLayOut: {
      if (mVhallIALive) {
         mVhallIALive->HandleSetLayout(event);
      }
      break;
   }
   default:
      break;
   }
}

void MainUILogic::HandleMsgReConnect() {
   VH::CComPtr<ICommonData> pCommonData;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
   ClientApiInitResp respInitData;
   pCommonData->GetInitApiResp(respInitData);
   if (respInitData.selectLiveType == eLiveType_VhallActive) {
      if (mVhallIALive) {
         mVhallIALive->HandleMsgReConnect();
      }
   }
}

void MainUILogic::HandleSocketIOConeect() {
   if (mVhallIALive) {
      mVhallIALive->HandleSocketIOConeect();
   }
}

void MainUILogic::HandleSubScribedError(QEvent *event) {
   if (mVhallIALive) {
      mVhallIALive->HandleSubScribedError(event);
   }
}

void MainUILogic::HandleRemoteStreamAdd(QEvent *event) {
   CustomRecvMsgEvent* msgEvent = dynamic_cast<CustomRecvMsgEvent*>(event);
   if (msgEvent) {
      TRACE6("%s msgEvent->mStreamId:%s\n", __FUNCTION__, msgEvent->mStreamId.toStdString().c_str());
      vlive::GetWebRtcSDKInstance()->SubScribeRemoteStream(msgEvent->mStreamId.toStdString());
   }
}

void MainUILogic::HandleSendNotice(QEvent* event) {
   VH::CComPtr<ICommonData> pCommonData;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
   ClientApiInitResp respInitData;
   pCommonData->GetInitApiResp(respInitData);
   if (respInitData.selectLiveType == eLiveType_Live) {
      if (m_pLiveMainDlg) {
         m_pLiveMainDlg->HandleSendNotice(event);
      }
   }
   else {
      if (mVhallIALive) {
         mVhallIALive->HandleSendNotice(event);
      }
   }
}

void MainUILogic::HandleConnectFailedEvent(QEvent* event) {
   CustomOnHttpResMsgEvent* msgEvent = dynamic_cast<CustomOnHttpResMsgEvent*>(event);
   switch (msgEvent->mEventCode)
   {
   case RoomEvent_Login: {
      TRACE6("%s\n", __FUNCTION__);
      if (msgEvent->mCode == 30009) {
         msgEvent->mMsg = ONLY_START_HOST;
      }
      AlertTipsDlg tip(msgEvent->mMsg, false, NULL);
      tip.exec();
      SetEnabledLiveListWdg(true);
      m_pWaiting->Close();
      m_bEnableStartLive = true;
      CheckPrivateStartExitApp();
      vlive::DestoryPaasSDKInstance();
      break;
   }
   case RoomEvent_Start_PublishInavAnother: {
      if (mVhallIALive) {
         mVhallIALive->RoomMixStreamState(false);
      }
      break;
   }
   case RoomEvent_Stop_PublishInavAnother: {
      //if (mVhallIALive) {
      //    mVhallIALive->HandleStopInavAnother(event);
      //}
      break;
   }
   case RoomEvent_SetLayOut: {
      if (mVhallIALive) {
         mVhallIALive->HandleSetLayout(event);
      }
      break;
   }
   default:
      break;
   }
}

void MainUILogic::HandleCloseVhallActive(bool exitToLiveList, int exitReason) {
   TRACE6("%s\n", __FUNCTION__);
   if (mVhallIALive) {
      StopPlayMediaFile();
      delete mVhallIALive;
      mVhallIALive = NULL;
   }
   TRACE6("%s  StopInteraction\n", __FUNCTION__);
   StopInteraction();
   TRACE6("%s  StopInteraction\n", __FUNCTION__);
   mMsgEventListCS.Lock();
   for (int i = 0; i < mMsgEventList.count(); i++) {
      delete mMsgEventList[i];
   }
   mMsgEventList.clear();
   mMsgEventListCS.UnLock();
   SetEnabledLiveListWdg(true);
   if (m_pLivePlugInUnitDlg) {
      m_pLivePlugInUnitDlg->hide();
   }
   m_bEnableStartLive = true;
   if (exitToLiveList) {
      TRACE6("%s exitToLiveList\n", __FUNCTION__);
      VH::CComPtr<ICommonData> pCommonData;
      DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
      ClientApiInitResp respInitData;
      pCommonData->GetInitApiResp(respInitData);
      if (pCommonData->GetStartMode() != eStartMode_desktop) {
         PostEventToExitApp();
      }
      else {
         //true代表加入互动时从互动列表处进入。退出时要退出到互动列表
         if (m_pLiveListWdg) {
            m_pLiveListWdg->show();
            m_pLiveListWdg->activateWindow();
            m_pLiveListWdg->CenterWindow(NULL);
         }
      }
   }
   else {
      TRACE6("%s not to exitToLiveList\n", __FUNCTION__);
      //false代表加入互动时从登陆入口处进入。退出时要退出到登陆界面
      STRU_MAINUI_WIDGET_SHOW param;
      HandleShowLoginDlg(param);
   }
}

bool MainUILogic::IsUploadLiveTypeSuc(int type, const QString token) {
   //http://domain/api/client/v1/interact?host-way=1&token=ajfkldsafdsa
   //int	是	1sdk互动 0非sdk互动
   QString domain = ConfigSetting::ReadString(CPathManager::GetToolConfigPath(), GROUP_DEFAULT, KEY_URL_DOMAIN, domainUrl);
   QString hostWayUrl = QString("%1/api/client/v1/interact/host-way?host_way=%2&token=%3").arg(domain).arg(type).arg(token);
   QString httpUrl = hostWayUrl;
   QString configPath = CPathManager::GetConfigPath();
   QString proxyHost;
   QString proxyUsr;
   QString proxyPwd;
   int proxyPort;
   int is_http_proxy = ConfigSetting::ReadInt(configPath, GROUP_DEFAULT, PROXY_OPEN, 0);
   if (is_http_proxy) {
      TRACE6("%s is_http_proxy:%d\n", __FUNCTION__, is_http_proxy);
      proxyHost = ConfigSetting::ReadString(configPath, GROUP_DEFAULT, PROXY_HOST, "");
      proxyPort = ConfigSetting::ReadInt(configPath, GROUP_DEFAULT, PROXY_PORT, 80);
      proxyUsr = ConfigSetting::ReadString(configPath, GROUP_DEFAULT, PROXY_USERNAME, "");
      proxyPwd = ConfigSetting::ReadString(configPath, GROUP_DEFAULT, PROXY_PASSWORD, "");
   }
   if (!httpUrl.isEmpty()) {
      HTTP_GET_REQUEST request(httpUrl.toStdString());
      request.SetHttpPost(true);
      QObject *recvObj = this;
      GetHttpManagerInstance()->HttpGetRequest(request, [&, type, httpUrl, recvObj](const std::string& msg, int code, const std::string userData) {
         if (recvObj) {
            QCoreApplication::postEvent(recvObj, new UploadLiveType(CustomEvent_UploadLiveType, code, QString::fromStdString(msg), type));
         }
      });
      TRACE6("%s mUploadLiveTypeLoop wait\n", __FUNCTION__);
      mUploadLiveTypeLoop.exec();
   }

   bool bRet = mbIsUploadLiveSuc;
   mbIsUploadLiveSuc = true;
   return bRet;
}

void MainUILogic::ClearHttpTaskThread() {
   std::map<DWORD, HttpTaskWorkThread* >::iterator iter = mHttpTaskThreadList.begin();
   while (iter != mHttpTaskThreadList.end()) {
      if (iter->second != NULL && iter->second->isFinished()) {
         delete iter->second;
         iter = mHttpTaskThreadList.erase(iter);
         TRACE6("%s delete httpTaskThread exit\n", __FUNCTION__);
      }
      else {
         iter->second->QuitEventLoop();
         iter->second->quit();
         if (iter->second->wait(5000)) {
            delete iter->second;
            iter = mHttpTaskThreadList.erase(iter);
            TRACE6("%s delete httpTaskThread exit\n", __FUNCTION__);
         }
         else {
            TRACE6("%s wait httpTaskThread exit\n", __FUNCTION__);
            iter++;
         }
      }
   }
}

#include "ISettingLogic.h"
void STDMETHODCALLTYPE MainUILogic::SetCutRecordDisplay(const int iCutRecord) {
   mIDisplyCutRecord = iCutRecord;
   VH::CComPtr<ISettingLogic> pSettingLogic;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IMainUI, IID_ISettingLogic, pSettingLogic, return);

   if (pSettingLogic){
      pSettingLogic->SetCutRecordDisplay(mIDisplyCutRecord);
   }
}

int MainUILogic::GetDisplayCutRecord() {
   return mIDisplyCutRecord;
}

bool STDMETHODCALLTYPE MainUILogic::IsRecordBtnhide()
{
   bool bHide = true;
   if (NULL != m_pLiveMainDlg)
   {
      bHide = m_pLiveMainDlg->IsRecordBtnhide();
   }
   return bHide;
}

void MainUILogic::Slot_HandleAppExit() {
   TRACE6("%s\n", __FUNCTION__);
   HandleAppExit(false);
}

void MainUILogic::RecvMsg(QJsonObject e)
{
   Event* event = new Event;
   if (event != NULL) {
      if (!AnalysisEventObject(e, *event)) {
         delete event;
         return;
      }
      mMsgEventListCS.Lock();
      mMsgEventList.push_back(event);
      mMsgEventListCS.UnLock();
      QCoreApplication::postEvent(this, new CustomRecvMsgEvent(CustomEvent_CustomRecvSocketIOMsgEvent, ""), Qt::LowEventPriority);
   }
}

void MainUILogic::GetVSSMemberList(int pageIndex) {
   VH::CComPtr<ICommonData> pCommonData;
   ClientApiInitResp initResp;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
   pCommonData->GetInitApiResp(initResp);
   QString domain = ConfigSetting::ReadString(CPathManager::GetToolConfigPath(), GROUP_DEFAULT, KEY_VSS_URL_DOMAIN, vss_domainUrl);
   VSSAPIManager vssApi;
   QObject* recvObj = this;
   vssApi.InitAPIRequest(domain.toStdString(), initResp.vss_token.toStdString(), initResp.vss_room_id.toStdString());
   if (pageIndex > 0) {
      TRACE6("%s\n", __FUNCTION__);
      vssApi.VSSGetOnlineList(pageIndex, 50, [&, recvObj](int libCurlCode, const std::string& msg, int page, int pagesize) {
         MemberParam param;
         int nCode;
         QString outMsg;
         Event memberDetail;
         memberDetail.Init();
         param.ParamToMemberInfo(libCurlCode, QString::fromStdString(msg), e_RQ_UserOnlineList, nCode, outMsg, &memberDetail);
         QJsonObject obj = VHAliveInteraction::MakeEventObject(&memberDetail);
         if (recvObj) {
            QCoreApplication::postEvent(recvObj, new CustomSocketIOMsg(CustomEvent_CustomRecvSocketIOMsgEvent, obj), Qt::LowEventPriority);
         }
      });
   }
   else {
      vssApi.VSSGetBannedList(1, 20, [&, recvObj](int libCurlCode, const std::string& msg, int page, int pagesize) {
         MemberParam param;
         int nCode;
         QString outMsg;
         Event memberDetail;
         memberDetail.Init();
         param.ParamToMemberInfo(libCurlCode, QString::fromStdString(msg), e_RQ_UserProhibitSpeakList, nCode, outMsg, &memberDetail);
         QJsonObject obj = VHAliveInteraction::MakeEventObject(&memberDetail);
         if (recvObj) {
            QCoreApplication::postEvent(recvObj, new CustomSocketIOMsg(CustomEvent_CustomRecvSocketIOMsgEvent, obj), Qt::LowEventPriority);
         }
      });

      vssApi.VSSGetKickedList(1, 20, [&, recvObj](int libCurlCode, const std::string& msg, int page, int pagesize) {
         MemberParam param;
         int nCode;
         QString outMsg;
         Event memberDetail;
         memberDetail.Init();
         param.ParamToMemberInfo(libCurlCode, QString::fromStdString(msg), e_RQ_UserKickOutList, nCode, outMsg, &memberDetail);
         QJsonObject obj = VHAliveInteraction::MakeEventObject(&memberDetail);
         if (recvObj) {
            QCoreApplication::postEvent(recvObj, new CustomSocketIOMsg(CustomEvent_CustomRecvSocketIOMsgEvent, obj), Qt::LowEventPriority);
         }
      });

      vssApi.VSSGetSpecialList(1, 20, [&, recvObj](int libCurlCode, const std::string& msg, int page, int pagesize) {
         MemberParam param;
         int nCode;
         QString outMsg;
         Event memberDetail;
         memberDetail.Init();
         param.ParamToMemberInfo(libCurlCode, QString::fromStdString(msg), e_RQ_UserSpecialList, nCode, outMsg, &memberDetail);
         QJsonObject obj = VHAliveInteraction::MakeEventObject(&memberDetail);
         if (recvObj) {
            QCoreApplication::postEvent(recvObj, new CustomSocketIOMsg(CustomEvent_CustomRecvSocketIOMsgEvent, obj), Qt::LowEventPriority);
         }
      });
 
      vssApi.VSSGetOnlineList(1, 50, [&, recvObj](int libCurlCode, const std::string& msg, int page, int pagesize) {
         MemberParam param;
         int nCode;
         QString outMsg;
         Event memberDetail;
         memberDetail.Init();
         param.ParamToMemberInfo(libCurlCode, QString::fromStdString(msg), e_RQ_UserOnlineList, nCode, outMsg, &memberDetail);
         QJsonObject obj = VHAliveInteraction::MakeEventObject(&memberDetail);
         if (recvObj) {
            QCoreApplication::postEvent(recvObj, new CustomSocketIOMsg(CustomEvent_CustomRecvSocketIOMsgEvent, obj), Qt::LowEventPriority);
         }
      });
   }
}

void MainUILogic::SlotOnConnected() {
   VH::CComPtr<ICommonData> pCommonData;
   ClientApiInitResp initResp;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
   pCommonData->GetInitApiResp(initResp);
   if (initResp.player == H5_LIVE_CREATE_TYPE) {
      GetVSSMemberList();
   }
   else {
      TRACE6("%s\n", __FUNCTION__);
      RQData vData;
      vData.m_eMsgType = e_RQ_UserOnlineList;                          //在线列表   
      vData.m_iCurPage = 1;
      SendMsg(vData.m_eMsgType, vData);
      vData.m_eMsgType = e_RQ_UserProhibitSpeakList;                 //禁言列表
      SendMsg(vData.m_eMsgType, vData);
      vData.m_eMsgType = e_RQ_UserKickOutList;                       //踢出列表
      SendMsg(vData.m_eMsgType, vData);
      vData.m_eMsgType = e_RQ_UserSpecialList;                       //主持人、嘉宾、助理
      SendMsg(vData.m_eMsgType, vData);
   }
   TRACE6("%s end\n", __FUNCTION__);
}

void MainUILogic::SlotReqApointPageUserList(const int& iApoint)
{
   VH::CComPtr<ICommonData> pCommonData;
   ClientApiInitResp initResp;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
   pCommonData->GetInitApiResp(initResp);
   if (initResp.player == H5_LIVE_CREATE_TYPE) {
      GetVSSMemberList(iApoint);
   }
   else {
      TRACE6("%s\n", __FUNCTION__);
      RQData vData;
      vData.m_eMsgType = e_RQ_UserOnlineList; //在线列表第  iApoint 数据 
      vData.m_iCurPage = iApoint;
      SendMsg(vData.m_eMsgType, vData);
      if (1 == iApoint) {
         vData.m_eMsgType = e_RQ_UserSpecialList;//主持人、嘉宾、助理
         SendMsg(vData.m_eMsgType, vData);
      }
   }
}

void MainUILogic::SlotUnInitFinished()
{
   if (this->m_pLiveListWdg) {
      this->m_pWaiting->Close();
   }
}

void MainUILogic::ControlUser(RQData &data) {
   VH::CComPtr<ICommonData> pCommonData;
   ClientApiInitResp initResp;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
   pCommonData->GetInitApiResp(initResp);
   QString domain = ConfigSetting::ReadString(CPathManager::GetToolConfigPath(), GROUP_DEFAULT, KEY_VSS_URL_DOMAIN, vss_domainUrl);
   VSSAPIManager vssApi;
   vssApi.InitAPIRequest(domain.toStdString(), initResp.vss_token.toStdString(), initResp.vss_room_id.toStdString());
   QString user_id = QString::fromStdWString(data.m_oUserInfo.userId);
   QString nickName = QString::fromStdWString(data.m_oUserInfo.userName);
   QString role_name = QString::fromStdWString(data.m_oUserInfo.role);

   QObject* recvObj = this;
   //禁言与取消禁言
   if (data.m_eMsgType == e_RQ_UserProhibitSpeakOneUser || data.m_eMsgType == e_RQ_UserAllowSpeakOneUser) {
      int status = data.m_eMsgType == e_RQ_UserProhibitSpeakOneUser ? 1 : 0;
      vssApi.VSSSetBanned(user_id.toStdString(), status, [&, user_id, nickName, role_name, status, recvObj](int libCurlCode, const std::string& msg, const std::string receive_join_id, int status) {
         TRACE6("%s libCurlCode:%d msg:%s\n", __FUNCTION__, libCurlCode, msg.c_str());
         if (recvObj) {
            QString outMsg;
            Event memberDetail;
            memberDetail.Init();

            memberDetail.m_oUserInfo.role = role_name.toStdWString();
            memberDetail.m_oUserInfo.userName = nickName.toStdWString();
            memberDetail.m_oUserInfo.userId = user_id.toStdWString();

            memberDetail.m_eMsgType = status == 1 ? e_RQ_UserProhibitSpeakOneUser : e_RQ_UserAllowSpeakOneUser;
            memberDetail.m_eventType = e_eventType_msg;
            memberDetail.context = QString::fromStdString(msg);
            memberDetail.m_bSendMsg = 1;
            QJsonObject obj = VHAliveInteraction::MakeEventObject(&memberDetail);
            if (recvObj) {
               QCoreApplication::postEvent(recvObj, new CustomSocketIOMsg(CustomEvent_CustomRecvSocketIOMsgEvent, obj), Qt::LowEventPriority);
            }
         }
      });
   }
   else if (data.m_eMsgType == e_RQ_UserKickOutOneUser || data.m_eMsgType == e_RQ_UserAllowJoinOneUser) {
      int status = data.m_eMsgType == e_RQ_UserKickOutOneUser ? 1 : 0;
      vssApi.VSSSetKicked(user_id.toStdString(), status, [&, user_id, nickName, role_name, status, recvObj](int libCurlCode, const std::string& msg, const std::string receive_join_id, int status) {
         TRACE6("%s libCurlCode:%d msg:%s\n", __FUNCTION__, libCurlCode, msg.c_str());
         if (recvObj) {
            Event memberDetail;
            memberDetail.Init();
            memberDetail.m_oUserInfo.role = role_name.toStdWString();
            memberDetail.m_oUserInfo.userName = nickName.toStdWString();
            memberDetail.m_oUserInfo.userId = user_id.toStdWString();

            memberDetail.m_eMsgType = status == 1 ? e_RQ_UserKickOutOneUser : e_RQ_UserAllowJoinOneUser;
            memberDetail.m_eventType = e_eventType_msg;
            memberDetail.context = QString::fromStdString(msg);
            memberDetail.m_bSendMsg = 1;
            QJsonObject obj = VHAliveInteraction::MakeEventObject(&memberDetail);
            if (recvObj) {
               QCoreApplication::postEvent(recvObj, new CustomSocketIOMsg(CustomEvent_CustomRecvSocketIOMsgEvent, obj), Qt::LowEventPriority);
            }
         }
      });
   }
}

void MainUILogic::SlotControlUser(RQData* RqData)
{
   VH::CComPtr<ICommonData> pCommonData;
   ClientApiInitResp initResp;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
   pCommonData->GetInitApiResp(initResp);
   if (initResp.player == H5_LIVE_CREATE_TYPE) {
      ControlUser(*RqData);
   }
   else {
      if (mAliveInteraction != nullptr) {
         mAliveInteraction->MessageRQ(RqData->m_eMsgType, RqData);
      }
   }
}


int MainUILogic::OpenPlayMediaFile(const char* szMediaFile, int liveType) {
   if (mVhallIALive && liveType == eLiveType_VhallActive) {
      return mVhallIALive->OpenPlayMediaFile(szMediaFile);
   }
   return -1;
   TRACE6("%s\n", __FUNCTION__);
}

int MainUILogic::SetPlayMediaFileVolume(int vol, int liveType) {
   if (liveType == eLiveType_VhallActive && mVhallIALive) {
      mVhallIALive->SetPlayMeidaFileVolume(vol);
   }
   return 0;
}

void MainUILogic::ClosePlayMediaFile(int liveType) {
   if (mVhallIALive && liveType == eLiveType_VhallActive) {
      mVhallIALive->StopPlayMediaFile();
   }
   TRACE6("%s\n", __FUNCTION__);
}

int MainUILogic::RestartMediaFile(int liveType) {
   int nRet = -1;
   if (mVhallIALive && liveType == eLiveType_VhallActive) {

   }
   TRACE6("%s nRet:%d\n", __FUNCTION__, nRet);
   return nRet;
}

int MainUILogic::PausePlayMediaFile(int liveType) {
   int nRet = -1;
   if (mVhallIALive && liveType == eLiveType_VhallActive) {
      nRet = mVhallIALive->PausePlayMediaFile();
   }
   TRACE6("%s nRet:%d\n", __FUNCTION__, nRet);
   return nRet;
}

int MainUILogic::ResumePlayMediaFile(int liveType) {
   int nRet = -1;
   if (mVhallIALive && liveType == eLiveType_VhallActive) {
      nRet = mVhallIALive->ResumePlayMediaFile();
   }
   TRACE6("%s nRet:%d\n", __FUNCTION__, nRet);
   return nRet;
}

int MainUILogic::SetPlayMediaFilePos(const signed long long& n64Pos, int liveType) {
   int nRet = -1;
   if (mVhallIALive && liveType == eLiveType_VhallActive) {
      nRet = mVhallIALive->SetPlayMediaFilePos(n64Pos);
   }
   TRACE6("%s nRet:%d\n", __FUNCTION__, nRet);
   return nRet;
}

int MainUILogic::GetPlayMediaFilePos(signed long long& n64Pos, signed long long& n64MaxPos, int liveType) {
   int nRet = -1;
   if (mVhallIALive && liveType == eLiveType_VhallActive) {
      nRet = mVhallIALive->GetPlayMediaFilePos(n64Pos, n64MaxPos);
   }
   return nRet;
}

int MainUILogic::GetPlayFileState(int typeLive) {
   int nRet = -1;
   if (mVhallIALive && typeLive == eLiveType_VhallActive) {
      nRet = mVhallIALive->GetPlayFileState();
   }
   return nRet;
}

void MainUILogic::SendMsg(int eMsgType, RQData& vData)
{
   if (NULL != mAliveInteraction) {
      mAliveInteraction->MessageRQ((MsgRQType)eMsgType, &vData);
   }
}

void MainUILogic::StopPlayMediaFile() {
   TRACE6("%s \n", __FUNCTION__);
   VH::CComPtr<IVedioPlayLogic> pVedioPlayLogic = NULL;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IMainUI, IID_IVedioPlayLogic, pVedioPlayLogic, return);
   if (pVedioPlayLogic) {
      TRACE6("%s StopAdmin\n", __FUNCTION__);
      pVedioPlayLogic->StopAdmin(true);
      TRACE6("%s ForceHide\n", __FUNCTION__);
      pVedioPlayLogic->ForceHide(true);
      TRACE6("%s StopPlayFile\n", __FUNCTION__);
      pVedioPlayLogic->StopPlayFile();
   }
}

void STDMETHODCALLTYPE MainUILogic::SetVedioPlayUi(QWidget* pVedioPlayUI, int iLiveType)
{
   if (mVhallIALive && iLiveType == eLiveType_VhallActive) {
      mVhallIALive->SetVedioPlayUi(pVedioPlayUI);
   }
}

void MainUILogic::HandleRecord(QEvent* event) {
   if (mVhallIALive && event) {
      mVhallIALive->HandleRecord(event);
   }
}

void MainUILogic::HandleStartLive(QEvent* event) {
   QHttpResponeEvent* startLiveEvent = dynamic_cast<QHttpResponeEvent*>(event);
   if (startLiveEvent) {
      int code = startLiveEvent->mCode;
      QString respData = startLiveEvent->mData;
      TRACE6("MainUILogic::StartLive init resp = %s\n", respData.toStdString().c_str());
      if (code == 0 && respData.length() > 0) {
         QString domain = ConfigSetting::ReadString(CPathManager::GetToolConfigPath(), GROUP_DEFAULT, KEY_URL_DOMAIN, domainUrl);
         VH::CComPtr<ICommonData> pCommonData;
         DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
         QByteArray ba(respData.toStdString().c_str(), respData.length());
         QJsonDocument doc = QJsonDocument::fromJson(ba);
         QJsonObject obj = doc.object();
         int code = obj["code"].toInt();
         QJsonObject data = obj["data"].toObject();
         QString msg = obj["msg"].toString();
         if (code != 200) {
            QString errMsg = msg;
            if (msg.isEmpty()) {
               errMsg = INIT_LIVE_ERR;
            }
            AlertTipsDlg tip(errMsg, false, NULL);
            tip.exec();
            SetEnabledLiveListWdg(true);
            m_pWaiting->Close();
            m_bEnableStartLive = true;
            CheckPrivateStartExitApp();
            return;
         }

         QJsonArray address = data["address"].toArray();
         doc.setArray(address);
         QString urls = QString::fromUtf8(doc.toJson(QJsonDocument::Indented).data());
         urls = urls.replace("\"name\"", "\"alias\"");
         urls = urls.replace("\"srv\"", "\"url\"");
         strcpy(mStartParam.streamURLS, urls.toUtf8().data());

         ClientApiInitResp respInitData;
         pCommonData->ParamToInitApiResp(respData);
         pCommonData->SetCurVersion(mCurrentVersion.toStdWString());
         pCommonData->GetInitApiResp(respInitData);
         ConfigSetting::writeValue(CPathManager::GetConfigPath(), GROUP_DEFAULT, KEY_CRASH_LIVE_NAME, respInitData.webinar_name);
         mIsInteractive = false;
         int nInteractType = respInitData.interact_type; //0：直播 1：腾讯互动 2：微吼互动
         int show_checkbox = respInitData.show_checkbox;////是否显示弹窗1显示0不显示
         if (show_checkbox == 1) { //如果弹出选择框说明是老活动，需要选择进入直播还是互动
            if (nInteractType == 2) {
               m_pWaiting->Close();
               Slot_DisableWebListDlg();
               VhallActivityModeChoiceDlg modeChoice(m_pLiveListWdg);
               if (modeChoice.exec() != QDialog::Accepted) {
                  SetEnabledLiveListWdg(true);
                  m_bEnableStartLive = true;
                  return;
               }
               m_pWaiting->Show();
               Slot_DisableWebListDlg();
               mIsInteractive = modeChoice.GetIsInterActive();  // false表示 直播 true表示互动
               modeChoice.close();
               modeChoice.setModal(false);
            }
         }
         else {
            mIsInteractive = nInteractType == 2 ? true : false; // 2 为互动 其他为直播
         }

         if (mIsInteractive) {
            mILiveType = eLiveType_VhallActive;
            pCommonData->SetSelectLiveType(eLiveType_VhallActive); //微吼互动
         }
         else {
            pCommonData->SetSelectLiveType(eLiveType_Live);//普通直播
         }

         //flash 活动
         if (respInitData.player == FLASH_LIVE_CREATE_TYPE) {
            HandleStartTurnToLiveMainUI(true);
         }
         //h5 活动
         else {
            SendGetRoomBaseInfo(respInitData.vss_token.toStdString(), respInitData.vss_room_id.toStdString());
         }
         UploadInitInfo();
      }
      else {
         AlertTipsDlg tip(INIT_LIVE_TIMEOUT, false, NULL);
         tip.exec();
         SetEnabledLiveListWdg(true);
         m_pWaiting->Close();
         m_bEnableStartLive = true;
      }
   }
}

void MainUILogic::SendGetRoomBaseInfo(const std::string vss_token, const std::string vss_room_id, int time) {
   QString vss_domain = ConfigSetting::ReadString(CPathManager::GetToolConfigPath(), GROUP_DEFAULT, KEY_VSS_URL_DOMAIN, vss_domainUrl);
   QObject* recvObj = this;
   VSSAPIManager vssApi;
   vssApi.InitAPIRequest(vss_domain.toStdString(), vss_token, vss_room_id);
   vssApi.VSSGetRoomBaseInfo([&, recvObj, time](int libCurlCode, const std::string& msg, const std::string userData) {
      if (recvObj) {
         CustomOnHttpResMsgEvent* event = new CustomOnHttpResMsgEvent(CustomEvent_VSSGetRoomBaseInfo);
         if (event) {
            event->mCode = libCurlCode;
            event->mMsg = QString::fromStdString(msg);
            event->mRequestTime = time;
            QCoreApplication::postEvent(recvObj, event, Qt::LowEventPriority);
         }
      }
   });
}

void MainUILogic::HandleWebSocketReconnecting() {
   if (m_pLiveMainDlg) {
      m_pLiveMainDlg->FadeOutTip(WEBSOCKET_RECONNECTING, TipsType_Error);
   }
   else if (mVhallIALive) {
      mVhallIALive->FadeOutTip(WEBSOCKET_RECONNECTING, TipsType_Error);
   }
}

void MainUILogic::HandleStartTurnToLiveMainUI(bool hostJoin) {
   VH::CComPtr<ICommonData> pCommonData;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
   ClientApiInitResp respInitData;
   pCommonData->GetInitApiResp(respInitData);

   wstring appPath = GetAppPath();
   QString domain = ConfigSetting::ReadString(CPathManager::GetToolConfigPath(), GROUP_DEFAULT, KEY_URL_DOMAIN, domainUrl);
   PublishInfo loStreamInfo;
   loStreamInfo.init(respInitData.nick_name.toStdString(), mStartParam.streamURLS, mStartParam.streamToken, mStartParam.streamName, mStartParam.channelID, mStartParam.bHideLogo);
   loStreamInfo.nProtocolType = respInitData.protocol_type;
   loStreamInfo.mUserId = respInitData.user_id;//流名与用户ID的组合   
   loStreamInfo.mWebinarType = respInitData.ismix == 0 ? "1" : "2";//flash传递参数，是否为多嘉宾活动 ismix 
   loStreamInfo.mAccesstoken = respInitData.accesstoken;//flash传递参数
   loStreamInfo.mRole = respInitData.role_name;
   loStreamInfo.mMsgToken = mStartParam.msgToken;
   loStreamInfo.mScheduler = respInitData.switchDomain;
   loStreamInfo.mRoomToken = respInitData.room_token;
   loStreamInfo.InitMultitcpPubLine(QString::fromStdWString(appPath) + "\\CustomStreamURL.txt");
   loStreamInfo.mWebinarName = respInitData.webinar_name;
   SetCutRecordDisplay(respInitData.display_cut_record);
   
   //初始化通用信息
   STRU_VHALLRIGHTEXTRAWIDGET_COMMONINFO commonInfo;
   if (mStartParam.bLoadExtraRightWidget) {
      strcpy(commonInfo.msgToken, mStartParam.msgToken);
      sprintf(commonInfo.msgUrl, "%s/api/client/v1/clientapi/", domain.toUtf8().data());
      domain.toWCharArray(commonInfo.m_domain);
      commonInfo.roomId = atol(mStartParam.streamName);
      commonInfo.forbidchat = respInitData.forbidchat != 0;
      strcpy(commonInfo.filterurl, respInitData.filterurl.toLocal8Bit().data());
      commonInfo.chat_port = respInitData.chat_port.toInt();
      commonInfo.msg_port = respInitData.msg_port.toInt();
      respInitData.chat_srv.toWCharArray(commonInfo.chat_srv);
      respInitData.chat_url.toWCharArray(commonInfo.chat_url);
      respInitData.msg_srv.toWCharArray(commonInfo.msg_srv);

      bool is_http_proxy = ConfigSetting::ReadInt(msConfPath, GROUP_DEFAULT, PROXY_OPEN, 0) != 0;
      if (is_http_proxy) {
         QString host = ConfigSetting::ReadString(msConfPath, GROUP_DEFAULT, PROXY_HOST, "");
         int port = ConfigSetting::ReadInt(msConfPath, GROUP_DEFAULT, PROXY_PORT, 80);
         QString usr = ConfigSetting::ReadString(msConfPath, GROUP_DEFAULT, PROXY_USERNAME, "");
         QString pwd = ConfigSetting::ReadString(msConfPath, GROUP_DEFAULT, PROXY_PASSWORD, "");
         strcpy(commonInfo.proxy_ip, host.toStdString().c_str());
         strcpy(commonInfo.proxy_port, QString::number(port).toStdString().c_str());
         strcpy(commonInfo.proxy_username, usr.toStdString().c_str());
         strcpy(commonInfo.proxy_password, pwd.toStdString().c_str());
      }
      strcpy(commonInfo.domain, domain.toUtf8().data());
      commonInfo.host = hostJoin;
      commonInfo.bStartInteraction = eLiveType_VhallActive != mILiveType ? true : false;
   }
   pCommonData->SetLiveType(mILiveType);
   pCommonData->SetStreamInfo(&loStreamInfo);
   pCommonData->SetLoginUserHostType(respInitData.host_type);
   pCommonData->SetUserID(respInitData.join_uid.toStdString().c_str());
   if (respInitData.selectLiveType == eLiveType_VhallActive) {	//微吼互动
      m_bIsHostStartLive = false;
      mILiveType = eLiveType_VhallActive;
      emit SigJoinActivetyEnd(true, "");
      pCommonData->SetSelectLiveType(eLiveType_VhallActive);
      if (showAliveDlg() == 0) {
         initCommonInfo(&commonInfo);
      }
      QString pluginUrl = respInitData.interact_plugins_url + "&version=" + mCurrentVersion + "&webinar_id=" + mStartParam.streamName;//访问连接  和 流Id
      loStreamInfo.mPluginUrl = pluginUrl;
      if (respInitData.player == H5_LIVE_CREATE_TYPE) {
         pluginUrl = respInitData.web_doc_url + QString("&hide=1");
      }
      m_pLivePlugInUnitDlg->InitPluginUrl(pluginUrl, this);
      m_pWaiting->Close();
      TRACE6("%s eLiveType_VhallActive\n", __FUNCTION__);
   }
   else {//直播
      pCommonData->SetSelectLiveType(eLiveType_Live);
      if (!IsUploadLiveTypeSuc(upload_normal_live, mStartParam.msgToken)) {
         TRACE6("%s upload_active_live err", __FUNCTION__);
         SetEnabledLiveListWdg(true);
         m_pWaiting->Close();
         m_bEnableStartLive = true;
         return;
      }
      
      if (m_pPluginDataProcessor) {
         m_pPluginDataProcessor->ClearAllData();
         m_pPluginDataProcessor->SetEnableRecvMsg(true);
      }
      createLiveMainDlg();
      bool bEnableVoiceTransition = respInitData.isVoice == 1 ? true : false;
      VH::CComPtr<IMainUILogic> pMainUILogic;
      DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IMainUI, IID_IMainUILogic, pMainUILogic, return, ASSERT(FALSE));
      if (pMainUILogic) {
         pMainUILogic->EnableVoiceTranslate(bEnableVoiceTransition);  //使能实时字幕功能
         int crashState = ConfigSetting::ReadInt(CPathManager::GetConfigPath(), GROUP_DEFAULT, KEY_CRASH_STATE, 0);
         int lastOpenState = ConfigSetting::ReadInt(CPathManager::GetConfigPath(), GROUP_DEFAULT, KEY_OPEN_VT, 0);
         if (crashState == 1 && lastOpenState == 1) {
            pMainUILogic->OpenVoiceTranslateFun(true); // 默认先不显示回显
            StartVoiceTranslate(true);
         }
         else {
            pMainUILogic->OpenVoiceTranslateFun(false); // 默认先不显示回显
         }
         TRACE6("%s enable vt %d\n", __FUNCTION__, bEnableVoiceTransition);
         if (m_pLiveMainDlg && bEnableVoiceTransition) {
            m_pLiveMainDlg->AddVoiceTranslate();
         }
         else if (m_pLiveMainDlg && !bEnableVoiceTransition) {
            m_pLiveMainDlg->RemoveVoiceTranslate();
         }
      }
      m_pWaiting->Close();
      m_pLiveMainDlg->SetStreamId(QString::number(commonInfo.roomId));
      initCommonInfo(&commonInfo);
      //显示主界面
      STRU_MAINUI_WIDGET_SHOW loShow;
      loShow.m_bIsShow = TRUE;
      if (mIsInteractive) {
         loShow.m_eType = widget_show_type_InterActive;
         TRACE6("%s widget_show_type_InterActive\n", __FUNCTION__);
      }
      else {
         loShow.m_eType = widget_show_type_Main;
         TRACE6("%s widget_show_type_Main\n", __FUNCTION__);
      }
      loShow.m_bIsWebinarPlug = FALSE;
      loShow.bLoadExtraRightWidget = mStartParam.bLoadExtraRightWidget;
      respInitData.webinar_name.toWCharArray(loShow.m_webinarName);
      QString::fromStdString(mStartParam.streamName).toWCharArray(loShow.m_streamName);
      TRACE6("%s MSG_MAINUI_WIDGET_SHOW\n", __FUNCTION__);
      SingletonMainUIIns::Instance().PostCRMessage(MSG_MAINUI_WIDGET_SHOW, &loShow, sizeof(loShow));
      m_pWaiting->Close();
   }

   VH::CComPtr<IVedioPlayLogic> pVedioPlayLogic = NULL;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IMainUI, IID_IVedioPlayLogic, pVedioPlayLogic, return);
   if (pVedioPlayLogic) {
      pVedioPlayLogic->StopAdmin(false);
   }
   TRACE6("%s end\n", __FUNCTION__);
}

void MainUILogic::HandleJoinActiveDirect(QEvent *event) {
   QHttpJoinActiveResponeEvent* joinActiveLiveEvent = dynamic_cast<QHttpJoinActiveResponeEvent*>(event);
   if (joinActiveLiveEvent) {
      VH::CComPtr<ICommonData> pCommonData = NULL;
      DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData)
      int code = joinActiveLiveEvent->mCode;
      QString respData = joinActiveLiveEvent->mData;
      TRACE6("MainUILogic::HandleJoinActiveDirect resp = %s\n", respData.toStdString().c_str());
      if (code == 0 && respData.length() > 0) {
         QByteArray dataByte(respData.toStdString().c_str(), respData.length());
         memset(&mStartParam, 0, sizeof(VHStartParam));
         mStartParam.bLoadExtraRightWidget = true;
         mStartParam.bConnectToVhallService = false;
         QJsonDocument doc = QJsonDocument::fromJson(dataByte);
         QJsonObject obj = doc.object();
         int code = obj["code"].toInt();
         if (code != 200) {
            QString msg = obj["msg"].toString();
            if (msg.isEmpty()) {
               emit SigJoinActivetyEnd(false, NETWORK_REQUEST_ERR);
            }
            else {
               emit SigJoinActivetyEnd(false, msg);
            }
            return;
         }

         QJsonObject data = obj["data"].toObject();
         QString stream_token = data["stream_token"].toString();
         QString msg_token = data["msg_token"].toString();
         QString hide_logo = data["hide_logo"].toString();
         pCommonData->SetStartLiveParam(stream_token, joinActiveLiveEvent->mRoomId, msg_token, hide_logo.toInt() == 1 ? true : false);
         int host_way = data["host_way"].toInt();	//sdk互动1是，0不是；当0的时候助手需要执行原来腾讯验证逻辑
         hostNickName = data["host_nick_name"].toString();
         hostJoinUid = data["host_join_uid"].toString();
         strcpy(mStartParam.streamToken, stream_token.toUtf8().data());
         strcpy(mStartParam.streamName, joinActiveLiveEvent->mRoomId.toUtf8().data());
         strcpy(mStartParam.msgToken, msg_token.toUtf8().data());
         mStartParam.bHideLogo = (hide_logo != "0");
         if (host_way != 1) {
            QString msg = NOT_SURPPORT_RTC;
            emit SigJoinActivetyEnd(false, msg);
            return;
         }

         QString initUrl = m_domain + "/api/client/v1/clientapi/init?token=" + msg_token + QString("&version=%1").arg(mCurrentVersion);
         TRACE6("%s initUrl %s\n", __FUNCTION__, initUrl.toStdString().c_str());
         QString configPath = CPathManager::GetConfigPath();
         QString proxyHost;
         QString proxyUsr;
         QString proxyPwd;
         int proxyPort;
         int is_http_proxy = ConfigSetting::ReadInt(configPath, GROUP_DEFAULT, PROXY_OPEN, 0);
         if (is_http_proxy) {
            TRACE6("%s is_http_proxy:%d\n", __FUNCTION__, is_http_proxy);
            proxyHost = ConfigSetting::ReadString(configPath, GROUP_DEFAULT, PROXY_HOST, "");
            proxyPort = ConfigSetting::ReadInt(configPath, GROUP_DEFAULT, PROXY_PORT, 80);
            proxyUsr = ConfigSetting::ReadString(configPath, GROUP_DEFAULT, PROXY_USERNAME, "");
            proxyPwd = ConfigSetting::ReadString(configPath, GROUP_DEFAULT, PROXY_PASSWORD, "");
         }
         QObject *recvObj = this;
         QString room_id = joinActiveLiveEvent->mRoomId;
         QString nickName = joinActiveLiveEvent->mJoinNickName;
         QString roomPwd = joinActiveLiveEvent->mRoomPwd;
         bool isPwdLogin = joinActiveLiveEvent->mIsPwdLogin;
         HTTP_GET_REQUEST request(initUrl.toStdString());
         GetHttpManagerInstance()->HttpGetRequest(request, [&, room_id, nickName, roomPwd, isPwdLogin, recvObj](const std::string& msg, int code, const std::string userData) {
            if (recvObj) {
               QCoreApplication::postEvent(recvObj, new QHttpJoinActiveResponeEvent(CustomEvent_JoinActivityInit, code, QString::fromStdString(msg), room_id, nickName, roomPwd, isPwdLogin));
            }
         });
      }
      else {
         emit SigJoinActivetyEnd(false, NETWORK_REQUEST_ERR);
         TRACE6("MainUILogic::SlotToJoinActivity Failed1\n");
         return;
      }
   }
}

void MainUILogic::HandleDefaultRecord(QEvent* event) {
   //mUploadLiveTypeLoop.quit();
   QHttpResponeEvent* create_record = dynamic_cast<QHttpResponeEvent*>(event);
   if (create_record) {
      if (create_record->mCode != 0) {
         if (m_pLiveMainDlg) {
            m_pLiveMainDlg->FadeOutTip(CREATE_RECORD_ERR, TipsType_Error);
         }
         CloseExitWaiting();
         HandleCloseLiveUi(create_record->mIsClose);
         return;
      }
      QString respData = create_record->mData;
      if (respData.length() > 0) {
         QByteArray ba(respData.toStdString().c_str(), respData.length());
         QJsonDocument doc = QJsonDocument::fromJson(ba);
         QJsonObject obj = doc.object();
         QString respCode = obj["code"].isString() ? obj["code"].toString() : QString::number(obj["code"].toInt());
         QString msg = obj["msg"].toString();
         if (respCode == "200") {
            CloseExitWaiting();
         }
         else {
            if (m_pLiveMainDlg) {
               m_pLiveMainDlg->FadeOutTip(msg, TipsType_Error);
            }
            CloseExitWaiting();
         }
         HandleCloseLiveUi(create_record->mIsClose);
      }
   }
}

void  MainUILogic::HandleCloseLiveUi(bool isClose) {
   VH::CComPtr<ICommonData> pCommonData;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
   ClientApiInitResp initResp;
   pCommonData->GetInitApiResp(initResp);
   if (initResp.selectLiveType == eLiveType_Live) {
      if (isClose) {
         CloseTurnToLiveListDlg();
      }
   }
   else {
      if (isClose && mVhallIALive) {
         mVhallIALive->HandleCloseAndExit();
      }
   }
}

void MainUILogic::HandleCreateRecord(QEvent *event) {

   QHttpResponeEvent* create_record = dynamic_cast<QHttpResponeEvent*>(event);
   if (create_record) {
      if (create_record->mCode != 0) {
         CloseExitWaiting();
         HandleCloseLiveUi(create_record->mIsClose);
         return;
      }
      QString respData = create_record->mData;
      if (respData.length() > 0) {
         QByteArray ba(respData.toStdString().c_str(), respData.length());
         QJsonDocument doc = QJsonDocument::fromJson(ba);
         QJsonObject obj = doc.object();
         QString respCode = obj["code"].isString() ? obj["code"].toString() : QString::number(obj["code"].toInt());
         QString msg = obj["msg"].toString();
         if (respCode == "200") {
            QJsonObject data = obj["data"].toObject();
            int id = data["id"].toInt();
            m_stopId = QString::number(id);
            AskGenerateRecord(m_stopId.toStdString().c_str(), create_record->mIsClose);
         }
         else {
            CloseExitWaiting();
            HandleCloseLiveUi(create_record->mIsClose);
         }
      }
   }
}

void MainUILogic::HandleJoinActivityByPwd(QEvent *event) {
   QHttpJoinActiveResponeEvent* joinActiveLiveEvent = dynamic_cast<QHttpJoinActiveResponeEvent*>(event);
   if (joinActiveLiveEvent) {
      VH::CComPtr<ICommonData> pCommonData = NULL;
      DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData)
      int code = joinActiveLiveEvent->mCode;
      QString respData = joinActiveLiveEvent->mData;
      TRACE6("%s init resp = %s\n", __FUNCTION__, respData.toStdString().c_str());
      if (code == 0 && respData.length() > 0) {
         QByteArray dataByte(respData.toStdString().c_str(), respData.length());
         QJsonDocument doc = QJsonDocument::fromJson(dataByte);
         QJsonObject obj = doc.object();
         code = obj["code"].toInt();
         if (code != 200) {
            QString msg = obj["msg"].toString();
            emit SigJoinActivetyEnd(false, msg);
            return;
         }
         else {
            QJsonObject data = obj["data"].toObject();
            LoginRespInfo loginInfo;
            loginInfo.plugins_url = data["plugins_url"].toString();
            loginInfo.interact_plugins_url = data["interact_plugins_url"].toString();
            loginInfo.list_url = data["list_url"].toString();
            loginInfo.token = data["token"].toString();
            loginInfo.nick_name = data["nick_name"].toString();
            loginInfo.avatar = "http:" + data["avatar"].toString();
            loginInfo.chat_url = data["chat_url"].toString();
            loginInfo.userid = data["client_unique"].toString();

            if (!loginInfo.userid.isEmpty()) {
               QString configPath = CPathManager::GetConfigPath();
               ConfigSetting::writeValue(configPath, GROUP_DEFAULT, USER_PWD_LOGIN_CLIENT_UNIQUE, loginInfo.userid);
            }

            m_domain = ConfigSetting::ReadString(CPathManager::GetToolConfigPath(), GROUP_DEFAULT, KEY_URL_DOMAIN, domainUrl);
            m_bShowTeaching = ConfigSetting::ReadInt(CPathManager::GetToolConfigPath(), GROUP_DEFAULT, KEY_SHOW_TEACHING, 1) == 1 ? true : false;
            ConfigSetting::writeValue(CPathManager::GetToolConfigPath(), GROUP_DEFAULT, KEY_SHOW_TEACHING, 0);

            mRoomID = joinActiveLiveEvent->mRoomId;
            mRoomPwd = joinActiveLiveEvent->mRoomPwd;
            //bool bActiveLive = true;
            //SingletonMainUIIns::Instance().PostCRMessage(MSG_COMMONDATA_DATA_INIT, &bActiveLive, sizeof(bool));
            ToJoinActivityDirect(mRoomID, joinActiveLiveEvent->mJoinNickName, joinActiveLiveEvent->mRoomPwd, true, loginInfo.token);
         }
      }
      else {
         emit SigJoinActivetyEnd(false, NETWORK_REQUEST_ERR);
         return;
      }
   }
}

void MainUILogic::HandleDispatchPublish(QEvent *event) {
   bool bScheduling = false;
   CustomOnHttpResMsgEvent* msgEvent = dynamic_cast<CustomOnHttpResMsgEvent*>(event);
   if (msgEvent) {
      if (msgEvent->mCode != 0) {
         SetEnabledLiveListWdg(true);
         TRACE6("%s dispatch_publish failed\n", __FUNCTION__);
         AlertTipsDlg tip(NETWORK_REQUEST_ERR, false, NULL);
         tip.CenterWindow(NULL);
         tip.exec();
         if (m_pDesktopSharingUI) {
            m_pDesktopSharingUI->SetStartLiveBtnEnable(true);
         }
         return;
      }
      else {
         QJsonDocument doc = QJsonDocument::fromJson(msgEvent->mMsg.toStdString().c_str());
         QJsonObject obj = doc.object();
         QString code = obj["code"].toString();
         QString msg = obj["msg"].toString();
         if (code == "200") {
            QJsonObject data = obj["data"].toObject();
            QJsonObject publish_args = data["publish_args"].toObject();
            publish_args_accesstoken = publish_args["accesstoken"].toString();
            publish_args_mixer = publish_args["mixer"].toString();
            publish_args_token = publish_args["token"].toString();
            publish_args_vhost = publish_args["vhost"].toString();
            QString da = data["publish_domainname"].toString();
            bScheduling = true;
         }
         else {
            SetEnabledLiveListWdg(true);
            TRACE6("%s dispatch_publish failed\n", __FUNCTION__);
            AlertTipsDlg tip(msg.isEmpty() ? NETWORK_REQUEST_ERR : msg, false, NULL);
            tip.CenterWindow(NULL);
            tip.exec();
            if (m_pDesktopSharingUI) {
               m_pDesktopSharingUI->SetStartLiveBtnEnable(true);
            }
            return;
         }
         HandlePushStreamState(bScheduling, msgEvent->status, msgEvent->mediaCoreStreamErr, msgEvent->bIsActiveLive);
      }
   }
}

void MainUILogic::HandleVSSGetPushInfo(QEvent *event) {
   QHttpResponeEvent* startWebniarEvent = dynamic_cast<QHttpResponeEvent*>(event);
   if (startWebniarEvent) {
      int code = startWebniarEvent->mCode;
      QString respData = startWebniarEvent->mData;
      QString msg;
      TRACE6("MainUILogic::HandleVSSGetRoomAttributes init resp = %s\n", respData.toStdString().c_str());
      if (code == 0 && respData.length() > 0) {
         QByteArray ba(respData.toStdString().c_str(), respData.length());
         QJsonDocument doc = QJsonDocument::fromJson(ba);
         QJsonObject obj = doc.object();
         QString respCode = obj["code"].isString() ? obj["code"].toString() : QString::number(obj["code"].toInt());
         msg = obj["msg"].toString();
         VH::CComPtr<ICommonData> pCommonData;
         DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
         ClientApiInitResp initResp;
         pCommonData->GetInitApiResp(initResp);
         if (respCode == "200") {
            VSSGetRoomInfo vssRoomInfo;
            pCommonData->GetVSSRoomBaseInfo(vssRoomInfo);
            pCommonData->ParamToVSSRoomPushUrl(respData);
            //h5 活动链接paas-socketio.成功或失败监听 CustomEvent_OnRoomSuccessedEvent、CustomEvent_OnRoomFailedEvent
            callbackReciver.SetEventReciver(this);
            QString paas_domain = ConfigSetting::ReadString(CPathManager::GetToolConfigPath(), GROUP_DEFAULT, KEY_PAAS_DOMAIN, VALUE_PAAS_DOMAIN);
            int is_http_proxy = ConfigSetting::ReadInt(CPathManager::GetConfigPath(), GROUP_DEFAULT, PROXY_OPEN, 0);
            if (is_http_proxy) {
               TRACE6("%s is_http_proxy:%d\n", __FUNCTION__, is_http_proxy);
               QString proxyHost = ConfigSetting::ReadString(CPathManager::GetConfigPath(), GROUP_DEFAULT, PROXY_HOST, "");
               int proxyPort = ConfigSetting::ReadInt(CPathManager::GetConfigPath(), GROUP_DEFAULT, PROXY_PORT, 80);
               QString proxyUsr = ConfigSetting::ReadString(CPathManager::GetConfigPath(), GROUP_DEFAULT, PROXY_USERNAME, "");
               QString proxyPwd = ConfigSetting::ReadString(CPathManager::GetConfigPath(), GROUP_DEFAULT, PROXY_PASSWORD, "");
               vlive::GetPaasSDKInstance()->SetHttpProxy(true, proxyHost.toStdString(), proxyPort, proxyUsr.toStdString(), proxyPwd.toStdString());
            }
            else {
               vlive::GetPaasSDKInstance()->SetHttpProxy(false);
            }

            std::string context = GetPaasSDKJoinContext();
            vlive::GetPaasSDKInstance()->InitSDK(&callbackReciver, &callbackReciver, paas_domain.toStdString());
            if (!vlive::GetPaasSDKInstance()->LoginRoom(vssRoomInfo.paas_access_token.toStdString(), vssRoomInfo.app_id.toStdString(), vssRoomInfo.third_party_user_id.toStdString(), vssRoomInfo.inav_id.toStdString(), vssRoomInfo.room_id.toStdString(), vssRoomInfo.channel_id.toStdString(), context)) {
               AlertTipsDlg tip(NETWORK_REQUEST_ERR, false, NULL);
               tip.exec();
               SetEnabledLiveListWdg(true);
               m_pWaiting->Close();
               m_bEnableStartLive = true;
               CheckPrivateStartExitApp();
               return;
            }
         }
         else {
            if (startWebniarEvent->mRetryTime < MAX_RETRY_TIME) {
               TRACE6("%s retry  SendGetPushInfo\n", __FUNCTION__);
               SendGetPushInfo(initResp.vss_token.toStdString(), initResp.vss_room_id.toStdString(), startWebniarEvent->mRetryTime + 1);
               return;
            }
            SetEnabledLiveListWdg(true);
            AlertTipsDlg tip(msg.isEmpty() ? NETWORK_REQUEST_ERR : msg, false, NULL);
            tip.CenterWindow(NULL);
            tip.exec();
            m_pWaiting->Close();
            CheckPrivateStartExitApp();
            TRACE6("%s GetStartStreamUrl failed\n", __FUNCTION__);
         }
      }
      else {
         SetEnabledLiveListWdg(true);
         AlertTipsDlg tip(NETWORK_REQUEST_ERR, false, NULL);
         tip.CenterWindow(NULL);
         tip.exec();
         TRACE6("%s GetStartStreamUrl failed\n", __FUNCTION__);
         CheckPrivateStartExitApp();
         m_pWaiting->Close();
      }
   }
}

std::string MainUILogic::GetPaasSDKJoinContext() {
   VH::CComPtr<ICommonData> pCommonData;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return std::string(), ASSERT(FALSE));
   ClientApiInitResp respInitData;
   pCommonData->GetInitApiResp(respInitData);
   VSSGetRoomInfo roomInfo;
   pCommonData->GetVSSRoomBaseInfo(roomInfo);
   QJsonObject obj;
   obj["nickname"] = respInitData.nick_name;
   obj["role_name"] = respInitData.role_name == USER_HOST ? "1" : "4";
   obj["is_banned"] = 0;
   obj["device_type"] = 2;
   obj["device_status:"] = 0;
   obj["avatar"] = respInitData.avatar;

   QJsonDocument document = QJsonDocument(obj);
   QByteArray array = document.toJson();

   QString jsonData = array;
   std::string sendMsg = std::string(jsonData.toUtf8().data(), jsonData.toUtf8().length());
   TRACE6("%s sendMsg:%s\n", __FUNCTION__, sendMsg.c_str());
   return sendMsg;
}

void MainUILogic::CheckPrivateStartExitApp() {
   if (m_pJoinActivity && !m_pJoinActivity->isHidden())
      emit SigJoinActivetyEnd(true, "");

   VH::CComPtr<ICommonData> pCommonData;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
   if (pCommonData->GetStartMode() != eStartMode_desktop) {
      PostEventToExitApp();
   }
}

void MainUILogic::HandleVSSGetRoomAttributes(QEvent *event) {
   QHttpResponeEvent* startWebniarEvent = dynamic_cast<QHttpResponeEvent*>(event);
   if (startWebniarEvent) {
      int code = startWebniarEvent->mCode;
      QString respData = startWebniarEvent->mData;
      QString msg;
      TRACE6("MainUILogic::HandleVSSGetRoomAttributes init resp = %s\n", respData.toStdString().c_str());
      if (code == 0 && respData.length() > 0) {
         QByteArray ba(respData.toStdString().c_str(), respData.length());
         QJsonDocument doc = QJsonDocument::fromJson(ba);
         QJsonObject obj = doc.object();
         QString respCode = obj["code"].isString() ? obj["code"].toString() : QString::number(obj["code"].toInt());
         msg = obj["msg"].toString();
         VH::CComPtr<ICommonData> pCommonData;
         DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
         ClientApiInitResp initResp;
         pCommonData->GetInitApiResp(initResp);
         if (respCode == "200") {
            VSSGetRoomInfo vssRoomInfo;
            pCommonData->GetVSSRoomBaseInfo(vssRoomInfo);
            pCommonData->ParamToVSSRoomAttributes(respData);
            //如果是视频直播，还需要先请求推流地址。
            if (initResp.selectLiveType == eLiveType_Live) {
               SendGetPushInfo(initResp.vss_token.toStdString(), initResp.vss_room_id.toStdString());
            }
            else {
               //h5 活动链接paas-socketio.成功或失败监听 CustomEvent_OnRoomSuccessedEvent、CustomEvent_OnRoomFailedEvent
               callbackReciver.SetEventReciver(this);
               QString paas_domain = ConfigSetting::ReadString(CPathManager::GetToolConfigPath(), GROUP_DEFAULT, KEY_PAAS_DOMAIN, VALUE_PAAS_DOMAIN);
               vlive::GetPaasSDKInstance()->InitSDK(&callbackReciver, &callbackReciver, paas_domain.toStdString());
               std::string context = GetPaasSDKJoinContext();
               if (!vlive::GetPaasSDKInstance()->LoginRoom(vssRoomInfo.paas_access_token.toStdString(), vssRoomInfo.app_id.toStdString(), vssRoomInfo.third_party_user_id.toStdString(), vssRoomInfo.inav_id.toStdString(), vssRoomInfo.room_id.toStdString(), vssRoomInfo.channel_id.toStdString(), context)) {
                  AlertTipsDlg tip(NETWORK_REQUEST_ERR, false, NULL);
                  tip.exec();
                  SetEnabledLiveListWdg(true);
                  m_pWaiting->Close();
                  m_bEnableStartLive = true;
                  vlive::DestoryPaasSDKInstance();
               }
            }
            return;
         }
         else {
            if (startWebniarEvent->mRetryTime < MAX_RETRY_TIME) {
               TRACE6("%s retry SendGetRoomAttributes\n", __FUNCTION__);
               SendGetRoomAttributes(initResp.vss_token.toStdString(), initResp.vss_room_id.toStdString(), startWebniarEvent->mRetryTime + 1);
               return;
            }
            SetEnabledLiveListWdg(true);
            AlertTipsDlg tip(msg.isEmpty() ? NETWORK_REQUEST_ERR : msg, false, NULL);
            tip.CenterWindow(NULL);
            tip.exec();
            TRACE6("%s GetStartStreamUrl failed\n", __FUNCTION__);
            if (m_pWaiting) {
               m_pWaiting->Close();
            }
         }
      }
      else {
         SetEnabledLiveListWdg(true);
         AlertTipsDlg tip(NETWORK_REQUEST_ERR, false, NULL);
         tip.CenterWindow(NULL);
         tip.exec();
         TRACE6("%s GetStartStreamUrl failed\n", __FUNCTION__);
         if (m_pWaiting)
            m_pWaiting->Close();

      }
   }
}

void MainUILogic::SendGetPushInfo(const std::string vss_token, const std::string vss_room_id, int time) {
   QString domain = ConfigSetting::ReadString(CPathManager::GetToolConfigPath(), GROUP_DEFAULT, KEY_VSS_URL_DOMAIN, vss_domainUrl);
   VSSAPIManager vssApi;
   QObject* recvObj = this;
   vssApi.InitAPIRequest(domain.toStdString(), vss_token, vss_room_id);
   vssApi.VSSGetPushInfo([&, recvObj, time](int libCurlCode, const std::string& msg, const std::string userData) {
      if (recvObj) {
         QHttpResponeEvent *eventMsg = new QHttpResponeEvent(CustomEvent_VSSGetPushInfo, libCurlCode, QString::fromStdString(msg));
         if (eventMsg) {
            eventMsg->mRetryTime = time;
            QCoreApplication::postEvent(recvObj, eventMsg);
         }
      }
   });
}

void MainUILogic::HandleStartWebniar(QEvent* event) {
   QHttpResponeEvent* startWebniarEvent = dynamic_cast<QHttpResponeEvent*>(event);
   if (startWebniarEvent) {
      int code = startWebniarEvent->mCode;
      QString respData = startWebniarEvent->mData;
      QString msg;
      TRACE6("MainUILogic::HandleStartWebniar init resp = %s\n", respData.toStdString().c_str());
      if (code == 0 && respData.length() > 0) {
         QByteArray ba(respData.toStdString().c_str(), respData.length());
         QJsonDocument doc = QJsonDocument::fromJson(ba);
         QJsonObject obj = doc.object();
         QString respCode = obj["code"].isString() ? obj["code"].toString() : QString::number(obj["code"].toInt());
         msg = obj["msg"].toString();
         if (respCode == "200") {
            //开始直播成功时记录状态位1。结束直播时恢复状态为0 ，表示直播过程中无异常
            ConfigSetting::writeValue(CPathManager::GetConfigPath(), GROUP_DEFAULT, KEY_CRASH_STATE, 1);
            if (m_pLiveMainDlg) {
               m_pLiveMainDlg->SetStreamStatus(true);
               m_pLiveMainDlg->SetStreamButtonStatus(true);
               m_pLiveMainDlg->closeSuspendTip();
               m_pLiveMainDlg->StartTimeClock(TRUE);
               m_pLiveMainDlg->CallDocWebViewtartLive();
            }
            VH::CComPtr<IOBSControlLogic> pOBSControlLogic;
            DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IOBSControl, IID_IOBSControlLogic, pOBSControlLogic, return, ASSERT(FALSE));
            if (m_pDesktopSharingUI) {
               m_pDesktopSharingUI->setEnablePlguin(true);
               m_pDesktopSharingUI->SetStreamButtonStatus(true);
               if (!m_pDesktopSharingUI->isHidden() || (pOBSControlLogic && pOBSControlLogic->IsMediaPlaying())) {
                  InteractAPIManager apiManager(this);
                  apiManager.HttpSendChangeWebWatchLayout(1);
               }
            }

            if (m_pWaiting) {
               m_pWaiting->Close();
            }
            if (m_pStartWaiting) {
               m_pStartWaiting->Close();
            }

            PublishInfo loStreamInfo;
            VH::CComPtr<ICommonData> pCommonData = NULL;
            DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData);
            pCommonData->GetStreamInfo(loStreamInfo);

            STRU_MAINUI_LOG log;
            wstring uploadData = L"key=stream_start&k=%d&tv=false&pl=" + QString::fromStdString(loStreamInfo.mCurRtmpUrl).toStdWString();
            swprintf_s(log.m_wzRequestUrl, DEF_MAX_HTTP_URL_LEN, uploadData.c_str(), eLogRePortK_StartStream);
            StreamStatus status;
            memset(&status, 0, sizeof(StreamStatus));
            QJsonObject body;
            body["tv"] = "false";   	
            QString json = CPathManager::GetStringFromJsonObject(body);
            strcpy_s(log.m_wzRequestJson, json.mid(0, DEF_MAX_HTTP_URL_LEN).toLatin1().data());
            SingletonMainUIIns::Instance().PostCRMessage(MSG_MAINUI_LOG, &log, sizeof(STRU_MAINUI_LOG));
            HandlePushStream(true, false, startWebniarEvent->mIsScheduling);
         }
         else {
            SetEnabledLiveListWdg(true);
            if (respCode == "10001" || respCode == "10002" || respCode == "10003" || respCode == "10004") {
               QString domain = ConfigSetting::ReadString(CPathManager::GetToolConfigPath(), GROUP_DEFAULT, KEY_URL_DOMAIN, domainUrl);
               QString url = domain + "/account/center";
               AskForOpenWebPage(TO_BUY, CANCEL, NOTICE, msg, url);
               m_pLiveMainDlg->SetStreamStatus(false);
            }
            else {
               FadoutTip((wchar_t*)msg.toStdWString().c_str());
            }
            if (m_pWaiting) {
               m_pWaiting->Close();
            }
            if (m_pStartWaiting) {
               m_pStartWaiting->Close();
            }
            if (m_pDesktopSharingUI) {
               m_pDesktopSharingUI->SetStartLiveBtnEnable(true);
            }
            return;
         }
      }
      else {
         SetEnabledLiveListWdg(true);
         AlertTipsDlg tip(NETWORK_REQUEST_ERR, false, NULL);
         tip.CenterWindow(NULL);
         tip.exec();
         TRACE6("%s GetStartStreamUrl failed\n", __FUNCTION__);
         if (m_pWaiting) {
            m_pWaiting->Close();
         }
         if (m_pStartWaiting) {
            m_pStartWaiting->Close();
         }
      }
   }
   if (m_pDesktopSharingUI) {
      m_pDesktopSharingUI->SetStartLiveBtnEnable(true);
   }
}

void MainUILogic::HandlePushStream(bool bIsStartPush, bool mediaCoreStreamErr, bool bScheduling) {
   VH::CComPtr<ICommonData> pCommonData = NULL;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData);
   ClientApiInitResp initResp;
   pCommonData->GetInitApiResp(initResp);

   QString savePath = ConfigSetting::ReadString(msConfPath, GROUP_DEFAULT, KEY_RECORD_PATH, "");
   if (savePath.trimmed().isEmpty()) {
      savePath = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation).first();
      ConfigSetting::writeValue(msConfPath, GROUP_DEFAULT, KEY_RECORD_PATH, savePath);
   }
   QString fileName = ConfigSetting::ReadString(msConfPath, GROUP_DEFAULT, KEY_RECORD_FILENAME, "");
   fileName = savePath + "/" + fileName;
   int iQuality = ConfigSetting::ReadInt(msConfPath, GROUP_DEFAULT, KEY_PUBLISH_QUALITY, 4);
   if (iQuality < 0) {
      iQuality = 0;
   }
   else if (iQuality > 4) {
      iQuality = 4;
   }

   if (!GetIsEnabled1080p() && iQuality == 3) {
      iQuality = 4;
   }

   int iRecordeMode = ConfigSetting::ReadInt(msConfPath, GROUP_DEFAULT, KEY_RECORD_MODE, 0);
   //通知OBSControl开始推流
   STRU_OBSCONTROL_PUBLISH loPublish;
   loPublish.m_bIsStartPublish = bIsStartPush;
   loPublish.m_bExit = false;
   loPublish.m_iQuality = gDownScale[iQuality];
   loPublish.m_bIsSaveFile = (bool)iRecordeMode;
   loPublish.m_bMediaCoreReConnect = mediaCoreStreamErr; //当mediaCoreStreamErr为true时，表示底层media_core推流失败了，需要重新进行推流处理。
   if (!bIsStartPush) {
      VH::CComPtr<ISettingLogic> pSettingLogic;
      DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IMainUI, IID_ISettingLogic, pSettingLogic, return, ASSERT(FALSE));
      loPublish.m_bServerPlayback = pSettingLogic->GetIsServerPlayback();
   }
   TRACE6("%s GetIsServerPlayback end\n", __FUNCTION__);
   this->m_iVcodecFPS = ConfigSetting::ReadInt(msConfPath, GROUP_DEFAULT, KEY_VCODEC_FPS, DEFAULT_VCODEC_FPS);
   wcscpy(loPublish.m_wzSavePath, fileName.toStdWString().c_str());
   loPublish.m_bDispatch = bScheduling;
   QString streamId, userId, role;
   PublishInfo loStreamInfo;
   if (pCommonData) {
      pCommonData->GetStreamInfo(loStreamInfo);
      streamId = QString::fromStdString(loStreamInfo.mStreamName);//流名
      role = loStreamInfo.mRole;
      userId = loStreamInfo.mUserId;
      if (pCommonData->GetStartMode() == eStartMode_flash) {
         loPublish.m_bDispatchParam.ismix = loStreamInfo.mWebinarType.toInt() == 2 ? 1 : 0;
      }
      else {
         loPublish.m_bDispatchParam.ismix = initResp.ismix; //是否嘉宾活动，0或1 
      }
   }

   if (bScheduling) {
      strcpy_s(loPublish.m_bDispatchParam.vhost, sizeof(loPublish.m_bDispatchParam.vhost), publish_args_vhost.toUtf8().data());
      strcpy_s(loPublish.m_bDispatchParam.token, sizeof(loPublish.m_bDispatchParam.token), publish_args_token.toUtf8().data());
      strcpy_s(loPublish.m_bDispatchParam.webinar_id, sizeof(loPublish.m_bDispatchParam.webinar_id), streamId.toUtf8().data());
      strcpy_s(loPublish.m_bDispatchParam.mixserver, sizeof(loPublish.m_bDispatchParam.mixserver), publish_args_mixer.toUtf8().data());
      strcpy_s(loPublish.m_bDispatchParam.accesstoken, sizeof(loPublish.m_bDispatchParam.accesstoken), publish_args_accesstoken.toUtf8().data());
      strcpy_s(loPublish.m_bDispatchParam.userId, sizeof(loPublish.m_bDispatchParam.userId), userId.toUtf8().data());
      strcpy_s(loPublish.m_bDispatchParam.role, sizeof(loPublish.m_bDispatchParam.role), role.toUtf8().data());
   }

   TRACE6("%s MSG_OBSCONTROL_PUBLISH webinar_type:%d\n", __FUNCTION__, loPublish.m_bDispatchParam.ismix);
   SingletonMainUIIns::Instance().PostCRMessage(MSG_OBSCONTROL_PUBLISH, &loPublish, sizeof(STRU_OBSCONTROL_PUBLISH));
}

void MainUILogic::HandleStopUIState(bool closeMainUi) {
   TRACE6("%s \n", __FUNCTION__);
   VH::CComPtr<ICommonData> pCommonData;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
   VH::CComPtr<IOBSControlLogic> pOBSControlLogic;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IOBSControl, IID_IOBSControlLogic, pOBSControlLogic, return, ASSERT(FALSE));
   TRACE6("%s StopPushStream\n", __FUNCTION__);
   pOBSControlLogic->StopPushStream();
   TRACE6("%s StopPushStream end\n", __FUNCTION__);
   CloseExitWaiting();
   long liveTime = 0;
   if (m_pLiveMainDlg) {
      liveTime = m_pLiveMainDlg->GetStartTime();
      m_pLiveMainDlg->SetStreamStatus(false);
   }

   if (m_pDesktopSharingUI) {
      m_pDesktopSharingUI->setEnablePlguin(false);
   }
   if (m_pPluginDataProcessor) {
      //当停止直播时，清空白板推流信息。并且重新加载下白板插件。
      m_pPluginDataProcessor->SetPushStreamState(false);
      TRACE6("%s SetPushStreamState end\n", __FUNCTION__);
      m_pPluginDataProcessor->ClearAllData();
   }

   TRACE6("%s ClearAllData end\n", __FUNCTION__);
   m_pLivePlugInUnitDlg->SlotRefresh();
   TRACE6("%s SlotRefresh end\n", __FUNCTION__);

   if (m_pLiveMainDlg) {
      //结束计时
      m_pLiveMainDlg->StartTimeClock(FALSE);
      TRACE6("%s StartTimeClock end\n", __FUNCTION__);
      /*m_pDesktopSharingUI->UpdateStreamSpeedUI("", 0);*/
      TRACE6("start push SetStreamButtonStatus false\n");
      m_pLiveMainDlg->SetStreamButtonStatus(false);
      TRACE6("%s SetStreamButtonStatus end\n", __FUNCTION__);
      m_pDesktopSharingUI->SetStreamButtonStatus(false);
      TRACE6("%s SetStreamButtonStatus end\n", __FUNCTION__);
      if (pCommonData && pCommonData->GetStartMode() == eStartMode_flash) {
         m_pLiveMainDlg->ShowSuspend(5000);
         //设置结束直播标识位
         pCommonData->SetPublishState(false);
      }
   }

   VH::CComPtr<ISettingLogic> pSettingLogic;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IMainUI, IID_ISettingLogic, pSettingLogic, return);
   if (pSettingLogic) {
      pSettingLogic->SetLiveState(false);
   }

   TRACE6("%s liveTime :%ld\n", __FUNCTION__, liveTime);
   if (pCommonData->GetStartMode() != eStartMode_flash) {
      if (liveTime < 30 && m_pLiveMainDlg) {
         m_pLiveMainDlg->FadeOutTip(LIVE_TIME_TOO_SHORT, TipsType_Error);
         CloseExitWaiting();
         if (closeMainUi) {
            CloseTurnToLiveListDlg();
         }
      }
      else {
         SetDefaultRecord(closeMainUi);
      }
   }
   TRACE6("%s StopWebnair end\n", __FUNCTION__);
   return;
}

void MainUILogic::HandleStopWebniar(QEvent* event) {
   if (m_pDesktopSharingUI) {
      m_pDesktopSharingUI->SetStartLiveBtnEnable(true);
   }
   QHttpResponeEvent* stopWebniarEvent = dynamic_cast<QHttpResponeEvent*>(event);
   if (stopWebniarEvent) {
      VH::CComPtr<ICommonData> pCommonData;
      DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
      int code = stopWebniarEvent->mCode;
      QString respData = stopWebniarEvent->mData;
      QString msg;
      TRACE6("MainUILogic::HandleStopWebniar init resp = %s\n", respData.toStdString().c_str());
      if (code == 0 && respData.length() > 0) {
         QByteArray ba(respData.toStdString().c_str(), respData.length());
         QJsonDocument doc = QJsonDocument::fromJson(ba);
         QJsonObject obj = doc.object();
         QString respCode = obj["code"].isString() ? obj["code"].toString() : QString::number(obj["code"].toInt());
         msg = obj["msg"].toString();
         if (respCode == "200") {
            commitRecord();
            CloseExitWaiting();
            if (IsStopRecord()) {
               ShowExitWaiting();
            }
            HandleStopUIState(stopWebniarEvent->mIsClose);
            //结束直播时恢复状态为0 ，表示直播过程中无异常
            ConfigSetting::writeValue(CPathManager::GetConfigPath(), GROUP_DEFAULT, KEY_CRASH_STATE, 0);
            STRU_MAINUI_LOG log;
            wstring uploadData = L"key=stream_start&k=%d&tv=rtmp";
            swprintf_s(log.m_wzRequestUrl, DEF_MAX_HTTP_URL_LEN, uploadData.c_str(), eLogRePortK_StopStream);
            SingletonMainUIIns::Instance().PostCRMessage(MSG_MAINUI_LOG, &log, sizeof(STRU_MAINUI_LOG));
            return;
         }
      }
   }

   TRACE6("%s StopWebnair eStopWebNair_Fail\n", __FUNCTION__);
   if (m_pLiveMainDlg) {
      AlertTipsDlg tip(STOP_LIVING_ERR, false, m_pLiveMainDlg);
      tip.CenterWindow(m_pLiveMainDlg);
      tip.exec();
   }
   CloseExitWaiting();
   return;
}

void  MainUILogic::HandleVSSGetRoomBaseInfo(QEvent* event) {
   CustomOnHttpResMsgEvent* MsgEvent = dynamic_cast<CustomOnHttpResMsgEvent*>(event);
   if (MsgEvent->mCode != 0) {
      AlertTipsDlg tip(NETWORK_REQUEST_ERR, false, NULL);
      tip.exec();
      SetEnabledLiveListWdg(true);
      m_pWaiting->Close();
      m_bEnableStartLive = true;
      CheckPrivateStartExitApp();
      return;
   }
   else {
      int nCode = 0;
      std::string respMsg;
      InteractAPIManager::ParamHttpResp(MsgEvent->mMsg.toStdString(), nCode, respMsg);
      QString errMsg = QString::fromStdString(respMsg);
      VH::CComPtr<ICommonData> pCommonData;
      DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
      pCommonData->ParamToGetVSSRoomBaseInfo(MsgEvent->mMsg);
      ClientApiInitResp initResp;
      pCommonData->GetInitApiResp(initResp);
      if (nCode != 200) {
         if (errMsg == "") {
            errMsg = NETWORK_REQUEST_ERR;
         }
         if (MsgEvent->mRequestTime < MAX_RETRY_TIME) {
            TRACE6("%s retry to SendGetRoomBaseInfo\n", __FUNCTION__);
            SendGetRoomBaseInfo(initResp.vss_token.toStdString(), initResp.vss_room_id.toStdString(), MsgEvent->mRequestTime + 1);
            return;
         }
         AlertTipsDlg tip(errMsg, false, NULL);
         tip.exec();
         SetEnabledLiveListWdg(true);
         m_pWaiting->Close();
         m_bEnableStartLive = true;
         CheckPrivateStartExitApp();
         return;
      }
      else {
         SendGetRoomAttributes(initResp.vss_token.toStdString(), initResp.vss_room_id.toStdString());
      }
   }
}

void MainUILogic::SendGetRoomAttributes(const std::string vss_token, const std::string vss_room_id, int time) {
   QString domain = ConfigSetting::ReadString(CPathManager::GetToolConfigPath(), GROUP_DEFAULT, KEY_VSS_URL_DOMAIN, vss_domainUrl);
   VSSAPIManager vssApi;
   QObject* recvObj = this;
   vssApi.InitAPIRequest(domain.toStdString(), vss_token, vss_room_id);
   vssApi.VSSGetRoomAttributes([&, recvObj, time](int libCurlCode, const std::string& msg, const std::string userData) {
      if (recvObj) {
         QHttpResponeEvent *eventMsg = new QHttpResponeEvent(CustomEvent_VSSGetRoomAttributes, libCurlCode, QString::fromStdString(msg));
         if (eventMsg) {
            eventMsg->mRetryTime = time;
            QCoreApplication::postEvent(recvObj, eventMsg);
         }
      }
   });
}

void MainUILogic::HandleUploadLiveType(QEvent* event) {
   UploadLiveType* LiveEvent = dynamic_cast<UploadLiveType*>(event);
   if (LiveEvent) {
      int code = LiveEvent->mCode;
      QString respData = LiveEvent->mData;
      TRACE6("%s init resp = %s\n", __FUNCTION__, respData.toStdString().c_str());
      if (code == 0 && respData.length() > 0) {
         QByteArray dataByte(respData.toStdString().c_str(), respData.length());
         QJsonDocument doc = QJsonDocument::fromJson(dataByte);
         QJsonObject obj = doc.object();
         code = obj["code"].toInt();
         if (code == 200) {
            mbIsUploadLiveSuc = true;
            mUploadLiveTypeLoop.quit();
            TRACE6("%s mUploadLiveTypeLoop quit\n", __FUNCTION__);
            return;
         }
      }
   }
   AlertTipsDlg tip(UPLOAD_LIVE_TYPE, false, NULL);
   tip.exec();
   SetEnabledLiveListWdg(true);
   m_pWaiting->Close();
   m_bEnableStartLive = true;
   mbIsUploadLiveSuc = false;
   mUploadLiveTypeLoop.quit();
   TRACE6("%s mUploadLiveTypeLoop quit\n", __FUNCTION__);
}

void MainUILogic::HandleDownLoadHead(QEvent *event) {
   VH::CComPtr<ICommonData> pCommonData;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
   ClientApiInitResp respInitData;
   pCommonData->GetInitApiResp(respInitData);
   if (eLiveType_VhallActive == mILiveType) {
      if (mVhallIALive) {
         mVhallIALive->HandleDownLoadHead(event);
      }
   }
   else {
      if (m_pLiveMainDlg) {
         m_pLiveMainDlg->HandleDownLoadHead(event);
      }
   }
}

void MainUILogic::HandleCreateObsLogic() {
   VH::CComPtr<IOBSControlLogic> pObsControlLogic = NULL;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IOBSControl, IID_IOBSControlLogic, pObsControlLogic, return, ASSERT(FALSE));
   m_pLiveMainDlg->SetRenderWidgetUpdate(false);
   TRACE6("MainUILogic::customEvent StartLive pObsControlLogic Create\n");
   VH::CComPtr<ISettingLogic> pSettingLogic;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IMainUI, IID_ISettingLogic, pSettingLogic, return);
   TRACE6("MainUILogic::customEvent InitConfig\n");
   pSettingLogic->InitConfig();
   TRACE6("MainUILogic::customEvent InitConfig end\n");
   m_pLiveMainDlg->SetRenderWidgetUpdate(true);
   ShowMainWidget();
   TRACE6("MainUILogic::customEvent ShowMainWidget\n");
   if (m_pLiveListWdg) {
      m_pLiveListWdg->hide();
   }
   m_pWaiting->Close();
}

void MainUILogic::HandleStartCreateOBS() {
   m_pWaiting->Show();
   VH::CComPtr<ISettingLogic> pSettingLogic;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IMainUI, IID_ISettingLogic, pSettingLogic, return);
   TRACE6("MainUILogic::customEvent pSettingLogic->Create()\n");
   pSettingLogic->Create();
   TRACE6("MainUILogic::customEvent pSettingLogic->Create() end\n");
   mRenderWidgetID = ((QWidget*)m_pLiveMainDlg->GetRenderWidget())->winId();
   if (mThreadToopTask) {
      mThreadToopTask->DoWork(CustomEvent_CreateObsLogic);
   }
   TRACE6("mThreadToopTask->DoWork\n");
}

void MainUILogic::HandleStartRepushStream() {
   TRACE6("%s MSG_OBSCONTROL_STREAM_RESET\n", __FUNCTION__);
   //重连机制需要延时再进行重连，否则media_core底层快速重连会导致服务器任务流还存在
   m_restartTimer.stop();
   m_restartTimer.start(1500);
   if (m_pPluginDataProcessor) {
      m_pPluginDataProcessor->SetPushStreamState(false);
   }
}

void MainUILogic::HandleJoinActivityInit(QEvent *event) {
   QHttpJoinActiveResponeEvent* joinActiveLiveEvent = dynamic_cast<QHttpJoinActiveResponeEvent*>(event);
   if (joinActiveLiveEvent) {
      VH::CComPtr<ICommonData> pCommonData = NULL;
      DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData)
      int code = joinActiveLiveEvent->mCode;
      QString respData = joinActiveLiveEvent->mData;
      TRACE6("%s init resp = %s\n", __FUNCTION__, respData.toStdString().c_str());
      if (code == 0 && respData.length() > 0) {
         QByteArray dataByte(respData.toStdString().c_str(), respData.length());
         QJsonDocument doc = QJsonDocument::fromJson(dataByte);
         QJsonObject obj = doc.object();
         code = obj["code"].toInt();
         if (code != 200) {
            QString msg = obj["msg"].toString();
            emit SigJoinActivetyEnd(false, msg);
            return;
         }

         ClientApiInitResp respInitData;
         pCommonData->ParamToInitApiResp(respData);
         pCommonData->SetCurVersion(mCurrentVersion.toStdWString());
         pCommonData->GetInitApiResp(respInitData);

         QJsonObject data = obj["data"].toObject();
         data = obj["data"].toObject();
         bool bEnableCommentCheck = false;   //聊天过滤后台不勾选，不显示聊天过滤按钮
         bool bShowChatForbidBtn = false;    //全体禁言后台不勾选， 不显示全体禁言按钮   
         bool bEnableMembersManager = false; //成员列表 ，显示列表，但不显示禁言、踢出按钮
         bool bEnableWebinarNotice = false;  //后台不勾选，不显示全员禁言按钮
         QJsonArray permission = data["permission_data"].toArray();
         if (!permission.isEmpty()) {
            if (permission.contains("comment_check")) {
               bEnableCommentCheck = true;
            }
            if (permission.contains("webinar_notice")) {
               bEnableWebinarNotice = true;
            }
            if (permission.contains("disable_msg")) {
               bShowChatForbidBtn = true;
            }
            if (permission.contains("members_manager")) {
               bEnableMembersManager = true;
            }
         }

         //设置嘉宾加入互动连麦，全局权限标志。
         if (pCommonData) {
            pCommonData->SetShowChatFilterBtn(bEnableCommentCheck);
            pCommonData->SetMembersManager(bEnableMembersManager);
            pCommonData->SetWebinarNotice(bEnableWebinarNotice);
            pCommonData->SetShowChatForbidBtn(bShowChatForbidBtn);
            pCommonData->SetLoginUserHostType(respInitData.host_type);
            pCommonData->SetLiveType(eLiveType_LoginVhallActive);
            pCommonData->SetUserID(respInitData.join_uid.toStdString().c_str());
            pCommonData->SetSelectLiveType(eLiveType_VhallActive); //微吼互动
            TRACE6("%s bEnableCommentCheck:%d bShowChatForbidBtn:%d bEnableWebinarNotice:%d\n", __FUNCTION__, bEnableCommentCheck, bShowChatForbidBtn, bEnableWebinarNotice);
         }

         mIsInteractive = true;
         mILiveType = eLiveType_VhallActive;       //2时表示加入微吼互动直播 , 否则加入腾讯互动
         //flash 活动
         if (respInitData.player == FLASH_LIVE_CREATE_TYPE) {
            HandleStartTurnToLiveMainUI(false);
         }
         //h5 活动
         else {
            SendGetRoomBaseInfo(respInitData.vss_token.toStdString(), respInitData.vss_room_id.toStdString());
         }
      }
      else {
         emit SigJoinActivetyEnd(false, NETWORK_REQUEST_ERR);
         TRACE6("%s Failed1\n", __FUNCTION__);
         return;
      }
   }
}

void MainUILogic::HandleShowMainWdg() {
   if (m_pDesktopSharingUI && m_pDesktopSharingUI->isHidden() && m_pLiveMainDlg) {
      m_pLiveMainDlg->show();
      TRACE6("%s  m_pLiveMainDlg->show()\n", __FUNCTION__);
      m_pLiveMainDlg->activateWindow();
      UploadInitInfo();
   }
}

bool STDMETHODCALLTYPE MainUILogic::IsLoadUrlFinished() {
   bool bLoadFinished = true;
   if (m_pLivePlugInUnitDlg) {
      bLoadFinished = m_pLivePlugInUnitDlg->IsLoadUrlFinished();
      TRACE6("%s m_pLivePlugInUnitDlg->IsLoadUrlFinished() %d\n", __FUNCTION__, bLoadFinished);
   }
   if (m_bLoadExtraRightWidget && m_pLiveMainDlg != NULL && bLoadFinished) {
      TRACE6("%s m_pLiveMainDlg->IsLoadWebEngienViewFinished() %d\n", __FUNCTION__, bLoadFinished);
   }
   return bLoadFinished;
}

char* STDMETHODCALLTYPE MainUILogic::GetMsgToken() {
   char *msgToken = NULL;
   if (!m_msgToken.isEmpty()) {
      int len = m_msgToken.toLocal8Bit().size();
      msgToken = new char[len + 1];
      memset(msgToken, 0, len + 1);
      memcpy(msgToken, m_msgToken.toLocal8Bit().data(), len);
   }
   return msgToken;
}

void  MainUILogic::SetMessageSocketIOCallBack(InteractionMsgCallBack callback)
{
   if (mAliveInteraction) {
      mAliveInteraction->SetMessageSocketIOCallBack(callback);
   }
}

void MainUILogic::StopInteraction()
{
   if (NULL != mAliveInteraction) {
      mAliveInteraction->Destory();
      delete mAliveInteraction;
      mAliveInteraction = NULL;
   }
}

void MainUILogic::CreateVhallAliveInteracton(bool isInteractiveLive) {
   VH::CComPtr<ICommonData> pCommonData;
   ClientApiInitResp initResp;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
   pCommonData->GetInitApiResp(initResp);
   // flash活动或者普通活动。
   if (NULL == mAliveInteraction && initResp.player == FLASH_LIVE_CREATE_TYPE) {
      QString qsConfPath = CPathManager::GetConfigPath();
      QString proxyIp;
      QString proxyPwd;
      QString proxyUser;
      int proxyPort;
      int is_http_proxy = ConfigSetting::ReadInt(qsConfPath, GROUP_DEFAULT, PROXY_OPEN, 0);
      if (is_http_proxy) {
         proxyIp = ConfigSetting::ReadString(qsConfPath, GROUP_DEFAULT, PROXY_HOST, "");
         proxyPort = ConfigSetting::ReadInt(qsConfPath, GROUP_DEFAULT, PROXY_PORT, 80);
         proxyUser = ConfigSetting::ReadString(qsConfPath, GROUP_DEFAULT, PROXY_USERNAME, "");
         proxyPwd = ConfigSetting::ReadString(qsConfPath, GROUP_DEFAULT, PROXY_PASSWORD, "");
         if (!proxyIp.isEmpty()) {
            is_http_proxy = 1;
         }
      }
      //互动不使用代理。
      if (isInteractiveLive) {
         is_http_proxy = 0;
      }
      mAliveInteraction = new VHAliveInteraction(is_http_proxy == 1 ? true : false);
      if (NULL != mAliveInteraction)
      {
         if (is_http_proxy && !proxyIp.isEmpty()) {
            mAliveInteraction->SetHttpProxy(true, proxyIp.toStdString().c_str(), QString::number(proxyPort).toStdString().c_str(), proxyUser.toStdString().c_str(), proxyPwd.toStdString().c_str());
         }
         mAliveInteraction->SetEventReciver(this);
         connect(mAliveInteraction, SIGNAL(SigOnConnected()), this, SLOT(SlotOnConnected()));
         connect(mAliveInteraction, SIGNAL(SigUnInitFinished()), this, SLOT(SlotUnInitFinished()));
         if (!mAliveInteraction->Create()) {
            delete mAliveInteraction;
            mAliveInteraction = NULL;
         }
      }
   }
}

void MainUILogic::DealInitCommonInfo(void* apData, DWORD adwLen){
   CreateVhallAliveInteracton();
   if (mAliveInteraction && !mAliveInteraction->isInit()){
      DEF_CR_MESSAGE_DATA_DECLARE_CA(STRU_VHALLRIGHTEXTRAWIDGET_COMMONINFO, loMessage, return);
      initCommonInfo(&loMessage);
   }
}

void MainUILogic::InitFlashDocWebServer() {
   //为文档插件创建http server
   if (m_pPluginHttpServer == NULL) {
      m_pPluginHttpServer = GetHttpServerInstance();
      if (m_pPluginHttpServer) {
         m_pPluginHttpServer->RegisterHttpDateCallback(OnHttpRequestData);
         int nRet = m_pPluginHttpServer->StartHttpServer("127.0.0.1", 968);
         if (nRet != 0) {
            TRACE6("%s StartHttpServer err. %d\n", __FUNCTION__, nRet);
         }
         else {
            TRACE6("%s StartHttpServer ok.  %d\n", __FUNCTION__, nRet);
         }
      }
   }
}

int MainUILogic::showAliveDlg()
{
   VH::CComPtr<ICommonData> pCommonData = NULL;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData)
   ClientApiInitResp respInitData;
   pCommonData->GetInitApiResp(respInitData);
   StartLiveUrl startLiveData;
   pCommonData->GetStartLiveUrl(startLiveData);

   if (respInitData.role_name.compare("host") == 0 && !IsUploadLiveTypeSuc(upload_active_live, startLiveData.msg_token)) {
      TRACE6("%s upload_active_live err", __FUNCTION__);
      return -1;
   }
   createAliveDlg();
   if (mVhallIALive) {
      if (respInitData.player == FLASH_LIVE_CREATE_TYPE) {
         InitFlashDocWebServer();
      }
      CreateVhallAliveInteracton(true);
      TRACE6("%s CreateVhallAliveInteracton\n", __FUNCTION__);
      if (mLoginDlg) {
         mLoginDlg->setEnabled(true);
         mLoginDlg->hide();
      }
      TRACE6("%s Init\n", __FUNCTION__);
      callbackReciver.SetEventReciver(this);
      mVhallIALive->Init(this, &callbackReciver);
      mVhallIALive->show();
      ConfigSetting::MoveWindow(mVhallIALive);
      UploadInitInfo();
      TRACE6("%s show\n", __FUNCTION__);
      if (m_pLiveListWdg) {
         m_pLiveListWdg->hide();
      }
      TRACE6("%s showAliveDlg end\n", __FUNCTION__);
      m_showMainWidget = false;
   }
   return 0;
}

void MainUILogic::createAliveDlg()
{
   if (NULL == mVhallIALive) {
      mVhallIALive = new VhallIALive(m_bShowTeaching);
      m_bShowTeaching = false;
      connect(mVhallIALive, &VhallIALive::signal_MemberListRefresh, this, &MainUILogic::SlotOnConnected);
      //connect(mVhallIALive, &VhallIALive::sig_OperationUser, this, &MainUILogic::SlotControlUser);
      connect(mVhallIALive, &VhallIALive::sig_ReqApointPageUserList, this, &MainUILogic::SlotReqApointPageUserList);
      connect(mVhallIALive, &VhallIALive::sigInteractiveClicked, this, &MainUILogic::slot_OnClickedPluginUrlFromDesktop);
      connect(mVhallIALive, &VhallIALive::sigToStopDesktopSharing, this, &MainUILogic::StopDesktopSharing);
      if (m_pPluginDataProcessor) {
         m_pPluginDataProcessor->ClearAllData();
         m_pPluginDataProcessor->SetEnableRecvMsg(false);
      }
      PublishInfo loStreamInfo;
      VH::CComPtr<ICommonData> pCommonData;
      DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
      pCommonData->GetStreamInfo(loStreamInfo);
      QString::fromStdString(loStreamInfo.mStreamName).toWCharArray(m_wzStreamId);
   }
}

void MainUILogic::initCommonInfo(const STRU_VHALLRIGHTEXTRAWIDGET_COMMONINFO* commonInfo)
{
   m_msgToken = commonInfo->msgToken;
   m_msgUrl = commonInfo->msgUrl;
   CreateVhallAliveInteracton();
   TRACE6("%s \n", __FUNCTION__);
   if (NULL != mAliveInteraction) {
      qDebug() << "##MainUILogic::Init ConnectServer##";
      memset(&mKeyData, 0, sizeof(mKeyData));
      memset(mKeyData.m_szMsgToken, 0, sizeof(mKeyData.m_szMsgToken));

      m_msgToken.toWCharArray(mKeyData.m_szMsgToken);
      mKeyData.m_lRoomID = commonInfo->roomId;
      wcscpy(mKeyData.m_msgAPP, L"vhall");

      strcpy(mKeyData.m_domain, commonInfo->domain);
      strcpy(mKeyData.m_proxy_ip, commonInfo->proxy_ip);
      strcpy(mKeyData.m_proxy_port, commonInfo->proxy_port);
      strcpy(mKeyData.m_proxy_username, commonInfo->proxy_username);
      strcpy(mKeyData.m_proxy_password, commonInfo->proxy_password);

      // todo, 初始化sdk连接参数
      wsprintf(mKeyData.m_MSG_SERVER_URL, L"%s/api/client/v1/clientapi/", commonInfo->m_domain);
      mKeyData.m_chatPORT = commonInfo->chat_port;
      wcscpy(mKeyData.m_msgHOST, commonInfo->msg_srv);
      wsprintf(mKeyData.m_msgURL, L"ws://%s/socket.io/", commonInfo->msg_srv);

      //消息接受
      wcscpy(mKeyData.m_chatURL, commonInfo->chat_srv);
      //消息发送
      wsprintf(mKeyData.m_CHAT_SERVER_URL, L"http:%s", commonInfo->chat_url);
      SetMessageSocketIOCallBack(std::bind(&MainUILogic::RecvSocketIOMsg, this, placeholders::_1));
      mAliveInteraction->Init(mKeyData);
      TRACE6("%s Init end\n", __FUNCTION__);
      if (NULL != m_pLiveMainDlg) {
         m_pLiveMainDlg->SetStreamId(QString::number(commonInfo->roomId));
      }
   }
   TRACE6("%s end\n", __FUNCTION__);
}

void MainUILogic::createLiveMainDlg()
{
   if (NULL == m_pLiveMainDlg) {
      //主界面创建
      m_pLiveMainDlg = new VhallLiveMainDlg();
      if (NULL == m_pLiveMainDlg || !m_pLiveMainDlg->Create(this)) {
         TRACE6("MainUILogic::Create() new CMainWidget! or m_pMainWidget->Create Failed!\n");
      }
      else {
         //connect(m_pLiveMainDlg, &VhallLiveMainDlg::sig_OperationUser, this, &MainUILogic::SlotControlUser);
         connect(m_pLiveMainDlg, &VhallLiveMainDlg::signal_MemberListRefresh, this, &MainUILogic::SlotOnConnected);
         connect(m_pLiveMainDlg, SIGNAL(sig_ExitApp()), this, SLOT(Slot_ExitApp()));
         connect(m_pLiveMainDlg, &VhallLiveMainDlg::sig_ReqApointPageUserList, this, &MainUILogic::SlotReqApointPageUserList);
      }

      connect(this, &MainUILogic::sigMainWidgetShowNormal, m_pLiveMainDlg, &VhallLiveMainDlg::ShowNormal);
      connect(m_pLiveMainDlg, SIGNAL(sigMove()), this, SLOT(MainWidgetMove()));
      connect(m_pLiveMainDlg, SIGNAL(exitClicked(bool)), this, SLOT(AppExit(bool)));
      m_pLiveMainDlg->SetRenderWidgetUpdate(true);
      VH::CComPtr<ICommonData> pCommonData;
      DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
      StartLiveUrl liveUrl;
      pCommonData->GetStartLiveUrl(liveUrl);
      m_pLiveMainDlg->HideLogo(liveUrl.hide_logo);

      //创建桌面分享UI界面
      m_pDesktopSharingUI = new VHDesktopSharingUI(m_pLiveMainDlg);
      if (NULL != m_pDesktopSharingUI) {
         m_pDesktopSharingUI->SetPlayerShow(false);
         m_pDesktopSharingUI->SetLiveState(true);
         connect(m_pDesktopSharingUI, &VHDesktopSharingUI::sigToStopDesktopSharing, this, &MainUILogic::StopDesktopSharing);
         connect(m_pDesktopSharingUI, &VHDesktopSharingUI::sigCameraClicked, this, &MainUILogic::slotCameraClicked);
         connect(m_pDesktopSharingUI, &VHDesktopSharingUI::sigSettingClicked, this, &MainUILogic::slotSettingClicked);
         connect(m_pDesktopSharingUI, &VHDesktopSharingUI::sigLiveClicked, this, &MainUILogic::slotLiveClicked);
         connect(m_pDesktopSharingUI, &VHDesktopSharingUI::sigMicClicked, this, &MainUILogic::slotMicClicked);
         connect(m_pDesktopSharingUI, &VHDesktopSharingUI::sigChatClicked, m_pLiveMainDlg, &VhallLiveMainDlg::slotChatClicked);
         connect(m_pDesktopSharingUI, &VHDesktopSharingUI::sigMemberClicked, m_pLiveMainDlg, &VhallLiveMainDlg::slotMemberClicked);
         connect(m_pDesktopSharingUI, &VHDesktopSharingUI::sigInteractiveClicked, this, &MainUILogic::slot_OnClickedPluginUrlFromDesktop);
         connect(m_pDesktopSharingUI, &VHDesktopSharingUI::sigShareSelect, this, &MainUILogic::slot_OnShareSelect);
         m_pDesktopSharingUI->setEnablePlguin(false);
      }
      m_pExitWaiting = new VhallWaiting(m_pLiveMainDlg);
      if (m_pExitWaiting) {
         m_pExitWaiting->SetPixmap(":/interactivity/cb2");
         m_pExitWaiting->SetCerclePos(55, 50);
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
         m_pExitWaiting->SetParentEffectiveWidth(LIVEMAINDLGWIDTH);
         connect(m_pExitWaiting, SIGNAL(SigShowWaiting()), this, SLOT(Slot_DisableWebListDlg()));
         connect(m_pExitWaiting, SIGNAL(SigCloseWating()), this, SLOT(Slot_EnableWebListDlg()));
      }

      m_pStartWaiting = new VhallWaiting(m_pLiveMainDlg);
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
         m_pStartWaiting->SetParentEffectiveWidth(LIVEMAINDLGWIDTH);
         connect(m_pStartWaiting, SIGNAL(SigCloseWating()), this, SLOT(Slot_EnableWebListDlg()));
         connect(m_pStartWaiting, SIGNAL(SigShowWaiting()), this, SLOT(Slot_DisableWebListDlg()));
      }

      PublishInfo loStreamInfo;
      pCommonData->GetStreamInfo(loStreamInfo);
      QString::fromStdString(loStreamInfo.mStreamName).toWCharArray(m_wzStreamId);
   }
}

void MainUILogic::StopWebnair(bool isClose) {
   ShowExitWaiting();
   InteractAPIManager api(this);
   api.HttpSendStopLive(eLiveType_Live, isClose);
   return;
}

bool MainUILogic::Get(QString method, QString param, bool bExit) {
   QString url = m_msgUrl + method + "?token=" + m_msgToken + param;
   TRACE6("%s url:%s\n", __FUNCTION__, url.toStdString().c_str());
   m_lastGetError = "";
   m_lastGetCode = 0;
   qDebug() << url;
   if (bExit || ("createrecord" == method) || ("stopwebinar" == method)) {
      VHHttp http;
      QJsonObject res = http.Get(url, 15000);
      if (!res.isEmpty()) {
         if ("stopwebinar" == method) {
            QString code = res["code"].toString();
            TRACE6("%s method:%s code:%d\n", __FUNCTION__, method.toStdString().c_str(), code);
            if (code == "200") {
               if ("createrecord" == method) {
                  QJsonObject data = res["data"].toObject();
                  int id = data["id"].toInt();
                  m_stopId = QString::number(id);
               }
               m_lastGetCode = 200;
            }
            else {
               m_lastGetCode = code.toInt();
               return false;
            }
         }
         else if ("startwebinar" == method) {
            QString code = res["code"].toString();
            TRACE6("%s startwebinar code:%d\n", __FUNCTION__, code);
            if (code == "200") {
               m_lastGetCode = 200;
               return true;
            }
            m_lastGetCode = code.toInt();
            m_lastGetError = res["msg"].toString();

            qDebug() << "VhallRightExtraWidgetLogic::Get startwebinar Failed!" << res << m_lastGetError;
            return false;
         }
         else {
            int code = res["code"].toInt();
            TRACE6("%s code:%d\n", __FUNCTION__, code);
            m_lastGetCode = code;
            if (code == 200) {
               if ("createrecord" == method) {
                  QJsonObject data = res["data"].toObject();
                  int id = data["id"].toInt();
                  m_stopId = QString::number(id);
               }
            }
            else {
               return false;
            }
         }
      }
      else {
         TRACE6("%s return false\n", __FUNCTION__);
         return false;
      }
   }
   else {
/*      STRU_HTTPCENTER_HTTP_RQ loRQ;
      loRQ.m_dwPluginId = ENUM_PLUGIN_OBSCONTROL;
      wcsncpy(loRQ.m_wzRequestUrl, url.toStdWString().c_str(), DEF_MAX_HTTP_URL_LEN);
      SingletonMainUIIns::Instance().PostCRMessage(MSG_HTTPCENTER_HTTP_RQ, &loRQ, sizeof(loRQ))*/;
      QObject *recvObj = this;
      HTTP_GET_REQUEST request(url.toStdString());
      GetHttpManagerInstance()->HttpGetRequest(request, [&, recvObj](const std::string& msg, int code, const std::string userData) {
         TRACE6("%s msg %s\n", __FUNCTION__, msg.c_str());
         //QHttpResponeEvent* customEvent = new QHttpResponeEvent(CustomEvent_Default_Record, code, QString::fromStdString(msg));
         //customEvent->mIsClose = isClose;
         //QCoreApplication::postEvent(recvObj, customEvent);
      });

      TRACE6("%s PostCRMessage\n", __FUNCTION__);
   }
   return true;
}

bool MainUILogic::commitRecord()
{
   bool bRef = false;
   if (IsStopRecord()) {
      bRef = true;
   }
   else {
      //结束直播时不用再次通知结束录制，只需要更改本地状态即可。
      m_lastGetError = "";
      m_lastGetCode = 200;
      bRef = true;
      VH::CComPtr<IOBSControlLogic> pOBSControlLogic;
      DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IOBSControl, IID_IOBSControlLogic, pOBSControlLogic, return bRef, ASSERT(FALSE));
      long liveTime = 0;
      if(m_pLiveMainDlg){
         m_pLiveMainDlg->GetStartTime();
      }
      pOBSControlLogic->StopRecord(false, liveTime);
      SetRecordState(eRecordState_Stop);
   }
   return bRef;
}

void MainUILogic::RunTask(int type,void *appdata) {
   if (mThreadToopTask) {
      mThreadToopTask->DoWork(type, appdata);
   }
}

int MainUILogic::CreateRecord(bool isClose) {
   SetDefaultRecord(isClose);
   return 0;
}

int MainUILogic::AskGenerateRecord(const char* stopId /*= NULL*/ , bool isClose) {
   TRACE6("%s isClose %d\n", __FUNCTION__, isClose);
   VH::CComPtr<ICommonData> pCommonData;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return QDialog::Rejected, ASSERT(FALSE));
   ClientApiInitResp initResp;
   pCommonData->GetInitApiResp(initResp);
   TRACE6("%s initResp.defualt_record %d\n", __FUNCTION__, initResp.defualt_record);
   if (initResp.defualt_record) {
      QString strStopId = m_stopId;
      if (stopId != NULL) {
         strStopId = QString(stopId);
      }
      VH::CComPtr<IVedioPlayLogic> pVedioPlayLogic = NULL;
      DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IMainUI, IID_IVedioPlayLogic, pVedioPlayLogic, return 0);
      if (pVedioPlayLogic) {
         pVedioPlayLogic->CloseFileChoiceUI();
      }
      AlertTipsDlg tip(SET_DEFUALT_RECORE, true, initResp.selectLiveType == eLiveType_Live ? (QWidget*)m_pLiveMainDlg : (QWidget*)mVhallIALive);
      tip.CenterWindow(initResp.selectLiveType == eLiveType_Live ? (QWidget*)m_pLiveMainDlg : (QWidget*)mVhallIALive);
      if (tip.exec() == QDialog::Accepted) {
         QString domain = ConfigSetting::ReadString(CPathManager::GetToolConfigPath(), GROUP_DEFAULT, KEY_URL_DOMAIN, domainUrl);
         QString url = m_msgUrl + "default-record?token=" + m_msgToken + "&id=" + strStopId;
         TRACE6("%s url:%s\n", __FUNCTION__, url.toStdString().c_str());
         //代理配置
         QString configPath = CPathManager::GetConfigPath();
         QString proxyHost;
         QString proxyUsr;
         QString proxyPwd;
         int proxyPort;
         int is_http_proxy = ConfigSetting::ReadInt(configPath, GROUP_DEFAULT, PROXY_OPEN, 0);
         if (is_http_proxy) {
            TRACE6("%s is_http_proxy:%d\n", __FUNCTION__, is_http_proxy);
            proxyHost = ConfigSetting::ReadString(configPath, GROUP_DEFAULT, PROXY_HOST, "");
            proxyPort = ConfigSetting::ReadInt(configPath, GROUP_DEFAULT, PROXY_PORT, 80);
            proxyUsr = ConfigSetting::ReadString(configPath, GROUP_DEFAULT, PROXY_USERNAME, "");
            proxyPwd = ConfigSetting::ReadString(configPath, GROUP_DEFAULT, PROXY_PASSWORD, "");
         }

         QObject *recvObj = this;
         HTTP_GET_REQUEST request(url.toStdString());
         GetHttpManagerInstance()->HttpGetRequest(request, [&, recvObj, isClose](const std::string& msg, int code, const std::string userData) {
            TRACE6("%s msg %s\n", __FUNCTION__, msg.c_str());
            QHttpResponeEvent* customEvent = new QHttpResponeEvent(CustomEvent_Default_Record, code, QString::fromStdString(msg));
            customEvent->mIsClose = isClose;
            QCoreApplication::postEvent(recvObj, customEvent);
         });
         TRACE6("%s end\n", __FUNCTION__);
         return QDialog::Accepted;
      }
      else {
         HandleCloseLiveUi(isClose);
      }
   }
   //mUploadLiveTypeLoop.quit();
   TRACE6("%s QDialog::Rejected\n", __FUNCTION__);
   return QDialog::Rejected;
}
