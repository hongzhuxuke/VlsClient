#include "SocketClient.h"
#include <QPixmap>
#include <QPainter>
#include <QImage>
#include <dwmapi.h>
#include <map>
#include <stdint.h>
#include <QDir>
#include <windows.h>
#include "StdAfx.h"
#include "OBSControlLogic.h"
#include "IMainUILogic.h"
#include "IGraphics.h"
#include "IAudioCapture.h"
#include "IMediaCore.h"
#include "ICommonData.h"
#include "IMediaReader.h"
#include "ISettingLogic.h"
#include "PublishInfo.h"
#include "ConfigSetting.h"      
#include "MediaDefs.h"
#include "Msg_OBSControl.h"
#include "Msg_CommonToolKit.h"
#include "Msg_MainUI.h"
#include "Defs.h"
#include "live_open_define.h"
#include "CoreAudio.h"
#include "vhall_log.h"
#include "pathManage.h"
#include "pathmanager.h"
#include "pub.Const.h"
#include "pub.Struct.h"

static OSVERSIONINFO G_osVersionInfo;

#define DefaultZoomRatio (double)1.2
#define LogoSize { 101, 45 }
#define DefaultBaseSize { 1920, 1080 }
QPixmap OBSControlLogicCreateTextPixmap(QString str) {
   int pointSize = 30;
   QFont f;
   f.setPointSize(pointSize);

   QFontMetrics fm = QFontMetrics(f);
   int textW = fm.width(str);

   int textH = pointSize;
   QPixmap textPixmap = QPixmap(textW, textH*1.5);
   textPixmap.fill(QColor(0, 0, 0, 0));

   QPainter pText(&textPixmap);
   pText.setFont(f);

   QPen pen;
   pen.setColor(QColor(158, 158, 158, 255));
   pText.setPen(pen);
   pText.drawText(0, textH*1.25, str);

   return textPixmap;
}


bool OBSControlLogicCreateTextureByText(wchar_t *text, unsigned char *&buf, int &w, int &h) {
   QString str = CHANGE_CAMERA;
   QString failedStr = CAMERA_CHANGE_SIZE;
   QPixmap pfailed = OBSControlLogicCreateTextPixmap(failedStr);
   QPixmap p = OBSControlLogicCreateTextPixmap(str);
   if (p.isNull() || pfailed.isNull()) {
      return false;
   }

   QImage imgFailed = pfailed.toImage();
   QImage img = p.toImage();

   w = img.width();
   h = img.height();

   int w_failed = imgFailed.width();
   int h_failed = imgFailed.height();

   if (w == 0 || h == 0 || w_failed == 0 || h_failed == 0) {
      return false;
   }

   int tw = w > w_failed ? w : w_failed;
   int th = h + h_failed;

   buf = new unsigned char[tw*th * 4];
   memset(buf, 0, tw*th * 4);

   int px = tw - w;
   px /= 2;

   for (int i = 0; i < h; i++) {
      for (int j = 0; j < w; j++) {
         union argbPixel {
            unsigned int pixel;
            unsigned char pixels[4];
         };

         argbPixel pixel;
         pixel.pixel = img.pixel(j, i);

         unsigned char b = pixel.pixels[0];
         unsigned char g = pixel.pixels[1];
         unsigned char r = pixel.pixels[2];
         unsigned char a = pixel.pixels[3] = 0xF0;
         *(unsigned int *)(buf + i * w * 4 + (j + px) * 4) = a << 24 | b << 16 | g << 8 | r;
      }
   }

   px = tw - w_failed;
   px /= 2;

   for (int i = 0; i < h_failed; i++) {
      for (int j = 0; j < w_failed; j++) {
         union argbPixel {
            unsigned int pixel;
            unsigned char pixels[4];
         };

         argbPixel pixel;
         pixel.pixel = imgFailed.pixel(j, i);

         unsigned char b = pixel.pixels[0];
         unsigned char g = pixel.pixels[1];
         unsigned char r = pixel.pixels[2];
         unsigned char a = pixel.pixels[3] = 0xF0;
         *(unsigned int *)(buf + (i + h)*w * 4 + (j + px) * 4) = a << 24 | b << 16 | g << 8 | r;
      }
   }

   w = tw;
   h = th;
   TRACE0("OBSControlLogicCreateTextureByText[%p]\n", buf);

   return true;
}
void OBSControlLogicFreeMemory(unsigned char *buf) {
   if (!buf) {
      return;
   }
   delete buf;
}

RECT getLogoRect(SIZE gBaseSize, SIZE gDefaultSize, SIZE gDefaultLogoSize, double gDefaultZoomRatio);


OBSControlLogic::OBSControlLogic(void)
   : m_lRefCount(1)
   , m_pGraphics(NULL)
   , m_pAudioCapture(NULL)
   , m_pMediaCore(NULL)
   , m_pMediaReader(NULL)
   , m_pMediaItem(NULL)
   , m_pAudioItemVedioPlay(NULL)
   , m_bStartVedioPlay(false)
   , m_bStartStream(false)
   , m_bIsAudioExist(true)
   , m_bIsSaveFile(false)
   , mRecordEOF(true) {

}

OBSControlLogic::~OBSControlLogic(void) {
   TRACE6("%s delete end\n", __FUNCTION__);
}
long OBSControlLogicPostMsg(DWORD adwMessageID, void * apData, DWORD adwLen) {
   return SingletonOBSControlIns::Instance().PostCRMessage(adwMessageID, apData, adwLen);
}

int OBSControlLogic::OSGetVersion() {
   if (G_osVersionInfo.dwMajorVersion > 6)
      return 8;

   if (G_osVersionInfo.dwMajorVersion == 6) {
      //Windows 8
      if (G_osVersionInfo.dwMinorVersion >= 2)
         return 8;
      //Windows 7
      if (G_osVersionInfo.dwMinorVersion == 1)
         return 7;
      //Vista
      if (G_osVersionInfo.dwMinorVersion == 0)
         return 6;
   }
   return 0;
}

void GetCurVersion(const wchar_t* exeFile, char *version) {
   DWORD          dwVerHnd = 0;
   VS_FIXEDFILEINFO* pFileInfo = NULL;
   unsigned int      dwInfoSize = 0;
   char  *        pVersionInfo = NULL;
   DWORD          dwVerInfoSize = GetFileVersionInfoSizeW(exeFile, &dwVerHnd);

   if (dwVerInfoSize) {
      pVersionInfo = new char[dwVerInfoSize];
      GetFileVersionInfoW(exeFile, dwVerHnd, dwVerInfoSize, pVersionInfo);
      VerQueryValue(pVersionInfo, (LPTSTR)("\\"), (void**)&pFileInfo, &dwInfoSize);
      sprintf(version, "%d.%d.%d.%d",
         HIWORD(pFileInfo->dwFileVersionMS),
         LOWORD(pFileInfo->dwFileVersionMS),
         HIWORD(pFileInfo->dwFileVersionLS),
         LOWORD(pFileInfo->dwFileVersionLS));


      delete[] pVersionInfo;
      pVersionInfo = NULL;
   }
}

BOOL OBSControlLogic::Create() {
   do {
      mbIsDestoryed = false;
      wstring confPath = GetAppDataPath() + CONFIGPATH;
      QString qsConfPath = qsConfPath.fromStdWString(confPath);
      int audio_debug = ConfigSetting::ReadInt(qsConfPath, GROUP_DEFAULT, DEBUG_AUDIO_LISTEN, 0);
      if (audio_debug == 1 && m_vhPlayback == nullptr) {
         m_vhPlayback = new VHCoreAudio();
         m_vhPlayback->Reset(NULL, 44100, 2);
      }

      TRACE6("%s CreateMediaCore start\n", __FUNCTION__);
      //创建并初始化MediaCore
      if (m_pMediaCore == nullptr) {
         m_pMediaCore = CreateMediaCore();
         if (NULL == m_pMediaCore) {
            ASSERT(FALSE);
            break;
         }
      }

      TRACE6("%s CreateMediaCore ok\n", __FUNCTION__);
      int logLevel = ConfigSetting::ReadInt(qsConfPath, GROUP_DEFAULT, MEDIA_CORE_LOG_LEVEL, VHALL_LOG_LEVEL_INFO);
      if (logLevel > 0) {
         QString logPath = QString::fromStdWString(GetAppDataPath()) + QString::fromWCharArray(VH_LOG_DIR) + "\\media_core_log";
         m_pMediaCore->SetLogLevel(logPath.toLocal8Bit().data(), logLevel);
      }

      G_osVersionInfo.dwOSVersionInfoSize = sizeof(G_osVersionInfo);
      GetVersionEx(&G_osVersionInfo);

      char currVersion[128] = { 0 };
      WCHAR          szApp[1024 * 4] = L"";
      GetModuleFileNameW(NULL, szApp, 1024 * 4);
      GetCurVersion(szApp, currVersion);
      m_pMediaCore->SetVersion(currVersion);

      VH::CComPtr<IMainUILogic> pMainUILogic;
      DEF_GET_INTERFACE_PTR(SingletonOBSControlIns, PID_IMainUI, IID_IMainUILogic, pMainUILogic, return FALSE, ASSERT(FALSE));
      HWND hMsgWnd = NULL;
      HWND hRenderWnd = NULL;
      if (pMainUILogic) {
         pMainUILogic->GetHwnd((void**)&hMsgWnd, (void**)&hRenderWnd);
      }
      TRACE6("%s pMainUILogic->GetHwnd ok\n", __FUNCTION__);
      VH::CComPtr<ICommonData> pCommonData;
      DEF_GET_INTERFACE_PTR(SingletonOBSControlIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return FALSE, ASSERT(FALSE));
      OBSOutPutInfo outputInfo;
      if (pCommonData) {
         pCommonData->GetOutputInfo(&outputInfo);
      }
      TRACE6("%s CreateGraphics start  outputInfo.m_baseSize  %d  %d\n", __FUNCTION__, outputInfo.m_baseSize.cx, outputInfo.m_baseSize.cy);
      //创建并初始化Graphic
      if (m_pGraphics == nullptr) {
         m_pGraphics = CreateGraphics(hMsgWnd, hRenderWnd, outputInfo.m_baseSize, 0);
         if (NULL == m_pGraphics) {
            ASSERT(FALSE);
            return FALSE;
         }
      }
      TRACE6("%s CreateGraphics ok\n", __FUNCTION__);
      m_pGraphics->SetMediaCore(m_pMediaCore);
      TRACE6("%s SetMediaCore ok\n", __FUNCTION__);
      m_pMediaCore->InitCapture(this, m_pGraphics);
      TRACE6("%s InitCapture ok\n", __FUNCTION__);
      m_pGraphics->SetDataReceiver(m_pMediaCore->GetDataReceiver());
      TRACE6("%s SetDataReceiver ok\n", __FUNCTION__);
      m_pGraphics->SetCreateTextTextureMemory(OBSControlLogicCreateTextureByText, OBSControlLogicFreeMemory);
      TRACE6("%s SetCreateTextTextureMemory ok\n", __FUNCTION__);
      m_pGraphics->SetPostMsgHook(OBSControlLogicPostMsg);
      TRACE6("%s SetPostMsgHook ok\n", __FUNCTION__);
      TRACE6("%s SetCreateTextureFromFileHook ok\n", __FUNCTION__);
      m_pGraphics->SetSourceDeleteHook(OBSSourceDeleteHook, this);
      TRACE6("%s SetSourceDeleteHook ok\n", __FUNCTION__);

      m_pGraphics->Preview(outputInfo.m_baseSize, outputInfo.m_outputSize, outputInfo.m_isAuto/*, outputInfo.videoBits*/);
      TRACE6("%s Preview ok  outputInfo.m_baseSize  %d  %d outputInfo.m_outputSize %d %d \n", __FUNCTION__, outputInfo.m_baseSize.cx, outputInfo.m_baseSize.cy, outputInfo.m_outputSize.cx, outputInfo.m_outputSize.cy);
      int iAudioSampleRate = ConfigSetting::ReadInt(qsConfPath, GROUP_DEFAULT, KEY_AUDIO_SAMPLERATE, 44100);
      if (44100 != iAudioSampleRate) {
         ConfigSetting::writeValue(qsConfPath, GROUP_DEFAULT, KEY_AUDIO_SAMPLERATE, 44100);
         iAudioSampleRate = 44100;
      }
      TRACE6("%s CreateAudioCapture start\n", __FUNCTION__);
      //创建并初始化AudioCapture
      if (m_pAudioCapture == nullptr) {
         m_pAudioCapture = CreateAudioCapture(m_pMediaCore->GetDataReceiver(), iAudioSampleRate);
         if (NULL == m_pAudioCapture) {
            ASSERT(FALSE);
            break;
         }
      }

      TRACE6("%s CreateAudioCapture ok\n", __FUNCTION__);
      m_pAudioCapture->SetAudioListening(this);
      TRACE6("%s CreateAudioCapture ok\n", __FUNCTION__);
      TRACE6("%s InitPlaybackSource ok\n", __FUNCTION__);
      int interval = ConfigSetting::ReadInt(qsConfPath, GROUP_DEFAULT, AUDIO_INTERVAL, 5);
      m_pAudioCapture->SetInterval(interval);
      int priority = ConfigSetting::ReadInt(qsConfPath, GROUP_DEFAULT, AUDIO_PRIORITY, THREAD_PRIORITY_NORMAL);
      m_pAudioCapture->SetPriority(priority);//线程 优先级
      TRACE6("%s m_pAudioCapture->Start() ok\n", __FUNCTION__);
      //创建并初始化MediaReader
      if (mInitVLCThread == nullptr) {
         mInitVLCThread = new std::thread(OBSControlLogic::ThreadCreateMediaReader, this);
      }

      TRACE6("%s CreateMediaReader ok\n", __FUNCTION__);
      //初始状态默认单声道
      SetForceMono(true);
      //初始化扬声器为 静音状态
      m_pAudioCapture->MuteSpeaker(true);
      TRACE6("%s MuteSpeaker ok\n", __FUNCTION__);
      return TRUE;
   } while (FALSE);
   return FALSE;
}

