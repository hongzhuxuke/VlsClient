#include "stdafx.h"
#include "SystemSettingDlg.h"
#include "TitleWidget.h"

#include <QBoxLayout>
#include <QFileDialog>
#include <QStandardPaths>
#include <QMessageBox>
#include <QPainter>
#include <QDesktopWidget>
#include <QListView>
#include <QToolTip>
#include <QtNetwork/QNetworkProxy>
#include <QListWidget>
#include <QDesktopServices>
#include <QButtonGroup>
#include "VhallNetWorkInterface.h"
#include "VH_Macro.h"
#include "IDeviceManager.h"
#include "ICommonData.h"
#include "IDShowPlugin.h"
#include "ConfigSetting.h"
#include "ToolButton.h"
#include "ToolButtonEx.h"
#include "push_button.h"
#include "IDeckLinkDevice.h"
#include "Msg_MainUI.h"
#include "Msg_OBSControl.h"  
#include "vhproxytest.h"
#include "pathManage.h"
#include "pathmanager.h"
#include "pub.Const.h"
#include "pub.Struct.h"
#include "HttpProxyGet.h"
#include "WebRtcSDKInterface.h"


#define OOPACITY 0.99
#define MAX_SUPPORT_W   848
#define MAX_SUPPORT_H   480
#define  MAXINPUTSIZE 20
#define NO_INPUT  QString::fromWCharArray(L"请勾选摄像设备后重试")
#define MIC_E_SILDER_DISABLE_TOOLTIP "该硬件设备不支持麦克风增强功能"
#define MIC_CHANNEL_COMMON_TOOLTIP "不勾选时，声道效果为单声道。\n单声道：播放时只通过一个声道来播放声音。\n立体声：播放时使用两个不同的声道来播放声音，以达到立体声的效果。"
#define DESKTOP_SPEAKER_COMMON_TOOLTIP "采集电脑桌面的声音；\n打开后会传输电脑桌面中播放的声音；\n关闭后观众将无法听到电脑桌面中的声音；"
#define BUTTON_DOWN_STYLE   "\
                           QPushButton{ \
                           background-color:#5A393A;border:0px; \
                           font-size:14px;\
                           font-family:微软雅黑; \
                           color:#FFFFFF;} "

#define BUTTON_UP_STYLE    "QPushButton{ \
                           background-color:#323232; border:0px;\
                           font-size:14px;\
                           font-family:微软雅黑; \
                           color:#D6D6D6;} \
                           QPushButton:hover{background-color:#5A393A;border:0px;color:#FFFFFF;font-size:14px;font-family:微软雅黑; }"

#define DesktopCaptureTips_enable QString("font-size : 12px;font-family : 微软雅黑;color : #D6D6D6;")
#define DesktopCaptureTips_disable QString("font-size : 12px;font-family : 微软雅黑;color: rgb(142, 142, 142);")

const char limitChar[] = { '\\', '/', ':', '*', '?', '<', '>', '|' };

QString GetformatStr_x(VideoFormat format) {
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

void CameraSettingUIResolutionList::Clear() {
   resList.clear();
   m_currentIndex = 0;
}
void CameraSettingUIResolutionList::sort() {
   for (int i = 0; i < resList.count(); i++) {
      for (int j = i + 1; j < resList.count(); j++) {
         if (resList[i].w < resList[j].w) {
            resList.swap(i, j);
         }
      }
   }

}
void CameraSettingUIResolutionList::Append(FrameInfo info) {
   bool isAdd = false;
   for (int i = 0; i < resList.count(); i++) {
      CameraSettingUIResolution res = resList[i];
      if (res.w == info.maxCX&&
         res.h == info.maxCY&&
         res.format == info.format) {
         isAdd = true;
         CameraSettingUIFrameInternal frameInternal;
         frameInternal.minInternal = info.minFrameInterval;
         frameInternal.maxInternal = info.maxFrameInterval;
         resList[i].frameInternalList.append(frameInternal);
         break;
      }
   }
   if (!isAdd) {
      CameraSettingUIResolution res;
      res.w = info.maxCX;
      res.h = info.maxCY;
      res.format = info.format;
      CameraSettingUIFrameInternal frameInternal;
      frameInternal.minInternal = info.minFrameInterval;
      frameInternal.maxInternal = info.maxFrameInterval;
      res.frameInternalList.append(frameInternal);
      resList.append(res);
   }
}
int CameraSettingUIResolutionList::Count() {
   return resList.count();
}
bool CameraSettingUIResolutionList::GetFrameInfo(int index, CameraSettingUIResolution *frameInfo) {
   if (index < 0 || index >= resList.count() || !frameInfo) {
      return false;
   }

   *frameInfo = resList[index];
   return true;
}
QString CameraSettingUIResolutionList::DisplayString(int index) {
   if (index < 0 || index >= resList.count()) {
      return "ERROR";
   }

   CameraSettingUIResolution res = resList.at(index);
   CameraSettingUIFrameInternal frameInternal = res.frameInternalList[0];
#ifdef _DEBUG
   return QString("%1x%2 %3 %4 %5")
      .arg(res.w)
      .arg(res.h)
      .arg(GetformatStr_x(res.format))
      .arg(frameInternal.minInternal)
      .arg(frameInternal.maxInternal);
#else
   return QString("%1x%2 %3")
      .arg(res.w)
      .arg(res.h)
      .arg(GetformatStr_x(res.format));

#endif
}
int CameraSettingUIResolutionList::SetCurrentFrameInfo(FrameInfo info) {
   int index = 0;
   for (int i = 0; i < resList.count(); i++) {
      CameraSettingUIResolution res = resList.at(i);
      if (res.w == info.maxCX&&res.h == info.maxCY&&res.format == info.format) {
         index = i;
         break;
      }
   }

   m_currentIndex = index;
   return index;
}
void CameraSettingUIResolutionList::SetCurrentIndex(int index) {
   m_currentIndex = index;
}
VideoFormat CameraSettingUIResolutionList::GetCurrentVideoFormat() {
   if (m_currentIndex < resList.count()) {
      CameraSettingUIResolution res = resList.at(m_currentIndex);
      return res.format;
   }
   return VideoFormat::Any;
}
bool CameraSettingUIResolutionList::GetCurrentResolution(int &w, int &h) {
   if (m_currentIndex < resList.count()) {
      CameraSettingUIResolution res = resList.at(m_currentIndex);
      w = res.w;
      h = res.h;
      return true;
   }
   return false;
}

//获得当前的帧率
UINT64 CameraSettingUIResolutionList::GetCurrentFrameInternal(QString str) {
   UINT64 frameInternal = 333333;
   if (m_currentIndex < resList.count()) {
      bool ok = false;
      double resolution = str.toDouble(&ok);
      CameraSettingUIResolution res = resList.at(m_currentIndex);
      if (!ok) {
         frameInternal = res.frameInternalList[0].minInternal;
      }
      else {
         double tmpFrameInternal = 10000000;
         tmpFrameInternal /= resolution;
         frameInternal = tmpFrameInternal;
         bool find = false;
         for (int i = 0; i < res.frameInternalList.count(); i++) {
            CameraSettingUIFrameInternal frameInternalPair = res.frameInternalList[i];
            if (frameInternal <= frameInternalPair.maxInternal
               &&frameInternal >= frameInternalPair.minInternal) {
               find = true;
               break;
            }
         }

         if (!find) {
            UINT minInternal = 0;
            UINT maxInternal = 0;
            for (int i = 0; i < res.frameInternalList.count(); i++) {
               CameraSettingUIFrameInternal frameInternalPair = res.frameInternalList[i];
               if (i == 0) {
                  minInternal = frameInternalPair.minInternal;
                  maxInternal = frameInternalPair.maxInternal;
               }
               else {
                  if (minInternal > frameInternalPair.minInternal) {
                     minInternal = frameInternalPair.minInternal;
                  }
                  if (maxInternal < frameInternalPair.maxInternal) {
                     maxInternal = frameInternalPair.maxInternal;
                  }
               }
            }

            if (frameInternal < minInternal) {
               frameInternal = minInternal;
            }

            if (frameInternal > maxInternal) {
               frameInternal = maxInternal;
            }
         }

      }
   }

   return frameInternal;
}

SystemSettingDlg::SystemSettingDlg(QDialog *parent)
   : VHDialog(parent)
   , m_pApplyBtn(NULL)
   , m_pChangeDir(NULL)
   , m_pOpenCurDir(NULL)
{
   ui.setupUi(this);
   ui.pushButton_debug->hide();//不显示调试信息。
   //程序内部置顶
   setModal(true);
   this->installEventFilter(this);
   currentParent = parent;
   this->setWindowTitle(SETTING);
   QString toolConfPath = CPathManager::GetAppDataPath() + QString::fromStdWString(VHALL_TOOL_CONFIG);
   int hideLogo = ConfigSetting::ReadInt(toolConfPath, GROUP_DEFAULT, KEY_VHALL_LOGO_HIDE, 0);
   if (hideLogo == 1) {
      TRACE6("%s appEnableUpdata:%d\n", __FUNCTION__);
      ui.label_copyright->hide();
      ui.label_vhallTel->hide();
      ui.label_notice->hide();
   }
   ui.cmbAudioSampleRate->hide();
   ui.label_25->hide();
   ui.labelNoiseReductionTips->hide();
   ui.labAudioSampleRate->hide();
   mPlayerDevId = ConfigSetting::ReadString(CPathManager::GetConfigPath(), GROUP_DEFAULT, PLAYER_DEFAULT_DEVICE, "");
   ui.comboBox_beautyCamera->addItem(NO_INPUT);
  
}

SystemSettingDlg::~SystemSettingDlg() {
}

bool SystemSettingDlg::Create() {
   m_pTitleBar = new TitleWidget("设置", this);
   if (NULL == m_pTitleBar) {
      ASSERT(FALSE);
      return false;
   }
   connect(m_pTitleBar, SIGNAL(closeWidget()), this, SLOT(OnClose()));
   ui.Layout_title->addWidget(m_pTitleBar, 0, Qt::AlignTop);

   m_pButtonGroup = new QButtonGroup(this);
   m_pButtonGroup->addButton(ui.pushButton_common, 0);
   m_pButtonGroup->addButton(ui.pushButton_vedioDevice, 1);
   m_pButtonGroup->addButton(ui.pushButton_audioDevice, 2);
   m_pButtonGroup->addButton(ui.pushButton_ai, 3);
   m_pButtonGroup->addButton(ui.pushButton_record, 4);
   m_pButtonGroup->addButton(ui.pushButton_voiceTranslate, 5);
   m_pButtonGroup->addButton(ui.pushButton_proxy, 6);
   m_pButtonGroup->addButton(ui.pushButton_debug, 7);
   m_pButtonGroup->addButton(ui.pushButton_question, 8);
   m_pButtonGroup->addButton(ui.pushButton_aboutUs, 9);

   for (int i = 0; i < 10; i++) {
      m_pButtonGroup->button(i)->setStyleSheet(BUTTON_UP_STYLE);
   }

   //默认打开通用设置
   ui.stackedWidget->setCurrentIndex(0);
   ui.pushButton_common->setStyleSheet(BUTTON_DOWN_STYLE);
   connect(m_pButtonGroup, SIGNAL(buttonPressed(int)), this, SLOT(OnButtonJudge(int)));

   //最大长度为20字符
   ui.lineEdit_fileName->setMaxLength(20);
   connect(ui.lineEdit_fileName, SIGNAL(textChanged(const QString &)), this, SLOT(CheckInputText(const QString &)));
   ui.lineEdit_fileName->setPlaceholderText("请输入录制文件名称");

   int checked = ConfigSetting::ReadInt(CPathManager::GetConfigPath(), GROUP_DEFAULT, DESKTOP_ENHANCE, 0);
   if (checked > 0) {
      ui.checkBox_desktopEnHance->setCheckState(Qt::CheckState::Checked);
   }
   else {
      ui.checkBox_desktopEnHance->setCheckState(Qt::CheckState::Unchecked);
   }
   //问题反馈
   ui.comboBox_questionType->addItem("请选择反馈问题的类型");
   ui.comboBox_questionType->addItem("微吼直播客户端崩溃");
   ui.comboBox_questionType->addItem("插入视频问题");
   ui.comboBox_questionType->addItem("摄像设备问题");
   ui.comboBox_questionType->addItem("音频问题");
   ui.comboBox_questionType->addItem("软件源共享失败");
   ui.comboBox_questionType->addItem("其他");

   ui.textBrowser_Content->setPlaceholderText("感谢您的建议！\n\n请描述您在使用过程中出现的问题或困扰，以便我们优化我们的产品和服务。");
   connect(ui.textBrowser_Content, SIGNAL(textChanged()), this, SLOT(OnMaxLength()));

   //去交错
   RefreshDeinterLace();
   connect(ui.tBtnAdvancedSetting, SIGNAL(clicked()), this, SLOT(OnDeviceOption()));

   //音频设置
   QPixmap micEnhancePM(":/sysButton/micEnhanceAble");
   ui.pushButton_micMute->loadPixmap(":/sysButton/micVolumeAble");
   ui.pushButton_noiseStatus->setStyleSheet("QPushButton#pushButton_noiseStatus{border-image: url(:/sysButton/img/sysButton/noiseEnable.png);}");
   ui.pushButton_speakerMute->loadPixmap(":/sysButton/speakerVolumeAble");
   connect(ui.micVolumeSlider, SIGNAL(valueChanged(int)), this, SLOT(OnMicVolumeChange(int)));
   connect(ui.speakerVolumeSlider, SIGNAL(valueChanged(int)), this, SLOT(OnSpeakerVolumeChange(int)));
   connect(ui.pushButton_micMute, SIGNAL(clicked()), this, SLOT(OnMicMuteClick()));
   connect(ui.pushButton_speakerMute, SIGNAL(clicked()), this, SLOT(OnSpeakerMuteClick()));
   //信号槽连接
   connect(ui.micListCombx, SIGNAL(currentIndexChanged(int)), this, SLOT(OnMicListChanged(int)));
   connect(ui.speakerListCombx, SIGNAL(currentIndexChanged(int)), this, SLOT(OnSpeakerListChanged(int)));
   connect(ui.checkBox_Channel, SIGNAL(stateChanged(int)), this, SLOT(OnSoundChanelListChanged(int)));
   connect(ui.comboBox_frame, SIGNAL(currentIndexChanged(int)), this, SLOT(OnFrameChg(int)));
   connect(ui.comboBox_deinterLace, SIGNAL(currentIndexChanged(int)), this, SLOT(OnDetainterChg(int)));
   connect(ui.comboBox_resolution, SIGNAL(currentIndexChanged(int)), this, SLOT(OnCommonResolutionChg(int)));
   connect(ui.comboBox_publishLines, SIGNAL(currentIndexChanged(int)), this, SLOT(OnLinesChg(int)));
   //connect(ui.widget_noiseSetting, SIGNAL((SignalNoiseValueChange(int))), this, SLOT(slot_widget_noiseSetting_SignalNoiseValueChange(int)));
   connect(ui.comboBox_vt_language, SIGNAL(currentIndexChanged(int)), this, SLOT(OnVTLanguagenChg(int)));
   connect(ui.comboBox_vt_fontSize, SIGNAL(currentIndexChanged(int)), this, SLOT(OnVTFontSizeChg(int)));
   connect(ui.pushButton_reset, SIGNAL(clicked()), this, SLOT(OnVTReset()));
   connect(ui.checkBox_desktopEnHance, SIGNAL(stateChanged(int)), this, SLOT(OnDesktopEnhanceChanged(int)));
   
   //美颜
   ui.horizontalSlider_Beauty->setMinimum(0);
   ui.horizontalSlider_Beauty->setMaximum(100);
   ui.horizontalSlider_Beauty->setValue(0);
   ui.horizontalSlider_Beauty->setEnabled(false);
   connect(ui.horizontalSlider_Beauty, SIGNAL(valueChanged(int)), this, SLOT(OnBeautyValueChange(int)));
   connect(ui.comboBox_beautyCamera, SIGNAL(currentIndexChanged(int)), this, SLOT(OnBeautyDeviceSelect(int)));

   if (m_pNoiseTips == NULL) {
      m_pNoiseTips = new NoiseTooltip();
   }
   connect(ui.pushButton_save, SIGNAL(clicked()), this, SLOT(on_confirm_clicked()));
   connect(ui.pushButton_chagne_dir, SIGNAL(clicked()), this, SLOT(on_savedDirBtn_clicked()));
   connect(ui.pushButton_openDir, SIGNAL(clicked()), this, SLOT(on_openCurDir_clicked()));

   //提示语初始隐藏
   ui.label_limitAlert->hide();

   installEventFilter(this);
   ui.pushButton_micMute->installEventFilter(this);
   ui.checkBox->installEventFilter(this);
   ui.pushButton_speakerMute->installEventFilter(this);
   ui.recordCbx->installEventFilter(this);
   ui.tBtnAdvancedSetting->installEventFilter(this);
   ui.micVolumeSlider->installEventFilter(this);
   ui.speakerVolumeSlider->installEventFilter(this);
   ui.lineEdit_fileName->installEventFilter(this);
   ui.textBrowser_Content->installEventFilter(this);
   ui.lineEdit_fileName->installEventFilter(this);
   ui.comboBox_device->installEventFilter(this);
   ui.comboBox_deviceResolution->installEventFilter(this);
   ui.comboBox_deinterLace->installEventFilter(this);
   ui.comboBox_frame->installEventFilter(this);
   ui.labNoiseValve->installEventFilter(this);
   ui.micGainSld->installEventFilter(this);
   ui.labelNoiseReductionTips->hide();
   ui.labKbps->installEventFilter(this); //音频码率
   ui.labAudioSampleRate->installEventFilter(this);//音频采样频率
   ui.labMicGain->installEventFilter(this);	//麦克风增益
   ui.labHighQualityCod->installEventFilter(this);
   ui.label_voiceTranslateTip->installEventFilter(this);
   ui.labRecord->installEventFilter(this);
   ui.label_desktop_hance->installEventFilter(this);
   ui.label_videoQuality->installEventFilter(this);

   connect(ui.checkBox_videoQuality, SIGNAL(stateChanged(int)), this, SLOT(OnVideoQuality(int)));
   connect(ui.recordCbx, SIGNAL(stateChanged(int)), this, SLOT(OnRecordCheck(int)));
   connect(ui.checkBox, SIGNAL(stateChanged(int)), this, SLOT(OnIsNoiseChecked(int)));
   connect(ui.checkBox_desktopcapture, SIGNAL(stateChanged(int)), this, SLOT(OnDesktopCaptureTypeChagned(int)));

   //修改“桌面音频”的tooltip
   ui.label_16->setToolTip(tr(DESKTOP_SPEAKER_COMMON_TOOLTIP));
   ui.speakerListCombx->setToolTip(tr(DESKTOP_SPEAKER_COMMON_TOOLTIP));
   ui.checkBox_Channel->setToolTip(tr(MIC_CHANNEL_COMMON_TOOLTIP));
   ui.speakerListCombx->setStyleSheet(TOOLTIPQSS);

   connect(ui.lineEdit_proxy_ip, SIGNAL(textChanged(const QString &)), this, SLOT(CheckApplyStatus()));
   connect(ui.lineEdit_proxy_port, SIGNAL(textChanged(const QString &)), this, SLOT(CheckApplyStatus()));
   connect(ui.lineEdit_proxy_userName, SIGNAL(textChanged(const QString &)), this, SLOT(CheckApplyStatus()));
   connect(ui.lineEdit_proxy_password, SIGNAL(textChanged(const QString &)), this, SLOT(CheckApplyStatus()));

   ui.comboBox_resolution->view()->parentWidget()->setWindowOpacity(OOPACITY);
   ui.comboBox_publishLines->view()->parentWidget()->setWindowOpacity(OOPACITY);

   bool is_support = vlive::GetWebRtcSDKInstance()->IsSupprotBeauty();
   if (!is_support) {
      ui.horizontalSlider_Beauty->setEnabled(false);
      ui.widget_24->show();
   }
   else {
      ui.widget_24->hide();
   }

   return true;
}

void SystemSettingDlg::Destroy() {
   if (NULL != m_pTitleBar) {
      delete m_pTitleBar;
      m_pTitleBar = NULL;
   }
   if (NULL != m_pNoiseTips) {
      delete  m_pNoiseTips;
      m_pNoiseTips = NULL;
   }
}
void SystemSettingDlg::Init() {
   ui.comboBox_resolution->clear();
   ui.comboBox_publishLines->clear();
}

void SystemSettingDlg::SetActivityID(char* szActivityID) {
   ui.label_activityID->setText(QString::fromStdString(szActivityID));
}

void SystemSettingDlg::SetResolution(int nIndex) {
   int nCount = ui.comboBox_resolution->count();
   if (nIndex < 0 || nIndex >= nCount) {
      ASSERT(FALSE);
      return;
   }
   disconnect(ui.comboBox_resolution, SIGNAL(currentIndexChanged(int)), this, SLOT(OnCommonResolutionChg(int)));
   ui.comboBox_resolution->setCurrentIndex(nIndex);
   connect(ui.comboBox_resolution, SIGNAL(currentIndexChanged(int)), this, SLOT(OnCommonResolutionChg(int)));
}

void SystemSettingDlg::AddResolution(QString sResolution) {
   disconnect(ui.comboBox_resolution, SIGNAL(currentIndexChanged(int)), this, SLOT(OnCommonResolutionChg(int)));
   ui.comboBox_resolution->addItem(sResolution);
   connect(ui.comboBox_resolution, SIGNAL(currentIndexChanged(int)), this, SLOT(OnCommonResolutionChg(int)));
}

void SystemSettingDlg::AddPubLines(QString qsPubLine) {
   disconnect(ui.comboBox_publishLines, SIGNAL(currentIndexChanged(int)), this, SLOT(OnLinesChg(int)));
   ui.comboBox_publishLines->addItem(qsPubLine);
   connect(ui.comboBox_publishLines, SIGNAL(currentIndexChanged(int)), this, SLOT(OnLinesChg(int)));
}

void SystemSettingDlg::SetCurLine(int iLine) {
   int nCount = ui.comboBox_publishLines->count();
   if (iLine < 0 || iLine >= nCount) {
      ASSERT(FALSE);
      return;
   }
   disconnect(ui.comboBox_publishLines, SIGNAL(currentIndexChanged(int)), this, SLOT(OnLinesChg(int)));
   ui.comboBox_publishLines->setCurrentIndex(iLine);
   connect(ui.comboBox_publishLines, SIGNAL(currentIndexChanged(int)), this, SLOT(OnLinesChg(int)));
}

int SystemSettingDlg::GetCurPublishIndex() {
   int index = ui.comboBox_publishLines->currentIndex();
   return index;
}

void SystemSettingDlg::DealLiveStatus(int liveStatus) {
   if (liveStatus == 1) {
      ui.checkBox_videoQuality->setEnabled(false);
   }
   else {
      ui.checkBox_videoQuality->setEnabled(true);
   }
}

void SystemSettingDlg::SetSavePath(QString qsSavePath) {
   ui.lineEdit_savePath->setText(qsSavePath);
   CheckApplyStatus();
}
QString SystemSettingDlg::GetCurrentSavePath() {
   return ui.lineEdit_savePath->text();
}

void SystemSettingDlg::on_savedDirBtn_clicked() {
   QString defaultChoosePath = GetCurrentSavePath();
   if (defaultChoosePath.isEmpty() || defaultChoosePath.isNull() || defaultChoosePath.length() == 0) {
      defaultChoosePath = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation).first();
   }

   m_qsSavedDir = QFileDialog::getExistingDirectory(this, QObject::tr("选择目录"), defaultChoosePath, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
   if (!m_qsSavedDir.isNull()) {
      SetSavePath((char*)m_qsSavedDir.toStdString().c_str());
   }
}

void SystemSettingDlg::on_openCurDir_clicked() {
   QString url = "file:///" + m_qsSavedDir;
   QDesktopServices::openUrl(QUrl(url, QUrl::TolerantMode));
}

void SystemSettingDlg::OnClose() {
   VH::CComPtr<IOBSControlLogic> pOBSControlLogic;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IOBSControl, IID_IOBSControlLogic, pOBSControlLogic, return);
   if (pOBSControlLogic) {
      pOBSControlLogic->SetSaveMicAudio(false);
   }

   SaveNoiseSet();
   EnableNoiceUI(false);
   reject();
}
int SystemSettingDlg::PageTypeToId(e_page_type eType) {
   int nIndex = 0;
   switch (eType) {
   case e_page_common:
      nIndex = 0;
      break;
   case e_page_vedio:
      nIndex = 1;
      break;
   case e_page_audio:
      nIndex = 2;
      break;
   case e_page_ai:
      nIndex = 3;
      break;
   case e_page_record:
      nIndex = 4;
      break;
   case e_page_voicetranslate:
      nIndex = 5;
      break;
   case e_page_proxy:
      nIndex = 6;
      break;
   case e_page_debug:
      nIndex = 7;
      break;
   case e_page_question:
      nIndex = 8;
      break;
   case e_page_aboutUs:
      nIndex = 9;
      break;
   default:
      break;
   }
   return nIndex;
}

e_page_type SystemSettingDlg::PageIdToType(int index) {
   return (e_page_type)(0x01 << index);
}

void SystemSettingDlg::show() {
   VH::CComPtr<IMainUILogic> pMainUILogic;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IMainUI, IID_IMainUILogic, pMainUILogic, return, ASSERT(FALSE));
   if (pMainUILogic && !pMainUILogic->GetVoiceTranslate()) {
      ui.pushButton_voiceTranslate->hide();
   }
   else if (pMainUILogic && pMainUILogic->GetVoiceTranslate()) {
      ui.pushButton_voiceTranslate->show();
   }
   VHDialog::show();
}

void SystemSettingDlg::OnButtonJudge(int nButtonId) {
   //调试信息/问题反馈/关于我们 隐藏应用按钮
   if ((e_page_type)PageIdToType(nButtonId) == e_page_vedio) {
      VH::CComPtr<IMainUILogic> pMainUILogic;
      DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IMainUI, IID_IMainUILogic, pMainUILogic, return, ASSERT(FALSE));
      if (!pMainUILogic->IsLoadUrlFinished()) {
         TRACE6("%s loading url ,can not showDeviceList\n", __FUNCTION__);
         return;
      }
   }

   ChangePage((e_page_type)PageIdToType(nButtonId));
   ui.comboBox_resolution->view()->parentWidget()->setWindowOpacity(OOPACITY);
   ui.comboBox_publishLines->view()->parentWidget()->setWindowOpacity(OOPACITY);
   ui.comboBox_device->view()->parentWidget()->setWindowOpacity(OOPACITY);
   ui.comboBox_deviceResolution->view()->parentWidget()->setWindowOpacity(OOPACITY);
   ui.comboBox_frame->view()->parentWidget()->setWindowOpacity(OOPACITY);
   ui.comboBox_deinterLace->view()->parentWidget()->setWindowOpacity(OOPACITY);
   ui.micListCombx->view()->parentWidget()->setWindowOpacity(OOPACITY);
   ui.speakerListCombx->view()->parentWidget()->setWindowOpacity(OOPACITY);
   ui.comboBox_questionType->view()->parentWidget()->setWindowOpacity(OOPACITY);
   ui.cmbKbps->view()->parentWidget()->setWindowOpacity(OOPACITY);
   ui.cmbAudioSampleRate->view()->parentWidget()->setWindowOpacity(OOPACITY);
   ui.cmbHighQualityCod->view()->parentWidget()->setWindowOpacity(OOPACITY);
}

