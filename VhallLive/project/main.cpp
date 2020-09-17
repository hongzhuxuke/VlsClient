#include "stdafx.h"
//#include "vld.h"

#include "RuntimeInstance.h"
#include "PublishInfo.h"
#include "IDeckLinkDevice.h"
#include "IDShowPlugin.h"
#include "charset.h"
#include "ExceptionDump.h"
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDir>
#include <QIcon>
#include "tlhelp32.h"
#include "psapi.h"
#include <ShlObj.h>
#include "pathManage.h"
#include "ConfigSetting.h"
#include "pathmanager.h"
#include <QtNetwork/QNetworkConfiguration> 
#include <QtNetwork/QNetworkConfigurationManager>
#include <comdef.h>
#include <Wbemidl.h>
#include "AMDAdapterCheck.h"
#include "VhallNetWorkInterface.h"
#include <QFontDatabase> 
#include <QLabel>
#include <QDesktopWidget>
#include <QTranslator>
//#include "vld.h"
#include "vhallloginwidget.h"
#pragma comment(lib, "wbemuuid.lib")

bool copyFileToPath(QString sourceDir, QString toDir, bool coverFileIfExist) {
   toDir.replace("/", "\\");
   if (sourceDir == toDir) {
      return true;
   }
   if (QFile::exists(toDir)) {
      return false;
   }

   if (!QFile::copy(sourceDir, toDir))
   {
      return false;
   }
   return true;
}

void InitLogDir() {
   std::wstring logPath = GetAppDataPath() + L"vhlog";
   QDir dir;
   bool res = false;
   if (!dir.exists(QString::fromWCharArray(logPath.c_str()))) {
      res = dir.mkpath(QString::fromWCharArray(logPath.c_str()));
      TRACE6("mkpath vhlog \n");
   }
   QString toDir = QString::fromStdWString(GetAppDataPath());
   copyFileToPath("C:\\WINDOWS\\system32\\ntdll.dll", toDir + "ntdll.dll", false);
   copyFileToPath("C:\\WINDOWS\\system32\\kernel32.dll", toDir + "kernel32.dll", false);
   copyFileToPath("C:\\WINDOWS\\system32\\kernelbase.dll", toDir + "kernelbase.dll", false);
}

LPWSTR GetLastErrorText(DWORD lastErr = GetLastError()) {
   static LPWSTR lastMsg = NULL;
   wstring msg;
   WCHAR buf[128];
   DWORD lastError = lastErr;

   if (lastMsg) {
      free(lastMsg);
      lastMsg = NULL;
   }
   DWORD nRet = FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
      NULL, lastError, /*LANG_USER_DEFAULT*/MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), buf, (sizeof buf) / 2, NULL);
   if (nRet) {
      LPWSTR msgBuf = *((LPWSTR *)buf);
      int msgLen = wcslen(msgBuf);
      if (msgLen >= 1 && (msgBuf[msgLen - 1] == L'\n' || msgBuf[msgLen - 1] == L'\r'))
         msgBuf[msgLen - 1] = L'\0';
      if (msgLen >= 2 && (msgBuf[msgLen - 2] == L'\n' || msgBuf[msgLen - 2] == L'\r'))
         msgBuf[msgLen - 2] = L'\0';
      msg = L"\"";
      msg += msgBuf;
      LocalFree(msgBuf);
      msg += L"\"";
      ZeroMemory(buf, sizeof buf);
      swprintf_s(buf, ARRAYSIZE(buf), L" (0x%08X).", lastError);
      msg += buf;
   }
   else {
      ZeroMemory(buf, sizeof buf);
      swprintf_s(buf, ARRAYSIZE(buf), L"Win32 error code: 0x%08X.", lastError);
      msg = buf;
   }
   lastMsg = _wcsdup(msg.c_str());
   return lastMsg;
}

BOOL InitSystem();
BOOL UnInitSystem();


