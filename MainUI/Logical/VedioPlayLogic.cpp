#include "StdAfx.h"
#include "VedioPlayLogic.h"
#include "VedioPlayUI.h"
#include "VideoChoiceUI.h"
#include "IOBSControlLogic.h"
#include "InteractAPIManager.h"
#include "Msg_MainUI.h"
#include "Msg_OBSControl.h"
#include "ICommonData.h"
#include <QFileDialog>
#include <QDebug>

#define PLAYUI_PLAYSTATE
#define PLAYUI_PLAYSTATE_STOPPING  -2
#define PLAYUI_PLAYSTATE_FAIL      -1
#define PLAYUI_PLAYSTATE_NOTHING   0
#define PLAYUI_PLAYSTATE_OPENING   1
#define PLAYUI_PLAYSTATE_BUFFERING 2
#define PLAYUI_PLAYSTATE_PALYERING 3
#define PLAYUI_PLAYSTATE_PAUSED    4
#define PLAYUI_PLAYSTATE_STOP      5
#define PLAYUI_PLAYSTATE_END       6
#define PLAYUI_PLAYSTATE_ERROR     7


VedioPlayLogic::VedioPlayLogic(void)
   : m_lRefCount(1)
   , m_pVedioPlayUI(NULL)
   , m_bShowState(FALSE) {
   m_bForceHide = false;
   mbIsEnablePlayOutAudio = false;
}

VedioPlayLogic::~VedioPlayLogic(void) {
   TRACE6("%s delete end\n", __FUNCTION__);
}

BOOL VedioPlayLogic::Create() {
   do {
      VH::CComPtr<IMainUILogic> pMainUILogic;
      DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IMainUI, IID_IMainUILogic, pMainUILogic, return FALSE, ASSERT(FALSE));
      QWidget* pMainUI = NULL;
      pMainUILogic->GetMainUIWidget((void**)&pMainUI);
      m_pVideoChoiceUI = new VideoChoiceUI((QDialog*)pMainUI);
      if (NULL == m_pVideoChoiceUI || !m_pVideoChoiceUI->Create()) {
         ASSERT(FALSE);
         break;
      }
      mbIsEnablePlayOutAudio = false;
      return TRUE;
   } while (FALSE);
   return FALSE;
}

void VedioPlayLogic::Destroy() {
   //销毁插播窗口
   if (NULL != m_pVedioPlayUI) {
      m_pVedioPlayUI->Destroy();
      delete m_pVedioPlayUI;
      m_pVedioPlayUI = NULL;
   }

   //destory video choice ui
   if (NULL != m_pVideoChoiceUI) {
      m_pVideoChoiceUI->Destroy();
      delete m_pVideoChoiceUI;
      m_pVideoChoiceUI = NULL;
   }
   TRACE6("%s delete end\n", __FUNCTION__);
}

HRESULT STDMETHODCALLTYPE VedioPlayLogic::QueryInterface(REFIID riid, void ** appvObject) {
   if (riid == IID_VHIUnknown) {
      *appvObject = (VH_IUnknown*)this;
      AddRef();
      return CRE_OK;
   }
   else if (IID_IVedioPlayLogic == riid) {
      *appvObject = (IVedioPlayLogic*)this;
      AddRef();
      return CRE_OK;
   }
   return CRE_NOINTERFACE;
}

ULONG STDMETHODCALLTYPE VedioPlayLogic::AddRef(void) {
   return ::InterlockedIncrement(&m_lRefCount);
}

ULONG STDMETHODCALLTYPE VedioPlayLogic::Release(void) {
   return ::InterlockedDecrement(&m_lRefCount);
}

void VedioPlayLogic::DealMessage(DWORD adwMessageID, void* apData, DWORD adwLen) {
   switch (adwMessageID) {
      //CLICK控制
   case MSG_MAINUI_CLICK_CONTROL:
      DealClickControl(apData, adwLen);
      break;
      //插播播放
   case MSG_MAINUI_VEDIOPLAY_PLAY:
      DealVedioPlayClick(apData, adwLen);
      break;
      //添加文件
   case MSG_MAINUI_VEDIOPLAY_ADDFILE:
      DealAddFiles(apData, adwLen);
      break;
      //停止插播
   case MSG_MAINUI_VEDIOPLAY_STOPPLAY:
      DealStopPlay(apData, adwLen);
      break;
      //播放列表改变
   case MSG_MAINUI_MAINUI_PLAYLISTCHG:
      DealPlayListChg(apData, adwLen);
      break;
      //音量改变
   case MSG_MAINUI_VOLUME_CHANGE:
      DealVolumeChange(apData, adwLen);
      break;
   default:
      break;
   }
}

