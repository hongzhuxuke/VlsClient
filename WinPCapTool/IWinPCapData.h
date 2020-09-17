#pragma once

#include "VH_IUnknown.h"
#include <QObject>

// 公共数据
// {E17F81D0-3309-40B2-8474-42CCB1EB77EE}
DEFINE_GUID(IID_IWINPCAP,
   0xedf196da5, 0xe606, 0x4292, 0xbe, 0x6e, 0x35, 0x29, 0x49, 0xc2, 0xa7, 0x50);


class IWINPCAP : public VH_IUnknown {
public:
   virtual bool STDMETHODCALLTYPE Create() = 0;
   virtual void STDMETHODCALLTYPE Destroy() = 0;
   virtual void STDMETHODCALLTYPE GetSendAndRecvDatLen(int& netRecv,int &netSend) = 0;
   virtual bool STDMETHODCALLTYPE IsNetDevChanged() = 0;
   virtual bool STDMETHODCALLTYPE StartCaptureThread() = 0;
   virtual int STDMETHODCALLTYPE StopCaptureThread() = 0;
};