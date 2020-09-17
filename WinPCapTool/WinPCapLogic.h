#pragma once

#include "IWinPCapData.h"
#include "WPcap.h"

class WinPCapLogic: public IWINPCAP
{
public:
   WinPCapLogic();
   ~WinPCapLogic();

   virtual bool STDMETHODCALLTYPE Create();
   virtual void STDMETHODCALLTYPE Destroy();

   virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void ** appvObject);
   virtual ULONG STDMETHODCALLTYPE AddRef(void);
   virtual ULONG STDMETHODCALLTYPE Release(void);

   virtual void STDMETHODCALLTYPE GetSendAndRecvDatLen(int& netRecv, int &netSend);
   virtual bool STDMETHODCALLTYPE IsNetDevChanged();
   virtual bool STDMETHODCALLTYPE StartCaptureThread();
   virtual int STDMETHODCALLTYPE StopCaptureThread();

private:
   long           m_lRefCount;
   WPcap* mWPcap = NULL;
};

