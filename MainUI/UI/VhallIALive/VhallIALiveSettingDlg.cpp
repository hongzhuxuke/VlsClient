#include "VhallIALiveSettingDlg.h"
#include "DebugTrace.h"
#include "vhallwaiting.h"
#include "NoiseTooltip.h"
#include "Pathmanager.h"
#include "ConfigSetting.h"
#include "pathManage.h"
#include <QFileDialog>
#include <QDesktopWidget> 
#include "AlertTipsDlg.h"
#include "pub.Const.h"
#include "InteractAPIManager.h"
#include "VhallUI_define.h"
#include "MainUIIns.h"
#include "Msg_MainUI.h"
#include "arraysize.h"
#include <windows.h>
#pragma  comment(lib,"Msimg32.lib") //TransparentBlt函数包含的库文件

using namespace vlive;

#define SET_BUTTON_DOWN_STYLE   "\
                           QPushButton{ \
                           background-color:#5A393A;border:0px; \
                           font-size:14px;\
                           font-family:微软雅黑; \
                           color:#FFFFFF;} \
                           "

#define SET_BUTTON_UP_STYLE    "QPushButton{ \
                           background-color:#323232; border:0px;\
                           font-size:14px;\
                           font-family:微软雅黑; \
                           color:#D6D6D6;} \
                           QPushButton:hover{background-color:#5A393A;border:0px;color:#FFFFFF;font-size:14px;font-family:微软雅黑; }\
                           "

#define LAYOUT_FLOAT_ENABLE "border-image: url(:/vhallActive/img/vhallactive/float_select.png);"
#define LAYOUT_FLOAT_DISABLE "border-image: url(:/vhallActive/img/vhallactive/float.png);"
#define LAYOUT_TILED_ENABLE  "border-image: url(:/vhallActive/img/vhallactive/tiled_select.png);"
#define LAYOUT_TILED_DISABLE "border-image: url(:/vhallActive/img/vhallactive/tiled.png);"
#define LAYOUT_GRID_ENABLE "border-image: url(:/vhallActive/img/vhallactive/grid_select.png);"
#define LAYOUT_GRID_DISABLE "border-image: url(:/vhallActive/img/vhallactive/grid.png);"

bool VhallIALiveSettingDlg::mbIsHost = false;

ReleaseThread::ReleaseThread(QObject *parent /*= NULL*/)
   :QThread(parent) {

}
ReleaseThread::~ReleaseThread() {

}

void ReleaseThread::SetSubscribe(VhallIALiveSettingDlg* sub, CloseType type) {
   mParamPtr = sub;
   mCloseType = type;
}

void ReleaseThread::run() {
   if (mParamPtr) {
      mParamPtr->HandleReleaseStream(mCloseType);
   }
}

VhallIALiveSettingDlg::VhallIALiveSettingDlg(QWidget *parent, bool host, QString definition)
   : parentWdg(parent)

   , mbIsStartLiving(false)
   , mLocalVideoDefinition(RTC_VIDEO_PROFILE_480P_16x9_M)
{
   ui.setupUi(this);
   mbIsHost = host;
   this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint);
   //setAttribute(Qt::WA_TranslucentBackground);
   //setAutoFillBackground(true);
   ui.pushButton_close->loadPixmap(":/sysButton/close_button");
   ui.pushButton_close->setToolTip(QStringLiteral("关闭"));
   ui.pushButton_close->setStyleSheet(TOOLTIPQSS);
   if (mbIsHost) {
      ui.label_guestTips->hide();
   }
   else {
      ui.label_hostTips->hide();
      ui.widget_15->hide();
      ui.label_6->hide();
   }

   mRTCVideoReciver = std::make_shared<RTCVideoReciver>();
   mRTCVideoReciver->SetReiver(this);

   //问题反馈
   ui.comboBox_questionType->addItem(SETTING_FEEDBACK_TYPE);
   ui.comboBox_questionType->addItem(SETTING_FEEDBACK_CRASH);
   ui.comboBox_questionType->addItem(SETTING_FEEDBACK_FILE);
   ui.comboBox_questionType->addItem(SETTING_FEEDBACK_DEV);
   ui.comboBox_questionType->addItem(SETTING_FEEDBACK_AUDIO);
   ui.comboBox_questionType->addItem(SETTING_FEEDBACK_SOURCE);
   ui.comboBox_questionType->addItem(SETTING_FEEDBACK_OTHER);

   ui.textBrowser_Content->setPlaceholderText(SETTING_FEEDBACK_THANK);
   connect(ui.textBrowser_Content, SIGNAL(textChanged()), this, SLOT(OnMaxLength()));
   connect(ui.pushButton_AdvancedSetting, SIGNAL(clicked()), this, SLOT(slot_OnAdvancedSetting()));

   //美颜
   ui.horizontalSlider_Beauty->setMinimum(0);
   ui.horizontalSlider_Beauty->setMaximum(100);
   //ui.horizontalSlider_Beauty->setPageStep(1);
   //int beauty = ConfigSetting::ReadInt(CPathManager::GetConfigPath(), GROUP_DEFAULT, CAMERA_BEAUTY, 0);
   ui.horizontalSlider_Beauty->setValue(0);
   connect(ui.horizontalSlider_Beauty, SIGNAL(valueChanged(int)), this, SLOT(OnBeautyValueChange(int)));

   if (mbIsHost) {
      ui.comboBox_localDefinition->addItem(High_Pro_Tips, PROFILE_INDEX_HIGH);
      ui.comboBox_localDefinition->addItem(Standard_Pro_Tips, PROFILE_INDEX_STANDER);
      ui.comboBox_localDefinition->addItem(Fluency_Pro_Tips, PROFILE_INDEX_FLUENCY);
      QString profile = ConfigSetting::ReadString(CPathManager::GetConfigPath(), GROUP_DEFAULT, KEY_LOCAL_VIDEO_PROFILE, PROFILE_INDEX_STANDER);
      if (profile == PROFILE_INDEX_HIGH) {
         ui.comboBox_localDefinition->setCurrentIndex(0);
      }
      else if (profile == PROFILE_INDEX_STANDER) {
         ui.comboBox_localDefinition->setCurrentIndex(1);
      }
      else if (profile == PROFILE_INDEX_FLUENCY) {
         ui.comboBox_localDefinition->setCurrentIndex(2);
      }
   }
   else {
      ui.comboBox_localDefinition->hide();
   }

   QString desktop_profile = ConfigSetting::ReadString(CPathManager::GetConfigPath(), GROUP_DEFAULT, KEY_DESKTOP_PROFILE, DESKTOP_PROFILE_5FPS);
   if (desktop_profile == DESKTOP_PROFILE_5FPS) {
      ui.comboBox->setCurrentIndex(0);
   }
   else {
      ui.comboBox->setCurrentIndex(1);
   }

   
   int dpi_enable = ConfigSetting::ReadInt(CPathManager::GetConfigPath(), GROUP_DEFAULT, DESKTOP_DIP, 1);
   dpi_enable == 1 ? ui.checkBox_dpi->setCheckState(Qt::CheckState::Checked) : ui.checkBox_dpi->setCheckState(Qt::CheckState::Unchecked);
   
   int checked = ConfigSetting::ReadInt(CPathManager::GetConfigPath(), GROUP_DEFAULT, DESKTOP_ENHANCE, 0);
   if (checked > 0) {
      ui.checkBox_desktopEnHance->setCheckState(Qt::CheckState::Checked);
   }
   else {
      ui.checkBox_desktopEnHance->setCheckState(Qt::CheckState::Unchecked);
   }
   connect(ui.checkBox_desktopEnHance, SIGNAL(stateChanged(int)), this, SLOT(OnDesktopEnhanceChanged(int)));

   connect(ui.comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(slot_CurrentDesktopProfileChanged(int)));
   connect(ui.pushButton_audio, SIGNAL(clicked()), this, SLOT(slot_OnSelectAudioPage()));
   connect(ui.pushButton_video, SIGNAL(clicked()), this, SLOT(slot_OnSelectVideoPage()));
   connect(ui.pushButton_ai, SIGNAL(clicked()), this, SLOT(slot_OnSelectAIPage()));
   connect(ui.pushButton_system, SIGNAL(clicked()), this, SLOT(slot_OnSelectSystemPage()));
   connect(ui.pushButton_aboutUs, SIGNAL(clicked()), this, SLOT(slot_OnSelectAboutUsPage()));
   connect(ui.pushButton_changeLayoutMode, SIGNAL(clicked()), this, SLOT(slot_OnChangeLayoutPage()));
   connect(ui.pushButton_feedBack, SIGNAL(clicked()), this, SLOT(slot_OnChangeFeedBackPage()));
   connect(ui.pushButton_ok, SIGNAL(clicked()), this, SLOT(slot_OnClickedOK()));
   connect(ui.pushButton_close, &PushButton::clicked, this, &VhallIALiveSettingDlg::slot_OnClickedClose);
   connect(ui.comboBox_cameraSelect, SIGNAL(currentIndexChanged(int)), this, SLOT(slot_CurrentCameraSelectChanged(int)));
   connect(ui.comboBox_beautyCamera, SIGNAL(currentIndexChanged(int)), this, SLOT(slot_CurrentBeautyCameraSelectChanged(int)));
   
   connect(ui.comboBox_micDev, SIGNAL(currentIndexChanged(int)), this, SLOT(slot_CurrentMicSelectChanged(int)));
   connect(ui.comboBox_playerDev, SIGNAL(currentIndexChanged(int)), this, SLOT(slot_CurrentPlayerSelectChanged(int)));
   connect(ui.pushButton_gridMode, SIGNAL(clicked()), this, SLOT(slot_SelectGridMode()));
   connect(ui.pushButton_tiledMode, SIGNAL(clicked()), this, SLOT(slot_SelectTiledMode()));
   connect(ui.pushButton_floatMode, SIGNAL(clicked()), this, SLOT(slot_SelectFloatMode()));
   connect(ui.pushButton_selectPic, SIGNAL(clicked()), this, SLOT(slot_SelectPushPic()));
   connect(ui.radioButton_pic, SIGNAL(clicked(bool)), this, SLOT(slot_OpenPushPic(bool)));
   connect(ui.radioButton_camera, SIGNAL(clicked(bool)), this, SLOT(slot_RadioCamera(bool)));
   connect(ui.btnPlayer, &PushButton::clicked, this, &VhallIALiveSettingDlg::sig_PlayerBtnClicked);
   connect(ui.playerVolumeSlider, &QSlider::valueChanged, this, &VhallIALiveSettingDlg::sigVolumnChanged);
   connect(ui.comboBox_localDefinition, SIGNAL(currentIndexChanged(int)), this, SLOT(slot_localDefinitionChanged(int)));
   connect(ui.chkbDesktopAudio, SIGNAL(clicked(bool)), this, SLOT(slot_OpenDesktopAudio(bool)));
   connect(ui.pushButton_deskTopCaptureVolmue, SIGNAL(clicked()), this, SLOT(slot_OnMicMuteClick()));
   ui.speakerVolumeSlider->setRange(0, 100);
   ui.speakerVolumeSlider->setValue(100);
   mCapturePlayValue = 100;
   connect(ui.comboBox_playCapture, SIGNAL(currentIndexChanged(int)), this, SLOT(slot_SelectplayCapture(int)));
   connect(ui.speakerVolumeSlider, SIGNAL(valueChanged(int)), this, SLOT(slot_OnSpeakerVolumeChange(int)));

   ui.stackedWidget_privew->setCurrentIndex(1);
   ui.labDesktopAudio->hide();
   slot_OnSelectVideoPage();
   mpExitWaiting = new QDialog();
   if (mpExitWaiting) {
      mpExitWaiting->setWindowFlags(Qt::FramelessWindowHint | Qt::Tool);
      mpExitWaiting->setFixedSize(150, 150);
      mpExitWaiting->setStyleSheet("border-image:url(:/vhallActive/img/vhallactive/closing.png);background-color: rgba(0, 0, 0, 0);");
      mpExitWaiting->hide();
   }
   mpSaveWaiting = new QDialog();
   if (mpSaveWaiting) {
      mpSaveWaiting->setWindowFlags(Qt::FramelessWindowHint | Qt::Tool);
      mpSaveWaiting->setFixedSize(150, 150);
      mpSaveWaiting->setStyleSheet("border-image: url(:/vhallActive/img/vhallactive/saving.png);background-color: rgba(0, 0, 0, 0);");
      mpSaveWaiting->hide();
   }
   if (mbIsHost) {
      ui.widget_layout->show();
      ui.pushButton_changeLayoutMode->show();
   }
   else {
      ui.widget_layout->hide();
      ui.pushButton_changeLayoutMode->hide();
   }
   m_pNoiseTips = new NoiseTooltip(this);
   if (m_pNoiseTips) {
      m_pNoiseTips->hide();
      m_pNoiseTips->setText(SETTING_LAYOUT_MODE);
      m_pNoiseTips->setFixedHeight(125);
   }
   ui.label_tips->installEventFilter(this);
   ui.label_DesktopProfileTips->installEventFilter(this);
   ui.label_desktop_hance->installEventFilter(this);
   ui.label_layoutChangeTips->hide();

   wstring confPath = GetAppDataPath() + VHALL_TOOL_CONFIG;
   QString toolConfPath = CPathManager::GetAppDataPath() + QString::fromStdWString(VHALL_TOOL_CONFIG);
   int hideLogo = ConfigSetting::ReadInt(toolConfPath, GROUP_DEFAULT, KEY_VHALL_LOGO_HIDE, 0);
   if (hideLogo != 0) {
      ui.pushButton_aboutUs->hide();
   }

   QString qsConfPath = qsConfPath.fromStdWString(confPath);
   QString vhallHelper = ConfigSetting::ReadString(qsConfPath, GROUP_DEFAULT, KEY_VHALL_HELPER, VHALL_LIVE_HELPER);


   SetCopyrightInfo(vhallHelper, SETTING_VHALL);
   ui.btnPlayer->loadPixmap(":/sysButton/speakerVolumeAble");
   mNormalRect = this->geometry();

   ZeroMemory(&bmi_, sizeof(bmi_));
   bmi_.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
   bmi_.bmiHeader.biPlanes = 1;
   bmi_.bmiHeader.biBitCount = 32;
   bmi_.bmiHeader.biCompression = BI_RGB;
   bmi_.bmiHeader.biWidth = 1;
   bmi_.bmiHeader.biHeight = -1;
   bmi_.bmiHeader.biSizeImage = 1 * 1 * (bmi_.bmiHeader.biBitCount >> 3);

   mPlayView = (HWND)ui.widget_preView->winId();
   mBeautyView = (HWND)ui.page_render->winId();

   bool is_support = GetWebRtcSDKInstance()->IsSupprotBeauty();
   if (!is_support) {
      ui.horizontalSlider_Beauty->setEnabled(false);
      ui.widget_26->show();
   }
   else {
      ui.widget_26->hide();
   }
}

VhallIALiveSettingDlg::~VhallIALiveSettingDlg()
{
   ui.label_tips->removeEventFilter(this);
   if (mpExitWaiting) {
      delete mpExitWaiting;
      mpExitWaiting = NULL;
   }

   if (NULL != m_pNoiseTips) {
      delete  m_pNoiseTips;
      m_pNoiseTips = NULL;
   }

   if (mpSaveWaiting) {
      delete mpSaveWaiting;
      mpSaveWaiting = NULL;
   }
}

void VhallIALiveSettingDlg::SerVersion(const QString& version) {
   ui.label_version->setText("V " + version);
}

void VhallIALiveSettingDlg::SetCopyrightInfo(QString qsAppName, QString qsCopyright) {
   ui.label_appName->setText(qsAppName);
   ui.label_copyright->setText(qsCopyright);
}

void VhallIALiveSettingDlg::SetStartLiving(bool start) {
   TRACE6("%s mbIsStartLiving :%d\n", __FUNCTION__, start);
   mbIsStartLiving = start;
   if (start) {
      ui.comboBox->setEnabled(false);
   }
   else {
      ui.comboBox->setEnabled(true);
   }
}

void VhallIALiveSettingDlg::ResetProfile(int index) {
   ui.comboBox_localDefinition->setCurrentIndex(index);
}

void VhallIALiveSettingDlg::SetSize(int width, int height) {
   if (width == bmi_.bmiHeader.biWidth && -height == bmi_.bmiHeader.biHeight) {
      return;
   }

   bmi_.bmiHeader.biWidth = width;
   bmi_.bmiHeader.biHeight = -height;
   bmi_.bmiHeader.biSizeImage = width * height *
      (bmi_.bmiHeader.biBitCount >> 3);
}


void VhallIALiveSettingDlg::RenderWnd(HWND wnd, const unsigned char* video, int length, int input_width, int input_height) {

   //mbIsRecvData = true;
   //mRecvDataTime = QDateTime::currentDateTime().toMSecsSinceEpoch();

   HWND wnd_ = wnd;
   RECT rc;
   ::GetClientRect(wnd_, &rc);

   const BITMAPINFO& bmi = bmi_;
   int height = abs(bmi.bmiHeader.biHeight);
   int width = bmi.bmiHeader.biWidth;
   double screen_dipSize = ConfigSetting::GetCurrentScreenDPISize(CPathManager::GetConfigPath(), this);
   const uint8_t* image = video;
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

      int x = (logical_area.x / 2) - (width / 2);
      int y = (logical_area.y / 2) - (height / 2);

      StretchDIBits(dc_mem, x, y, width, height, 0, 0, width, height, image, &bmi, DIB_RGB_COLORS, SRCCOPY);
      BitBlt(mWindowDC, 0, 0, logical_area.x, logical_area.y, dc_mem, 0, 0, SRCCOPY);

      ::SelectObject(dc_mem, bmp_old);
      ::DeleteObject(bmp_mem);
      ::DeleteDC(dc_mem);
      ::ReleaseDC(wnd_, mWindowDC);
   }
}


