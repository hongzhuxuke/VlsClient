#include "AudioControl.h"
#include "OBSControlIns.h"

#include "Msg_OBSControl.h"

const IID IID_IDeviceTopology = __uuidof(IDeviceTopology);
const IID IID_IPart = __uuidof(IPart);
const IID IID_IAudioVolumeLevel = __uuidof(IAudioVolumeLevel);
const IID IID_IConnector = __uuidof(IConnector);

CAudioEndpointVolumeCallback::CAudioEndpointVolumeCallback(CAudioControl* pAudioControl)
: m_lRef(1)
, m_pAudioControl(pAudioControl) {

}

CAudioEndpointVolumeCallback::~CAudioEndpointVolumeCallback() {
   m_pAudioControl = NULL;
}

HRESULT STDMETHODCALLTYPE CAudioEndpointVolumeCallback::OnNotify(PAUDIO_VOLUME_NOTIFICATION_DATA pNotify) {
   if (pNotify == NULL) {
      return E_INVALIDARG;
   }

   if (NULL == m_pAudioControl) {
      _ASSERT(0);
      return E_INVALIDARG;
   }

   m_pAudioControl->OnNotify(pNotify);

   return S_OK;
}

CAudioControl::CAudioControl(HWND hWnd)
: m_pAudioVolumeCallback(NULL)
, m_pMicEnhancedLevel(NULL)
, m_pEndptVolCtrl(NULL)
, m_pDevice(NULL)
, m_pEnumerator(NULL) {
}


CAudioControl::~CAudioControl() {
}

bool CAudioControl::Create() {
   HRESULT hr = S_OK;

   // Initialize COM component
   hr = CoInitialize(NULL);

   if (FAILED(hr)) {
      printf("CoInitialize failed");
   }

   // Create instance for MMDevices...
   hr = CoCreateInstance(__uuidof(MMDeviceEnumerator),
                         NULL, CLSCTX_INPROC_SERVER,
                         __uuidof(IMMDeviceEnumerator),
                         (void**)&m_pEnumerator);
   if (FAILED(hr)) {
      printf("CoCreateInstance failed");
   }

   m_pAudioVolumeCallback = new CAudioEndpointVolumeCallback(this);

   if (NULL == m_pAudioVolumeCallback) {
      _ASSERT(0);
      return false;
   }

   DWORD flags = DEVICE_STATE_ACTIVE;
   /*flags |= DEVICE_STATE_UNPLUGGED;*/
   IMMDeviceCollection *pCollection = NULL;
   if (FAILED(m_pEnumerator->EnumAudioEndpoints(eAll, flags, &pCollection))) {
      _ASSERT(0);
      SafeRelease(m_pEnumerator)
         return false;
   }

   UINT uCount = 0;
   if (SUCCEEDED(pCollection->GetCount(&uCount))) {
      for (UINT i = 0; i < uCount; i++) {
         IMMDevice *pDevice = NULL;
         if (SUCCEEDED(pCollection->Item(i, &pDevice))) {
            if (NULL != pDevice) {
               IAudioEndpointVolume* pCallVolCtrl = NULL;
               if (SUCCEEDED(pDevice->Activate(__uuidof(IAudioEndpointVolume),
                  CLSCTX_ALL, NULL, (void**)&pCallVolCtrl))) {
                  if (FAILED(pCallVolCtrl->RegisterControlChangeNotify((IAudioEndpointVolumeCallback*)m_pAudioVolumeCallback))) {
                     _ASSERT(0);
                     return false;
                  }

                  m_pCallVolCtrlList.push_back(pCallVolCtrl);
                  m_pCallDeviceList.push_back(pDevice);
               }
            }
         }
      }
   }
   return true;
}

