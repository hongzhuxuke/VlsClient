// VhallService.cpp : 定义控制台应用程序的入口点。
//

#include <stdio.h>
#include <tchar.h>
#include "SocketServer.h"
//#include "ClientInfo.h"
#include "common/baseclass/pathManage.h"
#include "common/Logging.h"

#include "ServiceMain.h"
#include "VhallService.h"
#include "common/Defs.h"
#include "Conn.h"

Logger *g_pLogger = NULL;
HANDLE  gAutoServerThread = NULL;
SocketServer* gLocalServer = NULL;
int    gLocalServerPort = SERVER_PORT;
//HANDLE gLocalEvent = NULL;

HANDLE gProcessThreadHandle = NULL;
bool gProcessRunning = false;

std::string gUserName;
std::string gRtmpAddress;
std::string gToken;
std::string gStreamName;

std::string gUserId;
std::string gRole;
std::string gWebinarType;
std::string gAccesstoken;
std::string gScheduler;

bool gIsHideLogo;

LPWSTR GetLastErrorText();

int _tmain(int argc, _TCHAR* argv[]) {
   //hide the console window
	g_pLogger = new Logger(L"VhallService.log", SYSTEM);
   ShowWindow(GetConsoleWindow(), SW_SHOWNORMAL);
   WSADATA wsaData;
   int retVal;
   retVal = WSAStartup(MAKEWORD(2, 2), &wsaData);
   if (retVal != 0) {
      g_pLogger->logError(L"wmain WSAStartup() failed %s\n",
                        GetLastErrorText());
      return false;
   }
   
   //for test
   // StartVhallClient("lixiongbing", "rtmp://192.168.1.7/vhall", "xdfsfsfsdf-fsdf=fds", "test02");
   //getchar();
   //StopVhallClient();
   if (argc > 1) {
      if (wcscmp(argv[1], L"-i") == 0) {
         InstallCmdService(SERVICE_NAME);
      } else if (wcscmp(argv[1], L"-d") == 0) {
         RemoveCmdService(SERVICE_NAME);
      } else {
         printf("\nUnknown Switch Usage\nFor Install use Servicetest -i\nFor UnInstall use Servicetest -d\n");
         /* gLocalServer = new SocketServer(gLocalServerPort);
          gLocalServer->StartServer();
          getchar();*/
      }
   } else {
      SERVICE_TABLE_ENTRY DispatchTable[] = { { L"Vhall Service", ServiceMain }, { NULL, NULL } };
      StartServiceCtrlDispatcher(DispatchTable);
   }
   /* gLocalServer = new SocketServer(gLocalServerPort);
    gLocalServer->StartServer();
    getchar();*/
   WSACleanup();
   return 0;
}

bool stringToWString(const std::string& narrowStr, std::wstring& wStr) {
   size_t wcsLen;
   DWORD errorCode = ERROR_SUCCESS;
   std::string errorMsg;

   if (narrowStr.length() == 0) {
      wStr.clear();
   }
   wcsLen = MultiByteToWideChar(CP_UTF8, 0, narrowStr.c_str(), -1, NULL, 0);
   if (wcsLen == 0) {
      errorCode = GetLastError();
      g_pLogger->logError("stringToWString: Error getting buffer size, error = %0x08x", errorCode);
      return false;
   }

   WCHAR *wcsStr = new WCHAR[wcsLen + 1];
   wcsLen = MultiByteToWideChar(CP_UTF8, 0, narrowStr.c_str(), -1, wcsStr, (int)wcsLen);
   if (wcsLen == 0) {
      errorCode = GetLastError();
      g_pLogger->logError("stringToWString: MultiByteToWideChar failed, error = %0x08x.", errorCode);
      return false;
   }
   wcsStr[wcsLen] = L'\0';
   wStr = wcsStr;
   delete[] wcsStr;
   return true;
}

#include <tlhelp32.h>
#include <Wtsapi32.h>
#include <Userenv.h>
#pragma comment (lib, "Userenv.lib")
#pragma comment (lib, "Wtsapi32.lib")