bool VhallIALiveSettingDlg::IsOpenCameraSuccess() {
   return !mbGetCameraSuccess;
}

void VhallIALiveSettingDlg::VideoFrame(const unsigned char* video, int length, int width, int height) { 
   if (!mbRecvData) {
      mbRecvData = true;
      QApplication::postEvent(this, new QEvent(CustomEvent_RTC_LIVE_RECVCAPTURE_FARME));
   }
   SetSize(width, height);
   RenderWnd(mPlayView, video, length, width, height);
   RenderWnd(mBeautyView, video, length, width, height);
}


void VhallIALiveSettingDlg::VideoFrameFromUsing(const unsigned char* video, int length, int width, int height) {
   //美颜预览时如果已经直播，并且开启了摄像头。则回显当前正在使用的摄像头画面。
   //通过正在使用的设备id和ui.comboBox_cameraSelect已经选中的进行对比判断。
   if (mbIsStartLiving && mbGetCameraEnd && !mbGetCameraSuccess) {
      QString qsConfPath = CPathManager::GetConfigPath();
      QString configCameraDevID = ConfigSetting::ReadString(qsConfPath, GROUP_DEFAULT, CAPTURE_CAMERA_ID, "");
      int openPushPic = ConfigSetting::ReadInt(qsConfPath, GROUP_DEFAULT, PUSH_PIC_OPEN, 0);
      if (!openPushPic && configCameraDevID == mCurCameraDevID) {
         if (!mbRecvData) {
            mbRecvData = true;
            QApplication::postEvent(this, new QEvent(CustomEvent_RTC_LIVE_RECVCAPTURE_FARME));
         }
         SetSize(width, height);
         RenderWnd(mPlayView, video, length, width, height);
         RenderWnd(mBeautyView, video, length, width, height);
      }
   }
}

void VhallIALiveSettingDlg::UpdateStartCaptureInfo(QString guid, int videoProfile) {
   mStartCaptureDevId = guid;
   mStartCaptureVideoProfile = videoProfile;
}

void VhallIALiveSettingDlg::PreviewCameraCallback(bool succuss) {
   if (succuss) {
      HandleGetCameraSuc();
   }
   else {
      HandleGetCameraErr();
   }
}

void VhallIALiveSettingDlg::RegisterListener(DeviceChangedListener* listener) {
   mpDevChangedListener = listener;
}

void VhallIALiveSettingDlg::RegisterPlayMediaFileObj(IPlayMediaFileObj* playMediaFile) {
   mpMediaFile = playMediaFile;
}

void VhallIALiveSettingDlg::slot_OnChangeLayoutPage() {
   TRACE6("%s \n", __FUNCTION__);
   ui.stackedWidget->setCurrentIndex(layout_page);
   ui.pushButton_audio->setStyleSheet(SET_BUTTON_UP_STYLE);
   ui.pushButton_video->setStyleSheet(SET_BUTTON_UP_STYLE);
   ui.pushButton_aboutUs->setStyleSheet(SET_BUTTON_UP_STYLE);
   ui.pushButton_changeLayoutMode->setStyleSheet(SET_BUTTON_DOWN_STYLE);
   ui.pushButton_feedBack->setStyleSheet(SET_BUTTON_UP_STYLE);
   ui.pushButton_system->setStyleSheet(SET_BUTTON_UP_STYLE);
   ui.pushButton_ai->setStyleSheet(SET_BUTTON_UP_STYLE);
}

void VhallIALiveSettingDlg::slot_OnChangeFeedBackPage() {
   TRACE6("%s \n", __FUNCTION__);
   ui.stackedWidget->setCurrentIndex(about_feedback);
   ui.pushButton_audio->setStyleSheet(SET_BUTTON_UP_STYLE);
   ui.pushButton_video->setStyleSheet(SET_BUTTON_UP_STYLE);
   ui.pushButton_aboutUs->setStyleSheet(SET_BUTTON_UP_STYLE);
   ui.pushButton_feedBack->setStyleSheet(SET_BUTTON_DOWN_STYLE);
   ui.pushButton_changeLayoutMode->setStyleSheet(SET_BUTTON_UP_STYLE);
   ui.pushButton_system->setStyleSheet(SET_BUTTON_UP_STYLE);
   ui.pushButton_ai->setStyleSheet(SET_BUTTON_UP_STYLE);
}

void VhallIALiveSettingDlg::slot_OnSelectAudioPage() {
   TRACE6("%s \n", __FUNCTION__);
   ui.stackedWidget->setCurrentIndex(audio_page);
   ui.pushButton_audio->setStyleSheet(SET_BUTTON_DOWN_STYLE);
   ui.pushButton_video->setStyleSheet(SET_BUTTON_UP_STYLE);
   ui.pushButton_aboutUs->setStyleSheet(SET_BUTTON_UP_STYLE);
   ui.pushButton_changeLayoutMode->setStyleSheet(SET_BUTTON_UP_STYLE);
   ui.pushButton_feedBack->setStyleSheet(SET_BUTTON_UP_STYLE);
   ui.pushButton_system->setStyleSheet(SET_BUTTON_UP_STYLE);
   ui.pushButton_ai->setStyleSheet(SET_BUTTON_UP_STYLE);
}

void VhallIALiveSettingDlg::slot_OnSelectSystemPage() {
   TRACE6("%s \n", __FUNCTION__);
   ui.stackedWidget->setCurrentIndex(system_page);
   ui.pushButton_audio->setStyleSheet(SET_BUTTON_UP_STYLE);
   ui.pushButton_video->setStyleSheet(SET_BUTTON_UP_STYLE);
   ui.pushButton_aboutUs->setStyleSheet(SET_BUTTON_UP_STYLE);
   ui.pushButton_changeLayoutMode->setStyleSheet(SET_BUTTON_UP_STYLE);
   ui.pushButton_feedBack->setStyleSheet(SET_BUTTON_UP_STYLE);
   ui.pushButton_system->setStyleSheet(SET_BUTTON_DOWN_STYLE);
   ui.pushButton_ai->setStyleSheet(SET_BUTTON_UP_STYLE);
}

