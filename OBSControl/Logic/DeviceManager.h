#ifndef __DEVICE_MANAGER__H_INCLUDE__
#define __DEVICE_MANAGER__H_INCLUDE__

#pragma once
#include <mutex>
#include "IDeviceManager.h"

class CAudioControl;
class CDeviceObtain;

class DeviceManager : public IDeviceManager {
public:
   DeviceManager(void);
   ~DeviceManager(void);

public:
   virtual BOOL STDMETHODCALLTYPE Create();

   virtual void STDMETHODCALLTYPE Destroy();

public:

   /////////////////////////////IUnknown½Ó¿Ú/////////////////////////////////////////
   virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void ** appvObject);
   virtual ULONG STDMETHODCALLTYPE   AddRef(void);
   virtual ULONG STDMETHODCALLTYPE   Release(void);

private:

   virtual void STDMETHODCALLTYPE GetMicDevices(DeviceList& deviceList);

   virtual void STDMETHODCALLTYPE GetSpeakerDevices(DeviceList& deviceList);

   virtual void STDMETHODCALLTYPE GetVedioDevices(DeviceList& deviceList);

   virtual void STDMETHODCALLTYPE SetMute(LPWSTR wsDeviceId, bool bMute);

   virtual void STDMETHODCALLTYPE GetMute(LPWSTR wsDeviceId, bool* bMute);

   virtual void STDMETHODCALLTYPE SetMasterVolume(DeviceInfo, float fVolume);

   virtual void STDMETHODCALLTYPE GetMasterVolume(DeviceInfo, float* fVolume);

   virtual void STDMETHODCALLTYPE SetEnhanceLevel(LPWSTR wsDeviceId, float fLevel);

   virtual void STDMETHODCALLTYPE GetEnhanceLevel(LPWSTR wsDeviceId, float *pfLevel, float *pfMin, float *pfMax, float *pfStep);

   virtual void STDMETHODCALLTYPE GetResolution(vector<FrameInfo>& mediaOutputList, vector<SIZE>& resolutions, UINT64& minFrameInterval, UINT64& maxFrameInterval, const wstring& deviceName, const wstring& deviceID);

   virtual void STDMETHODCALLTYPE OpenPropertyPages(HWND hWndOwner, DeviceInfo);

private:
   std::mutex   mDevMutex;
   CAudioControl* m_pAudioControl;
   CDeviceObtain* m_pDeviceObtain;

   long           m_lRefCount;
};

#endif //__DEVICE_MANAGER__H_INCLUDE__