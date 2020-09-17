#pragma once
#include <windows.h>
#include <thread> 
#include "VhallUI_define.h"
class RtcGetDeviceList
{
public:
   RtcGetDeviceList();
   ~RtcGetDeviceList();
   void SetSubscribe(DevicelistSubscribe* sub);
   void SetRequestType(int type);
   void ProcessTask();
   static DWORD WINAPI ThreadProcess(LPVOID p);
private:
   DevicelistSubscribe *mDevSub = NULL;
   int mRequestType;
   std::thread* mGetDevThread = nullptr;
   static HANDLE threadHandle;
   static std::atomic_bool bManagerThreadRun;
};

