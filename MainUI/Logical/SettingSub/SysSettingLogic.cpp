#include "StdAfx.h"
#include "SysSettingLogic.h"
#include "SystemSettingDlg.h"
#include "ConfigSetting.h"
#include <QFileDialog>
#include <QDesktopServices>
#include <QUrl>
#include <QDesktopWidget> 
#include "Msg_MainUI.h"
#include "Msg_CommonToolKit.h"
#include "Msg_OBSControl.h"
#include "Msg_VhallRightExtraWidget.h"
#include "ICommonData.h"
#include "IOBSControlLogic.h"
#include "IDeviceManager.h"
#include "IDShowPlugin.h"
#include "IDeckLinkDevice.h"
#include "pathManager.h"
#include "pub.Struct.h"
#include "pathManage.h"
#include "VhallNetWorkInterface.h"

#define QUESTION_RUL "http://e.vhall.com/api/client/v1/util/feed-back?report_type=%1&report_content=%2&webinar_id=%3"
#define MAX_STREAM 5
static int gCountLog = 0;
std::atomic_bool SysSettingLogic::bManagerThreadRun = false;
HANDLE SysSettingLogic::mThreadEvent = nullptr;

SysSettingLogic::SysSettingLogic(void)
: m_pSystemSettingDlg(NULL)
, m_iLastQuality(-1)
, m_iLastLine(0)
, m_bIsSaveFile(0) {

}

SysSettingLogic::~SysSettingLogic(void) {
   ProcessStreamDisConnectted();
}

BOOL SysSettingLogic::Create() {
   do {
      VH::CComPtr<IMainUILogic> pMainUILogic;
      DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IMainUI, IID_IMainUILogic, pMainUILogic, return FALSE, ASSERT(FALSE));

      QWidget* pMainUI = NULL;
      pMainUILogic->GetMainUIWidget((void**)&pMainUI);

      //系统设置窗口创建
      m_pSystemSettingDlg = new SystemSettingDlg((QDialog*)pMainUI);
      if (NULL == m_pSystemSettingDlg || !m_pSystemSettingDlg->Create()) {
         ASSERT(FALSE);
         break;
      }

      //初始化视频保存目录
      //wstring confPath = GetAppPath() + CONFIGPATH;
      QString qsConfPath = CPathManager::GetConfigPath();
      QString defaultChoosePath = ConfigSetting::ReadString(qsConfPath, GROUP_DEFAULT, KEY_RECORD_PATH, "");
      if (defaultChoosePath.isEmpty() || defaultChoosePath.isNull() || defaultChoosePath.length() == 0) {
         defaultChoosePath = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation).first();
      }

      QString fileName = ConfigSetting::ReadString(qsConfPath, GROUP_DEFAULT, KEY_RECORD_FILENAME, "");
      int iRecordeMode = ConfigSetting::ReadInt(qsConfPath, GROUP_DEFAULT, KEY_RECORD_MODE, 0);
      m_pSystemSettingDlg->SetRecord(iRecordeMode == 1 ? true : false);

      m_pSystemSettingDlg->SetSavePath((char*)defaultChoosePath.toStdString().c_str());
      m_pSystemSettingDlg->SetSaveFileName(fileName);
      m_pSystemSettingDlg->SetSavedDir(defaultChoosePath);

      this->m_iVcodecFPS = ConfigSetting::ReadInt(qsConfPath, GROUP_DEFAULT, KEY_VCODEC_FPS, DEFAULT_VCODEC_FPS);
      TRACE6("%s defaultChoosePath: %s fileName:%s\n", __FUNCTION__, defaultChoosePath.toStdString().c_str(), fileName.toStdString().c_str());
      return TRUE;

   } while (FALSE);

   return FALSE;
}

void SysSettingLogic::Destroy() {
   //销毁系统设置窗口
}

DWORD WINAPI SysSettingLogic::ThreadProcess(LPVOID p) {
   TRACE6("SysSettingLogic::ThreadProcess\n");
   while (bManagerThreadRun) {
      DWORD ret = WaitForSingleObject(mThreadEvent, 2000);
      if (p) {
         SysSettingLogic* sdk = (SysSettingLogic*)(p);
         if (sdk) {
            sdk->ProcessTask();
         }
      }
   }
   TRACE6("SysSettingLogic::ThreadProcess end\n");
   return 0;
}

void SysSettingLogic::ProcessTask() {
   UpdateDebugState();
}

void SysSettingLogic::DealMessage(DWORD adwMessageID, void* apData, DWORD adwLen) {
   switch (adwMessageID) {
      //点击控制
   case MSG_MAINUI_CLICK_CONTROL:
      DealClickControl(apData, adwLen);
      break;
   case MSG_MAINUI_START_LIVE_STATUS: {
       DealLiveStatus(apData, adwLen);
       break;
   }
   case MSG_MAINUI_CLOSE_SYS_SETTINGWND:
      if (m_pSystemSettingDlg && !m_pSystemSettingDlg->isHidden()) {
         m_pSystemSettingDlg->OnClose();
      }
      break;
      //音量改变
   case MSG_MAINUI_VOLUME_CHANGE:
      DealVolumeChange(apData, adwLen);
      break;
      //静音设置
   case MSG_MAINUI_MUTE:
      DealMuteSetting(apData, adwLen);
      break;
   case MSG_COMMONDATA_DATA_INIT:
      DealDataInitNotify(apData, adwLen);
      break;
      //流状态改变
   case MSG_OBSCONTROL_STREAMSTATUS_CHANGE:
      DealStreamStatusChanged(apData, adwLen);
      break;
      //设置摄像头
   case MSG_MAINUI_CAMERA_SETTING:
      DealCameraSetting(apData, adwLen);
      break;

   case MSG_MAINUI_DEVICE_CHANGE:
      DealDeviceChange(apData, adwLen);
      break;
   case MSG_VHALLRIGHTEXTRAWIDGET_CREATE:
      DealRightExternWidgetCreate(apData, adwLen);
      break;
      //呼出音频设置界面
   case MSG_MAINUI_SHOW_AUDIO_SETTING_CARD:
      DealAudioSettingCard(apData, adwLen);
      break;
   case MSG_MAINUI_DESKTOP_SHOW_SETTING:
      DealDesktopSetting(apData, adwLen);
      break;
      //离开推流页面
   case MSG_MAINUI_DO_CLOSE_MAINWINDOW:
      ProcessStreamDisConnectted();
      break;
   case MSG_MAINUI_NOISE_VALUE_CHANGE:
      DealNoiseValueChange(apData, adwLen);
      break;
   default:
      break;
   }
}

void SysSettingLogic::InitDev() {
   DealDataInitNotify(NULL, 0);
}

void SysSettingLogic::DealRightExternWidgetCreate(void *apData, DWORD adwLen) {
   if (!m_pSystemSettingDlg) {
      return;
   }
   m_pSystemSettingDlg->EnabledServerPlayBack(true);
   m_bServerPlaybackEffect = true;
}

