#ifndef VHALLMICCONTROL_H
#define VHALLMICCONTROL_H
#include <endpointvolume.h>
#include <QObject>
#include <XString>
using namespace std;
class VHallMicControl;
class VHallMicControlVolumnCallBack :public IAudioEndpointVolumeCallback {
public:
   VHallMicControlVolumnCallBack();
   void setVhallControl(VHallMicControl *);
   virtual HRESULT STDMETHODCALLTYPE OnNotify(
      PAUDIO_VOLUME_NOTIFICATION_DATA pNotify);
   ULONG STDMETHODCALLTYPE AddRef() {
      return InterlockedIncrement(&m_lRef);
   }

   ULONG STDMETHODCALLTYPE Release() {
      ULONG ulRef = InterlockedDecrement(&m_lRef);
      if (0 == ulRef) {
         delete this;
      }
      return ulRef;

   }

   HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, VOID **ppvInterface) {
      if (IID_IUnknown == riid) {
         AddRef();
         *ppvInterface = (IUnknown*)this;
      } else if (__uuidof(IAudioEndpointVolumeCallback) == riid) {
         AddRef();
         *ppvInterface = (IAudioEndpointVolumeCallback*)this;
      } else {
         *ppvInterface = NULL;
         return E_NOINTERFACE;
      }
      return S_OK;
   }
private:
   LONG m_lRef;
   VHallMicControl *control = NULL;
};
class VHallMicControl : public QObject {
   Q_OBJECT
public:
   explicit VHallMicControl(QObject *parent = 0);
   ~VHallMicControl();
   bool openDevice(QString);
   void setVolumn(int);
   void volumnNotify(int);
   int getVolumn();
signals:
   void sigVolumnChanged(int);
   public slots:
private:
   bool isInit;
   QString defaultDeviceId;
   VHallMicControlVolumnCallBack *callBack = NULL;

};

#endif // VHALLMICCONTROL_H
