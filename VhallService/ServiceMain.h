#ifndef __SERVICE_MAIN_INCLUDE__
#define __SERVICE_MAIN_INCLUDE__
#define SERVICE_NAME  L"Vhall Service"
#define SERVICE_DES  L"Vhall Service For BroadCast Agent."

void WINAPI ServiceMain(DWORD dwArgc,LPTSTR *lpArgv);
void InstallCmdService(WCHAR* serviceFileName);
void RemoveCmdService(WCHAR* serviceFileName);
LPWSTR GetLastErrorText();

#endif