#include "StdAfx.h"
#include "DeviceManager.h"
#include "AudioControl.h"
#include "DeviceObtain.h"


DeviceManager::DeviceManager(void)
: m_lRefCount(1)
, m_pAudioControl(NULL)
, m_pDeviceObtain(NULL) {

}

DeviceManager::~DeviceManager(void) {

}

BOOL DeviceManager::Create() {

   do {
      //音頻控制
      m_pAudioControl = new CAudioControl(NULL);
      if (NULL == m_pAudioControl || !m_pAudioControl->Create()) {
         ASSERT(FALSE);
         break;
      }

      //设备获取
      m_pDeviceObtain = new CDeviceObtain();
      if (NULL == m_pDeviceObtain || !m_pDeviceObtain->Create()) {
         ASSERT(FALSE);
         break;
      }

      return TRUE;
   } while (FALSE);
   return FALSE;
}

void DeviceManager::Destroy() {
   //销毁设备获取
   std::unique_lock<std::mutex> lock(mDevMutex);
   if (NULL != m_pDeviceObtain) {
      m_pDeviceObtain->Destroy();
      delete m_pDeviceObtain;
      m_pDeviceObtain = NULL;
   }

   //销毁AudioControl
   if (NULL != m_pAudioControl) {
      m_pAudioControl->Destroy();
      delete m_pAudioControl;
      m_pAudioControl = NULL;
   }
}

HRESULT STDMETHODCALLTYPE DeviceManager::QueryInterface(REFIID riid, void ** appvObject) {
   if (riid == IID_VHIUnknown) {
      *appvObject = (VH_IUnknown*)this;
      AddRef();
      return CRE_OK;
   } else if (IID_IDeviceManager == riid) {
      *appvObject = (IDeviceManager*)this;
      AddRef();
      return CRE_OK;
   }
   return CRE_NOINTERFACE;
}

ULONG STDMETHODCALLTYPE DeviceManager::AddRef(void) {
   return ::InterlockedIncrement(&m_lRefCount);
}

ULONG STDMETHODCALLTYPE DeviceManager::Release(void) {
   return ::InterlockedDecrement(&m_lRefCount);
}

void STDMETHODCALLTYPE DeviceManager::GetMicDevices(DeviceList& deviceList) {
   std::unique_lock<std::mutex> lock(mDevMutex);
   if (NULL != m_pDeviceObtain) {
      m_pDeviceObtain->GetAudioDevices(deviceList, eCapture);
   }
}

void STDMETHODCALLTYPE DeviceManager::GetSpeakerDevices(DeviceList& deviceList) {
   std::unique_lock<std::mutex> lock(mDevMutex);
   if (NULL != m_pDeviceObtain) {
      m_pDeviceObtain->GetAudioDevices(deviceList, eRender);
   }
}

void STDMETHODCALLTYPE DeviceManager::GetVedioDevices(DeviceList& deviceList) {
   std::unique_lock<std::mutex> lock(mDevMutex);
   if (NULL != m_pDeviceObtain) {
      m_pDeviceObtain->GetVedioDevices(deviceList);
   }
}

void STDMETHODCALLTYPE DeviceManager::SetMute(LPWSTR wsDeviceId, bool bMute) {
   std::unique_lock<std::mutex> lock(mDevMutex);
   if (NULL != m_pAudioControl) {
      m_pAudioControl->SetControl(wsDeviceId);
      m_pAudioControl->SetMute(bMute);
   }
}

void STDMETHODCALLTYPE DeviceManager::GetMute(LPWSTR wsDeviceId, bool* bMute) {
   std::unique_lock<std::mutex> lock(mDevMutex);
   if (NULL != m_pAudioControl) {
      m_pAudioControl->SetControl(wsDeviceId);
      m_pAudioControl->GetMute(bMute);
   }
}

void STDMETHODCALLTYPE DeviceManager::SetMasterVolume(DeviceInfo device, float fVolume) {
   if (NULL != m_pAudioControl) {
      //m_pAudioControl->SetControl(device);
      //m_pAudioControl->SetMasterVolumeLevelScalar(fVolume);
      //未完成
      ASSERT(FALSE);
   }
}

void STDMETHODCALLTYPE DeviceManager::GetMasterVolume(DeviceInfo device, float* fVolume) {
   if (NULL != m_pAudioControl) {
      //m_pAudioControl->SetControl(device);
      //m_pAudioControl->GetMasterVolumeLevelScalar(fVolume);
      //未完成
      ASSERT(FALSE);
   }
}

void STDMETHODCALLTYPE DeviceManager::SetEnhanceLevel(LPWSTR wsDeviceId, float fLevel) {
   std::unique_lock<std::mutex> lock(mDevMutex);
   if (NULL != m_pAudioControl) {
      m_pAudioControl->SetControl(wsDeviceId);
      m_pAudioControl->SetEnhanceLevel(fLevel);
   }
}

void STDMETHODCALLTYPE DeviceManager::GetEnhanceLevel(LPWSTR wsDeviceId, float *pfLevel, float *pfMin, float *pfMax, float *pfStep) {
   std::unique_lock<std::mutex> lock(mDevMutex);
   if (NULL != m_pAudioControl) {
      m_pAudioControl->SetControl(wsDeviceId);
      m_pAudioControl->GetEnhanceLevel(pfLevel, pfMin, pfMax, pfStep);
   }
}

void STDMETHODCALLTYPE DeviceManager::GetResolution(vector<FrameInfo>& mediaOutputList, vector<SIZE>& resolutions, UINT64& minFrameInterval, UINT64& maxFrameInterval, const wstring& deviceName, const wstring& deviceID) {
   std::unique_lock<std::mutex> lock(mDevMutex);
   if (NULL != m_pDeviceObtain) {
      m_pDeviceObtain->GetResolution(mediaOutputList, resolutions, minFrameInterval, maxFrameInterval, deviceName, deviceID);
   }
}

void STDMETHODCALLTYPE DeviceManager::OpenPropertyPages(HWND hWndOwner,DeviceInfo deviceInfo) {
   std::unique_lock<std::mutex> lock(mDevMutex);
   if (NULL != m_pDeviceObtain) {
      m_pDeviceObtain->OpenPropertyPagesByName(hWndOwner, deviceInfo);
   }
}