void VedioPlayLogic::DealClickControl(void* apData, DWORD adwLen) {
   DEF_CR_MESSAGE_DATA_DECLARE_CA(STRU_MAINUI_CLICK_CONTROL, loMessage, return);
   switch (loMessage.m_eType) {
      //插播
   case control_VideoSrc: {
      VH::CComPtr<IOBSControlLogic> pOBSControlLogic;
      DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IOBSControl, IID_IOBSControlLogic, pOBSControlLogic, return, ASSERT(FALSE));
      if (pOBSControlLogic->IsHasMonitorSource()) {
         return;
      }
      if (mLiveType != (eLiveType)loMessage.m_dwExtraData) {
         StopPlayFile();
      }
      VH::CComPtr<ICommonData> pCommonData;
      DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
      ClientApiInitResp initApiResp;
      pCommonData->GetInitApiResp(initApiResp);
      mLiveType = initApiResp.selectLiveType;
      CreatePlayMediaFileUI();
      ForceHide(false);
      int nRet = ShowVedioPlayWidget(TRUE);
      if (loMessage.m_bIsPlayEnd && QDialog::Rejected == nRet) {
         SingletonMainUIIns::Instance().PostCRMessage(MSG_MAINUI_VEDIOPLAY_STOPPLAY, NULL, 0);
      }
      TRACE6("[BP] VedioPlayLogic::DealClickControl control_VideoSrc\n");
      break;
   }
   default:
      break;
   }
}

void VedioPlayLogic::DealVedioPlayClick(void* apData, DWORD adwLen) {
   DEF_CR_MESSAGE_DATA_DECLARE_CA(STRU_MAINUI_PLAY_CLICK, loMessage, return);
   switch (mLiveType) {
   case eLiveType_Live: {
      HandlePlayLiveMediaFile();
      break;
   }
   case eLiveType_VhallActive: {
      HandlePlayVhallActiveMediaFile();
      break;
   }
   default:
      break;
   }
}

void VedioPlayLogic::DealStopPlay(void* apData, DWORD adwLen) {
   TRACE6("%s   \n", __FUNCTION__);
   //关闭循环播放定时器
   m_qCycleTimer.stop();
   //隐藏UI
   ShowVedioPlayWidget(FALSE);
   ForceHide(true);
   m_bPlayCurrentFile = true;
   switch (mLiveType) {
   case eLiveType_Live: {
      //停止插播视频
      VH::CComPtr<IOBSControlLogic> pOBSControlLogic;
      DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IOBSControl, IID_IOBSControlLogic, pOBSControlLogic, return, ASSERT(FALSE));
      pOBSControlLogic->MediaStop();
      InteractAPIManager apiManager(this);
      apiManager.HttpSendChangeWebWatchLayout(0);
      break;
   }
   case eLiveType_VhallActive: {
      VH::CComPtr<IMainUILogic> pMainUILogic;
      DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IMainUI, IID_IMainUILogic, pMainUILogic, return, ASSERT(FALSE));
      pMainUILogic->ClosePlayMediaFile(eLiveType_VhallActive);
      break;
   }
   default:
      break;
   }
}

void VedioPlayLogic::DealAddFiles(void* apData, DWORD adwLen) {
   if (!m_pVedioPlayUI) {
      return;
   }

   if (!m_pVideoChoiceUI) {
      return;
   }

   int currentIndex = 0;
   QStringList playList = m_pVedioPlayUI->GetPlayList(currentIndex);
   int circleMode = (int)m_pVedioPlayUI->GetCurPlayMode();
   VH::CComPtr<IMainUILogic> pMainUILogic;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IMainUI, IID_IMainUILogic, pMainUILogic, return, ASSERT(FALSE));
   QWidget* pMainUI = NULL;
   pMainUILogic->GetMainUIWidget((void**)&pMainUI);
   m_pVideoChoiceUI->Clear();
   m_pVideoChoiceUI->AppendFileList(playList, currentIndex);
   m_pVideoChoiceUI->SetCircleMode(circleMode);
   m_pVideoChoiceUI->FlashTips();
   m_pVideoChoiceUI->CenterWindow(pMainUI);

   m_pVedioPlayUI->raise();
   m_pVideoChoiceUI->WillShow(this);
   IsEnableShowPlayOutChoise();

   if (m_pVideoChoiceUI->exec() == QDialog::Accepted) {
      int currentIndex = 0;
      QStringList playList = m_pVideoChoiceUI->GetPlayList(currentIndex);
      int circleMode = m_pVideoChoiceUI->getCircleMode();
      if (playList.count() > 0) {
         m_pVedioPlayUI->SetCurPlayMode((CycleType)circleMode);
         m_pVedioPlayUI->AppendPlayList(playList, currentIndex);
      }
      if (!m_qCycleTimer.isActive()) {
         m_qCycleTimer.start(400);
      }
      m_pVideoChoiceUI->FLushFileList(true);
      mbIsEnablePlayOutAudio = m_pVideoChoiceUI->IsEnablePlayOutAudio();
   }
   else {
      m_pVideoChoiceUI->FLushFileList(false);
   }

   m_pVedioPlayUI->SlotSetForceActice(true);
   m_pVedioPlayUI->TryHideToolBar(2000);
}

