#include "stdafx.h"
#include <QTimer>
#include <QJsonObject>
#include <QDir>
#include <QtNetwork\QNetworkProxy>
#include <sys/timeb.h>
#include "RuntimeInstance.h"
#include "RuntimePlugManager.h"
#include "RuntimeUIThreadManager.h" 
#include "UACUtility.h"
#include <QUuid>
#include "Defs.h"
#include "vhhttp.h"
#include "vhproxytest.h"
//#include <QWebEngineProfile>
#include "Msg_MainUI.h"
#include "Msg_CommonToolKit.h"
#include "Msg_VhallRightExtraWidget.h"
#include "VhallNetWorkInterface.h"
#include "ICommonData.h"
#include "IMainUILogic.h"
#include "IOBSControlLogic.h"
#include "CheckVersionDlg.h"

#include "AlertTipsDlg.h"
#include "ConfigSetting.h"
#include "charset.h"
#include "vhnetworktips.h"
#include "vhnetworkproxy.h"
#include "vhallloginwidget.h"
#include "pathManage.h"
#include "pathManager.h"
#include "SocketClient.h"
#include "pub.Const.h"

#define DEF_DEAL_MSG_TIMER_ID					100
#define DEF_DEAL_MSG_TIMER_INTERVAL			30

std::wstring gCurrentVersion = L"0.0.0.0";
std::string gCurStreamID = "0";
std::string gToken = "0";
std::wstring gCurClientType = L"vhallLive";

using namespace std;

bool stringToWString(const string& narrowStr, wstring& wStr) {
   size_t wcsLen;
   DWORD errorCode = ERROR_SUCCESS;
   string errorMsg;

   if (narrowStr.length() == 0) {
      wStr.clear();
   }
   wcsLen = MultiByteToWideChar(CP_UTF8, 0, narrowStr.c_str(), -1, NULL, 0);
   if (wcsLen == 0) {
      errorCode = GetLastError();
      TRACE6("stringToWString: Error getting buffer size, error = %0x08x", errorCode);
      return false;
   }

   WCHAR *wcsStr = new WCHAR[wcsLen + 1];
   wcsLen = MultiByteToWideChar(CP_UTF8, 0, narrowStr.c_str(), -1, wcsStr, (int)wcsLen);
   if (wcsLen == 0) {
      errorCode = GetLastError();
      TRACE6("stringToWString: MultiByteToWideChar failed, error = %0x08x.", errorCode);
      return false;
   }
   wcsStr[wcsLen] = L'\0';
   wStr = wcsStr;
   delete[] wcsStr;
   return true;
}

void ReplaceString(std::wstring& str, const std::wstring& src, const std::wstring& des) {
   int pos = -1;
   int curPos = 0;
   while (-1 != (pos = str.find(src, curPos))) {
      str.replace(pos, src.size(), des);
      curPos = pos + des.size();
   }
}
LPWSTR GetLastErrorText(DWORD lastErr = GetLastError());

CRuntimeInstance::STRU_MESSAGE_ITEM::STRU_MESSAGE_ITEM()
   : m_dwSenderID(0)
   , m_dwMessageID(0)
   , m_pData(NULL)
   , m_dwLen(0) {
}

CRuntimeInstance::STRU_MESSAGE_ITEM::~STRU_MESSAGE_ITEM() {
   if (NULL != m_pData) {
      CGlobalMemPool::Free(m_pData);
   }
   m_pData = NULL;
}

BOOL CRuntimeInstance::STRU_MESSAGE_ITEM::SetData(const void * apData, DWORD adwLen) {
   if (NULL != m_pData) {
      CGlobalMemPool::Free(m_pData);
   }

   m_pData = NULL;
   m_dwLen = 0;

   if (NULL == apData || adwLen < 1) {
      return TRUE;
   }

   m_pData = CGlobalMemPool::Malloc(adwLen);

   if (NULL == m_pData) {
      return FALSE;
   }

   memcpy(m_pData, apData, adwLen);
   m_dwLen = adwLen;

   return TRUE;
}

//-----------------------------------------------------------------------------------------------------//
CRuntimeInstance::CRuntimeInstance(void)
   : m_lRefCount(1)
   , m_dwCycleMsgDealNum(30)
   , m_lCurrentDealMessage(0)
   , m_pCheckVerDlg(NULL)
   , m_pDelLogTimer(NULL),
   mIDisplayCutRecord(eDispalyCutRecord_Hide) {
   mConfigPath = CPathManager::GetConfigPath();
}


CRuntimeInstance::~CRuntimeInstance(void) {
   if (NULL != m_pCheckVerDlg)
   {
      delete m_pCheckVerDlg;
      m_pCheckVerDlg = NULL;
   }
   if (NULL != m_pDelLogTimer)
   {
      if (m_pDelLogTimer->isActive())
      {
         m_pDelLogTimer->stop();
      }
      delete m_pDelLogTimer;
      m_pDelLogTimer = NULL;
   }
}

HRESULT STDMETHODCALLTYPE CRuntimeInstance::QueryInterface(REFIID riid, void ** appvObject) {
   if (NULL == appvObject) {
      return CRE_INVALIDARG;
   }

   if (riid == IID_VHIUnknown) {
      *appvObject = (VH_IUnknown*)this;
      AddRef();
      return CRE_OK;
   }
   else if (riid == IID_IMessageEvent) {
      *appvObject = (IMessageEvent*)this;
      AddRef();
      return CRE_OK;
   }

   return CRE_NOINTERFACE;
}

ULONG STDMETHODCALLTYPE CRuntimeInstance::AddRef(void) {
   ::InterlockedIncrement(&m_lRefCount);
   return m_lRefCount;
}

ULONG STDMETHODCALLTYPE CRuntimeInstance::Release(void) {
   ::InterlockedDecrement(&m_lRefCount);
   return m_lRefCount;
}

//与 IMessageDispatcher 的连接
HRESULT STDMETHODCALLTYPE CRuntimeInstance::Connect(IMessageDispatcher * apMessageDispatcher) {
   return CRE_OK;
}

//断开与 IMessageDispatcher 的连接
HRESULT STDMETHODCALLTYPE CRuntimeInstance::Disconnect() {
   return CRE_OK;
}

//接收数据
HRESULT STDMETHODCALLTYPE CRuntimeInstance::OnRecvMessage(DWORD adwSenderID, DWORD adwMessageID, void * apData, DWORD adwLen) {
   STRU_MESSAGE_ITEM* lpMessage = NULL;

   //分配
   lpMessage = new STRU_MESSAGE_ITEM();

   //分配失败
   if (NULL == lpMessage) {
      return CRE_FALSE;
   }

   //保存参数
   lpMessage->m_dwSenderID = adwSenderID;
   lpMessage->m_dwMessageID = adwMessageID;
   lpMessage->SetData(apData, adwLen);

   m_oCriticalSectionMQ.Lock();
   m_oMessageQueue.push_back(lpMessage);
   m_oCriticalSectionMQ.UnLock();

   return CRE_OK;
}