void SystemSettingDlg::UpdateDebugInfo(QString sServerIP, QString sBandWidth, QString sCurLostFrame, QString sAllLostFrame) {
   ui.label_serverIP->setText(sServerIP);
   ui.label_bandWidth->setText(sBandWidth);
   ui.label_curLostFrame->setText(sCurLostFrame);
   ui.label_allLostFrame->setText(sAllLostFrame);
}

void SystemSettingDlg::ApplySettings() {
   int dpi_enable = ui.checkBox_dpi->checkState() == Qt::CheckState::Checked ? 1 : 0;
   ConfigSetting::writeValue(CPathManager::GetConfigPath(), GROUP_DEFAULT, DESKTOP_DIP, dpi_enable);
   if (!m_ApplyIdentify) {
      return;
   }
   repaint();

   int applyIdentify = m_ApplyIdentify;
#define APPLYSTATUSCHECK(X) if (m_ApplyIdentify&X){m_ApplyInitialStatus[X]=ApplyPageStatus(X);}
   APPLYSTATUSCHECK(e_page_common);
   APPLYSTATUSCHECK(e_page_vedio);
   APPLYSTATUSCHECK(e_page_audio);
   APPLYSTATUSCHECK(e_page_ai);
   APPLYSTATUSCHECK(e_page_record);
   APPLYSTATUSCHECK(e_page_proxy);
   APPLYSTATUSCHECK(e_page_voicetranslate)
   CheckApplyStatus();
   TRACE6("%s pSettingLogic ApplySettings", __FUNCTION__);
   VH::CComPtr<ISettingLogic> pSettingLogic;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IMainUI, IID_ISettingLogic, pSettingLogic, return, ASSERT(FALSE));
   pSettingLogic->ApplySettings(applyIdentify);
   TRACE6("%s pSettingLogic ApplySettings end", __FUNCTION__);
   if (ui.speakerListCombx->count() > 0) {
      QString curDevId = ui.speakerListCombx->currentData().toString();
      if (mPlayerDevId != curDevId) {
         mPlayerDevId = curDevId;
         ConfigSetting::writeValue(CPathManager::GetConfigPath(), GROUP_DEFAULT, PLAYER_DEFAULT_DEVICE, mPlayerDevId);
         VH::CComPtr<IOBSControlLogic> pObsControlLogic = NULL;
         DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IOBSControl, IID_IOBSControlLogic, pObsControlLogic, return, ASSERT(FALSE));
         pObsControlLogic->InitPlayDevice(mPlayerDevId.toStdWString().c_str());
      }
   }
   TRACE6("%s ApplySettings end", __FUNCTION__);
}

void SystemSettingDlg::on_confirm_clicked() {
   if (ui.textBrowser_Content->toPlainText().length() > 0) {
      m_ApplyIdentify |= e_page_question;
   }
   ApplySettings();
   OnClose();
}

void SystemSettingDlg::on_apply_clicked() {
   ApplySettings();
   m_ApplyIdentify &= 0;
}

void SystemSettingDlg::CheckInputText(const QString &text) {
   //存在限制字符
   ModifyLimitText(text);
   if (text.count() >= MAXINPUTSIZE) {
      ui.label_limitAlert->setText("录制文件名最长输入20个字符");
      ui.label_limitAlert->show();
   }
   else {
      ui.label_limitAlert->setText("");
      ui.label_limitAlert->hide();
   }
   CheckApplyStatus();
}