bool SysSettingLogic::GetIsServerPlayback() {
   if (m_bServerPlaybackEffect) {
      QString qsConfPath = CPathManager::GetConfigPath();
      bool serverPlayback = (ConfigSetting::ReadInt(qsConfPath, GROUP_DEFAULT, KEY_SERVER_PLAYBACK, 0) != 0);
      return serverPlayback;
   }
   return false;
}

//呼出音频设置界面
void SysSettingLogic::DealAudioSettingCard(void *apData, DWORD adwLen) {
    if (!m_pSystemSettingDlg) {
        return;
    }
    VH::CComPtr<IMainUILogic> pMainUILogic;
    DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IMainUI, IID_IMainUILogic, pMainUILogic, return, ASSERT(FALSE));

    QWidget* pMainUI = NULL;
    pMainUILogic->GetMainUIWidget((void**)&pMainUI);
    if(NULL != pMainUI)
	    m_pSystemSettingDlg->Show(QApplication::desktop()->availableGeometry(pMainUI)  ,e_page_audio);
    else
	    m_pSystemSettingDlg->Show(QApplication::desktop()->availableGeometry(m_pSystemSettingDlg), e_page_audio);
}

void SysSettingLogic::DealDeviceChange(void *apData, DWORD adwLen) {
   m_deviceReset = true;
}

//处理扬声器设置
void SysSettingLogic::DealCameraSetting(void* apData, DWORD adwLen) {
   if (!m_pSystemSettingDlg) {
      return;
   }
   DEF_CR_MESSAGE_DATA_DECLARE_CA(STRU_MAINUI_CAMERACTRL, loMessage, return);
   m_pSystemSettingDlg->CameraSetting(loMessage.m_Device);
}

void SysSettingLogic::SetUsedDevice(bool add, DeviceInfo info) {
   if (m_pSystemSettingDlg) {
      add ? m_pSystemSettingDlg->SelectCamera(info) : m_pSystemSettingDlg->RemoveCamera(info);
   }
}

void SysSettingLogic::SetPriviewMic(DeviceInfo info) {
   if (m_pSystemSettingDlg) {
      m_pSystemSettingDlg->SetCurMicItem(info);
   }
   STRU_OBSCONTROL_AUDIO_CAPTURE loAudioCapture;
   loAudioCapture.info = info;
   loAudioCapture.isNoise = m_pSystemSettingDlg->GetIsNoise();    //是否开启噪音阀
   int noiseGate = m_pSystemSettingDlg->GetThresHoldValue();      //噪音阀值
   loAudioCapture.openThreshold = (-96 + noiseGate + 6) < 0 ? (-96 + noiseGate + 6) : 0;
   loAudioCapture.closeThreshold = (-96 + noiseGate) < 0 ? (-96 + noiseGate) : 0;
   loAudioCapture.iKbps = m_pSystemSettingDlg->GetKbps();
   loAudioCapture.iAudioSampleRate = m_pSystemSettingDlg->GetAudioSampleRate();
   loAudioCapture.bNoiseReduction = m_pSystemSettingDlg->GetNoiseReduction();
   loAudioCapture.fMicGain = m_pSystemSettingDlg->GetMicGain();
   TRACE6("[BP] SysSettingLogic::ApplySettings e_page_audio [%s]\n", QString::fromWCharArray(loAudioCapture.info.m_sDeviceDisPlayName).toLocal8Bit().data());
   SingletonMainUIIns::Instance().PostCRMessage(MSG_OBSCONTROL_AUDIO_CAPTURE, &loAudioCapture, sizeof(loAudioCapture));
}