void CAudioControl::Destroy() {
   //销毁所有使用的volCtrl
   list<IAudioEndpointVolume*>::iterator itorCtrl = m_pCallVolCtrlList.begin();
   while (itorCtrl != m_pCallVolCtrlList.end()) {
      SafeRelease(*itorCtrl);
      itorCtrl++;
   }

   //销毁所有使用的device
   list<IMMDevice*>::iterator itorDevice = m_pCallDeviceList.begin();
   while (itorDevice != m_pCallDeviceList.end()) {
      SafeRelease(*itorDevice)
         itorDevice++;
   }

   if (NULL != m_pAudioVolumeCallback) {
      delete m_pAudioVolumeCallback;
      m_pAudioVolumeCallback = NULL;
   }

   SafeRelease(m_pMicEnhancedLevel)
      SafeRelease(m_pEndptVolCtrl)
      SafeRelease(m_pDevice)
      SafeRelease(m_pEnumerator)

      CoUninitialize();
}

bool CAudioControl::SetControl(LPWSTR wstrDeviceId) {

   if (0 == wcscmp(wstrDeviceId, m_wsCurDeviceID.c_str())) {
      return true;
   }
   m_wsCurDeviceID = wstrDeviceId;

   SafeRelease(m_pEndptVolCtrl)
      SafeRelease(m_pDevice)

   if (NULL == m_pEnumerator) {
      printf("Init failed");
      return false;
   }

   if (FAILED(m_pEnumerator->GetDevice(wstrDeviceId, &m_pDevice))) {
      printf("Init failed");
      return false;
   }

   // Activate devices...
   if (FAILED(m_pDevice->Activate(__uuidof(IAudioEndpointVolume),
      CLSCTX_ALL, NULL,
      (void**)&m_pEndptVolCtrl))) {
      printf("Init failed");
      return false;
   }

   return true;
}

bool CAudioControl::GetMute(bool *pbMute) {
   BOOL isMult;
   if ((NULL == m_pEndptVolCtrl) || (m_pEndptVolCtrl->GetMute(&isMult) != S_OK))
      return false;
   else {
      if (isMult)
         *pbMute = true;
      else
         *pbMute = false;
      return true;
   }
}

bool CAudioControl::SetMute(bool bMute) {
   if ((NULL == m_pEndptVolCtrl) || (m_pEndptVolCtrl->SetMute(bMute, &GUID_NULL) != S_OK)) {
      return false;
   } else {
      return true;
   }
}

bool CAudioControl::GetMasterVolumeLevelScalar(float *pfLevel) {
   if ((NULL == m_pEndptVolCtrl) || (m_pEndptVolCtrl->GetMasterVolumeLevelScalar(pfLevel) != S_OK)) {
      return false;
   } else {
      return true;
   }
}

bool CAudioControl::SetMasterVolumeLevelScalar(float fLevel) {
   if ((NULL == m_pEndptVolCtrl) || FAILED(m_pEndptVolCtrl->SetMasterVolumeLevelScalar(fLevel, &GUID_NULL))) {
      return false;
   } else {
      return true;
   }
}

