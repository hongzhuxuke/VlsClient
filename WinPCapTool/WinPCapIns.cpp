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
//ʵ���ӿڲ�ѯ
HRESULT WinPCapIns::InstanceQueryInterface(REFIID riid, void ** appvObject) {
   if (mWinPCapLogic) {
      return mWinPCapLogic->QueryInterface(riid, appvObject);
   }
   return 0;
}

//�������
HRESULT WinPCapIns::InstanceCreatePlugin() {
   InitDebugTrace(L"WinPCapIns", 6);
   return 0;
}

//���ٲ��
HRESULT WinPCapIns::InstanceDestroyPlugin() {
   if (mWinPCapLogic) {
      delete mWinPCapLogic;
      mWinPCapLogic = nullptr;
   }
   return 0;
}

//��ʼ����Ҫע�����Ϣ
HRESULT WinPCapIns::InstanceInitRegMessage(CRMessageIDQueue& aoCRMessageIDQueue) {
   return 0;
}

//������Ϣ
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