// TraceLog
void InitDebugTrace(int aiTraceLevel);
void CheckVHLogDir(wstring &path);
bool DShowFileOpen(char *);
bool DSHowFileClear();
bool DShowFileRead(DeviceInfo &deviceInfo, UINT &width, UINT &height, int &frameInternal, DeinterlacingType &type, VideoFormat &format);
bool DShowFileWrite(DeviceInfo deviceInfo, UINT width, UINT height, int frameInternal, DeinterlacingType type, VideoFormat format);
bool DShowFileClose();
void CheckLogDir() {
   QString path = qApp->applicationDirPath() + QString::fromWCharArray(VH_LOG_DIR);
   if (!QDir(path).exists()) {
      QDir().mkdir(path);
   }
   else
   {
      CSingletonRuntimeInstance::Instance().startDelOverdueLog();
   }
}

int main(int argc, char *argv[]) {


   //--------------------------DEBUG----------------------------------
#if 0
   for (int i = 0; i < argc; i++) {
      char title[128] = { 0 };
      char data[256] = { 0 };
      sprintf(title, "%d/%d", i, argc);
      MessageBoxA(NULL, argv[i], title, 0);
   }
#endif

   //#ifdef _DEBUG

#if 0
   char *targv[] = {
      "C:\\Users\\vhall\\Documents\\VhallLive2.0\\VhallLive.exe",
      "undefined",
     "[{\"url\":\"rtmp://rs.live.vhou.net/vhall\",\"alias\":\"自动\"},{\"url\":\"rtmp://bjrs01.live.vhou.net/vhall\",\"alias\":\"华北区\"},{\"url\":\"rtmp://shrs01.live.vhou.net/vhall\",\"alias\":\"华东区\"},{\"url\":\"rtmp://szrs01.live.vhou.net/vhall\",\"alias\":\"华南区\"}]",
     "AC4481CBt2635D",
      "384657318",
      "0",
      "0",
      "0",
      "232522410",
      "host",
      "2",
      "vhall",
      "//mss.e.vhall.com/"
   };
   argc = sizeof(targv) / sizeof(char *);
   argv = targv;
#else 
#if 0
   char *targv[] = {
      /*0*/      "VhallLive",
      /*1*/      "undefined",//userName
      /*2*/      "[{\"alias\":\"自动测试\",\"url\":\"rtmp://rs.live.t.vhou.net/vhall\"},{\"alias\":\"华北区测试\",\"url\":\"rtmp://rs.live.t.vhou.net/vhall\"},{\"alias\":\"华东区测试\",\"url\":\"rtmp://rs.live.t.vhou.net/vhall\"},{\"alias\":\"华南区测试\",\"url\":\"rtmp://rs.live.t.vhou.net/vhall\"}]",//streamUrls
      /*3*/      "134C1048t26335",//stremtoken
      /*4*/      "283758311",//streamName
      /*5*/      "0",//hideLogo
      /*6*/      "0",//bMengzhu
      /*7*/      "0",//channelId
      /*8*/      "232520755",//userid
      /*9*/      "host",//role
      /*10*/      "2",//webinar_type 1普通活动 2切换嘉宾活动
      /*11*/      "vhall",//accesstoken
      /*12*/      "//mss.e.vhall.com/"
   };
   argc = sizeof(targv) / sizeof(char *);
   argv = targv;

#else
#if 0
   argc = 2;
   char *targv[] = {
      "C:\\Users\\vhall\\Documents\\VhallLive2.0\\VhallLive.exe",
       "vhalllive://vhalllive/?stream_name=755760929&stream_token=eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzI1NiJ9.eyJzdHJlYW1uYW1lIjoiNzU1NzYwOTI5In0.6l7r9LQ_euGgGVZ8t9cwE6agl-WTP0m2B8U11xG4n-A&msg_token=eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzI1NiJ9.eyJpZCI6IjI4MjkwNjk0OSIsIm5hbWUiOiJcdTkwYjFcdTViODcxMTExMSIsInJvb20iOiI3NTU3NjA5MjkiLCJyb2xlIjoiaG9zdCIsInVpZCI6IjE4NDYxNjgxIiwidHJhY2tJZCI6IjUwNzkzMDgzOCIsImRldmljZV9zdGF0dXMiOjF9.mCnjljki6vAqLfQe7luUEGP5QE-t4fEL8AjUPV8TsYk&hide_logo=0"
   };
   argv = targv;
#endif

   //#if 1
   //    argc = 3;
   //    char *targv[] = {
   //        "vlive",
   //        "-r",
   //       "restart"
   //    };
   //    argv = targv;
   //#endif

#endif
#endif
//#endif

#ifdef DEBUG
   //
#endif // DEBUG
    //VLDEnable();
   InitLogDir();
   InitDebugTrace(6);
   TRACE6("Application VhallLive  start \n");
   wstring appPath = GetAppPath();
   QString toolConfPath = CPathManager::GetAppDataPath() + QString::fromStdWString(VHALL_TOOL_CONFIG);
   appPath = appPath + L"Qtplugins\\";
   QString pluginsPath = pluginsPath.fromStdWString(appPath);
   TRACE6("addLibraryPath  start \n");
   TRACE6("addLibraryPath ok\n");
   ConfigSetting::InitScreenDPI(CPathManager::GetConfigPath());

   QDesktopWidget * deskTop = QApplication::desktop();
   int dpi_enable = ConfigSetting::ReadInt(CPathManager::GetConfigPath(), GROUP_DEFAULT, DESKTOP_DIP, 1);
   TRACE6("Application VhallLive  start dpi_enable %d\n", dpi_enable);
   if (dpi_enable == 1) {
      qputenv("QT_ENABLE_HIGHDPI_SCALING", "1");
      QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
   }

   _setmaxstdio(2048);
   QApplication a(argc, argv);
   QTranslator translator;
   translator.load(QString(":/qt_zh_cn"));
   a.installTranslator(&translator);
   TRACE6("installTranslator \n");
   //加载qss
   QFile qss(":/Style.qss");
   qss.open(QFile::ReadOnly);
   a.setStyleSheet(qss.readAll());
   qss.close();
   qputenv("QT_BEARER_POLL_TIMEOUT", QByteArray::number(-1));
   qputenv("QTWEBENGINE_REMOTE_DEBUGGING", "9223");
   TRACE6("qputenv \n");
   //解决pepflashplayer.dll在no-sanbox模式下弹窗cmd黑框
   std::wstring wpath = GetAppPath() + L"cmd.exe";
   SetEnvironmentVariableW(L"ComSpec", wpath.c_str());
   TRACE6("SetEnvironmentVariableW \n");
   TRACE6("********************************  Application VhallLive Start. ************************************1\n");
   TRACE6("GetHttpManagerInstance Init \n");
   std::wstring logPath = GetAppDataPath() + L"vhlog";
   GetHttpManagerInstance()->Init(logPath);
   int is_http_proxy = ConfigSetting::ReadInt(CPathManager::GetConfigPath(), GROUP_DEFAULT, PROXY_OPEN, 0);
   QString proxy_ip = ConfigSetting::ReadString(CPathManager::GetConfigPath(), GROUP_DEFAULT, PROXY_HOST, "");
   int proxy_port = ConfigSetting::ReadInt(CPathManager::GetConfigPath(), GROUP_DEFAULT, PROXY_PORT, 80);
   QString user = ConfigSetting::ReadString(CPathManager::GetConfigPath(), GROUP_DEFAULT, PROXY_USERNAME, "");
   QString pwd = ConfigSetting::ReadString(CPathManager::GetConfigPath(), GROUP_DEFAULT, PROXY_PASSWORD, "");
   if (is_http_proxy == 1 && !proxy_ip.isEmpty()) {
      GetHttpManagerInstance()->SwitchHttpProxy(true, proxy_ip.toStdString(), proxy_port, user.toStdString(), pwd.toStdString());
   }
   else {
      GetHttpManagerInstance()->SwitchHttpProxy(false);
   }

   TRACE6("GetHttpManagerInstance Init ok\n");
   CSingletonRuntimeInstance::Instance().InitCurVersion();
   //崩溃日志捕获
   CExceptionDump g_oDmpFile;

   // 初始化系统
   if (!InitSystem()) {
      TRACE6("Application VhallLive InitSystem Failed!. \n");
      GetHttpManagerInstance()->Release();
      return 0;
   }


   TRACE6("Application CSingletonRuntimeInstance::Instance().CheckParam\n");
   while (CRE_OK != CSingletonRuntimeInstance::Instance().CheckParam(argc, argv)) {
      bool bRestart = CSingletonRuntimeInstance::Instance().GetISRestart();
      if (!bRestart) {
         TRACE6("Application CSingletonRuntimeInstance::Instance().CheckParam Failed!\n");
         GetHttpManagerInstance()->Release();
         return 0;
      }
      TRACE6("Application CSingletonRuntimeInstance::Instance().CheckParam Again\n");
   }
   CheckLogDir();
   TRACE6("Application CSingletonRuntimeInstance::Instance().Create\n");

   // 初始实例
   if (CRE_OK != CSingletonRuntimeInstance::Instance().Create()) {
      TRACE6("CSingletonRuntimeInstance::Instance().Create err \n");
      GetHttpManagerInstance()->Release();
      ASSERT(FALSE);
      return 0;
   }

   bool bExit = false;
   // 启动
   if (CRE_OK != CSingletonRuntimeInstance::Instance().StartSystem(&bExit)) {
      TRACE6("CSingletonRuntimeInstance::Instance().StarSystem 失败 \n");
      GetHttpManagerInstance()->Release();
      ASSERT(FALSE);
      return 0;
   }
   TRACE6("Application a.exec\n");
   while (!bExit) {
      a.exec();
   }
   GetHttpManagerInstance()->Release();
   DestoryHttpManagerInstance();
   UnInitSystem();
   TRACE6("******************************** Application VhallLive Quit. ************************************\n");
   return 0;
}