void SysSettingLogic::ApplySettings(int applyIdentify) {
    TRACE6("[BP] SysSettingLogic::ApplySettings %X\n", applyIdentify);
    if (!m_pSystemSettingDlg) {
        TRACE6("[BP] SysSettingLogic::ApplySettings return a\n");
        return;
    }
    VH::CComPtr<ICommonData> pCommonData;
    DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));


    VH::CComPtr<IOBSControlLogic> pObsControlLogic = NULL;
    DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IOBSControl, IID_IOBSControlLogic, pObsControlLogic, return, ASSERT(FALSE));

    VH::CComPtr<IMainUILogic> pMainUILogic;
    DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IMainUI, IID_IMainUILogic, pMainUILogic, return, ASSERT(FALSE));

    if (applyIdentify&e_page_common) {
        TRACE6("[BP] SysSettingLogic::ApplySettings e_page_common\n");
        PublishInfo loStreamInfo;
        int publishLine = m_pSystemSettingDlg->GetPublishLine();
        int quality = m_pSystemSettingDlg->GetQuality();
        TRACE6("[BP] SysSettingLogic::ApplySettings e_page_common publishLine[%d] quality[%d]\n", publishLine, quality);
        pCommonData->GetStreamInfo(loStreamInfo);
        if (publishLine < 0 || publishLine >= loStreamInfo.mPubLineInfo.size()) {
            ASSERT(FALSE);
            TRACE6("[BP] SysSettingLogic::ApplySettings return b\n");
            return;
        }

        loStreamInfo.mCurRtmpUrlIndex = publishLine;
        loStreamInfo.mCurRtmpUrl = loStreamInfo.mPubLineInfo[loStreamInfo.mCurRtmpUrlIndex].mStrDomain;
        pCommonData->SetStreamInfo(&loStreamInfo);
        pCommonData->SetOutputInfo(&GOutputInfo[quality]);

        const char *currentUrl = loStreamInfo.mPubLineInfo[m_iLastLine].mStrDomain.c_str();
        const char *nextUrl = loStreamInfo.mPubLineInfo[publishLine].mStrDomain.c_str();
        pObsControlLogic->ResetPublishInfo(currentUrl, nextUrl);
        TRACE6("[BP] SysSettingLogic::ApplySettings ResetPublishInfo currentUrl[%s] nextUrl[%s]\n", currentUrl, nextUrl);
	    pObsControlLogic->Reset(m_iLastQuality != quality);//线路改变
        TRACE6("[BP] SysSettingLogic::ApplySettings pObsControlLogic->Reset End\n");

        m_iLastLine = publishLine;
	    m_iLastQuality = quality;

        QJsonObject body;
        body["rsl"] = m_pSystemSettingDlg->GetQualityText();								//分辨率索引
        body["pl"] = QString(nextUrl);//推流线路索引
	    SingletonMainUIIns::Instance().reportLog(STR_VH_LIVE_OPERATE, eLogRePortK_SystemSeting,L"SystemSeting", body);
    }

    if (applyIdentify&e_page_vedio) {
        TRACE6("[BP] SysSettingLogic::ApplySettings e_page_vedio\n");
        STRU_OBSCONTROL_VIDIO_SET vidioSet;
        vidioSet.high_codec_open = m_pSystemSettingDlg->GetHighQualityCod();
        if (m_pSystemSettingDlg->GetCurrentCameraDeviceInfo(vidioSet.m_DeviceInfo)) {
            TRACE6("[BP] SysSettingLogic::ApplySettings e_page_vedio [%s]\n", QString::fromWCharArray(vidioSet.m_DeviceInfo.m_sDeviceDisPlayName).toLocal8Bit().data());
            SingletonMainUIIns::Instance().PostCRMessage(
            MSG_OBSCONTROL_VIDIO_HIGHQUALITYCOD, &vidioSet, sizeof(STRU_OBSCONTROL_VIDIO_SET));
            QJsonObject body;
            body["vdn"] = QString::fromWCharArray(vidioSet.m_DeviceInfo.m_sDeviceDisPlayName);//vdn:设备用于显示的名字
            body["vn"] = QString::fromWCharArray(vidioSet.m_DeviceInfo.m_sDeviceName);
            body["vid"] = QString::fromWCharArray(vidioSet.m_DeviceInfo.m_sDeviceID);
            body["vt"] = vidioSet.m_DeviceInfo.m_sDeviceType;
            body["rsl"] = m_pSystemSettingDlg->GetVedioResolution();
            body["fr"] = m_pSystemSettingDlg->GetFrame();
            body["dl"] = m_pSystemSettingDlg->GetDeinterLace();
            body["hc"] = vidioSet.high_codec_open;
		    SingletonMainUIIns::Instance().reportLog(STR_VH_LIVE_OPERATE, eLogRePortK_VedioSeting, L"VedioSeting", body);
        }
    }

    if (applyIdentify&e_page_audio) {
        bool isForceMono = m_pSystemSettingDlg->GetForceMono();
        pObsControlLogic->SetForceMono(isForceMono);
        STRU_OBSCONTROL_AUDIO_CAPTURE loAudioCapture;
        loAudioCapture.info = m_pSystemSettingDlg->GetMicDeviceInfo();
        loAudioCapture.isNoise = m_pSystemSettingDlg->GetIsNoise();    //是否开启噪音阀
        int noiseGate = m_pSystemSettingDlg->GetThresHoldValue();      //噪音阀值
        loAudioCapture.openThreshold = (-96 + noiseGate + 6) < 0 ? (-96 + noiseGate + 6) : 0;
        loAudioCapture.closeThreshold = (-96 + noiseGate) < 0 ? (-96 + noiseGate) : 0;
        loAudioCapture.iKbps = m_pSystemSettingDlg->GetKbps();
        loAudioCapture.iAudioSampleRate = m_pSystemSettingDlg->GetAudioSampleRate();
        loAudioCapture.bNoiseReduction = m_pSystemSettingDlg->GetNoiseReduction();
        loAudioCapture.fMicGain = m_pSystemSettingDlg->GetMicGain();
        TRACE6("[BP] SysSettingLogic::ApplySettings e_page_audio [%s]\n", QString::fromWCharArray(loAudioCapture.info.m_sDeviceDisPlayName).toLocal8Bit().data());
        SingletonMainUIIns::Instance().PostCRMessage(MSG_OBSCONTROL_AUDIO_CAPTURE, &loAudioCapture, sizeof(loAudioCapture));

        QJsonObject body;
        body["_nr"] = loAudioCapture.isNoise; //是否开启噪音阀
        body["vdn"] = QString::fromWCharArray(loAudioCapture.info.m_sDeviceDisPlayName);//音频设备名称
        body["vn"] = QString::fromWCharArray(loAudioCapture.info.m_sDeviceName);
        body["vid"] = QString::fromWCharArray(loAudioCapture.info.m_sDeviceID);
        body["vt"] = loAudioCapture.info.m_sDeviceType;
        body["ngt"] = noiseGate;											//噪音阀值
        body["kbps"] = loAudioCapture.iKbps;						//音频码率
        body["asr"] = loAudioCapture.iAudioSampleRate;//音频采样率   
        body["nrd"] = loAudioCapture.bNoiseReduction;//麦克风降噪
        body["gain"] = loAudioCapture.fMicGain;//麦克风增益
	    SingletonMainUIIns::Instance().reportLog(STR_VH_LIVE_OPERATE, eLogRePortK_AudioSeting, L"AudioSeting", body);
        //扬声器
    }

    if (applyIdentify&e_page_record) {
        bool bSaveFile = m_pSystemSettingDlg->IsRecord();
        wchar_t recordPath[MAX_BUFF] = { 0 };
        m_pSystemSettingDlg->GetRecordFilePath(recordPath);
        TRACE6("[BP] SysSettingLogic::ApplySettings e_page_record [%s]\n", bSaveFile ? QString::fromWCharArray(recordPath).toLocal8Bit().data() :"NULL");
        pObsControlLogic->SetRecordPath(bSaveFile, recordPath);
        QJsonObject body;
        body["ir"] = bSaveFile;	//是否录制FLV
        body["fn"] = QString::fromWCharArray(recordPath);//录制文件名
        SingletonMainUIIns::Instance().reportLog(STR_VH_LIVE_OPERATE, eLogRePortK_Record, L"Record", body);
    }

    if (applyIdentify&e_page_proxy) {
        TRACE6("[BP] SysSettingLogic::ApplySettings pObsControlLogic Reset\n");
        pObsControlLogic->Reset(false);
        TRACE6("[BP] SysSettingLogic::ApplySettings pObsControlLogic Reset end\n");
        QJsonObject body;
        //判断，如果设置了代理，则生成代理的字符串并调用
        if (m_pSystemSettingDlg != NULL && m_pSystemSettingDlg->GetIsEnableProxy()) {
            QString proxyAddr = m_pSystemSettingDlg->GetProxyIP();
            QString proxyPort = m_pSystemSettingDlg->GetProxyPort();
            QString proxyUserName = m_pSystemSettingDlg->GetProxyUserName();
            QString proxyPwd = m_pSystemSettingDlg->GetProxyPwd();
            body["pe"] = QString("USE");	//代理类型
            body["ip"] = proxyAddr;//地址
            body["por"] = proxyPort;//端口
            body["un"] = proxyAddr;//用户名
            body["pas"] = proxyAddr;//密码

            TRACE6("[BP] SysSettingLogic::ApplySettings set proxy ok ip %s proxyPort %s proxyUserName %s proxyPwd %s\n", proxyAddr.toStdString().c_str(), proxyPort.toStdString().c_str() ,proxyUserName.toStdString().c_str(), proxyPwd.toStdString().c_str());

            if (pMainUILogic) {
                pMainUILogic->SetHttpProxy(true, proxyAddr.toStdString().c_str(), proxyPort.toStdString().c_str(), proxyUserName.toStdString().c_str(), proxyPwd.toStdString().c_str());
            }
        } else {
            body["pe"] = QString("NOT USE");	//代理类型
            TRACE6("[BP] SysSettingLogic::ApplySettings set proxy null\n");
            if (pMainUILogic) {
                pMainUILogic->SetHttpProxy(false);
            }
        }
        SingletonMainUIIns::Instance().reportLog(STR_VH_LIVE_OPERATE, eLogRePortK_proxy, L"proxy", body);
    }

    if (applyIdentify&e_page_question) {
        TRACE6("[BP] SysSettingLogic::ApplySettings e_page_question\n");
        ReportQuestion();
    }
    TRACE6("[BP] SysSettingLogic::ApplySettings return end\n");
}