// // 注册消息
//HRESULT CRuntimeInstance::RegisterMessage(BOOL bIsReg) {
//   //// 获取主消息派发起
//   //VH::CComPtr<IMessageDispatcher> ptrMessageDispatcher;
//
//   ////获取消息派发器接口
//   //if (!GetMessageDispatcher(ptrMessageDispatcher)) {
//   //   ASSERT(FALSE);
//   //   return CRE_FALSE;
//   //}
//
//   //if (bIsReg) {
//   //   ptrMessageDispatcher->RegisterMessage(MSG_HTTPCENTER_HTTP_RS, 0, this);
//
//   //} else {
//   //   ptrMessageDispatcher->UnRegisterMessage(MSG_HTTPCENTER_HTTP_RS, this);
//   //}
//
//   return CRE_OK;
//}

// 创建
HRESULT CRuntimeInstance::Create() {
   //获取程序路径
   wstring path = GetAppPath();
   wcsncpy_s(m_szGGSysPath, path.c_str(), MAX_PATH);
   TRACE6("%s %ws\n", __FUNCTION__, m_szGGSysPath);

   //创建消息定时器
   m_pQTimer = new QTimer(this);
   if (NULL == m_pQTimer) {
      TRACE6("%s NULL == m_pQTimer\n", __FUNCTION__);
      ASSERT(FALSE);
      DestorySplashTip();
      return CRE_FALSE;
   }

   m_pQTimer->setInterval(30);
   connect(m_pQTimer, SIGNAL(timeout()), this, SLOT(updateClock()));
   m_pQTimer->start();
   // 保存主线程ID
   m_dwCurThreadID = GetCurrentThreadId();

   //初始化插件UI线程管理器，实际上，应该保证这个调用是在UI线程中进行的
   CSingletonMainUIThreadMgr::Instance().Init(m_dwCurThreadID);

   // 初始化插件管理
   if (CRE_OK != CSingletonRuntimePlugManager::Instance().InitSystem(m_szGGSysPath, m_param.bLoadExtraRightWidget)) {
      TRACE6("CSingletonRuntimePlugManager::Instance().InitSystem err");
      DestorySplashTip();
      ASSERT(FALSE);
      return CRE_FALSE;
   }
   TRACE6("%s CSingletonRuntimePlugManager::Instance().InitSystem ok\n", __FUNCTION__);
   //// 注册消息
   TRACE6("%s ok\n", __FUNCTION__);
   VH::CComPtr<ICommonData> pCommonData;
   CSingletonRuntimePlugManager::Instance().GetPluginInterface(PID_ICommonToolKit, IID_ICommonData, pCommonData);
   if (NULL != pCommonData) {
      pCommonData->SetStartMode(mIStartMode);
   }

   VH::CComPtr<IMainUILogic> pMainUILogic;
   CSingletonRuntimePlugManager::Instance().GetPluginInterface(PID_IMainUI, IID_IMainUILogic, pMainUILogic);
   if (NULL != pMainUILogic) {
      pMainUILogic->SetCutRecordDisplay(mIDisplayCutRecord);
   }
   DestorySplashTip();
   return CRE_OK;
}

void CRuntimeInstance::DestorySplashTip() {
   if (m_pCheckVerDlg) {
      delete m_pCheckVerDlg;
      m_pCheckVerDlg = nullptr;
   }
}

// 销毁
HRESULT CRuntimeInstance::Destory() {
   if (m_pCheckVerDlg) {
      delete m_pCheckVerDlg;
      m_pCheckVerDlg = NULL;
   }
   DestorySplashTip();
   ////反注册消息
   //if (CRE_OK != RegisterMessage(FALSE)) {
   //   ASSERT(FALSE);
   //}
   //关闭消息派发器
   CloseMessageDispatcher();

   m_oCriticalSectionMQ.Lock();
   while (1) {
      if (m_oMessageQueue.empty()) {
         break;
      }

      STRU_MESSAGE_ITEM* lpMessage = m_oMessageQueue.front();
      m_oMessageQueue.pop_front();
      if (NULL != lpMessage) {
         delete lpMessage;
         lpMessage = NULL;
      }
   }
   m_oCriticalSectionMQ.UnLock();

   // 反初始化插件管理
   if (CRE_OK != CSingletonRuntimePlugManager::Instance().UnInitSystem()) {
      ASSERT(FALSE);
   }
   //反初始化插件UI线程管理器  
   CSingletonMainUIThreadMgr::Instance().UnInit();

   //反注册TIMER
   if (NULL != m_pQTimer) {
      m_pQTimer->stop();

      delete m_pQTimer;
      m_pQTimer = NULL;
   }
   return CRE_OK;
}
HRESULT CRuntimeInstance::SetSocketClient(FuncNotifyService f)
{
   VH::CComPtr<IOBSControlLogic> pObsControlLogic;
   CSingletonRuntimePlugManager::Instance().GetPluginInterface(PID_IOBSControl, IID_IOBSControlLogic, pObsControlLogic);
   if (!pObsControlLogic)
   {
      return CRE_FALSE;
   }

   //pObsControlLogic->SetSocketClient(f);

   return CRE_OK;
}



//更新检测
BOOL CRuntimeInstance::RunAdminCheck(wstring cmdLine) {
   TRACE6("CRuntimeInstance::RunAdminCheck \n");
   if (false/*!IsRunAsAdmin()*/) {
      TRACE6("RunAdminCheck not admin\n");
      wchar_t szPath[MAX_PATH];
      if (GetModuleFileName(NULL, szPath, ARRAYSIZE(szPath))) {
         TRACE6("will run as admin [%ws]\n", cmdLine.c_str());
         ElevatePrivileges(szPath, cmdLine.c_str());
         return FALSE;
      }
   }
   else {
      TRACE6("RunAdminCheck admin 1\n");
   }
   return TRUE;
}

BOOL CRuntimeInstance::AppExistCheck(bool skipAppCheck) {
   if (skipAppCheck) {
      return FALSE;
   }

   gNameCore = CreateMutex(NULL, FALSE, L"VhallLive");
   if (GetLastError() == ERROR_ALREADY_EXISTS) {
      CloseHandle(gNameCore);
      gNameCore = NULL;
      Sleep(10000);
      gNameCore = CreateMutex(NULL, FALSE, L"VhallLive");

      m_param.pgNameCore = &gNameCore;
      if (GetLastError() == ERROR_ALREADY_EXISTS) {
         AlertTipsDlg alertDlg(EXIST_RUNNING_APP, false, NULL);
         alertDlg.SetYesBtnText(KEEP_WATING);
         alertDlg.setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
         alertDlg.exec();

         CloseHandle(gNameCore);
         gNameCore = NULL;
         return TRUE;
      }
   }
   return FALSE;
}