bool SystemSettingDlg::IsRecord() {
   bool isRecord = ui.recordCbx->isChecked();
   QJsonObject obj = GetPageData(e_page_record);
   if (!obj.empty()) {
      isRecord = obj[PAGE_RECORD_ISRECORD].toBool();
   }
   return isRecord;
}

void SystemSettingDlg::GetQuestionInfo(int& nTpye, QString& sContent) {
   nTpye = ui.comboBox_questionType->currentIndex();
   sContent = ui.textBrowser_Content->toPlainText();
}

void SystemSettingDlg::AddCameraItem(DeviceInfo deviceInfo) {
   ui.comboBox_deviceResolution->setEnabled(true);
   RefreshDeinterLace();
   if (!ui.comboBox_device->isEnabled()) {
      ui.comboBox_device->setEnabled(true);
      disconnect(ui.comboBox_device, SIGNAL(currentIndexChanged(int)), this, SLOT(OnChgDevice(int)));
      ui.comboBox_device->clear();
      connect(ui.comboBox_device, SIGNAL(currentIndexChanged(int)), this, SLOT(OnChgDevice(int)));
   }

   disconnect(ui.comboBox_device, SIGNAL(currentIndexChanged(int)), this, SLOT(OnChgDevice(int)));
   QVariant qItemData;
   QByteArray ba;
   ba.setRawData((const char *)&deviceInfo, sizeof(DeviceInfo));
   QString str = QString::fromLocal8Bit(ba.toBase64());
   qItemData.setValue(str);
   ui.comboBox_device->addItem(QString::fromWCharArray(deviceInfo.m_sDeviceDisPlayName), qItemData);
   connect(ui.comboBox_device, SIGNAL(currentIndexChanged(int)), this, SLOT(OnChgDevice(int)));

   ui.label_deviceNum->setText(QString::fromStdWString(L"摄像设备 ")
      + QString::number(1)
   );
}

void SystemSettingDlg::AddFrame(QString sFrame) {
   disconnect(ui.comboBox_frame, SIGNAL(currentIndexChanged(int)), this, SLOT(OnFrameChg(int)));
   ui.comboBox_frame->addItem(sFrame);
   connect(ui.comboBox_frame, SIGNAL(currentIndexChanged(int)), this, SLOT(OnFrameChg(int)));
}

bool SystemSettingDlg::GetForceMono() {
   bool isForceMono = ui.checkBox_Channel->isChecked();
   return isForceMono;
}

//是否降噪
bool SystemSettingDlg::GetIsNoise() {
   bool isNoise = ui.checkBox->isChecked();
   return isNoise;
}

//当前麦克风
DeviceInfo SystemSettingDlg::GetMicDeviceInfo() {
   DeviceInfo deviceInfo;
   int index = ui.micListCombx->currentIndex();
   QJsonObject obj = GetPageData(e_page_audio);
   if (!obj.empty()) {
      index = obj[PAGE_AUDIO_MIC].toInt();
   }
   GetMicItem(deviceInfo, index);
   return deviceInfo;
}

void SystemSettingDlg::SetCurDeinterLacing(DeinterlacingType type) {
   ui.comboBox_deinterLace->setCurrentIndex((int)type);
}

//设置分辨率
void SystemSettingDlg::SetResolutionList(FrameInfoList &frameInfoList) {
   disconnect(ui.comboBox_deviceResolution, SIGNAL(currentIndexChanged(int)), this, SLOT(OnResolutionIndexChg(int)));
   m_resolutionList.Clear();
   ui.comboBox_deviceResolution->clear();
   ui.comboBox_frame->clear();
   m_frameInternalList.clear();

   for (auto itor = frameInfoList.infos.begin(); itor != frameInfoList.infos.end(); itor++) {
      m_resolutionList.Append(*itor);
   }
   m_resolutionList.sort();
   for (int i = 0; i < m_resolutionList.Count(); i++) {
      ui.comboBox_deviceResolution->addItem(m_resolutionList.DisplayString(i), QVariant(i));
   }
   connect(ui.comboBox_deviceResolution, SIGNAL(currentIndexChanged(int)), this, SLOT(OnResolutionIndexChg(int)));
}

void SystemSettingDlg::SetCurResolution(FrameInfo &frameInfo) {
   int index = 0;
   index = m_resolutionList.SetCurrentFrameInfo(frameInfo);

   if (ui.comboBox_deviceResolution->currentIndex() == index) {
      OnResolutionIndexChg(index);
   }
   else {
      ui.comboBox_deviceResolution->setCurrentIndex(index);
   }

   for (int i = 0; i < m_frameInternalList.count(); i++) {
      if (frameInfo.maxFrameInterval == m_frameInternalList[i]) {
         if (i < ui.comboBox_frame->count()) {
            disconnect(ui.comboBox_frame, SIGNAL(currentIndexChanged(int)), this, SLOT(OnFrameChg(int)));
            ui.comboBox_frame->setCurrentIndex(i);
            connect(ui.comboBox_frame, SIGNAL(currentIndexChanged(int)), this, SLOT(OnFrameChg(int)));
         }
         break;
      }
   }
}

void SystemSettingDlg::OnResolutionIndexChg(int nIndex) {
   if (nIndex < 0) {
      return;
   }
   CameraSettingUIResolution res;
   if (!m_resolutionList.GetFrameInfo(nIndex, &res)) {
      return;
   }

   ui.comboBox_frame->clear();
   m_frameInternalList.clear();
   QList<UINT64> frameList;
   for (int i = 0; i < res.frameInternalList.count(); i++) {
      frameList.append(res.frameInternalList[i].minInternal);
      frameList.append(res.frameInternalList[i].maxInternal);
   }

   for (int i = 0; i < frameList.count(); i++) {
      for (int j = i + 1; j < frameList.count(); j++) {
         frameList.swap(i, j);
      }
   }

   for (int i = 0; i < frameList.count(); i++) {
      UINT64 frameInternal = frameList[i];
      bool isFind = false;
      for (int j = 0; j < m_frameInternalList.count(); j++) {
         if (m_frameInternalList[j] == frameInternal) {
            isFind = true;
            break;
         }
      }
      if (!isFind) {
         m_frameInternalList.append(frameInternal);
      }
   }

   for (int i = 0; i < m_frameInternalList.count(); i++) {
      for (int j = i + 1; j < m_frameInternalList.count(); j++) {
         if (m_frameInternalList[i] > m_frameInternalList[j]) {
            m_frameInternalList.swap(i, j);
         }
      }
   }

   double maxFrameNum = 0;
   double minFrameNum = 0;

   for (int i = 0; i < m_frameInternalList.count(); i++) {
      double frameInternal = 10000000.0f;
      frameInternal /= m_frameInternalList[i];
      disconnect(ui.comboBox_frame, SIGNAL(currentIndexChanged(int)), this, SLOT(OnFrameChg(int)));
      ui.comboBox_frame->addItem(QString::number(frameInternal, 'g', 4), QVariant(i));
      connect(ui.comboBox_frame, SIGNAL(currentIndexChanged(int)), this, SLOT(OnFrameChg(int)));
      if (i == 0) {
         maxFrameNum = frameInternal;
      }
      if (i == m_frameInternalList.count() - 1) {
         minFrameNum = frameInternal;
      }
   }

   if (m_frameInternalList.count() != 0) {
      ui.comboBox_frame->setValidator(new QIntValidator(minFrameNum + 0.5, maxFrameNum + 0.5, this));
   }

   m_resolutionList.SetCurrentIndex(nIndex);
   CheckApplyStatus();

   VH::CComPtr<ICommonData> pCommonData;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
   pCommonData->ReportEvent("sz02sxsb02");
}

void SystemSettingDlg::OnDeviceOption() {
   DeviceInfo deviceInfo;
   if (!GetCameraDeviceInfo(ui.comboBox_device->currentIndex(), deviceInfo)) {
      return;
   }

   VH::CComPtr<IDeviceManager> pDeviceManager;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IOBSControl, IID_IDeviceManager, pDeviceManager, return, ASSERT(FALSE));

   pDeviceManager->OpenPropertyPages((HWND)winId(), deviceInfo);

   VH::CComPtr<ICommonData> pCommonData;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
   pCommonData->ReportEvent("sz02sxsb05");
}

void SystemSettingDlg::SetMicVolume(float fVolume) {
   bool bMute = fVolume == 0.0f;
   disconnect(ui.micVolumeSlider, SIGNAL(valueChanged(int)), this, SLOT(OnMicVolumeChange(int)));
   ui.pushButton_micMute->loadPixmap(bMute ? ":/sysButton/micVolumeUnable" : ":/sysButton/micVolumeAble");
   ui.micVolumeSlider->setValue(100 * fVolume);
   connect(ui.micVolumeSlider, SIGNAL(valueChanged(int)), this, SLOT(OnMicVolumeChange(int)));
}

void SystemSettingDlg::SetSpeakerVolume(float fVolume) {
   bool bMute = fVolume == 0.0f;
   disconnect(ui.speakerVolumeSlider, SIGNAL(valueChanged(int)), this, SLOT(OnSpeakerVolumeChange(int)));
   ui.pushButton_speakerMute->loadPixmap(bMute ? ":/sysButton/speakerVolumeUnable" : ":/sysButton/speakerVolumeAble");
   ui.speakerVolumeSlider->setValue(100 * fVolume);
   connect(ui.speakerVolumeSlider, SIGNAL(valueChanged(int)), this, SLOT(OnSpeakerVolumeChange(int)));
}

void SystemSettingDlg::OnMicVolumeChange(int nValue) {
   if (ui.micVolumeSlider->hasFocus()) {
      QToolTip::showText(QCursor::pos(), QString("%1").arg(nValue));
   }

   ui.pushButton_micMute->loadPixmap(nValue == 0 ? ":/sysButton/micVolumeUnable" : ":/sysButton/micVolumeAble");
   ui.pushButton_micMute->update();

   STRU_MAINUI_VOLUME_CHANGE volumeChange;
   volumeChange.m_eType = change_Mic;
   volumeChange.m_nVolume = (float)nValue / 100;
   SingletonMainUIIns::Instance().PostCRMessage(MSG_MAINUI_VOLUME_CHANGE, &volumeChange, sizeof(volumeChange));
}

void SystemSettingDlg::OnSpeakerVolumeChange(int nValue) {
   if (ui.speakerVolumeSlider->hasFocus()) {
      QToolTip::showText(QCursor::pos(), QString("%1").arg(nValue));
   }

   ui.pushButton_speakerMute->loadPixmap(nValue == 0 ? ":/sysButton/speakerVolumeUnable" : ":/sysButton/speakerVolumeAble");
   STRU_MAINUI_VOLUME_CHANGE volumeChange;
   volumeChange.m_eType = change_Speaker;
   volumeChange.m_nVolume = (float)nValue / 100;
   SingletonMainUIIns::Instance().PostCRMessage(MSG_MAINUI_VOLUME_CHANGE, &volumeChange, sizeof(volumeChange));
}

void SystemSettingDlg::on_micGainSld_valueChanged(int value){
   if (ui.micGainSld->hasFocus()) {
      QToolTip::showText(QCursor::pos(), QString("%1").arg(1.0*value / 100));
   }
   m_ApplyIdentify |= e_page_audio;
}

void SystemSettingDlg::OnMicMuteClick() {
   bool m_bMicMute = ui.micVolumeSlider->value() != 0;
   ui.pushButton_micMute->loadPixmap(m_bMicMute ? ":/sysButton/micVolumeUnable" : ":/sysButton/micVolumeAble");

   STRU_MAINUI_MUTE loMute;
   loMute.m_eType = mute_Mic;
   loMute.m_bMute = m_bMicMute;
   SingletonMainUIIns::Instance().PostCRMessage(MSG_MAINUI_MUTE, &loMute, sizeof(loMute));

   VH::CComPtr<ICommonData> pCommonData;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
   pCommonData->ReportEvent("mkf01");
}

void SystemSettingDlg::OnSpeakerMuteClick() {
   bool m_bSpeakerMute = ui.speakerVolumeSlider->value() != 0;
   ui.pushButton_speakerMute->loadPixmap(m_bSpeakerMute ? ":/sysButton/speakerVolumeUnable" : ":/sysButton/speakerVolumeAble");

   STRU_MAINUI_MUTE loMute;
   loMute.m_eType = mute_Speaker;
   loMute.m_bMute = m_bSpeakerMute;
   SingletonMainUIIns::Instance().PostCRMessage(MSG_MAINUI_MUTE, &loMute, sizeof(loMute));

   VH::CComPtr<ICommonData> pCommonData;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
   pCommonData->ReportEvent("ysq01");
}

void SystemSettingDlg::AddMicItem(wchar_t* wzDeviceName, DeviceInfo itemData) {
   disconnect(ui.micListCombx, SIGNAL(currentIndexChanged(int)), this, SLOT(OnMicListChanged(int)));
   QVariant qItemData;
   QByteArray ba;
   ba.setRawData((const char *)&itemData, sizeof(DeviceInfo));
   QString str = QString::fromLocal8Bit(ba.toBase64());
   qItemData.setValue(str);
   ui.micListCombx->addItem(QString::fromWCharArray(wzDeviceName), qItemData);
   connect(ui.micListCombx, SIGNAL(currentIndexChanged(int)), this, SLOT(OnMicListChanged(int)));
}

void SystemSettingDlg::SetCurMicItem(DeviceInfo& itemData) {
   int index = 0;
   if (ui.micListCombx->count() == 0) {
      return;
   }

   for (; index < ui.micListCombx->count(); index++) {
      QVariant auserData = ui.micListCombx->itemData(index);
      QString str = auserData.value<QString>();
      QByteArray ba = QByteArray::fromBase64(str.toLocal8Bit());
      if (ba.size() == sizeof(DeviceInfo)) {
         DeviceInfo deviceInfo = *(DeviceInfo *)ba.data();
         if (itemData == deviceInfo) {
            ui.micListCombx->setCurrentIndex(index);
            break;
         }
      }
   }
}

bool SystemSettingDlg::GetMicItem(DeviceInfo& itemData, int index) {
   if (ui.micListCombx->count() == 0) {
      return false;
   }
   if (index >= ui.micListCombx->count()) {
      index = 0;
   }
   QVariant auserData = ui.micListCombx->itemData(index);
   QString str = auserData.value<QString>();
   QByteArray ba = QByteArray::fromBase64(str.toLocal8Bit());
   if (ba.size() != sizeof(DeviceInfo)) {
      return false;
   }

   itemData = *(DeviceInfo *)ba.data();
   return true;
}

bool SystemSettingDlg::GetCameraDeviceInfo(int index, DeviceInfo &deviceInfo) {
   if (index < 0 || index >= ui.comboBox_device->count()) {
      return false;
   }

   QVariant auserData = ui.comboBox_device->itemData(index);
   QString str = auserData.value<QString>();
   QByteArray ba = QByteArray::fromBase64(str.toLocal8Bit());
   if (ba.size() != sizeof(DeviceInfo)) {
      return false;
   }

   deviceInfo = *(DeviceInfo *)ba.data();
   return true;
}

bool SystemSettingDlg::GetCurMicItem(DeviceInfo& itemData) {
   QVariant auserData = ui.micListCombx->itemData(ui.micListCombx->currentIndex());
   QString str = auserData.value<QString>();
   QByteArray ba = QByteArray::fromBase64(str.toLocal8Bit());
   if (ba.size() != sizeof(DeviceInfo)) {
      return false;
   }

   itemData = *(DeviceInfo *)ba.data();
   return true;
}