void OBSControlLogic::SetAudioCapture(bool ok) {
   if (m_pAudioCapture) {
      if (ok) {
         m_pAudioCapture->Start();
      }
      else {
         m_pAudioCapture->Shutdown();
      }
   }
}

void OBSControlLogic::Reset(const bool bIsNewRecord) {

   if (NULL == m_pGraphics) {
      ASSERT(FALSE);
      return;
   }

   bool toResetStream = m_bStartStream;
   VH::CComPtr<ICommonData> pCommonData;
   DEF_GET_INTERFACE_PTR(SingletonOBSControlIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
   OBSOutPutInfo outputInfo;
   pCommonData->GetOutputInfo(&outputInfo);

   VH::CComPtr<IMainUILogic> pMainUILogic = NULL;
   DEF_GET_INTERFACE_PTR(SingletonOBSControlIns, PID_IMainUI, IID_IMainUILogic, pMainUILogic);

   if (toResetStream) {
      StopStream(true, bIsNewRecord && !pMainUILogic->IsStopRecord(), false);
   }

   SIZE curSize = m_pGraphics->GetBaseSize();
   if (curSize.cx != outputInfo.m_outputSize.cx || curSize.cy != outputInfo.m_outputSize.cy) {
      m_pGraphics->StopPreview(true);
      VideoSceneType sceneType = SceneType_Unknown;
      m_pGraphics->GetSceneInfo(sceneType);
      m_pGraphics->Preview(outputInfo.m_baseSize, outputInfo.m_outputSize, outputInfo.m_isAuto/*, outputInfo.videoBits*/);
      TRACE6("%s Reset Preview ok  outputInfo.m_baseSize  %d  %d outputInfo.m_outputSize %d %d \n", __FUNCTION__, outputInfo.m_baseSize.cx, outputInfo.m_baseSize.cy, outputInfo.m_outputSize.cx, outputInfo.m_outputSize.cy);

      m_pMediaItem = m_pGraphics->GetMediaSource();
      RefitItem();
   }

   if (toResetStream) {
      StartStream(true, bIsNewRecord && !pMainUILogic->IsStopRecord(), false);
   }
}

bool OBSControlLogic::IsHasPlaybackAudioDevice() {
   if (m_bIsAudioExist == false) {
      m_bIsAudioExist = m_pAudioCapture->Start();
   }

   return m_bIsAudioExist;
}

DWORD WINAPI OBSControlLogic::ThreadCreateMediaReader(LPVOID p) {
   if (p) {
      OBSControlLogic* logic = (OBSControlLogic*)p;
      if (logic) {
         logic->CreateMediaReaderInstance();
      }
   }
   return 0;
}

void OBSControlLogic::CreateMediaReaderInstance() {
   QMutexLocker locker(&mutex);
   TRACE6("%s CreateMediaReaderInstance start\n", __FUNCTION__);
   m_pMediaReader = CreateMediaReader();
   TRACE6("%s CreateMediaReaderInstance end\n", __FUNCTION__);
}

bool OBSControlLogic::IsExistMediaReader() {
   QMutexLocker locker(&mutex);
   return m_pMediaReader == nullptr ? false : true;
}

bool OBSControlLogic::SetRecordPath(bool isSave, wchar_t *path) {
   m_bIsSaveFile = isSave;
   wcscpy(m_wSaveFilePath, path);
   if (!m_pMediaCore) {
      ASSERT(FALSE);
      return false;
   }
   if (!m_bStartStream) {
      return false;
   }
   return true;
}

void OBSControlLogic::GetAudioMeter(float& audioMag, float& audioPeak, float& audioMax) {
   if (!m_pAudioCapture) {
      audioMag = audioPeak = audioMax = 0.0f;
   }
   else {
      m_pAudioCapture->GetAudioMeter(audioMag, audioPeak, audioMax);
   }
}

void OBSControlLogic::GetMicAudioMeter(float& audioMag, float& audioPeak, float& audioMax) {
   if (!m_pAudioCapture) {
      audioMag = audioPeak = audioMax = 0.0f;
   }
   else {
      m_pAudioCapture->GetMicOrignalAudiometer(audioMag, audioPeak, audioMax);
   }
}

void OBSControlLogic::SetSaveMicAudio(bool bSave) {
   if (m_pAudioCapture) {
      m_pAudioCapture->SetSaveMicAudio(bSave);
   }
}

int OBSControlLogic::GetGraphicsDeviceInfoCount() {
   if (!m_pGraphics) {
      return -1;
   }

   return m_pGraphics->GetGraphicsDeviceInfoCount();
}

bool OBSControlLogic::GetGraphicsDeviceInfo(DeviceInfo &deviceInfo,
   DataSourcePosType &posType, int count) {
   if (!m_pGraphics) {
      return false;
   }
   return m_pGraphics->GetGraphicsDeviceInfo(deviceInfo, posType, count);
}

bool OBSControlLogic::GetGraphicsDeviceInfoExist(DeviceInfo &deviceInfo, bool &isFullScreen) {
   if (!m_pGraphics) {
      return false;
   }
   return m_pGraphics->GetGraphicsDeviceInfoExist(deviceInfo, isFullScreen);
}

bool OBSControlLogic::ModifyDeviceSource(DeviceInfo&srcDevice,
   DeviceInfo&desDevice, DataSourcePosType posType) {
   if (!m_pGraphics) {
      return false;
   }
   return m_pGraphics->ModifyDeviceSource(srcDevice, desDevice, posType);
}

bool OBSControlLogic::ReloadDevice(DeviceInfo &deviceInfo) {
   if (!m_pGraphics) {
      return false;
   }
   return m_pGraphics->ReloadDevice(deviceInfo);
}

void OBSControlLogic::ResetNoiseCancellingbyMicState(bool mic_open) {
   wstring confPath = GetAppDataPath() + CONFIGPATH;
   QString qsConfPath = qsConfPath.fromStdWString(confPath);
   bool bNoiseReduction = ConfigSetting::ReadInt(qsConfPath, GROUP_DEFAULT, KEY_AUDIO_NOISEREDUCTION, 0);
   if (mic_open) { //麦克风开启了，如果已经开启降噪，则打开降噪
      if (bNoiseReduction == 1) {
         if (m_pMediaCore) {
            m_pMediaCore->SetOpenNoiseCancelling(true);
         }
      }
   }
   else {  //麦克风关闭了。则关闭麦克风降噪
      if (m_pMediaCore) {
         m_pMediaCore->SetOpenNoiseCancelling(false);
      }
   }
}


float OBSControlLogic::MuteMic(bool ok) {
   if (!m_pAudioCapture) {
      return 0;
   }
   VH::CComPtr<IMainUILogic> pMainUILogic = NULL;
   DEF_GET_INTERFACE_PTR(SingletonOBSControlIns, PID_IMainUI, IID_IMainUILogic, pMainUILogic);
   if (ok) {
      TRACE6("OBSControlLogic::MuteMic VHAV_STATUS_CLOSED\n");
      m_pAudioCapture->MuteMic(true);
      ResetNoiseCancellingbyMicState(false);
      if (pMainUILogic) {
         pMainUILogic->SetMicMute(true);
      }

   }
   else {
      m_pAudioCapture->MuteMic(false);
      ResetNoiseCancellingbyMicState(true);
      pMainUILogic->SetMicMute(m_pAudioCapture->GetMicVolunm() == 0);
      TRACE6("OBSControlLogic::MuteMic VHAV_STATUS_OPEND\n");
   }
   return m_pAudioCapture->GetMicVolunm();
}

float OBSControlLogic::MuteSpeaker(bool ok) {
   if (!m_pAudioCapture) {
      return 0;
   }

   if (ok) {
      m_pAudioCapture->MuteSpeaker(true);
   }
   else {
      m_pAudioCapture->MuteSpeaker(false);
   }
   return m_pAudioCapture->GetSpekerVolumn();
}

void OBSControlLogic::SetMicVolunm(float v) {
   if (!m_pAudioCapture) {
      return;
   }
   m_pAudioCapture->SetMicVolume(v);

   VH::CComPtr<IMainUILogic> pMainUILogic = NULL;
   DEF_GET_INTERFACE_PTR(SingletonOBSControlIns, PID_IMainUI, IID_IMainUILogic, pMainUILogic);
   if (0 == v) {
      TRACE6("OBSControlLogic::MuteMic VHAV_STATUS_CLOSED\n");
      if (pMainUILogic) {
         pMainUILogic->SetMicMute(true);
      }
   }
   else {
      TRACE6("OBSControlLogic::MuteMic VHAV_STATUS_OPEND\n");
      m_pAudioCapture->MuteMic(false);
      pMainUILogic->SetMicMute(m_pAudioCapture->GetMicVolunm() == 0);
      ResetNoiseCancellingbyMicState(true);
   }
}

void OBSControlLogic::SetSpekerVolumn(float v) {
   if (!m_pAudioCapture) {
      return;
   }
   m_pAudioCapture->SetPlaybackVolume(v);
}

float OBSControlLogic::GetMicVolunm() {
   if (!m_pAudioCapture) {
      return 0.0f;
   }
   return m_pAudioCapture->GetMicVolunm();
}
float OBSControlLogic::GetSpekerVolumn() {
   if (!m_pAudioCapture) {
      return 0.0f;
   }
   return m_pAudioCapture->GetSpekerVolumn();
}

bool OBSControlLogic::GetCurrentMic(DeviceInfo &deviceInfo) {
   if (!m_pAudioCapture) {
      return false;
   }
   return m_pAudioCapture->GetCurrentMic(deviceInfo);
}

bool OBSControlLogic::IsHasSource(SOURCE_TYPE type) {
   if (!m_pGraphics) {
      return false;
   }
   return m_pGraphics->IsHasSource(type);
}

void OBSControlLogic::DeviceRecheck() {
   if (!m_pGraphics) {
      return;
   }

   DeviceList deviceList;
   VH::CComPtr<IDeviceManager> pDeviceManager;
   DEF_GET_INTERFACE_PTR(SingletonOBSControlIns, PID_IOBSControl, IID_IDeviceManager, pDeviceManager, return, ASSERT(FALSE));
   pDeviceManager->GetVedioDevices(deviceList);
   m_pGraphics->DeviceRecheck(deviceList);
}

void OBSControlLogic::Destroy() {
   QMutexLocker locker(&mMediaCoreMutex);
   mbIsDestoryed = true;
   TRACE6("%s delete m_pAudioCapture start\n", __FUNCTION__);
   m_bIsAddLogoSource = false;
   if (mInitVLCThread) {
      mInitVLCThread->join();
      delete mInitVLCThread;
      mInitVLCThread = nullptr;
   }
   //销毁AudioCapture
   if (NULL != m_pAudioCapture) {
      m_pAudioCapture->Shutdown();
   }
   TRACE6("%s delete m_pAudioCapture end\n", __FUNCTION__);
   if (m_pMediaItem) {
      m_pGraphics->ClearSource(m_pMediaItem);
      m_pMediaItem = NULL;
   }
   TRACE6("%s delete m_pMediaItem end\n", __FUNCTION__);
   //销毁MediaReader
   if (m_pMediaReader) {
      m_pMediaReader->VhallStop();
      DestoryMediaReader(&m_pMediaReader);
   }
   TRACE6("%s delete m_pMediaReader end\n", __FUNCTION__);
   if (NULL != m_pAudioCapture) {
      DestroyAudioCapture(&m_pAudioCapture);
   }
   TRACE6("%s delete m_pAudioCapture end\n", __FUNCTION__);
   //stop MediaCore
   if (NULL != m_pMediaCore) {
      StopRecord(false);
      m_pMediaCore->StopRtmp(NULL);
   }
   TRACE6("%s delete m_pMediaCore end\n", __FUNCTION__);
   //销毁Graphics
   if (NULL != m_pGraphics) {
      m_pGraphics->StopPreview(false);
      m_pGraphics->Destory();
      DestoryGraphics(&m_pGraphics);
   }
   TRACE6("%s delete m_pGraphics end\n", __FUNCTION__);
   //销毁MediaCore
   if (NULL != m_pMediaCore) {
      m_pMediaCore->UninitCapture();
      DestoryMediaCore(&m_pMediaCore);
   }
   TRACE6("%s delete m_pMediaCore end\n", __FUNCTION__);
   if (m_vhPlayback) {
      delete m_vhPlayback;
      m_vhPlayback = NULL;
   }
   TRACE6("%s delete m_vhPlayback end\n", __FUNCTION__);
}

HRESULT STDMETHODCALLTYPE OBSControlLogic::QueryInterface(REFIID riid, void ** appvObject) {
   if (riid == IID_VHIUnknown) {
      *appvObject = (VH_IUnknown*)this;
      AddRef();
      return CRE_OK;
   }
   else if (IID_IOBSControlLogic == riid) {
      *appvObject = (IOBSControlLogic*)this;
      AddRef();
      return CRE_OK;
   }

   return CRE_NOINTERFACE;
}

ULONG STDMETHODCALLTYPE OBSControlLogic::AddRef(void) {
   return ::InterlockedIncrement(&m_lRefCount);
}

ULONG STDMETHODCALLTYPE OBSControlLogic::Release(void) {
   return ::InterlockedDecrement(&m_lRefCount);
}

void STDMETHODCALLTYPE OBSControlLogic::GetStreamStatus(std::vector<StreamStatus> * streamStats) {
   if (NULL != m_pMediaCore) {
      int count = m_pMediaCore->RtmpCount();
      for (int i = 0; i < count; i++) {
         StreamStatus status;
         memset(&status, 0, sizeof(StreamStatus));
         m_pMediaCore->GetStreamStatus(i, &status); 
         streamStats->push_back(status);
      }
   }
}

void ToRefit(void *param) {
   OBSControlLogic* pThis = (OBSControlLogic *)param;
   if (pThis) {
      pThis->RefitItem();
   }
}

bool  STDMETHODCALLTYPE OBSControlLogic::SetEnablePlayOutMediaAudio(bool enable) {
   if (NULL == m_pGraphics || !IsExistMediaReader() || NULL == m_pAudioCapture) {
      ASSERT(FALSE);
      return false;
   }
   m_pMediaReader->SetPlayOutAudio(enable);
   return true;
}

bool STDMETHODCALLTYPE OBSControlLogic::IsMediaPlaying() {
   return m_bStartVedioPlay;
}

bool  STDMETHODCALLTYPE OBSControlLogic::MediaPlay(char* fileName, bool audioFile) {
   if (NULL == m_pGraphics || !IsExistMediaReader() || NULL == m_pAudioCapture) {
      ASSERT(FALSE);
      return false;
   }
   TRACE6("%s\n", __FUNCTION__);
   void *mediaSourceItem = NULL;
   void *audioSourceItem = NULL;

   if (m_pMediaItem == NULL) {
      TRACE6("%s AddNewSource\n", __FUNCTION__);
      bool bAddSrc = m_pGraphics->AddNewSource(SRC_MEDIA_OUT, m_pMediaReader->GetMediaOut(), &mediaSourceItem);
      if (!bAddSrc) {
         TRACE6("%s add SRC_MEDIA_OUT new src err\n", __FUNCTION__);
         return false;
      }
      m_pMediaItem = mediaSourceItem;
   }

   if (m_pAudioItemVedioPlay == NULL) {
      TRACE6("%s AddRecodingSource\n", __FUNCTION__);
      void* param[2] = { m_pMediaReader->GetMediaOut(), NULL };
      bool bAddSrc = m_pAudioCapture->AddRecodingSource(TYPE_MEDIAOUT, &audioSourceItem, false, param, 1);
      if (bAddSrc) {
         m_pAudioItemVedioPlay = audioSourceItem;
      }
      else {
         m_pGraphics->ClearSourceByType(SRC_MEDIA_OUT);
         m_pMediaItem = NULL;
         TRACE6("%s add AddRecodingSource new src err\n", __FUNCTION__);
         return false;
      }
   }
   if (m_pMediaReader && m_pMediaReader->GetMediaOut()) {
      m_pMediaReader->GetMediaOut()->SetRefit(ToRefit, this);
      m_bStartVedioPlay = m_pMediaReader->VhallPlay(fileName, audioFile);
      if (audioFile) {
         m_pGraphics->ClearSourceByType(SRC_MEDIA_OUT);
         m_pMediaItem = NULL;
      }
   }

   m_pAudioCapture->MuteMic(true);
   m_pAudioCapture->MuteSpeaker(true);
   ResetNoiseCancellingbyMicState(false);
   VH::CComPtr<IMainUILogic> pMainUILogic = NULL;
   DEF_GET_INTERFACE_PTR(SingletonOBSControlIns, PID_IMainUI, IID_IMainUILogic, pMainUILogic);
   if (pMainUILogic) {
      pMainUILogic->SetMicMute(m_pAudioCapture->GetMicVolunm() == 0);
   }
   return m_bStartVedioPlay;
}

void STDMETHODCALLTYPE OBSControlLogic::MediaPause() {
   if (IsExistMediaReader()) {
      m_pMediaReader->VhallPause();
   }
}

void STDMETHODCALLTYPE OBSControlLogic::MediaResume() {
   if (IsExistMediaReader()) {
      m_pMediaReader->VhallResume();
   }
}

void STDMETHODCALLTYPE OBSControlLogic::MediaStop() {
   TRACE6("OBSControlLogic::MediaStop()\n");
   if (!m_bStartVedioPlay) {
      TRACE6("OBSControlLogic::MediaStop() !m_bStartVedioPlay\n");
      return;
   }

   if (NULL == m_pAudioCapture || NULL == m_pGraphics || !IsExistMediaReader()) {
      TRACE6("OBSControlLogic::MediaStop() NULL == m_pAudioCapture\n");
      ASSERT(FALSE);
      return;
   }
   TRACE6("VhallStop\n");
   if (m_pMediaReader) {
      m_pMediaReader->VhallStop();
   }
   TRACE6("VhallStop end\n");
   if (m_pAudioItemVedioPlay) {
      m_pAudioCapture->DelAudioSource(m_pAudioItemVedioPlay);
      m_pAudioItemVedioPlay = NULL;
   }

   if (m_pMediaItem) {
      //m_pGraphics->ClearSource(m_pMediaItem);
      m_pGraphics->ClearSourceByType(SRC_MEDIA_OUT);
      m_pMediaItem = NULL;
   }
   m_bStartVedioPlay = false;
   TRACE6("OBSControlLogic::MediaStop() return\n");

   m_pAudioCapture->MuteMic(false);
   ResetNoiseCancellingbyMicState(true);
   VH::CComPtr<IMainUILogic> pMainUILogic = NULL;
   DEF_GET_INTERFACE_PTR(SingletonOBSControlIns, PID_IMainUI, IID_IMainUILogic, pMainUILogic);
   if (pMainUILogic) {
      pMainUILogic->SetMicMute(m_pAudioCapture->GetMicVolunm() == 0);
   }
   TRACE6("OBSControlLogic::MediaStop() end\n");
}

void STDMETHODCALLTYPE OBSControlLogic::MediaSeek(const unsigned long long& seekTime) {
   if (IsExistMediaReader()) {
      m_pMediaReader->VhallSeek(seekTime);
   }
}

int STDMETHODCALLTYPE OBSControlLogic::GetPlayerState() {
   if (IsExistMediaReader()) {
      return m_pMediaReader->GetPlayerState();
   }
   return -1;
}

void STDMETHODCALLTYPE OBSControlLogic::SetVolume(const unsigned int & volume) {
   if (IsExistMediaReader()) {
      m_pMediaReader->SetVolume(volume);
   }
}

const long STDMETHODCALLTYPE OBSControlLogic::VhallGetMaxDulation() {
   if (IsExistMediaReader()) {
      return m_pMediaReader->VhallGetMaxDulation();
   }
   return 0;
}

const long STDMETHODCALLTYPE OBSControlLogic::VhallGetCurrentDulation() {
   if (IsExistMediaReader()) {
      return m_pMediaReader->VhallGetCurrentDulation();
   }
   return 0;
}

void STDMETHODCALLTYPE OBSControlLogic::OnMouseEvent(const UINT& mouseEvent, const POINTS& mousePos) {
   if (NULL != m_pGraphics) {
      return m_pGraphics->OnMouseEvent(mouseEvent, mousePos);
   }
}

void STDMETHODCALLTYPE OBSControlLogic::Resize(const RECT& client, bool bRedrawRenderFrame) {
   if (NULL != m_pGraphics) {
      return m_pGraphics->Resize(client, bRedrawRenderFrame);
   }
}

bool STDMETHODCALLTYPE OBSControlLogic::IsHasNoPersistentSource() {
   if (NULL != m_pGraphics) {
      return m_pGraphics->IsHasNoPersistentSource();
   }
   return false;
}

bool STDMETHODCALLTYPE OBSControlLogic::IsCanModify() {
   if (NULL != m_pGraphics) {
      int type = m_pGraphics->GetCurrentItemType();
      if (type == SRC_MEDIA_OUT || type == SRC_MONITOR_AREA || type == SRC_MONITOR) {
         return true;
      }
      else {
         return false;
      }
   }
   return false;
}

SOURCE_TYPE STDMETHODCALLTYPE OBSControlLogic::GetCurrentItemType() {
   int type = m_pGraphics->GetCurrentItemType();
   return (SOURCE_TYPE)type;
}
bool STDMETHODCALLTYPE OBSControlLogic::IsHasMonitorSource() {
   if (!m_pGraphics) {
      return false;
   }
   return m_pGraphics->IsHasMonitorSource();
}

void STDMETHODCALLTYPE OBSControlLogic::GetCurrentFramePicSize(int &w, int &h) {
   SIZE resolution = m_pGraphics->GetResolution();
   w = resolution.cx;
   h = resolution.cy;
}
void STDMETHODCALLTYPE OBSControlLogic::GetBaseSize(int &w, int &h) {
   SIZE baseSize = m_pGraphics->GetBaseSize();
   w = baseSize.cx;
   h = baseSize.cy;
}

unsigned char** STDMETHODCALLTYPE OBSControlLogic::LockCurrentFramePic(unsigned long long &t) {
   return m_pGraphics->LockCurrentFramePic(t);
}
void STDMETHODCALLTYPE OBSControlLogic::UnlockCurrentFramePic() {
   return m_pGraphics->UnlockCurrentFramePic();
}

bool STDMETHODCALLTYPE OBSControlLogic::SetSourceVisible(wchar_t *sourceName, bool isVisible, bool isWait) {
   return m_pGraphics->SetSourceVisible(sourceName, isVisible, isWait);
}
bool STDMETHODCALLTYPE OBSControlLogic::WaitSetSourceVisible() {
   return m_pGraphics->WaitSetSourceVisible();
}
unsigned char * STDMETHODCALLTYPE OBSControlLogic::MemoryCreate(int size) {
   return m_pGraphics->OBSMemoryCreate(size);
}
void STDMETHODCALLTYPE OBSControlLogic::MemoryFree(void *p) {
   return m_pGraphics->OBSMemoryFree(p);
}

void OBSControlLogic::DealMessage(DWORD adwMessageID, void* apData, DWORD adwLen) {
   QMutexLocker locker(&mMediaCoreMutex);
   if (mbIsDestoryed) {
      return;
   }
   switch (adwMessageID) {
      //推流
   case MSG_OBSCONTROL_PUBLISH:
      DealStartPublish(apData, adwLen);
      break;
   case MSG_OBSCONTROL_ENABLE_VT:
      DealEnableVt(apData, adwLen);
      break;
      //文本
   case MSG_OBSCONTROL_TEXT:
      DealTextControl(apData, adwLen);
      break;
      //图片
   case MSG_OBSCONTROL_IMAGE:
      DealImageControl(apData, adwLen);
      break;
      //桌面共享
   case MSG_OBSCONTROL_DESKTOPSHARE:
      DealDesktopShare(apData, adwLen);
      break;
      //软件源
   case MSG_OBSCONTROL_WINDOWSRC:
      DealAddWndSrc(apData, adwLen);
      break;
      //区域共享
   case MSG_OBSCONTROL_SHAREREGION:
      DealRegionShare(apData, adwLen);
      break;
      //处理源相关
   case MSG_OBSCONTROL_PROCESSSRC:
      DealProcessSrc(apData, adwLen);
      break;
      //处理添加摄像头
   case MSG_OBSCONTROL_ADDCAMERA:
      DealAddCamera(apData, adwLen);
      break;
   case  MSG_OBSCONTROL_VIDIO_HIGHQUALITYCOD:
      DealVidioSet(apData, adwLen);
      break;
      //处理声音捕获
   case MSG_OBSCONTROL_AUDIO_CAPTURE:
      DealAudioCapture(apData, adwLen);
      break;
   case MSG_OBSCONTROL_PUSH_AMF0:
      DealPushAMF0(apData, adwLen);
      break;
   case MSG_OBSCONTROL_CLOSE_AUDIO_DEV:
      HandleCloseAudioDev();
      break;
      //Click控制
   case MSG_MAINUI_CLICK_CONTROL:
      DealClickControl(apData, adwLen);
      break;
   case MSG_HTTPCENTER_HTTP_RS:
      DealPointRecordRs(apData, adwLen);
      break;
   default:
      break;
   }
}
bool GetRtmpInfoFromCustomConfig(std::list<RtmpInfo> &rInfos) {
   bool ret = false;
   QFile f;
   f.setFileName("CustomStream.txt");
   if (!f.open(QIODevice::ReadOnly)) {
      return false;
   }

   QByteArray ba = f.readAll();
   QJsonDocument doc = QJsonDocument::fromJson(ba);
   QJsonArray array = doc.array();

   f.close();

   for (int i = 0; i < array.count(); i++) {
      QJsonObject obj = array[i].toObject();
      if (!obj.isEmpty()) {
         ret = true;
         RtmpInfo  rInfo;
         rInfo.iMultiConnNum = 0;
         rInfo.iMultiConnBufSize = 0;

         QString rtmpURL = obj["rtmpURL"].toString();
         QString streamname = obj["streamname"].toString();
         QString strUser = obj["strUser"].toString();
         QString strPass = obj["strPass"].toString();
         QString token = obj["token"].toString();

         strncpy_s(rInfo.rtmpURL, rtmpURL.toUtf8().data(), MAX_BUFF);
         strncpy_s(rInfo.streamname, streamname.toUtf8().data(), MAX_BUFF);
         strncpy_s(rInfo.strUser, strUser.toUtf8().data(), MAX_BUFF);
         strncpy_s(rInfo.strPass, strPass.toUtf8().data(), MAX_BUFF);
         strncpy_s(rInfo.token, token.toUtf8().data(), MAX_BUFF);
         if (obj.contains("multi_num")) {
            rInfo.iMultiConnNum = obj["multi_num"].toInt();
         }
         if (obj.contains("multi_buf_size")) {
            rInfo.iMultiConnBufSize = obj["multi_buf_size"].toInt();
         }
         rInfos.push_back(rInfo);
      }
   }
   return ret;
}

void OBSControlLogic::StartStream(bool isNotify, bool bIsNewRecord, bool conn /*= false*/) {
   if (m_bStartStream) {
      return;
   }

   //获取流信息
   PublishInfo loStreamInfo;
   VH::CComPtr<ICommonData> pCommonData;
   DEF_GET_INTERFACE_PTR(SingletonOBSControlIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
   wstring confPath = GetAppDataPath() + CONFIGPATH;
   wstring confToolPath = GetAppDataPath() + VHALL_TOOL_CONFIG;
   QString qsConfPath = qsConfPath.fromStdWString(confPath);
   QString toolConfigPath = qsConfPath.fromStdWString(confToolPath);
   pCommonData->GetStreamInfo(loStreamInfo);
   VSSRoomPushUrl pushInfo;
   ClientApiInitResp startLiveInfo;
   pCommonData->GetVSSRoomPushUrl(pushInfo);
   pCommonData->GetInitApiResp(startLiveInfo);

   int defaultLine = ConfigSetting::ReadInt(qsConfPath, GROUP_DEFAULT, KEY_VIDEO_LINE, 0);
   if (defaultLine >= 0 && defaultLine < loStreamInfo.mPubLineInfo.count()) {
      loStreamInfo.mCurRtmpUrlIndex = defaultLine;
      loStreamInfo.mCurRtmpUrl = loStreamInfo.mPubLineInfo[defaultLine].mStrDomain;
   }
   else {
      ConfigSetting::writeValue(qsConfPath, GROUP_DEFAULT, KEY_VIDEO_LINE, 0);
   }

   //获得码率分辨率等信息
   OBSOutPutInfo outPutInfo;
   pCommonData->GetOutputInfo(&outPutInfo);
   VideoSceneType sceneType = SceneType_Unknown;
   m_pGraphics->GetSceneInfo(sceneType);
   RtmpInfo rtmpInfo;
   unsigned int videoBitRate = outPutInfo.videoBits;
   unsigned int audioBitRate = ConfigSetting::ReadInt(qsConfPath, GROUP_DEFAULT, KEY_AUDIO_KBPS, 96);
   if (audioBitRate < 8) {
      audioBitRate = 8;
   }
   else if (audioBitRate > 1024) {
      audioBitRate = 1024;
   }

   //FileInfo fileInfo;
   memset(&rtmpInfo, 0, sizeof(RtmpInfo));
   strcpy_s(rtmpInfo.rtmpURL, MAX_BUFF, loStreamInfo.mCurRtmpUrl.c_str());
   strcpy_s(rtmpInfo.streamname, MAX_BUFF, loStreamInfo.mStreamName.c_str());
   strcpy_s(rtmpInfo.strPass, MAX_BUFF, "");
   strcpy_s(rtmpInfo.strUser, MAX_BUFF, "");
   strcpy_s(rtmpInfo.token, MAX_BUFF, loStreamInfo.mToken.c_str());
   rtmpInfo.iMultiConnNum = loStreamInfo.mIMultiConnNum;
   rtmpInfo.iMultiConnBufSize = loStreamInfo.mIMultiConnBufSize;

   int syncTime = ConfigSetting::ReadInt(qsConfPath, GROUP_DEFAULT, KEY_AUDIO_SYNC, 200);
   int vcodec_fps = DEFAULT_VCODEC_FPS;
   vcodec_fps = ConfigSetting::ReadInt(qsConfPath, GROUP_DEFAULT, KEY_VCODEC_FPS, DEFAULT_VCODEC_FPS);
   int vcodec_gop = ConfigSetting::ReadInt(toolConfigPath, GROUP_DEFAULT, KEY_VCODEC_GOP, DEFAULT_VCODEC_GOP);
   if (vcodec_gop < 2) {
      vcodec_gop = DEFAULT_VCODEC_GOP;
   }
   float mediaCoreGop = float(vcodec_gop) + 0.2;
   vcodec_fps = vcodec_fps <= 0 ? DEFAULT_VCODEC_FPS : vcodec_fps;

   int high_codec_open = ConfigSetting::ReadInt(qsConfPath, GROUP_DEFAULT, KEY_VCODEC_HIGH_PROFILE_OPEN, DEFAULT_VCODEC_HIGH_PROFILE_OPEN);

   TRACE6("InitX264Encoder: vcodec_fps = %d, vcodec_high_profile_open\n", vcodec_fps, high_codec_open);
   if (NULL == m_pGraphics) {
      ASSERT(FALSE);
      return;
   }

   int video_process_filters = ConfigSetting::ReadInt(qsConfPath, GROUP_DEFAULT, X264_EX_PARAM_VIDEO_PROCESS_FILTERS, VIDEO_PROCESS_SCENETYPE | VIDEO_PROCESS_DIFFCHECK);
   bool is_adjust_bitrate = ConfigSetting::ReadInt(qsConfPath, GROUP_DEFAULT, X264_EX_PARAM_IS_ADJUST_BITRATE, 0) != 0;
   bool is_encoder_debug = ConfigSetting::ReadInt(qsConfPath, GROUP_DEFAULT, X264_EX_PARAM_IS_ENCODER_DEBUG, 0) != 0;
   bool is_saving_data_debug = ConfigSetting::ReadInt(qsConfPath, GROUP_DEFAULT, X264_EX_PARAM_IS_SAVING_DATA_DEBUG, 0) != 0;
   bool is_quality_limited = ConfigSetting::ReadInt(qsConfPath, GROUP_DEFAULT, X264_EX_PARAM_IS_QUALITY_LIMITED, 1) != 0;
   bool bNoiseReduction = ConfigSetting::ReadInt(qsConfPath, GROUP_DEFAULT, KEY_AUDIO_NOISEREDUCTION, 0);
   if (m_pMediaCore) {
      m_pMediaCore->SetVideoParamEx(video_process_filters, is_adjust_bitrate, is_quality_limited, is_encoder_debug, is_saving_data_debug, high_codec_open);
      SIZE size = m_pGraphics->GetResolution();
      int videoQuality = ConfigSetting::ReadInt(qsConfPath, GROUP_DEFAULT, KEY_VCODEC_VIDEO_QUALITY, 0);
      m_pMediaCore->SetVideoQuality(videoQuality > 0 ? true : false);
      m_pMediaCore->SetVideoParam(mediaCoreGop, vcodec_fps, videoBitRate, size.cx, size.cy);
      TRACE6("OBSControlLogic::SetVideoParam %d  w %d  h %d\n", videoQuality, size.cx, size.cy);
      int iAudioSampleRate = ConfigSetting::ReadInt(qsConfPath, GROUP_DEFAULT, KEY_AUDIO_SAMPLERATE, 44100);
      m_pMediaCore->SetAudioParam(2, iAudioSampleRate, audioBitRate);
      

      bool is_http_proxy = ConfigSetting::ReadInt(qsConfPath, GROUP_DEFAULT, PROXY_OPEN, 0) != 0;
      QString host = ConfigSetting::ReadString(qsConfPath, GROUP_DEFAULT, PROXY_HOST, "");
      int port = ConfigSetting::ReadInt(qsConfPath, GROUP_DEFAULT, PROXY_PORT, 80);
      QString usr = ConfigSetting::ReadString(qsConfPath, GROUP_DEFAULT, PROXY_USERNAME, "");
      QString pwd = ConfigSetting::ReadString(qsConfPath, GROUP_DEFAULT, PROXY_PASSWORD, "");
      m_pMediaCore->SetProxy(is_http_proxy, host.toStdString(), port, usr.toStdString(), pwd.toStdString());

      m_pGraphics->SetFps(vcodec_fps);
      std::list<RtmpInfo> rInfos;
      m_pMediaCore->ResetVoiceTransition();
      if (bIsNewRecord && conn)//本地状态需要修改  重新连接
      {
         StopRecord(conn);
      }

      m_pMediaCore->StopRtmp(NULL);

      rInfos.clear();

      if (!GetRtmpInfoFromCustomConfig(rInfos)) {
         rInfos.push_back(rtmpInfo);
      }

      std::string rtmpUri = "";
      bool ret = true;
      bool toStartRtmp = true;

      if (rInfos.size() > 0) {
         for (std::list<RtmpInfo>::iterator itera = rInfos.begin(); itera != rInfos.end(); itera++) {
            QString line_push_info = itera->rtmpURL;
            rtmpUri = itera->rtmpURL;
            if (rtmpUri.find("http://") != std::string::npos) {
               if (m_bDispatch) {
                  rtmpUri += "&&token=";
                  rtmpUri += m_dispatchParam.token;
                  rtmpUri += "&&webinar_id=";
                  rtmpUri += m_dispatchParam.webinar_id;
                  rtmpUri += "&&ismix=";
                  rtmpUri += (m_dispatchParam.ismix == 0 ? "0" : "1");
                  rtmpUri += "&&mixserver=";
                  rtmpUri += m_dispatchParam.mixserver;
                  rtmpUri += "&&accesstoken=";
                  rtmpUri += m_dispatchParam.accesstoken;
                  rtmpUri += "/";
                  rtmpUri += m_dispatchParam.webinar_id;

                  if (m_dispatchParam.ismix != 0) {
                     if (strcmp(m_dispatchParam.role, "host") == 0) {
                        rtmpUri += "@host@";
                     }
                     else {

                        rtmpUri += "@";
                        rtmpUri += m_dispatchParam.userId;
                        rtmpUri += "@";
                     }
                  }

                  rtmpUri += ".flv";
               }
               else {

                  if (m_dispatchParam.ismix != 0) {
                     TRACE6("OBSControlLogic::StartStream Network_to_connect\n");
                     //SingletonOBSControlIns::Instance().PostCRMessage(MSG_OBSCONTROL_STREAM_RESET, NULL, 0);
                     UploadPushStreamErrorToUI();
                     toStartRtmp = false;
                  }
                  rtmpUri += "&&token=";
                  rtmpUri += itera->token;
                  rtmpUri += "/";
                  rtmpUri += itera->streamname;
                  rtmpUri += ".flv";
               }

            }
            else {
               if (m_bDispatch) {
                  rtmpUri += "?vhost=";
                  rtmpUri += m_dispatchParam.vhost;
                  rtmpUri += "?token=";
                  rtmpUri += m_dispatchParam.token;
                  rtmpUri += "?webinar_id=";
                  rtmpUri += m_dispatchParam.webinar_id;
                  rtmpUri += "?ismix=";
                  rtmpUri += (m_dispatchParam.ismix == 0 ? "0" : "1");
                  rtmpUri += "?mixserver=";
                  rtmpUri += m_dispatchParam.mixserver;
                  rtmpUri += "?accesstoken=";
                  rtmpUri += m_dispatchParam.accesstoken;
                  rtmpUri += "/";
                  rtmpUri += m_dispatchParam.webinar_id;
                  if (m_dispatchParam.ismix != 0) {
                     if (strcmp(m_dispatchParam.role, "host") == 0) {
                        rtmpUri += "@host@";
                     }
                     else {

                        rtmpUri += "@";
                        rtmpUri += m_dispatchParam.userId;
                        rtmpUri += "@";
                     }
                  }
               }
               else {
                  if (m_dispatchParam.ismix != 0) {
                     TRACE6("OBSControlLogic::StartStream Network_to_connect\n");
                     //SingletonOBSControlIns::Instance().PostCRMessage(MSG_OBSCONTROL_STREAM_RESET, NULL, 0);
                     UploadPushStreamErrorToUI();
                     toStartRtmp = false;
                  }
                  rtmpUri += "?vhost=indexVhost&&";
                  rtmpUri += "token=";
                  rtmpUri += itera->token;
                  rtmpUri += "/";
                  rtmpUri += itera->streamname;
               }
            }


            if (toStartRtmp) {
               //h5 活动不支持线路切换.
               if (startLiveInfo.player == FLASH_LIVE_CREATE_TYPE || pCommonData->GetStartMode() == eStartMode_flash) {
                  TRACE6("OBSControlLogic::StartStream player == 1 [%s]\n", rtmpUri.c_str());
                  m_pMediaCore->SetUploadLogParam(0, startLiveInfo.user_id.toStdString());
                  ret = m_pMediaCore->StartRtmp(rtmpUri.c_str());
               }
               else {
                  int index = pushInfo.push_address.indexOf("/", 10);
                  QString h5_push_info = pushInfo.push_address.mid(index, pushInfo.push_address.length());
                  line_push_info = line_push_info.replace("/vhall", "");
                  QString pushUrlAddr = line_push_info;
                  if (line_push_info.contains("http")) {
                     h5_push_info = h5_push_info.replace("?", "&&");
                     pushUrlAddr = line_push_info + h5_push_info;
                     pushUrlAddr = pushUrlAddr + "/" + pushInfo.stream_number + ".flv";
                  }
                  else {
                     pushUrlAddr = pushUrlAddr + h5_push_info;
                     pushUrlAddr = pushUrlAddr + "/" + pushInfo.stream_number;
                  }
                  TRACE6("OBSControlLogic::StartStream h5 live push addr [%s]\n", pushUrlAddr.toStdString().c_str());
                  m_pMediaCore->SetUploadLogParam(1, startLiveInfo.user_id.toStdString());
                  ret = m_pMediaCore->StartRtmp(pushUrlAddr.toStdString().c_str());
               }
            }
            else {
               TRACE6("OBSControlLogic::StartStream [%s] donnot start\n", rtmpUri.c_str());
            }
         }
      }
      if (isNotify) {
         if (ret == true) {
            STRU_OBSCONTROL_STREAMSTATUS streamStatus;
            streamStatus.m_eType = stream_status_connect;
            streamStatus.m_iStreamCount = rInfos.size();
            SingletonOBSControlIns::Instance().PostCRMessage(MSG_OBSCONTROL_STREAMSTATUS_CHANGE, &streamStatus, sizeof(STRU_OBSCONTROL_STREAMSTATUS));
         }
      }
      m_bStartStream = true;

      VH::CComPtr<IMainUILogic> pMainUILogic = NULL;
      DEF_GET_INTERFACE_PTR(SingletonOBSControlIns, PID_IMainUI, IID_IMainUILogic, pMainUILogic);
      if (bIsNewRecord && conn && !pMainUILogic->IsStopRecord()) { //判断   当前状态 录制中 / 暂停录制
         startRecord();
      }
      else if (bIsNewRecord) {
         if (m_bIsSaveFile) {// 开始直播时 ， 是否自动开启录制  true 自动开启  false 不开启
            commitRecord(eRecordReTyp_Start);
         }
      }

      /////log
      wchar_t stream_add[DEF_MAX_HTTP_URL_LEN] = { 0 };//推流地址
      wchar_t stream_config_param[DEF_MAX_HTTP_URL_LEN] = { 0 };//推流参数
      m_pMediaCore->GetVideoParamInfo(stream_config_param);
      m_pMediaCore->SetOpenNoiseCancelling(bNoiseReduction);
      bool b_agent = ConfigSetting::ReadInt(qsConfPath, GROUP_DEFAULT, PROXY_OPEN, 0) != 0;
      bool b_record = m_bIsSaveFile;//是否开启本地录制
      bool b_dispatch = m_bDispatch;//是否使用调度

      QString::fromUtf8(QString::fromStdString(rtmpUri).toUtf8().toBase64()).toWCharArray(stream_add);
   }   
}

void OBSControlLogic::StopStream(bool isNotify, bool bIsNewRecord, bool conn /*= false*/) {
   TRACE6("%s bIsNewRecord:%d\n", __FUNCTION__, bIsNewRecord);
   if (!m_bStartStream) {
      return;
   }
   if (bIsNewRecord&&conn){//重新连接
      StopRecord(conn);
      TRACE6("%s StopRecord \n", __FUNCTION__);
   }
   //else if (bIsNewRecord) {
   //   commitRecord(eRecordReTyp_Stop);
   //   TRACE6("%s commitRecord \n", __FUNCTION__);
   //}
   TRACE6("%s StopRtmp \n", __FUNCTION__);
   if (m_pMediaCore) {
      m_pMediaCore->StopRtmp(NULL);
   }
   TRACE6("%s StopRtmp end\n", __FUNCTION__);
   m_bStartStream = false;
   STRU_OBSCONTROL_STREAMSTATUS streamStatus;
   streamStatus.m_eType = stream_status_disconnect;
   SingletonOBSControlIns::Instance().PostCRMessage(MSG_OBSCONTROL_STREAMSTATUS_CHANGE, &streamStatus, sizeof(STRU_OBSCONTROL_STREAMSTATUS));
   OnMediaReportLog("OK_PUBLISH_END");
   TRACE6("%s end\n", __FUNCTION__);
}

void OBSControlLogic::DealStartPublish(void* apData, DWORD adwLen) {
   DEF_CR_MESSAGE_DATA_DECLARE_CA(STRU_OBSCONTROL_PUBLISH, loMessage, return);
   TRACE6("%s \n", __FUNCTION__);
   if (NULL == m_pGraphics || NULL == m_pMediaCore) {
      ASSERT(FALSE);
      return;
   }

   if (!m_bIsAudioExist) {
      return;
   }

   if (loMessage.m_bInteractive) {
      return;
   }

   m_bIsSaveFile = loMessage.m_bIsSaveFile;
   wcscpy(m_wSaveFilePath, loMessage.m_wzSavePath);
   m_bDispatch = loMessage.m_bDispatch;
   m_dispatchParam = loMessage.m_bDispatchParam;
   TRACE6("%s \n", __FUNCTION__);
   //开始推流
   if (loMessage.m_bIsStartPublish) {
      TRACE6("%s StopStream start\n", __FUNCTION__);
      StopStream(false, false, loMessage.m_bMediaCoreReConnect);
      TRACE6("%s StopStream end\n", __FUNCTION__);
      StartStream(true, true, loMessage.m_bMediaCoreReConnect);
      TRACE6("%s StartStream end\n", __FUNCTION__);
   }
   else {
      //结束推流
      StopStream(true, false, false);
      TRACE6("%s StopStream end\n", __FUNCTION__);
   }
   TRACE6("%s return\n", __FUNCTION__);
}

void OBSControlLogic::startRecord()
{
   if (m_bStartStream) {
      if (mRecordEOF) {
         mRecordEOF = false;
         FileInfo fileInfo;
         //判断文件目录是否存在 不存在就创建
         if (QString::fromWCharArray(m_wSaveFilePath).trimmed().isEmpty()) {
            QString qsConfPath = CPathManager::GetConfigPath();
            QString defaultRecordPath = CPathManager::GetAppDataPath();
            ConfigSetting::writeValue(qsConfPath, GROUP_DEFAULT, KEY_RECORD_PATH, defaultRecordPath);
            defaultRecordPath.toWCharArray(m_wSaveFilePath);
         }

         if (CPathManager::isDirExist(QString::fromWCharArray(m_wSaveFilePath))) {
            wsprintf(fileInfo.filePath, L"%s%s", m_wSaveFilePath, GetRecoderFileName().c_str());
            QString qFilePath = QString::fromWCharArray(fileInfo.filePath);
            TRACE6("OBSControlLogic::startRecord m_bIsSaveFile = true,path = [%s]\n", qFilePath.toLocal8Bit().data());
            QString dirInfo = QString::fromWCharArray(m_wSaveFilePath);
            while (dirInfo.length()) {
               if (dirInfo.at(dirInfo.length() - 1) != '/') {
                  dirInfo = dirInfo.left(dirInfo.length() - 1);
               }
               else {
                  break;
               }
            }

            if (QDir(dirInfo).exists()) {
               TRACE6("OBSControlLogic::startRecord Dir exist will record\n");
               if (m_pMediaCore) {
                  m_pMediaCore->StartRecord(qFilePath.toLocal8Bit().data());
               }
               RECORD_STATE_CHANGE recordState;
               recordState.iState = eRecordState_Recording;
               SingletonOBSControlIns::Instance().PostCRMessage(MSG_MAINUI_RECORD_CHANGE, &recordState, sizeof(RECORD_STATE_CHANGE));
            }
            else {
               TRACE6("OBSControlLogic::SetRecordPath Dir not exist [%s]\n", dirInfo.toLocal8Bit().data());
               VH::CComPtr<IMainUILogic> pMainUILogic = NULL;
               DEF_GET_INTERFACE_PTR(SingletonOBSControlIns, PID_IMainUI, IID_IMainUILogic, pMainUILogic);
               if (pMainUILogic) {
                  pMainUILogic->FadoutTip(L"录制文件保存失败");
               }
            }
         }
         else {
            TRACE6("OBSControlLogic::SetRecordPath isDirExist Fail\n");
            VH::CComPtr<IMainUILogic> pMainUILogic = NULL;
            DEF_GET_INTERFACE_PTR(SingletonOBSControlIns, PID_IMainUI, IID_IMainUILogic, pMainUILogic);
            commitRecord(eRecordReTyp_Stop);
            if (pMainUILogic) {
               pMainUILogic->FadoutTip(L"视频保存路径不存在，请设置有效的录制视频保存地址后重试！");
            }
         }
      }
      else {
         recoveryRecord();
      }
   }
   else
   {
   }

}

void OBSControlLogic::GetMediaFileWidthAndHeight(const char* path, int &width, int& height) {
   if (m_pMediaCore) {
      m_pMediaCore->GetMediaFileWidthAndHeight(path, width, height);
   }
}

void OBSControlLogic::StopPushStream() {
   StopStream(true, false, false);
}

void OBSControlLogic::StopRecord(bool bCoercion/* = false*/, long liveTime /*= -1*/)
{
   if (!mRecordEOF)
   {
      if (/*eFilesNum_Only == mIRecordFileNum ||*/ bCoercion) {//网络异常
         //m_pMediaCore->SuspendRecord();
         //suspendRecord();
      }
      else {//生成多个文件
         mRecordEOF = true;
         if (m_pMediaCore) {
            m_pMediaCore->StopRecord();
         }
         RECORD_STATE_CHANGE recordState;
         recordState.iState = eRecordState_Stop;
         recordState.nLiveTime = liveTime;
         SingletonOBSControlIns::Instance().PostCRMessage(MSG_MAINUI_RECORD_CHANGE, &recordState, sizeof(RECORD_STATE_CHANGE));

      }
   }
}

void OBSControlLogic::suspendRecord()//暂停录制
{
   QString strSuspendTime = QDateTime::currentDateTime().toString(STR_DATETIME_Sec);
   TRACE6("OBSControlLogic::suspendRecord only One Files\n");
   if (m_pMediaCore) {
      m_pMediaCore->SuspendRecord();
   }
   RECORD_STATE_CHANGE recordState;
   recordState.iState = eRecordState_Suspend;
   SingletonOBSControlIns::Instance().PostCRMessage(MSG_MAINUI_RECORD_CHANGE, &recordState, sizeof(RECORD_STATE_CHANGE));

}

void OBSControlLogic::recoveryRecord()
{
   TRACE6("OBSControlLogic::recoveryRecord only One Files\n");
   if (m_pMediaCore) {
      m_pMediaCore->RecoveryRecord();
   }
   RECORD_STATE_CHANGE recordState;
   recordState.iState = eRecordState_Recording;
   SingletonOBSControlIns::Instance().PostCRMessage(MSG_MAINUI_RECORD_CHANGE, &recordState, sizeof(RECORD_STATE_CHANGE));
}

void OBSControlLogic::DealClickControl(void* apData, DWORD adwLen)
{
   if (!m_bStartStream) {
      return;
   }
   DEF_CR_MESSAGE_DATA_DECLARE_CA(STRU_MAINUI_CLICK_CONTROL, loMessage, return);

   switch (loMessage.m_eType) {
   case control_StartRecord: {//开始录制 /结束录制
      if (loMessage.m_dwExtraData) {//true开始录制
         commitRecord(eRecordReTyp_Start);
      }
      else {
         commitRecord(eRecordReTyp_Stop);
      }
      TRACE6("[BP] OBSControlLogic::DealClickControl control_StartRecord\n");
      break;
   }
   case control_RecordSuspendOrRecovery: { //暂停录制/恢复录制
      if (loMessage.m_dwExtraData) {//恢复录制
         commitRecord(eRecordReTyp_Start);
      }
      else {//暂停录制
         commitRecord(eRecordReTyp_Suspend);
      }
      TRACE6("[BP] OBSControlLogic::DealClickControl control_RecordSuspendOrRecovery\n");
      break;
   }
   default:
      break;
   }
}

void OBSControlLogic::DealEnableVt(void* apData, DWORD adwLen) {
   DEF_CR_MESSAGE_DATA_DECLARE_CA(STRU_VT_INFO, loMessage, return);
   if (m_pMediaCore) {
      wstring confPath = GetAppDataPath() + CONFIGPATH;
      QString qsConfPath = qsConfPath.fromStdWString(confPath);
      unsigned int audioBitRate = ConfigSetting::ReadInt(qsConfPath, GROUP_DEFAULT, KEY_AUDIO_KBPS, 96);
      if (audioBitRate < 8) {
         audioBitRate = 8;
      }
      else if (audioBitRate > 1024) {
         audioBitRate = 1024;
      }
      int iAudioSampleRate = ConfigSetting::ReadInt(qsConfPath, GROUP_DEFAULT, KEY_AUDIO_SAMPLERATE, 44100);
      if (!m_bStartStream) {
         m_pMediaCore->SetAudioParam(2, iAudioSampleRate, audioBitRate);
      }

      int vcodec_fps = DEFAULT_VCODEC_FPS;
      int vcodec_gop = DEFAULT_VCODEC_GOP;

      wstring confToolPath = GetAppDataPath() + VHALL_TOOL_CONFIG;
      QString toolConfigPath = qsConfPath.fromStdWString(confToolPath);

      vcodec_gop = ConfigSetting::ReadInt(toolConfigPath, GROUP_DEFAULT, KEY_VCODEC_GOP, DEFAULT_VCODEC_GOP);
      if (vcodec_gop < 2) {
         vcodec_gop = DEFAULT_VCODEC_GOP;
      }
      float mediaCoreGop = float(vcodec_gop) + 0.2;
      vcodec_fps = vcodec_fps <= 0 ? DEFAULT_VCODEC_FPS : vcodec_fps;

      VH::CComPtr<ICommonData> pCommonData;
      DEF_GET_INTERFACE_PTR(SingletonOBSControlIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
      OBSOutPutInfo outPutInfo;
      pCommonData->GetOutputInfo(&outPutInfo);

      VideoSceneType sceneType = SceneType_Unknown;

      m_pGraphics->GetSceneInfo(sceneType);
      RtmpInfo rtmpInfo;
      unsigned int videoBitRate = outPutInfo.videoBits;
      SIZE size = m_pGraphics->GetResolution();
      if (!m_bStartStream) {
         m_pMediaCore->SetVideoParam(mediaCoreGop, vcodec_fps, videoBitRate, size.cx, size.cy);
         TRACE6("OBSControlLogic::DealEnableVt %d  w %d  h %d\n", videoBitRate, size.cx, size.cy);
      }
      m_pMediaCore->StartVoiceTransition(loMessage.bEnable, loMessage.fontSize, loMessage.lan);
   }
}

void OBSControlLogic::DealTextControl(void* apData, DWORD adwLen) {
   DEF_CR_MESSAGE_DATA_DECLARE_CA(STRU_OBSCONTROL_TEXT, loMessage, return);
   if (NULL != m_pGraphics) {
      m_pGraphics->AddNewSource(SRC_TEXT, &loMessage);
   }
}

void OBSControlLogic::DealImageControl(void* apData, DWORD adwLen) {
   DEF_CR_MESSAGE_DATA_DECLARE_CA(STRU_OBSCONTROL_IMAGE, loMessage, return);
   if (NULL != m_pGraphics) {
      m_pGraphics->AddNewSource(SRC_PIC, &loMessage);
   }
}

void OBSControlLogic::DealDesktopShare(void* apData, DWORD adwLen) {
   if (NULL != m_pGraphics) {
      if (apData == NULL) {
         //VH::CComPtr<ISettingLogic> pSettingLogic;
         //DEF_GET_INTERFACE_PTR(SingletonOBSControlIns, PID_IMainUI, IID_ISettingLogic, pSettingLogic, return, ASSERT(FALSE));
         //if (pSettingLogic) {
         //   pSettingLogic->SetDesktopShareState(false);
         //}
         MediaStop();
         m_pGraphics->ClearSourceByType(SRC_MONITOR);
         LeaveSharedDesktop();
         if (m_pMediaCore) {
            m_pMediaCore->SetDesktopStatus(false);
         }
      }
      else {
         VH::CComPtr<IMainUILogic> pMainUILogic;
         DEF_GET_INTERFACE_PTR(SingletonOBSControlIns, PID_IMainUI, IID_IMainUILogic, pMainUILogic, return, ASSERT(FALSE));
         m_pGraphics->ClearSourceByType(SRC_MONITOR_AREA);
         m_pGraphics->AddNewSource(SRC_MONITOR, apData);
         SingletonOBSControlIns::Instance().PostCRMessage(MSG_MAINUI_HIDE_AREA, NULL, 0);
         VH::CComPtr<ISettingLogic> pSettingLogic;
         DEF_GET_INTERFACE_PTR(SingletonOBSControlIns, PID_IMainUI, IID_ISettingLogic, pSettingLogic, return, ASSERT(FALSE));
         if (pSettingLogic) {
            pSettingLogic->SetDesktopShareState(true);
         }
         if (m_pMediaCore) {
            DEF_CR_MESSAGE_DATA_DECLARE_CA(VHD_WindowInfo, loMessage, return);
            m_pMediaCore->SetDesktopStatus(true);
            m_pMediaCore->DesktopEnhanceSwitchOn(loMessage.isOpenEnhance);
         }
      }
   }

   //VH::CComPtr<ISettingLogic> pSettingLogic;
   //DEF_GET_INTERFACE_PTR(SingletonOBSControlIns, PID_IMainUI, IID_ISettingLogic, pSettingLogic, return, ASSERT(FALSE));
   //pSettingLogic->UnlockVideo(NULL);
}

void OBSControlLogic::DealAddWndSrc(void* apData, DWORD adwLen) {
   DEF_CR_MESSAGE_DATA_DECLARE_CA(STRU_OBSCONTROL_WINDOWSRC, loMessage, return);
   if (NULL != m_pGraphics) {
      m_pGraphics->AddNewSource(SRC_WINDOWS, &loMessage);
   }
}

void OBSControlLogic::DealRegionShare(void* apData, DWORD adwLen) {
   DEF_CR_MESSAGE_DATA_DECLARE_CA(STRU_OBSCONTROL_SHAREREGION, loMessage, return);
   if (NULL != m_pGraphics) {
      m_pGraphics->AddNewSource(SRC_MONITOR_AREA, &loMessage.m_rRegionRect);
   }
}

void OBSControlLogic::DealProcessSrc(void* apData, DWORD adwLen) {
   DEF_CR_MESSAGE_DATA_DECLARE_CA(STRU_OBSCONTROL_PROCESSSRC, loMessage, return);
   if (NULL != m_pGraphics) {
      SOURCE_TYPE type = (SOURCE_TYPE)m_pGraphics->GetCurrentItemType();
      int nRet = m_pGraphics->ProcessSource(loMessage.m_dwType);
      if (nRet == 1) {
         SingletonOBSControlIns::Instance().PostCRMessage(MSG_MAINUI_OBSCONTROL_PROCESSSRC, &type, sizeof(int));
      }
   }
}

void OBSControlLogic::DealAddCamera(void* apData, DWORD adwLen) {
   DEF_CR_MESSAGE_DATA_DECLARE_CA(STRU_OBSCONTROL_ADDCAMERA, loMessage, return);
   if (NULL != m_pGraphics) {
      m_pGraphics->AddNewSource(SRC_DSHOW_DEVICE, &loMessage);
   }
}

int OBSControlLogic::GetRendState(DeviceInfo info) {
   if (NULL != m_pGraphics) {
      return m_pGraphics->GetRenderState(info);
   }
   return 0;
}

void OBSControlLogic::DealVidioSet(void* apData, DWORD adwLen)
{
   DEF_CR_MESSAGE_DATA_DECLARE_CA(STRU_OBSCONTROL_VIDIO_SET, loMessage, return);
   //如果视频编码质量修改 先断开推流 修改值后 重新推流
   if (m_bStartStream && m_pMediaCore->GetHighCodec() != loMessage.high_codec_open) {
      bool toResetStream = m_bStartStream;
      VH::CComPtr<IMainUILogic> pMainUILogic = NULL;
      DEF_GET_INTERFACE_PTR(SingletonOBSControlIns, PID_IMainUI, IID_IMainUILogic, pMainUILogic);
      if (toResetStream) {
         StopStream(true, false, true);
      }

      if (toResetStream) {
         StartStream(true, false, true);
      }
   }

   ReloadDevice(loMessage.m_DeviceInfo);
}

//处理音频捕获
void OBSControlLogic::DealAudioCapture(void* apData, DWORD adwLen) {
   DEF_CR_MESSAGE_DATA_DECLARE_CA(STRU_OBSCONTROL_AUDIO_CAPTURE, loMessage, return);
   if (NULL != m_pAudioCapture && m_pMediaCore != NULL) {
      bool bChange = m_pMediaCore->IsSamplesBitsChanged(loMessage.iKbps);
      m_pMediaCore->SetOpenNoiseCancelling(loMessage.bNoiseReduction);
      //先停止停止采集
      if (bChange) {
         VH::CComPtr<IMainUILogic> pMainUILogic = NULL;
         DEF_GET_INTERFACE_PTR(SingletonOBSControlIns, PID_IMainUI, IID_IMainUILogic, pMainUILogic);

         bool toResetStream = m_bStartStream;
         if (toResetStream) {
            StopStream(true, !pMainUILogic->IsStopRecord(), true);
         }
         SetAudioCapture(false);
         HRESULT reuslt;
         m_pAudioCapture->SetSampleRateHz(loMessage.iAudioSampleRate);
         bool bRet = m_pAudioCapture->SetMicDeviceInfo(loMessage.info, loMessage.isNoise, loMessage.closeThreshold, loMessage.openThreshold, loMessage.iAudioSampleRate, reuslt);
         if (!bRet) {
            wchar_t *msg = L"音视频设备打开失败，请检查设备是否被占用或没有访问权限";
            SingletonOBSControlIns::Instance().PostCRMessage(MSG_OBSCONTROL_STREAM_NOTIFY, msg, (wcslen(msg) + 1) * sizeof(wchar_t));
         }
         m_pMediaCore->SetAudioParamReduction(loMessage.iAudioSampleRate, loMessage.iKbps, loMessage.bNoiseReduction);
         SetAudioCapture(true);

         VH::CComPtr<ICommonData> pCommonData;
         DEF_GET_INTERFACE_PTR(SingletonOBSControlIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
         OBSOutPutInfo outputInfo;
         pCommonData->GetOutputInfo(&outputInfo);
         if (toResetStream) {
            StartStream(true, !pMainUILogic->IsStopRecord(), true);
         }
      }
      else {
         HRESULT reuslt;
         bool bRet = m_pAudioCapture->SetMicDeviceInfo(loMessage.info, loMessage.isNoise, loMessage.closeThreshold, loMessage.openThreshold, loMessage.iAudioSampleRate,reuslt);
         if (!bRet) {
            wchar_t *msg = L"音视频设备打开失败，请检查设备是否被占用或没有访问权限";
            SingletonOBSControlIns::Instance().PostCRMessage(MSG_OBSCONTROL_STREAM_NOTIFY, msg, (wcslen(msg) + 1) * sizeof(wchar_t));
         }
      }

      m_pMediaCore->SetVolumeAmplificateSize(loMessage.fMicGain);
   }
}

void OBSControlLogic::OnMediaCoreLog(const char *log) {
   TRACE6(log);
}

void OBSControlLogic::OnMediaReportLog(const char *info) {
   static unsigned int lastTime = 0;
   unsigned int currentTime = QDateTime::currentDateTime().toTime_t();
   if (currentTime - lastTime > 5) {
      STRU_MAINUI_LOG log;
      QString str = QString(info);
      wchar_t strw[1024] = { 0 };
      str = str.replace(" ", "-");
      str.toWCharArray(strw);

      swprintf_s(log.m_wzRequestUrl, DEF_MAX_HTTP_URL_LEN, L"key=stream_error&info=%s", strw);
      SingletonOBSControlIns::Instance().PostCRMessage(MSG_MAINUI_LOG, &log, sizeof(STRU_MAINUI_LOG));
      lastTime = currentTime;
   }
}

void OBSControlLogic::DesktopEnhanceControlSwitch(bool on) {
   if (m_pMediaCore) {
      m_pMediaCore->DesktopEnhanceSwitchOn(on);
   }
}

void OBSControlLogic::OnNetworkEvent(const MediaCoreEvent& eventType, void *data /*= NULL*/) {
   switch (eventType) {
   case Network_rtmp_header_send_failed:
   case Network_video_header_send_failed:
   case Network_audio_header_send_failed:
   case Network_server_disconnect:
   case Network_connect_failed: {
      if (m_bStartStream) {
         wchar_t *msg = L"当前网络异常，正在尝试重新推流";
         SingletonOBSControlIns::Instance().PostCRMessage(MSG_OBSCONTROL_STREAM_NOTIFY, msg, (wcslen(msg) + 1) * sizeof(wchar_t));
      }
      break;
   }
   case Network_to_push_birate_down: {
      SingletonOBSControlIns::Instance().PostCRMessage(MSG_OBSCONTROL_PUSH_STREAM_BITRATE_DOWN, L"", 0);
      break;
   }
   case Network_to_connect:
      //TRACE6("OBSControlLogic::OnNetworkEvent Network_to_connect\n");
      //SingletonOBSControlIns::Instance().PostCRMessage(MSG_OBSCONTROL_STREAM_RESET, NULL, 0);
      UploadPushStreamErrorToUI();
      break;
   case Network_to_push_connect:
      TRACE6("OBSControlLogic::OnNetworkEvent Network_to_push_connect\n");
      SingletonOBSControlIns::Instance().PostCRMessage(MSG_OBSCONTROL_STREAM_PUSH_SUCCESS, NULL, 0);
      break;
   default:
      break;
   }
}

void OBSControlLogic::UploadPushStreamErrorToUI() {
   TRACE6("OBSControlLogic::OnNetworkEvent Network_to_connect\n");
   VH::CComPtr<IMainUILogic> pMainUILogic;
   DEF_GET_INTERFACE_PTR(SingletonOBSControlIns, PID_IMainUI, IID_IMainUILogic, pMainUILogic, return, ASSERT(FALSE));
   if (pMainUILogic) {
      TRACE6("OBSControlLogic::OnNetworkEvent OnRecvPushStreamErrorEvent\n");
      pMainUILogic->OnRecvPushStreamErrorEvent();
   }
}

bool OBSControlLogic::OBSSourceDeleteHook(void *sourceItem, void *param, SOURCE_TYPE type) {
   if (!param) {
      return true;
   }
   TRACE6("%s type:%d\n", __FUNCTION__, type);
   OBSControlLogic *_this = (OBSControlLogic *)param;
   return _this->OBSSourceDelete(sourceItem, type);
}
bool OBSControlLogic::OBSSourceDelete(void* sourceItem, SOURCE_TYPE type) {
   TRACE6("%s\n", __FUNCTION__);
   if (type == SRC_MEDIA_OUT && sourceItem != NULL) {
      if (m_pAudioItemVedioPlay) {
         TRACE6("%s delete SRC_MEDIA_OUT\n", __FUNCTION__);
         m_pAudioCapture->DelAudioSource(m_pAudioItemVedioPlay);
         m_pAudioItemVedioPlay = NULL;
      }
      else {
         ASSERT(TRUE);
      }

      m_pMediaItem = NULL;
      m_pAudioItemVedioPlay = NULL;
      TRACE6("%s MSG_MAINUI_VEDIOPLAY_STOPPLAY\n", __FUNCTION__);
      SingletonOBSControlIns::Instance().PostCRMessage(MSG_MAINUI_VEDIOPLAY_STOPPLAY, NULL, 0);
   }
   else if (type == SRC_MONITOR && sourceItem != NULL) {
      TRACE6("%s delete SRC_MONITOR\n", __FUNCTION__);
      LeaveSharedDesktop();

   }
   else if (type == SRC_MONITOR_AREA && sourceItem != NULL) {
      TRACE6("%s delete SRC_MONITOR_AREA\n", __FUNCTION__);
      SingletonOBSControlIns::Instance().PostCRMessage(MSG_MAINUI_HIDE_AREA, NULL, 0);
   }
   else if (type == SRC_DSHOW_DEVICE && sourceItem != NULL) {
      HWND hwnd = m_pGraphics->GetDeviceRenderHwnd(sourceItem);
      STRU_MAINUI_DELETECAMERA msg;
      msg.hwnd = hwnd;
      SingletonOBSControlIns::Instance().PostCRMessage(MSG_MAINUI_CAMERA_DELETE, &msg, sizeof(STRU_MAINUI_DELETECAMERA));
   }
   TRACE6("%s end\n", __FUNCTION__);
   return true;
}

std::wstring OBSControlLogic::GetRecoderFileName() {
   SYSTEMTIME st;
   GetLocalTime(&st);
   wchar_t flvFileName[512];
   swprintf_s(flvFileName, 512, L"_%u%02u%02u %02u-%02u-%02u 00-00-00.flv", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
   return wstring(flvFileName);
}

void OBSControlLogic::RefitItem() {
   if (NULL != m_pGraphics && NULL != m_pMediaItem) {
      m_pGraphics->SourceRefit(m_pMediaItem);
   }
   if (m_pGraphics) {
      m_pGraphics->UpdataSourceElement();
   }
}


void OBSControlLogic::InitMicVolumeCapture() {
   if (NULL == m_pAudioCapture) {
      return;
   }
   wstring confPath = GetAppDataPath() + CONFIGPATH;
   QString qsConfPath = qsConfPath.fromStdWString(confPath);

   if (m_pAudioCapture) {
      m_pAudioCapture->ClearRecodingSource();
   }

   void* pAudioSource = NULL;
   VH::CComPtr<IDeviceManager> pDeviceManager;
   DEF_GET_INTERFACE_PTR(SingletonOBSControlIns, PID_IOBSControl, IID_IDeviceManager, pDeviceManager, return, ASSERT(FALSE));

   DeviceList micList;
   pDeviceManager->GetMicDevices(micList);
   if (micList.size() != 0) {
      DeviceInfo device = micList.m_deviceList.front();
      QString audioConf = CPathManager::GetAudiodevicePath();
      QString b64Str = ConfigSetting::ReadString(audioConf, GROUP_DEFAULT, AUDIO_DEFAULT_DEVICE, "");
      if (b64Str != "") {
         QByteArray ba = QByteArray::fromBase64(b64Str.toLocal8Bit());
         if (ba.length() > 0) {
            DeviceInfo itemData = *(DeviceInfo *)ba.data();
            for (auto itor = micList.m_deviceList.begin();
               itor != micList.m_deviceList.end(); itor++) {
               if (*itor == itemData) {
                  device = itemData;
                  break;
               }
            }
         }
      }
      else{
         for (auto itor = micList.m_deviceList.begin(); itor != micList.m_deviceList.end(); itor++) {
            if (itor->m_sDeviceType == TYPE_DSHOW_AUDIO) {
               device = *itor;
               break;
            }
         }
      }

      bool isNoise = ConfigSetting::ReadInt(qsConfPath, GROUP_DEFAULT, KEY_AUDIO_ISNOISE, 0);
      int noiseValue = ConfigSetting::ReadInt(qsConfPath, GROUP_DEFAULT, KEY_AUDIO_NOISE_VALUE, 0);
      int openThreshold = (-96 + noiseValue + 6) < 0 ? (-96 + noiseValue + 6) : 0;
      int closeThreshold = (-96 + noiseValue) < 0 ? (-96 + noiseValue) : 0;

      int iKbps = ConfigSetting::ReadInt(qsConfPath, GROUP_DEFAULT, KEY_AUDIO_KBPS, 96);
      int iAudioSampleRate = ConfigSetting::ReadInt(qsConfPath, GROUP_DEFAULT, KEY_AUDIO_SAMPLERATE, 44100);
      TRACE6("SetMicDeviceInfo name:%s\n", QString::fromStdWString(device.m_sDeviceName).toStdString().c_str());
      HRESULT reuslt;
      bool bRet = m_pAudioCapture->SetMicDeviceInfo(device, isNoise, closeThreshold, openThreshold, iAudioSampleRate, reuslt);
      if (!bRet) {
         wchar_t *msg = L"音视频设备打开失败，请检查设备是否被占用或没有访问权限";
         SingletonOBSControlIns::Instance().PostCRMessage(MSG_OBSCONTROL_STREAM_NOTIFY, msg, (wcslen(msg) + 1) * sizeof(wchar_t));
      }
   }
}

void OBSControlLogic::UnitMicVolumeCapture() {
   if (m_pAudioCapture == NULL) {
      return;
   }
   m_pAudioCapture->ClearRecodingSource();
}

void OBSControlLogic::CreateGraphic() {
   VH::CComPtr<ICommonData> pCommonData;
   DEF_GET_INTERFACE_PTR(SingletonOBSControlIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
   wstring path = GetAppPath();
   //获取流信息
   PublishInfo loStreamInfo;
   pCommonData->GetStreamInfo(loStreamInfo);
   //获取信息
   OBSOutPutInfo outputInfo;
   pCommonData->GetOutputInfo(&outputInfo);
   wstring bgPath;
   wstring logoPath;
   QString toolConfPath = CPathManager::GetAppDataPath() + QString::fromStdWString(VHALL_TOOL_CONFIG);
   if (path.length()) {
      if (!loStreamInfo.m_bHideLogo) {//不隐藏logo
         int hideLogo = ConfigSetting::ReadInt(toolConfPath, GROUP_DEFAULT, KEY_VHALL_LOGO_HIDE, 0);
         if (hideLogo == 1) {
            bgPath = path + L"VhallLiveBGNoVhall.png";
            logoPath = path + L"VhalllogoNoVhall.png";
         }
         else {
            bgPath = path + L"VhallLiveBG.png";
            logoPath = path + L"Vhalllogo.png";
         }
         TRACE6((char*)bgPath.c_str());
      }
      else {
         bgPath = path + L"VhallLiveBGNoVhall.png";
         logoPath = path + L"VhalllogoNoVhall.png";
      }
      RECT sz = { 0, 0, outputInfo.m_baseSize.cx, outputInfo.m_baseSize.cy };
      if (m_pGraphics) {
         m_pGraphics->AddPersistentSource(bgPath.c_str(), sz, -1);
      }
      sz = getLogoRect(outputInfo.m_baseSize, DefaultBaseSize, LogoSize, DefaultZoomRatio);
      if (m_pGraphics) {
         m_pGraphics->AddPersistentSource(logoPath.c_str(), sz, 0);
      }
   }
}

//
void OBSControlLogic::EnterSharedDesktop() {

   TRACE6("%s \n", __FUNCTION__);
   VH::CComPtr<IMainUILogic> pMainUILogic;
   DEF_GET_INTERFACE_PTR(SingletonOBSControlIns, PID_IMainUI, IID_IMainUILogic, pMainUILogic, return, ASSERT(FALSE));
   pMainUILogic->SetDesktopSharingUIShow(true);

   BOOL bComposition;
   DwmIsCompositionEnabled(&bComposition);
   m_bIsAero = bComposition;

   if (m_bIsAero) {
      DwmEnableComposition(DWM_EC_DISABLECOMPOSITION);
   }
   TRACE6("%s end\n", __FUNCTION__);
}

void OBSControlLogic::LeaveSharedDesktop() {
   TRACE6("%s \n", __FUNCTION__);
   SingletonOBSControlIns::Instance().PostCRMessage(MSG_MAINUI_DELETE_MONITOR, NULL, 0);
   //VH::CComPtr<ISettingLogic> pSettingLogic;
   //DEF_GET_INTERFACE_PTR(SingletonOBSControlIns, PID_IMainUI, IID_ISettingLogic, pSettingLogic, return, ASSERT(FALSE));
   //pSettingLogic->UnlockVideo(NULL);
   if (m_bIsAero) {
      DwmEnableComposition(DWM_EC_ENABLECOMPOSITION);
   }
   TRACE6("%s end\n", __FUNCTION__);
}

void OBSControlLogic::SetForceMono(bool bForceMono) {//设置声道，默认单声道
   if (!m_pAudioCapture) {
      return;
   }
   m_pAudioCapture->SetForceMono(bForceMono);
}
bool OBSControlLogic::GetForceMono() {
   if (!m_pAudioCapture) {
      return false;
   }
   return m_pAudioCapture->GetForceMono();
}
bool OBSControlLogic::ResetPublishInfo(const char *currentUrl, const char *nextUrl) {
   if (!m_pMediaCore) {
      return false;
   }
   return m_pMediaCore->ResetPublishInfo(currentUrl, nextUrl);
}
int OBSControlLogic::GetSumSpeed() {
   if (!m_pMediaCore) {
      return 0;
   }

   return  m_pMediaCore->GetSumSpeed();
}
UINT64 OBSControlLogic::GetSendVideoFrameCount() {
   if (!m_pMediaCore) {
      return 0;
   }

   int sendFrameCount = 0;
   int count = m_pMediaCore->RtmpCount();
   for (int i = 0; i < count; i++) {
      int ccount = m_pMediaCore->GetSendVideoFrameCount(i);
      sendFrameCount += ccount;
   }

   return sendFrameCount;

}
void OBSControlLogic::ModifyAreaShared(int left, int top, int right, int bottom) {
   if (!m_pGraphics) {
      return;
   }
   m_pGraphics->ModifyAreaShared(left, top, right, bottom);
}

void STDMETHODCALLTYPE OBSControlLogic::ClearSourceByType(SOURCE_TYPE type) {
   if (m_pGraphics) {
      m_pGraphics->ClearSourceByType(type);
   }
}

void OBSControlLogic::ClearAllSource(bool all) {
   TRACE6("%s\n", __FUNCTION__);
   if (m_pGraphics) {
      m_pGraphics->ClearSourceByType(SRC_MONITOR_AREA);
      m_pGraphics->ClearSourceByType(SRC_MONITOR);
      if (all) {
         m_pGraphics->ClearSourceByType(SRC_DSHOW_DEVICE);
         m_pGraphics->ClearSourceByType(SRC_WINDOWS);
         m_pGraphics->ClearSourceByType(SRC_TEXT);
         m_pGraphics->ClearSourceByType(SRC_PIC);
         m_pGraphics->ClearSourceByType(SRC_MEDIA_OUT);
         m_pGraphics->ReinitMedia();
      }
   }
   VH::CComPtr<ISettingLogic> pSettingLogic;
   DEF_GET_INTERFACE_PTR(SingletonOBSControlIns, PID_IMainUI, IID_ISettingLogic, pSettingLogic, return, ASSERT(FALSE));
   if (pSettingLogic) {
      pSettingLogic->ResetLockVideo();
   }
}
void OBSControlLogic::DealAddCameraSync(void* apData, DWORD adwLen) {
   DealAddCamera(apData, adwLen);
}
void OBSControlLogic::PushAudioSegment(float *buffer, unsigned int numFrames, unsigned long long timestamp) {
   if (m_vhPlayback) {
      m_vhPlayback->PlayBack(buffer, numFrames);
   }
}

void OBSControlLogic::RateControlSwitch(bool on) {
   if (m_pMediaCore) {
      m_pMediaCore->RateControlSwitch(on);
   }

}

void OBSControlLogic::DoHideLogo(bool bHide) {
   if (m_pGraphics) {
      m_pGraphics->DoHideLogo(bHide);
   }
}

QDateTime STDMETHODCALLTYPE OBSControlLogic::GetStartStreamTime()
{
   return m_StartStreamTime;
}

void STDMETHODCALLTYPE OBSControlLogic::LivePushAmf0Msg(const char* data, int length) {
   if (m_pMediaCore) {
      m_pMediaCore->LivePushAmf0Msg(data, length);
   }
}

void OBSControlLogic::DealPushAMF0(void* apData, DWORD adwLen) {
   DEF_CR_MESSAGE_DATA_DECLARE_CA(PLUGIN_DATA, loMessage, return);
   if (m_pMediaCore) {
      m_pMediaCore->LivePushAmf0Msg(loMessage.data, loMessage.length);
      delete[](char*)(loMessage.data);
   }
}

void OBSControlLogic::OnMeidaTransition(const wchar_t *data, int length) {
   VOICE_TRANS_MSG msg(data, length);
   SingletonOBSControlIns::Instance().PostCRMessage(MSG_OBSCONTROL_VOICE_TRANSITION, &msg, sizeof(VOICE_TRANS_MSG));
}

void OBSControlLogic::HandleCloseAudioDev() {
   UnitMicVolumeCapture();
}

bool STDMETHODCALLTYPE OBSControlLogic::IsStartStream()
{
   return m_bStartStream;
}

void OBSControlLogic::commitRecord(const int eRequestType)
{
   if (eRecordReTyp_Start == eRequestType)
   {
      if (!CPathManager::isDirExist(QString::fromWCharArray(m_wSaveFilePath)))
      {
         TRACE6("OBSControlLogic::SetRecordPath isDirExist Fail\n");
         VH::CComPtr<IMainUILogic> pMainUILogic = NULL;
         DEF_GET_INTERFACE_PTR(SingletonOBSControlIns, PID_IMainUI, IID_IMainUILogic, pMainUILogic);
         if (pMainUILogic) {
            pMainUILogic->FadoutTip(L"视频保存路径不存在，请设置有效的录制视频保存地址后重试！");
         }
         return;
      }
   }
   VH::CComPtr<IMainUILogic> pMainUILogic = NULL;
   DEF_GET_INTERFACE_PTR(SingletonOBSControlIns, PID_IMainUI, IID_IMainUILogic, pMainUILogic);


   if (NULL != pMainUILogic && pMainUILogic->IsRecordBtnhide())
   {
      return;
   }

   static QString strURL = ConfigSetting::ReadString(CPathManager::GetToolConfigPath(), GROUP_DEFAULT, KEY_URL_DOMAIN, domainUrl);
   PublishInfo loStreamInfo;
   VH::CComPtr<ICommonData> pCommonData;
   DEF_GET_INTERFACE_PTR(SingletonOBSControlIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
   pCommonData->GetStreamInfo(loStreamInfo);

   wchar_t *format = L"%s/api/client/v1/clientapi/cut-record?webinar_id=%s&status=%d&send=0&token=%s";// + m_token";

   STRU_HTTPCENTER_HTTP_RS loRQ;

   QString strStreamName = QString(loStreamInfo.mStreamName.c_str());
   QString strToken;

   if (pMainUILogic) {
      char *pMsgToken = pMainUILogic->GetMsgToken();
      if (pMsgToken) {
         strToken = QString(pMsgToken);
         delete[]pMsgToken;
      }
   }

   swprintf_s(loRQ.m_wzRequestUrl, DEF_MAX_HTTP_URL_LEN, format,
      reinterpret_cast<const wchar_t *>(strURL.utf16()),
      //HLS_POINT_RECORD,
      reinterpret_cast<const wchar_t *>(strStreamName.utf16()),
      eRequestType,
      reinterpret_cast<const wchar_t *>(strToken.utf16())
   );
   OBSControlLogicPostMsg(MSG_MAINUI_HTTP_TASK, &loRQ, sizeof(loRQ));
}

void OBSControlLogic::DealPointRecordRs(void* apData, DWORD adwLen)
{
   DEF_CR_MESSAGE_DATA_DECLARE_CA(STRU_HTTPCENTER_HTTP_RS, loMessage, return);
   QString strReqUrl = QString::fromWCharArray(loMessage.m_wzRequestUrl);

   int iPos = strReqUrl.indexOf("cut-record");
   if (iPos >= 0) {//判断是否是录制请求
      QJsonObject jeson = CPathManager::GetJsonObjectFromString(QString::fromWCharArray(loMessage.m_wzUrlData));
      int iCode = jeson["code"].toInt();
      QString strMsg = jeson["msg"].toString();

      if (200 == iCode)
      {
         QJsonObject datajeson = jeson["data"].toObject();
         QString strSend = datajeson["send"].toString();
         if (0 == strSend.toInt())//  0 表示非互动/直播  1表示互动
         {
            int istatus = datajeson["status"].toString().toInt();
            switch (istatus)
            {
            case eRecordReTyp_Start: {
               startRecord();
               break;
            }
            case eRecordReTyp_Suspend: {
               suspendRecord();
               break;
            }
            case eRecordReTyp_Stop: {
               StopRecord();
               break;
            }
            default:
               break;
            }
         }
      }
      else
      {
         //提示网络异常 稍后重新
         VH::CComPtr<IMainUILogic> pMainUILogic = NULL;
         DEF_GET_INTERFACE_PTR(SingletonOBSControlIns, PID_IMainUI, IID_IMainUILogic, pMainUILogic);
         if (pMainUILogic) {
            wchar_t strw[1024] = { 0 };
            strMsg.toWCharArray(strw);
            pMainUILogic->FadoutTip(strw);
            TRACE6("%ws:%ws\n", strw, loMessage.m_wzRequestUrl);
         }
      }
   }
   else {

   }
}

void STDMETHODCALLTYPE OBSControlLogic::InitCapture() {
   QMutexLocker locker(&mMediaCoreMutex);
   //创建图形接口
   CreateGraphic();
   //初始化麦克风捕获
   InitMicVolumeCapture();
   if (m_pAudioCapture != NULL) {
      VH::CComPtr<IDeviceManager> pDeviceManager;
      DEF_GET_INTERFACE_PTR(SingletonOBSControlIns, PID_IOBSControl, IID_IDeviceManager, pDeviceManager, return, ASSERT(FALSE));
      QString strCurDevid = ConfigSetting::ReadString(CPathManager::GetConfigPath(), GROUP_DEFAULT, PLAYER_DEFAULT_DEVICE, "");
      //获取扬声器列表
      DeviceList deviceSpeakerList;
      pDeviceManager->GetSpeakerDevices(deviceSpeakerList);
      auto itor = deviceSpeakerList.begin();
      for (; itor != deviceSpeakerList.end(); itor++) {
         if (strCurDevid == QString::fromStdWString(itor->m_sDeviceID)) {
            m_pAudioCapture->InitPlaybackSource(itor->m_sDeviceID);
            return;
         }
      }
      m_pAudioCapture->InitPlaybackSource(NULL);
   }
}

bool STDMETHODCALLTYPE OBSControlLogic::InitPlayDevice(const wchar_t* devId) {
   TRACE6("InitPlayDevice\n");
   if (m_pAudioCapture != NULL) {
      m_pAudioCapture->InitPlaybackSource((wchar_t*)devId);
      TRACE6("InitPlayDevice end\n");
      return true;
   }
   TRACE6("InitPlayDevice end\n");
   return false;
}

RECT getLogoRect(SIZE gBaseSize, SIZE gDefaultSize, SIZE gDefaultLogoSize, double gDefaultZoomRatio) {
   RECT rc;
   double baseZoomRatio = (double)gBaseSize.cx / gDefaultSize.cx;
   SIZE sz = { (long)gDefaultLogoSize.cx * gDefaultZoomRatio * baseZoomRatio, (long)gDefaultLogoSize.cy * gDefaultZoomRatio * baseZoomRatio };

   int topY = 20, rightX;
   rightX = 20;
   rc = { gBaseSize.cx - sz.cx - rightX, topY, gBaseSize.cx - rightX, sz.cy + topY };
   return rc;
}