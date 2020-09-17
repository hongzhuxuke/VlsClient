#include "StdAfx.h"
#include "SettingLogic.h"
#include "SysSettingLogic.h"
#include "TextSettingLogic.h"
#include "WindowSettingLogic.h"
#include "CameraMgrLogic.h"

SettingLogic::SettingLogic(void)
: m_lRefCount(1) {

}

SettingLogic::~SettingLogic(void) {
   TRACE6("%s delete end\n", __FUNCTION__);
}

BOOL SettingLogic::Create() {
   do {
      if (m_pSysSettingLogic == nullptr) {
         m_pSysSettingLogic = new SysSettingLogic;
         if (NULL == m_pSysSettingLogic || !m_pSysSettingLogic->Create()) {
            ASSERT(FALSE);
            break;
         }
      }

      if (m_pTextSettingLogic == nullptr) {
         m_pTextSettingLogic = new TextSettingLogic;
         if (NULL == m_pTextSettingLogic || !m_pTextSettingLogic->Create()) {
            ASSERT(FALSE);
            break;
         }
      }

      if (m_pWindowSettingLogic == nullptr) {
         m_pWindowSettingLogic = new WindowSettingLogic;
         if (NULL == m_pWindowSettingLogic || !m_pWindowSettingLogic->Create()) {
            ASSERT(FALSE);
            break;
         }
      }

      if (m_pCameraMgrLogic == nullptr) {
         m_pCameraMgrLogic = new CameraMgrLogic;
         if (NULL == m_pCameraMgrLogic || !m_pCameraMgrLogic->Create()) {
            ASSERT(FALSE);
            break;
         }
      }
      return TRUE;

   } while (FALSE);

   return FALSE;
}

void SettingLogic::Destroy() {
   if (NULL != m_pCameraMgrLogic) {
      m_pCameraMgrLogic->Destroy();
      delete m_pCameraMgrLogic;
      m_pCameraMgrLogic = NULL;
   }
   TRACE6("%s delete m_pCameraMgrLogic end\n", __FUNCTION__);
   if (NULL != m_pWindowSettingLogic) {
      m_pWindowSettingLogic->Destroy();
      delete m_pWindowSettingLogic;
      m_pWindowSettingLogic = NULL;
   }
   TRACE6("%s delete m_pWindowSettingLogic end\n", __FUNCTION__);
   if (NULL != m_pTextSettingLogic) {
      m_pTextSettingLogic->Destroy();
      delete m_pTextSettingLogic;
      m_pTextSettingLogic = NULL;
   }
   TRACE6("%s delete m_pTextSettingLogic end\n", __FUNCTION__);
   if (NULL != m_pSysSettingLogic) {
      m_pSysSettingLogic->Destroy();
      delete m_pSysSettingLogic;
      m_pSysSettingLogic = NULL;
   }
   TRACE6("%s delete m_pSysSettingLogic end\n", __FUNCTION__);
}

void SettingLogic::DealMessage(DWORD adwMessageID, void* apData, DWORD adwLen) {
   if (NULL != m_pSysSettingLogic) {
      m_pSysSettingLogic->DealMessage(adwMessageID, apData, adwLen);
   }

   if (NULL != m_pTextSettingLogic) {
      m_pTextSettingLogic->DealMessage(adwMessageID, apData, adwLen);
   }

   if (NULL != m_pWindowSettingLogic) {
      m_pWindowSettingLogic->DealMessage(adwMessageID, apData, adwLen);
   }

   if (NULL != m_pCameraMgrLogic) {
      m_pCameraMgrLogic->DealMessage(adwMessageID, apData, adwLen);
   }
}

HRESULT STDMETHODCALLTYPE SettingLogic::QueryInterface(REFIID riid, void ** appvObject) {
   if (riid == IID_VHIUnknown) {
      *appvObject = (VH_IUnknown*)this;
      AddRef();
      return CRE_OK;
   } else if (IID_ISettingLogic == riid) {
      *appvObject = (ISettingLogic*)this;
      AddRef();
      return CRE_OK;
   }
   return CRE_NOINTERFACE;
}

ULONG STDMETHODCALLTYPE SettingLogic::AddRef(void) {
   return ::InterlockedIncrement(&m_lRefCount);
}

ULONG STDMETHODCALLTYPE SettingLogic::Release(void) {
   return ::InterlockedDecrement(&m_lRefCount);
}

bool STDMETHODCALLTYPE SettingLogic::IsCameraShow() {
   if (m_pCameraMgrLogic) {
      return m_pCameraMgrLogic->IsCameraShow();
   }
   return false;
}