bool CAudioControl::GetEnhanceLevel(float *pfLevel, float *pfMin, float *pfMax, float *pfStep) {
   //新设备重新初始化该项
   SafeRelease(m_pMicEnhancedLevel)

      HRESULT hr = S_OK;
   DataFlow flow;
   IDeviceTopology *pDeviceTopology = NULL;
   IConnector *pConnFrom = NULL;
   IConnector *pConnTo = NULL;
   IPart *pPartPrev = NULL;
   IPart *pPartNext = NULL;
   std::wstring microphoneBoostName = L"麦克风加强";//if your system language is English,the name is "microphone boost"  

   if (NULL == m_pDevice) {
      return false;
   }

   // Get the endpoint device's IDeviceTopology interface.  
   hr = m_pDevice->Activate(
      IID_IDeviceTopology, CLSCTX_ALL, NULL,
      (void**)&pDeviceTopology);

   if (FAILED(hr)) {
      _ASSERT(0);
      return false;
   }

   // The device topology for an endpoint device always  
   // contains just one connector (connector number 0).  
   hr = pDeviceTopology->GetConnector(0, &pConnFrom);

   if (FAILED(hr)) {
      _ASSERT(0);
      return false;
   }

   // Make sure that this is a capture device.  
   hr = pConnFrom->GetDataFlow(&flow);
   if (FAILED(hr) || flow != Out) {
      // Error -- this is a rendering device.  
      _ASSERT(0);
      return false;
   }

   // Outer loop: Each iteration traverses the data path  
   // through a device topology starting at the input  
   // connector and ending at the output connector.  
   while (TRUE) {
      BOOL bConnected;
      hr = pConnFrom->IsConnected(&bConnected);

      // Does this connector connect to another device?  
      if (bConnected == FALSE) {
         // This is the end of the data path that  
         // stretches from the endpoint device to the  
         // system bus or external bus. Verify that  
         // the connection type is Software_IO.  
         ConnectorType  connType;
         hr = pConnFrom->GetType(&connType);

         if (connType == Software_IO) {
            break;  // finished  
         }

      }
      // Get the connector in the next device topology,  
      // which lies on the other side of the connection.  
      hr = pConnFrom->GetConnectedTo(&pConnTo);
      SafeRelease(pConnFrom)

      if (NULL == pConnTo) {
         _ASSERT(0);
         return false;
      }

      // Get the connector's IPart interface.  
      hr = pConnTo->QueryInterface(
         IID_IPart, (void**)&pPartPrev);
      SafeRelease(pConnTo)

         // Inner loop: Each iteration traverses one link in a  
         // device topology and looks for input multiplexers.  
      while (TRUE) {
         PartType parttype;
         IPartsList *pParts = NULL;
         // Follow downstream link to next part.  
         hr = pPartPrev->EnumPartsOutgoing(&pParts);
         if(SUCCEEDED(hr)&&pParts) {
            hr = pParts->GetPart(0, &pPartNext);
            SafeRelease(pParts);
         }
         
         if (NULL == pPartNext){
            return false;
         }

         hr = pPartNext->GetPartType(&parttype);

         LPWSTR pName;
         if (SUCCEEDED(pPartNext->GetName(&pName))) {
            // Failure of the following call means only that  
            // the part is not a boost (micrphone boost).  
            if (microphoneBoostName.compare(pName) == 0) {
               //get IAudioVolumeLevel to control volume   
               hr = pPartNext->Activate(CLSCTX_ALL, __uuidof(IAudioVolumeLevel), (void**)&m_pMicEnhancedLevel);

               if (FAILED(hr)) {
                  _ASSERT(0);
                  return false;
               }

               //获取麦克风增强的最小最大值，步长  
               hr = m_pMicEnhancedLevel->GetLevelRange(0, pfMin, pfMax, pfStep);

               if (FAILED(hr)) {
                  _ASSERT(0);
                  return false;
               }

               //获取当前麦克风增强值
               hr = m_pMicEnhancedLevel->GetLevel(0, pfLevel);

               if (FAILED(hr)) {
                  _ASSERT(0);
                  return false;
               }

               goto Exit;
            }
            CoTaskMemFree(pName);
         }
         GUID subType;
         pPartNext->GetSubType(&subType);
         if (parttype == Connector) {
            // We've reached the output connector that  
            // lies at the end of this device topology.  
            hr = pPartNext->QueryInterface(
               IID_IConnector,
               (void**)&pConnFrom);
            SafeRelease(pPartPrev)
               SafeRelease(pPartNext)
               break;
         }
         SafeRelease(pPartPrev)
            pPartPrev = pPartNext;
         pPartNext = NULL;
      }
   }
Exit:
   SafeRelease(pConnFrom)
      SafeRelease(pConnTo)
      SafeRelease(pPartPrev)
      SafeRelease(pPartNext)
      return true;
}

bool CAudioControl::SetEnhanceLevel(float fLevel) {
   if ((NULL == m_pMicEnhancedLevel) || (m_pMicEnhancedLevel->SetLevelUniform(fLevel, NULL) != S_OK)) {
      return false;
   }

   return true;
}

void CAudioControl::OnNotify(PAUDIO_VOLUME_NOTIFICATION_DATA pNotify) {
   if (NULL == pNotify) {
      _ASSERT(0);
      return;
   }

   SingletonOBSControlIns::Instance().PostCRMessage(MSG_OBSCONTROL_AUDIO_NOTIFY, NULL, 0);
}