//初始化系统
BOOL InitSystem() {
   //初始化COM
   TRACE6("InitSystem \n");
   CoInitialize(0);

   InitDeckLinkDeviceManager();
   TRACE6("InitDeckLinkDeviceManager \n");
   InitDShowPlugin(DShowFileOpen, DSHowFileClear, DShowFileRead, DShowFileWrite, DShowFileClose);
   TRACE6("InitDShowPlugin \n");
   //初始化套接字
   WSADATA wsaData;
   int retVal;
   retVal = WSAStartup(MAKEWORD(2, 2), &wsaData);
   if (retVal != 0) {
      TRACE6("wmain WSAStartup() failed %ws\n", GetLastErrorText(GetLastError()));
      return FALSE;
   }
   TRACE6("InitSystem ok \n");
   return TRUE;
}

//反初始化系统
BOOL UnInitSystem() {
   UnInitDShowPlugin();
   UnInitDeckLinkDeviceManager();
   CoUninitialize();
   return TRUE;
}

void InitDebugTrace(int aiTraceLevel) {
   wchar_t lwzDllPath[MAX_PATH + 1] = { 0 };
   SHGetFolderPathW(NULL, CSIDL_MYDOCUMENTS, NULL, SHGFP_TYPE_CURRENT, lwzDllPath);
   wcscat_s(lwzDllPath, _MAX_PATH, L"\\VHallHelper");

   //首先设置日志打印选项
   CDebugTrace::SetTraceLevel(aiTraceLevel);
   CDebugTrace::SetTraceOptions(CDebugTrace::GetTraceOptions() \
      | CDebugTrace::Timestamp & ~CDebugTrace::LogLevel \
      & ~CDebugTrace::FileAndLine | CDebugTrace::AppendToFile\
      | CDebugTrace::PrintToConsole);

   //生成TRACE文件名
   SYSTEMTIME loSystemTime;
   GetLocalTime(&loSystemTime);

   wchar_t lwzLogFileName[255] = { 0 };
   wsprintf(lwzLogFileName, L"%s%sVhallLive_%4d_%02d_%02d_%02d_%02d%s",
      lwzDllPath, VH_LOG_DIR,
      loSystemTime.wYear,
      loSystemTime.wMonth,
      loSystemTime.wDay,
      loSystemTime.wHour,
      loSystemTime.wMinute, L".log");

   CDebugTrace::SetLogFileName(lwzLogFileName);
}

