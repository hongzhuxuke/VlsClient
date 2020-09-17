#ifndef __SETTING_LOGIC__H_INCLUDE__
#define __SETTING_LOGIC__H_INCLUDE__
#include "ISettingLogic.h"

#pragma once

class SysSettingLogic;
class TextSettingLogic;
class WindowSettingLogic;
class CameraMgrLogic;
class SettingLogic : public ISettingLogic {
public:
   SettingLogic(void);
   ~SettingLogic(void);
   virtual BOOL STDMETHODCALLTYPE Create();
   virtual void STDMETHODCALLTYPE Destroy();


   /////////////////////////////IUnknown接口/////////////////////////////////////////
   virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void ** appvObject);
   virtual ULONG STDMETHODCALLTYPE   AddRef(void);
   virtual ULONG STDMETHODCALLTYPE   Release(void);
   // 处理消息
   void DealMessage(DWORD adwMessageID, void* apData, DWORD adwLen);
   
   virtual bool STDMETHODCALLTYPE InitConfig();
   virtual bool STDMETHODCALLTYPE GetDevices();
   virtual bool STDMETHODCALLTYPE IsCameraShow();
   virtual void STDMETHODCALLTYPE ApplySettings(int);
   virtual void STDMETHODCALLTYPE SetPriviewMic(DeviceInfo info);
   virtual void STDMETHODCALLTYPE InitAudioSetting();
   virtual void STDMETHODCALLTYPE InitCameraSetting();
   virtual bool STDMETHODCALLTYPE GetIsServerPlayback();   
   virtual void STDMETHODCALLTYPE ActiveSettingUI();   
   virtual void *STDMETHODCALLTYPE LockVideo(QString strDeviceID = QString(),int index = -1);
   virtual void STDMETHODCALLTYPE UnlockVideo(void *);
   virtual void STDMETHODCALLTYPE ResetLockVideo();
   virtual void STDMETHODCALLTYPE DealCameraSelect(void* apData, DWORD adwLen);
   virtual void STDMETHODCALLTYPE InitSysSetting();
   virtual void STDMETHODCALLTYPE SetDesktopShareState(bool);
   virtual void STDMETHODCALLTYPE SetCutRecordDisplay(const int iCutRecord);
   virtual void STDMETHODCALLTYPE ReSelectPushStreamLine();
   virtual void STDMETHODCALLTYPE PushStreamLineSuccess();
   virtual void STDMETHODCALLTYPE SetDesktopShare(bool isCapture);
   virtual void STDMETHODCALLTYPE SetLiveState(bool isCapture);
   virtual bool STDMETHODCALLTYPE IsHasAudioDevice();
   virtual void STDMETHODCALLTYPE SetUsedDevice(bool add, DeviceInfo info);

private:
   SysSettingLogic* m_pSysSettingLogic = nullptr;
   TextSettingLogic* m_pTextSettingLogic = nullptr;
   WindowSettingLogic* m_pWindowSettingLogic = nullptr;
   CameraMgrLogic* m_pCameraMgrLogic = nullptr;
   long m_lRefCount;
};

#endif //__SETTING_LOGIC__H_INCLUDE__