void VedioPlayLogic::DealPlayListChg(void* apData, DWORD adwLen) {
   PlaySelectedFile();
}

int VedioPlayLogic::ShowVedioPlayWidget(BOOL bShow) {
   int nRet = QDialog::Rejected;
   TRACE6("%s  bShow %d \n", __FUNCTION__, bShow);
   if (NULL != m_pVedioPlayUI) {
      if (bShow) {
         int currentIndex = 0;
         QStringList fileList = m_pVedioPlayUI->GetPlayList(currentIndex);
         m_pVideoChoiceUI->Clear();
         m_pVideoChoiceUI->AppendFileList(fileList, currentIndex);
         VH::CComPtr<IMainUILogic> pMainUILogic;
         DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IMainUI, IID_IMainUILogic, pMainUILogic, return 0, ASSERT(FALSE));
         QWidget* pMainUI = NULL;
         pMainUILogic->GetMainUIWidget((void**)&pMainUI);
         bool isVedioPlayUIShow = false;
         isVedioPlayUIShow = m_pVedioPlayUI->IsShow();
         CycleType currentMode = m_pVedioPlayUI->GetCurPlayMode();
         m_pVideoChoiceUI->SetCircleMode((int)currentMode);
         m_pVideoChoiceUI->FlashTips();
         m_pVideoChoiceUI->CenterWindow(pMainUI);
         m_pVideoChoiceUI->SetLiveType(mLiveType);          

         IsEnableShowPlayOutChoise();
         VHDialog::WillShow(m_pVideoChoiceUI);

         if (m_pVideoChoiceUI->exec() == QDialog::Accepted) {
            nRet = QDialog::Accepted;
            int currentIndex = 0;
            QStringList playList = m_pVideoChoiceUI->GetPlayList(currentIndex);
            int circleMode = m_pVideoChoiceUI->getCircleMode();
            if (playList.count() > 0) {
               m_pVedioPlayUI->SetCurPlayMode((CycleType)circleMode);
               m_pVedioPlayUI->AppendPlayList(playList, currentIndex);
               m_pVedioPlayUI->show();
               m_bShowState = bShow;
               isVedioPlayUIShow = true;
               ReposVedioPlay(/*true*/);
            }
            if (!m_qCycleTimer.isActive()) {
               m_qCycleTimer.start(400);
            }
            m_pVideoChoiceUI->FLushFileList(true);
            mbIsEnablePlayOutAudio = m_pVideoChoiceUI->IsEnablePlayOutAudio();
         }
         else {
            int currentIndex = 0;
            QStringList playList = m_pVideoChoiceUI->GetPlayList(currentIndex);
            if (playList.count() == 0) {
               DealStopPlay(NULL, 0);
            }
            m_pVideoChoiceUI->FLushFileList(false);
         }
         if (isVedioPlayUIShow) {
            m_pVedioPlayUI->SlotSetForceActice(true);
            m_pVedioPlayUI->TryHideToolBar();
         }
      }
      else {
         m_pVedioPlayUI->hide();
         m_pVedioPlayUI->StopAdmin();
         m_bShowState = bShow;
         mbIsPlayEnd = true;
      }
   }
   return nRet;
}