void VhallIALiveSettingDlg::slot_OnSelectAIPage() {
   TRACE6("%s \n", __FUNCTION__);
   ui.stackedWidget->setCurrentIndex(ai_page);
   ui.pushButton_audio->setStyleSheet(SET_BUTTON_UP_STYLE);
   ui.pushButton_video->setStyleSheet(SET_BUTTON_UP_STYLE);
   ui.pushButton_aboutUs->setStyleSheet(SET_BUTTON_UP_STYLE);
   ui.pushButton_changeLayoutMode->setStyleSheet(SET_BUTTON_UP_STYLE);
   ui.pushButton_feedBack->setStyleSheet(SET_BUTTON_UP_STYLE);
   ui.pushButton_system->setStyleSheet(SET_BUTTON_UP_STYLE);
   ui.pushButton_ai->setStyleSheet(SET_BUTTON_DOWN_STYLE);
}

void VhallIALiveSettingDlg::slot_OnSelectVideoPage() {
   TRACE6("%s \n", __FUNCTION__);
   ui.stackedWidget->setCurrentIndex(video_page);
   ui.pushButton_audio->setStyleSheet(SET_BUTTON_UP_STYLE);
   ui.pushButton_video->setStyleSheet(SET_BUTTON_DOWN_STYLE);
   ui.pushButton_aboutUs->setStyleSheet(SET_BUTTON_UP_STYLE);
   ui.pushButton_changeLayoutMode->setStyleSheet(SET_BUTTON_UP_STYLE);
   ui.pushButton_feedBack->setStyleSheet(SET_BUTTON_UP_STYLE);
   ui.pushButton_system->setStyleSheet(SET_BUTTON_UP_STYLE);
   ui.pushButton_ai->setStyleSheet(SET_BUTTON_UP_STYLE);
}

void VhallIALiveSettingDlg::slot_OnSelectAboutUsPage() {
   TRACE6("%s \n", __FUNCTION__);
   ui.stackedWidget->setCurrentIndex(aboutUs_page);
   ui.pushButton_audio->setStyleSheet(SET_BUTTON_UP_STYLE);
   ui.pushButton_video->setStyleSheet(SET_BUTTON_UP_STYLE);
   ui.pushButton_changeLayoutMode->setStyleSheet(SET_BUTTON_UP_STYLE);
   ui.pushButton_aboutUs->setStyleSheet(SET_BUTTON_DOWN_STYLE);
   ui.pushButton_feedBack->setStyleSheet(SET_BUTTON_UP_STYLE);
   ui.pushButton_system->setStyleSheet(SET_BUTTON_UP_STYLE);
   ui.pushButton_ai->setStyleSheet(SET_BUTTON_UP_STYLE);
}

void VhallIALiveSettingDlg::ShowDesktopCaptrue(bool show) {
   if (show) {
      ui.widget_desktopCapture->show();
   }
   else {
      ui.widget_desktopCapture->hide();
      ui.chkbDesktopAudio->setChecked(false);
   }
}

void VhallIALiveSettingDlg::setPlayerVolume(const int& iValue)
{
   ui.playerVolumeSlider->setValue(iValue);
}

void VhallIALiveSettingDlg::OnOpenPlayer(bool bOpen)
{
   if (bOpen) {
      ui.btnPlayer->loadPixmap(":/sysButton/speakerVolumeAble");
   }
   else {
      ui.btnPlayer->loadPixmap(":/sysButton/speakerVolumeUnable");
   }
}

void VhallIALiveSettingDlg::slot_OnClickedApply() {
   TRACE6("%s \n", __FUNCTION__);
   QString qsConfPath = CPathManager::GetConfigPath();
   //检测播放扬声器的设备是否发生改变,改变则重新选择。
   QString configCurPlayerID = ConfigSetting::ReadString(qsConfPath, GROUP_DEFAULT, PLAY_OUT_DEV_ID, "");
   if (ui.comboBox_playerDev->count() > 0) {
      QStringList devList = ui.comboBox_playerDev->currentData().toStringList();
      QString name = ui.comboBox_playerDev->currentText();
      if (devList.size() == 2) {
         mCurPlayerID = devList.at(0);
         mLastPlayerIndex = devList.at(1).toInt();
      }
      setPlayerVolume(100);
      TRACE6("%s selectPlayer name:%s index:%d\n", __FUNCTION__, name.toStdString().c_str(), mLastPlayerIndex);
   }
   else {
      mCurPlayerID = "";
      mLastPlayerIndex = -1;
      setPlayerVolume(0);
   }
   TRACE6("%s configCurPlayerID:%s mCurPlayerID:%s\n", __FUNCTION__, configCurPlayerID.toStdString().c_str(), mCurPlayerID.toStdString().c_str());
   if (configCurPlayerID != mCurPlayerID && ui.comboBox_playerDev->count() > 0) {
      GetWebRtcSDKInstance()->SetUsedPlay(mLastPlayerIndex, mCurPlayerID.toStdString());
   }
   ConfigSetting::writeValue(qsConfPath, GROUP_DEFAULT, PLAY_OUT_DEV_ID, mCurPlayerID);

   /////////////////////////////////////////////////////////////////////////////////////
   //检测麦克风和桌面采集设备是否发生变化。
   QString configMicDevID = ConfigSetting::ReadString(qsConfPath, GROUP_DEFAULT, CAPTURE_MIC_ID, "");
   QString configCapturePlayUid = ConfigSetting::ReadString(qsConfPath, GROUP_DEFAULT, PUSH_DESKTOP_CAPTURE_DEVID, "");
   int enableDeskTopCaptrue = ConfigSetting::ReadInt(qsConfPath, GROUP_DEFAULT, PUSH_DESKTOP_CAPTURE_OPEN, 0);
   TRACE6("%s enableDeskTopCaptrue:%d\n", __FUNCTION__, enableDeskTopCaptrue);
   if (ui.comboBox_micDev->count() > 0) {
      QStringList devList = ui.comboBox_micDev->currentData().toStringList();
      QString name = ui.comboBox_micDev->currentText();
      if (devList.size() == 2) {
         mCurMicDevID = devList.at(0);
         mLastMicIndex = devList.at(1).toInt();
      }
      TRACE6("%s selectMic name:%s index:%d\n", __FUNCTION__, name.toStdString().c_str(), mLastMicIndex);
   }
   else {
      mLastMicIndex = -1;
      mCurMicDevID = "";
   }

   int level = GetLevelFromSlider(ui.horizontalSlider_Beauty->value());
   GetWebRtcSDKInstance()->SetCameraBeautyLevel(level);
   if (ui.comboBox_playCapture->count() > 0) {
      QStringList devList = ui.comboBox_playCapture->currentData().toStringList();
      QString name = ui.comboBox_playCapture->currentText();
      if (devList.size() == 2) {
         mCapturePlayUid = devList.at(0);
         mCapturePlayIndex = devList.at(1).toInt();
      }
   }
   else {
      mCapturePlayUid = "";
      mCapturePlayIndex = -1;
   }
   //如果存在扬声器设备采集。
   if (ui.chkbDesktopAudio->isChecked()) {
      int volume = ConfigSetting::ReadInt(qsConfPath, GROUP_DEFAULT, PUSH_DESKTOP_CAPTURE_VOLUME, 0);
      int curDesktopCaptureVol = ui.speakerVolumeSlider->value();
      TRACE6("%s mCapturePlayUid :%s configCapturePlayUid:%s mLastMicIndex:%d\n", __FUNCTION__, mCapturePlayUid.toStdString().c_str(), configCapturePlayUid.toStdString().c_str(), mLastMicIndex);
      TRACE6("%s mCurMicDevID:%s configMicDevID:%s volume:%d curDesktopCaptureVol:%d\n", __FUNCTION__, mCurMicDevID.toStdString().c_str(), configMicDevID.toStdString().c_str(), volume, curDesktopCaptureVol);
      if (enableDeskTopCaptrue == 0 && (mCapturePlayUid != configCapturePlayUid || mCurMicDevID != configMicDevID)) {
         ui.speakerVolumeSlider->setValue(100);
         int nRet = GetWebRtcSDKInstance()->StartLocalCapturePlayer(mCapturePlayUid.toStdWString(), 100);
         if (nRet == 0) {
            vlive::GetWebRtcSDKInstance()->MuteAllSubScribeAudio(true);
         }
         QJsonObject body;
         SingletonMainUIIns::Instance().reportLog(STR_VH_LIVE_OPERATE, eLogRePortK_LIVE_PlayCapture, L"PlayCapture", body);
      }
      else if (!mCurMicDevID.isEmpty()) {
         int nRet = GetWebRtcSDKInstance()->StartLocalCapturePlayer(mCapturePlayUid.toStdWString(), curDesktopCaptureVol);
         if (nRet == 0) {
            vlive::GetWebRtcSDKInstance()->MuteAllSubScribeAudio(true);
         }
         QJsonObject body;
         SingletonMainUIIns::Instance().reportLog(STR_VH_LIVE_OPERATE, eLogRePortK_LIVE_PlayCapture, L"PlayCapture", body);
      }
      ConfigSetting::writeValue(qsConfPath, GROUP_DEFAULT, PUSH_DESKTOP_CAPTURE_VOLUME, curDesktopCaptureVol);
      ConfigSetting::writeValue(qsConfPath, GROUP_DEFAULT, PUSH_DESKTOP_CAPTURE_OPEN, 1);
   }
   else {
      int openDeskTopCapture = ConfigSetting::ReadInt(qsConfPath, GROUP_DEFAULT, PUSH_DESKTOP_CAPTURE_OPEN, 0);
      TRACE6("%s StartLocalCapturePlayer:%d openDeskTopCapture:%d\n", __FUNCTION__, mLastMicIndex, openDeskTopCapture);
      if (openDeskTopCapture == 1) {
         //如果上次已经打开了，那么才关闭桌面采集
         GetWebRtcSDKInstance()->StopLocalCapturePlayer();
      }
      else {
         GetWebRtcSDKInstance()->SetUsedMic(mLastMicIndex, mCurMicDevID.toStdString(), L"");
      }
      vlive::GetWebRtcSDKInstance()->MuteAllSubScribeAudio(false);
      TRACE6("%s MuteAllSubScribeAudio\n", __FUNCTION__);
      ConfigSetting::writeValue(qsConfPath, GROUP_DEFAULT, PUSH_DESKTOP_CAPTURE_OPEN, 0);
   }
   ConfigSetting::writeValue(qsConfPath, GROUP_DEFAULT, PUSH_DESKTOP_CAPTURE_DEVID, mCapturePlayUid);
   ConfigSetting::writeValue(qsConfPath, GROUP_DEFAULT, PUSH_DESKTOP_CAPTURE_INDEX, mCapturePlayIndex);
   ConfigSetting::writeValue(qsConfPath, GROUP_DEFAULT, CAPTURE_MIC_ID, mCurMicDevID);

   /////////////////////////////////////////////////////////////////////////////////////
   ////摄像头选择改变和推流图片改变
   if (ui.comboBox_cameraSelect->count() > 0) {
      QStringList devList = ui.comboBox_cameraSelect->currentData().toStringList();
      QString name = ui.comboBox_cameraSelect->currentText();
      if (devList.size() == 2) {
         mCurCameraDevID = devList.at(0);
         mLastCameraIndex = devList.at(1).toInt();
      }
      TRACE6("%s selectCamera name:%s index:%d\n", __FUNCTION__, name.toStdString().c_str(), mLastCameraIndex);
   }
   else {
      mLastCameraIndex = -1;
      mCurCameraDevID = "";
   }
   QString configCameraDevID = ConfigSetting::ReadString(qsConfPath, GROUP_DEFAULT, CAPTURE_CAMERA_ID, "");
   int openPushPic = ConfigSetting::ReadInt(qsConfPath, GROUP_DEFAULT, PUSH_PIC_OPEN, 0);
   //判断是否切换了清晰度。
   bool bChangeProfile = false;
   QString profile = ConfigSetting::ReadString(CPathManager::GetConfigPath(), GROUP_DEFAULT, KEY_LOCAL_VIDEO_PROFILE, PROFILE_INDEX_STANDER);
   if (profile != ui.comboBox_localDefinition->currentData().toString()) {
      bChangeProfile = true;
   }
   QString profileIndex = ui.comboBox_localDefinition->currentData().toString();
   ConfigSetting::writeValue(CPathManager::GetConfigPath(), GROUP_DEFAULT, KEY_LOCAL_VIDEO_PROFILE, profileIndex);
   mLocalVideoDefinition = GetPushStreamProfileByStreamType(VHStreamType_AVCapture);
   TRACE6("%s change profile profileIndex:%s bChangeProfile:%d\n", __FUNCTION__, profileIndex.toStdString().c_str(), bChangeProfile);
   int openDoubleStream = ConfigSetting::ReadInt(qsConfPath, GROUP_DEFAULT, PUSH_DOUBLE_STREAM, 1);
   if (mbIsHost) {
      openDoubleStream = 0;
   }
   else {
      openDoubleStream = 1;//
   }

   //如果进行图片推流。
   if (ui.radioButton_pic->isChecked()) {
      QString filePath = ui.lineEdit_PicPath->text();
      QString configPushPicPaht = ConfigSetting::ReadString(qsConfPath, GROUP_DEFAULT, PUSH_PIC_PATH, "");
      QString defaultPic = ConfigSetting::ReadString(CPathManager::GetToolConfigPath(), GROUP_DEFAULT, DEFAULT_PIC_NAME, VALUE_PIC_NAME);
      QFileInfo  fileInfo(filePath);
      if (filePath.isEmpty() || !fileInfo.exists()) {
         filePath = QApplication::applicationDirPath() + "\\" + defaultPic;
      }
      filePath = filePath.replace("/", "\\");
      TRACE6("%s openPushPic:%d mbIsStartLiving:%d filePath :%s\n", __FUNCTION__, openPushPic, mbIsStartLiving, filePath.toStdString().c_str());
      if ((openPushPic == 0 || filePath != configPushPicPaht || bChangeProfile) && mbIsStartLiving) {
         //新勾选图片推流，或者图片改变推流
         TRACE6("%s StartLocalCapturePicture\n", __FUNCTION__);
         GetWebRtcSDKInstance()->StartLocalCapturePicture(filePath.toStdString(), VhallIALiveSettingDlg::GetPushPicStreamProfile((VideoProfileIndex)mLocalVideoDefinition), /*openDoubleStream == 1 ? true : */false);
         TRACE6("%s StartLocalCapturePicture end\n", __FUNCTION__);
         QJsonObject body;
         SingletonMainUIIns::Instance().reportLog(STR_VH_LIVE_OPERATE, eLogRePortK_VHSDK_RTC_PIC_PUSH, L"rtc_pic", body);
      }
      ConfigSetting::writeValue(qsConfPath, GROUP_DEFAULT, PUSH_PIC_PATH, filePath);
      ConfigSetting::writeValue(qsConfPath, GROUP_DEFAULT, PUSH_PIC_OPEN, 1);
   }
   else {
      TRACE6("%s StartLocalCapture openPushPic:%d mbIsStartLivin:%d configCameraDevID:%s  mCurCameraDevID:%s mbGetCameraEnd:%d\n", __FUNCTION__, openPushPic, mbIsStartLiving, configCameraDevID.toStdString().c_str(), mCurCameraDevID.toStdString().c_str(), mbGetCameraEnd);
      //从图片推流切换到摄像头 或者设备切换了。
      if ((openPushPic == 1 || configCameraDevID != mCurCameraDevID || (configCameraDevID == mCurCameraDevID && mbGetCameraEnd && mbGetCameraSuccess) || bChangeProfile) && mbIsStartLiving) {
         int nRet = GetWebRtcSDKInstance()->StartLocalCapture(mCurCameraDevID.toStdString(), (VideoProfileIndex)mLocalVideoDefinition, openDoubleStream == 1 && !mCurCameraDevID.isEmpty() ? true : false);
         if (nRet == VhallLive_OK) {
            TRACE6("%s StartLocalCapture ok\n", __FUNCTION__);
         }
         TRACE6("%s StartLocalCapture\n", __FUNCTION__);
      }
      ConfigSetting::writeValue(qsConfPath, GROUP_DEFAULT, PUSH_PIC_OPEN, 0);
   }
   if (bChangeProfile) {
      emit sigProfileChanged();
   }
   ConfigSetting::writeValue(qsConfPath, GROUP_DEFAULT, CAPTURE_CAMERA_ID, mCurCameraDevID);
   //保存采集播放设备
   if (mpDevChangedListener) {
      mpDevChangedListener->OnDevChanged(mCurCameraDevID, mLastCameraIndex, mCurMicDevID, mLastMicIndex, mCurPlayerID, mLastPlayerIndex);
   }

   if (meLastLayoutMode != meLayoutMode) {
      if (mpDevChangedListener && mpMediaFile && !mpMediaFile->IsPlayMediaFile() && !mpMediaFile->IsPlayDesktopShare()) {
         mpDevChangedListener->OnChangeLayOutMode(meLayoutMode);
      }
   }

   int nType = ui.comboBox_questionType->currentIndex();
   QString sContent = ui.textBrowser_Content->toPlainText();
   if (!sContent.isEmpty()) {
      if (nType == 0) {
         nType = 6;
      }
      InteractAPIManager api(nullptr);
      nType = nType + 9;
      api.HttpSendFeedBack(nType, sContent);
   }

   int enhance_checked = ui.checkBox_desktopEnHance->checkState();
   int cur_enhance = ConfigSetting::ReadInt(CPathManager::GetConfigPath(), GROUP_DEFAULT, DESKTOP_ENHANCE, 0);
   if (enhance_checked != cur_enhance) {
      GetWebRtcSDKInstance()->SetDesktopEdgeEnhance(enhance_checked <= 0 ?  false : true);
      if (enhance_checked > 0) {
         QJsonObject body;
         SingletonMainUIIns::Instance().reportLog(STR_VH_LIVE_OPERATE, eLogRePortK_Desktop_Hance, L"desktop_enhance", body);
      }
      TRACE6("%s SetDesktopEdgeEnhance\n", __FUNCTION__);
   }
   ConfigSetting::writeValue(CPathManager::GetConfigPath(), GROUP_DEFAULT, DESKTOP_ENHANCE, enhance_checked);     
   
   if (ui.horizontalSlider_Beauty->value() > 0) {
      QJsonObject body;
      SingletonMainUIIns::Instance().reportLog(STR_VH_LIVE_OPERATE, eLogRePortK_Beauty_Open, L"beauty_open", body);
      TRACE6("%s beauty level %d\n", __FUNCTION__, ui.horizontalSlider_Beauty->value());
   }

   TRACE6("%s end\n", __FUNCTION__);
}