void SystemSettingDlg::ClearMicItem() {
   ui.micListCombx->clear();
}

void SystemSettingDlg::AddSpeakerItem(wchar_t* wzDeviceName, wchar_t* wzDeviceID) {
   disconnect(ui.speakerListCombx, SIGNAL(currentIndexChanged(int)), this, SLOT(OnSpeakerListChanged(int)));
   QVariant deviceID;
   deviceID.setValue(QString::fromWCharArray(wzDeviceID));
   ui.speakerListCombx->addItem(QString::fromWCharArray(wzDeviceName), deviceID);
   connect(ui.speakerListCombx, SIGNAL(currentIndexChanged(int)), this, SLOT(OnSpeakerListChanged(int)));
   if (mPlayerDevId.isEmpty()) {
      if (wzDeviceID == nullptr) {
         mPlayerDevId = "";
      }
      else {
         mPlayerDevId = QString::fromStdWString(wzDeviceID);
      }
      ConfigSetting::writeValue(CPathManager::GetConfigPath(), GROUP_DEFAULT, PLAYER_DEFAULT_DEVICE, mPlayerDevId);
   }
   if (wzDeviceID != nullptr && QString::fromStdWString(wzDeviceID) == mPlayerDevId) {
      int count = ui.speakerListCombx->count();
      ui.speakerListCombx->setCurrentIndex(count - 1);
   }
}

bool SystemSettingDlg::GetCurSpeakerItem(wchar_t* wzDeviceID) {
   QVariant auserData = ui.speakerListCombx->currentData();
   QString qstr = auserData.toString();
   wcscpy(wzDeviceID, qstr.toStdWString().c_str());
   return true;
}

void SystemSettingDlg::ClearSpeakerItem() {
   ui.speakerListCombx->clear();
}

void SystemSettingDlg::InitEnhanceParam(int nPageStep, int nSingleStep, int nMin, int nMax, int nValue, int stepValue) {
   mStepValue = stepValue;
}

void SystemSettingDlg::OnMicListChanged(int index) {
   if (index < 0) {
      return;
      ASSERT(FALSE);
   }

   VH::CComPtr<IDeviceManager> pDeviceManager;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IOBSControl, IID_IDeviceManager, pDeviceManager, return, ASSERT(FALSE));

   DeviceInfo deviceInfo;
   GetCurMicItem(deviceInfo);
   if (deviceInfo.m_sDeviceType == TYPE_COREAUDIO) {
      float fEnhanceLevel = 0;       //当前值
      float fMin = 0;        //最小值
      float fMax = 0;       //最大值
      float fStep = 0;       //步进

      QPixmap micEnhancePM(":/sysButton/micEnhanceAble");
      pDeviceManager->GetEnhanceLevel(deviceInfo.m_sDeviceID, &fEnhanceLevel, &fMin, &fMax, &fStep);
      if (fStep == 0.0f) {
      }
      else {
         InitEnhanceParam(1, 1, fMin / fStep, fMax / fStep, fEnhanceLevel / fStep, fStep);
      }
   }
   CheckApplyStatus();
}

void SystemSettingDlg::ChangePage(e_page_type eType) {
   int nIndex = PageTypeToId(eType);
   //按下按钮高亮，其他按钮保持抬起状态
   if (m_pButtonGroup) {
      m_pButtonGroup->button(nIndex)->setStyleSheet(BUTTON_DOWN_STYLE);
      for (int i = 0; i < 10; i++) {
         if (nIndex == i) {
            continue;
         }
         m_pButtonGroup->button(i)->setStyleSheet(BUTTON_UP_STYLE);
      }
   }

   //切换页面
   int count = ui.stackedWidget->count();
   if (nIndex < 0 || nIndex >= count) {
      ASSERT(FALSE);
      return;
   }

   ui.stackedWidget->setCurrentIndex(nIndex);
   if (m_ApplyInitialStatus.find(eType) == m_ApplyInitialStatus.end()) {
      m_ApplyInitialStatus[eType] = InitPageStatus(eType);
   }

   CheckApplyStatus();
}

QJsonObject SystemSettingDlg::GetPageData(e_page_type type) {
   QJsonObject obj = QJsonObject();
   auto itor = m_ApplyInitialStatus.find(type);
   if (itor != m_ApplyInitialStatus.end()) {
      obj = itor.value();
   }
   return obj;
}

QJsonObject SystemSettingDlg::ApplyPageStatus(e_page_type type) {
   QString qsConfPath = CPathManager::GetConfigPath();
   QJsonObject obj = QJsonObject();
   switch (type) {
   case e_page_common:
   {
      int defaultResolution = ui.comboBox_resolution->currentIndex();
      int defaultLine = ui.comboBox_publishLines->currentIndex();
      int iHighQualityCod = ui.cmbHighQualityCod->currentText().toInt();
      int videoQuality = ui.checkBox_videoQuality->checkState();
      int openDesktopCaptureType = ui.checkBox_desktopcapture->checkState();

      obj[PAGE_COMMON_RESOLUTION] = defaultResolution;
      obj[PAGE_COMMON_LINE] = defaultLine;
      obj[PAGE_VIDEO_HIGHQUALITYCOD] = iHighQualityCod;
      obj[PAGE_VIDEO_QUALITY] = videoQuality;
      obj[PAGE_COMMON_DESKTOP_CAPTURE_TYPE] = openDesktopCaptureType;

      ConfigSetting::writeValue(qsConfPath, GROUP_DEFAULT, KEY_PUBLISH_QUALITY, defaultResolution);
      ConfigSetting::writeValue(qsConfPath, GROUP_DEFAULT, KEY_VIDEO_LINE, defaultLine);
      ConfigSetting::writeValue(qsConfPath, GROUP_DEFAULT, KEY_VCODEC_HIGH_PROFILE_OPEN, iHighQualityCod);
      ConfigSetting::writeValue(qsConfPath, GROUP_DEFAULT, KEY_VCODEC_VIDEO_QUALITY, videoQuality);
      ConfigSetting::writeValue(qsConfPath, GROUP_DEFAULT, KEY_DESKCTOP_CAPTURE_TYPE, openDesktopCaptureType); 
      break;
   }
   case e_page_vedio:{
      int device = ui.comboBox_device->currentIndex();
      int resolution = ui.comboBox_deviceResolution->currentIndex();
      int fps = ui.comboBox_frame->currentIndex();
      int deinterlace = ui.comboBox_deinterLace->currentIndex();

      obj[PAGE_VIDEO_DEVICE] = device;
      obj[PAGE_VIDEO_RESOLUTION] = resolution;
      obj[PAGE_VIDEO_FPS] = fps;
      obj[PAGE_VIDEO_DEINTERLACE] = deinterlace;

      DeviceInfo currentDeviceInfo;
      if (GetCameraDeviceInfo(device, currentDeviceInfo)) {
         int width = 1280;
         int height = 720;
         //帧率
         UINT64 ui64FrameNum = 333333;
         VideoFormat format = m_resolutionList.GetCurrentVideoFormat();

         if (!m_resolutionList.GetCurrentResolution(width, height)) {
            break;
         }

         ui64FrameNum = m_resolutionList.GetCurrentFrameInternal(ui.comboBox_frame->currentText());
         //去交错
         int nIndex = ui.comboBox_deinterLace->currentIndex();
         TRACE6("[BP] SystemSettingDlg::ApplyPageStatus e_page_vedio [%s][%d][%d][%u][%d][%d]\n",
            QString::fromWCharArray(currentDeviceInfo.m_sDeviceDisPlayName).toLocal8Bit().data(),
            width, height,
            ui64FrameNum,
            nIndex,
            format
         );
         //设置分辨率、帧率、去交错
         SetDeviceDefaultAttribute(currentDeviceInfo,
            width,
            height,
            ui64FrameNum,
            (DeinterlacingType)nIndex, format);
      }
      break;
   }
   case e_page_audio: {
      bool isNoise = ui.checkBox->isChecked();
      bool isForceMono = !ui.checkBox_Channel->isChecked();
      int noiseValue = ui.widget_noiseSetting->GetNoiseValue();
      int iKbps = ui.cmbKbps->currentText().toInt();
      int iAudioSampleRate = ui.cmbAudioSampleRate->currentText().toInt();
      bool bNoiseReduction = ui.ckbNoiseReduction->isChecked();
      int iMicGain = ui.micGainSld->value();

      ConfigSetting::writeValue(qsConfPath, GROUP_DEFAULT, KEY_AUDIO_NOISE_VALUE, noiseValue);
      ConfigSetting::writeValue(qsConfPath, GROUP_DEFAULT, KEY_AUDIO_ISNOISE, isNoise ? 1 : 0);
      ConfigSetting::writeValue(qsConfPath, GROUP_DEFAULT, KEY_AUDIO_FORCEMONO, isForceMono ? 1 : 0);
      ConfigSetting::writeValue(qsConfPath, GROUP_DEFAULT, KEY_AUDIO_KBPS, iKbps);
      ConfigSetting::writeValue(qsConfPath, GROUP_DEFAULT, KEY_AUDIO_SAMPLERATE, 44100);
      ConfigSetting::writeValue(qsConfPath, GROUP_DEFAULT, KEY_AUDIO_NOISEREDUCTION, bNoiseReduction ? 1 : 0);
      ConfigSetting::writeValue(qsConfPath, GROUP_DEFAULT, KEY_AUDIO_MICGAIN, iMicGain);

      ui.checkBox->setChecked(isNoise);
      if (ui.micListCombx->count() > 0) {
         int micIndex = ui.micListCombx->currentIndex();
         obj[PAGE_AUDIO_MIC] = micIndex;
         DeviceInfo itemData;
         if (GetMicItem(itemData, micIndex)) {
            //wstring audioConfPath = GetAppPath() + CONFIGPATH_DEVICE;
            QString audioConf = CPathManager::GetAudiodevicePath();
            QByteArray ba;
            ba.setRawData((const char *)&itemData, sizeof(DeviceInfo));
            QString str = QString::fromLocal8Bit(ba.toBase64());
            ConfigSetting::writeValue(audioConf, GROUP_DEFAULT, AUDIO_DEFAULT_DEVICE, str);
         }
      }

      obj[PAGE_AUDIO_ISNOISE] = isNoise;
      obj[PAGE_AUDIO_FORCEMONO] = isForceMono;
      obj[PAGE_AUDIO_KBPS] = iKbps;
      obj[PAGE_AUDIO_SAMPLERATE] = iAudioSampleRate;
      obj[PAGE_AUDIO_NOISEREDUCTION] = bNoiseReduction;
      obj[PAGE_AUDIO_MICGAIN] = iMicGain;

      if (ui.speakerListCombx->count() > 0) {
         obj[PAGE_AUDIO_SPEAKER] = ui.speakerListCombx->currentIndex();
      }
      break;
   }
   case e_page_ai: {
      obj[PAGE_DESKTOP_ENHANCE] = ui.checkBox_desktopEnHance->checkState();
      break;
   }
   case e_page_record: {
      bool bRecord = ui.recordCbx->isChecked();
      QString filePath = ui.lineEdit_savePath->text();
      QString fileName = ui.lineEdit_fileName->text().trimmed();

      obj[PAGE_RECORD_ISRECORD] = bRecord;
      obj[PAGE_RECORD_FILEPATH] = filePath;
      obj[PAGE_RECORD_FILENAME] = fileName;
      ConfigSetting::writeValue(qsConfPath, GROUP_DEFAULT, KEY_RECORD_MODE, bRecord ? 1 : 0);
      ConfigSetting::writeValue(qsConfPath, GROUP_DEFAULT, KEY_RECORD_PATH, filePath);
      ConfigSetting::writeValue(qsConfPath, GROUP_DEFAULT, KEY_RECORD_FILENAME, fileName);
      break;
   }
   case e_page_voicetranslate:{
      int font = ui.comboBox_vt_fontSize->currentText().toInt();
      QString lan = ui.comboBox_vt_language->currentText();
      int selectLanguageIndex = ui.comboBox_vt_language->currentIndex();   //在UI文件中，普通话对应索引：0  /  粤语对应索引：1  / 四川话对应索引：2

      obj[PAGE_VT_FONT] = font;
      obj[PAGE_VT_LAN] = lan;

      ConfigSetting::writeValue(qsConfPath, GROUP_DEFAULT, KEY_VT_FONT_SIZE, font);
      ConfigSetting::writeValue(qsConfPath, GROUP_DEFAULT, KEY_VT_LANGUAGE, lan);

      VH::CComPtr<IMainUILogic> pMainUILogic;
      DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IMainUI, IID_IMainUILogic, pMainUILogic, return obj, ASSERT(FALSE));
      if (pMainUILogic && pMainUILogic->GetVoiceTranslate() && pMainUILogic->IsOpenVoiceTranslateFunc()) {
         STRU_VT_INFO info;
         info.lan = selectLanguageIndex;
         info.fontSize = font;
         info.bEnable = true;
         SingletonMainUIIns::Instance().PostCRMessage(MSG_OBSCONTROL_ENABLE_VT, &info, sizeof(STRU_VT_INFO));
         pMainUILogic->SetVoiceTranslateFontSize(font);
      }
      break;
   }
   case e_page_proxy: {
      bool currentBProxyOpen = ui.comboBox_proxyEnable->currentIndex() != 0;
      QString currentHost = ui.lineEdit_proxy_ip->text();
      int currentPort = ui.lineEdit_proxy_port->text().toInt();
      QString currentUsr = ui.lineEdit_proxy_userName->text();
      QString currentPwd = ui.lineEdit_proxy_password->text();
      int proxyType = ui.comboBox_proxyEnable->currentIndex();
      obj[PAGE_PROXY_OPEN] = currentBProxyOpen;
      obj[PAGE_PROXY_IP] = currentHost;
      obj[PAGE_PROXY_PORT] = currentPort;
      obj[PAGE_PROXY_USERNAME] = currentUsr;
      obj[PAGE_PROXY_PASSWORD] = currentPwd;

      ConfigSetting::writeValue(qsConfPath, GROUP_DEFAULT, PROXY_OPEN, currentBProxyOpen ? 1 : 0);
      ConfigSetting::writeValue(qsConfPath, GROUP_DEFAULT, PROXY_HOST, currentHost);
      ConfigSetting::writeValue(qsConfPath, GROUP_DEFAULT, PROXY_PORT, currentPort);
      ConfigSetting::writeValue(qsConfPath, GROUP_DEFAULT, PROXY_USERNAME, currentUsr);
      ConfigSetting::writeValue(qsConfPath, GROUP_DEFAULT, PROXY_PASSWORD, currentPwd);
      ConfigSetting::writeValue(qsConfPath, GROUP_DEFAULT, PROXY_TYPE, proxyType);

      if (currentBProxyOpen && !currentHost.isEmpty()) {
         GetHttpManagerInstance()->SwitchHttpProxy(true, currentHost.toStdString(), currentPort, currentUsr.toStdString(), currentPwd.toStdString());
      }
      else {
         GetHttpManagerInstance()->SwitchHttpProxy(false);
      }

      TRACE6("[BP] SystemSettingDlg::ApplyPageStatus e_page_proxy [%s][%s][%d][%s][%s]\n",
         currentBProxyOpen ? "Open" : "Close",
         currentHost.toLocal8Bit().data(),
         currentPort,
         currentUsr.toLocal8Bit().data(),
         currentPwd.toLocal8Bit().data());

      QNetworkProxy proxy;
      if (currentBProxyOpen) {
         proxy.setType(QNetworkProxy::HttpProxy);
         proxy.setHostName(currentHost);
         proxy.setPort(currentPort);
         proxy.setUser(currentUsr);
         proxy.setPassword(currentPwd);
      }
      else {
         proxy.setType(QNetworkProxy::NoProxy);
      }
      QNetworkProxy::setApplicationProxy(proxy);
      break;
   }
   case e_page_debug:
   case e_page_question:
   case e_page_aboutUs:
   default:
      break;
   }
   return obj;
}