void VedioPlayLogic::AddPlayFiles() {
   if (NULL == m_pVedioPlayUI) {
      return;
   }

   QString selectFileName;
   TCHAR szPathName[2048];
   OPENFILENAME ofn = { sizeof(OPENFILENAME) };
   ofn.hwndOwner = NULL;// 打开OR保存文件对话框的父窗口
   //过滤器 如果为 NULL 不使用过滤器
   //具体用法看上面  注意 /0

   if (mLiveType == eLiveType_Live) {
      ofn.lpstrFilter = TEXT("文件(*.mp3 *.wav *.mp4 *.flv *.mov *.rmvb)\0*.mp3;*.wav;*.mp4;*.flv;*.mov;*.rmvb\0\0");
   }
   else if (mLiveType == eLiveType_VhallActive) {
      ofn.lpstrFilter = TEXT("文件(*.mp4 *.flv *.mov *.rmvb)\0*.mp4;*.flv;*.mov;*.rmvb\0\0");
   }
   lstrcpy(szPathName, TEXT(""));
   ofn.lpstrFile = szPathName;
   ofn.nMaxFile = sizeof(szPathName);//存放用户选择文件的 路径及文件名 缓冲区
   ofn.lpstrTitle = TEXT("选择文件");//选择文件对话框标题
   ofn.Flags = OFN_EXPLORER;//如果需要选择多个文件 则必须带有  OFN_ALLOWMULTISELECT标志
   BOOL bOk = GetOpenFileName(&ofn);//调用对话框打开文件
   TRACE6("%s GetOpenFileName\n", __FUNCTION__);
   if (bOk) {
      selectFileName = QString::fromStdWString(szPathName);
   }
   foreach(QString filename, selectFileName) {
      m_pVedioPlayUI->AddPlayList((char*)filename.toStdString().c_str());
   }
}

void VedioPlayLogic::PlaySelectedFile() {
   m_bPlayCurrentFile = true;
}

//时间戳转换
QString VedioPlayLogic::dulation2StrPri(long long dulation) {
   dulation /= 1000;
   int H = (dulation / 3600) % 60;
   int M = (dulation / 60) % 60;
   int S = dulation % 60;

   QString sH = QString::number(H);
   if (H < 10) {
      sH = "0" + sH;
   }
   QString sM = QString::number(M);
   if (M < 10) {
      sM = "0" + sM;
   }
   QString sS = QString::number(S);
   if (S < 10) {
      sS = "0" + sS;
   }
   return QString("%1:%2:%3").arg(sH, sM, sS);
}

QString VedioPlayLogic::dulation2Str(long long currDulation, long long maxDulation) {
   QString dulationStr = "";
   if (currDulation > maxDulation) {
      currDulation = maxDulation;
   }

   dulationStr = dulation2StrPri(currDulation) + "/" + dulation2StrPri(maxDulation);
   return dulationStr;
}

void VedioPlayLogic::OnCyclePlay() {
   switch (mLiveType) {
   case eLiveType_Live:
      HandleLiveCyclePlay();
      break;
   case eLiveType_VhallActive: {
      HandleVhallActiveCyclePlay();
      break;
   }
   default:
      break;
   }
}

void VedioPlayLogic::SlotRepos(bool bShow) {
   ReposVedioPlay(/*bShow*/);
}

void VedioPlayLogic::SlotForceHide(bool bHide) {
   ForceHide(bHide);
}

bool STDMETHODCALLTYPE VedioPlayLogic::ShowPlayUI(bool show) {
   if (m_pVedioPlayUI) {
      if (show) {
         m_pVedioPlayUI->TryShowToolBar();
      }
      else {
         m_pVedioPlayUI->TryHideToolBar();
      }
   }
   return true;
}

void VedioPlayLogic::ForceHide(bool bHide) {
   if (m_pVedioPlayUI == NULL) {
      return;
   }
   m_bForceHide = bHide;
   ReposVedioPlay(/*!bHide*/);
   if (bHide) {
      m_pVedioPlayUI->StopAdminIMMediately(false);
   }
}
void VedioPlayLogic::StopAdmin(bool bForceHideAdmin) {
   if (!m_pVedioPlayUI) {
      return;
   }
   m_pVedioPlayUI->StopAdminIMMediately(bForceHideAdmin);
}

void VedioPlayLogic::ReposVedioPlay(/*bool bShow*/) {
   int left = 0;
   int right = 0;
   if (!m_pVedioPlayUI) {
      return;
   }

   VH::CComPtr<IMainUILogic> pMainUILogic;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IMainUI, IID_IMainUILogic, pMainUILogic, return, ASSERT(FALSE));
   if (pMainUILogic) {
      QWidget* pMainUI = NULL;
      QWidget* pContentUI = NULL;
      pMainUILogic->GetMainUIWidget((void**)&pMainUI, mLiveType);
      pMainUILogic->GetLiveActiveMediaPlayBasePos((void**)&pContentUI, mLiveType);
      if (!pContentUI || !pMainUI) {
         return;
      }

      if (mLiveType == eLiveType_Live) {
         QPoint pos = pMainUI->mapToGlobal(pContentUI->pos());
         int x = pos.x()/* + 60*/;
         int y = pos.y() + pContentUI->height() /*+ 41*/ - m_pVedioPlayUI->height();
         m_pVedioPlayUI->setFixedWidth(pContentUI->width());
         m_pVedioPlayUI->move(x, y);
      }
      else if (mLiveType == eLiveType_VhallActive) {
         QPoint pos = pMainUI->mapToGlobal(pContentUI->pos());
         int x = pos.x() + 60;
         int y = pos.y() + pContentUI->height() - 27 - m_pVedioPlayUI->height();
         m_pVedioPlayUI->setFixedWidth(pContentUI->width() - 60);
         m_pVedioPlayUI->move(x, y);
      }
   }
}

