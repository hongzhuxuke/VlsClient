/*
   implement windows service
   */
#include <stdio.h>
#include "SocketServer.h"
#include "common/Logging.h"
#include "ServiceMain.h"
#include "VhallService.h"
#include <string>
#include <wchar.h>
using namespace std;


static SERVICE_STATUS        ServiceStatus;
static SERVICE_STATUS_HANDLE ServiceStatusHandle;

void WINAPI ServiceMain(DWORD dwArgc, LPTSTR *lpArgv);
void WINAPI ServiceCtrlHandler(DWORD dwCode);

LPWSTR GetLastErrorText() {
   static LPWSTR lastMsg = NULL;
   wstring msg;
   WCHAR buf[128];
   DWORD lastError = GetLastError();
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
   } else {
      ZeroMemory(buf, sizeof buf);
      swprintf_s(buf, ARRAYSIZE(buf), L"Win32 error code: 0x%08X.", lastError);
      msg = buf;
   }
   lastMsg = _wcsdup(msg.c_str());
   return lastMsg;
}

////////////////////////////////////////
//service handle
void WINAPI ServiceMain(DWORD dwArgc, LPTSTR *lpArgv) {
   HWINSTA hwinsta = OpenWindowStation(L"winsta0", FALSE,
                                       WINSTA_ACCESSCLIPBOARD |
                                       WINSTA_ACCESSGLOBALATOMS |
                                       WINSTA_CREATEDESKTOP |
                                       WINSTA_ENUMDESKTOPS |
                                       WINSTA_ENUMERATE |
                                       WINSTA_EXITWINDOWS |
                                       WINSTA_READATTRIBUTES |
                                       WINSTA_READSCREEN |
                                       WINSTA_WRITEATTRIBUTES);
   SetProcessWindowStation(hwinsta);


   bool ret = false;
   g_pLogger->logInfo("=============[ServiceMain]================\n");
   ServiceStatus.dwServiceType = SERVICE_WIN32;
   ServiceStatus.dwCurrentState = SERVICE_START_PENDING;
   ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP
      | SERVICE_ACCEPT_PAUSE_CONTINUE;
   ServiceStatus.dwServiceSpecificExitCode = 0;
   ServiceStatus.dwWin32ExitCode = 0;
   ServiceStatus.dwCheckPoint = 0;
   ServiceStatus.dwWaitHint = 0;

   ServiceStatusHandle = RegisterServiceCtrlHandler(L"VhallService", ServiceCtrlHandler);
   if (ServiceStatusHandle == 0) {
      g_pLogger->logError("[ServiceMain] RegisterServiceCtrlHandler Error !\n");
      ::MessageBoxW(NULL, L"[ServiceMain] RegisterServiceCtrlHandler error", SERVICE_NAME, MB_OK);
      return;
   }
   
   g_pLogger->logInfo("[ServiceMain] Will Start NetWork!\n");
   //start the work   
   gLocalServer = new SocketServer(gLocalServerPort);
   ret = gLocalServer->StartServer();
   if (ret == false) {
      ServiceStatus.dwCurrentState = SERVICE_STOPPED;      
      g_pLogger->logInfo("[ServiceMain] gLocalServer->StartServer Failed!\n");
   }
   else {
      ServiceStatus.dwCurrentState = SERVICE_RUNNING;      
      g_pLogger->logInfo("[ServiceMain] gLocalServer->StartServer Successed!\n");
   }

   ServiceStatus.dwCurrentState = SERVICE_RUNNING;
   ServiceStatus.dwCheckPoint = 0;
   ServiceStatus.dwWaitHint = 0;

   if (SetServiceStatus(ServiceStatusHandle, &ServiceStatus) == 0) {
      g_pLogger->logError("[ServiceMain] SetServiceStatus in CmdStart Error !\n");
      ::MessageBoxW(NULL, L"[ServiceMain] SetServiceStatus in CmdStart Error !", SERVICE_NAME, MB_OK);
      return;
   }

   
   g_pLogger->logInfo("=============[ServiceMain] return ================\n");
   return;
}

