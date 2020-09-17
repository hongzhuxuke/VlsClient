#include "DeviceTestingWdg.h"
#include "ConfigSetting.h"
#include "pathmanager.h"
#include <ctime>
#include <QDesktopWidget>
#include <QDesktopServices>
#include <QUrl>
#include "WebRtcSDKInterface.h"
#include "VHPaasSDKCallback.h"
#include "Pathmanager.h"
#include "ConfigSetting.h"
#include "pathManage.h"
#include "AlertTipsDlg.h"
#include "InteractAPIManager.h"
#include "DebugTrace.h"
#include "libyuv/convert_from.h"
#include "libyuv/convert.h"
#include "libyuv/convert_from_argb.h"
#include "libyuv/convert_argb.h"
#include "IDshowEngine.h"
#include "format.hpp"
#include "arraysize.h"

using namespace vlive;
static std::atomic_bool bIsRecvVideo = false;
vlive::IDShowEngine* gDShow;
HWND mHwnd = nullptr;
BITMAPINFO bmi_;
uint8_t* rgb = nullptr;

void SetSize(int type, int width, int height) {
   if (width == bmi_.bmiHeader.biWidth && height == bmi_.bmiHeader.biHeight) {
      return;
   }
   if (rgb) {
      delete[]rgb;
   }
   rgb = new uint8_t[width * height * 4];
   bmi_.bmiHeader.biWidth = width;
   bmi_.bmiHeader.biHeight = height;
   bmi_.bmiHeader.biSizeImage = (width * height * 4);
   bmi_.bmiHeader.biBitCount = 32;
   bmi_.bmiHeader.biPlanes = 1;
   bmi_.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
}

void RecvVideoCallBack(const DShow::VideoConfig &config, unsigned char *data, size_t size, long long startTime, long long stopTime, long rotation);
void HDCRender(int type, HWND desHwnd, LPBYTE bytes, int cx, int cy);

#define MAX_PLAY_TIME   8000

#define AUDIO_FILE  "yisell_sound_48k.pcm"

#define LABEL_SUCESS    QStringLiteral("QLabel{border-image: url(:/ctrl/img/ctrl/big/Device_Sucess.png);background:rgba(82, 204, 144, 1);border-radius:20px; }")

#define LABEL_FAILED    QStringLiteral("QLabel{border-image: url(:/ctrl/img/ctrl/big/Device_Error.png);background:rgba(252, 86, 89, 1);border-radius:20px; }")

#define LABEL_ICO_SUCESS   QStringLiteral("QLabel{border-image: url(:/check/images/device_check/device_ok.png);}")

#define LABEL_ICO_FAILED    QStringLiteral("QLabel{border-image: url(:/check/images/device_check/failed.png);}")

#define LABEL_ICO__SUCESS   QStringLiteral("QLabel{color: rgb(94, 166, 236);}")

#define LABEL_ICO__FAILED    QStringLiteral("QLabel{color:rgba(252,86,89,1);}")

#define LABEL_RESULT_SUCESS    QStringLiteral("QLabel{border-image: url(:/check/images/device_check/device_ok.png);}")

#define LABEL_RESULT_FAILED   QStringLiteral("QLabel{border-image: url(:/check/images/device_check/device_err.png); }")

#define LABEL_TITLE_SUCESS   QStringLiteral("QLabel{color:rgba(94,166,236); font-size:14px;font-family : \"微软雅黑\"; }")

#define LABEL_TITLE_FAILED   QStringLiteral("QLabel{color:rgba(252,86,89); font-size:14px;font-family : \"微软雅黑\";}")


#define CameraLoad QString("border-image: url(:/check/images/device_check/camera.png);")
#define CameraLoadFailed QString("border-image: url(:/check/images/device_check/device_disable.png);")

#define CheckState_CameraRed  QString("background:rgba(242,242,242,1);border-image: url(:/check/images/device_check/camera_red.png);border-radius:20px;")
#define CheckState_CameraGray  QString("background:rgba(242,242,242,1);border-image: url(:/check/images/device_check/camera_gray.png);border-radius:20px;")
#define CheckState_MicRed      QString("background:rgba(242,242,242,1);border-image:url(:/check/images/device_check/mic_red.png);border-radius:20px;")
#define CheckState_MicGray      QString("background:rgba(242,242,242,1);border-image:url(:/check/images/device_check/mic_gray.png);border-radius:20px;")
#define CheckState_PlayerRed      QString("background:rgba(242,242,242,1);border-image:url(:/check/images/device_check/player_red.png);border-radius:20px;")
#define CheckState_PlayerGray      QString("background:rgba(242,242,242,1);border-image:url(:/check/images/device_check/player_gray.png);border-radius:20px;")

#define CHECK_DEV_SUCCESS  QString("background:rgba(242,242,242,1);border-image:url(:/check/images/device_check/check_success.png);border-radius:20px;") 
#define CHECK_DEV_FAILED  QString("background:rgba(242,242,242,1);border-image:url(:/check/images/device_check/check_error.png);border-radius:20px;") 


#define FilePlay_Playing      QString("border-image: url(:/check/images/device_check/icon-play.png);")
#define FilePlay_ToPlay      QString("border-image: url(:/check/images/device_check/icon- pause.png);")

#define LableRed  QString("color: rgb(252, 86, 89);") 
#define LableGray  QString("color: rgb(214, 214, 214);") 