void VedioPlayLogic::DealVolumeChange(void* apData, DWORD adwLen) {
   DEF_CR_MESSAGE_DATA_DECLARE_CA(STRU_MAINUI_VOLUME_CHANGE, loMessage, return);
   switch (loMessage.m_eType) {
   case change_VedioPlay: {
      if (mLiveType == eLiveType_Live) {
         VH::CComPtr<IOBSControlLogic> pOBSControlLogic;
         DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IOBSControl, IID_IOBSControlLogic, pOBSControlLogic, return, ASSERT(FALSE));
         pOBSControlLogic->SetVolume(loMessage.m_nVolume);
      }
      else if (eLiveType_VhallActive) {
         VH::CComPtr<IMainUILogic> pMainUILogic;
         DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IMainUI, IID_IMainUILogic, pMainUILogic, return, ASSERT(FALSE));
         if (pMainUILogic) {
            pMainUILogic->SetPlayMediaFileVolume(loMessage.m_nVolume, eLiveType_VhallActive);
         }
      }
      break;
   }                          
   default:
      break;
   }
}

void VedioPlayLogic::StopPlayFile() {
   TRACE6("%s \n", __FUNCTION__);
   m_qCycleTimer.stop();
   if (m_pVedioPlayUI != NULL) {
      disconnect(m_pVedioPlayUI, SIGNAL(SigRepos(bool)), this, SLOT(SlotRepos(bool)));
      disconnect(m_pVedioPlayUI, SIGNAL(SigForceHide(bool)), this, SLOT(SlotForceHide(bool)));
      disconnect(&m_qCycleTimer, SIGNAL(timeout()), this, SLOT(OnCyclePlay()));
      disconnect(m_pVideoChoiceUI, SIGNAL(SigSyncFileList(QStringList, int)), m_pVedioPlayUI, SLOT(SlotFlushPlayList(QStringList, int)));
      StopAdmin(true);
      ForceHide(true);
      VH::CComPtr<IMainUILogic> pMainUILogic;
      DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IMainUI, IID_IMainUILogic, pMainUILogic, return, ASSERT(FALSE));
      if (pMainUILogic) {
         pMainUILogic->SetVedioPlayUi(NULL, mLiveType);
      }
      m_pVedioPlayUI->Destroy();
      delete m_pVedioPlayUI;
      m_pVedioPlayUI = NULL;
      mLiveType = -1;
   }
   TRACE6("%s \n", __FUNCTION__);
}

void STDMETHODCALLTYPE VedioPlayLogic::CloseFileChoiceUI() {
   if (m_pVideoChoiceUI) {
      m_pVideoChoiceUI->reject();
   }
}

void STDMETHODCALLTYPE VedioPlayLogic::ResetPlayUiSize(int width) {
   if (m_pVedioPlayUI) {
      m_pVedioPlayUI->setFixedWidth(width);
   }
}

void VedioPlayLogic::HandlePlayLiveMediaFile() {
   VH::CComPtr<IOBSControlLogic> pOBSControlLogic;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IOBSControl, IID_IOBSControlLogic, pOBSControlLogic, return, ASSERT(FALSE));
   int nState = pOBSControlLogic->GetPlayerState();
   if (nState == PLAYUI_PLAYSTATE_PAUSED) {
      pOBSControlLogic->MediaResume();
   }
   else if (nState == PLAYUI_PLAYSTATE_PALYERING) {
      pOBSControlLogic->MediaPause();
   }
   else {
      QString fileName;
      if (m_pVedioPlayUI == NULL) {
         return;
      }
      bool bCanPlay = m_pVedioPlayUI->GetCurFile(fileName);
      unsigned int iVolume = m_pVedioPlayUI->GetCurVolume();
      if (bCanPlay) {
         bool audioFile = false;
         if (fileName.contains(".wav") || fileName.contains(".WAV") || fileName.contains(".mp3") || fileName.contains(".MP3")) {
            audioFile = true;
         }
         bool bRet = pOBSControlLogic->MediaPlay((char*)fileName.toStdString().c_str(), audioFile);
         m_pVedioPlayUI->TryHideToolBar(5);
         mbIsPlayEnd = true;
         InteractAPIManager apiManager(this);
         apiManager.HttpSendChangeWebWatchLayout(1);
         TRACE6("%s playfile:%ws\n", __FUNCTION__, fileName.toStdWString().c_str());
         pOBSControlLogic->SetVolume(iVolume);
      }
      else {
         AddPlayFiles();
         PlaySelectedFile();
      }
   }
}