BOOL CRuntimeInstance::CreateServerConnect(string sStreamName, bool bMengZhu) {
   if (!bMengZhu) {
      gSocketClient = new SocketClient(SERVICE_ADDR, SERVER_PORT);
   }
   else {
      gSocketClient = new SocketClient(SERVICE_ADDR, MENGZHU_SERVER_PORT);
   }

   if (gSocketClient == NULL || gSocketClient->Start() == false) {
      MessageBoxW(GetDesktopWindow(), L"请确认权限，没有找到vhallService运行！", APP_NAME, MB_OK | MB_TOPMOST);
      TRACE6("Can't connect to service.  %ws", GetLastErrorText(GetLastError()));
      return false;
   }

   char* szStreamName = new char[256];
   strcpy(szStreamName, sStreamName.c_str());
   HANDLE hThread = CreateThread(0, 0, CheckStopThead, szStreamName, NULL, NULL);
   //accept client, and create thread.
   if (hThread == NULL) {
      TRACE6("ExecVhallClient  _beginthread openProcessThreadProc failed  %ws", GetLastErrorText(GetLastError()));
      return false;
   }

   return true;
}
void CRuntimeInstance::NotifyService(char *msg) {
   CSingletonRuntimeInstance::Instance().NotifyMsg(msg);
}
SocketClient *CRuntimeInstance::GetSocketClient() {
   return gSocketClient;
}
HANDLE *CRuntimeInstance::GetNameCore() {
   return &gNameCore;
}

