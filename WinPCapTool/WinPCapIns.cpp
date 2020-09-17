#include "WinPCapIns.h"
#include "StdAfx.h"

WinPCapIns::WinPCapIns()
:CPluginBase(PID_IWinPCap, ENUM_PLUGIN_WINPCAP, 10)
{
   mWinPCapLogic = new WinPCapLogic();
}


WinPCapIns::~WinPCapIns()
{

}


//----------------------------------------------------------------------------------------------------------------
//实例接口查询
HRESULT WinPCapIns::InstanceQueryInterface(REFIID riid, void ** appvObject) {
   if (mWinPCapLogic) {
      return mWinPCapLogic->QueryInterface(riid, appvObject);
   }
   return 0;
}

//创建插件
HRESULT WinPCapIns::InstanceCreatePlugin() {
   InitDebugTrace(L"WinPCapIns", 6);
   return 0;
}

//销毁插件
HRESULT WinPCapIns::InstanceDestroyPlugin() {
   if (mWinPCapLogic) {
      delete mWinPCapLogic;
      mWinPCapLogic = nullptr;
   }
   return 0;
}

//初始化需要注册的消息
HRESULT WinPCapIns::InstanceInitRegMessage(CRMessageIDQueue& aoCRMessageIDQueue) {
   return 0;
}

//处理消息
HRESULT WinPCapIns::InstanceDealCRMessage(DWORD adwSenderID, DWORD adwMessageID, void * apData, DWORD adwLen) {
   return 0;
}


unsigned int WinPCapIns::CheckMessageThread(void * apParam) {
   return 0;
}

unsigned int WinPCapIns::CheckMessageWork() {
   return 0;
}

extern "C" HRESULT WINAPI GetClassObject(REFIID riid, void ** appvObj) {
   return SingletonWinPCapToolKitIns::Instance().QueryInterface(riid, appvObj);
}