void SysSettingLogic::DealLiveStatus(void* apData, DWORD adwLen) {
    DEF_CR_MESSAGE_DATA_DECLARE_CA(int, loMessage, return);
    if (m_pSystemSettingDlg) {
        m_pSystemSettingDlg->DealLiveStatus(loMessage);
    }
}

void SysSettingLogic::DealClickControl(void* apData, DWORD adwLen) {
   DEF_CR_MESSAGE_DATA_DECLARE_CA(STRU_MAINUI_CLICK_CONTROL, loMessage, return);
   switch (loMessage.m_eType) {
      //设置显示
   case control_ShowSetting:
      TRACE6("[BP] SysSettingLogic::DealClickControl control_ShowSetting\n");
      ShowSettingUI();
      break;
   default:
      break;
   }
}
//显示设置窗口
void SysSettingLogic::ShowSettingUI() {
    VH::CComPtr<IMainUILogic> pMainUILogic;
    DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IMainUI, IID_IMainUILogic, pMainUILogic, return, ASSERT(FALSE));
    if (!pMainUILogic->IsLoadUrlFinished()) {
        TRACE6("%s loading url ,can not showDeviceList\n", __FUNCTION__);
        return;
    }

    if (NULL != m_pSystemSettingDlg) {
        //显示窗口
	    QWidget* pMainUI = NULL;
	    pMainUILogic->GetMainUIWidget((void**)&pMainUI);
	    if (NULL != pMainUI)
		    m_pSystemSettingDlg->Show(QApplication::desktop()->availableGeometry(pMainUI), e_page_common);
	    else
		    m_pSystemSettingDlg->Show(QApplication::desktop()->availableGeometry(m_pSystemSettingDlg), e_page_common);
    }
}
void SysSettingLogic::ActiveSettingUI() {

}

void SysSettingLogic::DealDataInitNotify(void* apData, DWORD adwLen) {
   if (apData == NULL) {
      //-----------------------关于我们--------------------------//
      wstring wzGurVersion;
      VH::CComPtr<ICommonData> pCommonData;
      DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
      pCommonData->GetCurVersion(wzGurVersion);
      if (m_pSystemSettingDlg) {
         m_pSystemSettingDlg->SetVersion(QString::fromStdWString(wzGurVersion.c_str()));
         PublishInfo loStreamInfo;
         pCommonData->GetStreamInfo(loStreamInfo);
         QString Ver;
         wstring confPath = GetAppDataPath() + VHALL_TOOL_CONFIG;
         QString qsConfPath = qsConfPath.fromStdWString(confPath);
         QString vhallHelper = ConfigSetting::ReadString(qsConfPath, GROUP_DEFAULT, KEY_VHALL_HELPER, VHALL_LIVE_HELPER);
         m_pSystemSettingDlg->SetCopyrightInfo(vhallHelper, "微吼直播版权所有");
         m_pSystemSettingDlg->EnabledExtraResolution(GetIsEnabled1080p());
      }

      //-----------------------系统设置--------------------------//
      InitSysSetting();
      //摄像设备的初始化为每次打开设置窗时，初始化，不需要在系统初始化时初始化
      //-----------------------摄像设备--------------------------//
      //-----------------------音频设备--------------------------//
      InitAuidoUiInfo();
      VH::CComPtr<IMainUILogic> pMainUILogic;
      DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IMainUI, IID_IMainUILogic, pMainUILogic, return, ASSERT(FALSE));
      pMainUILogic->RunTask(CustomEvent_INIT_DEVICE);
   }
}

void SysSettingLogic::UpdateDebugState() {
   VH::CComPtr<ICommonData> pCommonData;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
   if (pCommonData->GetPublishState()) {
      VH::CComPtr<IOBSControlLogic> pOBSControlLogic;
      DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IOBSControl, IID_IOBSControlLogic, pOBSControlLogic, return, ASSERT(FALSE));
      m_oStreamStatusList.clear();
      pOBSControlLogic->GetStreamStatus(&m_oStreamStatusList);
      //更新流状态面板UI
      int sumSpeed = 0;
      for (int i = 0; i < m_oStreamStatusList.size(); i++) {
         StreamStatus status = m_oStreamStatusList[i];
         float dropFramePers = status.sumFrames != 0 ? status.droppedFrames * 25 / status.sumFrames : 0;
         QString streamInfo;
         sumSpeed += status.bytesSpeed;

         if (status.bytesSpeed < 1024) {
            streamInfo = streamInfo.sprintf("%d Bps", status.bytesSpeed);
         } else if (status.bytesSpeed >= 1024 && status.bytesSpeed < 1024 * 1024) {
            streamInfo = streamInfo.sprintf("%d KBps", status.bytesSpeed / 1024);
         } else if (status.bytesSpeed >= 1024 * 1024) {
            streamInfo = streamInfo.sprintf("%d MBps", status.bytesSpeed / 1024 / 1024);
         }

         if (gCountLog >= 20) {
            TRACE6("[STREAM]IP:%s [%s] currentDrop[%lu] sumDrop [%lu] sum [%lu]\n",status.serverIP,streamInfo.toStdString().c_str(),status.currentDroppedFrames,status.droppedFrames, status.sumFrames);
            gCountLog = 0;
         }
         gCountLog++;

         SingletonMainUIIns::Instance().PostCRMessage(MSG_MAINUI_PUSH_STREAM_STATUS, &status, sizeof(StreamStatus));
         static uint mLastTime = 0;
         uint currentTime = QDateTime::currentDateTime().toTime_t();
         if (currentTime - mLastTime > 15) {
            mLastTime = currentTime;
            wchar_t stream_ip_w[128] = { 0 };
            QString stream_ip_q = QString::fromUtf8(status.serverIP);
            if (stream_ip_q.length() == 0) {
               stream_ip_q = "0";
            }
            stream_ip_q.toWCharArray(stream_ip_w);
            QJsonObject body;

            double cpuf = CalCpuUtilizationRate();
            if (cpuf > 0.0 && cpuf <= 100.0) {
               int nMemTotal = 0;
               int nMemUsed = 0;
               GetSysMemory(nMemTotal, nMemUsed);
               //获得码率分辨率等信息
               OBSOutPutInfo outPutInfo;
               pCommonData->GetOutputInfo(&outPutInfo);
               int outputWidth = outPutInfo.m_outputSize.cx;
               int outputHeight = outPutInfo.m_outputSize.cy;
               STRU_MAINUI_LOG log;
               swprintf_s(log.m_wzRequestUrl, DEF_MAX_HTTP_URL_LEN, L"key=stream_info&stream_ip=%s&bitrate=%u&lost_frame=%lu&iw=%d&ih=%d&k=%d",stream_ip_w,outPutInfo.videoBits,status.droppedFrames, outputWidth, outputHeight,eLogRePortK_StreamState);

               body["_bw"] = QString::number(status.bytesSpeed);		//带宽
               body["_rt"] = QString::number(outPutInfo.videoBits);			//码率
               body["tt"] = pOBSControlLogic->GetStartStreamTime().msecsTo(QDateTime::currentDateTime());		   //当前推流时长
               body["cu"] = cpuf;			//CPU使用率
               body["mu"] = ((1.0 * nMemUsed) / nMemTotal);			//内存使用率

               QString json = CPathManager::GetStringFromJsonObject(body);
               strcpy_s(log.m_wzRequestJson, json.mid(0, DEF_MAX_HTTP_URL_LEN).toLatin1().data());
               SingletonMainUIIns::Instance().PostCRMessage(MSG_MAINUI_LOG, &log, sizeof(STRU_MAINUI_LOG));
            }
         }
      }
   }
}