void CRuntimeInstance::NotifyMsg(char *msg) {
   if (!msg) {
      return;
   }

   char resbuff[256];
   sprintf_s(resbuff, 256, MSG_BODY, msg, gCurStreamID.c_str());
   if (gSocketClient)
      gSocketClient->Send(resbuff, strlen(resbuff));
}
bool *g_bExit = NULL;
DWORD __stdcall CRuntimeInstance::CheckStopThead(void* param) {
   SocketClient* socketClient = CSingletonRuntimeInstance::Instance().GetSocketClient();
   HANDLE *nameCore = CSingletonRuntimeInstance::Instance().GetNameCore();
   Sleep(5000);
   char resbuff[256];
   char* szStreamName = (char*)param;
   int checkConnectedCount = 0;
   while (true) {
      if (socketClient->IsConnected()) {
         sprintf_s(resbuff, 256, MSG_BODY, MSG_TYPE_HEARBEAT, szStreamName);
         socketClient->Send(resbuff, strlen(resbuff));
         checkConnectedCount = 0;
      }
      else
      {
         if (socketClient->StopImmediately()) {
            TRACE6("Flash Close VhallLive  %d\n", checkConnectedCount);
            break;
         }
         else {
            TRACE6("Flash not close VhallLive\n");
         }

         TRACE6("AS3 miss %d\n", checkConnectedCount);
         checkConnectedCount++;
         if (checkConnectedCount > 15) {
            TRACE6("CheckStopThead will close VhallLive\n");
            break;
         }

         Sleep(500);
      }

      socketClient->WaitEvent(1000);
   }

   TRACE6("waitForStopEventThead will exit.\n");
   TRACE6("log as3 client, shutdown application");
   if (nameCore) {
      if (*nameCore) {
         CloseHandle(*nameCore);
         *nameCore = NULL;
      }
   }
   *g_bExit = true;
   qApp->quit();
   delete param;
   return 1;
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
QJsonObject CRuntimeInstance::Get(QString method, QString param) {
   VHHttp http;
   QString domain = m_domain + "/api/client/v1/clientapi/";
   QString url = domain + method + "?token=" + m_msgToken;
   if (param.length() > 0) {
      url = url + "?" + param;
   }

   TRACE6("network get!");
   TRACE6(url.toLocal8Bit().data());

   return http.Get(url, 10000);
}

bool CRuntimeInstance::AnalysisStartParam(int argc, char **argv, VHStartParam &param) {
   TRACE6("CRuntimeInstance::AnalysisStartParam argc is %d\n", argc);
   for (int i = 0; i < argc; i++) {
      TRACE6("argv[%d]:%s\n", i, argv[i]);
   }

   mIStartMode = eStartMode_flashNoDispatch;
   memset(&param, 0, sizeof(VHStartParam));
   param.bLoadExtraRightWidget = false;
   m_bSkipAppCheck = false;

   //wstring confPath = GetAppPath2() + CONFIGPATH;
   QString qsConfPath = CPathManager::GetConfigPath();
   int is_http_proxy = ConfigSetting::ReadInt(qsConfPath, GROUP_DEFAULT, PROXY_OPEN, 0);
   if (is_http_proxy) {
      QString host = ConfigSetting::ReadString(qsConfPath, GROUP_DEFAULT, PROXY_HOST, "");
      int port = ConfigSetting::ReadInt(qsConfPath, GROUP_DEFAULT, PROXY_PORT, 80);
      QString usr = ConfigSetting::ReadString(qsConfPath, GROUP_DEFAULT, PROXY_USERNAME, "");
      QString pwd = ConfigSetting::ReadString(qsConfPath, GROUP_DEFAULT, PROXY_PASSWORD, "");

      QNetworkProxy proxy;
      proxy.setType(QNetworkProxy::HttpProxy);
      proxy.setHostName(host);
      proxy.setPort(port);
      proxy.setUser(usr);
      proxy.setPassword(pwd);
      QNetworkProxy::setApplicationProxy(proxy);
   }
   else {
      QNetworkProxy proxy;
      proxy.setType(QNetworkProxy::NoProxy);
      QNetworkProxy::setApplicationProxy(proxy);
   }
   ConfigSetting::writeValue(qsConfPath, GROUP_DEFAULT, KEY_DESKCTOP_CAPTURE_TYPE, 0);
   //desktop  桌面启动
   if (argc == 1 || (argc == 3 && (strcmp(argv[1], "-r") == 0 && strcmp(argv[2], "restart") == 0))) {
      if (!CheckVersionAndUpdate()) {
         DestorySplashTip();
         return FALSE;
      }
      UpdateLogIn(eStartMode_desktop);
      m_param.m_eType = widget_show_type_logIn;
      mIStartMode = eStartMode_desktop;
      param.bLoadExtraRightWidget = true;
      m_bSkipAppCheck = true;
      //首先进行管理员权限检测
      if (false/*!IsRunAsAdmin()*/) {
         TRACE6("RunAdminCheck not admin\n");
         wchar_t szPath[MAX_PATH];
         if (GetModuleFileName(NULL, szPath, ARRAYSIZE(szPath))) {
            ElevatePrivileges(szPath, L"");
            DestorySplashTip();
            return FALSE;
         }
      }
   }
   //VhallLive://  私有协议启动  网页发起直播助手
   else if (argc == 2) {
      if (!CheckVersionAndUpdate()) {
         DestorySplashTip();
         return FALSE;
      }
      ConfigSetting::writeValue(CPathManager::GetConfigPath(), GROUP_DEFAULT, KEY_CRASH_LIVE_URL, "");
      ConfigSetting::writeValue(CPathManager::GetConfigPath(), GROUP_DEFAULT, KEY_CRASH_LIVE_NAME, "");
      m_param.m_eType = widget_show_type_Main;
      mIStartMode = eStartMode_private;
      param.bLoadExtraRightWidget = true;
      param.bConnectToVhallService = false;

      string cmd = argv[1];
      mPrivateStartInfo = QString::fromStdString(cmd);
      QJsonDocument doc;
      QJsonObject tobj = AnalysisUrl(argv[1]);
      strcpy(param.streamName, tobj["stream_name"].toString().toUtf8().data());
      param.bHideLogo = (tobj["hide_logo"].toString() != "0");
      UpdateLogIn(eStartMode_private, tobj["stream_name"].toString());
   }
   else if (argc == 3) {
      //if (strcmp(argv[1], "-r") == 0 && strcmp(argv[2], "restart") == 0) {
      //    TRACE6("CRuntimeInstance::AnalysisStartParam restart\n");
      //    //if (GetCommand(m_param.startUpCmd)) {
      //    //    wchar_t szPath[MAX_PATH];
      //    //    if (GetModuleFileName(NULL, szPath, ARRAYSIZE(szPath))) {
      //    //        TRACE6("will run as admin [%ws]\n", m_param.startUpCmd);
      //    //        SHELLEXECUTEINFO sei = { sizeof(sei) };
      //    //        sei.lpVerb = L"runas";
      //    //        sei.lpFile = szPath;
      //    //        sei.hwnd = NULL;
      //    //        sei.lpParameters = m_param.startUpCmd;
      //    //        sei.nShow = SW_NORMAL;
      //    //        if (!ShellExecuteEx(&sei)) {
      //    //            DWORD dwError = GetLastError();
      //    //            if (dwError == ERROR_CANCELLED) {
      //    //                // The user refused to allow privileges elevation.
      //    //                // Do nothing ...
      //    //                return false;
      //    //            }
      //    //        }
      //    //        if (!CheckVersionAndUpdate()){
      //    //            return FALSE;
      //    //        }
      //    //    }
      //    //}
      //    CheckVersionAndUpdate();
      //}
   }
   //vhallservice
   else if (argc >= 13) {
      if (!CheckVersionAndUpdate()) {
         DestorySplashTip();
         return FALSE;
      }
      ConfigSetting::writeValue(CPathManager::GetConfigPath(), GROUP_DEFAULT, KEY_CRASH_LIVE_URL, "");
      ConfigSetting::writeValue(CPathManager::GetConfigPath(), GROUP_DEFAULT, KEY_CRASH_LIVE_NAME, "");
      m_param.m_eType = widget_show_type_Main;
      param.bHideLogo = (0 == strcmp(argv[5], "1") ? true : false);
      strcpy_s(param.userName, sizeof(param.userName), argv[1]);
      string strAddr;
      ReadAdress(strAddr);
      TRACE6("ReadAdress  out: %s", strAddr.c_str());
      //strcpy_s(param.streamURLS, sizeof(param.streamURLS), G2U(argv[2]));
      strcpy_s(param.streamURLS, sizeof(param.streamURLS), strAddr.c_str());
      TRACE6("CRuntimeInstance  streamURLS: %s", param.streamURLS);
      strcpy_s(param.streamToken, sizeof(param.streamToken), argv[3]);
      strcpy_s(param.streamName, sizeof(param.streamName), argv[4]);
      strcpy_s(param.channelID, sizeof(param.channelID), "123456");
      strcpy_s(param.userId, sizeof(param.userId), argv[8]);
      strcpy_s(param.userRole, sizeof(param.userRole), argv[9]);
      strcpy_s(param.webinarType, sizeof(param.webinarType), argv[10]);
      strcpy_s(param.accesstoken, sizeof(param.accesstoken), argv[11]);
      strcpy_s(param.scheduler, sizeof(param.scheduler), argv[12]);

      ConfigSetting::writeValue(mConfigPath, GROUP_DEFAULT, LOG_ID_BASE, QString(param.userId));
      if (strcmp(param.scheduler, "NULL") == 0) {
         mIStartMode = eStartMode_flashNoDispatch;
      }
      else {
         mIStartMode = eStartMode_flash;
      }
      UpdateLogIn(eStartMode_flash, QString(param.streamName));
      //#ifndef _DEBUG
      param.bConnectToVhallService = true;
      wstring userName,
         rtmpAddress,
         token,
         streamName,
         chanelId,
         userId,
         userRole,
         webinarType,
         accesstoken,
         scheduler;

      string sRtmpUrl,
         sUserName,
         sToken,
         sStreamName,
         sChanelId,
         suserId,
         suserRole,
         swebinarType,
         saccesstoken,
         sscheduler;

      sRtmpUrl = param.streamURLS;
      sUserName = param.streamName;
      sToken = param.streamToken;
      sStreamName = param.streamName;
      sChanelId = param.channelID;
      suserId = param.userId;
      suserRole = param.userRole;
      swebinarType = param.webinarType;
      saccesstoken = param.accesstoken;

      string utf8Str = stringToUtf8(sRtmpUrl);
      rtmpAddress = Utf8ToWstring(utf8Str);
      stringToWString(sUserName, userName);
      stringToWString(sToken, token);
      stringToWString(sStreamName, streamName);
      stringToWString(sChanelId, chanelId);
      stringToWString(suserId, userId);
      stringToWString(suserRole, userRole);
      stringToWString(swebinarType, webinarType);
      stringToWString(saccesstoken, accesstoken);
      stringToWString(sscheduler, scheduler);
      ReplaceString(rtmpAddress, L"\"", L"\\\"");
      wstring cmdLine = userName
         + L" \""
         + rtmpAddress
         + L"\" "
         + token
         + L" "
         + streamName
         + L" "
         + (param.bHideLogo ? L"1" : L"0")
         + L" "
         + (L"0")
         + L" "
         + chanelId
         + L" "
         + userId
         + L" "
         + userRole
         + L" "
         + webinarType
         + L" "
         + accesstoken
         + L" "
         + scheduler;

      wcscpy(param.startUpCmd, cmdLine.c_str());
      TRACE6("CRuntimeInstance::AnalysisStartParam %s", QString::fromWCharArray(param.startUpCmd).toUtf8().data());
   }
   return true;
}

void CRuntimeInstance::ReadAdress(string& strAddr)
{
   WCHAR logFilePath[_MAX_PATH];
   GetWindowsDirectoryW(logFilePath, _MAX_PATH);

   char output[_MAX_PATH];
   memset(output, 0, _MAX_PATH);
   sprintf_s(output, "%ws\\address.ini", logFilePath);
   TRACE6("ReadAdress Path: %s", output);
   ifstream infile;
   infile.open(output);
   if (!infile) return;
   getline(infile, strAddr);
   TRACE6("ReadAdress : %s", strAddr.c_str());
}

void CRuntimeInstance::SaveStartParam(const VHStartParam &param) {

   wchar_t appPath[MAX_BUFF] = { 0 };

   wstring path = GetAppPath();
   wcsncpy_s(appPath, path.c_str(), MAX_PATH);

   QString qAppPath = QString::fromWCharArray(appPath);


   //存储流信息
   PublishInfo loStreamInfo;
   VH::CComPtr<ICommonData> pCommonData;
   CSingletonRuntimePlugManager::Instance().GetPluginInterface(PID_ICommonToolKit, IID_ICommonData, pCommonData);
   if (NULL == pCommonData) {
      ASSERT(FALSE);
      return;
   }

   loStreamInfo.init(param.userName,
      param.streamURLS,
      param.streamToken,
      param.streamName,
      param.channelID,
      param.bHideLogo
   );

   loStreamInfo.mUserId = param.userId;//流名与用户ID的组合   
   loStreamInfo.mWebinarType = param.webinarType;//flash传递参数，是否为多嘉宾活动 ismix 
   loStreamInfo.mAccesstoken = param.accesstoken;//flash传递参数
   loStreamInfo.mRole = param.userRole;
   loStreamInfo.mScheduler = param.scheduler;

   loStreamInfo.InitMultitcpPubLine(qAppPath + "\CustomStreamURL.txt");

   pCommonData->SetStreamInfo(&loStreamInfo);
   TRACE6("UserName = %s, RtmpAddress = %s, Token = %s, StreamName = %s, IsHideLogo = %s, CurRtmpUrl = %s \n",
      "VhallUser",
      loStreamInfo.mRtmpUrlsStr.c_str(), loStreamInfo.mToken.c_str(), loStreamInfo.mStreamName.c_str(),
      loStreamInfo.m_bHideLogo ? "1" : "0", loStreamInfo.mCurRtmpUrl.c_str());

   //记录版本号

   //记录当前流ID(崩溃上报使用)
   gCurStreamID = loStreamInfo.mStreamName;
   //pCommonData->SetStartUpMode(param.startMode);
}

void CRuntimeInstance::customEvent(QEvent* event) {
   if (event) {
      switch (event->type())
      {
         case CustomEvent_AppExit: {
            HandleAppExit();
            break;
         }
         case CustomEvent_QuitApp: {
            *g_bExit = true;
            QApplication::quit();
            break;
         }
         default: {
            HandleBuParam(event);
            break;
         }
      }
   }
}

void CRuntimeInstance::HandleAppExit() {
   TRACE6("Application CSingletonRuntimeInstance::Instance().ShutdownSystem()\n");
   // 关闭
   if (CRE_OK != ShutdownSystem()) {
      TRACE6("CSingletonRuntimeInstance::Instance().ShutdownSystem 失败 \n");
      ASSERT(FALSE);
   }
   TRACE6("Application CSingletonRuntimeInstance::Instance().Destory()\n");
   // 销毁                                                                                        
   if (CRE_OK != Destory()) {
      TRACE6("CSingletonRuntimeInstance::Instance().Destory 失败 \n");
      ASSERT(FALSE);
   }
   QApplication::postEvent(this,new QEvent(CustomEvent_QuitApp));
}

void CRuntimeInstance::HandleBuParam(QEvent* event) {
   QHttpResponeEvent* authEvent = dynamic_cast<QHttpResponeEvent*>(event);
   if (authEvent) {
      int code = authEvent->mCode;
      QString msg = authEvent->mData;
      if (code == 0 && msg.length() > 0) {
         QByteArray ba(msg.toStdString().c_str(), msg.length());
         QJsonDocument doc = QJsonDocument::fromJson(ba);
         QJsonObject obj = doc.object();
         int code = 0;
         if (obj["code"].isString()) {
            code = obj["code"].toString().toInt();
         }
         else {
            code = obj["code"].toInt();
         }
         QString msg = obj["msg"].toString();
         if (code == 200) {
            QJsonObject data = obj["data"].toObject();
            if (data.contains("bu")) {
               int bu = data["bu"].toInt();
               bool bRet = ConfigSetting::writeValue(CPathManager::GetToolConfigPath(), GROUP_DEFAULT, KEY_VHALL_BU, bu);
               if (bRet) {
                  TRACE6("%s write bu:%d\n", __FUNCTION__, bu);
               }
            }
         }
      }
   }
}

void CRuntimeInstance::GetAuthConfig() {
   QByteArray respData;
   HttpRequest req;
   QString domain = ConfigSetting::ReadString(CPathManager::GetToolConfigPath(), GROUP_DEFAULT, KEY_URL_DOMAIN, domainUrl);
   QString resqUrl = domain + "/api/vhallassistant/auth/config";
   TRACE6("%s resqUrl:%s\n", __FUNCTION__, resqUrl.toStdString().c_str());


   //代理配置
   QString proxyHost;
   QString proxyUsr;
   QString proxyPwd;
   int proxyPort;
   int is_http_proxy = ConfigSetting::ReadInt(mConfigPath, GROUP_DEFAULT, PROXY_OPEN, 0);
   if (is_http_proxy) {
      TRACE6("%s is_http_proxy:%d\n", __FUNCTION__, is_http_proxy);
      proxyHost = ConfigSetting::ReadString(mConfigPath, GROUP_DEFAULT, PROXY_HOST, "");
      proxyPort = ConfigSetting::ReadInt(mConfigPath, GROUP_DEFAULT, PROXY_PORT, 80);
      proxyUsr = ConfigSetting::ReadString(mConfigPath, GROUP_DEFAULT, PROXY_USERNAME, "");
      proxyPwd = ConfigSetting::ReadString(mConfigPath, GROUP_DEFAULT, PROXY_PASSWORD, "");
   }
   HTTP_GET_REQUEST request(resqUrl.toStdString());
   QObject *recvObj = this;
   GetHttpManagerInstance()->HttpGetRequest(request, [&, recvObj](const std::string& msg, int code, const std::string userData) {
      if (recvObj) {
         QCoreApplication::postEvent(recvObj, new QHttpResponeEvent(CustomEvent_AuthCode, code, QString::fromStdString(msg)));
      }
   });
}

bool CRuntimeInstance::CheckVersionAndUpdate() {
   GetAuthConfig();

   TRACE6("%s start\n", __FUNCTION__);
   //先判断当前app是否能够支持在线更新。
   QString toolConfPath = CPathManager::GetAppDataPath() + QString::fromStdWString(VHALL_TOOL_CONFIG);
   int appEnableUpdata = ConfigSetting::ReadInt(toolConfPath, GROUP_DEFAULT, KEY_VHALL_APPUPDATE_ENABLE, 1);
   if (appEnableUpdata == 0) {
      TRACE6("%s appEnableUpdata:%d\n", __FUNCTION__, appEnableUpdata);
      return true;
   }

   if (NULL == m_pCheckVerDlg) {
      TRACE6("%s new CCheckVersionDlg\n", __FUNCTION__);
      m_pCheckVerDlg = new CCheckVersionDlg;
   }
   m_pCheckVerDlg->SetSplashHide(m_bHideSplash);
   ConfigSetting::MoveWindow(m_pCheckVerDlg);
   TRACE6("%s SetSplashHide\n", __FUNCTION__);
   if (m_pCheckVerDlg->exec() == QDialog::Accepted) {
      TRACE6("%s m_pCheckVerDlg->exec() == QDialog::Accepted\n", __FUNCTION__);
      m_wsCurrentVersion = m_pCheckVerDlg->GetVersion();
      QString newClient = m_pCheckVerDlg->GetDownloadFilePath();
      if (newClient.length() > 0) {
         WCHAR tmp[1024] = L"";
         newClient.toWCharArray(tmp);
         SaveCommand();
         TRACE6("Will run update [%ws]", tmp);
         //执行新的客户端程序
         RunProcess(tmp, NULL);
         delete m_pCheckVerDlg;
         m_pCheckVerDlg = NULL;
         return false;
      }
   }
   else {
      if (m_pCheckVerDlg && !m_pCheckVerDlg->GetSuspensionLoad()){
         TRACE6("%s m_pCheckVerDlg->GetVersion()\n", __FUNCTION__);
         m_wsCurrentVersion = m_pCheckVerDlg->GetVersion();
      }
   }
   if (m_pCheckVerDlg) {
      m_pCheckVerDlg->Release();
      m_pCheckVerDlg->SetSplashHide(false);
      m_pCheckVerDlg->show();
   }

   TRACE6("%s end\n", __FUNCTION__);
   return true;
}

HRESULT CRuntimeInstance::CheckParam(int argc, char *argv[]) {
   wchar_t appPath[MAX_BUFF] = { 0 };
   QString qsConfPath = CPathManager::GetConfigPath();
   //读取配置
   QString domain = ConfigSetting::ReadString(CPathManager::GetToolConfigPath(), GROUP_DEFAULT, KEY_URL_DOMAIN, domainUrl);
   if (domain == "") {
      domain = "http://e.vhall.com";
      ConfigSetting::writeValue(CPathManager::GetToolConfigPath(), GROUP_DEFAULT, KEY_URL_DOMAIN, domain);
   }

   int iAudioSampleRate = ConfigSetting::ReadInt(qsConfPath, GROUP_DEFAULT, KEY_AUDIO_SAMPLERATE, 44100);
   if (44100 != iAudioSampleRate) {
      ConfigSetting::writeValue(qsConfPath, GROUP_DEFAULT, KEY_AUDIO_SAMPLERATE, 44100);
   }
   //实例存在检测
   if (AppExistCheck(m_bSkipAppCheck)) {
      TRACE6("CRuntimeInstance::CheckParam AppExistCheck Failed\n");
      return CRE_FALSE;
   }

   m_domain = domain;
   if (!AnalysisStartParam(argc, argv, m_param)) {

      TRACE6("CRuntimeInstance::CheckParam AnalysisStartParam Failed!\n");
      return CRE_FALSE;
   }

   //管理员权限检测
   if (!RunAdminCheck(m_param.startUpCmd)) {
      TRACE6("CRuntimeInstance::CheckParam RunAdminCheck Failed\n");
      DestorySplashTip();
      return CRE_FALSE;
   }

   TRACE6("CRuntimeInstance::CheckParam RunAdminCheck Successed\n");
   return CRE_OK;
}
void CRuntimeInstance::SaveCommand() {
   if (wcslen(m_param.startUpCmd) == 0) {
      //MessageBoxA(NULL,"m_param.startUpCmd len is 0","CRuntimeInstance::SaveCommand",0);
      return;
   }

   //MessageBoxW(NULL,m_param.startUpCmd,L"WillSave",0);

   QString qResetConfPath = QDir::tempPath() + "/vhallLive.reset";
   QByteArray ba = QString::fromWCharArray(m_param.startUpCmd).toUtf8().toBase64();
   QFile f;
   f.setFileName(qResetConfPath);
   if (f.open(QIODevice::WriteOnly)) {
      f.write(ba);

      f.close();
   }
}
bool CRuntimeInstance::GetCommand(wchar_t *cmd) {
   QString qResetConfPath = QDir::tempPath() + "/vhallLive.reset";
   QFile f;
   f.setFileName(qResetConfPath);
   if (f.open(QIODevice::ReadOnly)) {
      QByteArray ba = f.readAll();
      f.close();
      f.remove();
      if (!ba.isEmpty()) {
         QString str = QByteArray::fromBase64(ba);
         str.toWCharArray(cmd);

         return true;
      }
   }

   return false;
}

void CRuntimeInstance::InitCurVersion()
{
   gCurrentVersion = GetVersion();
}

HRESULT CRuntimeInstance::StartSystem(bool *bExit) {
   g_bExit = bExit;
   SaveStartParam(m_param);

   //调试时不连接vhallservice
   //#ifndef _DEBUG
   VH::CComPtr<ICommonData> pCommonData = NULL;
   CSingletonRuntimePlugManager::Instance().GetPluginInterface(PID_ICommonToolKit, IID_ICommonData, pCommonData);
   if (pCommonData) {
      pCommonData->SetCurVersion(gCurrentVersion);
      pCommonData->SetPrivateStartParam(mPrivateStartInfo);
      pCommonData->RegisterRuntimeInstance(this);
   }

   if (m_param.bConnectToVhallService) {
      //创建与VhallService通信的Socket
      if (!CreateServerConnect(m_param.streamName, false)) {
         DestorySplashTip();
         return CRE_FALSE;
      }
      CSingletonRuntimeInstance::Instance().SetSocketClient(NotifyService);
   }

   //显示主界面
   STRU_MAINUI_WIDGET_SHOW loShow;
   loShow.m_bIsShow = TRUE;
   loShow.m_eType = m_param.m_eType;
   loShow.bExit = bExit;
   loShow.bLoadExtraRightWidget = m_param.bLoadExtraRightWidget;
   loShow.pgNameCore = m_param.pgNameCore;

   QString::fromStdWString(m_param.webinarName).toWCharArray(loShow.m_webinarName);
   QString::fromStdString(m_param.streamName).toWCharArray(loShow.m_streamName);
   QString::fromWCharArray(m_param.chat_url).toWCharArray(loShow.chat_url);
   loShow.m_bIsWebinarPlug = m_param.bWebinarPlug;
   QString::fromStdString(m_param.pluginsUrl).toWCharArray(loShow.m_plugUrl);

   mToken.toWCharArray(loShow.token);
   mListUrl.toWCharArray(loShow.m_listUrl);
   mPluginUrl.toWCharArray(loShow.m_plugUrl);
   mUserImageUrl.toWCharArray(loShow.m_imgUrl);
   mUserName.toWCharArray(loShow.m_userName);
   loShow.m_bShowTeaching = mShowTeaching;
   wcscpy(loShow.m_version, m_wsCurrentVersion.c_str());
   loShow.mbIsPwdLogin = m_param.bIsPwdLogin;
   wcscpy(loShow.m_roomid, m_param.roomId);
   wcscpy(loShow.m_roompwd, m_param.roompwd);

   //如果启动方式不是桌面启动，则启动时便需要初始化底层设备
   int iStartMode = eStartMode_flashNoDispatch;
   if (NULL != pCommonData) {
      iStartMode = pCommonData->GetStartMode();
   }
   if (eStartMode_private == iStartMode) {
      PostCRMessage(MSG_MAINUI_START_LIVE_PRIVATE, &loShow, sizeof(loShow));
   }
   else if (eStartMode_flash == iStartMode) {
      QString room_id = QString::fromStdString(m_param.streamName);
      QString stream_token = QString::fromStdString(m_param.streamToken);
      QString msg_token = QString::fromStdString(m_param.msgToken);
      int hide_logo = m_param.bHideLogo == true ? 1 : 0;
      pCommonData->SetStartLiveParam(stream_token, room_id, msg_token, hide_logo);
      PostCRMessage(MSG_MAINUI_WIDGET_SHOW, &loShow, sizeof(loShow));
      //PostCRMessage(MSG_COMMONDATA_DATA_INIT, NULL, 0);
   }
   else {
      VH::CComPtr<IMainUILogic> pMainUILogic;
      CSingletonRuntimePlugManager::Instance().GetPluginInterface(PID_IMainUI, IID_IMainUILogic, pMainUILogic);
      if (NULL != pMainUILogic) {
         pMainUILogic->ShowLoginDlg(bExit);
      }
   }
   TRACE6("%s end\n", __FUNCTION__);
   return CRE_OK;
}

HRESULT CRuntimeInstance::ShutdownSystem() {
   if (gNameCore) {
      CloseHandle(gNameCore);
      gNameCore = NULL;
   }

   return CRE_OK;
}

std::string CRuntimeInstance::GetStreamName() {
   //存储流信息
   PublishInfo loStreamInfo;
   VH::CComPtr<ICommonData> pCommonData;
   CSingletonRuntimePlugManager::Instance().GetPluginInterface(PID_ICommonToolKit, IID_ICommonData, pCommonData);
   if (NULL == pCommonData) {
      ASSERT(FALSE);
      return NULL;
   }
   pCommonData->GetStreamInfo(loStreamInfo);
   return loStreamInfo.mStreamName;
}

void CRuntimeInstance::updateClock() {
   CSingletonMainUIThreadMgr::Instance().OnTimerBroadcastDealMessage();

   // 消息分发
   //DispatchCachedMessage();
}


// 预处理
BOOL CRuntimeInstance::PreTranslateMessage(MSG* pMsg) {
   return CSingletonMainUIThreadMgr::Instance().OnPreTranslateMessage(pMsg);
}

// 投递消息
HRESULT CRuntimeInstance::PostCRMessage(DWORD adwMessageID, void * apData, DWORD adwLen) {
   VH::CComPtr<IMessageDispatcher> lptrMessageDispatcher;

   //取得消息派发器
   if (!GetMessageDispatcher(lptrMessageDispatcher)) {
      ASSERT(FALSE);
      return CRE_FALSE;
   }

   //不能取得消息派发器
   if (NULL == lptrMessageDispatcher) {
      return CRE_FALSE;
   }

   return lptrMessageDispatcher->PostCRMessage(ENUM_PLUGIN_RUNTIME, adwMessageID, apData, adwLen);
}

//派发缓存消息
HRESULT CRuntimeInstance::DispatchCachedMessage() {
   //for (DWORD i = 0; i < m_dwCycleMsgDealNum; i++) {
   //   STRU_MESSAGE_ITEM* lpCRMessage = NULL;

   //   do {
   //      m_oCriticalSectionMQ.Lock();
   //      if (m_oMessageQueue.size() > 0) {
   //         lpCRMessage = m_oMessageQueue.front();
   //         m_oMessageQueue.pop_front();
   //      }
   //      m_oCriticalSectionMQ.UnLock();
   //   } while (0);

   //   //没有消息了
   //   if (NULL == lpCRMessage) {
   //      break;
   //   }

   //   //通知处理消息
   //   InstanceDealMessage(lpCRMessage->m_dwSenderID, lpCRMessage->m_dwMessageID, lpCRMessage->m_pData, lpCRMessage->m_dwLen);

   //   //处理完后释放内存
   //   if (NULL != lpCRMessage) {
   //      delete lpCRMessage;
   //      lpCRMessage = NULL;
   //   }

   //   //累加处理消息数量
   //   ::InterlockedIncrement(&m_lCurrentDealMessage);

   //   //检查内存池
   //   TimerCheckMemPool();
   //}

   return CRE_OK;
}

//检查内存池
HRESULT CRuntimeInstance::TimerCheckMemPool() {
   if (m_lCurrentDealMessage > 1000) {
      TRACE6("m_lCurrentDealMessage > 1000 start\n");
      CGlobalMemPool::Recovery();
      ::InterlockedExchange(&m_lCurrentDealMessage, 0);
      TRACE6("m_lCurrentDealMessage > 1000 end\n");
   }
   return CRE_OK;
}

//处理消息
HRESULT CRuntimeInstance::InstanceDealMessage(DWORD adwSenderID, DWORD adwMessageID, void * apData, DWORD adwLen) {

   return CRE_OK;
}

const wchar_t * CRuntimeInstance::GetVersion() {
   if (m_wsCurrentVersion.empty()) {
      WCHAR				szApp[1024 * 4] = L"";
      GetModuleFileNameW(NULL, szApp, 1024 * 4);
      CCheckVersionDlg::GetCurVersion(szApp, m_wsCurrentVersion);
   }
   return m_wsCurrentVersion.c_str();
}

std::string CRuntimeInstance::GetEvents() {
   VH::CComPtr<ICommonData> pCommonData;
   CSingletonRuntimePlugManager::Instance().GetPluginInterface(PID_ICommonToolKit, IID_ICommonData, pCommonData);
   if (NULL == pCommonData) {
      ASSERT(FALSE);
      return "";
   }

   return pCommonData->GetEvents();
}
bool CRuntimeInstance::GetISRestart() {
   bool ret = bRestart;
   bRestart = false;
   return ret;
}
void CRuntimeInstance::NetworkError() {
   VHNetworkTips tips;
   tips.show();

   if (tips.ProxyConfigure()) {
      VHNetworkProxy proxy;
      proxy.show();
      this->bRestart = proxy.IsRestart();
   }
}


void CRuntimeInstance::startDelOverdueLog()
{
   if (NULL == m_pDelLogTimer)
   {
      m_pDelLogTimer = new QTimer;
      connect(m_pDelLogTimer, &QTimer::timeout, this, &CRuntimeInstance::slot_rmLogs);
   }

   m_pDelLogTimer->start(1000);
   //slot_rmLogs();
}

void CRuntimeInstance::slot_rmLogs()
{
   if (NULL != m_pDelLogTimer)
   {
      if (m_pDelLogTimer->isActive())
      {
         m_pDelLogTimer->stop();
      }
      m_pDelLogTimer->deleteLater();
      m_pDelLogTimer = NULL;
   }

   QString vhallHelperPath = CPathManager::GetAppDataPath();

   QString strTemp = vhallHelperPath + QString::fromWCharArray(VH_LOG_DIR);
   RemovOverLog(strTemp);

   strTemp = vhallHelperPath + QString("logs");
   RemovOverLog(strTemp);

   strTemp = vhallHelperPath + QString("dshowlog");
   RemovOverLog(strTemp);

   strTemp = vhallHelperPath + QString("txlog");
   RemovOverLog(strTemp);
}

void CRuntimeInstance::RemovOverLog(QString& strPath)
{
   QDir dir(strPath);
   dir.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
   dir.setSorting(QDir::Size | QDir::Reversed);
   QFileInfoList list = dir.entryInfoList();

   QDateTime currentDt = QDateTime::currentDateTime();

   for (int i = 0; i < list.size(); ++i) {
      QFileInfo fileInfo = list.at(i);
      if (fileInfo.lastModified().daysTo(currentDt) > (31))
      {
         QFile::remove(fileInfo.absoluteFilePath());
      }

   }
}

void CRuntimeInstance::UpdateLogIn(int mode, QString streamId) {
   QNetworkProxy proxy;
   proxy.setType(QNetworkProxy::NoProxy);
   QString qsConfPath = CPathManager::GetConfigPath();
   QString strLogReportUrl = ConfigSetting::ReadString(CPathManager::GetToolConfigPath(), GROUP_DEFAULT, KEY_LOGREPORT_URL, defaultLogReportURL);
   QString logIdBase = ConfigSetting::ReadString(qsConfPath, GROUP_DEFAULT, LOG_ID_BASE, "");
   if (logIdBase.isEmpty()) {
      logIdBase = "vhall_helper";// QString::number(QDateTime::currentDateTime().toTime_t());
   }
   QJsonObject body;
   DWORD processId = GetCurrentProcessId();//当前进程id  
   body["aid"] = streamId;
   body["pid"] = QString::number(processId);
   body["pf"] = QString::number(6);
   body["uid"] = logIdBase;
   QString token;
   QString json_data = QJsonDocument(body).toJson();
   QByteArray text(json_data.toUtf8());
   token = QString::fromUtf8(text.toBase64());
   QString session_id = QUuid::createUuid().toString();

   QString httpUrl = strLogReportUrl + QString("?id=%1&key=log_in&k=12003&pf=6&bu=0&mo=%2&version=%3&streamid=%4&aid=%5&s=%6&uid=%7&token=%8")
                  .arg(logIdBase)
                  .arg(mode)
                  .arg(QString::fromStdWString(m_wsCurrentVersion.c_str()))
                  .arg(streamId)
                  .arg(streamId)
                  .arg(session_id)
                  .arg(logIdBase)
                  .arg(token);

   HTTP_GET_REQUEST request(httpUrl.toStdString());
  // request.SetHttpPost(true);
   request.mbIsNeedSyncWork = true;
   GetHttpManagerInstance()->HttpGetRequest(request, [&](const std::string& msg, int code, const std::string userData) {

   });

   /*QNetworkAccessManager  netAccessMgr;
   netAccessMgr.setProxy(proxy);
   QEventLoop loop;
   QObject::connect(&netAccessMgr, SIGNAL(finished(QNetworkReply*)), &loop, SLOT(quit()), Qt::DirectConnection);
   QString urlStr = httpUrl;
   QUrl url = QUrl(urlStr);
   QNetworkRequest req = QNetworkRequest(url);
   req.setRawHeader("HOST", url.host().toUtf8());
   req.setRawHeader("PORT", QString::number(url.port()).toUtf8());
   QNetworkReply *reply = netAccessMgr.get(req);
   QTimer timeOut;
   QObject::connect(&timeOut, SIGNAL(timeout()), &loop, SLOT(quit()));
   timeOut.setSingleShot(true);
   timeOut.start(5000);
   loop.exec();
   if (reply) {
      QString qStrResult = reply->readAll();
      int errCode = reply->error();
      if (!reply->isFinished()) {
         reply->abort();
      }
      delete reply;
   }
   else {
      TRACE6("ExecuteRemoteTask Reply NULL\n");
   }
   QObject::disconnect(&netAccessMgr, SIGNAL(finished(QNetworkReply*)), &loop, SLOT(quit()));*/
   TRACE6("%s end\n", __FUNCTION__);
}