DeviceTestingWdg::DeviceTestingWdg(int liveType, QWidget *parent)
   : mLiveType(liveType),
   QDialog(parent)
{
   this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint | Qt::Tool);
   ui.setupUi(this);
   ui.horizontalSlider_deskTopCaptureVol->setValue(100);
   ui.stackedWidget->setCurrentIndex(DEVICE_CAMERA);
   //setFixedSize(450, 278);
   mQMovie = new QMovie(":/check/images/device_check/loading.gif");
   ui.label_gif->setMovie(mQMovie);
   ui.label_gif->hide();
   mHwnd = (HWND)ui.widget_render->winId();
   ui.widget_micVolume->SetVolumeType(VolumeType_PreviewMic);
   connect(ui.comboBox_camera, SIGNAL(currentIndexChanged(int)), this, SLOT(on_cmbcamera_currentIndexChanged(int)));
   // connect(ui.pushButton_gotest, SIGNAL(clicked()), this, SLOT(slot_Testing()));
   connect(ui.stackedWidget, SIGNAL(currentChanged(int)), this, SLOT(slot_currentChanged(int)));
   connect(ui.pushButton_TestScuess_camera, SIGNAL(clicked()), this, SLOT(slot_TestScuess()));
   connect(ui.pushButton_TestFaile_camera, SIGNAL(clicked()), this, SLOT(slot_TestFailed()));
   connect(ui.pushButton_TestScuess_mic, SIGNAL(clicked()), this, SLOT(slot_TestScuess()));
   connect(ui.pushButton_TestFaile_mic, SIGNAL(clicked()), this, SLOT(slot_TestFailed()));
   connect(ui.pushButton_TestScuess_speak, SIGNAL(clicked()), this, SLOT(slot_TestScuess()));
   connect(ui.pushButton_TestFaile_speak, SIGNAL(clicked()), this, SLOT(slot_TestFailed()));
   connect(ui.pushButton, SIGNAL(clicked()), this, SLOT(slot_TestPlay()));
   connect(ui.pushButton_playState, SIGNAL(clicked()), this, SLOT(slot_TestPlay()));
   
   connect(ui.pushButton_ReDevTesting, SIGNAL(clicked()), this, SLOT(slot_ReDevTesting()));
   connect(ui.close_btn, SIGNAL(clicked()), this, SLOT(slot_Close()));
   connect(ui.pushButton_GoClass, SIGNAL(clicked()), this, SLOT(slot_GoClass()));
   //connect(ui.pushButton_close, SIGNAL(clicked()), this, SLOT(slot_GoClass())); 
   //connect(ui.pushButton_NotDTesting, SIGNAL(clicked()), this, SLOT(slot_GoClass()));
   connect(ui.pushButton_help, SIGNAL(clicked()), this, SLOT(slot_Help()));
   connect(ui.horizontalSlider_deskTopCaptureVol, SIGNAL(valueChanged(int)), this, SLOT(slot_DesktopCaptureValueChanged(int)));
   connect(ui.comboBox_micDev, SIGNAL(currentIndexChanged(int)), this, SLOT(slot_CurrentMicSelectChanged(int)));
   connect(ui.comboBox_playerDev, SIGNAL(currentIndexChanged(int)), this, SLOT(slot_CurrentPlayerSelectChanged(int)));
   connect(ui.widget_micVolume, SIGNAL(sig_havVolume()), this, SLOT(slot_CurrentMicHasVolume()));
   connect(ui.comboBox_info, SIGNAL(currentIndexChanged(int)), this, SLOT(slot_CurrentCameraFormatChanged(int)));

   mCheckRendDataTimer = new QTimer(this);
   connect(mCheckRendDataTimer, SIGNAL(timeout()), this, SLOT(slot_CheckRendData()));
   std::vector<DShow::VideoDevice> devices;
   ui.widget_micVolume->SetLiveType(mLiveType);
   if (mLiveType == eLiveType_VhallActive) {
      ui.widget_29->hide();
   }
   else {
      gDShow = vlive::CreateEngine();
      if (gDShow) {
         gDShow->CreateInputDevice(DShow::DeviceInputType::Video_Input);

         gDShow->GetVideoDevice(devices);
      }
   }
   ui.comboBox_camera->setEnabled(false);
   mDevThread = new std::thread(ThreadProFun, this);
   ui.close_btn->hide();

   mPlayingTimer = new QTimer(this);
   if (mPlayingTimer) {
      connect(mPlayingTimer, SIGNAL(timeout()), this, SLOT(slot_PlayFileTimeout()));
      mPlayingTimer->setSingleShot(true);
   }
   ui.pushButton_TestScuess_camera->setEnabled(false);
   ui.pushButton_TestScuess_speak->setEnabled(false);
   ui.pushButton_TestScuess_mic->setEnabled(false);

   mIPlayerEngine = vlive::CreateEngineInstance();
}

DeviceTestingWdg::~DeviceTestingWdg(){
   if (mPlayingTimer) {
      mPlayingTimer->stop();
   }
   if (mLiveType == eLiveType_Live) {
      VideoLiveStopPreview();
   }
   else {
      GetWebRtcSDKInstance()->StopPreviewCamera();
      GetWebRtcSDKInstance()->SetCurrentPlayVol(100);
   }
   if (mIPlayerEngine) {
      vlive::DestoryEngineInstance();
   }
   if (gDShow) {
      delete gDShow;
      gDShow = nullptr;
   }
}

void DeviceTestingWdg::ThreadProFun(void *obj) {
   if (obj) {
      DeviceTestingWdg *runObj = (DeviceTestingWdg*)(obj);
      runObj->ProcessFun();
   }
}

void DeviceTestingWdg::RtcLiveDevice() {
   playerDevList.clear();
   cameraDevList.clear();
   micDevList.clear();
   std::list<vhall::VideoDevProperty> cameraList;
   GetWebRtcSDKInstance()->GetCameraDevices(cameraList);
   std::list<vhall::VideoDevProperty>::iterator it = cameraList.begin();
   while (it != cameraList.end()) {
      VhallLiveDeviceInfo info(QString::fromStdString(it->mDevId), QString::fromStdString(it->mDevName), it->mIndex);
      cameraDevList.push_back(info);
      it++;
   }

   std::list<vhall::AudioDevProperty> micList;
   GetWebRtcSDKInstance()->GetMicDevices(micList);
   std::list<vhall::AudioDevProperty>::iterator itMic = micList.begin();
   while (itMic != micList.end()) {
      VhallLiveDeviceInfo info(QString::fromStdWString(itMic->mDevGuid), QString::fromStdWString(itMic->mDevName), itMic->mIndex);
      micDevList.push_back(info);
      itMic++;
   }

   std::list<vhall::AudioDevProperty> playerList;
   GetWebRtcSDKInstance()->GetPlayerDevices(playerList);
   std::list<vhall::AudioDevProperty>::iterator itPlayer = playerList.begin();
   while (itPlayer != playerList.end()) {
      VhallLiveDeviceInfo info(QString::fromStdWString(itPlayer->mDevGuid), QString::fromStdWString(itPlayer->mDevName), itPlayer->mIndex);
      playerDevList.push_back(info);
      itPlayer++;
   }
}

void DeviceTestingWdg::VideoLiveDevice() {
   VH::CComPtr<IDeviceManager> pDeviceManager;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IOBSControl, IID_IDeviceManager, pDeviceManager, return, ASSERT(FALSE));
   m_oMicList.clear();
   m_oSpeakerList.clear();
   m_CameraItemMap.clear();

   pDeviceManager->GetVedioDevices(m_CameraItemMap);
   SyncDeviceList(m_CameraItemMap);

   VH::CComPtr<IOBSControlLogic> pOBSControlLogic;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IOBSControl, IID_IOBSControlLogic, pOBSControlLogic, return, ASSERT(FALSE));
   pOBSControlLogic->GetCurrentMic(m_currentMic);
   //获取麦克风列表
   pDeviceManager->GetMicDevices(m_oMicList);
   //获取扬声器列表
   pDeviceManager->GetSpeakerDevices(m_oSpeakerList);
}

void DeviceTestingWdg::ProcessFun() {

   if (mLiveType == eLiveType_VhallActive) {
      RtcLiveDevice();
   }
   else {
      VideoLiveDevice();
   }
   QApplication::postEvent(this, new QEvent(CustomEvent_SelectGetDev));
}

void DeviceTestingWdg::slot_ReDevTesting() {
   if (mDevThread) {
      mDevThread->join();
      delete mDevThread;
      mDevThread = new std::thread(ThreadProFun, this);
   }
   GetWebRtcSDKInstance()->StopPreviewCamera();
   ui.pushButton_TestScuess_camera->setEnabled(false);
   ui.stackedWidget->setCurrentIndex(DEVICE_CAMERA);
   ui.widget_7->show();
   ui.label_camera->setStyleSheet(CheckState_CameraRed);
   ui.label_camera_2->setStyleSheet(LableRed);
   ui.label_camera_2->setText(CHECKING);
   ui.label_mic->setStyleSheet(CheckState_MicGray);
   ui.label_mic_2->setStyleSheet(LableGray);
   ui.label_mic_2->setText(MIC_DEV);
   ui.label_player->setStyleSheet(CheckState_PlayerGray);
   ui.label_player_2->setStyleSheet(LableGray);
   ui.label_player_2->setText(PLAYER_DEV);
   ui.label_gif->show();
   ui.stackedWidget_render->setCurrentIndex(0);
   ui.label_render_camera_ico->hide();
   ui.close_btn->hide();

   ui.widget_26->show();
   ui.pushButton_GoClass->show();
   ui.pushButton_ReDevTesting->show();
}