QJsonObject SystemSettingDlg::InitPageStatus(e_page_type type) {
   int iApplyIdentify = m_ApplyIdentify;
   QJsonObject obj = QJsonObject();

   QString qsConfPath = CPathManager::GetConfigPath();
   VH::CComPtr<ISettingLogic> pSettingLogic;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IMainUI, IID_ISettingLogic, pSettingLogic, return obj, ASSERT(FALSE));
   switch (type) {
   case e_page_common:{
      int defaultLine = ConfigSetting::ReadInt(qsConfPath, GROUP_DEFAULT, KEY_VIDEO_LINE, 0);
      if (defaultLine >= ui.comboBox_publishLines->count() || defaultLine < 0) {
         defaultLine = 0;
      }
      int defaultResolution = ConfigSetting::ReadInt(qsConfPath, GROUP_DEFAULT, KEY_PUBLISH_QUALITY, 2);
      if (defaultResolution >= ui.comboBox_resolution->count() || defaultResolution < 0) {
         defaultResolution = 0;
      }
      int videoQuality = ConfigSetting::ReadInt(qsConfPath, GROUP_DEFAULT, KEY_VCODEC_VIDEO_QUALITY, 0);
      int HighQualityCod = ConfigSetting::ReadInt(qsConfPath, GROUP_DEFAULT, KEY_VCODEC_HIGH_PROFILE_OPEN, 0);
      bool defaultServerPlayBack = (ConfigSetting::ReadInt(qsConfPath, GROUP_DEFAULT, KEY_SERVER_PLAYBACK, 0) != 0);
      int openDesktopCaptureType = ConfigSetting::ReadInt(qsConfPath, GROUP_DEFAULT, KEY_DESKCTOP_CAPTURE_TYPE, 0);
      if (openDesktopCaptureType == 0) {
         ui.checkBox_desktopcapture->setCheckState(Qt::CheckState::Unchecked);
      }
      else {
         ui.checkBox_desktopcapture->setCheckState(Qt::CheckState::Checked);
      }
      obj[PAGE_COMMON_RESOLUTION] = defaultResolution;
      obj[PAGE_COMMON_LINE] = defaultLine;
      obj[PAGE_COMMON_SERVER_PLAYBACK] = defaultServerPlayBack;
      obj[PAGE_VIDEO_QUALITY] = videoQuality;
      obj[PAGE_VIDEO_HIGHQUALITYCOD] = HighQualityCod;
      obj[PAGE_COMMON_DESKTOP_CAPTURE_TYPE] = openDesktopCaptureType;
      
      qDebug() << "InitPageStatus PAGE_COMMON_SERVER_PLAYBACK" << defaultServerPlayBack;
      ui.cmbHighQualityCod->setCurrentText(QString::number(HighQualityCod));
      if (videoQuality > 0) {
         ui.checkBox_videoQuality->setCheckState(Qt::CheckState::Checked);
      }
      else {
         ui.checkBox_videoQuality->setCheckState(Qt::CheckState::Unchecked);
      }
      if (defaultLine < ui.comboBox_publishLines->count()) {
         ui.comboBox_publishLines->setCurrentIndex(defaultLine);
      }
      if (defaultResolution < ui.comboBox_resolution->count()) {
         ui.comboBox_resolution->setCurrentIndex(defaultResolution);
      }   
      break;
   }
   case e_page_vedio: {
      pSettingLogic->InitCameraSetting();
      int device = ui.comboBox_device->currentIndex();
      int resolution = ui.comboBox_deviceResolution->currentIndex();
      int fps = ui.comboBox_frame->currentIndex();
      int deinterlace = ui.comboBox_deinterLace->currentIndex();

      obj[PAGE_VIDEO_DEVICE] = device;
      obj[PAGE_VIDEO_RESOLUTION] = resolution;
      obj[PAGE_VIDEO_FPS] = fps;
      obj[PAGE_VIDEO_DEINTERLACE] = deinterlace;
      break;
   }     
   case e_page_audio: {
      pSettingLogic->InitAudioSetting();
      bool isNoise = !!ConfigSetting::ReadInt(qsConfPath, GROUP_DEFAULT, KEY_AUDIO_ISNOISE, 0);
      int noiseValue = ConfigSetting::ReadInt(qsConfPath, GROUP_DEFAULT, KEY_AUDIO_NOISE_VALUE, 0);
      bool isForceMono = !!ConfigSetting::ReadInt(qsConfPath, GROUP_DEFAULT, KEY_AUDIO_FORCEMONO, 1);
      int iKbps = ConfigSetting::ReadInt(qsConfPath, GROUP_DEFAULT, KEY_AUDIO_KBPS, 96);
      int iAudioSampleRate = ConfigSetting::ReadInt(qsConfPath, GROUP_DEFAULT, KEY_AUDIO_SAMPLERATE, 44100);
      bool bNoiseReduction = ConfigSetting::ReadInt(qsConfPath, GROUP_DEFAULT, KEY_AUDIO_NOISEREDUCTION, 0) == 0 ? false : true;
      int iMicGain = ConfigSetting::ReadInt(qsConfPath, GROUP_DEFAULT, KEY_AUDIO_MICGAIN, 16);

      obj[PAGE_AUDIO_KBPS] = iKbps;
      obj[PAGE_AUDIO_SAMPLERATE] = iAudioSampleRate;
      obj[PAGE_AUDIO_NOISEREDUCTION] = bNoiseReduction;
      obj[PAGE_AUDIO_MICGAIN] = iMicGain;

      ui.ckbNoiseReduction->setCheckState(bNoiseReduction == true ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
      ui.cmbKbps->setCurrentText(QString::number(iKbps));
      ui.cmbAudioSampleRate->setCurrentText(QString::number(iAudioSampleRate));
      ui.micGainSld->setValue(iMicGain);

      if (ui.micListCombx->count() > 0) {
         int micIndex = ui.micListCombx->currentIndex();
         obj[PAGE_AUDIO_MIC] = micIndex;
      }

      ui.widget_noiseSetting->InitNoiseSetting(isNoise, noiseValue);
      obj[PAGE_AUDIO_ISNOISE] = isNoise;
      obj[PAGE_AUDIO_FORCEMONO] = isForceMono;

      ui.checkBox_Channel->setChecked(!isForceMono);
      ui.checkBox->setChecked(isNoise);

      if (ui.speakerListCombx->count() > 0) {
         obj[PAGE_AUDIO_SPEAKER] = ui.speakerListCombx->currentIndex();
      }
      break;
   }        
   case e_page_record:{
      bool bRecord = ConfigSetting::ReadInt(qsConfPath, GROUP_DEFAULT, KEY_RECORD_MODE, 0);
      QString filePath = ConfigSetting::ReadString(qsConfPath, GROUP_DEFAULT, KEY_RECORD_PATH, "");
      if (filePath == "") {
         filePath = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation).first();
         ConfigSetting::writeValue(qsConfPath, GROUP_DEFAULT, KEY_RECORD_PATH, filePath);
      }
      QString fileName = ConfigSetting::ReadString(qsConfPath, GROUP_DEFAULT, KEY_RECORD_FILENAME, "");
      obj[PAGE_RECORD_ISRECORD] = bRecord;
      obj[PAGE_RECORD_FILENAME] = fileName;
      obj[PAGE_RECORD_FILEPATH] = filePath;
      SetRecord(bRecord);
      SetSaveFileName(fileName);
      SetSavePath(filePath);
      break;
   }
   case e_page_proxy:{
      bool bProxyOpen = ConfigSetting::ReadInt(qsConfPath, GROUP_DEFAULT, PROXY_OPEN, 0);
      if (bProxyOpen) {
         int proxyType = ConfigSetting::ReadInt(qsConfPath, GROUP_DEFAULT, PROXY_TYPE, ProxyConfig_Http);
         ui.comboBox_proxyEnable->setCurrentIndex(proxyType);
         UIEnableProxy();
      }
      else {
         ui.comboBox_proxyEnable->setCurrentIndex(0);
         UIDisableProxy();
      }

      QString host = ConfigSetting::ReadString(qsConfPath, GROUP_DEFAULT, PROXY_HOST, "");
      int port = ConfigSetting::ReadInt(qsConfPath, GROUP_DEFAULT, PROXY_PORT, 80);
      QString usr = ConfigSetting::ReadString(qsConfPath, GROUP_DEFAULT, PROXY_USERNAME, "");
      QString pwd = ConfigSetting::ReadString(qsConfPath, GROUP_DEFAULT, PROXY_PASSWORD, "");

      ui.lineEdit_proxy_ip->setText(host);
      ui.lineEdit_proxy_port->setText(QString::number(port));
      ui.lineEdit_proxy_userName->setText(usr);
      ui.lineEdit_proxy_password->setText(pwd);

      obj[PAGE_PROXY_OPEN] = bProxyOpen;
      obj[PAGE_PROXY_IP] = host;
      obj[PAGE_PROXY_PORT] = port;
      obj[PAGE_PROXY_USERNAME] = usr;
      obj[PAGE_PROXY_PASSWORD] = pwd;

      ui.label_tip->clear();
      if (!bProxyOpen) {
         ui.btn_testing->setEnabled(false);
      }
      break;
   }
   case e_page_voicetranslate: {
      int fontSize = ConfigSetting::ReadInt(qsConfPath, GROUP_DEFAULT, KEY_VT_FONT_SIZE, 15);
      QString lan = ConfigSetting::ReadString(qsConfPath, GROUP_DEFAULT, KEY_VT_LANGUAGE, QString::fromStdWString(TextLanguage_Mandarin));
      QString fileName = ConfigSetting::ReadString(qsConfPath, GROUP_DEFAULT, KEY_RECORD_FILENAME, "");
      obj[PAGE_VT_FONT] = fontSize;
      obj[PAGE_VT_LAN] = lan;
      ui.comboBox_vt_fontSize->setCurrentText(QString::number(fontSize));
      ui.comboBox_vt_language->setCurrentText(lan);
      break;
   }                  
   case e_page_debug:
   case e_page_question:
   case e_page_aboutUs:
   default:
      break;
   }
   m_ApplyIdentify = iApplyIdentify;
   return obj;
}

void SystemSettingDlg::CheckApplyStatus() {
   int currentIndex = ui.stackedWidget->currentIndex();
   e_page_type type = PageIdToType(currentIndex);

   auto itor = m_ApplyInitialStatus.find(type);
   if (itor == m_ApplyInitialStatus.end()) {
      return;
   }

   QJsonObject obj = itor.value();
   if (obj.empty()) {
      return;
   }

   switch (type) {
   case e_page_common: {
      int currentResolution = ui.comboBox_resolution->currentIndex();
      int currentLine = ui.comboBox_publishLines->currentIndex();
      int defaultResolution = obj[PAGE_COMMON_RESOLUTION].toInt();
      int defaultLine = obj[PAGE_COMMON_LINE].toInt();
      int iHighQualityCod = obj[PAGE_VIDEO_HIGHQUALITYCOD].toInt();
      int videoQuality = obj[PAGE_VIDEO_QUALITY].toInt();
      int currentVideoQulity = ui.checkBox_videoQuality->checkState();
      int currentHighQualityCod = ui.cmbHighQualityCod->currentText().toInt();
      int currentDesktopCaptureType = ui.checkBox_desktopcapture->checkState();
      int lastDesktopCaptureType = obj[PAGE_COMMON_DESKTOP_CAPTURE_TYPE].toInt();
      if (videoQuality != currentVideoQulity || currentResolution != defaultResolution || currentLine != defaultLine || iHighQualityCod != currentHighQualityCod || currentDesktopCaptureType != lastDesktopCaptureType) {
         m_ApplyIdentify |= e_page_common;
      }
      else {
         m_ApplyIdentify &= ~e_page_common;
      }
      break;
   }
   case e_page_vedio:
   {
      int device = obj[PAGE_VIDEO_DEVICE].toInt();
      int resolution = obj[PAGE_VIDEO_RESOLUTION].toInt();
      int fps = obj[PAGE_VIDEO_FPS].toInt();
      int deinterlace = obj[PAGE_VIDEO_DEINTERLACE].toInt();
      if (!(ui.comboBox_device->count() && ui.comboBox_deviceResolution->count() && ui.comboBox_frame->count() && ui.comboBox_deinterLace->count())) {
         break;
      }

      int currentDevice = ui.comboBox_device->currentIndex();
      int currentResolution = ui.comboBox_deviceResolution->currentIndex();
      int currentFps = ui.comboBox_frame->currentIndex();
      int currentDeinterlace = ui.comboBox_deinterLace->currentIndex();
      int currentHighQualityCod = ui.cmbHighQualityCod->currentText().toInt();
      if (device != currentDevice || resolution != currentResolution || fps != currentFps || deinterlace != currentDeinterlace) {
         m_ApplyIdentify |= e_page_vedio;
      }
      else {
         m_ApplyIdentify &= ~e_page_vedio;
      }
      break;
   }
   case e_page_audio:{
      bool bChange = false;
      bool isNoise = obj[PAGE_AUDIO_ISNOISE].toBool();
      bool isForceMono = obj[PAGE_AUDIO_FORCEMONO].toBool();
      bool currentIsNoise = ui.checkBox->isChecked();
      bool currentIsForceMono = !ui.checkBox_Channel->isChecked();

      if (isNoise != currentIsNoise || isForceMono != currentIsForceMono) {
         bChange = true;
      }
      if (!bChange) {
         if (ui.micListCombx->count() > 0) {
            int micIndex = obj[PAGE_AUDIO_MIC].toInt();
            int currentMicIndex = ui.micListCombx->currentIndex();
            if (micIndex != currentMicIndex) {
               bChange = true;
            }
         }

         if (ui.speakerListCombx->count() > 0) {
            int speakerIndex = obj[PAGE_AUDIO_SPEAKER].toInt();
            int currentSpeakerIndex = ui.speakerListCombx->currentIndex();
            if (speakerIndex != currentSpeakerIndex) {
               bChange = true;
            }
         }
      }

      if (!bChange && ui.cmbKbps->currentText().toInt() != obj[PAGE_AUDIO_KBPS].toInt()) {
         bChange = true;
      }

      if (!bChange && ui.cmbAudioSampleRate->currentText().toInt() != obj[PAGE_AUDIO_SAMPLERATE].toInt()) {
         bChange = true;
      }

      if (!bChange && ui.ckbNoiseReduction->isChecked() != obj[PAGE_AUDIO_NOISEREDUCTION].toBool()) {
         bChange = true;
      }

      QString str = QString(QJsonDocument(obj).toJson());
      int iNu = ui.micGainSld->value();
      if (!bChange && ui.micGainSld->value() != obj[PAGE_AUDIO_MICGAIN].toInt()) {
         bChange = true;
      }

      if (bChange) {
         m_ApplyIdentify |= e_page_audio;
      }
      else {
         m_ApplyIdentify &= ~e_page_audio;
      }
      break;
   }
   case e_page_ai: {
      int checked = ui.checkBox_desktopEnHance->checkState();
      ConfigSetting::writeValue(CPathManager::GetConfigPath(), GROUP_DEFAULT, DESKTOP_ENHANCE, checked);
      VH::CComPtr<IOBSControlLogic> pOBSControlLogic;
      DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IOBSControl, IID_IOBSControlLogic, pOBSControlLogic, return);
      if (pOBSControlLogic) {
         pOBSControlLogic->DesktopEnhanceControlSwitch(Qt::CheckState::Checked == checked ? true : false);
         if (Qt::CheckState::Checked == checked) {
            QJsonObject body;
            SingletonMainUIIns::Instance().reportLog(STR_VH_LIVE_OPERATE, eLogRePortK_Desktop_Hance, L"desktop_enhance", body);
         }
         TRACE6("%s DesktopEnhanceControlSwitch \n", __FUNCTION__);
      }
      if (ui.horizontalSlider_Beauty->value() > 0) {
         QJsonObject body;
         SingletonMainUIIns::Instance().reportLog(STR_VH_LIVE_OPERATE, eLogRePortK_Beauty_Open, L"beauty_open", body);
         TRACE6("%s beauty level %d\n", __FUNCTION__, ui.horizontalSlider_Beauty->value());
      }
      break;
   }
   case e_page_record: {
      bool bRecord = obj[PAGE_RECORD_ISRECORD].toBool();
      QString filePath = obj[PAGE_RECORD_FILEPATH].toString();
      QString fileName = obj[PAGE_RECORD_FILENAME].toString();
      bool currentRecord = ui.recordCbx->isChecked();
      QString currentFilePath = ui.lineEdit_savePath->text();
      QString currentFileName = ui.lineEdit_fileName->text().trimmed();
      if (bRecord != currentRecord || filePath != currentFilePath || fileName != currentFileName
         ) {
         m_ApplyIdentify |= e_page_record;
      }
      else {
         m_ApplyIdentify &= ~e_page_record;
      }
      break;
   }
   case e_page_proxy: {
      bool bProxyOpen = obj[PAGE_PROXY_OPEN].toBool();
      bool currentBProxyOpen = ui.comboBox_proxyEnable->currentIndex() != 0;
      int port = obj[PAGE_PROXY_PORT].toInt();
      int currentPort = ui.lineEdit_proxy_port->text().toInt();
      QString usr = obj[PAGE_PROXY_USERNAME].toString();
      QString pwd = obj[PAGE_PROXY_PASSWORD].toString();
      QString host = obj[PAGE_PROXY_IP].toString();
      QString currentHost = ui.lineEdit_proxy_ip->text();
      QString currentUsr = ui.lineEdit_proxy_userName->text();
      QString currentPwd = ui.lineEdit_proxy_password->text();

      if (bProxyOpen != currentBProxyOpen|| host != currentHost|| port != currentPort|| usr != currentUsr|| pwd != currentPwd){
         m_ApplyIdentify |= e_page_proxy;
      }
      else {
         m_ApplyIdentify &= ~e_page_proxy;
      }
      break;
   }        
   case e_page_voicetranslate: {
      int nFont = obj[PAGE_VT_FONT].toInt();
      QString lan = obj[PAGE_VT_LAN].toString();
      int currentFont = ui.comboBox_vt_fontSize->currentText().toInt();
      QString currentLan = ui.comboBox_vt_language->currentText();
      if (nFont != currentFont || lan != currentLan) {
         m_ApplyIdentify |= e_page_voicetranslate;
      }
      else {
         m_ApplyIdentify &= ~e_page_voicetranslate;
      }
      break;
   }             
   default:
      break;
   }
}

