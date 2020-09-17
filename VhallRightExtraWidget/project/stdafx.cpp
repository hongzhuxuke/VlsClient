#include "stdafx.h"
#include "ConfigSetting.h"
//#include "SocketClient.h"
#include <ShlObj.h>
#include "VhallRightExtraWidgetIns.h"

#pragma comment (lib, "Version.lib")
extern "C" HRESULT WINAPI GetClassObject(REFIID riid, void ** appvObj) {
   return SingletonVhallRightExtraWidgetIns::Instance().QueryInterface(riid, appvObj);
}