void DeviceTestingWdg::slot_Close() {

   if (!mbTestCamera || !mbTestMic) {
      AlertTipsDlg tip_notice(CHECK_DEVICE_CLOSE, true, this);
      tip_notice.CenterWindow(this);
      tip_notice.SetYesBtnText(DETERMINE);
      tip_notice.SetNoBtnText(CANCEL);
      if (tip_notice.exec() != QDialog::Accepted) {
         return;
      }
   }
   if (mPlayingTimer) {
      mPlayingTimer->stop();
   }
   if (mLiveType == eLiveType_Live) {
      VideoLiveStopPreview();
   }
   else {
      GetWebRtcSDKInstance()->StopPreviewCamera();
      GetWebRtcSDKInstance()->SetCurrentPlayVol(100);
   }
   if (mIPlayerEngine) {
      mIPlayerEngine->StopPlay();
      vlive::DestoryEngineInstance();
   }
   mIsPlayFile = false;
   accept();
}

void DeviceTestingWdg::RtcLiveGetDevices() {
   ui.comboBox_camera->setEnabled(true);
   ui.comboBox_camera->clear();
   if (cameraDevList.size() > 0) {
      for (int i = 0; i < cameraDevList.size(); i++) {
         DevInfo info;
         info.mDevId = cameraDevList.at(i).devId.toStdString();
         info.index = cameraDevList.at(i).devIndex;
         QVariant var = QVariant::fromValue(info);
         ui.comboBox_camera->addItem(cameraDevList.at(i).devName, var);
      }
      ui.comboBox_camera->setCurrentIndex(0);
   }
   else {
      ui.comboBox_camera->addItem(SETTING_NO_DEV, NO_DEVICE);
      ui.pushButton_TestScuess_camera->setEnabled(false);
      ui.label_render_camera_ico->show();
      ui.label_gif->hide();
      ui.label_CameraOpenState->setText(NO_VIDEO_DEVICES);
      ui.label_render_camera_ico->setStyleSheet(CameraLoadFailed);
   }

   //麦克风显示
   ui.comboBox_micDev->clear();
   if (micDevList.size() > 0) {
      for (int i = 0; i < micDevList.size(); i++) {
         DevInfo info;
         info.mDevId = micDevList.at(i).devId.toStdString();
         info.index = micDevList.at(i).devIndex;
         QVariant var = QVariant::fromValue(info);
         ui.comboBox_micDev->addItem(micDevList.at(i).devName, var);
      }
      ui.comboBox_micDev->setCurrentIndex(0);
   }
   else {
      ui.comboBox_micDev->addItem(SETTING_NO_DEV, NO_DEVICE);
      ui.pushButton_TestScuess_mic->setEnabled(false);
   }

   //扬声器
   ui.comboBox_playerDev->clear();
   if (playerDevList.size() > 0) {
      for (int i = 0; i < playerDevList.size(); i++) {
         DevInfo info;
         info.mDevId = playerDevList.at(i).devId.toStdString();
         info.index = playerDevList.at(i).devIndex;
         QVariant var = QVariant::fromValue(info);
         ui.comboBox_playerDev->addItem(playerDevList.at(i).devName, var);
      }
      ui.comboBox_playerDev->setCurrentIndex(0);
   }
   else {
      ui.comboBox_playerDev->addItem(SETTING_NO_DEV, NO_DEVICE);
      ui.pushButton_TestScuess_speak->setEnabled(false);
   }
   GetWebRtcSDKInstance()->StopPreviewCamera();
   StartPreView();
}

void FuncAppendInfo(FrameInfo info, void *ptr) {
   std::list<FrameInfo> *infos = (std::list<FrameInfo> *)ptr;
   infos->push_back(info);
}

QString GetformatStrByFormat(VideoFormat format) {
   QString str = "[INIT]";
   switch (format) {
   case VideoFormat::Any:
      str = "[Any]";
      break;
   case VideoFormat::Unknown:
      str = "[Unknown]";
      break;
   case VideoFormat::ARGB:
      str = "[ARGB]";
      break;
   case VideoFormat::XRGB:
      str = "[XRGB]";
      break;
   case VideoFormat::I420:
      str = "[I420]";
      break;
   case VideoFormat::NV12:
      str = "[NV12]";
      break;
   case VideoFormat::YV12:
      str = "[YV12]";
      break;
   case VideoFormat::Y800:
      str = "[Y800]";
      break;
   case VideoFormat::YVYU:
      str = "[YVYU]";
      break;
   case VideoFormat::YUY2:
      str = "[YUY2]";
      break;

   case VideoFormat::UYVY:
      str = "[HDYC]";
      break;

   case VideoFormat::HDYC:
      str = "[HDYC]";
      break;

   case VideoFormat::MJPEG:
      str = "[MJPEG]";
      break;

   case VideoFormat::H264:
      str = "[H264]";
      break;
   default:
      str = "[DEFAULT]";
      break;
   }

   return str;
}

void DeviceTestingWdg::GetVideoFormat(DeviceInfo cameraDevInfo) {
   ////去交错
    ui.comboBox_info->clear();
    DeinterlacingType deinterType = DEINTERLACING_NONE;
    //帧率列表
    frameInfoList.infos.clear();
    //当前帧率
    //FrameInfo currentFrameInfo;
    if (cameraDevInfo.m_sDeviceType == TYPE_DSHOW_VIDEO) {
       frameInfoList.appendFunc = FuncAppendInfo;
       //获得视频设备列表
       if (!GetDShowVideoFrameInfoList(cameraDevInfo, &frameInfoList, &currentFrameInfo, deinterType)) {
          return;
       }
       int index = 0;
       int currentIndex = 0;
       disconnect(ui.comboBox_info, SIGNAL(currentIndexChanged(int)), this, SLOT(slot_CurrentCameraFormatChanged(int)));

       m_resolutionList.Clear();
       for (auto itor = frameInfoList.infos.begin(); itor != frameInfoList.infos.end(); itor++) {
          m_resolutionList.Append(*itor);
       }
       m_resolutionList.sort();
       for (int i = 0; i < m_resolutionList.Count(); i++) {
          ui.comboBox_info->addItem(m_resolutionList.DisplayString(i), QVariant(i));
       }

       index = m_resolutionList.SetCurrentFrameInfo(currentFrameInfo);

       ui.comboBox_info->setCurrentIndex(index);
       connect(ui.comboBox_info, SIGNAL(currentIndexChanged(int)), this, SLOT(slot_CurrentCameraFormatChanged(int)));
    }
    else if (cameraDevInfo.m_sDeviceType == TYPE_DECKLINK) {
       UINT w, h;
       int frameInternal;
       UINT Twidth = 0;
       UINT Theight = 0;
       int TframeInternal = 0;
       VideoFormat format;
       if (!GetDeckLinkDeviceInfo(cameraDevInfo, w, h, frameInternal)) {
          return;
       }

       currentFrameInfo.minFrameInterval = currentFrameInfo.maxFrameInterval = frameInternal;
       currentFrameInfo.minCX = currentFrameInfo.maxCX = w;
       currentFrameInfo.minCY = currentFrameInfo.maxCY = h;
       frameInfoList.PushBack(currentFrameInfo);
       GetDeviceDefaultAttribute(cameraDevInfo, Twidth, Theight, TframeInternal, deinterType, format);
    }
   //// 去交错
   // ReloadCameraDeinterLace(deinterType);
   // //分辨率帧率
   // ReloadCameraResolution(frameInfoList, currentFrameInfo);
}