void VhallIALiveSettingDlg::SetCamerCaptureFailed() {
   mCurCameraDevID.clear();
}

void VhallIALiveSettingDlg::slot_OnClickedOK() {
   GetWebRtcSDKInstance()->StopPreviewCamera();
   TRACE6("%s start mbGetCameraEnd:%d \n", __FUNCTION__, mbGetCameraEnd);
   if (mbGetCameraEnd) {
      mReleaseThread.SetSubscribe(this, CloseWithSave);
      mReleaseThread.start();
      if (mpSaveWaiting) {
         QPoint pos = this->pos();
         mpSaveWaiting->move(pos + QPoint((this->width() - 150) / 2, (this->height() - 150) / 2));
         mpSaveWaiting->exec();
      }
      close();
   }
   else if (meLastLayoutMode != meLayoutMode) {
      mReleaseThread.SetSubscribe(this, CloseWithSaveLayoutMode);
      mReleaseThread.start();
      if (mpSaveWaiting) {
         QPoint pos = this->pos();
         mpSaveWaiting->move(pos + QPoint((this->width() - 150) / 2, (this->height() - 150) / 2));
         mpSaveWaiting->exec();
      }
      close();
   }
   ui.textBrowser_Content->clear();
   ui.label_layoutChangeTips->hide();
   int dpi_enable = ui.checkBox_dpi->checkState() == Qt::CheckState::Checked ? 1 : 0;
   ConfigSetting::writeValue(CPathManager::GetConfigPath(), GROUP_DEFAULT, DESKTOP_DIP, dpi_enable);
   TRACE6("%s end\n", __FUNCTION__);
}

void VhallIALiveSettingDlg::slot_OnClickedClose() {
   TRACE6("%s start \n", __FUNCTION__);
   GetWebRtcSDKInstance()->StopPreviewCamera();
   ui.comboBox_cameraSelect->setEnabled(true);
   ui.textBrowser_Content->clear();
   close();
   TRACE6("%s end\n", __FUNCTION__);
}

void VhallIALiveSettingDlg::HandleReleaseStream(int type) {
   TRACE6("%s \n", __FUNCTION__);
   vlive::GetWebRtcSDKInstance()->StopPreviewCamera();
   QApplication::postEvent(this, new CustomRecvMsgEvent(CustomEvent_CloseSettingDlg, QString::number(type)));
   TRACE6("%s end  type:%d\n", __FUNCTION__, type);
}

LAYOUT_MODE VhallIALiveSettingDlg::GetLayOutMode() {
   return meLayoutMode;
}

void VhallIALiveSettingDlg::SetLayOutMode(LAYOUT_MODE mode) {
   meLayoutMode = mode;
   if (meLayoutMode == LAYOUT_MODE_FLOAT) {
      ui.pushButton_gridMode->setStyleSheet(LAYOUT_GRID_DISABLE);
      ui.pushButton_floatMode->setStyleSheet(LAYOUT_FLOAT_ENABLE);
      ui.pushButton_tiledMode->setStyleSheet(LAYOUT_TILED_DISABLE);
   }
   else if (meLayoutMode == LAYOUT_MODE_GRID) {
      ui.pushButton_gridMode->setStyleSheet(LAYOUT_GRID_ENABLE);
      ui.pushButton_floatMode->setStyleSheet(LAYOUT_FLOAT_DISABLE);
      ui.pushButton_tiledMode->setStyleSheet(LAYOUT_TILED_DISABLE);
   }
   else if (meLayoutMode == LAYOUT_MODE_TILED) {
      ui.pushButton_gridMode->setStyleSheet(LAYOUT_GRID_DISABLE);
      ui.pushButton_floatMode->setStyleSheet(LAYOUT_FLOAT_DISABLE);
      ui.pushButton_tiledMode->setStyleSheet(LAYOUT_TILED_ENABLE);
   }
}

void VhallIALiveSettingDlg::SetLastLayoutMode() {
   meLastLayoutMode = meLayoutMode;
}

void VhallIALiveSettingDlg::showEvent(QShowEvent *event) {
   this->setAttribute(Qt::WA_Mapped);
   QWidget::showEvent(event);
}