void VedioPlayLogic::HandlePlayVhallActiveMediaFile() {
   VH::CComPtr<IMainUILogic> pMainUILogic;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IMainUI, IID_IMainUILogic, pMainUILogic, return, ASSERT(FALSE));
   int nState = pMainUILogic->GetPlayFileState(eLiveType_VhallActive);
   if (nState == PLAYUI_PLAYSTATE_PAUSED) {
      pMainUILogic->ResumePlayMediaFile(eLiveType_VhallActive);
   }
   else if (nState == PLAYUI_PLAYSTATE_PALYERING) {
      pMainUILogic->PausePlayMediaFile(eLiveType_VhallActive);
   }
   else {
      QString fileName;
      if (m_pVedioPlayUI == NULL) {
         return;
      }
      bool bCanPlay = m_pVedioPlayUI->GetCurFile(fileName);
      unsigned int iVolume = m_pVedioPlayUI->GetCurVolume();
      if (bCanPlay) {
         TRACE6("%s playfile:%ws\n", __FUNCTION__, fileName.toStdWString().c_str());
         if (pMainUILogic->OpenPlayMediaFile((char*)fileName.toStdString().c_str(), eLiveType_VhallActive)) {
            pMainUILogic->SetPlayMediaFileVolume(iVolume, eLiveType_VhallActive);
         }
      }
      else {
         AddPlayFiles();
         PlaySelectedFile();
      }
   }
}

void VedioPlayLogic::HandleLiveCyclePlay() {
   if (!m_pVedioPlayUI) {
      return;
   }

   VH::CComPtr<IOBSControlLogic> pOBSControlLogic;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IOBSControl, IID_IOBSControlLogic, pOBSControlLogic, return, ASSERT(FALSE));
   int nState = pOBSControlLogic->GetPlayerState();
   long dulation = pOBSControlLogic->VhallGetCurrentDulation();
   long maxDulation = pOBSControlLogic->VhallGetMaxDulation();

   QString dulationStr = dulation2Str(dulation, maxDulation);
   if (nState == PLAYUI_PLAYSTATE_STOP ||
      nState == PLAYUI_PLAYSTATE_END ||
      nState == PLAYUI_PLAYSTATE_FAIL) {
      QString dulationStr = dulation2Str(0, 0);
      m_pVedioPlayUI->UpdatePlayTime(dulationStr);
      m_pVedioPlayUI->UpdatePlayStates(FALSE);
      m_pVedioPlayUI->SetProgressValue(0, 0);
   }
   else if (nState == PLAYUI_PLAYSTATE_PAUSED) {
      m_pVedioPlayUI->UpdatePlayTime(dulationStr);
      m_pVedioPlayUI->UpdatePlayStates(FALSE);
      m_pVedioPlayUI->SetProgressValue(dulation, maxDulation);
   }
   else if (PLAYUI_PLAYSTATE_STOPPING == nState) {
      m_pVedioPlayUI->UpdatePlayTime(dulationStr);
      m_pVedioPlayUI->UpdatePlayStates(FALSE);
      m_pVedioPlayUI->SetProgressValue(dulation, maxDulation);
   }
   else {
      m_pVedioPlayUI->UpdatePlayTime(dulationStr);
      m_pVedioPlayUI->UpdatePlayStates(TRUE);
      m_pVedioPlayUI->SetProgressValue(dulation, maxDulation);
   }

   switch (nState) {
   case PLAYUI_PLAYSTATE_STOPPING:
   case PLAYUI_PLAYSTATE_OPENING:
   case PLAYUI_PLAYSTATE_BUFFERING:
      m_pVedioPlayUI->Enabled(false);
      break;
   default:
      m_pVedioPlayUI->Enabled(true);
      break;
   }

   if (nState == PLAYUI_PLAYSTATE_END) {
      switch (m_pVedioPlayUI->GetCurPlayMode()) {
         //顺序播放
      case CycleTyle_None:
         if (!m_pVedioPlayUI->IsArriveTheEnd()) {
            m_pVedioPlayUI->SwitchToNext();
            PlaySelectedFile();
         }
         else {
            if (mbIsPlayEnd && m_pVideoChoiceUI && m_pVideoChoiceUI->isHidden() && m_pVedioPlayUI && !m_pVedioPlayUI->isHidden()) {
               mbIsPlayEnd = false;
               SingletonMainUIIns::Instance().PostCRMessage(MSG_MAINUI_VEDIOPLAY_STOPPLAY, NULL, 0);
            }
         }
         break;
      case CycleTyle_List:       //列表循环
      {
         m_pVedioPlayUI->SwitchToNext();
         PlaySelectedFile();
      }
      break;
      case CycleTyle_Single:     //单视频循环
      {
         PlaySelectedFile();
      }
      break;
      case CycleTyle_END:
         break;
      default:
         break;
      }
   }

   if (m_bPlayCurrentFile) {
      if (NULL != m_pVedioPlayUI) {
         QString szFileName;
         bool bCanPlay = m_pVedioPlayUI->GetCurFile(szFileName);
         unsigned int iVolume = m_pVedioPlayUI->GetCurVolume();
         if (bCanPlay) {
            bool audioFile = false;
            if (szFileName.contains(".wav") || szFileName.contains(".WAV") || szFileName.contains(".mp3") || szFileName.contains(".MP3")) {
               audioFile = true;
            }
            if (pOBSControlLogic->MediaPlay((char*)szFileName.toStdString().c_str(), audioFile)) {
               m_pVedioPlayUI->TryHideToolBar(5);
               InteractAPIManager apiManager(this);
               apiManager.HttpSendChangeWebWatchLayout(1);
               TRACE6("%s szFileName:%ws\n", __FUNCTION__, szFileName.toStdWString().c_str());
               pOBSControlLogic->SetVolume(iVolume);
               m_bPlayCurrentFile = false;
               mbIsPlayEnd = true;
            }
            pOBSControlLogic->SetEnablePlayOutMediaAudio(mbIsEnablePlayOutAudio);
         }
      }
   }
}