//service request handler
void WINAPI ServiceCtrlHandler(DWORD dwCode) {
   g_pLogger->logInfo("=========[ServiceCtrlHandler]========== .... %d", dwCode);
   switch (dwCode) {
   case SERVICE_CONTROL_PAUSE:
      ServiceStatus.dwCurrentState = SERVICE_PAUSED;
      break;
   case SERVICE_CONTROL_CONTINUE:
      ServiceStatus.dwCurrentState = SERVICE_RUNNING;
      break;
   case SERVICE_CONTROL_STOP:
      gLocalServer->ShutDown();
      ServiceStatus.dwCurrentState = SERVICE_STOPPED;
      ServiceStatus.dwWin32ExitCode = 0;
      ServiceStatus.dwCheckPoint = 0;
      ServiceStatus.dwWaitHint = 0;
      if (SetServiceStatus(ServiceStatusHandle, &ServiceStatus) == 0) {
         g_pLogger->logError("[ServiceCtrlHandler] SetServiceStatus in CmdControl in Switch Error !\n");
         ::MessageBoxW(NULL, L"[ServiceCtrlHandler] SetServiceStatus in CmdControl in Switch Error !", SERVICE_NAME, MB_OK);
      }
      return;
   case SERVICE_CONTROL_INTERROGATE:
      break;
   default:
      break;
   }
   if (SetServiceStatus(ServiceStatusHandle, &ServiceStatus) == 0) {
      g_pLogger->logError("[ServiceCtrlHandler] SetServiceStatus in CmdControl out Switch Error !\n");
   }

   return;
}
////////////////////////////////////////

////////////////////////////////////////
// install/uninstall service
//use in in install packet and unistall packet
void InstallCmdService(WCHAR* serviceFileName) {
   SC_HANDLE        schSCManager;
   SC_HANDLE        schService;
   SERVICE_STATUS   InstallServiceStatus;
   DWORD            dwErrorCode;
   HKEY             key;
   WCHAR            svExeFile[MAX_PATH];
   WCHAR            szPath[512];

   g_pLogger->logInfo("=============[ServiceCtrlHandler] ================\n");
   if (GetModuleFileNameW(NULL, szPath, 512) == 0) {
      g_pLogger->logError(L"[ServiceCtrlHandler] Unable to install %s - %s\n", SERVICE_NAME,
                        GetLastErrorText());
      return;
   }
   schSCManager = OpenSCManagerW(NULL, NULL,
                                 SC_MANAGER_CONNECT | SC_MANAGER_CREATE_SERVICE);
   if (schSCManager == NULL) {
      g_pLogger->logError(L"[ServiceCtrlHandler] schSCManager failed %s - %s\n", SERVICE_NAME,
                        GetLastErrorText());
      return;
   }
   g_pLogger->logInfo("[ServiceCtrlHandler] Creating Service .... ");
   schService = CreateService(schSCManager,
                              SERVICE_NAME,
                              SERVICE_NAME,
                              SERVICE_ALL_ACCESS,
                              SERVICE_WIN32_OWN_PROCESS | SERVICE_INTERACTIVE_PROCESS,
                              SERVICE_AUTO_START,
                              SERVICE_ERROR_IGNORE,
                              szPath, NULL, NULL, NULL, NULL, NULL);


   if (schService == NULL) {
      dwErrorCode = GetLastError();
      if (dwErrorCode != ERROR_SERVICE_EXISTS) {
         g_pLogger->logError(L"[ServiceCtrlHandler] CreateService failed %s - %s\n", SERVICE_NAME,
                           GetLastErrorText());
         CloseServiceHandle(schSCManager);
         return;
      } else {
         g_pLogger->logInfo(L"[ServiceCtrlHandler] CreateService  service %s exists...", SERVICE_NAME);
         schService = OpenService(schSCManager, SERVICE_NAME, SERVICE_START);
         if (schService == NULL) {
            g_pLogger->logError(L"[ServiceCtrlHandler] OpenService failed %s - %s\n", SERVICE_NAME,
                              GetLastErrorText());
            CloseServiceHandle(schSCManager);
            return;
         }
      }
   } 
   else {
      g_pLogger->logInfo("[ServiceCtrlHandler] CreateService Success !\n");
      lstrcpy(svExeFile, L"SYSTEM\\CurrentControlSet\\Services\\");
      lstrcat(svExeFile, L"DeviceServe");
      if (RegOpenKey(HKEY_LOCAL_MACHINE, svExeFile, &key) == ERROR_SUCCESS) {
         RegSetValueEx(key, L"Description", 0, REG_SZ, (BYTE *)SERVICE_DES, lstrlen(SERVICE_DES));
         RegCloseKey(key);
      }
   }
   //start service
   g_pLogger->logInfo("[ServiceCtrlHandler] Starting Service .... ");
   if (StartService(schService, 0, NULL) == 0) {
      dwErrorCode = GetLastError();
      if (dwErrorCode == ERROR_SERVICE_ALREADY_RUNNING) {
         g_pLogger->logInfo("[ServiceCtrlHandler] already Running !\n");
         CloseServiceHandle(schSCManager);
         CloseServiceHandle(schService);
         return;
      }
   }
   else {
      g_pLogger->logInfo("[ServiceCtrlHandler] Pending ... ");
   }

   while (QueryServiceStatus(schService, &InstallServiceStatus) != 0) {
      if (InstallServiceStatus.dwCurrentState == SERVICE_START_PENDING) {
         Sleep(100);
      } 
      else {
         break;
      }
   }
   if (InstallServiceStatus.dwCurrentState != SERVICE_RUNNING) {
      g_pLogger->logError("[ServiceCtrlHandler] InstallCmdServiceFailure !\n");
   } else {
      g_pLogger->logInfo("[ServiceCtrlHandler] InstallCmdService Success !\n");
   }
   CloseServiceHandle(schService);
   CloseServiceHandle(schSCManager);


   g_pLogger->logInfo("[ServiceCtrlHandler] return !\n");
   return;
}

