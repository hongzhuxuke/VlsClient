#include "WinPCapLogic.h"



WinPCapLogic::WinPCapLogic()
{

}


WinPCapLogic::~WinPCapLogic()
{
}



HRESULT STDMETHODCALLTYPE WinPCapLogic::QueryInterface(REFIID riid, void ** appvObject) {
   if (riid == IID_VHIUnknown) {
      *appvObject = (VH_IUnknown*)this;
      AddRef();
      return CRE_OK;
   }
   else if (IID_IWINPCAP == riid) {
      *appvObject = (WinPCapLogic*)this;
      AddRef();
      return CRE_OK;
   }
   return CRE_NOINTERFACE;
}

ULONG STDMETHODCALLTYPE WinPCapLogic::AddRef(void) {
   return ::InterlockedIncrement(&m_lRefCount);
}

ULONG STDMETHODCALLTYPE WinPCapLogic::Release(void) {
   return ::InterlockedDecrement(&m_lRefCount);
}

bool WinPCapLogic::Create() {
   if (mWPcap == NULL) {
      mWPcap = new WPcap;
   }
   return true;
}

void WinPCapLogic::Destroy() {
   if (mWPcap) {
      delete mWPcap;
      mWPcap = nullptr;
   }
}


void STDMETHODCALLTYPE WinPCapLogic::GetSendAndRecvDatLen(int& netRecv, int &netSend) {
   if (mWPcap) {
      mWPcap->GetSendAndRecvDatLen(netRecv, netSend);
   }
}
bool STDMETHODCALLTYPE WinPCapLogic::IsNetDevChanged(){
   bool bRet = false;
   if (mWPcap) {
      bRet = mWPcap->IsNetDevChanged();
   }
   return bRet;
}
bool STDMETHODCALLTYPE WinPCapLogic::StartCaptureThread(){
   if (mWPcap) {
      mWPcap->StartCaptureThread();
   }
   return true;
}
int STDMETHODCALLTYPE WinPCapLogic::StopCaptureThread() {
   if (mWPcap) {
      mWPcap->StopCaptureThread();
   }
   return 0;
}