void VedioPlayLogic::HandleVhallActiveCyclePlay() {
   if (!m_pVedioPlayUI) {
      return;
   }

   VH::CComPtr<IMainUILogic> pMainUILogic;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IMainUI, IID_IMainUILogic, pMainUILogic, return, ASSERT(FALSE));
   signed long long curPos, maxPos;
   int nState = pMainUILogic->GetPlayFileState(eLiveType_VhallActive);
   int nRet = pMainUILogic->GetPlayMediaFilePos(curPos, maxPos, eLiveType_VhallActive);
   long dulation = curPos;   //腾讯互动返回的时间为秒,计算进度时根据ms来计算
   long maxDulation = maxPos;

   QString dulationStr = dulation2Str(dulation, maxDulation);
   if (nState == PLAYUI_PLAYSTATE_STOP ||
      nState == PLAYUI_PLAYSTATE_END ||
      nState == PLAYUI_PLAYSTATE_FAIL) {
      QString dulationStr = dulation2Str(0, 0);
      m_pVedioPlayUI->UpdatePlayTime(dulationStr);
      m_pVedioPlayUI->UpdatePlayStates(FALSE);
      m_pVedioPlayUI->SetProgressValue(0, 0);
   }
   else if (nState == PLAYUI_PLAYSTATE_PAUSED) {
      m_pVedioPlayUI->UpdatePlayTime(dulationStr);
      m_pVedioPlayUI->UpdatePlayStates(FALSE);
      m_pVedioPlayUI->SetProgressValue(dulation, maxDulation);
   }
   else if (PLAYUI_PLAYSTATE_STOPPING == nState) {
      m_pVedioPlayUI->UpdatePlayTime(dulationStr);
      m_pVedioPlayUI->UpdatePlayStates(FALSE);
      m_pVedioPlayUI->SetProgressValue(dulation, maxDulation);
   }
   else {
      m_pVedioPlayUI->UpdatePlayTime(dulationStr);
      m_pVedioPlayUI->UpdatePlayStates(TRUE);
      m_pVedioPlayUI->SetProgressValue(dulation, maxDulation);
   }

   switch (nState) {
   case PLAYUI_PLAYSTATE_STOPPING:
   case PLAYUI_PLAYSTATE_OPENING:
   case PLAYUI_PLAYSTATE_BUFFERING:
      m_pVedioPlayUI->Enabled(false);
      break;
   default:
      m_pVedioPlayUI->Enabled(true);
      break;
   }

   if (nState == PLAYUI_PLAYSTATE_END) {
      switch (m_pVedioPlayUI->GetCurPlayMode()) {
      case CycleTyle_None: {//顺序播放
         if (!m_pVedioPlayUI->IsArriveTheEnd()) {
            m_pVedioPlayUI->SwitchToNext();
            PlaySelectedFile();
         }
         else {
            TRACE6("%s MSG_MAINUI_VEDIOPLAY_STOPPLAY\n", __FUNCTION__);
            SingletonMainUIIns::Instance().PostCRMessage(MSG_MAINUI_VEDIOPLAY_STOPPLAY, NULL, 0);
            if (mbIsPlayEnd && m_pVideoChoiceUI && m_pVideoChoiceUI->isHidden() && m_pVedioPlayUI && !m_pVedioPlayUI->isHidden()) {
               mbIsPlayEnd = false;
               SingletonMainUIIns::Instance().PostCRMessage(MSG_MAINUI_VEDIOPLAY_STOPPLAY, NULL, 0);
            }
         }  
         break;
      }   
      case CycleTyle_List: {	//列表循环
         m_pVedioPlayUI->SwitchToNext();
         PlaySelectedFile();
         break;
      } 
      case CycleTyle_Single: { //单视频循环
         PlaySelectedFile();
         break;
      }  
      case CycleTyle_END:
         break;
      default:
         break;
      }
   }

   if (m_bPlayCurrentFile) {
      if (NULL != m_pVedioPlayUI) {
         QString szFileName;
         bool bCanPlay = m_pVedioPlayUI->GetCurFile(szFileName);
         unsigned int iVolume = m_pVedioPlayUI->GetCurVolume();
         if (bCanPlay) {
            if (pMainUILogic->OpenPlayMediaFile((char*)szFileName.toStdString().c_str(), eLiveType_VhallActive) == 0) {
               TRACE6("%s szFileName:%ws\n", __FUNCTION__, szFileName.toStdWString().c_str());
               pMainUILogic->SetPlayMediaFileVolume(iVolume, eLiveType_VhallActive);
               m_bPlayCurrentFile = false;
            }
         }
      }
   }
}