void SystemSettingDlg::Clear() {
   ui.comboBox_questionType->setCurrentIndex(0);
   ui.textBrowser_Content->clear();
   m_ApplyInitialStatus.clear();
}

void SystemSettingDlg::Show(const QRect& rect, e_page_type eType, bool mode) {
   VH::CComPtr<ICommonData> pCommonData;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
   if (pCommonData->GetPublishState()) {
      ui.checkBox_videoQuality->setEnabled(false);
   }
   else {
      ui.checkBox_videoQuality->setEnabled(true);
   }
   ClientApiInitResp startLiveInfo;
   pCommonData->GetInitApiResp(startLiveInfo);
   ui.label_lineText->show();
   ui.comboBox_publishLines->show();
   ////h5 活动不支持线路切换
   int dpi_enable = ConfigSetting::ReadInt(CPathManager::GetConfigPath(), GROUP_DEFAULT, DESKTOP_DIP, 1);
   dpi_enable == 1 ? ui.checkBox_dpi->setCheckState(Qt::CheckState::Checked) : ui.checkBox_dpi->setCheckState(Qt::CheckState::Unchecked);
   Clear();
   ChangePage(eType);
   CenterWindow((QWidget *)this->currentParent);
   m_ApplyIdentify = 0;
   CheckApplyStatus();

   VH::CComPtr<IVedioPlayLogic> pVedioPlayLogic = NULL;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IMainUI, IID_IVedioPlayLogic, pVedioPlayLogic, return);
   if (pVedioPlayLogic) {
      pVedioPlayLogic->StopAdmin();
   }

   VH::CComPtr<IOBSControlLogic> pOBSControlLogic;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IOBSControl, IID_IOBSControlLogic, pOBSControlLogic, return);
   if (pOBSControlLogic) {
      pOBSControlLogic->SetSaveMicAudio(true);
   }
   setModal(mode);
   int iX = rect.x() + (rect.width() - width()) / 2;
   int iy = rect.y() + (rect.height() - height()) / 2;

   this->move(iX, iy);
   show();
}

void SystemSettingDlg::CenterWindow(QWidget* parent) {
   int x = 0;
   int y = 0;
   if (NULL == parent) {
      const QRect rect = QApplication::desktop()->availableGeometry();
      x = rect.left() + (rect.width() - width()) / 2;
      y = rect.top() + (rect.height() - height()) / 2;
   }
   else {
      QPoint point(0, 0);
      point = parent->mapToGlobal(point);
      x = point.x() + (parent->width() - width()) / 2;
      y = point.y() + (parent->height() - height()) / 2;
   }
   move(x, y);
}
void SystemSettingDlg::GetRecordFilePath(wchar_t *wfilePath) {
   if (!wfilePath) {
      return;
   }

   QString filePath = ui.lineEdit_savePath->text();
   QString fileName = ui.lineEdit_fileName->text().trimmed();

   QJsonObject obj = GetPageData(e_page_record);
   if (!obj.empty()) {
      filePath = obj[PAGE_RECORD_FILEPATH].toString();
      fileName = obj[PAGE_RECORD_FILENAME].toString();
   }

   QString recordPath = filePath + "/" + fileName;
   recordPath.toWCharArray(wfilePath);
}

void SystemSettingDlg::BindDevice(DeviceInfo &info) {
   for (int i = 0; i < ui.comboBox_device->count(); i++) {
      DeviceInfo dInfo;
      if (GetCameraDeviceInfo(i, dInfo)) {
         if (dInfo == info) {
            ui.comboBox_device->setCurrentIndex(i);
            break;
         }
      }
   }
   m_ApplyInitialStatus[e_page_vedio] = InitPageStatus(e_page_vedio);
   CheckApplyStatus();
}

void SystemSettingDlg::ClearItem() {
   disconnect(ui.comboBox_device, SIGNAL(currentIndexChanged(int)), this, SLOT(OnChgDevice(int)));
   ui.comboBox_device->clear();
   ui.comboBox_device->addItem(QString::fromStdWString(L"没有检测到摄像设备"));
   ui.comboBox_device->setEnabled(false);
   connect(ui.comboBox_device, SIGNAL(currentIndexChanged(int)), this, SLOT(OnChgDevice(int)));
   ui.comboBox_deviceResolution->clear();
   m_resolutionList.Clear();
   ui.comboBox_frame->clear();
   ui.comboBox_deinterLace->clear();
}

static void FuncAppend(FrameInfo info, void *ptr){
   std::list<FrameInfo> *infos = (std::list<FrameInfo> *)ptr;
   infos->push_back(info);
}

//重设帧率
void SystemSettingDlg::ReloadCameraFps(int currentFps) {
   disconnect(ui.comboBox_frame, SIGNAL(currentIndexChanged(int)), this, SLOT(OnFrameChg(int)));
   connect(ui.comboBox_frame, SIGNAL(currentIndexChanged(int)), this, SLOT(OnFrameChg(int)));
}

//重设分辨率
void SystemSettingDlg::ReloadCameraResolution(FrameInfoList &frameInfoList, FrameInfo currentFrameInfo) {
   int index = 0;
   disconnect(ui.comboBox_deviceResolution, SIGNAL(currentIndexChanged(int)), this, SLOT(OnResolutionIndexChg(int)));
   ui.comboBox_deviceResolution->clear();
   ui.comboBox_frame->clear();

   m_resolutionList.Clear();
   for (auto itor = frameInfoList.infos.begin(); itor != frameInfoList.infos.end(); itor++){
      m_resolutionList.Append(*itor);
   }

   m_resolutionList.sort();
   for (int i = 0; i < m_resolutionList.Count(); i++){
      ui.comboBox_deviceResolution->addItem(m_resolutionList.DisplayString(i), QVariant(i));
   }

   index = m_resolutionList.SetCurrentFrameInfo(currentFrameInfo);
   ui.comboBox_deviceResolution->setCurrentIndex(index);
   connect(ui.comboBox_deviceResolution, SIGNAL(currentIndexChanged(int)), this, SLOT(OnResolutionIndexChg(int)));

   OnResolutionIndexChg(index);
}

//重设去交错
void SystemSettingDlg::ReloadCameraDeinterLace(DeinterlacingType deinterType) {
   disconnect(ui.comboBox_deinterLace, SIGNAL(currentIndexChanged(int)), this, SLOT(OnDetainterChg(int)));
   ui.comboBox_deinterLace->setCurrentIndex((int)deinterType);
   connect(ui.comboBox_deinterLace, SIGNAL(currentIndexChanged(int)), this, SLOT(OnDetainterChg(int)));
}
void SystemSettingDlg::RecheckFps() {

}

void SystemSettingDlg::ReloadCameraAtttibute(DeviceInfo deviceInfo) {
   //去交错
   DeinterlacingType deinterType = DEINTERLACING_NONE;
   //帧率列表
   FrameInfoList frameInfoList;
   //当前帧率
   FrameInfo currentFrameInfo;
   if (deviceInfo.m_sDeviceType == TYPE_DSHOW_VIDEO){
      frameInfoList.appendFunc = FuncAppend;
      //获得视频设备列表
      if (!GetDShowVideoFrameInfoList(deviceInfo, &frameInfoList, &currentFrameInfo, deinterType)){
         return;
      }
   }
   else if (deviceInfo.m_sDeviceType == TYPE_DECKLINK){
      UINT w, h;
      int frameInternal;
      UINT Twidth = 0;
      UINT Theight = 0;
      int TframeInternal = 0;
      VideoFormat format;
      if (!GetDeckLinkDeviceInfo(deviceInfo, w, h, frameInternal)){
         return;
      }

      currentFrameInfo.minFrameInterval = currentFrameInfo.maxFrameInterval = frameInternal;
      currentFrameInfo.minCX = currentFrameInfo.maxCX = w;
      currentFrameInfo.minCY = currentFrameInfo.maxCY = h;
      frameInfoList.PushBack(currentFrameInfo);
      GetDeviceDefaultAttribute(deviceInfo, Twidth, Theight, TframeInternal, deinterType, format);
   }
   //去交错
   ReloadCameraDeinterLace(deinterType);
   //分辨率帧率
   ReloadCameraResolution(frameInfoList, currentFrameInfo);
}
//
void SystemSettingDlg::OnChgDevice(int nIndex) {
   if (nIndex < 0) {
      return;
   }
   if (nIndex >= ui.comboBox_device->count()) {
      return;
   }

   ui.label_deviceNum->setText(QString::fromStdWString(L"摄像设备 ")+ QString::number(nIndex + 1));
   //或的当前设备
   DeviceInfo deviceInfo;
   if (!GetCameraDeviceInfo(nIndex, deviceInfo)) {
      return;
   }

   ReloadCameraAtttibute(deviceInfo);
   CheckApplyStatus();
}

bool SystemSettingDlg::GetCurrentCameraDeviceInfo(DeviceInfo &info) {
   if (0 == ui.comboBox_device->count()) {
      return false;
   }
   int index = ui.comboBox_device->currentIndex();
   QJsonObject obj = GetPageData(e_page_vedio);
   if (!obj.empty()) {
      index = obj[PAGE_VIDEO_DEVICE].toInt();
   }
   return GetCameraDeviceInfo(index, info);
}

void SystemSettingDlg::OnBeautyDeviceSelect(int index) {
   if (index < 0) {
      return;
   }

   QVariant auserData = ui.comboBox_beautyCamera->itemData(index);
   QString str = auserData.value<QString>();
   QByteArray ba = QByteArray::fromBase64(str.toLocal8Bit());
   if (ba.size() != sizeof(DeviceInfo)) {
      return;
   }

   mCurrentBeautyDev = *(DeviceInfo *)ba.data();
   ReloadCameraAtttibute(mCurrentBeautyDev);
   disconnect(ui.horizontalSlider_Beauty, SIGNAL(valueChanged(int)), this, SLOT(OnBeautyValueChange(int)));
   std::map<QString, int>::iterator iter = mDevBeautyLeavel.find(QString::fromStdWString(mCurrentBeautyDev.m_sDeviceID));
   if (iter != mDevBeautyLeavel.end()) {
      ui.horizontalSlider_Beauty->setValue(iter->second);
   }
   else {
      ui.horizontalSlider_Beauty->setValue(0);
   }
   connect(ui.horizontalSlider_Beauty, SIGNAL(valueChanged(int)), this, SLOT(OnBeautyValueChange(int)));
}

void SystemSettingDlg::SelectCamera(DeviceInfo &deviceInfo){
   if (ui.comboBox_beautyCamera->count() == 1 && ui.comboBox_beautyCamera->currentText() == NO_INPUT) {
      ui.comboBox_beautyCamera->clear();
   }

   for (int i = 0; i < ui.comboBox_beautyCamera->count(); i++) {
      QVariant auserData = ui.comboBox_beautyCamera->itemData(i);
      QString str = auserData.value<QString>();
      QByteArray ba = QByteArray::fromBase64(str.toLocal8Bit());
      if (ba.size() == sizeof(DeviceInfo)) {
         DeviceInfo item_info = *(DeviceInfo *)ba.data();
         if (QString::fromStdWString(deviceInfo.m_sDeviceID) == QString::fromStdWString(item_info.m_sDeviceID) &&
            QString::fromStdWString(deviceInfo.m_sDeviceDisPlayName) == QString::fromStdWString(item_info.m_sDeviceDisPlayName)) {
            return;
         }
      }
   }

   disconnect(ui.comboBox_beautyCamera, SIGNAL(currentIndexChanged(int)), this, SLOT(OnBeautyDeviceSelect(int)));
   QVariant qItemData;
   QByteArray ba;
   ba.setRawData((const char *)&deviceInfo, sizeof(DeviceInfo));
   QString str = QString::fromLocal8Bit(ba.toBase64());
   qItemData.setValue(str);
   ui.comboBox_beautyCamera->addItem(QString::fromWCharArray(deviceInfo.m_sDeviceDisPlayName), qItemData);
   if (ui.comboBox_beautyCamera->count() == 1) {
      mCurrentBeautyDev = deviceInfo;
   }
   connect(ui.comboBox_beautyCamera, SIGNAL(currentIndexChanged(int)), this, SLOT(OnBeautyDeviceSelect(int)));
   std::map<QString, int>::iterator iter = mDevBeautyLeavel.find(QString::fromStdWString(deviceInfo.m_sDeviceID));
   if (iter == mDevBeautyLeavel.end()) {
      mDevBeautyLeavel.insert(::make_pair(QString::fromStdWString(deviceInfo.m_sDeviceID), 0));
   }
   ui.horizontalSlider_Beauty->setEnabled(true);
}

