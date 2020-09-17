#include "vhallmiccontrol.h"
#include <string>
#include <windows.h>
#pragma hdrstop
#pragma argsused

#include <tchar.h>
#include <stdio.h>
#include <conio.h>
#include <commctrl.h>
#include "mmsystem.h"
#include <Winerror.h>
#include <devicetopology.h>
#include <mmdeviceapi.h>
#include <QDebug>
#include <endpointvolume.h>
#include <audioclient.h>
#define SafeRelease(punk)  \
if ((punk) != NULL)  \
{ (punk)->Release(); (punk) = NULL; }

const IID IID_IDeviceTopology = __uuidof(IDeviceTopology);
const IID IID_IPart = __uuidof(IPart);
const IID IID_IAudioVolumeLevel = __uuidof(IAudioVolumeLevel);
const IID IID_IConnector = __uuidof(IConnector);
static IMMDeviceEnumerator *G_pEnumerator = NULL;
static IMMDevice *G_pDevice = NULL;
static IAudioEndpointVolume *G_pEndptVolCtrl = NULL;
bool GetDefaultDevice(QString &defaultDeviceId) {
   EDataFlow df = eCapture;
   const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
   const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
   IMMDeviceEnumerator *mmEnumerator;
   HRESULT err;
   err = CoInitialize(NULL);
   if (FAILED(err))
      return false;
   err = CoCreateInstance(CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL, IID_IMMDeviceEnumerator, (void**)&mmEnumerator);
   if (FAILED(err) || mmEnumerator == NULL)
      return false;
   //-------------------------------------------------------

   IMMDevice *defDevice;
   if (FAILED(mmEnumerator->GetDefaultAudioEndpoint(df, eCommunications, &defDevice))) {
      SafeRelease(mmEnumerator);
      CoUninitialize();
      return false;
   }

   wchar_t *wstrDefaultID = NULL;
   if (FAILED(defDevice->GetId((LPWSTR*)&wstrDefaultID))) {
      SafeRelease(defDevice);
      SafeRelease(mmEnumerator);
      CoUninitialize();

      return false;
   }

   defaultDeviceId = QString::fromWCharArray(wstrDefaultID);
   CoTaskMemFree((LPVOID)wstrDefaultID);
   SafeRelease(defDevice);
   SafeRelease(mmEnumerator);
   CoUninitialize();

   return true;
}
VHallMicControl::VHallMicControl(QObject *parent) :
QObject(parent),
isInit(false) {
   callBack = new VHallMicControlVolumnCallBack();
   callBack->setVhallControl(this);
   if (!GetDefaultDevice(defaultDeviceId)) {
      qDebug() << "GetDefaultDevice failed!";
      return;
   }
   HRESULT hr = S_OK;

   // Initialize COM component
   hr = CoInitialize(NULL);

   if (FAILED(hr)) {
      qDebug() << "CoInitialize failed!";
      return;
   }

   hr = CoCreateInstance(__uuidof(MMDeviceEnumerator),
                         NULL, CLSCTX_INPROC_SERVER,
                         __uuidof(IMMDeviceEnumerator),
                         (void**)&G_pEnumerator);

   if (FAILED(hr)) {
      qDebug() << "CoCreateInstance failed!";
      return;
   }

   isInit = true;
   qDebug() << "init true";
   this->openDevice(this->defaultDeviceId);
}
VHallMicControl::~VHallMicControl() {
   qDebug() << "~VHallMicControl";
   if (G_pEndptVolCtrl) {
      G_pEndptVolCtrl->UnregisterControlChangeNotify(callBack);
      SafeRelease(G_pEndptVolCtrl);
      delete callBack;
      callBack = NULL;
   }
   if (G_pDevice) {
      SafeRelease(G_pDevice);
   }


   if (G_pEnumerator) {
      SafeRelease(G_pEnumerator);
   }
   CoUninitialize();
}
bool VHallMicControl::openDevice(QString str) {
   if (G_pDevice != NULL) {
      SafeRelease(G_pDevice);
   }

   if (G_pEndptVolCtrl != NULL) {
      SafeRelease(G_pEndptVolCtrl);
   }

   wchar_t deviceId[1024] = { 0 };
   str.toWCharArray(deviceId);
   if (FAILED(G_pEnumerator->GetDevice(deviceId, &G_pDevice))) {
      qDebug() << "GetDevice failed!";
      return false;
   }
   if (FAILED(G_pDevice->Activate(__uuidof(IAudioEndpointVolume),
      CLSCTX_ALL, NULL,
      (void**)&G_pEndptVolCtrl))) {
      qDebug() << "Activate failed!";
      return false;
   }
   G_pEndptVolCtrl->RegisterControlChangeNotify((IAudioEndpointVolumeCallback*)this->callBack);
   qDebug() << "openDevice Successed!";
   return true;
}
void VHallMicControl::setVolumn(int value) {
   if (G_pEndptVolCtrl) {
      float fLevel = value;
      fLevel /= 100;
      G_pEndptVolCtrl->SetMasterVolumeLevelScalar(fLevel, &GUID_NULL);
   }
}
HRESULT STDMETHODCALLTYPE VHallMicControlVolumnCallBack::OnNotify(
   PAUDIO_VOLUME_NOTIFICATION_DATA pNotify) {
   if (pNotify) {
      if (this->control != NULL) {
         this->control->volumnNotify(pNotify->fMasterVolume * 100);
      }
   }
   return S_OK;
}
void VHallMicControl::volumnNotify(int value) {
   emit this->sigVolumnChanged(value);
   qDebug()<<"VHallMicControl::volumnNotify "<<value;
}
void VHallMicControlVolumnCallBack::setVhallControl(VHallMicControl *control) {
   this->control = control;
}
VHallMicControlVolumnCallBack::VHallMicControlVolumnCallBack() :
control(NULL) {

}
int VHallMicControl::getVolumn()
{
   int ret=-1;
   if(G_pEndptVolCtrl)
   {
      float vol=-1;
      G_pEndptVolCtrl->GetMasterVolumeLevelScalar(&vol);
      ret=vol*100;
   }

   
   return ret;
}