static bool have_clockfreq = false;
static LARGE_INTEGER clock_freq;

static inline unsigned long long get_clockfreq(void) {
   if (!have_clockfreq)
      QueryPerformanceFrequency(&clock_freq);
   return clock_freq.QuadPart;
}

static unsigned long long os_gettime_ns(void) {
   LARGE_INTEGER current_time;
   double time_val;

   QueryPerformanceCounter(&current_time);
   time_val = (double)current_time.QuadPart;
   time_val *= 1000000000.0;
   time_val /= (double)get_clockfreq();

   return (unsigned long long)time_val;
}

void ReportFrameInfo(string k, string token, long reportId, char* streamId) {
   QString url = "http://la.e.vhall.com:1780/login?k=%1&id=%2&s=%3&token=%4";
   url = url.arg(QString::fromStdString(k)).arg(reportId).arg(streamId).arg(QString(QString::fromStdString(token).toLatin1().toBase64()));
   wchar_t urlwchar[1024] = { 0 };
   if (url.toWCharArray(urlwchar) > 0) {
      HTTP_GET_REQUEST request(url.toStdString());
      GetHttpManagerInstance()->HttpGetRequest(request, [&](const std::string& msg, int code, const std::string userData) {
         TRACE6("%s msg %s\n", __FUNCTION__, msg.c_str());
      });
   }
}

#define SAMPLECOUNT 10 

void SysSettingLogic::StreamStatusAnalysis() {
   if (m_oStreamStatusList.size() <= 0) {
      return;
   }
   if (mLogFrameStatusList.count() != m_oStreamStatusList.size()) {
      mLogFrameStatusList.clear();
      for (int i = 0; i < m_oStreamStatusList.size(); i++) {
         mLogFrameStatusList.append(QList<struct LogFrameStatus>());
      }
   }
   //遍历每一个流
   for (int i = 0; i < m_oStreamStatusList.size(); i++) {
      //TRACE6("=========================================================\n");
      StreamStatus streamStatus = m_oStreamStatusList[i];
  
      //编码率
      struct LogFrameStatus logFrameStatus;
      logFrameStatus.encodeFrameCount = streamStatus.sumFrames;
      logFrameStatus.os_times = os_gettime_ns();
      //丢帧率
      logFrameStatus.dropFrameCount = streamStatus.droppedFrames;
      logFrameStatus.totalFrameCount = streamStatus.sumFrames;
      if (mLogFrameStatusList[i].count() > 0) {
         if (mLogFrameStatusList[i].last().encodeFrameCount == logFrameStatus.encodeFrameCount) {
            continue;
         } else if (mLogFrameStatusList[i].last().encodeFrameCount > logFrameStatus.encodeFrameCount) {
            mLogFrameStatusList[i].clear();
            continue;
         }
      }

      mLogFrameStatusList[i].append(logFrameStatus);
      if (SAMPLECOUNT <= mLogFrameStatusList[i].count()) {
         qreal _fr = 0;
         qreal _er = 0;
         qreal tha = 0;
         qreal thal = 0;
         qreal lastfr = 0;
         for (int j = 0; j < mLogFrameStatusList[i].count(); j++) {
            logFrameStatus = mLogFrameStatusList[i][j];
            double fr = logFrameStatus.dropFrameCount;
            fr /= logFrameStatus.totalFrameCount;
            if (_fr<fr) {
               _fr = fr;
            }
            if (i>0) {
               qreal ttha = fr - lastfr;
               if (tha<ttha) {
                  tha = ttha;
               }
               if (thal>ttha) {
                  thal = ttha;
               }
            }
            lastfr = fr;
         }

         unsigned long long dTime = mLogFrameStatusList[i].last().os_times - mLogFrameStatusList[i].front().os_times;

         unsigned long dCount = mLogFrameStatusList[i].last().encodeFrameCount - mLogFrameStatusList[i].front().encodeFrameCount;
         dTime /= 1000000;
         dCount *= 1000;

         _er = dCount;
         _er /= dTime;
         mLogFrameStatusList[i].clear();

         _er /= this->m_iVcodecFPS;
         _er *= 100;
         //_er 编码性能指数:实际编码数量/应编码数量
         //_fr 丢帧率:丢帧数/总帧数
         //tha 网络性能下降指数:丢帧率上升幅度
         //thal 网络性能提升指数:丢帧率下降幅度

         QString strER = QString::number((int)_er) + "%";
         //上报主机性能
         static int _erCount = 0;
         if (_er<90) {
            _erCount++;
         } else {
            _erCount = 0;
         }
         if (_erCount>1) {
            QString startStreamInfo = "{\"p\":\"%1\",\"_er\":\"%2\",\"si\":\"%3\"}"; 
            startStreamInfo =startStreamInfo.arg(streamStatus.streamID).arg(strER).arg(streamStatus.serverIP);
            VH::CComPtr<ICommonData> pCommonData;
            DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
            ReportFrameInfo("14003", startStreamInfo.toStdString(), pCommonData->GetReportID(), streamStatus.streamID);
            TRACE6("14003:%s\n", startStreamInfo.toLocal8Bit().data());
         }

         float _tr = tha;
         //上报网络不足
         if (_tr > 0.1) {
            QString startStreamInfo = "{\"p\":\"%1\",\"_fr\":\"%2\",\"_tr\":\"%3\",\"si\":\"%4\"}";
            startStreamInfo = startStreamInfo.arg(streamStatus.streamID).arg(_fr).arg(_tr).arg(streamStatus.serverIP);
            VH::CComPtr<ICommonData> pCommonData;
            DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
            ReportFrameInfo("14002", startStreamInfo.toStdString(), pCommonData->GetReportID(), streamStatus.streamID);
            TRACE6("14002:%s\n", startStreamInfo.toLocal8Bit().data());
         }
      }
   }
}