void DeviceTestingWdg::VideoLiveGetDevice() {
   ui.comboBox_camera->setEnabled(true);
   ui.comboBox_camera->clear();

   int camera_index = 0;
   if (m_CameraItemMap.size() > 0) {       
      disconnect(ui.comboBox_camera, SIGNAL(currentIndexChanged(int)), this, SLOT(on_cmbcamera_currentIndexChanged(int)));
      std::list<DeviceInfo>::iterator iterator = m_CameraItemMap.begin();
      while (iterator != m_CameraItemMap.end()) {
         if(camera_index == 0){
            m_LastCameraDevInfo = *iterator;
         }
         camera_index++;
         DevInfo info;
         info.mDevId = QString::fromStdWString(iterator->m_sDeviceID).toStdString();
         info.index = iterator->mIndex;
         QVariant var = QVariant::fromValue(info);
         ui.comboBox_camera->addItem(QString::fromStdWString(iterator->m_sDeviceDisPlayName), var);
         iterator++;
      }
      ui.comboBox_camera->setCurrentIndex(0);
      connect(ui.comboBox_camera, SIGNAL(currentIndexChanged(int)), this, SLOT(on_cmbcamera_currentIndexChanged(int)));
      
      AddCapture();
      mCheckRendDataTimer->start(2000);
   }
   else {
      ui.comboBox_camera->addItem(SETTING_NO_DEV, NO_DEVICE);
      ui.pushButton_TestScuess_camera->setEnabled(false);
      ui.label_render_camera_ico->show();
      ui.label_gif->hide();
      ui.label_CameraOpenState->setText(NO_VIDEO_DEVICES);
      ui.label_render_camera_ico->setStyleSheet(CameraLoadFailed);
   }

   //麦克风显示
   disconnect(ui.comboBox_micDev, SIGNAL(currentIndexChanged(int)), this, SLOT(slot_CurrentMicSelectChanged(int)));
   int select_mic = 0;
   int index = 0;
   ui.comboBox_micDev->clear();
   if (m_oMicList.size() > 0) {
      std::list<DeviceInfo>::iterator iterator = m_oMicList.begin();
      while (iterator != m_oMicList.end()) {
         DevInfo info;
         info.mDevId = QString::fromStdWString(iterator->m_sDeviceID).toStdString();
         info.index = iterator->mIndex;
         QVariant var = QVariant::fromValue(info);
         ui.comboBox_micDev->addItem(QString::fromStdWString(iterator->m_sDeviceDisPlayName), var);
         QString audioConf = CPathManager::GetAudiodevicePath();
         QString b64Str = ConfigSetting::ReadString(audioConf, GROUP_DEFAULT, AUDIO_DEFAULT_DEVICE, "");
         if (b64Str != "") {
            QByteArray ba = QByteArray::fromBase64(b64Str.toLocal8Bit());
            if (ba.length() > 0) {
               DeviceInfo itemData = *(DeviceInfo *)ba.data();
               if (QString::fromStdWString(itemData.m_sDeviceID).toStdString() == info.mDevId &&
                   QString::fromStdWString(itemData.m_sDeviceDisPlayName) == QString::fromStdWString(iterator->m_sDeviceDisPlayName)) {
                  select_mic = index;
               }
            }
         }
         index++;
         iterator++;
      }
      ui.comboBox_micDev->setCurrentIndex(select_mic);
   }
   else {
      ui.comboBox_micDev->addItem(SETTING_NO_DEV, NO_DEVICE);
      ui.pushButton_TestScuess_mic->setEnabled(false);
   }
   connect(ui.comboBox_micDev, SIGNAL(currentIndexChanged(int)), this, SLOT(slot_CurrentMicSelectChanged(int)));

   //扬声器
   disconnect(ui.comboBox_playerDev, SIGNAL(currentIndexChanged(int)), this, SLOT(slot_CurrentPlayerSelectChanged(int)));

   select_mic = 0;
   index = 0;
   ui.comboBox_playerDev->clear();
   if (m_oSpeakerList.size() > 0) {
      std::list<DeviceInfo>::iterator iterator = m_oSpeakerList.begin();
      while (iterator != m_oSpeakerList.end()) {
         DevInfo info;
         info.mDevId = QString::fromStdWString(iterator->m_sDeviceID).toStdString();
         info.index = iterator->mIndex;
         QVariant var = QVariant::fromValue(info);
         ui.comboBox_playerDev->addItem(QString::fromStdWString(iterator->m_sDeviceDisPlayName), var);
         QString strCurDevid = ConfigSetting::ReadString(CPathManager::GetConfigPath(), GROUP_DEFAULT, PLAYER_DEFAULT_DEVICE, "");
         if (strCurDevid.toStdString() == info.mDevId) {
            select_mic = index;
         }
         index++;
         iterator++;
      }
      ui.comboBox_playerDev->setCurrentIndex(select_mic);
   }
   else {
      ui.comboBox_playerDev->addItem(SETTING_NO_DEV, NO_DEVICE);
      ui.pushButton_TestScuess_speak->setEnabled(false);
   }
   connect(ui.comboBox_playerDev, SIGNAL(currentIndexChanged(int)), this, SLOT(slot_CurrentPlayerSelectChanged(int)));

}

void DeviceTestingWdg::customEvent(QEvent* e) {
   if (e && e->type() == CustomEvent_SelectGetDev) {
      if (mLiveType == eLiveType_VhallActive) {
         RtcLiveGetDevices();
      }
      else {
         VideoLiveGetDevice();
      }   
   }
}

void DeviceTestingWdg::slot_GoClass() {
   slot_Close();
}


void DeviceTestingWdg::mousePressEvent(QMouseEvent *event) {
   if (event) {
      mPressPoint = this->pos() - event->globalPos();
      mIsMoved = true;
   }
}

void DeviceTestingWdg::mouseMoveEvent(QMouseEvent *event) {
   if (event && (event->buttons() == Qt::LeftButton) && mIsMoved  && m_bIsEnableMove) {
      this->move(event->globalPos() + mPressPoint);
      this->raise();
   }
}

void DeviceTestingWdg::mouseReleaseEvent(QMouseEvent *) {
   if (mIsMoved) {
      mIsMoved = false;
   }
   QPoint pos = this->pos();
   QRect rect = QApplication::desktop()->availableGeometry(pos);
   QRect wnd_rect = this->frameGeometry();

   if (pos.y() > rect.height() - 150) {
      pos.setY(rect.height() - 150);
      this->move(pos);
   }
   else if (pos.y() < rect.y()) {
      this->move(pos + QPoint(0, +100));
   }
   else if (wnd_rect.x() >= rect.x() + rect.width() - 100) {
      this->move(QPoint(wnd_rect.x(), wnd_rect.y()) + QPoint(-100, 0));
   }
   else if (wnd_rect.x() + this->width() < rect.x() + 100) {
      this->move(QPoint(wnd_rect.x(), wnd_rect.y()) + QPoint(100, 0));
   }
}

void DeviceTestingWdg::keyPressEvent(QKeyEvent *event) {
   switch (event->key())
   {
   case Qt::Key_Escape:
      break;
   default:
      QDialog::keyPressEvent(event);
   }
}


void DeviceTestingWdg::slot_Help() {
   QDesktopServices::openUrl(QUrl("https://www.vhall.com/saas/doc/1722.html"));
}

void DeviceTestingWdg::slot_Testing() {

}