void SystemSettingDlg::RemoveCamera(DeviceInfo &info){
   for (int i = 0; i < ui.comboBox_beautyCamera->count(); i++) {
      QVariant auserData = ui.comboBox_beautyCamera->itemData(i);
      QString str = auserData.value<QString>();
      QByteArray ba = QByteArray::fromBase64(str.toLocal8Bit());
      if (ba.length() > 0) {
         DeviceInfo item_info = *(DeviceInfo *)ba.data();
         if (QString::fromStdWString(info.m_sDeviceID) == QString::fromStdWString(item_info.m_sDeviceID) &&
            QString::fromStdWString(info.m_sDeviceDisPlayName) == QString::fromStdWString(item_info.m_sDeviceDisPlayName)) {
            ui.comboBox_beautyCamera->removeItem(i);
            break;
         }
      } 
   }
   std::map<QString, int>::iterator iter = mDevBeautyLeavel.find(QString::fromStdWString(info.m_sDeviceID));
   if (iter != mDevBeautyLeavel.end()) {
      mDevBeautyLeavel.erase(iter);
   }
   if (ui.comboBox_beautyCamera->count() == 0) {
      ui.horizontalSlider_Beauty->setValue(0);
      ui.horizontalSlider_Beauty->setEnabled(false);
      ui.widget_24->hide();
   }
}

int SystemSettingDlg::GetQuality() {
   int quality = ui.comboBox_resolution->currentIndex();
   QJsonObject obj = GetPageData(e_page_common);
   if (!obj.empty()) {
      quality = obj[PAGE_COMMON_RESOLUTION].toInt();
   }
   return quality;
}

int SystemSettingDlg::GetPublishLine() {
   int line = ui.comboBox_publishLines->currentIndex();
   QJsonObject obj = GetPageData(e_page_common);
   if (!obj.empty()) {
      line = obj[PAGE_COMMON_LINE].toInt();
   }
   return line;
}

bool SystemSettingDlg::eventFilter(QObject *obj, QEvent *ev) {
   if (ev) {
      if (ev->type() == QEvent::MouseButtonPress ||
         ev->type() == QEvent::MouseButtonRelease ||
         ev->type() == QEvent::Wheel) {
         CheckApplyStatus();
      }

      if (ev->type() == QEvent::MouseButtonPress) {
         //麦克风音量slider, 点击即到
         if (obj == ui.micVolumeSlider  && ui.micVolumeSlider->isEnabled()) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(ev);
            if (mouseEvent->button() == Qt::LeftButton)
            {
               int dur = ui.micVolumeSlider->maximum() - ui.micVolumeSlider->minimum();
               int pos = ui.micVolumeSlider->minimum() + dur * ((double)(mouseEvent->x() - 5) / (ui.micVolumeSlider->width() - 10));
               if (pos != ui.micVolumeSlider->sliderPosition())
               {
                  ui.micVolumeSlider->setValue(pos);
               }
            }
         }
         // 扬声器音量slider, 点击即到
         if (obj == ui.speakerVolumeSlider && ui.speakerVolumeSlider->isEnabled()) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(ev);
            if (mouseEvent->button() == Qt::LeftButton)
            {
               int dur = ui.speakerVolumeSlider->maximum() - ui.speakerVolumeSlider->minimum();
               int pos = ui.speakerVolumeSlider->minimum() + dur * ((double)(mouseEvent->x() - 5) / (ui.speakerVolumeSlider->width() - 10));
               if (pos != ui.speakerVolumeSlider->sliderPosition())
               {
                  ui.speakerVolumeSlider->setValue(pos);
               }
            }
         }
         //麦克风降噪
         if (obj == ui.micGainSld && ui.micGainSld->isEnabled()) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(ev);
            if (mouseEvent->button() == Qt::LeftButton) {
               int dur = ui.micGainSld->maximum() - ui.micGainSld->minimum();
               int pos = ui.micGainSld->minimum() + dur * ((double)(mouseEvent->x() - 5) / (ui.micGainSld->width() - 10));
               if (pos != ui.micGainSld->sliderPosition()) {
                  ui.micGainSld->setValue(pos);
               }
            }
         }
      }

      if (ev->type() == QEvent::FocusIn) {
         if (obj == ui.textBrowser_Content) {
            ui.textBrowser_Content->setPlaceholderText("");
         }

         if (obj == ui.lineEdit_fileName) {
            ui.lineEdit_fileName->setPlaceholderText("");
         }
      }

      if (ev->type() == QEvent::FocusOut) {
         if (obj == ui.lineEdit_fileName) {
            ui.lineEdit_fileName->setPlaceholderText("请输入录制文件名称");
         }
         if (obj == ui.textBrowser_Content) {
            if (ui.textBrowser_Content->placeholderText().length() == 0) {
               ui.textBrowser_Content->setPlaceholderText("感谢您的建议！\n\n请描述您在使用过程中出现的问题或困扰，以便我们优化我们的产品和服务。");
            }
         }
         if (obj == ui.comboBox_frame) {
            RecheckFps();
         }
      }

      if (ev->type() == QEvent::Enter) {
         if (obj == ui.labNoiseValve && m_pNoiseTips != NULL /*&& m_pNoiseTips->isHidden()*/) {
            m_pNoiseTips->setText(QString::fromWCharArray(L"当前输入音量小于噪音闸时，麦克风输出音量自动静音；当音量输入音量大于噪音闸时，音量大于噪音闸的声音会被传输出去。"));
            m_pNoiseTips->show();
            QPoint pos = this->mapToGlobal(ui.labNoiseValve->pos());
            m_pNoiseTips->move(pos + QPoint(0, 60));
            m_pNoiseTips->setFixedHeight(125);
         }
         //音频码率
         if (obj == ui.labKbps && m_pNoiseTips != NULL/* && m_pNoiseTips->isHidden()*/) {
            m_pNoiseTips->setFixedHeight(105);
            m_pNoiseTips->setText(QString::fromWCharArray(L"合适的码率会使直播的声音更清晰，同时会占用带宽。"));
            m_pNoiseTips->show();
            QPoint pos = this->mapToGlobal(ui.labKbps->pos());
            m_pNoiseTips->move(pos + QPoint(0, 60));
         }
         //音频采样频率
         if (obj == ui.labAudioSampleRate && m_pNoiseTips != NULL /*&& m_pNoiseTips->isHidden()*/) {
            m_pNoiseTips->setText(QString::fromWCharArray(L"针对某些麦克风采集声音不清楚时，更改此选项会改善音质。"));
            m_pNoiseTips->setFixedHeight(110);
            m_pNoiseTips->show();
            QPoint pos = this->mapToGlobal(ui.labAudioSampleRate->pos());
            m_pNoiseTips->move(pos + QPoint(0, 60));
         }
         //实时字幕语言提示
         if (obj == ui.label_voiceTranslateTip && m_pNoiseTips != NULL) {
            m_pNoiseTips->setText(QString::fromWCharArray(L"选择需要识别的语言类型，以便达到最好的识别效果"));
            m_pNoiseTips->setFixedHeight(110);
            m_pNoiseTips->show();
            QPoint pos = this->mapToGlobal(ui.label_voiceTranslateTip->pos());
            m_pNoiseTips->move(pos + QPoint(0, 103));
         }
         //麦克风增益
         if (obj == ui.labMicGain && m_pNoiseTips != NULL /*&& m_pNoiseTips->isHidden()*/) {
            m_pNoiseTips->setText(QString::fromWCharArray(L"观众听到的麦克风声音较小时，可以调整该选项，增大声音，但同时也会放大噪音，建议同时开启降噪功能。"));
            m_pNoiseTips->setFixedHeight(125);
            m_pNoiseTips->show();
            QPoint pos = this->mapToGlobal(ui.labMicGain->pos());
            m_pNoiseTips->move(pos + QPoint(0, 60));
         }
         //高质量编码
         if (obj == ui.labHighQualityCod && m_pNoiseTips != NULL /*&& m_pNoiseTips->isHidden()*/) {
            m_pNoiseTips->setText(QString::fromWCharArray(L"0为关闭，1-9为开启，编码精度由低到高逐渐递增，画质会更加精细，同时会消耗更多的带宽。"));
            m_pNoiseTips->setFixedHeight(125);
            m_pNoiseTips->show();
            QPoint pos = this->mapToGlobal(ui.labHighQualityCod->pos());
            m_pNoiseTips->move(pos + QPoint(20, 230));
         }
         //麦克风降噪
         if (obj == ui.labelNoiseReductionTips && m_pNoiseTips != NULL /*&& m_pNoiseTips->isHidden()*/) {
            m_pNoiseTips->setText(QString::fromWCharArray(L"降噪功能，目前只适用于32000的音频采样率。"));
            m_pNoiseTips->setFixedHeight(110);
            m_pNoiseTips->show();
            QPoint pos = this->mapToGlobal(ui.labelNoiseReductionTips->pos());
            m_pNoiseTips->move(pos + QPoint(0, 60));
         }
         //开始直播后，自动开启录制功能
         if (obj == ui.labRecord && m_pNoiseTips != NULL /*&& m_pNoiseTips->isHidden()*/) {
            m_pNoiseTips->setText(QString::fromWCharArray(L"录制内容会在本地和SaaS平台同步生成FLV格式视频文件。 "));
            m_pNoiseTips->setFixedHeight(110);
            m_pNoiseTips->show();
            QPoint pos = this->mapToGlobal(ui.labRecord->pos());
            m_pNoiseTips->move(pos + QPoint(0, 60));
         }
         //视频质量评估
         if (obj == ui.label_videoQuality && m_pNoiseTips != NULL) {
            m_pNoiseTips->setText(QString::fromWCharArray(L"开启后，将基于电脑的性能动态调整视频画\n面质量，但会消耗额外计算资源，建议在\n高性能电脑上开启此功能。 "));
            m_pNoiseTips->setFixedHeight(120);
            m_pNoiseTips->show();
            QPoint pos = this->mapToGlobal(ui.label_videoQuality->pos());
            m_pNoiseTips->move(pos + QPoint(20, 270));
         }
         //桌面共享锐化
         if (obj == ui.label_desktop_hance && m_pNoiseTips != NULL) {
            m_pNoiseTips->setText(QString::fromWCharArray(L"使用桌面共享演示文档时，将对图片和文字进行锐化处理，改善观看体验，但会消耗额外计算资源，建议在高性能电脑上开启此功能。"));
            m_pNoiseTips->setFixedHeight(130);
            m_pNoiseTips->show();
            QPoint pos = this->mapToGlobal(ui.label_desktop_hance->pos());
            m_pNoiseTips->move(pos + QPoint(20, 160));
         }
      }
      if (ev->type() == QEvent::Leave) {
         if ((obj == ui.labNoiseValve || obj == ui.labKbps || obj == ui.labAudioSampleRate
            || obj == ui.label_voiceTranslateTip || obj == ui.labMicGain || obj == ui.labHighQualityCod
            || obj == ui.labelNoiseReductionTips || obj == ui.labRecord || obj == ui.label_videoQuality || obj == ui.label_desktop_hance)
            && m_pNoiseTips != NULL && !m_pNoiseTips->isHidden()) {
            m_pNoiseTips->close();
         }
      }
   }
   return false;
}

void SystemSettingDlg::OnSpeakerListChanged(int index) {
   if (index < 0) {
      return;
      ASSERT(FALSE);
   }
   CheckApplyStatus();
}

void SystemSettingDlg::OnDesktopEnhanceChanged(int index) {
   int checked = ConfigSetting::ReadInt(CPathManager::GetConfigPath(), GROUP_DEFAULT, DESKTOP_ENHANCE, 0);
   if (checked != ui.checkBox_desktopEnHance->checkState()) {
      m_ApplyIdentify |= e_page_ai;
   }
   else {
      m_ApplyIdentify &= ~e_page_ai;
   }
}

void SystemSettingDlg::OnBeautyValueChange(int value) {
   TRACE6("%s value %d",__FUNCTION__,value);
   int level = 0;
   //int beauty = ConfigSetting::writeValue(CPathManager::GetConfigPath(), GROUP_DEFAULT, CAMERA_BEAUTY, value);
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
   STRU_OBSCONTROL_ADDCAMERA  loAddCamera;
   loAddCamera.m_deviceInfo = mCurrentBeautyDev;
   loAddCamera.m_PosType = enum_PosType_auto;
   loAddCamera.m_dwType = device_operator_beauty;
   //loAddCamera.m_PriviewRenderHwnd = (HWND)ui.page_render->winId();
   loAddCamera.m_HideSourceItem = 0;

   CameraSettingUIResolution res;
   int nIndex = ui.comboBox_deviceResolution->currentIndex();
   m_resolutionList.GetFrameInfo(nIndex, &res);
   if (res.w > MAX_SUPPORT_W && res.h > MAX_SUPPORT_H) {
      ui.label_39->setText(QString::fromWCharArray(L"美颜仅支持848x480以下分辨率"));
      ui.widget_24->show();
      loAddCamera.m_beautyLevel = 0;
      disconnect(ui.horizontalSlider_Beauty, SIGNAL(valueChanged(int)), this, SLOT(OnBeautyValueChange(int)));
      ui.horizontalSlider_Beauty->setValue(0);
      connect(ui.horizontalSlider_Beauty, SIGNAL(valueChanged(int)), this, SLOT(OnBeautyValueChange(int)));

   }
   else {
      ui.widget_24->hide();
      loAddCamera.m_beautyLevel = level;
   }

   SingletonMainUIIns::Instance().PostCRMessage(MSG_OBSCONTROL_ADDCAMERA, &loAddCamera, sizeof(STRU_OBSCONTROL_ADDCAMERA));
   TRACE6("%s DealAddCameraSync end\n", __FUNCTION__);
   std::map<QString, int>::iterator iter = mDevBeautyLeavel.begin();
   while (iter != mDevBeautyLeavel.end()) {
      if (QString::fromStdWString(mCurrentBeautyDev.m_sDeviceID) == iter->first) {
         iter->second = value;
      }
      else {
         iter->second = 0;
      }
      iter++;
   }
}

void SystemSettingDlg::OnSoundChanelListChanged(int index) {
   if (index < 0) {
      return;
      ASSERT(FALSE);
   }

   CheckApplyStatus();

   VH::CComPtr<ICommonData> pCommonData;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
   pCommonData->ReportEvent("sz03yp04");
}
//帧率变化
void SystemSettingDlg::OnFrameChg(int nIndex) {
   if (nIndex < 0) {
      return;
      ASSERT(FALSE);
   }
   CheckApplyStatus();
   VH::CComPtr<ICommonData> pCommonData;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
   pCommonData->ReportEvent("sz02sxsb03");
}
//去交错变化
void SystemSettingDlg::OnDetainterChg(int nIndex) {
   if (nIndex < 0) {
      return;
      ASSERT(FALSE);
   }
   CheckApplyStatus();
   VH::CComPtr<ICommonData> pCommonData;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
   pCommonData->ReportEvent("sz02sxsb04");
}
//分辨率变化
void SystemSettingDlg::OnCommonResolutionChg(int nIndex) {
   if (nIndex < 0) {
      return;
      ASSERT(FALSE);
   }
   CheckApplyStatus();
   VH::CComPtr<ICommonData> pCommonData;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
   pCommonData->ReportEvent("sz01xtsz01");
}