//推流已经连接
void SysSettingLogic::ProcessStreamConnectted(WCHAR *msg, int streamCount) {
   //创建流状态线程
   TRACE6("%s\n",__FUNCTION__);
   if (mPushStreamStatusThread == nullptr) {
      mThreadEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
      bManagerThreadRun = true;
      mPushStreamStatusThread = new std::thread(SysSettingLogic::ThreadProcess, this);
   }
   TRACE6("%s\n", __FUNCTION__);
   //if (!m_pStreamStateTimer) {
   //   return;
   //}

   //m_oStreamStatusList.clear();
   //for (int i = 0; i < streamCount; i++) {
   //   StreamStatus status;
   //   memset(&status, 0, sizeof(StreamStatus));
   //   m_oStreamStatusList.push_back(status);
   //}
   ////启动更新流状态UI定时器
   ////m_pStreamStateTimer->start(2000);

   ////reset the Byte&Frame Recode Timer
   //m_iNumSecondsWaited4Byte = 0;
   //m_iNumSecondsWaited4Frame = 0;
   //m_LastBytesSentList.clear();
   //m_LastDropFramesList.clear();

   ////初始化队列
   //for (int i = 0; i < 5; i++) {
   //   m_LastDropFramesList.append(0);
   //   if (i < 3) {
   //      m_LastBytesSentList.append(0);
   //   }
   //}
}

void SysSettingLogic::DealStreamStatusChanged(void* apData, DWORD adwLen) {
   DEF_CR_MESSAGE_DATA_DECLARE_CA(STRU_OBSCONTROL_STREAMSTATUS, loMessage, return);
   switch (loMessage.m_eType) {
   case stream_status_connect:
      ProcessStreamConnectted(loMessage.m_wzMsg, loMessage.m_iStreamCount);
      break;
   case stream_status_disconnect:
      ProcessStreamDisConnectted();
      break;
   default:
      break;
   }
}

void SysSettingLogic::ProcessStreamDisConnectted() {
   TRACE6("%s 1\n", __FUNCTION__);
   if (mPushStreamStatusThread) {
      bManagerThreadRun = false;
      ::SetEvent(mThreadEvent);
      TRACE6("%s 2\n",__FUNCTION__);
      mPushStreamStatusThread->join();
      TRACE6("%s 3\n", __FUNCTION__);
      CloseHandle(mThreadEvent);
      delete mPushStreamStatusThread;
      mPushStreamStatusThread = nullptr;
   }
   TRACE6("%s 4\n", __FUNCTION__);
}

void SysSettingLogic::ReportQuestion() {

   if (NULL == m_pSystemSettingDlg) {
      ASSERT(FALSE);
      return;
   }

   int nType = -1;
   QString sQuestion;
   m_pSystemSettingDlg->GetQuestionInfo(nType, sQuestion);

   bool bEmpty = sQuestion.isEmpty();

   //未选中上报类型 和 未填写上报内容
   if (nType < 0 || bEmpty) {
      return;
   }
   //如果没有选择，按照其他算
   else if (nType == 0) {
      nType = 6;
   }

   m_pSystemSettingDlg->ClearReportContent();
   PublishInfo loStreamInfo;
   VH::CComPtr<ICommonData> pCommonData;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
   pCommonData->GetStreamInfo(loStreamInfo);

   nType = nType + 9;      //兼容其他程序WEB已有
   QString qsUrl = QUESTION_RUL;
   qsUrl = qsUrl.arg(nType).arg(sQuestion).arg(loStreamInfo.mStreamName.c_str());

   //STRU_HTTPCENTER_HTTP_RQ loRQ;
   //loRQ.m_dwPluginId = ENUM_PLUGIN_MAINUI;
   //wcsncpy(loRQ.m_wzRequestUrl, qsUrl.toStdWString().c_str(), DEF_MAX_HTTP_URL_LEN);
   //SingletonMainUIIns::Instance().PostCRMessage(MSG_HTTPCENTER_HTTP_RQ, &loRQ, sizeof(loRQ));

   HTTP_GET_REQUEST request(qsUrl.toStdString());
   GetHttpManagerInstance()->HttpGetRequest(request, [&](const std::string& msg, int code, const std::string userData) {
      TRACE6("%s msg %s\n", __FUNCTION__, msg.c_str());
   });
}

void FuncAppendx(FrameInfo info, void *ptr) {
   std::list<FrameInfo> *infos = (std::list<FrameInfo> *)ptr;
   infos->push_back(info);
}

void SysSettingLogic::InitCameraSetting() {
   if (!m_deviceReset) {
      return;
   }
   m_deviceReset = false;
   m_pSystemSettingDlg->ClearItem();
   ObtainDeviceList(m_CameraItemMap);

   if (m_CameraItemMap.size() == 0) {
      return;
   }

   std::vector<DeviceInfo> deviceInfoVector;
   DeviceList::iterator &itor = m_CameraItemMap.begin();
   deviceInfoVector.resize(m_CameraItemMap.size());
   while (itor != m_CameraItemMap.end()) {
      int num = GetDeviceDisplayNumber(*itor);
      if (num >= 0 && num < m_CameraItemMap.size()) {
         deviceInfoVector[num] = *itor;
      }
      itor++;
   }

   //设备列表
   for (int i = 0; i < deviceInfoVector.size(); i++) {
      m_pSystemSettingDlg->AddCameraItem(deviceInfoVector[i]);
   }
   m_pSystemSettingDlg->OnChgDevice(0);

}
bool SysSettingLogic::IsHasAudioDevice() {
   if (m_oMicList.size() > 0 || m_oSpeakerList.size() > 0) {
      return true;
   }
   return false;
}

void SysSettingLogic::GetDevice() {
   VH::CComPtr<IDeviceManager> pDeviceManager;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IOBSControl, IID_IDeviceManager, pDeviceManager, return, ASSERT(FALSE));
   m_oMicList.clear();
   m_oSpeakerList.clear();

   VH::CComPtr<IOBSControlLogic> pOBSControlLogic;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IOBSControl, IID_IOBSControlLogic, pOBSControlLogic, return, ASSERT(FALSE));
   pOBSControlLogic->GetCurrentMic(m_currentMic);
   //获取麦克风列表
   pDeviceManager->GetMicDevices(m_oMicList);
   //获取扬声器列表
   pDeviceManager->GetSpeakerDevices(m_oSpeakerList);
}