QFile G_dshowFile;
QJsonArray G_dshowArray;
int G_dshowIndex = 0;
bool DShowFileOpen(char *filename) {
   G_dshowFile.setFileName(QDir::tempPath() + filename);
   G_dshowIndex = 0;
   bool ret = G_dshowFile.open(QIODevice::ReadWrite);
   if (ret) {
      QByteArray ba = G_dshowFile.readAll();
      QJsonDocument doc = QJsonDocument::fromJson(ba);
      G_dshowArray = doc.array();
   }
   return ret;
}
bool DSHowFileClear() {
   G_dshowIndex = 0;
   return G_dshowFile.resize(0);
}

bool DShowSerialize(QJsonObject &obj, DeviceInfo deviceInfo, UINT width, UINT height, int frameInternal, DeinterlacingType type, VideoFormat format) {
   QString disPlayName = QString::fromWCharArray(deviceInfo.m_sDeviceDisPlayName);
   QString name = QString::fromWCharArray(deviceInfo.m_sDeviceName);
   QString id = QString::fromWCharArray(deviceInfo.m_sDeviceID);
   int deviceType = (int)deviceInfo.m_sDeviceType;

   obj["disPlayName"] = disPlayName;
   obj["name"] = name;
   obj["id"] = id;
   obj["deviceType"] = (int)deviceType;
   obj["width"] = (int)width;
   obj["height"] = (int)height;
   obj["frameInternal"] = (int)frameInternal;
   obj["type"] = (int)type;
   obj["format"] = (int)format;

   return true;
}