void RemoveCmdService(WCHAR* serviceFileName) {
   SC_HANDLE        schSCManager;
   SC_HANDLE        schService;
   SERVICE_STATUS   RemoveServiceStatus;
   
   g_pLogger->logInfo("=============[RemoveCmdService] ================\n");
   schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
   if (schSCManager == NULL) {
      g_pLogger->logError(L"[RemoveCmdService] schSCManager failed %s - %s\n", SERVICE_NAME,
                        GetLastErrorText());
      return;
   }

   schService = OpenService(schSCManager, serviceFileName, SERVICE_ALL_ACCESS);
   if (schService == NULL) {
      g_pLogger->logError(L"[RemoveCmdService] OpenService failed %s - %s\n", SERVICE_NAME,
                        GetLastErrorText());
      CloseServiceHandle(schSCManager);
   } 
   else {
      g_pLogger->logInfo("[RemoveCmdService] Stopping Service .... ");
      if (QueryServiceStatus(schService, &RemoveServiceStatus) != 0) {
         if (RemoveServiceStatus.dwCurrentState == SERVICE_STOPPED) {
            g_pLogger->logInfo("[RemoveCmdService] RemoveCmdService already Stopped !");
         } else {
            g_pLogger->logInfo("[RemoveCmdService] RemoveCmdService Pending ... ");
            if (ControlService(schService, SERVICE_CONTROL_STOP, &RemoveServiceStatus) != 0) {
               while (RemoveServiceStatus.dwCurrentState == SERVICE_STOP_PENDING) {
                  Sleep(10);
                  QueryServiceStatus(schService, &RemoveServiceStatus);
               }
               
               if (RemoveServiceStatus.dwCurrentState == SERVICE_STOPPED) {
                  g_pLogger->logInfo("[RemoveCmdService] Success !");
               }
               else {
                  g_pLogger->logInfo("[RemoveCmdService] Failure !");
               }
            } 
            else {
               g_pLogger->logInfo("[RemoveCmdService] Failure !");
            }
         }
      }
      else {
         g_pLogger->logWarning("[RemoveCmdService] Query Failure !");
      }

      g_pLogger->logInfo("[RemoveCmdService] Removing Service .... ");
      if (DeleteService(schService) == 0) {
         g_pLogger->logError(L"[RemoveCmdService] DeleteService failed %s - %s", SERVICE_NAME,
                           GetLastErrorText());
      } else {
         g_pLogger->logInfo("[RemoveCmdService] Success !");
      }
   }

   CloseServiceHandle(schSCManager);
   CloseServiceHandle(schService);

   g_pLogger->logInfo("[RemoveCmdService] return");
   return;
}