void SysSettingLogic::InitAuidoUiInfo() {
   VH::CComPtr<IOBSControlLogic> pOBSControlLogic;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IOBSControl, IID_IOBSControlLogic, pOBSControlLogic, return, ASSERT(FALSE));
   if (NULL != m_pSystemSettingDlg) {
      m_pSystemSettingDlg->ClearMicItem();
      m_pSystemSettingDlg->ClearSpeakerItem();
      //麦克风列表
      auto itor = m_oMicList.begin();
      for (; itor != m_oMicList.end(); itor++) {
         m_pSystemSettingDlg->AddMicItem(itor->m_sDeviceDisPlayName, *itor);
      }

      //未检测到设备
      if (0 == m_oMicList.size()) {
         m_pSystemSettingDlg->AddMicItem(L"未检测到设备", DeviceInfo());
         m_pSystemSettingDlg->SetCurMicItem(DeviceInfo());
      }

      //扬声器列表
      itor = m_oSpeakerList.begin();
      for (; itor != m_oSpeakerList.end(); itor++) {
         m_pSystemSettingDlg->AddSpeakerItem(itor->m_sDeviceDisPlayName,
            (wchar_t*)itor->m_sDeviceID);
      }
      //未检测到设备
      if (0 == m_oSpeakerList.size()) {
         m_pSystemSettingDlg->AddSpeakerItem(L"未检测到设备", NULL);

      }
   }

   m_pSystemSettingDlg->SetCurMicItem(m_currentMic);
   float micVolumn = pOBSControlLogic->GetMicVolunm();
   m_pSystemSettingDlg->SetMicVolume(micVolumn);
   //todo: 可用此方法，在第一次打开时初始化“麦克风增强”栏。（0参数为无用参数，在OnMicListChanged()函数里）
   m_pSystemSettingDlg->OnMicListChanged(0);
   float speakerVolumn = pOBSControlLogic->GetSpekerVolumn();
   m_pSystemSettingDlg->SetSpeakerVolume(speakerVolumn);
}

void SysSettingLogic::InitAudioSetting() {
   GetDevice();
   InitAuidoUiInfo();
}

void SysSettingLogic::ObtainDeviceList(DeviceList& deviceList) {
   deviceList.clear();
   VH::CComPtr<IDeviceManager> pDeviceManager;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IOBSControl, IID_IDeviceManager, pDeviceManager, return, ASSERT(FALSE));
   pDeviceManager->GetVedioDevices(deviceList);
   SyncDeviceList(deviceList);
}

void SysSettingLogic::InitSysSetting() {
   if (NULL != m_pSystemSettingDlg) {
      m_pSystemSettingDlg->Init();

      //初始化分辨率
      m_pSystemSettingDlg->AddResolution("640*360 360P");
      m_pSystemSettingDlg->AddResolution("960*540 540P");

      VH::CComPtr<ICommonData> pCommonData;
      DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
      PublishInfo loStreamInfo;
      pCommonData->GetStreamInfo(loStreamInfo);
      m_pSystemSettingDlg->AddResolution("1280*720 720P");

      BOOL bEnable1080P = GetIsEnabled1080p();
      if (!bEnable1080P) {
         QString qsResolution = tr("%1*%2 ").arg(GOutputInfo[3].m_baseSize.cx)
            .arg(GOutputInfo[3].m_baseSize.cy) + QString::fromUtf8("原始比例(最大支持1080P)");

         m_pSystemSettingDlg->AddResolution(qsResolution);
      } else {
		  QString qsResolution = tr("%1*%2 ").arg(GOutputInfo[4].m_baseSize.cx)
			  .arg(GOutputInfo[4].m_baseSize.cy) + QString::fromUtf8("原始比例");

         m_pSystemSettingDlg->AddResolution("1920*1080 1080P");
         m_pSystemSettingDlg->AddResolution(qsResolution);
      }


      //初始化活动ID
      m_pSystemSettingDlg->SetActivityID((char*)loStreamInfo.mStreamName.c_str());
      //初始化分辨率
      QString qsConfPath = CPathManager::GetConfigPath();
      m_iLastQuality = ConfigSetting::ReadInt(qsConfPath, GROUP_DEFAULT, KEY_PUBLISH_QUALITY, 2);
      if (m_iLastQuality < 0) {
          m_iLastQuality = 0;
      }
      m_pSystemSettingDlg->SetResolution(m_iLastQuality);

      //初始化线路
      for (int i = 0; i < loStreamInfo.mPubLineInfo.length(); i++) {
         string strAlias = loStreamInfo.mPubLineInfo[i].mStrAlias;
		 
		 m_pSystemSettingDlg->AddPubLines(QString().fromUtf8(strAlias.c_str()));
      }
      m_pSystemSettingDlg->SetCurLine(loStreamInfo.mCurRtmpUrlIndex);
   }
}

void SysSettingLogic::DealMuteSetting(void* apData, DWORD adwLen) {
   DEF_CR_MESSAGE_DATA_DECLARE_CA(STRU_MAINUI_MUTE, loMessage, return);
   if (NULL == m_pSystemSettingDlg) {
      return;
   }

   VH::CComPtr<IOBSControlLogic> pOBSControlLogic;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IOBSControl, IID_IOBSControlLogic, pOBSControlLogic, return, ASSERT(FALSE));

   if (loMessage.m_eType == mute_Mic) {
      float volumn = pOBSControlLogic->MuteMic(loMessage.m_bMute);
      m_pSystemSettingDlg->SetMicVolume(volumn);

      VH::CComPtr<IMainUILogic> pMainUILogic;
      DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IMainUI, IID_IMainUILogic, pMainUILogic, return, ASSERT(FALSE));
      pMainUILogic->SetMicMute(!(volumn != 0));

      TRACE6("%s mic value:%f\n", __FUNCTION__, volumn);

   } else if (loMessage.m_eType == mute_Speaker) {
      float volumn = pOBSControlLogic->MuteSpeaker(loMessage.m_bMute);
      m_pSystemSettingDlg->SetSpeakerVolume(volumn);

      VH::CComPtr<IMainUILogic> pMainUILogic;
      DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IMainUI, IID_IMainUILogic, pMainUILogic, return, ASSERT(FALSE));
      pMainUILogic->SetSpeakerMute(!(volumn != 0));
      TRACE6("%s mic value:%f\n", __FUNCTION__, volumn);
   }
}

void SysSettingLogic::DealVolumeChange(void* apData, DWORD adwLen) {
   DEF_CR_MESSAGE_DATA_DECLARE_CA(STRU_MAINUI_VOLUME_CHANGE, loMessage, return);
   switch (loMessage.m_eType) {
      //麦克风
   case change_Mic:
      AdjustMicVolume(loMessage.m_nVolume);
      break;
      //扬声器
   case change_Speaker:
      AdjustSpeakerVolume(loMessage.m_nVolume);
      break;

   default:
      break;
   }
}