void DeviceTestingWdg::slot_CurrentMicSelectChanged(int index) {
   if (mLiveType == eLiveType_VhallActive) {
      ui.pushButton_TestScuess_mic->setEnabled(false);
      DevInfo info;
      info = ui.comboBox_micDev->currentData().value<DevInfo>();
      DevInfo playerInfo;
      playerInfo = ui.comboBox_playerDev->currentData().value<DevInfo>();
      int nRet = GetWebRtcSDKInstance()->SetUsedMic(info.index, info.mDevId, L"");
      GetWebRtcSDKInstance()->SetCurrentMicVol(100);
      wstring confPath = GetAppDataPath() + VHALL_TOOL_CONFIG;
      QString qsConfPath = qsConfPath.fromStdWString(confPath);
      ConfigSetting::writeValue(qsConfPath, GROUP_DEFAULT, CAPTURE_MIC_ID, QString::fromStdString(info.mDevId));
   }
   else {
      VH::CComPtr<ISettingLogic> pSettingLogic;
      DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IMainUI, IID_ISettingLogic, pSettingLogic, return, ASSERT(FALSE));
      DeviceInfo cur_mic;
      if (m_oMicList.size() > 0 && index >= 0) {
         std::list<DeviceInfo>::iterator iterator = m_oMicList.begin();
         int select_index = 0;
         while (iterator != m_oMicList.end()){
            if (select_index == index) {
               cur_mic = *iterator;
               pSettingLogic->SetPriviewMic(cur_mic);
               QString audioConf = CPathManager::GetAudiodevicePath();
               QByteArray ba;
               ba.setRawData((const char *)&cur_mic, sizeof(DeviceInfo));
               QString str = QString::fromLocal8Bit(ba.toBase64());
               ConfigSetting::writeValue(audioConf, GROUP_DEFAULT, AUDIO_DEFAULT_DEVICE, str);
               break;
            }
            select_index++;
            iterator++;
         }      
      }

   }
}

void DeviceTestingWdg::slot_CurrentMicHasVolume() {
   ui.pushButton_TestScuess_mic->setEnabled(true);
}

void DeviceTestingWdg::slot_CheckRendData() {
   if (m_LastCameraDevInfo.m_sDeviceType == TYPE_DSHOW_VIDEO) {
      if (bIsRecvVideo) {
         HandlePreviewCamera(true, 0);
      }
      else {
         HandlePreviewCamera(false, 0);
      }
   }
   else {
      VH::CComPtr<IOBSControlLogic> pObsControlLogic = NULL;
      DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IOBSControl, IID_IOBSControlLogic, pObsControlLogic, return);
      int nRet = pObsControlLogic->GetRendState(m_LastCameraDevInfo);
      if (nRet == 1) {
         HandlePreviewCamera(true, 0);
      }
      else {
         HandlePreviewCamera(false, 0);
      }
   }
}

void DeviceTestingWdg::slot_CurrentCameraFormatChanged(int index) {
   if (index < 0) {
      return;
   }
   int currentIndex = 0;
   CameraSettingUIResolution curInfo;
   m_resolutionList.GetFrameInfo(index,&curInfo);
   if (m_LastCameraDevInfo.m_sDeviceType == TYPE_RECODING_SOURCE::TYPE_DSHOW_VIDEO) {
      DShow::VideoConfig video_config;
      video_config.recvVideoCallback = RecvVideoCallBack;
      video_config.cx = curInfo.w;
      video_config.cy_abs = curInfo.h;
      video_config.format = (DShow::VideoFormat)curInfo.format;
      video_config.frameInterval = curInfo.frameInternalList[0].minInternal;
      video_config.internalFormat = (DShow::VideoFormat)curInfo.format;
      video_config.name = m_LastCameraDevInfo.m_sDeviceName;
      video_config.path = m_LastCameraDevInfo.m_sDeviceID;
      bIsRecvVideo = false;
      gDShow->StartCaptureDevice(video_config);
   }
   else {
      int width = curInfo.w;
      int height = curInfo.h;
      //帧率
      UINT64 ui64FrameNum = 333333;
      VideoFormat format = curInfo.format;
      ui64FrameNum = 10000000.0 / curInfo.frameInternalList[0].minInternal;
      //设置分辨率、帧率、去交错
      SetDeviceDefaultAttribute(m_LastCameraDevInfo, width, height, ui64FrameNum, DEINTERLACING_NONE, format);

      STRU_OBSCONTROL_VIDIO_SET vidioSet;
      vidioSet.m_DeviceInfo = m_LastCameraDevInfo;
      SingletonMainUIIns::Instance().PostCRMessage(MSG_OBSCONTROL_VIDIO_HIGHQUALITYCOD, &vidioSet, sizeof(STRU_OBSCONTROL_VIDIO_SET));
   }

}

void DeviceTestingWdg::slot_CurrentPlayerSelectChanged(int index) {
   mPlayingTimer->stop();
   slot_PlayFileTimeout();
   
   if (mLiveType == eLiveType_Live) {
      if (m_oSpeakerList.size() > 0) {
         DevInfo info;
         info = ui.comboBox_playerDev->currentData().value<DevInfo>();
         ConfigSetting::writeValue(CPathManager::GetConfigPath(), GROUP_DEFAULT, PLAYER_DEFAULT_DEVICE, QString::fromStdString(info.mDevId));
         VH::CComPtr<IOBSControlLogic> pObsControlLogic = NULL;
         DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IOBSControl, IID_IOBSControlLogic, pObsControlLogic, return, ASSERT(FALSE));
         pObsControlLogic->InitPlayDevice(QString::fromStdString(info.mDevId).toStdWString().c_str());
         GetWebRtcSDKInstance()->SetUsedPlay(info.index, info.mDevId);
      }
   }
   else {
      DevInfo info;
      info = ui.comboBox_playerDev->currentData().value<DevInfo>();
      GetWebRtcSDKInstance()->SetUsedPlay(info.index, info.mDevId);
      wstring confPath = GetAppDataPath() + VHALL_TOOL_CONFIG;
      QString qsConfPath = qsConfPath.fromStdWString(confPath);
      ConfigSetting::writeValue(qsConfPath, GROUP_DEFAULT, PLAY_OUT_DEV_ID, QString::fromStdString(info.mDevId));
   }
   ui.pushButton_TestScuess_speak->setEnabled(false);
}


void DeviceTestingWdg::slot_DesktopCaptureValueChanged(int value) {
   if (mIPlayerEngine) {
      mIPlayerEngine->SetPlayVolume(value);
   }
}

void DeviceTestingWdg::on_cmbcamera_currentIndexChanged(int index) {
   StartPreView();
   ui.pushButton_TestScuess_camera->setEnabled(false);
}

void DeviceTestingWdg::slot_PlayFileTimeout() {
   if (mIPlayerEngine) {
      mIPlayerEngine->StopPlay();
   }
   mIsPlayFile = false;
   ui.pushButton_playState->setStyleSheet(FilePlay_ToPlay);
}