int STDMETHODCALLTYPE VedioPlayLogic::GetPlayUILiveType() {
   return mLiveType;
}

bool STDMETHODCALLTYPE VedioPlayLogic::IsPlayMediaFileUIShown() {
   if (m_pVedioPlayUI == NULL) {
      return false;
   }
   return m_pVedioPlayUI->isHidden() ? false : true;
}

void VedioPlayLogic::IsEnableShowPlayOutChoise() {
   if (mLiveType == eLiveType_VhallActive) {
      m_pVideoChoiceUI->SetCurrentPlayOutState(false);
      m_pVideoChoiceUI->HidePlayOutCheck(true);
      mbIsEnablePlayOutAudio = false;
   }
   else {
      m_pVideoChoiceUI->HidePlayOutCheck(false);
      m_pVideoChoiceUI->SetCurrentPlayOutState(mbIsEnablePlayOutAudio);
   }
}


void VedioPlayLogic::CreatePlayMediaFileUI() {
   if (m_pVedioPlayUI == NULL) {
      VH::CComPtr<IMainUILogic> pMainUILogic;
      DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IMainUI, IID_IMainUILogic, pMainUILogic, return, ASSERT(FALSE));
      if (pMainUILogic) {
         QWidget* pMainUI = NULL;
         QWidget* pContentUI = NULL;
         pMainUILogic->GetMainUIWidget((void**)&pMainUI, mLiveType);
         int width = pMainUILogic->GetMediaPlayUIWidth(mLiveType);

         if (pMainUI == NULL || width == 0) {
            return;
         }
         //插播窗口创建
         m_pVedioPlayUI = new VedioPlayUI((QWidget*)pMainUI);
         if (m_pVedioPlayUI == NULL) {
            return;
         }
         m_qCycleTimer.stop();
         pMainUILogic->SetVedioPlayUi(m_pVedioPlayUI, mLiveType);
         m_pVedioPlayUI->Create();
         m_pVedioPlayUI->SetCaptureType(mLiveType);
         m_pVedioPlayUI->setFixedWidth(width);
         connect(m_pVedioPlayUI, SIGNAL(SigRepos(bool)), this, SLOT(SlotRepos(bool)));
         connect(m_pVedioPlayUI, SIGNAL(SigForceHide(bool)), this, SLOT(SlotForceHide(bool)));
         connect(&m_qCycleTimer, SIGNAL(timeout()), this, SLOT(OnCyclePlay()));
         connect(m_pVideoChoiceUI, SIGNAL(SigSyncFileList(QStringList, int)), m_pVedioPlayUI, SLOT(SlotFlushPlayList(QStringList, int)));
      }
   }
}