//调整麦克风音量
void SysSettingLogic::AdjustMicVolume(float fVolume) {
   VH::CComPtr<IOBSControlLogic> pOBSControlLogic;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IOBSControl, IID_IOBSControlLogic, pOBSControlLogic, return, ASSERT(FALSE));
   pOBSControlLogic->SetMicVolunm(fVolume);

   VH::CComPtr<IMainUILogic> pMainUILogic;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IMainUI, IID_IMainUILogic, pMainUILogic, return, ASSERT(FALSE));
   pMainUILogic->SetMicMute(!(fVolume != 0));
}
//调整扬声器音量
void SysSettingLogic::AdjustSpeakerVolume(float fVolume) {
   VH::CComPtr<IOBSControlLogic> pOBSControlLogic;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IOBSControl, IID_IOBSControlLogic, pOBSControlLogic, return, ASSERT(FALSE));
   pOBSControlLogic->SetSpekerVolumn(fVolume);

   VH::CComPtr<IMainUILogic> pMainUILogic;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IMainUI, IID_IMainUILogic, pMainUILogic, return, ASSERT(FALSE));
   pMainUILogic->SetSpeakerMute(!(fVolume != 0));
}
void SysSettingLogic::DealDesktopSetting(void *apData, DWORD adwLen) {
   //m_pSystemSettingDlg->Show(e_page_common,false);
	VH::CComPtr<IMainUILogic> pMainUILogic;
	DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IMainUI, IID_IMainUILogic, pMainUILogic, return, ASSERT(FALSE));

	QWidget* pMainUI = NULL;
	pMainUILogic->GetShareUIWidget((void**)&pMainUI);
	if (NULL != pMainUI)
		m_pSystemSettingDlg->Show(QApplication::desktop()->availableGeometry(pMainUI), e_page_common, false);
	else
		m_pSystemSettingDlg->Show(QApplication::desktop()->availableGeometry(m_pSystemSettingDlg), e_page_common);

}

void SysSettingLogic::DealNoiseValueChange(void *apData, DWORD adwLen) {

   STRU_OBSCONTROL_AUDIO_CAPTURE loAudioCapture;
   loAudioCapture.info = m_pSystemSettingDlg->GetMicDeviceInfo();
   loAudioCapture.isNoise = m_pSystemSettingDlg->GetIsNoise();
   int noiseGate = m_pSystemSettingDlg->GetThresHoldValue();
   loAudioCapture.openThreshold = (-96 + noiseGate + 6) < 0 ? (-96 + noiseGate + 6) : 0;
   loAudioCapture.closeThreshold = (-96 + noiseGate) < 0 ? (-96 + noiseGate) : 0;

   loAudioCapture.iKbps = m_pSystemSettingDlg->GetKbps();
   loAudioCapture.iAudioSampleRate = m_pSystemSettingDlg->GetAudioSampleRate();
   loAudioCapture.bNoiseReduction = m_pSystemSettingDlg->GetNoiseReduction();
   loAudioCapture.fMicGain = m_pSystemSettingDlg->GetMicGain();

   TRACE6("[BP] SysSettingLogic::ApplySettings e_page_audio [%s]\n", QString::fromWCharArray(loAudioCapture.info.m_sDeviceDisPlayName).toLocal8Bit().data());
   SingletonMainUIIns::Instance().PostCRMessage(
      MSG_OBSCONTROL_AUDIO_CAPTURE, &loAudioCapture, sizeof(loAudioCapture));
}

void SysSettingLogic::SetCutRecordDisplay(const int iCutRecord)
{
	if (NULL!=m_pSystemSettingDlg){
		m_pSystemSettingDlg->SetCutRecordDisplay(iCutRecord);
	}
}

void SysSettingLogic::SelectFirstProtocol(QString protocol) {
    VH::CComPtr<ICommonData> pCommonData;
    DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
    PublishInfo loStreamInfo;
    pCommonData->GetStreamInfo(loStreamInfo);
    int countPushUrl = loStreamInfo.mPubLineInfo.size();
    for (int i = 0; i < countPushUrl; i++) {
        std::string pushListDomain = loStreamInfo.mPubLineInfo[i].mStrDomain;
        if (QString::fromStdString(pushListDomain).contains(protocol)) {
            mPublishLine = i;
            break;
        }
    }
}

//推流异常重新推流
void SysSettingLogic::ReSelectPushStreamLine() {
    VH::CComPtr<ICommonData> pCommonData;
    DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
    PublishInfo loStreamInfo;
    pCommonData->GetStreamInfo(loStreamInfo);
    TRACE6("%s\n",__FUNCTION__);

    mPublishLine = m_pSystemSettingDlg->GetCurPublishIndex();
    int countPushUrl = loStreamInfo.mPubLineInfo.size();
    std::string currentUrl = loStreamInfo.mPubLineInfo[mPublishLine].mStrDomain;
    if (mPublishLine == m_iLastLine) {
        //相同线路重推三次，则换线路。
        mPushErrorCount++;
    }
    else {
        mPushErrorCount = 0;
    }

    if (mPushErrorCount > 3) {
        if (mPublishLine < countPushUrl - 1) {
            mPublishLine++;
        }
        else {
            mPublishLine = 0;
        }
        //std::string nextDomain = loStreamInfo.mPubLineInfo[mPublishLine].mStrDomain;
        //if (QString::fromStdString(currentUrl).contains("rtmp")) {
        //    if (!QString::fromStdString(nextDomain).contains("rtmp")) {
        //        SelectFirstProtocol("rtmp");
        //    }
        //}
        //else if (QString::fromStdString(currentUrl).contains("http")) {
        //    if (!QString::fromStdString(nextDomain).contains("http")) {
        //        SelectFirstProtocol("http");
        //    }
        //}
        //else if (QString::fromStdString(currentUrl).contains("aestp")) {
        //    if (!QString::fromStdString(nextDomain).contains("aestp")) {
        //        SelectFirstProtocol("aestp");
        //    }
        //}
        mPushErrorCount = 0;
    }

    m_pSystemSettingDlg->SetCurLine(mPublishLine);
    QString qsConfPath = CPathManager::GetConfigPath();
    ConfigSetting::writeValue(qsConfPath, GROUP_DEFAULT, KEY_VIDEO_LINE, mPublishLine);
    loStreamInfo.mCurRtmpUrlIndex = mPublishLine;
    loStreamInfo.mCurRtmpUrl = loStreamInfo.mPubLineInfo[loStreamInfo.mCurRtmpUrlIndex].mStrDomain;
    TRACE6("%s publishLine[%s]\n", __FUNCTION__, loStreamInfo.mCurRtmpUrl.c_str());
    pCommonData->SetStreamInfo(&loStreamInfo);
    m_iLastLine = mPublishLine;
}

void SysSettingLogic::PushStreamLineSuccess() {
    mPushErrorCount = 0;
}

void SysSettingLogic::SetDesktopShare(bool isCapture) {
   if (m_pSystemSettingDlg) {
      m_pSystemSettingDlg->SetDesktopShare(isCapture);
   }
}

void SysSettingLogic::SetLiveState(bool isLiveing){
   if (m_pSystemSettingDlg) {
      m_pSystemSettingDlg->SetLiveState(isLiveing);
   }
}