void VhallIALiveSettingDlg::customEvent(QEvent* event) {
   if (event) {
      TRACE6("%s enter %d\n", __FUNCTION__, event->type());
      switch (event->type())
      {
      case CustomEvent_SettingGetCameraDevSuc: {
         HandleGetCameraSuc();
         break;
      }
      case CustomEvent_RTC_LIVE_RECVCAPTURE_FARME:{
         HandleGetCaptureFrame();
         ui.page_render->setUpdatesEnabled(false);
         ui.page_render->update();
         break;
      }
      case CustomEvent_SettingGetCameraDevErr: {
         HandleGetCameraErr();
         break;
      }
      case CustomEvent_CloseSettingDlg: {
         GetWebRtcSDKInstance()->StopPreviewCamera();
         TRACE6("%s CustomEvent_CloseSettingDlg\n", __FUNCTION__);
         CustomRecvMsgEvent* msgEvent = (CustomRecvMsgEvent*)event;
         if (msgEvent) {
            int type = msgEvent->msg.toInt();
            if (type == CloseWithSave) {
               slot_OnClickedApply();
            }
            else if (type == CloseWithSaveLayoutMode) {
               TRACE6("%s CloseWithSaveLayoutMode\n", __FUNCTION__);
               if (mpDevChangedListener && mpMediaFile && !mpMediaFile->IsPlayMediaFile() && !mpMediaFile->IsPlayDesktopShare()) {
                  TRACE6("%s OnChangeLayOutMode\n", __FUNCTION__);
                  mpDevChangedListener->OnChangeLayOutMode(meLayoutMode);
               }
            }
         }
         if (mpSaveWaiting) {
            mpSaveWaiting->accept();
            mpSaveWaiting->close();
         }
         if (mpExitWaiting) {
            mpExitWaiting->accept();
            mpExitWaiting->close();
         }
         ui.textBrowser_Content->clear();
         TRACE6("%s CustomEvent_CloseSettingDlg end\n", __FUNCTION__);
         break;
      }
      default:
         break;
      }
      TRACE6("%s leave %d\n", __FUNCTION__, event->type());
   }
}

void VhallIALiveSettingDlg::HandleGetCamera(QString selectCamerId) {
   TRACE6("%s enter\n", __FUNCTION__);
   if (ui.comboBox_cameraSelect->count() == 0 || selectCamerId.isEmpty()) {
      TRACE6("%s comboBox_cameraSelect is 0\n", __FUNCTION__);
      return;
   }
   ui.comboBox_cameraSelect->setEnabled(false);
   mbGetCameraEnd = false;
   mbEnableUserSelectCamera = false;
   HWND playView = (HWND)ui.widget_preView->winId();
   ui.widget_preView->update();
   ui.widget_deviceNotice->hide();
   ui.widget_deviceNotice_2->hide();
   ui.stackedWidget_beauty->setCurrentIndex(1);
   int nRet = GetWebRtcSDKInstance()->StartPreviewCamera(mRTCVideoReciver, selectCamerId.toStdString(), (VideoProfileIndex)mLocalVideoDefinition);
   mbRecvData = false;
   mbGetCameraSuccess = false;
   mCurCameraDevID = selectCamerId;
   mbEnableUserSelectCamera = true;
   TRACE6("%s leave\n", __FUNCTION__);
}

void VhallIALiveSettingDlg::HandleGetCaptureFrame() {
   TRACE6("%s enter\n", __FUNCTION__);
   mbEnableUserSelectCamera = true;
   mbGetCameraEnd = true;
   ui.comboBox_cameraSelect->setEnabled(true);
   ui.widget_preView->setUpdatesEnabled(false);
   ui.widget_preView->update();
   ui.widget_deviceNotice->hide();
   ui.widget_deviceNotice_2->hide();
   ui.stackedWidget_beauty->setCurrentIndex(1);
   this->repaint();
   TRACE6("%s leave\n", __FUNCTION__);
}

void VhallIALiveSettingDlg::HandleGetCameraSuc() {
   HandleGetCaptureFrame();
   mbGetCameraSuccess = true;
   this->repaint();
   TRACE6("%s leave\n", __FUNCTION__);
}

void VhallIALiveSettingDlg::HandleGetCameraErr() {
   ui.widget_preView->setUpdatesEnabled(true);
   TRACE6("%s enter\n", __FUNCTION__);
   mbEnableUserSelectCamera = false;
   mbGetCameraEnd = true;
   ui.comboBox_cameraSelect->setEnabled(true);
   ui.page_render->setUpdatesEnabled(true);
   ui.page_render->update();
   mbGetCameraSuccess = false;
   ui.widget_preView->update();
   if (!mbRecvData) {
      ui.widget_deviceNotice->show();
      ui.widget_deviceNotice_2->show();
   }
   ui.stackedWidget_beauty->setCurrentIndex(1);
   TRACE6("%s leave\n", __FUNCTION__);
   this->repaint();
}

void VhallIALiveSettingDlg::on_chkbDesktopAudio_stateChanged(int state)
{
   //Qt::Unchecked 取消选中	Qt::Checked 选中
   int iCurState = state;
}

void VhallIALiveSettingDlg::SetCurrentDeviceInfo(const QList<VhallLiveDeviceInfo>& micList, const QString& curMicId, const QList<VhallLiveDeviceInfo>& cameraList, const QString& curCameraId, const QList<VhallLiveDeviceInfo>& playerList, const QString& curPlayerId) {
   TRACE6("%s enter mic Num:%d camera Num:%d player Num:%d\n", __FUNCTION__, micList.size(), cameraList.size(), playerList.size());
   ui.widget_preView->setUpdatesEnabled(true);

   slot_OnSelectVideoPage();
   mbEnableUserSelectCamera = true;
   mbGetCameraEnd = true;
   disconnect(ui.comboBox_cameraSelect, SIGNAL(currentIndexChanged(int)), this, SLOT(slot_CurrentCameraSelectChanged(int)));
   disconnect(ui.comboBox_beautyCamera, SIGNAL(currentIndexChanged(int)), this, SLOT(slot_CurrentBeautyCameraSelectChanged(int)));
   
   disconnect(ui.comboBox_micDev, SIGNAL(currentIndexChanged(int)), this, SLOT(slot_CurrentMicSelectChanged(int)));
   disconnect(ui.comboBox_playerDev, SIGNAL(currentIndexChanged(int)), this, SLOT(slot_CurrentPlayerSelectChanged(int)));

   QString configPath = CPathManager::GetConfigPath();
   ui.comboBox_cameraSelect->clear(); 
   ui.comboBox_beautyCamera->clear();
   if (cameraList.size() > 0) {
      for (int i = 0; i < cameraList.size(); i++) {
         QStringList devList;
         devList.push_back(cameraList.at(i).devId);
         devList.push_back(QString::number(cameraList.at(i).devIndex));
         TRACE6("%s camera_dev_id:%s index:%d\n", __FUNCTION__, cameraList.at(i).devId.toStdString().c_str(), QString::number(cameraList.at(i).devIndex));

         QVariant var(devList);
         ui.comboBox_cameraSelect->addItem(cameraList.at(i).devName, var);
         ui.comboBox_beautyCamera->addItem(cameraList.at(i).devName, var);
         mCurCameraDevID = ConfigSetting::ReadString(configPath, GROUP_DEFAULT, CAPTURE_CAMERA_ID, "");
         if (mCurCameraDevID.isEmpty()) {
            mCurCameraDevID = cameraList.at(i).devId;
            mLastCameraIndex = cameraList.at(i).devIndex;
         }
         if (mCurCameraDevID.compare(cameraList.at(i).devId) == 0) {
            mCurCameraDevID = cameraList.at(i).devId;
            mLastCameraIndex = cameraList.at(i).devIndex;
         }
         mbEnableUserSelectCamera = false;
         mbGetCameraEnd = false;
      }
      ui.comboBox_cameraSelect->setCurrentIndex(mLastCameraIndex);
      ui.comboBox_beautyCamera->setCurrentIndex(mLastCameraIndex);
   }
   else {
      ui.comboBox_cameraSelect->addItem(SETTING_NO_DEV, NO_DEVICE);
      ui.comboBox_beautyCamera->addItem(SETTING_NO_DEV, NO_DEVICE);
   }

   //麦克风显示
   ui.comboBox_micDev->clear();
   for (int i = 0; i < micList.size(); i++) {
      QStringList devList;
      devList.push_back(micList.at(i).devId);
      devList.push_back(QString::number(micList.at(i).devIndex));
      QVariant var(devList);
      ui.comboBox_micDev->addItem(micList.at(i).devName, var);
      mCurMicDevID = ConfigSetting::ReadString(configPath, GROUP_DEFAULT, CAPTURE_MIC_ID, "");
      if (mCurMicDevID.isEmpty()) {
         mCurMicDevID = micList.at(i).devId;
         mLastMicIndex = micList.at(i).devIndex;
      }

      if (mCurMicDevID.compare(micList.at(i).devId) == 0) {
         mLastMicIndex = micList.at(i).devIndex;
      }
   }
   ui.comboBox_micDev->setCurrentIndex(mLastMicIndex);

   //扬声器
   ui.comboBox_playerDev->clear();
   ui.comboBox_playCapture->clear();
   if (playerList.size() > 0) {
      setPlayerVolume(100);
   }
   else {
      setPlayerVolume(0);
   }

   for (int i = 0; i < playerList.size(); i++) {
      QStringList devList;
      devList.push_back(playerList.at(i).devId);
      devList.push_back(QString::number(playerList.at(i).devIndex));
      QVariant var(devList);
      ui.comboBox_playerDev->addItem(playerList.at(i).devName, var);
      ui.comboBox_playCapture->addItem(playerList.at(i).devName, var);
      //桌面采集
      mCapturePlayUid = ConfigSetting::ReadString(configPath, GROUP_DEFAULT, PUSH_DESKTOP_CAPTURE_DEVID, "");
      if (mCapturePlayUid.isEmpty()) {
         mCapturePlayUid = playerList.at(i).devId;
         mCapturePlayIndex = playerList.at(i).devIndex;
      }
      if (mCapturePlayUid.compare(playerList.at(i).devId) == 0) {
         mCapturePlayIndex = playerList.at(i).devIndex;
      }

      //桌面播放扬声器设备
      mCurPlayerID = ConfigSetting::ReadString(configPath, GROUP_DEFAULT, PLAY_OUT_DEV_ID, "");
      if (mCurPlayerID.isEmpty()) {
         mCurPlayerID = playerList.at(i).devId;
         mLastPlayerIndex = playerList.at(i).devIndex;
      }
      if (mCurPlayerID.compare(playerList.at(i).devId) == 0) {
         mLastPlayerIndex = playerList.at(i).devIndex;
      }
   }

   TRACE6("%s cameraList.size():%d playerList.size() %d  mLastPlayerIndex:%d\n", __FUNCTION__, cameraList.size(), playerList.size(), mLastPlayerIndex);
   ui.comboBox_playerDev->setCurrentIndex(mLastPlayerIndex);
   ui.comboBox_playCapture->setCurrentIndex(mCapturePlayIndex);

   mbInitSuc = true;
   connect(ui.comboBox_cameraSelect, SIGNAL(currentIndexChanged(int)), this, SLOT(slot_CurrentCameraSelectChanged(int)));
   connect(ui.comboBox_beautyCamera, SIGNAL(currentIndexChanged(int)), this, SLOT(slot_CurrentBeautyCameraSelectChanged(int)));
   connect(ui.comboBox_micDev, SIGNAL(currentIndexChanged(int)), this, SLOT(slot_CurrentMicSelectChanged(int)));
   connect(ui.comboBox_playerDev, SIGNAL(currentIndexChanged(int)), this, SLOT(slot_CurrentPlayerSelectChanged(int)));

   if (cameraList.size() > 0 && mCurCameraDevID.count() > 0) {
      TRACE6("%s  cameraList.size()\n", __FUNCTION__);
      HandleGetCamera(mCurCameraDevID);
   }

   slot_CurrentMicSelectChanged(mLastMicIndex);

   QString qsConfPath = CPathManager::GetConfigPath();
   QString picPath = ConfigSetting::ReadString(qsConfPath, GROUP_DEFAULT, PUSH_PIC_PATH, "");
   if (picPath.isEmpty()) {
      QString defaultPic = ConfigSetting::ReadString(CPathManager::GetToolConfigPath(), GROUP_DEFAULT, DEFAULT_PIC_NAME, VALUE_PIC_NAME);
      picPath = QApplication::applicationDirPath() + "/" + defaultPic;
   }
   ui.lineEdit_PicPath->setText(picPath);
   if (CheckPicEffectiveness(picPath)) {
      LoadPrivewPic(picPath);
   }
   int openPushPic = ConfigSetting::ReadInt(qsConfPath, GROUP_DEFAULT, PUSH_PIC_OPEN, 0);
   if (openPushPic == 1) {
      ui.radioButton_camera->setChecked(false);
      ui.radioButton_camera->setEnabled(true);
      ui.pushButton_selectPic->setEnabled(true);
      ui.radioButton_pic->setChecked(true);
      ui.stackedWidget_privew->setCurrentIndex(0);
      ui.comboBox_localDefinition->setEnabled(true);
   }
   else {
      ui.radioButton_camera->setChecked(true);
      ui.radioButton_camera->setEnabled(false);
      ui.radioButton_pic->setChecked(false);
      ui.pushButton_selectPic->setEnabled(false);
      ui.stackedWidget_privew->setCurrentIndex(1);
      if (NO_DEVICE == ui.comboBox_cameraSelect->currentData()) {
         ui.comboBox_localDefinition->setEnabled(false);
      }
      else {
         ui.comboBox_localDefinition->setEnabled(true);
      }
   }

   int vol = ConfigSetting::ReadInt(qsConfPath, GROUP_DEFAULT, PUSH_DESKTOP_CAPTURE_VOLUME, 0);
   mCapturePlayValue = ConfigSetting::ReadInt(qsConfPath, GROUP_DEFAULT, PUSH_DESKTOP_CAPTURE_VOLUME, -1);
   bool m_bMicMute = mCapturePlayValue == 0;
   ui.pushButton_deskTopCaptureVolmue->loadPixmap(m_bMicMute ? ":/sysButton/micVolumeUnable" : ":/sysButton/micVolumeAble");
   ui.pushButton_deskTopCaptureVolmue->repaint();

   int enableDeskTopCaptrue = ConfigSetting::ReadInt(qsConfPath, GROUP_DEFAULT, PUSH_DESKTOP_CAPTURE_OPEN, 0);
   ui.chkbDesktopAudio->setChecked(enableDeskTopCaptrue == 1 ? true : false);
   ui.comboBox_playCapture->setEnabled(enableDeskTopCaptrue == 1 ? true : false);
   ui.pushButton_deskTopCaptureVolmue->SetEnabled(enableDeskTopCaptrue == 1 ? true : false);
   ui.speakerVolumeSlider->setEnabled(enableDeskTopCaptrue == 1 ? true : false);
   if (enableDeskTopCaptrue == 1) {
      ui.speakerVolumeSlider->setValue(mCapturePlayValue == -1 ? 100 : mCapturePlayValue);
   }
   TRACE6("%s leave\n", __FUNCTION__);
}