bool StartProcessAndBypassUAC(LPWSTR  commandLine, PROCESS_INFORMATION &procInfo) {
   g_pLogger->logInfo(L"==============[StartProcessAndBypassUAC]=================\n");
   
   HANDLE hPToken = NULL;
   HANDLE hUserTokenDup = NULL;
   SECURITY_ATTRIBUTES sa;
   LPVOID EnvironmentFromUser = NULL;
   // obtain the currently active session id; every logged on user in the system has a unique session id
   DWORD dwSessionId = WTSGetActiveConsoleSessionId();


   ZeroMemory(&procInfo, sizeof(PROCESS_INFORMATION));
   ZeroMemory(&sa, sizeof(SECURITY_ATTRIBUTES));
   if (!WTSQueryUserToken(dwSessionId, &hPToken)) {
      g_pLogger->logInfo(L"[StartProcessAndBypassUAC]WTSQueryUserToken Failed!\n");
      return false;
   }

   // copy the access token of the dwSessionId‘s User; the newly created token will be a primary token   SecurityIdentification
   if (!DuplicateTokenEx(hPToken, MAXIMUM_ALLOWED, &sa, SecurityIdentification, TokenPrimary, &hUserTokenDup)) {
      g_pLogger->logError(L"[StartProcessAndBypassUAC]  DuplicateTokenEx"
                        L"error: %s\n", GetLastErrorText());
      CloseHandle(hPToken);
      return false;
   }

   if (!CreateEnvironmentBlock(&EnvironmentFromUser, hUserTokenDup, false)) {
      g_pLogger->logError(L"[StartProcessAndBypassUAC]  CreateEnvironmentBlock"
                        L"error: %s\n", GetLastErrorText());
      CloseHandle(hPToken);
      CloseHandle(hUserTokenDup);
      return false;
   }

   // By default CreateProcessAsUser creates a process on a non-interactive window station, meaning
   // the window station has a desktop that is invisible and the process is incapable of receiving
   // user input. To remedy this we set the lpDesktop parameter to indicate we want to enable user 
   // interaction with the new process.
   STARTUPINFO startupInfo;

   //
   ZeroMemory(&startupInfo, sizeof(STARTUPINFO));
   startupInfo.lpDesktop = L"winsta0\\default";

   // flags that specify the priority and creation method of the process
   int dwCreationFlags = NORMAL_PRIORITY_CLASS | CREATE_NEW_CONSOLE | CREATE_UNICODE_ENVIRONMENT;
   g_pLogger->logInfo(L"[StartProcessAndBypassUAC] CreateProcessAsUserW\n");

   // create a new process in the current user‘s logon session
   BOOL result = CreateProcessAsUserW(hUserTokenDup,        // client‘s access token
                                      NULL,                   // file to execute
                                      commandLine,            // command line
                                      &sa,                 // pointer to process SECURITY_ATTRIBUTES
                                      &sa,                 // pointer to thread SECURITY_ATTRIBUTES
                                      FALSE,                  // handles are not inheritable
                                      dwCreationFlags,        // creation flags
                                      EnvironmentFromUser,    // pointer to new environment block 
                                      NULL,                   // name of current directory 
                                      &startupInfo,                 // pointer to STARTUPINFO structure
                                      &procInfo            // receives information about new process
                                      );

   // invalidate the handles
   CloseHandle(hPToken);
   CloseHandle(hUserTokenDup);
   DestroyEnvironmentBlock(EnvironmentFromUser);

   g_pLogger->logInfo(L"[StartProcessAndBypassUAC] CreateProcessAsUserW End ,result = %d\n",result?1:0);
   
   return result==TRUE; // return the result
}

