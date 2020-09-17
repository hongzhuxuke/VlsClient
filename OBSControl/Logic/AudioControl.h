#ifndef __AUDIO_CONTROL_INCLUDE_H_
#define __AUDIO_CONTROL_INCLUDE_H_

#include <windows.h>
#include <list>
#include "stdafx.h"

using namespace std;

//-----------------------------------------------------------
// Client implementation of IAudioEndpointVolumeCallback
// interface. When a method in the IAudioEndpointVolume
// interface changes the volume level or muting state of the
// endpoint device, the change initiates a call to the
// client's IAudioEndpointVolumeCallback::OnNotify method.
//-----------------------------------------------------------

class CAudioControl;

class CAudioEndpointVolumeCallback : public IAudioEndpointVolumeCallback
{
public:
	CAudioEndpointVolumeCallback(CAudioControl* pAudioControl);

	~CAudioEndpointVolumeCallback();

	// IUnknown methods -- AddRef, Release, and QueryInterface

	ULONG STDMETHODCALLTYPE AddRef()
	{
		return InterlockedIncrement(&m_lRef);
	}

	ULONG STDMETHODCALLTYPE Release()
	{
		ULONG ulRef = InterlockedDecrement(&m_lRef);
		if (0 == ulRef)
		{
			delete this;
		}
		return ulRef;

	}

	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, VOID **ppvInterface)
	{
		if (IID_IUnknown == riid)
		{
			AddRef();
			*ppvInterface = (IUnknown*)this;
		}
		else if (__uuidof(IAudioEndpointVolumeCallback) == riid)
		{
			AddRef();
			*ppvInterface = (IAudioEndpointVolumeCallback*)this;
		}
		else
		{
			*ppvInterface = NULL;
			return E_NOINTERFACE;
		}
		return S_OK;
	}

	// Callback method for endpoint-volume-change notifications.

	HRESULT STDMETHODCALLTYPE OnNotify(PAUDIO_VOLUME_NOTIFICATION_DATA pNotify);

private:
	CAudioControl* m_pAudioControl;
	LONG m_lRef;
};

class IAudioEndpointVolume;
class IMMDeviceEnumerator;
class IMMDevice;
class CAudioControl
{
public:
   CAudioControl(HWND hWnd);
	~CAudioControl();

   bool Create();
   void Destroy();

public:
	// 设置关联设备，使用前应先调用此函数
	bool SetControl(LPWSTR wstrDeviceId);

	//音量相关
	bool GetMute(bool *pbMute);
	bool SetMute(bool bMute);
	bool GetMasterVolumeLevelScalar(float *pfLevel);
	bool SetMasterVolumeLevelScalar(float fLevel);

	//增强相关
	bool GetEnhanceLevel(float *pfLevel, float *pfMin, float *pfMax, float *pfStep);
	bool SetEnhanceLevel(float fLevel);

	//系统声音改变消息通知
	void OnNotify(PAUDIO_VOLUME_NOTIFICATION_DATA pNotify);

private:
	IAudioEndpointVolume *m_pEndptVolCtrl;
	list<IAudioEndpointVolume*> m_pCallVolCtrlList;
	IMMDeviceEnumerator *m_pEnumerator;
	IMMDevice *m_pDevice;
	list<IMMDevice*> m_pCallDeviceList;
	CAudioEndpointVolumeCallback* m_pAudioVolumeCallback;

	IAudioVolumeLevel *m_pMicEnhancedLevel;

   wstring m_wsCurDeviceID;
};

#endif __AUDIO_CONTROL_INCLUDE_H_