void VhallIALiveSettingDlg::LoadPrivewPic(const QString picPath) {
   TRACE6("%s\n", __FUNCTION__);
   QImage privew_image;
   bool bRet = privew_image.load(picPath);
   if (bRet) {
      QPixmap image = QPixmap::fromImage(privew_image);
      int width = image.width();
      int height = image.height();
      QSize curSize = ui.stackedWidget_privew->size();
      while (true) {
         if (width > curSize.width() || height > curSize.height()) {
            width = width * 5 / 6;
            height = height * 5 / 6;
         }
         else {
            break;
         }
      }

      image = image.scaled(QSize(width, height), Qt::KeepAspectRatio);
      ui.label_previewPic->setFixedSize(QSize(width, height));
      ui.label_previewPic->setPixmap(image);
      ui.label_previewPic->show();
      ui.label_previewPic->setScaledContents(true);
   }
   TRACE6("%s end\n", __FUNCTION__);
}

void VhallIALiveSettingDlg::slot_CurrentCameraSelectChanged(int index) {
   TRACE6("%s index:%d\n", __FUNCTION__, index);
   if (!mbInitSuc) {
      return;
   }
   QStringList cameraList = ui.comboBox_cameraSelect->currentData().toStringList();
   if (cameraList.count() > 0) {
      QString id = cameraList.at(0);
      TRACE6("%s id:%s\n", __FUNCTION__, id.toStdString().c_str());
      HandleGetCamera(id);
   }
   TRACE6("%s end\n", __FUNCTION__);
   disconnect(ui.comboBox_beautyCamera, SIGNAL(currentIndexChanged(int)), this, SLOT(slot_CurrentCameraSelectChanged(int)));
   ui.comboBox_beautyCamera->setCurrentIndex(index);
   connect(ui.comboBox_beautyCamera, SIGNAL(currentIndexChanged(int)), this, SLOT(slot_CurrentCameraSelectChanged(int)));
}

void VhallIALiveSettingDlg::slot_CurrentBeautyCameraSelectChanged(int index) {
   TRACE6("%s index:%d\n", __FUNCTION__, index);
   if (!mbInitSuc) {
      return;
   }
   QStringList cameraList = ui.comboBox_beautyCamera->currentData().toStringList();
   if (cameraList.count() > 0) {
      QString id = cameraList.at(0);
      TRACE6("%s id:%s\n", __FUNCTION__, id.toStdString().c_str());
      HandleGetCamera(id);
   }
   TRACE6("%s end\n", __FUNCTION__);
   disconnect(ui.comboBox_cameraSelect, SIGNAL(currentIndexChanged(int)), this, SLOT(slot_CurrentCameraSelectChanged(int)));
   ui.comboBox_cameraSelect->setCurrentIndex(index);
   connect(ui.comboBox_cameraSelect, SIGNAL(currentIndexChanged(int)), this, SLOT(slot_CurrentCameraSelectChanged(int)));
}

void VhallIALiveSettingDlg::slot_CurrentMicSelectChanged(int index) {
   TRACE6("%s index:%d\n", __FUNCTION__, index);
   if (!mbInitSuc) {
      return;
   }
   TRACE6("%s end\n", __FUNCTION__);
}

void VhallIALiveSettingDlg::OnDesktopEnhanceChanged(int) {

}

void VhallIALiveSettingDlg::slot_CurrentDesktopProfileChanged(int index) {
   if (index == 0) {
      ConfigSetting::writeValue(CPathManager::GetConfigPath(), GROUP_DEFAULT, KEY_DESKTOP_PROFILE, DESKTOP_PROFILE_5FPS);
   }
   else {
      ConfigSetting::writeValue(CPathManager::GetConfigPath(), GROUP_DEFAULT, KEY_DESKTOP_PROFILE, DESKTOP_PROFILE_15FPS);
   }
}

void VhallIALiveSettingDlg::slot_CurrentPlayerSelectChanged(int index) {
   TRACE6("%s index:%d\n", __FUNCTION__, index);
   if (!mbInitSuc) {
      return;
   }
   TRACE6("%s end\n", __FUNCTION__);
}

void VhallIALiveSettingDlg::slot_localDefinitionChanged(int index) {

}

bool VhallIALiveSettingDlg::CheckPicEffectiveness(const QString selectFileName) {
   TRACE6("%s\n", __FUNCTION__);
   QFileInfo fileInfo(selectFileName);
   if (!fileInfo.exists()) {
      TRACE6("%s file not exists\n", __FUNCTION__);
      ui.label_previewPic->clear();
      QRect rect;
      rect.setWidth(320);
      rect.setHeight(240);
      ui.label_previewPic->setGeometry(rect);
      ui.label_previewPic->setFixedSize(QSize(320, 240));
      ui.label_previewPic->setText(QString::fromWCharArray(L"找不到图片，请重新选择"));
      ui.label_previewPic->repaint();
      ui.stackedWidget_privew->setCurrentIndex(0);
      TRACE6("%s file not exists end\n", __FUNCTION__);
      return false;
   }
   else {
      TRACE6("%s CheckPicEffectiveness start\n", __FUNCTION__);
      int nRet = vlive::GetWebRtcSDKInstance()->CheckPicEffectiveness(selectFileName.toStdString());
      if (nRet != 0) {
         TRACE6("%s nRet:%d\n", __FUNCTION__, nRet);
         ui.label_previewPic->clear();
         QRect rect;
         rect.setWidth(320);
         rect.setHeight(240);
         ui.label_previewPic->setGeometry(rect);
         ui.label_previewPic->setFixedSize(QSize(320, 240));
         ui.label_previewPic->setText(SETTING_NO_IMAGE);
         ui.label_previewPic->repaint();
         ui.stackedWidget_privew->setCurrentIndex(0);
         TRACE6("%s CheckPicEffectiveness err\n", __FUNCTION__);
         return false;
      }
   }
   TRACE6("%s  end\n", __FUNCTION__);
   return true;
}

void VhallIALiveSettingDlg::slot_OpenPushPic(bool checked) {
   if (checked) {
      ui.comboBox_localDefinition->setEnabled(true);

      QString selectFileName = ui.lineEdit_PicPath->text();
      if (CheckPicEffectiveness(selectFileName)) {
         LoadPrivewPic(selectFileName);
         ui.stackedWidget_privew->setCurrentIndex(0);
      }
      ui.radioButton_camera->setChecked(false);
      ui.radioButton_camera->setEnabled(true);
      ui.radioButton_pic->setEnabled(false);   
      ui.pushButton_selectPic->setEnabled(true);
   }
   else {
      ui.pushButton_selectPic->setEnabled(false);
   }
}

void VhallIALiveSettingDlg::slot_RadioCamera(bool checked) {
   if (checked) {
      ui.stackedWidget_privew->setCurrentIndex(1);
      if (NO_DEVICE == ui.comboBox_cameraSelect->currentData()) {
         ui.comboBox_localDefinition->setEnabled(false);
      }
      else {
         ui.comboBox_localDefinition->setEnabled(true);
      }
      ui.radioButton_pic->setEnabled(true);
      ui.radioButton_pic->setChecked(false);
      ui.pushButton_selectPic->setEnabled(false);

      ui.radioButton_camera->setEnabled(false);
   }
   else {

   }
}

void VhallIALiveSettingDlg::slot_OpenDesktopAudio(bool checked) {
   bool bIsCapturingStream = vlive::GetWebRtcSDKInstance()->IsCapturingStream(VHStreamType_MediaFile);
   if (bIsCapturingStream) {
      QString msg = SETTING_CLOSE_FILE_TO_SHARE;
      AlertTipsDlg tip(msg, true, this);
      tip.CenterWindow(this);
      tip.SetYesBtnText(CONFIRM_CLOSE);
      tip.SetNoBtnText(NOT_CLOSE);
      if (tip.exec() == QDialog::Accepted && parentWdg) {
         QApplication::postEvent(parentWdg, new CustomOnHttpResMsgEvent(CustomEvent_StopCaptureMediaFile));
      }
   }

   ui.comboBox_playCapture->setEnabled(checked);
   ui.speakerVolumeSlider->setEnabled(checked);
   ui.pushButton_deskTopCaptureVolmue->SetEnabled(checked);
}

void VhallIALiveSettingDlg::slot_OnMicMuteClick() {
   bool bMute = mCapturePlayValue != 0;
   ui.pushButton_deskTopCaptureVolmue->loadPixmap(bMute ? ":/sysButton/micVolumeUnable" : ":/sysButton/micVolumeAble");
   ui.pushButton_deskTopCaptureVolmue->repaint();
   if (bMute) {
      mCapturePlayValue = 0;
   }
   else {
      mCapturePlayValue = 100;
   }
   ui.speakerVolumeSlider->setValue(mCapturePlayValue);
}

void VhallIALiveSettingDlg::slot_OnSpeakerVolumeChange(int nValue) {
   bool bMute = nValue == 0;
   ui.pushButton_deskTopCaptureVolmue->loadPixmap(bMute ? ":/sysButton/micVolumeUnable" : ":/sysButton/micVolumeAble");
   ui.pushButton_deskTopCaptureVolmue->repaint();
   vlive::GetWebRtcSDKInstance()->SetLocalCapturePlayerVolume(nValue);
   mCapturePlayValue = nValue;
}