bool STDMETHODCALLTYPE SettingLogic::InitConfig() {
   if (!m_pSysSettingLogic) {
      return false;
   }
   m_pSysSettingLogic->InitDev();
   return true;
}

bool STDMETHODCALLTYPE SettingLogic::GetDevices(){
    if (!m_pSysSettingLogic) {
       return false;
    }
    m_pSysSettingLogic->GetDevice();
    return true;
}

void STDMETHODCALLTYPE SettingLogic::ApplySettings(int applyIdentify) {
   if(!m_pSysSettingLogic) {
      return ;
   }
   m_pSysSettingLogic->ApplySettings(applyIdentify);
}

void STDMETHODCALLTYPE SettingLogic::SetPriviewMic(DeviceInfo info) {
   if (!m_pSysSettingLogic) {
      return;
   }
   m_pSysSettingLogic->SetPriviewMic(info);
}

void STDMETHODCALLTYPE SettingLogic::SetUsedDevice(bool add, DeviceInfo info) {
   if (!m_pSysSettingLogic) {
      return;
   }
   m_pSysSettingLogic->SetUsedDevice(add, info);
}


void STDMETHODCALLTYPE SettingLogic::InitAudioSetting() {
   if(!m_pSysSettingLogic) {
      return ;
   }
   
   m_pSysSettingLogic->InitAudioSetting();
}
void STDMETHODCALLTYPE SettingLogic::InitCameraSetting() {
   if(!m_pSysSettingLogic) {
      return ;
   }

   m_pSysSettingLogic->InitCameraSetting();
}
bool STDMETHODCALLTYPE SettingLogic::GetIsServerPlayback() {
   if(!m_pSysSettingLogic) {
      return false;
   }

   return m_pSysSettingLogic->GetIsServerPlayback();   
}
void STDMETHODCALLTYPE SettingLogic::ActiveSettingUI(){
   if(!m_pSysSettingLogic) {
      return ;
   }

   return m_pSysSettingLogic->ActiveSettingUI();   
}

void *STDMETHODCALLTYPE SettingLogic::LockVideo(QString strDeviceID, int index ) {
   return m_pCameraMgrLogic->LockVideo(strDeviceID, index);
}
void STDMETHODCALLTYPE SettingLogic::UnlockVideo(void *v){
   m_pCameraMgrLogic->UnlockVideo(v);
}

void STDMETHODCALLTYPE SettingLogic::ResetLockVideo() {
   if (m_pCameraMgrLogic) {
      m_pCameraMgrLogic->ResetLockVideo();
   }
}

void STDMETHODCALLTYPE SettingLogic::DealCameraSelect(void* apData, DWORD adwLen){
   m_pCameraMgrLogic->DealCameraSelect(apData,adwLen);
}
void STDMETHODCALLTYPE SettingLogic::InitSysSetting() {
   if(!m_pSysSettingLogic) {
      return ;
   }
   m_pSysSettingLogic->InitSysSetting();
}

void STDMETHODCALLTYPE SettingLogic::SetDesktopShareState(bool state) {
   if (!m_pCameraMgrLogic) {
      return;
   }
   m_pCameraMgrLogic->SetDesktopShareState(state);
}

void STDMETHODCALLTYPE SettingLogic::SetCutRecordDisplay(const int iCutRecord)
{
	if (NULL != m_pSysSettingLogic){
		m_pSysSettingLogic->SetCutRecordDisplay(iCutRecord);
	}
}

void STDMETHODCALLTYPE SettingLogic::SetDesktopShare(bool isCapture) {
   if (NULL != m_pSysSettingLogic) {
      m_pSysSettingLogic->SetDesktopShare(isCapture);
   }
}

void STDMETHODCALLTYPE SettingLogic::SetLiveState(bool isCapture) {
   if (NULL != m_pSysSettingLogic) {
      m_pSysSettingLogic->SetLiveState(isCapture);
   }
}

bool STDMETHODCALLTYPE SettingLogic::IsHasAudioDevice() {
   if (NULL != m_pSysSettingLogic) {
      return m_pSysSettingLogic->IsHasAudioDevice();
   }
   return false;
}

void STDMETHODCALLTYPE SettingLogic::ReSelectPushStreamLine() {
    if (NULL != m_pSysSettingLogic){
        m_pSysSettingLogic->ReSelectPushStreamLine();
    }
}

void STDMETHODCALLTYPE SettingLogic::PushStreamLineSuccess() {
    if (NULL != m_pSysSettingLogic) {
        m_pSysSettingLogic->PushStreamLineSuccess();
    }
}