void DeviceTestingWdg::slot_TestPlay() {
   TRACE6("%s\n",__FUNCTION__);
   if (ui.comboBox_playerDev->count() > 0) {
      DevInfo playOutinfo = ui.comboBox_playerDev->currentData().value<DevInfo>();
      QString appPath = QCoreApplication::applicationDirPath();
      QString fileName = appPath + QString("/") + AUDIO_FILE;
      TRACE6("%s fileName %s\n", __FUNCTION__, fileName.toStdString().c_str());
      if (mIsPlayFile) {
         mIPlayerEngine->StopPlay();
         TRACE6("%s StopPlayAudioFile\n", __FUNCTION__);
         mIsPlayFile = false;
         ui.pushButton_playState->setStyleSheet(FilePlay_ToPlay);
         if (mPlayingTimer) {
            mPlayingTimer->stop();
         }
      }
      else {
         mIPlayerEngine->CreatePlayEngine(playOutinfo.mDevId.c_str(),(HWND)this->winId());
         int nRet = mIPlayerEngine->PlayAudio(48000, 2, 16, fileName.toStdString().c_str());
         TRACE6("%s nRet %d\n", __FUNCTION__, nRet);
         if (nRet == 0) {
            mIsPlayFile = true;
            ui.pushButton_playState->setStyleSheet(FilePlay_Playing);
            if (mPlayingTimer) {
               mPlayingTimer->start(MAX_PLAY_TIME);
            }
            ui.pushButton_TestScuess_speak->setEnabled(true);
         }

      }
   }
   else {
      ui.pushButton_TestScuess_speak->setEnabled(false);
   }
}

void DeviceTestingWdg::SetMicVolume(int value) {
   if (m_oMicList.size() > 0) {
      ui.widget_micVolume->SetMicVolume(value);
   }
}

void DeviceTestingWdg::HandlePreviewCamera(bool success, int code) {
   if (success) {
      ui.stackedWidget_render->setCurrentIndex(1);
      ui.pushButton_TestScuess_camera->setEnabled(true);
   }
   else {
      ui.stackedWidget_render->setCurrentIndex(0);
      ui.label_render_camera_ico->show();
      ui.label_CameraOpenState->setText(VIDEO_CAPTURE_OPEN_ERROR);
      ui.label_CameraOpenState->show();
      ui.label_gif->hide();
      ui.pushButton_TestScuess_camera->setEnabled(false);
   }
   if (mQMovie) {
      mQMovie->stop();
   }
}

void DeviceTestingWdg::SetRole(bool is_host) {
   if (is_host) {
      ui.pushButton_GoClass->setText(QString::fromStdWString(L"去直播"));
   }
   else {
      ui.pushButton_GoClass->setText(QString::fromStdWString(L"去互动"));
   }
}

void DeviceTestingWdg::RtcLiveStartPreView() {
   DevInfo dev;
   dev = ui.comboBox_camera->currentData().value<DevInfo>();
   if (dev.mDevId.length() == 0) {
      return;
   }
   ui.label_render_camera_ico->setStyleSheet(CameraLoad);
   std::string devId = dev.mDevId;
   HWND id = (HWND)(ui.widget_render->winId());
   if (mQMovie) {
      mQMovie->start();
   }

   ui.label_CameraOpenState->setText(VIDEO_CAPTURE_LOADING);
   ui.label_CameraOpenState->show();

   ui.label_gif->show();
   ui.stackedWidget_render->setCurrentIndex(0);
   ui.label_render_camera_ico->hide();
   GetWebRtcSDKInstance()->StopPreviewCamera();
   if (micDevList.size() > 0) {
      DevInfo info;
      info = ui.comboBox_micDev->currentData().value<DevInfo>();
      if (GetWebRtcSDKInstance()->StartPreviewCamera((void*)id, devId, RTC_VIDEO_PROFILE_480P_4x3_M, info.index) == 0) {
         ui.comboBox_camera->setEnabled(true);
      }
   }
   else {
      if (GetWebRtcSDKInstance()->StartPreviewCamera((void*)id, devId, RTC_VIDEO_PROFILE_480P_4x3_M) == 0) {
         ui.comboBox_camera->setEnabled(true);
      }
   }
   wstring confPath = GetAppDataPath() + VHALL_TOOL_CONFIG;
   QString qsConfPath = qsConfPath.fromStdWString(confPath);
   ConfigSetting::writeValue(qsConfPath, GROUP_DEFAULT, CAPTURE_CAMERA_ID, QString::fromStdString(devId));
}

void DeviceTestingWdg::VideoLiveStopPreview() {
   mCheckRendDataTimer->stop();
   if (m_LastCameraDevInfo.m_sDeviceType == TYPE_RECODING_SOURCE::TYPE_DSHOW_VIDEO) {
      if (gDShow) {
         gDShow->StopCapture();
      }
   }
   else {
      STRU_OBSCONTROL_ADDCAMERA  loAddCamera;
      loAddCamera.m_deviceInfo = m_LastCameraDevInfo;
      loAddCamera.m_PosType = enum_PosType_auto;
      loAddCamera.m_dwType = device_operator_del;
      loAddCamera.m_PriviewRenderHwnd = (HWND)ui.widget_render->winId();
      loAddCamera.m_HideSourceItem = 1;
      TRACE6("%s device_operator_del\n", __FUNCTION__);
      VH::CComPtr<IOBSControlLogic> pObsControlLogic = NULL;
      DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IOBSControl, IID_IOBSControlLogic, pObsControlLogic, return);
      pObsControlLogic->DealAddCameraSync(&loAddCamera, sizeof(STRU_OBSCONTROL_ADDCAMERA));
   }
   TRACE6("%s device_operator_del end\n", __FUNCTION__);
}

void DeviceTestingWdg::VideoLiveStartPreView() {
   VideoLiveStopPreview();

   DevInfo dev;
   dev = ui.comboBox_camera->currentData().value<DevInfo>();
   ui.label_render_camera_ico->setStyleSheet(CameraLoad);
   std::string devId = dev.mDevId;
   QString dev_name = ui.comboBox_camera->currentText();
   HWND id = (HWND)(ui.widget_render->winId());
   if (mQMovie) {
      mQMovie->start();
   }

   ui.label_CameraOpenState->setText(VIDEO_CAPTURE_LOADING);
   ui.label_CameraOpenState->show();

   ui.label_gif->show();
   ui.stackedWidget_render->setCurrentIndex(0);
   ui.label_render_camera_ico->hide();

   bool hasVideoDev = false;
   if (m_CameraItemMap.size() > 0) {
      std::list<DeviceInfo>::iterator iterator = m_CameraItemMap.begin();
      while (iterator != m_CameraItemMap.end()) {
         if (devId == QString::fromStdWString(iterator->m_sDeviceID).toStdString() || dev_name == QString::fromStdWString(iterator->m_sDeviceName)) {
            m_LastCameraDevInfo = *iterator;
            hasVideoDev = true;
            break;
         }
         iterator++;
      }
   }
   if (hasVideoDev) {
      AddCapture();
   }
   mCheckRendDataTimer->start(2000);
}

void DeviceTestingWdg::AddCapture() {
   GetVideoFormat(m_LastCameraDevInfo);
   if (m_LastCameraDevInfo.m_sDeviceType == TYPE_DSHOW_VIDEO) {
      DShow::VideoConfig video_config;
      video_config.recvVideoCallback = RecvVideoCallBack;
      video_config.cx = currentFrameInfo.maxCX;
      video_config.cy_abs = currentFrameInfo.maxCY;
      video_config.format = (DShow::VideoFormat)currentFrameInfo.format;
      video_config.frameInterval = currentFrameInfo.minFrameInterval;
      video_config.internalFormat = (DShow::VideoFormat)currentFrameInfo.format;
      video_config.name = m_LastCameraDevInfo.m_sDeviceName;
      video_config.path = m_LastCameraDevInfo.m_sDeviceID;
      bIsRecvVideo = false;
      gDShow->StartCaptureDevice(video_config);
   }
   else {
      STRU_OBSCONTROL_ADDCAMERA  loAddCamera;
      loAddCamera.m_deviceInfo = m_LastCameraDevInfo;
      loAddCamera.m_PosType = enum_PosType_auto;
      loAddCamera.m_dwType = device_operator_add;
      loAddCamera.m_PriviewRenderHwnd = (HWND)ui.widget_render->winId();
      loAddCamera.m_HideSourceItem = 1;
      VH::CComPtr<IOBSControlLogic> pObsControlLogic = NULL;
      DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IOBSControl, IID_IOBSControlLogic, pObsControlLogic, return);
      TRACE6("%s DealAddCameraSync\n", __FUNCTION__);
      pObsControlLogic->DealAddCameraSync(&loAddCamera, sizeof(STRU_OBSCONTROL_ADDCAMERA));
      TRACE6("%s DealAddCameraSync end\n", __FUNCTION__);
   }
}