void VhallIALiveSettingDlg::slot_SelectplayCapture(int index) {
   if (ui.comboBox_playCapture->count() == 0) {
      return;
   }
   QStringList curData = ui.comboBox_playCapture->currentData().toStringList();
   if (curData.count() == 2) {
      mCapturePlayUid = curData.at(0);
      mCapturePlayIndex = curData.at(1).toInt();
   }
}

void VhallIALiveSettingDlg::slot_SelectPushPic() {
   ui.pushButton_selectPic->setEnabled(false);
   TRACE6("%s \n", __FUNCTION__);
   QString selectFileName;
   TCHAR szPathName[2048];

   OPENFILENAME ofn = { sizeof(OPENFILENAME) };
   ofn.hwndOwner = NULL;
   ofn.lpstrFilter = TEXT("*.jpg *.JPG *.png *.PNG\0*.jpg;*.JPG;*.png;*.PNG\0\0");
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
   if (selectFileName.isEmpty()) {
      TRACE6("%s selectFileName is empty\n", __FUNCTION__);
      if (ui.radioButton_pic->isChecked()) {
         ui.pushButton_selectPic->setEnabled(true);
      }
      return;
   }
   if (CheckPicEffectiveness(selectFileName) && ui.radioButton_pic->isChecked()) {
      TRACE6("%s\n", __FUNCTION__);
      ui.lineEdit_PicPath->setText(selectFileName);
      LoadPrivewPic(selectFileName);
      ui.stackedWidget_privew->setCurrentIndex(0);

   }
   if (ui.radioButton_pic->isChecked()) {
      ui.pushButton_selectPic->setEnabled(true);
   }
   TRACE6("%s end\n", __FUNCTION__);
}

void VhallIALiveSettingDlg::slot_SelectFloatMode() {
   TRACE6("%s \n", __FUNCTION__);
   if (mpMediaFile == NULL) {
      return;
   }
   if (mpMediaFile->IsPlayMediaFile() || mpMediaFile->IsPlayDesktopShare()) {
      ui.label_layoutChangeTips->show();
      return;
   }
   ui.label_layoutChangeTips->hide();
   TRACE6("%s LAYOUT_MODE_FLOAT\n", __FUNCTION__);
   meLayoutMode = LAYOUT_MODE_FLOAT;
   ui.pushButton_gridMode->setStyleSheet(LAYOUT_GRID_DISABLE);
   ui.pushButton_floatMode->setStyleSheet(LAYOUT_FLOAT_ENABLE);
   ui.pushButton_tiledMode->setStyleSheet(LAYOUT_TILED_DISABLE);
}

void VhallIALiveSettingDlg::slot_SelectTiledMode() {
   TRACE6("%s \n", __FUNCTION__);
   if (mpMediaFile == NULL) {
      return;
   }
   if (mpMediaFile->IsPlayMediaFile() || mpMediaFile->IsPlayDesktopShare()) {
      ui.label_layoutChangeTips->show();
      return;
   }
   ui.label_layoutChangeTips->hide();
   TRACE6("%s LAYOUT_MODE_TILED\n", __FUNCTION__);
   meLayoutMode = LAYOUT_MODE_TILED;
   ui.pushButton_gridMode->setStyleSheet(LAYOUT_GRID_DISABLE);
   ui.pushButton_floatMode->setStyleSheet(LAYOUT_FLOAT_DISABLE);
   ui.pushButton_tiledMode->setStyleSheet(LAYOUT_TILED_ENABLE);
}

void VhallIALiveSettingDlg::slot_SelectGridMode() {
   TRACE6("%s \n", __FUNCTION__);
   if (mpMediaFile == NULL) {
      return;
   }
   if (mpMediaFile->IsPlayMediaFile() || mpMediaFile->IsPlayDesktopShare()) {
      ui.label_layoutChangeTips->show();
      return;
   }
   ui.label_layoutChangeTips->hide();
   TRACE6("%s LAYOUT_MODE_GRID\n", __FUNCTION__);
   meLayoutMode = LAYOUT_MODE_GRID;
   ui.pushButton_gridMode->setStyleSheet(LAYOUT_GRID_ENABLE);
   ui.pushButton_floatMode->setStyleSheet(LAYOUT_FLOAT_DISABLE);
   ui.pushButton_tiledMode->setStyleSheet(LAYOUT_TILED_DISABLE);
}


bool VhallIALiveSettingDlg::eventFilter(QObject *obj, QEvent *event) {
   if (obj && event) {
      if (event->type() == QEvent::Enter) {
         if (obj == ui.label_tips && m_pNoiseTips != NULL) {
            m_pNoiseTips->setFixedHeight(130);
            m_pNoiseTips->setText(SETTING_LAYOUT_TIPS);
            m_pNoiseTips->show();
            QPoint pos = this->mapToGlobal(ui.label_tips->pos());
            m_pNoiseTips->move(pos + QPoint(0, 90));
         }
         else 	if (ui.labDesktopAudio == obj && m_pNoiseTips != NULL){
            if (NULL != m_pNoiseTips){
               m_pNoiseTips->setFixedHeight(130);
               m_pNoiseTips->setText(SETTING_DESKTOP_AUDIO_TIPS);
               m_pNoiseTips->show();
               QPoint pos = this->mapToGlobal(ui.labDesktopAudio->pos());
               m_pNoiseTips->move(pos + QPoint(0, 60));
            }
         }
         else if (ui.label_DesktopProfileTips == obj && m_pNoiseTips != NULL) {
            if (NULL != m_pNoiseTips) {
               m_pNoiseTips->setFixedHeight(130);
               m_pNoiseTips->setFixedWidth(400);
               m_pNoiseTips->setText(SETTING_DESKTOP_FPS);
               m_pNoiseTips->show();
               QPoint pos =  ui.label_DesktopProfileTips->mapToGlobal(QPoint(0, 0)); //this->mapToGlobal(ui.label_DesktopProfileTips->pos());
               m_pNoiseTips->move(pos + QPoint(-(m_pNoiseTips->width() / 2), 30));
            }
         }
         else if (ui.label_desktop_hance == obj && m_pNoiseTips != NULL) {
            m_pNoiseTips->setText(QString::fromWCharArray(L"使用桌面共享演示文档时，将对图片和文字进行锐化处理，改善观看体验，但会消耗额外计算资源，建议在高性能电脑上开启此功能。"));
            m_pNoiseTips->setFixedHeight(130);
            m_pNoiseTips->show();
            QPoint pos = this->mapToGlobal(ui.label_desktop_hance->pos());
            m_pNoiseTips->move(pos + QPoint(20, 400));
         }
      }
      else if (event->type() == QEvent::Leave) {
         if ((obj == ui.labDesktopAudio || obj == ui.label_tips || ui.label_DesktopProfileTips == obj || ui.label_desktop_hance == obj) && m_pNoiseTips != NULL) {
            m_pNoiseTips->hide();
         }
      }
   }
   return QWidget::eventFilter(obj, event);
}

void VhallIALiveSettingDlg::mousePressEvent(QMouseEvent *event) {
   if (event) {
      mPressPoint = event->pos();
      if (ui.frame->geometry().contains(mPressPoint))
         mIsMoved = true;
   }
}

void VhallIALiveSettingDlg::mouseMoveEvent(QMouseEvent *event) {
   if (event && (event->buttons() == Qt::LeftButton) && mIsMoved  && m_bIsEnableMove) {
      QPoint parent_point = pos();
      parent_point.setX(parent_point.x() + event->x() - mPressPoint.x());
      parent_point.setY(parent_point.y() + event->y() - mPressPoint.y());
      move(parent_point);
   }
}

void VhallIALiveSettingDlg::mouseReleaseEvent(QMouseEvent *) {
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

int VhallIALiveSettingDlg::GetLevelFromSlider(int value) {
   int level = 0;
   if (value == 0) {
      level = 0;
   }
   else if (value > 0 && value <= 20) {
      level = 1;
   }
   else if (value > 20 && value <= 40) {
      level = 2;
   }
   else if (value > 40 && value <= 60) {
      level = 3;
   }
   else if (value > 60 && value <= 80) {
      level = 4;
   }
   else if (value > 80 && value <= 100) {
      level = 5;
   }
   TRACE6("%s value %d level %d", __FUNCTION__, value, level);
   return level;
}

void VhallIALiveSettingDlg::OnBeautyValueChange(int value) {
   int level = GetLevelFromSlider(value);
   if (mLastLevel != level) {
      mLastLevel = level;
      GetWebRtcSDKInstance()->SetPreviewCameraBeautyLevel(level);
      QString qsConfPath = CPathManager::GetConfigPath();
      QString configCameraDevID = ConfigSetting::ReadString(qsConfPath, GROUP_DEFAULT, CAPTURE_CAMERA_ID, "");
      if (!mbGetCameraSuccess || configCameraDevID == mCurCameraDevID) {
         GetWebRtcSDKInstance()->SetCameraBeautyLevel(level);
      }
   }
}

void VhallIALiveSettingDlg::slot_OnAdvancedSetting() {
   GUID matchGUID;
   DeviceInfo deviceInfo;
   QStringList cameraList = ui.comboBox_cameraSelect->currentData().toStringList();
   if (cameraList.count() > 0) {
      QString id = cameraList.at(0);
      TRACE6("%s id:%s\n", __FUNCTION__, id.toStdString().c_str());
      memcpy(deviceInfo.m_sDeviceName, ui.comboBox_cameraSelect->currentText().toStdWString().c_str(), ui.comboBox_cameraSelect->currentText().toStdWString().length());
      memcpy(deviceInfo.m_sDeviceID, id.toStdWString().c_str(), id.toStdWString().length());
      matchGUID = CLSID_VideoInputDeviceCategory;

      IBaseFilter *filter = GetDeviceByValue(matchGUID,L"FriendlyName", ui.comboBox_cameraSelect->currentText().toStdWString().c_str(),L"DevicePath", id.toStdWString().c_str());
      if (!filter) {
         return;
      }

      ISpecifyPropertyPages *propPages = NULL;
      CAUUID cauuid;
      if (SUCCEEDED(filter->QueryInterface(IID_ISpecifyPropertyPages, (void**)&propPages))) {
         if (SUCCEEDED(propPages->GetPages(&cauuid))) {
            if (cauuid.cElems) {
               OleCreatePropertyFrame((HWND)(this->winId()), 0, 0, NULL, 1, (LPUNKNOWN*)&filter, cauuid.cElems, cauuid.pElems, 0, 0, NULL);
               CoTaskMemFree(cauuid.pElems);
            }
         }
         propPages->Release();
      }
   }
}

IBaseFilter* VhallIALiveSettingDlg::GetDeviceByValue(const IID &enumType, const WCHAR* lpType, const WCHAR* lpName, const WCHAR* lpType2, const WCHAR* lpName2) {
   ICreateDevEnum *deviceEnum;
   IEnumMoniker *videoDeviceEnum;
   HRESULT err;
   err = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, (void**)&deviceEnum);
   if (FAILED(err) || deviceEnum == NULL) {
      return NULL;
   }
   err = deviceEnum->CreateClassEnumerator(enumType, &videoDeviceEnum, 0);
   if (FAILED(err) || videoDeviceEnum == NULL) {
      deviceEnum->Release();
      return NULL;
   }
   SafeRelease(deviceEnum);

   if (err == S_FALSE) //no devices, so NO ENUM FO U
      return NULL;

   IBaseFilter *bestFilter = NULL;
   IMoniker *deviceInfo;
   DWORD count;
   while (videoDeviceEnum->Next(1, &deviceInfo, &count) == S_OK) {
      IPropertyBag *propertyData;
      err = deviceInfo->BindToStorage(0, 0, IID_IPropertyBag, (void**)&propertyData);
      if (SUCCEEDED(err)) {
         VARIANT valueThingy;
         VARIANT valueThingy2;
         VariantInit(&valueThingy);
         VariantInit(&valueThingy2);
         if (SUCCEEDED(propertyData->Read(lpType, &valueThingy, NULL))) {
            if (lpType2 && lpName2) {
               if (FAILED(propertyData->Read(lpType2, &valueThingy2, NULL))) {
               }
            }
            SafeRelease(propertyData);
            wstring strVal1 = (const WCHAR*)valueThingy.bstrVal;
            if (0 == wcscmp(strVal1.c_str(), lpName) || 0 == wcscmp(lpName, L"Default")) {
               IBaseFilter *filter = NULL;
               err = deviceInfo->BindToObject(NULL, 0, IID_IBaseFilter, (void**)&filter);
               if (FAILED(err)) {
                  ASSERT(FALSE);
                  continue;
               }
               if (0 == wcscmp(lpName, L"Default")) {
                  if (!bestFilter) {
                     bestFilter = filter;
                     SafeRelease(deviceInfo);
                     SafeRelease(videoDeviceEnum);
                     return bestFilter;
                  }
               }
               if (!bestFilter) {
                  bestFilter = filter;
                  if (!lpType2 || !lpName2) {
                     SafeRelease(deviceInfo);
                     SafeRelease(videoDeviceEnum);                     return bestFilter;
                  }
               }
               else if (lpType2 && lpName2) {
                  wstring strVal2 = (const WCHAR*)valueThingy2.bstrVal;
                  if (0 == wcscmp(strVal2.c_str(), lpName2)) {
                     bestFilter->Release();
                     bestFilter = filter;
                     SafeRelease(deviceInfo);
                     SafeRelease(videoDeviceEnum);
                     return bestFilter;
                  }
               }
               else
                  filter->Release();
            }
         }
      }
      SafeRelease(deviceInfo);
   }
   SafeRelease(videoDeviceEnum);
   return bestFilter;
}

