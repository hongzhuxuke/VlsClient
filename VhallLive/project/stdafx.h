#pragma once
// Windows Header Files:
#include <WinSock2.h> 
#include <windows.h>
#include <tchar.h>
#include <QtWidgets/QApplication>
#include <string>

#include "VH_ConstDeff.h"
#include "VH_Error.h"
#include "DebugTrace.h"
#include "CRPluginDef.h"


#ifdef _DEBUG
#pragma comment(lib, "MessageDispatcher.lib")
#pragma comment(lib, "PluginManager.lib")
#else
#pragma comment(lib, "MessageDispatcherR.lib")
#pragma comment(lib, "PluginManagerR.lib")
#endif
#pragma comment (lib, "Version.lib")
#pragma comment (lib, "ws2_32.lib")

extern std::wstring gCurrentVersion;
extern std::string gCurStreamID;
extern std::string gToken;
extern std::wstring gCurClientType;


