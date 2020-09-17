#pragma once

#if _MSC_VER >= 1600  
#pragma execution_character_set("utf-8")  
#endif

#define _M_AXP64
#include <windows.h>
#include <QBoxLayout>
#include <string>

#include "BaseThread.h"
#include "DebugTrace.h"
#include "CRPluginDef.h"
#include "VH_ConstDeff.h"

#include "PublishInfo.h"
#include "Logging.h"

#include "MainUIIns.h"
#include "Defs.h"

using namespace std;

extern bool gIsHideLogo;
extern SIZE gBaseSize;
extern float gDownScale[4];
extern OBSOutPutInfo GOutputInfo[];

#define KBPS_360P    500
#define KBPS_540P    950
#define KBPS_720P    1250
#define KBPS_1080P   1800

//startup manager
//void Notify2Service(const char* msgType);

//wstring GetAppPath();
wstring GetAppDataPath();
DWORD CountSetBits(ULONG_PTR bitMask);
//void GetProcessor(int&coreCount, int& logicalCores, DWORD& dwProcessorSpeed);

bool stringToWString(const string& narrowStr, wstring& wStr);
void ReplaceString(std::wstring& str, const std::wstring& src, const std::wstring& des);
bool sendUpdateMsg();
void initStreamBitrate();
void Optimization();
bool GetIsEnabled1080p();