DWORD __stdcall openProcessThreadProc(void *pParam) {
   //create process
   bool ret = false;
   DWORD exitCode = 1;
   STARTUPINFOW si = { 0 };
   std::wstring cmdLine;
   LPWSTR cmdBuf;
   si.cb = sizeof(STARTUPINFOW);
   si.dwFlags = STARTF_USESHOWWINDOW;
   si.wShowWindow = SW_SHOW;
   PROCESS_INFORMATION procInfo;

   std::wstring userName;
   std::wstring rtmpAddress;
   std::wstring token;
   std::wstring streamName;

   std::wstring userId;
   std::wstring role;
   std::wstring webinarType;
   std::wstring accesstoken;
   std::wstring scheduler;
   
   stringToWString(gUserName, userName);
   stringToWString(gRtmpAddress, rtmpAddress);
   stringToWString(gToken, token);
   stringToWString(gStreamName, streamName);

   stringToWString(gUserId, userId);
   stringToWString(gRole, role);
   stringToWString(gWebinarType, webinarType);
   stringToWString(gAccesstoken, accesstoken);
   stringToWString(gScheduler, scheduler);   
   
   ReplaceString(rtmpAddress, L"\"", L"\\\"");

   if(userId == L""){
      userId=L"NULL";
   }

   if(role == L""){
      role=L"NULL";
   }

   if(webinarType == L"") {
      webinarType=L"NULL";
   }

   if(accesstoken == L"") {
      accesstoken=L"NULL";
   }

   if(scheduler == L"") {
      scheduler=L"NULL";
   }

   g_pLogger->logInfo(L"==============[openProcessThreadProc]=================\n");
   g_pLogger->logInfo(L"0  %s=[%s]",L"PATH",GetAppPath().c_str(), L"\\VhallLive.exe ");
   g_pLogger->logInfo(L"1  %s=[%s]",L"userName",userName.c_str());
   g_pLogger->logInfo(L"2  %s=[%s]",L"rtmpAddress",rtmpAddress.c_str());
   g_pLogger->logInfo(L"3  %s=[%s]",L"token",token.c_str());
   
   g_pLogger->logInfo(L"4  %s=[%s]",L"streamName",streamName.c_str());
   g_pLogger->logInfo(L"5  %s=[%s]",L"gIsHideLogo",(gIsHideLogo ? L"1" : L"0"));
   //g_pLogger->logInfo(L"6  %s=[%s]",L"",L"0");
   //g_pLogger->logInfo(L"7  %ws=[%ws]",L"",L"0");
   g_pLogger->logInfo(L"8  %s=[%s]",L"userId",userId.c_str());
   g_pLogger->logInfo(L"9  %s=[%s]",L"role",role.c_str());
   g_pLogger->logInfo(L"10 %s=[%s]",L"webinarType",webinarType.c_str());
   g_pLogger->logInfo(L"11 %s=[%s]",L"accesstoken",accesstoken.c_str());
   g_pLogger->logInfo(L"12 %s=[%s]",L"scheduler",scheduler.c_str());
   g_pLogger->logInfo(L"=====================================================\n");
   
   cmdLine = GetAppPath() 
   /* 0*/   + L"\\VhallLive.exe " 
   /* 1*/   + userName 
   /* 2*/   + L" \"" + rtmpAddress 
   /* 3*/   + L"\" " + token 
   /* 4*/   + L" " + streamName 
   /* 5*/   + L" " + (gIsHideLogo ? L"1" : L"0") 
   /* 6*/   + L" " + L"0"
   /* 7*/   + L" " + L"0"
   /* 8*/   + L" " + userId
   /* 9*/   + L" " + role
   /* 10*/  + L" " + webinarType
   /* 11*/  + L" " + accesstoken
   /* 12*/  + L" " + scheduler;

   //cmdLine = ""
   cmdBuf = _wcsdup(cmdLine.c_str());
   g_pLogger->logInfo(L"[openProcessThreadProc] StartProcessAndBypassUAC cmdBuf=%s", cmdBuf);

   ret = StartProcessAndBypassUAC(cmdBuf, procInfo);
   if (ret == false) {
      g_pLogger->logError(L"[openProcessThreadProc] StartProcessAndBypassUAC Error: %s.\n",
                        GetLastErrorText());
      return false;

   }
   else {
      if (WaitForSingleObject(procInfo.hProcess, INFINITE) != WAIT_OBJECT_0) {
         g_pLogger->logError(L"[openProcessThreadProc] WaitForSingleObject on process failed, "
                           L"error: %s\n", GetLastErrorText());
      }
      if (!GetExitCodeProcess(procInfo.hProcess, &exitCode)) {
         g_pLogger->logError(L"[openProcessThreadProc] GetExitCodeProcess failed, error: %s\n",
                           GetLastErrorText());
      }
      CloseHandle(procInfo.hProcess);
   }
   
   free(cmdBuf);
   
   g_pLogger->logInfo(L"==============[openProcessThreadProc] return================\n");
   return 0;
}