void DeviceTestingWdg::StartPreView() {
   if (mLiveType == eLiveType_VhallActive) {
      RtcLiveStartPreView();
   }
   else {
      VideoLiveStartPreView();
   }
}

void DeviceTestingWdg::slot_currentChanged(int index) {
   if (index == DEVICE_RESULT) {
      if (mbTestMic == false || mbTestCamera == false || mbTestSpeaker == false) {
         ui.label_result->setStyleSheet(LABEL_RESULT_FAILED);
         ui.label_title->setText(CHECKING_RESULT_ERR);
         ui.label_title->setStyleSheet(LABEL_TITLE_FAILED);
         ui.widget_notice->show();
      }
      else {
         ui.label_result->setStyleSheet(LABEL_RESULT_SUCESS);
         ui.label_title->setText(CHECKING_RESULT_SUC);
         ui.label_title->setStyleSheet(LABEL_TITLE_SUCESS);
         ui.widget_notice->hide();
      }
      if (mLiveType == eLiveType_Live) {
         //1.麦克风+者扬声器检查失败，只显示重新检查。显示关闭。
         if (mbTestMic == false && mbTestSpeaker == false) {
            ui.widget_26->hide();
            ui.pushButton_GoClass->hide();
            ui.close_btn->show();
            ui.pushButton_ReDevTesting->show();
         }
         else {
            if (mbTestMic == false || mbTestSpeaker == false || mbTestCamera == false) {
               ui.widget_26->show();
               ui.pushButton_GoClass->show();
               ui.pushButton_ReDevTesting->show();
            }
            else {
               ui.pushButton_ReDevTesting->hide();
               ui.widget_26->hide();
            }
            ui.close_btn->hide();
         }
         ui.widget_7->hide();
      }
      else{
         VH::CComPtr<ICommonData> pCommonData;
         DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
         ClientApiInitResp respInitData;
         pCommonData->GetInitApiResp(respInitData);
         //1.麦克风或者摄像头检查失败，只显示重新检查。显示关闭。
         if (mbTestMic == false || mbTestCamera == false) {
            ui.widget_26->hide();
            ui.pushButton_GoClass->hide();
            ui.close_btn->show();
            ui.pushButton_ReDevTesting->show();
            InteractAPIManager apiManager(NULL);
            apiManager.HttpSendSetDevice(2); //2不可上麦
         }
         else {
            InteractAPIManager apiManager(NULL);
            apiManager.HttpSendSetDevice(1); //1可上麦
            if (mbTestSpeaker) {
               ui.pushButton_ReDevTesting->hide();
               ui.widget_26->hide();
            }
            else {
               ui.widget_26->show();
               ui.pushButton_GoClass->show();
               ui.pushButton_ReDevTesting->show();
            }
            ui.close_btn->hide();
         }
         ui.widget_7->hide();
      }
      
   }
   else {
      ui.close_btn->hide();
      ui.widget_7->show();
   }
}

void DeviceTestingWdg::slot_TestScuess() {
   if (ui.stackedWidget->currentIndex() == DEVICE_CAMERA) {
      mbTestCamera = true;
      //更新样式
      ui.label_camera->setStyleSheet(CHECK_DEV_SUCCESS);
      ui.label_camera_2->setStyleSheet(LableGray);
      ui.label_camera_2->setText(CAMERA_DEV);
      ui.label_mic->setStyleSheet(CheckState_MicRed);
      ui.label_mic_2->setStyleSheet(LableRed);
      ui.label_mic_2->setText(CHECKING);
      ui.label_player->setStyleSheet(CheckState_PlayerGray);
      ui.label_player_2->setStyleSheet(LableGray);
      ui.label_ico_c->setText(QStringLiteral("正常"));
      ui.label_ico_c->setStyleSheet(LABEL_ICO__SUCESS);
      ui.label_ico_camera->setStyleSheet(LABEL_ICO_SUCESS);
   }
   else if (ui.stackedWidget->currentIndex() == DEVICE_MIC) {
      mbTestMic = true;
      //更新样式
      ui.label_camera_2->setStyleSheet(LableGray);
      ui.label_camera_2->setText(CAMERA_DEV);
      ui.label_mic->setStyleSheet(CHECK_DEV_SUCCESS);
      ui.label_mic_2->setStyleSheet(LableGray);
      ui.label_mic_2->setText(MIC_DEV);
      ui.label_player->setStyleSheet(CheckState_PlayerRed);
      ui.label_player_2->setStyleSheet(LableRed);
      ui.label_player_2->setText(CHECKING);
      ui.label_ico_m->setText(QStringLiteral("正常"));
      ui.label_ico_m->setStyleSheet(LABEL_ICO__SUCESS);
      ui.label_ico_mic->setStyleSheet(LABEL_ICO_SUCESS);
   }
   else if (ui.stackedWidget->currentIndex() == DEVICE_SPEAKER) {
      mbTestSpeaker = true;
      if (mIPlayerEngine) {
         mIPlayerEngine->StopPlay();
      }
      if (mPlayingTimer) {
         mPlayingTimer->stop();
      }
      mIsPlayFile = false;
      ui.pushButton_playState->setStyleSheet(FilePlay_ToPlay);
      ui.label_ico_s->setText(QStringLiteral("正常"));
      ui.label_ico_s->setStyleSheet(LABEL_ICO__SUCESS);
      ui.label_ico_speak->setStyleSheet(LABEL_ICO_SUCESS);
      ui.widget_7->hide();
   }
   ui.stackedWidget->setCurrentIndex(ui.stackedWidget->currentIndex() + 1);
}