void VhallIALiveSettingDlg::OnMaxLength() {
   QString textContent = ui.textBrowser_Content->toPlainText();
   int length = textContent.count();
   int maxLength = 300; // 最大字符数
   if (length > maxLength) {
      int position = ui.textBrowser_Content->textCursor().position();
      QTextCursor textCursor = ui.textBrowser_Content->textCursor();
      textContent.remove(position - (length - maxLength), length - maxLength);
      ui.textBrowser_Content->setText(textContent);
      textCursor.setPosition(position - (length - maxLength));
      ui.textBrowser_Content->setTextCursor(textCursor);
   }
}

VideoProfileIndex VhallIALiveSettingDlg::GetPushPicStreamProfile(VideoProfileIndex camera_profile) {
   if (mbIsHost) {
      if (camera_profile == RTC_VIDEO_PROFILE_480P_16x9_M) {
         return RTC_VIDEO_PROFILE_480P_16x9_L;
      }
      else if (camera_profile == RTC_VIDEO_PROFILE_360P_16x9_M) {
         return RTC_VIDEO_PROFILE_360P_16x9_L;
      }
      else if (camera_profile == RTC_VIDEO_PROFILE_240P_16x9_M) {
         return RTC_VIDEO_PROFILE_240P_16x9_L;
      }
      else {
         return RTC_VIDEO_PROFILE_360P_16x9_L;
      }
   }
   else {
      return RTC_VIDEO_PROFILE_360P_16x9_L;
   }
}

VideoProfileIndex VhallIALiveSettingDlg::GetPushStreamProfileByStreamType(int streamType) {
   QString curProfile = ConfigSetting::ReadString(CPathManager::GetConfigPath(), GROUP_DEFAULT, KEY_LOCAL_VIDEO_PROFILE, PROFILE_INDEX_STANDER);
   if (streamType == VHStreamType_AVCapture) {
      if (mbIsHost) {
         if (curProfile == PROFILE_INDEX_HIGH) {
            return RTC_VIDEO_PROFILE_480P_16x9_M;//RTC_VIDEO_PROFILE_540P_16x9_M;
         }
         else if (curProfile == PROFILE_INDEX_STANDER) {
            return RTC_VIDEO_PROFILE_360P_16x9_M;//RTC_VIDEO_PROFILE_480P_16x9_M;
         }
         else if (curProfile == PROFILE_INDEX_FLUENCY) {
            return RTC_VIDEO_PROFILE_240P_16x9_M;//RTC_VIDEO_PROFILE_360P_16x9_M;
         }
      }
      else {
         return RTC_VIDEO_PROFILE_360P_16x9_M;//RTC_VIDEO_PROFILE_480P_16x9_M;
      }
   }
   else if (streamType == VHStreamType_Desktop) {
      QString desktop_profile = ConfigSetting::ReadString(CPathManager::GetConfigPath(), GROUP_DEFAULT, KEY_DESKTOP_PROFILE, DESKTOP_PROFILE_5FPS);
      if (desktop_profile == DESKTOP_PROFILE_5FPS) {
         return RTC_VIDEO_PROFILE_1080P_16x9_L;
      }
      else {
         return RTC_VIDEO_PROFILE_1080P_16x9_H;
      }
   }
   else if (streamType == VHStreamType_MediaFile) {
      if (curProfile == PROFILE_INDEX_HIGH) {
         return RTC_VIDEO_PROFILE_720P_16x9_H/*VIDEO_PROFILE_720P_1_25F*/;
      }
      else if (curProfile == PROFILE_INDEX_STANDER) {
         return RTC_VIDEO_PROFILE_720P_16x9_M/*VIDEO_PROFILE_720P_1_15F*/;
      }
      else if (curProfile == PROFILE_INDEX_FLUENCY) {
         return RTC_VIDEO_PROFILE_720P_16x9_M/*VIDEO_PROFILE_720P_1_15F*/;
      }
   }
   return VIDEO_PROFILE_UNDEFINED;
}

VideoProfileIndex VhallIALiveSettingDlg::GetPlayMediaFileProfile(QString file) {
   VideoProfileIndex profileIndex = RTC_VIDEO_PROFILE_720P_16x9_H;
   int iSourceW = 0;
   int iSourceH = 0;
   int nRet = GetWebRtcSDKInstance()->GetPlayMeidaFileWidthAndHeight(file.toStdString(), iSourceW, iSourceH);
   if (nRet == 0) {
      if (iSourceW <= 160 && iSourceH <= 90) {
         profileIndex = RTC_VIDEO_PROFILE_90P_16x9_H/*VIDEO_PROFILE_120P_1_25F*/;
      }
      else if (iSourceW <= 160 && iSourceH <= 120) {
         profileIndex = RTC_VIDEO_PROFILE_120P_4x3_H/*VIDEO_PROFILE_120P_0_25F*/;
      }
      else if (iSourceW <= 240 && iSourceH <= 180) {
         profileIndex = RTC_VIDEO_PROFILE_180P_16x9_H/*VIDEO_PROFILE_180P_0_25F*/;
      }
      else if (iSourceW <= 320 && iSourceH <= 180) {
         profileIndex = RTC_VIDEO_PROFILE_180P_16x9_H;
      }
      else if (iSourceW <= 320 && iSourceH <= 240) {
         profileIndex = RTC_VIDEO_PROFILE_240P_4x3_H/*VIDEO_PROFILE_240P_0_25F*/;
      }
      else if (iSourceW <= 424 && iSourceH <= 240) {
         profileIndex = RTC_VIDEO_PROFILE_240P_16x9_H;
      }
      else if (iSourceW <= 480 && iSourceH <= 360) {
         profileIndex = RTC_VIDEO_PROFILE_360P_4x3_H /*VIDEO_PROFILE_360P_0_25F*/;
      }
      else if (iSourceW <= 640 && iSourceH <= 360) {
         profileIndex = RTC_VIDEO_PROFILE_360P_16x9_H;
      }
      else if (iSourceW <= 640 && iSourceH <= 480) {
         profileIndex = RTC_VIDEO_PROFILE_480P_4x3_H;
      }
      else if (iSourceW <= 848 && iSourceH <= 480) {
         profileIndex = RTC_VIDEO_PROFILE_480P_16x9_H;
      }
      else if (iSourceW <= 960 && iSourceH <= 720) {
         profileIndex = RTC_VIDEO_PROFILE_720P_4x3_H;
      }
      else {
         profileIndex = RTC_VIDEO_PROFILE_720P_16x9_H;
      }
      return profileIndex;
   }
   return VIDEO_PROFILE_UNDEFINED;
}

BroadCastVideoProfileIndex VhallIALiveSettingDlg::GetBroadCastVideoProfileIndex() {
    QString broadCastProfile = ConfigSetting::ReadString(CPathManager::GetToolConfigPath(), GROUP_DEFAULT, KEY_BROADCAST_VIDEO_PROFILE, "BROADCAST_VIDEO_PROFILE_1080P_16x9_25F");
    BroadCastVideoProfileIndex index = BROADCAST_VIDEO_PROFILE_1080P_16x9_25F/*BROADCAST_VIDEO_PROFILE_1080P_1*/;
    if (broadCastProfile == "BROADCAST_VIDEO_PROFILE_1080P_16x9_25F") {
        index = BROADCAST_VIDEO_PROFILE_1080P_16x9_25F/*BROADCAST_VIDEO_PROFILE_1080P_1*/;
    }
    else if (broadCastProfile == "BROADCAST_VIDEO_PROFILE_810P_16x9_5F") {
        index = BROADCAST_VIDEO_PROFILE_810P_16x9_5F /*BROADCAST_VIDEO_PROFILE_810P_1_5F*/;
    }
    else if (broadCastProfile == "BROADCAST_VIDEO_PROFILE_1080P_16x9_15F") {
        index = BROADCAST_VIDEO_PROFILE_1080P_16x9_15F/*BROADCAST_VIDEO_PROFILE_1080P_0_10F*/;
    }
    else if (broadCastProfile == "BROADCAST_VIDEO_PROFILE_810P_16x9_15F") {
        index = BROADCAST_VIDEO_PROFILE_810P_16x9_15F /*BROADCAST_VIDEO_PROFILE_810P_1_10F*/;
    }
    else if (broadCastProfile == "BROADCAST_VIDEO_PROFILE_1080P_4x3_25F") {
        index = BROADCAST_VIDEO_PROFILE_1080P_4x3_25F/*BROADCAST_VIDEO_PROFILE_1080P_0*/;
    }  
    else if (broadCastProfile == "BROADCAST_VIDEO_PROFILE_960P_4x3_25F") {
        index = BROADCAST_VIDEO_PROFILE_960P_4x3_25F/*BROADCAST_VIDEO_PROFILE_960P_0*/;
    } 
    else if (broadCastProfile == "BROADCAST_VIDEO_PROFILE_720P_16x9_25F") {
        index = BROADCAST_VIDEO_PROFILE_720P_16x9_25F;
    }
    else if (broadCastProfile == "BROADCAST_VIDEO_PROFILE_720P_4x3_25F") {
        index = BROADCAST_VIDEO_PROFILE_720P_4x3_25F;
    }
    else if (broadCastProfile == "BROADCAST_VIDEO_PROFILE_540P_16x9_25F") {
        index = BROADCAST_VIDEO_PROFILE_540P_16x9_25F;
    }
    else if (broadCastProfile == "BROADCAST_VIDEO_PROFILE_540P_4x3_25F") {
        index = BROADCAST_VIDEO_PROFILE_540P_4x3_25F;
    }
    else if (broadCastProfile == "BROADCAST_VIDEO_PROFILE_480P_16x9_25F") {
        index = BROADCAST_VIDEO_PROFILE_480P_16x9_25F;
    }
    else if (broadCastProfile == "BROADCAST_VIDEO_PROFILE_480P_4x3_25F") {
        index = BROADCAST_VIDEO_PROFILE_480P_4x3_25F;
    }
    TRACE6("%s index:%d\n", __FUNCTION__, index);
    return index;
}