bool StartVhallClient(const char *uname, 
   const char *address, 
   const char *token,
   const char *streamName, 
   bool isHideLogo,

   const char *userId,
   const char *role,
   const char *webninartype,
   const char *accesstoken,
   const char *scheduler
   ) 

{
   g_pLogger->logInfo("uname = %s, address = %s, token = %s, streamName = %s, hide logo =%d", uname, address, token, streamName, isHideLogo ? 1 : 0);
   gUserName = uname;
   gRtmpAddress = address;
   gToken = token;
   gStreamName = streamName;
   gIsHideLogo = isHideLogo;

   gUserId = userId;
   gRole = role;
   gWebinarType = webninartype;
   gAccesstoken = accesstoken;
   gScheduler = scheduler;

   /*if (StopVhallClient() == false) {
      return false;
      }*/
   // gLocalEvent = CreateEventW(NULL, FALSE, FALSE, LOCAL_EVENT_NAME);     //process will wait this event
   // ResetEvent(gLocalEvent);

   gProcessThreadHandle = CreateThread(0, 0, openProcessThreadProc, NULL,
                                       NULL, NULL);
   //accept client, and create thread.
   if (gProcessThreadHandle == NULL) {
      g_pLogger->logError(L"ExecVhallClient  _beginthread openProcessThreadProc failed  %s",
                        GetLastErrorText());
      return false;
   }
   return true;
}

bool StopVhallClient() {
   
   g_pLogger->logInfo(L"==============[StopVhallClient]================\n");

   /* if (gLocalEvent) {
       SetEvent(gLocalEvent);
       CloseHandle(gLocalEvent);
       }*/
   Conn* helperConn = SocketServer::mConnManager->GetConn(CONN_HLPER_CLIENT, NULL);
   if (helperConn){
      g_pLogger->logInfo(L"[StopVhallClient] GetConn And Will ShutDown\n");
      helperConn->ShutDown();
   }
   else{
      g_pLogger->logInfo(L"[StopVhallClient] Cannot GetConn\n");
   }

   while (gProcessThreadHandle && gProcessRunning) {
      DWORD waitResult = WaitForSingleObject(gProcessThreadHandle, 10000);
      if (waitResult == WAIT_OBJECT_0) {
         g_pLogger->logInfo(L"[StopVhallClient] WaitForSingleObject gProcessThreadHandle WAIT_OBJECT_0\n");
      } else if (waitResult == WAIT_TIMEOUT) {
         g_pLogger->logInfo(L"[StopVhallClient] WaitForSingleObject gProcessThreadHandle WAIT_TIMEOUT\n");
         return false;
      } else {
         g_pLogger->logInfo(L"[StopVhallClient] WaitForSingleObject gProcessThreadHandle Failed (%X)\n",waitResult);
         return false;
      }
      CloseHandle(gProcessThreadHandle);
      gProcessThreadHandle = NULL;
   }
   
   g_pLogger->logInfo(L"==============[StopVhallClient] return ================\n");
   return true;
}

std::string srs_string_replace(std::string str, std::string old_str, std::string new_str)
{
   std::string ret = str;

   if (old_str == new_str) {
      return ret;
   }

   size_t pos = 0;
   while ((pos = ret.find(old_str, pos)) != std::string::npos) {
      ret = ret.replace(pos, old_str.length(), new_str);
   }

   return ret;
}

void ReplaceString(std::wstring& str, const std::wstring& src, const std::wstring& des)
{
   int pos = -1;
   int curPos = 0;
   while (-1 != (pos = str.find(src, curPos)))
   {
      str.replace(pos, src.size(), des);
      curPos = pos + des.size();
   }
}