void SystemSettingDlg::OnLinesChg(int nIndex) {
   if (nIndex < 0) {
      return;
      ASSERT(FALSE);
   }
   CheckApplyStatus();
   VH::CComPtr<ICommonData> pCommonData;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
   pCommonData->ReportEvent("sz01xtsz02");
}


void SystemSettingDlg::RefreshDeinterLace() {
   if (0 != ui.comboBox_deinterLace->count()) {
      return;
   }
   QStringList qsDeinterlacingList;
   qsDeinterlacingList << "无" << "丢弃画面场" << "回溯/双倍扫描" << "混合" << "混合2x" << "线性" << "线性2x" << "Yadif" << "Yadif2x";
   for (int i = 0; i < qsDeinterlacingList.size(); i++) {
      ui.comboBox_deinterLace->addItem(qsDeinterlacingList.at(i));
   }
}

void SystemSettingDlg::OnMaxLength() {
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

void SystemSettingDlg::SetVersion(QString qsVersion) {
   ui.label_version->setText("V " + qsVersion);
}

void SystemSettingDlg::OnDesktopCaptureTypeChagned(int state) {
   CheckApplyStatus();
}

void SystemSettingDlg::OnIsNoiseChecked(int state) {
   CheckApplyStatus();
   if (state == Qt::Checked) {
      TRACE6("%s enable noicegate\n", __FUNCTION__);
      EnableNoiceUI(true);
      ui.pushButton_noiseStatus->setStyleSheet("QPushButton#pushButton_noiseStatus{border-image: url(:/sysButton/img/sysButton/noiseEnable.png);}");
   }
   else {
      TRACE6("%s disable noicegate\n", __FUNCTION__);
      EnableNoiceUI(false);
      ui.pushButton_noiseStatus->setStyleSheet("QPushButton#pushButton_noiseStatus{border-image: url(:/sysButton/img/sysButton/noiseDisable.png);}");
   }

   VH::CComPtr<ICommonData> pCommonData;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
   pCommonData->ReportEvent("sz03yp03");
}

void SystemSettingDlg::On_recordFilesNum_stateChanged(int state)
{
   CheckApplyStatus();
}

void SystemSettingDlg::OnVideoQuality(int state) {
   if (state > 0) {
      QJsonObject body;
      SingletonMainUIIns::Instance().reportLog(STR_VH_LIVE_OPERATE, eLogRePortK_LIVE_videoQuality, L"videoQuality", body);
   }
   CheckApplyStatus();
}

void SystemSettingDlg::OnRecordCheck(int state) {
   VH::CComPtr<ICommonData> pCommonData;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
   if (Qt::Checked == state) {
      pCommonData->ReportEvent("sz04lz01");
   }
   else if (Qt::Unchecked == state) {
      pCommonData->ReportEvent("sz04lz02");
   }
   CheckApplyStatus();
}
void SystemSettingDlg::RefreshAskList() {
   ui.textBrowser_Content->clear();
   ui.textBrowser_Content->setPlaceholderText("感谢您的建议！\n\n请描述您在使用过程中出现的问题或困扰，以便我们优化我们的产品和服务。");
}

void SystemSettingDlg::OnServerPlaybackStatusChanged(int) {
   CheckApplyStatus();
}

void SystemSettingDlg::on_lineEdit_proxy_ip_textChanged(const QString &txt) {
   ui.btn_testing->setEnabled(txt.length() > 0);
}

//摄像头设置
void SystemSettingDlg::CameraSetting(DeviceInfo deviceInfo) {
   //Show(e_page_vedio);

   VH::CComPtr<IMainUILogic> pMainUILogic;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IMainUI, IID_IMainUILogic, pMainUILogic, return, ASSERT(FALSE));

   QWidget* pMainUI = NULL;
   pMainUILogic->GetMainUIWidget((void**)&pMainUI);
   if (NULL != pMainUI)
      Show(QApplication::desktop()->availableGeometry(pMainUI), e_page_vedio);
   else
      Show(QApplication::desktop()->availableGeometry(this), e_page_vedio);

   BindDevice(deviceInfo);
}


void SystemSettingDlg::ModifyLimitText(QString text) {
   bool bExist = false;
   for (int i = 0; i < sizeof(limitChar); i++) {
      if (-1 != text.indexOf(limitChar[i])) {
         text.remove(limitChar[i]);
         bExist = true;
      }
   }

   if (bExist) {
      ui.lineEdit_fileName->setText(text);
   }
}

void SystemSettingDlg::SetRecord(bool bSaveMode) {
   ui.recordCbx->setChecked(bSaveMode);
}

QString SystemSettingDlg::GetSaveFileName() {
   return ui.lineEdit_fileName->text().trimmed();
}
void SystemSettingDlg::SetSaveFileName(QString fileName)
{
   ui.lineEdit_fileName->setText(fileName);
}
void SystemSettingDlg::ClearReportContent() {
   ui.textBrowser_Content->clear();
}

void SystemSettingDlg::SetCopyrightInfo(QString qsAppName, QString qsCopyright) {
   ui.label_appName->setText(qsAppName);
   ui.label_copyright->setText(qsCopyright);
}

void SystemSettingDlg::EnabledExtraResolution(bool bEnable1080p) {

   //执行此部分将导致重复
   return;
}

void SystemSettingDlg::EnabledServerPlayBack(bool bEnable) {

}

void SystemSettingDlg::UIEnableProxy() {
   ui.btn_testing->setEnabled(true);
   ui.lineEdit_proxy_ip->setEnabled(true);
   ui.lineEdit_proxy_port->setEnabled(true);
   ui.lineEdit_proxy_userName->setEnabled(true);
   ui.lineEdit_proxy_password->setEnabled(true);
   ui.label_proxy_password->setStyleSheet("color:#D6D6D6;");
   ui.label_proxy_port->setStyleSheet("color:#D6D6D6;");
   ui.label_proxy_ip->setStyleSheet("color:#D6D6D6;");
   ui.label_proxy_userName->setStyleSheet("color:#D6D6D6;");
   ui.btn_testing->setEnabled(ui.lineEdit_proxy_ip->text().length() > 0);
}
void SystemSettingDlg::UIDisableProxy() {
   ui.btn_testing->setEnabled(false);
   ui.lineEdit_proxy_ip->setEnabled(false);
   ui.lineEdit_proxy_port->setEnabled(false);
   ui.lineEdit_proxy_userName->setEnabled(false);
   ui.lineEdit_proxy_password->setEnabled(false);
   ui.label_proxy_password->setStyleSheet("color:#7F7F7F;");
   ui.label_proxy_port->setStyleSheet("color:#7F7F7F;");
   ui.label_proxy_ip->setStyleSheet("color:#7F7F7F;");
   ui.label_proxy_userName->setStyleSheet("color:#7F7F7F;");
}
void SystemSettingDlg::on_comboBox_proxyEnable_currentIndexChanged(int index) {
   if (index == 0) {
      UIDisableProxy();
   }
   else {
      UIEnableProxy();
      ui.lineEdit_proxy_ip->clear();
      ui.lineEdit_proxy_port->setText("80");
      ui.lineEdit_proxy_userName->clear();
      ui.lineEdit_proxy_password->clear();
      if (index == ProxyConfig_Browser) {
         QString ip, userName, userPwd;
         int port;
         HttpProxyGetInfo::GetProxyInfoFromBrowser(ip, port, userName, userPwd);
         if (!ip.isEmpty()) {
            ui.lineEdit_proxy_ip->setText(ip);
            ui.lineEdit_proxy_port->setText(QString::number(port));
            ui.lineEdit_proxy_userName->setText(userName);
            ui.lineEdit_proxy_password->setText(userPwd);
         }
      }
   }
   CheckApplyStatus();
}

void SystemSettingDlg::on_btn_testing_clicked() {
   ui.label_tip->clear();

#define VersionHttpAPI QString("/api/vhallassistant/auth/test")
   QString qsConfPath = CPathManager::GetConfigPath();

   QString ip = ui.lineEdit_proxy_ip->text();
   unsigned short port = ui.lineEdit_proxy_port->text().toUShort();
   QString userName = ui.lineEdit_proxy_userName->text();
   QString password = ui.lineEdit_proxy_password->text();
   QString domain = ConfigSetting::ReadString(CPathManager::GetToolConfigPath(), GROUP_DEFAULT, KEY_URL_DOMAIN, domainUrl);
   QString url = domain + VersionHttpAPI;
   HttpRequest req;
   if (req.VHProxyTest(ip, port, userName, password, url)) {
      qDebug() << "VHProxyTest Successed" << ip << port << userName << password << url;
      ui.label_tip->setText(QString::fromWCharArray(L"该代理可使用"));
   }
   else {
      qDebug() << "VHProxyTest Failed" << ip << port << userName << password << url;
      ui.label_tip->setText(QString::fromWCharArray(L"无法连接到代理服务器"));
   }

}

QString SystemSettingDlg::GetProxyIP() {
   return ui.lineEdit_proxy_ip->text();
}

QString SystemSettingDlg::GetProxyPort() {
   return ui.lineEdit_proxy_port->text();
}

QString SystemSettingDlg::GetProxyUserName() {
   return ui.lineEdit_proxy_userName->text();
}

QString SystemSettingDlg::GetProxyPwd() {
   return ui.lineEdit_proxy_password->text();
}

bool SystemSettingDlg::GetIsEnableProxy() {
   bool currentBProxyOpen = ui.comboBox_proxyEnable->currentIndex() != 0;
   return currentBProxyOpen;
}

void SystemSettingDlg::EnableNoiceUI(bool enable) {
   ui.widget_noiseSetting->EnableNoise(enable);
}

void SystemSettingDlg::SaveNoiseSet() {
   QString qsConfPath = CPathManager::GetConfigPath();
   bool isNoise = ui.checkBox->isChecked();
   int noiseValue = ui.widget_noiseSetting->GetNoiseValue();
   ConfigSetting::writeValue(qsConfPath, GROUP_DEFAULT, KEY_AUDIO_NOISE_VALUE, noiseValue);
   ConfigSetting::writeValue(qsConfPath, GROUP_DEFAULT, KEY_AUDIO_ISNOISE, isNoise ? 1 : 0);
}

int SystemSettingDlg::GetThresHoldValue() {
   return ui.widget_noiseSetting->GetNoiseValue();
}

int SystemSettingDlg::GetKbps()
{
   return ui.cmbKbps->currentText().toInt();
}

int SystemSettingDlg::GetAudioSampleRate()
{
   return ui.cmbAudioSampleRate->currentText().toInt();
}

bool SystemSettingDlg::GetNoiseReduction()
{
   return ui.ckbNoiseReduction->isChecked();
}

float SystemSettingDlg::GetMicGain()
{
   return ui.micGainSld->value() / 100.0;
}

void SystemSettingDlg::on_cmbKbps_currentTextChanged(const QString & text)
{
   m_ApplyIdentify |= e_page_audio;
}

void SystemSettingDlg::on_cmbAudioSampleRate_currentTextChanged(const QString & text)
{
   m_ApplyIdentify |= e_page_audio;
}

void SystemSettingDlg::on_ckbNoiseReduction_stateChanged(int state)
{
   m_ApplyIdentify |= e_page_audio;
}

void SystemSettingDlg::on_btnReStore_clicked(bool checked /*= false*/)
{
   m_ApplyIdentify |= e_page_audio;
   ui.micVolumeSlider->setValue(ui.micVolumeSlider->maximum());
   ui.speakerVolumeSlider->setValue(ui.speakerVolumeSlider->minimum());//音 频 音 量 关
   ui.checkBox_Channel->setChecked(false);
   ui.cmbKbps->setCurrentText("96");
   ui.cmbAudioSampleRate->setCurrentText("44100");

   ui.micGainSld->setValue(16);//麦克风增益  定

   ui.checkBox->setChecked(false);
   ui.widget_noiseSetting->EnableNoise(false);
   ui.widget_noiseSetting->OnSetNoiseValue(0);//噪音阀值  最小
   ApplySettings();
}

int SystemSettingDlg::GetHighQualityCod()
{
   return ui.cmbHighQualityCod->currentText().toInt();
}

void SystemSettingDlg::on_cmbHighQualityCod_currentTextChanged(const QString & text)
{
   m_ApplyIdentify |= e_page_common;
}

QString SystemSettingDlg::GetVedioResolution()
{
   return ui.comboBox_deviceResolution->currentText();
}

QString SystemSettingDlg::GetFrame()
{
   return ui.comboBox_frame->currentText();
}

QString SystemSettingDlg::GetDeinterLace()
{
   return ui.comboBox_deinterLace->currentText();
}

QString SystemSettingDlg::GetQualityText()
{
   return ui.comboBox_resolution->currentText();
}

void SystemSettingDlg::OnVTLanguagenChg(int nIndex) {
   if (nIndex < 0) {
      return;
      ASSERT(FALSE);
   }

   CheckApplyStatus();
}

void SystemSettingDlg::OnVTFontSizeChg(int nIndex) {
   if (nIndex < 0) {
      return;
      ASSERT(FALSE);
   }

   CheckApplyStatus();
}

void SystemSettingDlg::OnVTReset() {
   m_ApplyIdentify |= e_page_voicetranslate;

   ui.comboBox_vt_fontSize->setCurrentText("15");
   ui.comboBox_vt_language->setCurrentText(QString::fromStdWString(TextLanguage_Mandarin));

   ApplySettings();
}

void SystemSettingDlg::SetSavedDir(const QString& strSavedDir) {
   m_qsSavedDir = strSavedDir;
   TRACE6("%s m_qsSavedDir:%s\n", __FUNCTION__, m_qsSavedDir.toStdString().c_str());
}

QString SystemSettingDlg::GetSavedDir() {
   TRACE6("%s m_qsSavedDir:%s\n", __FUNCTION__, m_qsSavedDir.toStdString().c_str());
   return m_qsSavedDir;
}

void SystemSettingDlg::SetCutRecordDisplay(const int iCutRecord)
{
   if (NULL != ui.pushButton_record)
   {
      if (eDispalyCutRecord_Show == iCutRecord) {
         ui.pushButton_record->show();
      }
      else {
         ui.pushButton_record->hide();
      }
   }
}

void SystemSettingDlg::SetDesktopShare(bool isCapture) {
   //桌面共享中不能选择采集方式
   isCapture == true ? ui.checkBox_desktopcapture->setEnabled(false) : ui.checkBox_desktopcapture->setEnabled(true);
   isCapture == true ? ui.label_desktopcapture_tips->setStyleSheet(DesktopCaptureTips_disable) : ui.label_desktopcapture_tips->setStyleSheet(DesktopCaptureTips_enable);
}

void SystemSettingDlg::SetLiveState(bool isLiveing) {
   if (!isLiveing) {
      ui.checkBox_desktopcapture->setCheckState(Qt::CheckState::Unchecked);
      ConfigSetting::writeValue(CPathManager::GetConfigPath(), GROUP_DEFAULT, KEY_DESKCTOP_CAPTURE_TYPE, 0);
   }
}