bool DShowDeSerialize(QJsonObject obj, DeviceInfo &deviceInfo,
   UINT &width, UINT &height, int &frameInternal,
   DeinterlacingType &type, VideoFormat &format) {
   QString disPlayName;
   QString name;
   QString id;
   int deviceType;


   disPlayName = obj["disPlayName"].toString();
   name = obj["name"].toString();
   id = obj["id"].toString();

   disPlayName.toWCharArray(deviceInfo.m_sDeviceDisPlayName);
   name.toWCharArray(deviceInfo.m_sDeviceName);
   id.toWCharArray(deviceInfo.m_sDeviceID);

   deviceType = obj["deviceType"].toInt();
   deviceInfo.m_sDeviceType = (TYPE_RECODING_SOURCE)deviceType;

   width = obj["width"].toInt();
   height = obj["height"].toInt();
   frameInternal = obj["frameInternal"].toInt();
   type = (DeinterlacingType)obj["type"].toInt();
   format = (VideoFormat)obj["format"].toInt();
   return true;
}

bool DShowFileRead(DeviceInfo &deviceInfo, UINT &width, UINT &height, int &frameInternal, DeinterlacingType &type, VideoFormat &format) {
   if (G_dshowIndex < G_dshowArray.count()) {
      QJsonObject obj = G_dshowArray[G_dshowIndex].toObject();
      DShowDeSerialize(obj, deviceInfo, width, height, frameInternal, type, format);
      G_dshowIndex++;
   }
   else {
      return false;
   }
   return true;
}

bool DShowFileWrite(DeviceInfo deviceInfo, UINT width, UINT height, int frameInternal, DeinterlacingType type, VideoFormat format) {
   if (G_dshowIndex >= 0) {
      while (G_dshowArray.count()) {
         G_dshowArray.removeLast();
      }
      G_dshowIndex = -1;
   }

   QJsonObject obj;
   DShowSerialize(obj, deviceInfo, width, height, frameInternal, type, format);
   G_dshowArray.append(obj);

   return true;
}

bool DShowFileClose() {
   G_dshowFile.resize(0);
   G_dshowIndex = 0;

   QJsonDocument doc;
   doc.setArray(G_dshowArray);
   QByteArray ba = doc.toJson();
   G_dshowFile.write(ba);
   G_dshowFile.close();
   return true;
}

void CheckVHLogDir(wstring &path) {
   QDir dir(QString::fromStdWString(path));
   if (dir.exists()) {
      TRACE6("%s %ws exists\n", __FUNCTION__, path.c_str());
   }
   else {
      TRACE6("%s %ws not exists\n", __FUNCTION__, path.c_str());
   }
}