void DeviceTestingWdg::slot_TestFailed() {
   if (ui.stackedWidget->currentIndex() == DEVICE_CAMERA) {
      mbTestCamera = false;
      //更新样式
      ui.label_camera->setStyleSheet(CHECK_DEV_FAILED);
      ui.label_camera_2->setStyleSheet(LableGray);
      ui.label_camera_2->setText(CAMERA_DEV);
      ui.label_mic->setStyleSheet(CheckState_MicRed);
      ui.label_mic_2->setStyleSheet(LableRed);
      ui.label_mic_2->setText(CHECKING);
      ui.label_player->setStyleSheet(CheckState_PlayerGray);
      ui.label_player_2->setStyleSheet(LableGray);
      ui.label_ico_c->setText(QStringLiteral("异常"));
      ui.label_ico_c->setStyleSheet(LABEL_ICO__FAILED);
      ui.label_ico_camera->setStyleSheet(LABEL_ICO_FAILED);
   }
   else if (ui.stackedWidget->currentIndex() == DEVICE_MIC) {
      mbTestMic = false;
      //更新样式
      ui.label_camera_2->setStyleSheet(LableGray);
      ui.label_camera_2->setText(CAMERA_DEV);
      ui.label_mic->setStyleSheet(CHECK_DEV_FAILED);
      ui.label_mic_2->setStyleSheet(LableGray);
      ui.label_mic_2->setText(MIC_DEV);
      ui.label_player->setStyleSheet(CheckState_PlayerRed);
      ui.label_player_2->setStyleSheet(LableRed);
      ui.label_player_2->setText(CHECKING);
      ui.label_ico_m->setText(QStringLiteral("异常"));
      ui.label_ico_m->setStyleSheet(LABEL_ICO__FAILED);
      ui.label_ico_mic->setStyleSheet(LABEL_ICO_FAILED);
   }
   else if (ui.stackedWidget->currentIndex() == DEVICE_SPEAKER) {
      mbTestSpeaker = false;
      if (mIPlayerEngine) {
         mIPlayerEngine->StopPlay();
      }
      if (mPlayingTimer) {
         mPlayingTimer->stop();
      }
      mIsPlayFile = false;
      ui.pushButton_playState->setStyleSheet(FilePlay_ToPlay);
      ui.label_ico_s->setText(QStringLiteral("异常"));
      ui.label_ico_s->update();
      ui.label_ico_s->setStyleSheet(LABEL_ICO__FAILED);
      ui.widget_7->hide();
      ui.label_ico_speak->setStyleSheet(LABEL_ICO_FAILED);
      ui.close_btn->show();
   }
   ////麦克风或者摄像头检查失败，不显示去上课。
   //if (mbTestMic == false || mbTestCamera == false) {
   //   ui.widget_26->hide();
   //   ui.pushButton_GoClass->hide();
   //   ui.close_btn->show();
   //}
   //else {
   //   ui.widget_26->show();
   //   ui.pushButton_GoClass->show();
   //}

   //ui.pushButton_ReDevTesting->show();
   //ui.label_result->setStyleSheet(LABEL_RESULT_FAILED);
   //ui.label_title->setText(CHECKING_RESULT_ERR);
   //ui.label_title->setStyleSheet(LABEL_TITLE_FAILED);
   ui.stackedWidget->setCurrentIndex(ui.stackedWidget->currentIndex() + 1);

}

void RecvVideoCallBack(const DShow::VideoConfig &config, unsigned char *data, size_t size, long long startTime, long long stopTime, long rotation) {
   HDCRender((int)config.format, mHwnd, data, config.cx, config.cy_abs);
}

void HDCRender(int type, HWND desHwnd, LPBYTE bytes, int cx, int cy) {
   if (desHwnd == NULL) {
      return;
   }
   bIsRecvVideo = true;
   SetSize(type, cx, cy);
   HWND wnd_ = desHwnd;
   RECT rc;
   ::GetClientRect(wnd_, &rc);

   BITMAPINFO& bmi = bmi_;
   int height = abs(bmi.bmiHeader.biHeight);
   int width = bmi.bmiHeader.biWidth;

   const uint8_t* image = bytes;
   if (image != NULL) {
      auto mWindowDC = ::GetDC(wnd_);
      HDC dc_mem = ::CreateCompatibleDC(mWindowDC);
      ::SetStretchBltMode(dc_mem, HALFTONE);

      // Set the map mode so that the ratio will be maintained for us.
      HDC all_dc[] = { mWindowDC, dc_mem };
      for (int i = 0; i < arraysize(all_dc); ++i) {
         SetMapMode(all_dc[i], MM_ISOTROPIC);
         SetWindowExtEx(all_dc[i], width, height, NULL);
         SetViewportExtEx(all_dc[i], rc.right, rc.bottom, NULL);
      }

      HBITMAP bmp_mem = ::CreateCompatibleBitmap(mWindowDC, rc.right, rc.bottom);
      HGDIOBJ bmp_old = ::SelectObject(dc_mem, bmp_mem);

      POINT logical_area = { rc.right, rc.bottom };
      DPtoLP(mWindowDC, &logical_area, 1);

      HBRUSH brush = ::CreateSolidBrush(RGB(0, 0, 0));
      RECT logical_rect = { 0, 0, logical_area.x, logical_area.y };
      ::FillRect(dc_mem, &logical_rect, brush);
      ::DeleteObject(brush);

      int x = (logical_area.x >> 1) - (width >> 1);
      int y = (logical_area.y >> 1) - (height >> 1);

      if (DShow::VideoFormat::XRGB == (DShow::VideoFormat)type) {
         StretchDIBits(dc_mem, x, y, width, height, 0, 0, width, height, image, &bmi, DIB_RGB_COLORS, SRCCOPY);
         BitBlt(mWindowDC, 0, 0, logical_area.x, logical_area.y, dc_mem, 0, 0, SRCCOPY);
      }
      else if (DShow::VideoFormat::I420 == (DShow::VideoFormat)type || DShow::VideoFormat::YV12 == (DShow::VideoFormat)type) {
         uint8_t* src_u = bytes + width * height;
         uint8_t* src_v = src_u + width * (height >> 2);
         bmi.bmiHeader.biHeight = -bmi.bmiHeader.biHeight;
         if (DShow::VideoFormat::I420 == (DShow::VideoFormat)type) {
            libyuv::I420ToARGB(bytes, width, src_u, width >> 1, src_v, width >> 1, rgb, width * 4, width, height);
         }
         else {
            libyuv::I420ToARGB(bytes, width, src_v, width >> 1, src_u, width >> 1, rgb, width * 4, width, height);
         }
         StretchDIBits(dc_mem, x, y, width, height, 0, 0, width, height, rgb, &bmi, DIB_RGB_COLORS, SRCCOPY);
         BitBlt(mWindowDC, 0, 0, logical_area.x, logical_area.y, dc_mem, 0, 0, SRCCOPY);
      }
      else if ((DShow::VideoFormat)type == DShow::VideoFormat::YVYU || (DShow::VideoFormat)type == DShow::VideoFormat::YUY2) {
         uint8_t* src_u = bytes + width * height;
         uint8_t* src_v = src_u + width * (height >> 2);
         bmi.bmiHeader.biHeight = -bmi.bmiHeader.biHeight;
         libyuv::YUY2ToARGB(bytes, width * 2, rgb, width * 4, width, height);
         StretchDIBits(dc_mem, x, y, width, height, 0, 0, width, height, rgb, &bmi, DIB_RGB_COLORS, SRCCOPY);
         BitBlt(mWindowDC, 0, 0, logical_area.x, logical_area.y, dc_mem, 0, 0, SRCCOPY);
      }
      else if ((DShow::VideoFormat)type == DShow::VideoFormat::UYVY || (DShow::VideoFormat)type == DShow::VideoFormat::HDYC) {
         uint8_t* src_u = bytes + width * height;
         uint8_t* src_v = src_u + width * (height >> 2);
         bmi.bmiHeader.biHeight = -bmi.bmiHeader.biHeight;
         libyuv::UYVYToARGB(bytes, width * 2, rgb, width * 4, width, height);
         StretchDIBits(dc_mem, x, y, width, height, 0, 0, width, height, rgb, &bmi, DIB_RGB_COLORS, SRCCOPY);
         BitBlt(mWindowDC, 0, 0, logical_area.x, logical_area.y, dc_mem, 0, 0, SRCCOPY);
      }

      ::SelectObject(dc_mem, bmp_old);
      ::DeleteObject(bmp_mem);
      ::DeleteDC(dc_mem);
      ::ReleaseDC(wnd_, mWindowDC);
   }
}