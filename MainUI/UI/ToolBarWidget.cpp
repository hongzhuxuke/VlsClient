#include "stdafx.h"
#include <QSignalMapper>
#include <QDebug>
#include <QToolTip>
#include <QHBoxLayout>
#include "ConfigSetting.h"
#include "ToolButton.h"
#include "ToolBarWidget.h"
#include "volumedisplay_wiget.h"
#include "TimerClock.h"
#include "HoverLabel.h"

#include "VhallLiveMainDlg.h"

#include "Msg_MainUI.h"

#include "ICommonData.h"
#include "IOBSControlLogic.h"
#include "ISettingLogic.h"
#include "AudioSliderUI.h"
#include "Msg_OBSControl.h"
#include "dshowcapture.hpp"
#include "vhallwaiting.h"
#include "pub.Const.h"
#include "pub.Struct.h"
#include "pathManage.h"
#include "pathmanager.h"
#include "VhallLiveMainDlg.h"
#include <psapi.h>
#include "ButtonModTextWdg.h"

#define FONT_COLOR_GREEN         "#28e56b"
#define BORDERCOLOR "rgba(45, 45, 45, 1);"

#define STRNORMAL  "color: #DADADA;"
#define  STRHOVER   "color: #FC5659;"

#define PlayFileNormal "border-image:url(:/button/images/play_file_title.png);"
#define PlayFileClicked "border-image:url(:/button/images/play_file_title_hover.png);"

#define InsertTextNormal "border-image:url(:/button/images/insert_text.png);"
#define InsertTextClicked "border-image:url(:/button/images/insert_text_hover.png);"

#define CameraNormal "border-image:url(:/button/images/camera.png);"
#define CameraClicked "border-image:url(:/button/images/camera_hover.png);"

#define InsertImageNormal "border-image:url(:/button/images/insert_image.png);"
#define InsertImageClicked "border-image:url(:/button/images/insert_image_hover.png);"

#define ArrowImageNormal "border-image:url(:/button/images/dow_arrow.png);"
#define ArrowImageClicked "border-image:url(:/button/images/dow_arrow_hover.png);"


ToolBarWidget::ToolBarWidget(QWidget *parent)
: CWidget(parent)
, m_pStreamStatus(NULL)
, m_iRtmpStreamCount(-1)
, mVoiceTimer(NULL){
   setFixedHeight(56);
   QPalette palette;
   palette.setColor(QPalette::Background, QColor(28, 28, 28));
   setPalette(palette);

   setAutoFillBackground(true);
   m_pMicValueTimer = new QTimer(this);
   if (m_pMicValueTimer) {
	   connect(m_pMicValueTimer, SIGNAL(timeout()), this, SLOT(Slot_RecordMicValue()));
   }

   initPreData();

}

ToolBarWidget::~ToolBarWidget() {
	if (m_pMicValueTimer) {
		m_pMicValueTimer->stop();
		disconnect(m_pMicValueTimer, SIGNAL(timeout()), this, SLOT(Slot_RecordMicValue()));
	}
}

bool ToolBarWidget::Create() {
#define NEWPADDING(X) paddingWidget = new QWidget(this);paddingWidget->setMaximumWidth(X);paddingWidget->setMinimumWidth(X);m_pMainLayout->addWidget(paddingWidget);
#define PADDINGEXP() paddingWidget = new QWidget(this);paddingWidget->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);m_pMainLayout->addWidget(paddingWidget);
   QWidget *paddingWidget = NULL;
   QSpacerItem* pItem = NULL;
   m_pMainLayout = new QHBoxLayout(this);
   m_pMainLayout->setContentsMargins(0, 0, 0, 0);
   m_pMainLayout->setSpacing(0);
   NEWPADDING(10);

   m_pCameraBtnMTW = new ButtonModTextWdg(this);
   m_pCameraBtnMTW->Create(8, 15, 4, 12);
   m_pCameraBtnMTW->SetFrontIconImage(CameraNormal, CameraClicked, CameraNormal);
   m_pCameraBtnMTW->SetBackIconImage(ArrowImageNormal, ArrowImageClicked, ArrowImageNormal);
   m_pCameraBtnMTW->SetFrontIconFixedSize(18,15);
   m_pCameraBtnMTW->SetBackIconFixedSize(6, 3);
   m_pCameraBtnMTW->setIconAlignment(Qt::AlignVCenter | Qt::AlignRight);
   m_pCameraBtnMTW->setFontText(tr("摄像设备"));
   m_pCameraBtnMTW->setBackIconText(QChar(0xe71f));//后边下拉箭头
   connect(m_pCameraBtnMTW, &ButtonModTextWdg::btnClicked, this, &ToolBarWidget::SlotCameraClicked);
   m_pMainLayout->addWidget(m_pCameraBtnMTW);
   NEWPADDING(20);

   mpInsertVedio = new ButtonWdg(this);
   mpInsertVedio->Create(ButtonWdg::eFontDirection_Right, 5, 12, 15, PlayFileNormal, PlayFileClicked, PlayFileNormal);
   mpInsertVedio->setIconTextSize(18,15);
   mpInsertVedio->setBorderColor(BORDERCOLOR, BORDERCOLOR);
   mpInsertVedio->setFontText(tr("插播文件"));
   mpInsertVedio->setIconAlignment(Qt::AlignVCenter | Qt::AlignRight);
   connect(mpInsertVedio, &ButtonWdg::btnClicked, this, &ToolBarWidget::OnVedioPlayClick);
   m_pMainLayout->addWidget(mpInsertVedio);
   NEWPADDING(20);

   mpInsertImage = new ButtonWdg(this);
   mpInsertImage->Create(ButtonWdg::eFontDirection_Right, 5, 12, 15, InsertImageNormal, InsertImageClicked, InsertImageNormal);
   mpInsertImage->setIconTextSize(18, 15);
   mpInsertImage->setBorderColor(BORDERCOLOR, BORDERCOLOR);
   mpInsertImage->setFontText(tr("插入图片"));
   mpInsertImage->setIconAlignment(Qt::AlignVCenter | Qt::AlignRight);
   connect(mpInsertImage, &ButtonWdg::btnClicked, this, &ToolBarWidget::OnAddImageClick);
   m_pMainLayout->addWidget(mpInsertImage);
   NEWPADDING(20);

   mpInsertText = new ButtonWdg(this);
   mpInsertText->Create(ButtonWdg::eFontDirection_Right, 5, 12, 15, InsertTextNormal, InsertTextClicked, InsertTextNormal);
   mpInsertText->setIconTextSize(18, 15);
   mpInsertText->setBorderColor(BORDERCOLOR, BORDERCOLOR);
   mpInsertText->setFontText(tr("插入文字"));
   mpInsertText->setIconAlignment(Qt::AlignVCenter | Qt::AlignRight);
   connect(mpInsertText, &ButtonWdg::btnClicked, this, &ToolBarWidget::OnAddTextClick);
   m_pMainLayout->addWidget(mpInsertText);
   NEWPADDING(153);

   pItem = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
   m_pMainLayout->addItem(pItem);
   mListSpacerItem.append(pItem);
   
   m_pCameraBtnMTW->setIconSheetStyle(STRNORMAL, STRHOVER);
   mpInsertVedio->setIconSheetStyle(STRNORMAL, STRHOVER);
   mpInsertImage->setIconSheetStyle(STRNORMAL, STRHOVER);
   mpInsertText->setIconSheetStyle(STRNORMAL, STRHOVER);


   PADDINGEXP();
   pItem = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
   m_pMainLayout->addItem(pItem);
   mListSpacerItem.append(pItem);
   m_pMainLayout->addStretch();
   NEWPADDING(100);

   //实时字幕
   mpVoiceTranslateLabel = new  QLabel(tr("实时字幕"), this);
   m_pMainLayout->addWidget(mpVoiceTranslateLabel);
   NEWPADDING(5);
   mpVoiceTranslate = new QToolButton(/*":/CtrlBtn/img/CtrlButton/openVoiceTranslate.png",*/ this);
   mpVoiceTranslate->setObjectName("voiceTranslate");
   mpVoiceTranslate->setFixedSize(38, 20);
   mpVoiceTranslate->setCheckable(true);
   mpVoiceTranslate->setChecked(false);
   connect(mpVoiceTranslate, &QToolButton::clicked, this, &ToolBarWidget::OnVoiceTranslateClick);
   m_pMainLayout->addWidget(mpVoiceTranslate);
   mpVoiceTranslate->hide();
   mpVoiceTranslateLabel->hide();
   NEWPADDING(20);


   //麦克风 
   mpMicLabel = new QLabel(tr("麦克风"), this);
   m_pMainLayout->addWidget(mpMicLabel);
   NEWPADDING(5);
   m_pMicButton = new QToolButton(/*":/CtrlBtn/img/CtrlButton/mic_o.png",*/ this);
   m_pMicButton->setObjectName("micButton");
   m_pMicButton->setCheckable(true);
   SetMicVolumn(100);
   m_pMicButton->setFixedSize(38, 20);
   connect(m_pMicButton, &QToolButton::clicked, this, &ToolBarWidget::micBtnClicked);
   //connect(m_pMicButton, &QToolButton::sigEnter, this, &ToolBarWidget::micBtnEnter);
   m_pMainLayout->addWidget(m_pMicButton);
   NEWPADDING(10);

   mpVoiceTranslateLabel->setStyleSheet(STRNORMAL);
   mpMicLabel->setStyleSheet(STRNORMAL);

   //音量定时器
   mVoiceTimer = new QTimer(this);
   //if (NULL == m_pTimerClock) {
   //   ASSERT(FALSE);
   //   return false;
   //}
   //connect(mVoiceTimer, SIGNAL(timeout()), this, SLOT(updateVolumeDisplay()));
   //connect(mVoiceTimer, SIGNAL(timeout()), this, SLOT(logCPUAndMemory()));

   VH::CComPtr<IMainUILogic> pMainUILogic;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IMainUI, IID_IMainUILogic, pMainUILogic, return FALSE, ASSERT(FALSE));
   QWidget* pMainUI = NULL;
   pMainUILogic->GetMainUIWidget((void**)&pMainUI);

   m_pStreamStatus = new QLabel(pMainUI);
   m_pStreamStatus->setObjectName("streamStatus");
   m_pStreamStatus->setWordWrap(true);
   m_pStreamStatus->hide();
   m_pStreamStatus->setContentsMargins(6, 6, 6, 6);
   m_pMainLayout->addWidget(m_pStreamStatus);

   setLayout(m_pMainLayout);

      //麦克风音量条
	//m_pMicSliderUI = new AudioSliderUI((QDialog *)this);
	//if(NULL == m_pMicSliderUI || !m_pMicSliderUI->Create()) {
	//   ASSERT(FALSE);
	//   return false;   
	//}
	//connect(m_pMicSliderUI, SIGNAL(sigVolumnChanged(int)), this, SLOT(SetMicVolumn(int)));

   m_pSpeakerSliderUI = new AudioSliderUI((QDialog *)this);
   if (NULL == m_pSpeakerSliderUI || !m_pSpeakerSliderUI->Create()) {
      ASSERT(FALSE);
      return false;
   }
   connect(m_pSpeakerSliderUI, SIGNAL(sigVolumnChanged(int)), this, SLOT(SetSpeakerVolumn(int)));
   SetSpeakerMute(false);

   return true;
}

void ToolBarWidget::OnVedioPlayClick() {
	VH::CComPtr<IOBSControlLogic> pObsControlLogic = NULL;
	DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IOBSControl, IID_IOBSControlLogic, pObsControlLogic, return, ASSERT(FALSE));
	bool isHasMonitor = pObsControlLogic->IsHasSource(SRC_MONITOR);
	
	if (m_pLiveMainDlg&&isHasMonitor) {
		m_pLiveMainDlg->FadeOutTip(SHAREING_ADDSOURCE, TipsType_Error);
		return;
	}

	QJsonObject body;
	SingletonMainUIIns::Instance().reportLog(STR_VH_LIVE_OPERATE, eLogRePortK_MultiMedia_InsertVedio, L"InsertVedio", body);

	STRU_MAINUI_CLICK_CONTROL loControl;
	loControl.m_eType = (enum_control_type)(control_VideoSrc);
	loControl.m_dwExtraData = eLiveType_Live;
	SingletonMainUIIns::Instance().PostCRMessage(
		MSG_MAINUI_CLICK_CONTROL, &loControl, sizeof(STRU_MAINUI_CLICK_CONTROL));
	TRACE6("%s \n", __FUNCTION__);
}

void ToolBarWidget::OnAddImageClick() {
	VH::CComPtr<IOBSControlLogic> pObsControlLogic = NULL;
	DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IOBSControl, IID_IOBSControlLogic, pObsControlLogic, return, ASSERT(FALSE));
	bool isHasMonitor = pObsControlLogic->IsHasSource(SRC_MONITOR);

	if (m_pLiveMainDlg&&isHasMonitor) {
		m_pLiveMainDlg->FadeOutTip(SHAREING_ADDSOURCE, TipsType_Error);
		return;
	}

	QJsonObject body;
	SingletonMainUIIns::Instance().reportLog(STR_VH_LIVE_OPERATE, eLogRePortK_MultiMedia_InsertImage, L"InsertImage", body);

	STRU_MAINUI_CLICK_CONTROL loControl;
	loControl.m_eType = (enum_control_type)(control_AddImage);
	loControl.m_dwExtraData = 1;            //1添加 2修改
	SingletonMainUIIns::Instance().PostCRMessage(
		MSG_MAINUI_CLICK_CONTROL, &loControl, sizeof(STRU_MAINUI_CLICK_CONTROL));
	TRACE6("%s \n", __FUNCTION__);
}

void ToolBarWidget::OnAddTextClick() {
	VH::CComPtr<IOBSControlLogic> pObsControlLogic = NULL;
	DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IOBSControl, IID_IOBSControlLogic, pObsControlLogic, return, ASSERT(FALSE));
	bool isHasMonitor = pObsControlLogic->IsHasSource(SRC_MONITOR);

	if (m_pLiveMainDlg&&isHasMonitor) {
		m_pLiveMainDlg->FadeOutTip(SHAREING_ADDSOURCE, TipsType_Error);
		return;
	}
	QJsonObject body;
	SingletonMainUIIns::Instance().reportLog(STR_VH_LIVE_OPERATE, eLogRePortK_MultiMedia_InsertText, L"InsertText", body);

	STRU_MAINUI_CLICK_CONTROL loControl;
	loControl.m_eType = (enum_control_type)(control_AddText);
	SingletonMainUIIns::Instance().PostCRMessage(
		MSG_MAINUI_CLICK_CONTROL, &loControl, sizeof(STRU_MAINUI_CLICK_CONTROL));
	TRACE6("%s \n", __FUNCTION__);
}

void ToolBarWidget::OnVoiceTranslateClick() {
	VH::CComPtr<IMainUILogic> pMainUILogic;
	DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IMainUI, IID_IMainUILogic, pMainUILogic, return, ASSERT(FALSE));
	if (pMainUILogic && pMainUILogic->GetVoiceTranslate() && pMainUILogic->IsOpenVoiceTranslateFunc()) {
		pMainUILogic->OpenVoiceTranslateFun(false);
		//mpVoiceTranslate->changeImage(":/CtrlBtn/img/CtrlButton/openVoiceTranslate.png");
		mpVoiceTranslate->setChecked(false);
	}
	else if (pMainUILogic && pMainUILogic->GetVoiceTranslate() && !pMainUILogic->IsOpenVoiceTranslateFunc()) {
		//mpVoiceTranslate->changeImage(":/CtrlBtn/img/CtrlButton/closeVoiceTranslate.png");
		mpVoiceTranslate->setChecked(true);
		pMainUILogic->OpenVoiceTranslateFun(true);
	}

	STRU_MAINUI_CLICK_CONTROL loControl;
	loControl.m_eType = (enum_control_type)(control_VoiceTranslate);
	SingletonMainUIIns::Instance().PostCRMessage(MSG_MAINUI_CLICK_CONTROL, &loControl, sizeof(STRU_MAINUI_CLICK_CONTROL));
}

void ToolBarWidget::Destroy() {
	//if (m_pMicSliderUI) {
	//	delete m_pMicSliderUI;
	//	m_pMicSliderUI = NULL;
	//}

   if (m_pSpeakerSliderUI) {
      delete m_pSpeakerSliderUI;
      m_pSpeakerSliderUI = NULL;
   }

   if (mVoiceTimer) {
      mVoiceTimer->stop();
   }
}

void ToolBarWidget::SetStartLiveState(bool start) {
   if (start) {
      mVoiceTimer->start(150);
   }
   else {
      mVoiceTimer->stop();
   }
}

//void ToolBarWidget::UpdateStreamSpeedUI(QString qStrStreamState, int stageValue) {
//   if (NULL == m_pLiveSignalStatus) {
//      ASSERT(FALSE);
//      return;
//   }
//   m_pLiveSignalStatus->updateDisplay(stageValue);
//   m_pLiveSignalStatus->update();
//
//   if (m_pLiveSignalStatus->mbEnter) {
//      QPoint tipPos = m_pLiveSignalStatus->mapToGlobal(QPoint(0, -44));
//      QToolTip::showText(tipPos, qStrStreamState);
//   }
//}

float DBtoLog(float db) {
   /* logarithmic scale for audio meter */
   return -log10(0.0f - (db - 6.0f));
}

#define VOL_MIN -96
#define VOL_MAX 0
#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif
#ifndef min
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif
float minLinear = DBtoLog(VOL_MIN);
float maxLinear = DBtoLog(VOL_MAX);
static int index = 0;

void ToolBarWidget::logCPUAndMemory(int cpu) {

   index++;
   if (index % 30 != 0) {
      return;
   }
   index = 0;
   double cpuf = (double)(cpu);//CalCpuUtilizationRate();
   int nMemTotal = 0;
   int nMemUsed = 0;

   GetSysMemory(nMemTotal, nMemUsed);

   HANDLE handle = GetCurrentProcess();
   PROCESS_MEMORY_COUNTERS pmc;
   GetProcessMemoryInfo(handle, &pmc, sizeof(pmc));

   int currentMemory = pmc.PagefileUsage / 1024 / 1024;
   //DShowLog(DShowLogType_Level2_ALL, DShowLogLevel_Info, L"CPU[%f] MEMORY[%d][%d][%d]\n", cpuf
   //         , nMemTotal, nMemUsed, currentMemory);

   static int logIndex = 0;
   if (logIndex % 3 == 0 && cpuf >= 0.0 && cpuf <= 100.0) {
      logIndex = 0;
      STRU_MAINUI_LOG log;
      swprintf_s(log.m_wzRequestUrl, DEF_MAX_HTTP_URL_LEN, L"key=basic_info&cpu=%f&mem_phy=%d&mem_cur=%d&mem_vls=%d&k=%d",
                 cpuf,//Cpu使用率
                 nMemTotal,//内存总量
                 nMemUsed,//内存使用量
                 currentMemory,//当前分页项目
                 eLogRePortK_BasicInfo);
      QJsonObject body;
      body["cu"] = cpuf;   	//Cpu使用率
      body["mus"] = nMemTotal;	//内存总量
      body["muu"] = nMemUsed;	//内存使用量
      body["crm"] = nMemUsed;	//当前分页项目
      QString json = CPathManager::GetStringFromJsonObject(body);
      strcpy_s(log.m_wzRequestJson, json.mid(0, DEF_MAX_HTTP_URL_LEN).toLatin1().data());
      SingletonMainUIIns::Instance().PostCRMessage(MSG_MAINUI_LOG, &log, sizeof(STRU_MAINUI_LOG));
   }
   logIndex++;
}

void ToolBarWidget::updateVolumeDisplay() {
   ////实时更新音量图标
   //float audioMag = 0.0f;
   //float audioPeak = 0.0f;
   //float audioMax = 0.0f;

   //float workVol, workMax, rmsScale, maxScale, peakScale;
   //float curTopVolume;

   //VH::CComPtr<IOBSControlLogic> pOBSControlLogic;
   //DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IOBSControl, IID_IOBSControlLogic, pOBSControlLogic, return);
   //if (pOBSControlLogic) {
   //   pOBSControlLogic->GetAudioMeter(audioMag, audioPeak, audioMax);
   //   curTopVolume = audioMag;
   //   workVol = min(VOL_MAX, max(VOL_MIN, curTopVolume)); // Bound volume levels
   //   workMax = min(VOL_MAX, max(VOL_MIN, audioMax));
   //   rmsScale = (DBtoLog(workVol) - minLinear) / (maxLinear - minLinear); // Convert dB to logarithmic then to linear scale [0, 1]
   //   maxScale = (DBtoLog(workMax) - minLinear) / (maxLinear - minLinear);
   //   peakScale = (DBtoLog(audioPeak) - minLinear) / (maxLinear - minLinear);
   //}

   //VH::CComPtr<IMainUILogic> pMainUILogic;
   //DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IMainUI, IID_IMainUILogic, pMainUILogic, return, ASSERT(FALSE));
   //pMainUILogic->UpdateVolumn(rmsScale * 100);

   //UpdateVolumn(rmsScale * 100);
}

void ToolBarWidget::SetSpeakerVolumn(int v) {
   VH::CComPtr<IOBSControlLogic> pOBSControlLogic;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IOBSControl, IID_IOBSControlLogic, pOBSControlLogic, return, ASSERT(FALSE));
   float value = v;
   value /= 100.0f;
   pOBSControlLogic->SetSpekerVolumn(value);
   SetSpeakerMute(value == 0);

   QJsonObject body;
   body["dv"] = value;
   SingletonMainUIIns::Instance().reportLog(STR_VH_LIVE_OPERATE, eLogRePortK_ChangeMicNVal, L"ChangeMicNVal", body);

}

void ToolBarWidget::speakerBtnEnter() {
   m_bSpeakerShow = !m_pSpeakerSliderUI->isHidden();
}

void ToolBarWidget::speakerBtnClicked() {
}



void ToolBarWidget::liveToolClicked() {
   STRU_MAINUI_CLICK_CONTROL loControl;
   loControl.m_eType = control_LiveTool;
   SingletonMainUIIns::Instance().PostCRMessage(
      MSG_MAINUI_CLICK_CONTROL, &loControl, sizeof(STRU_MAINUI_CLICK_CONTROL));

   QJsonObject body;
   SingletonMainUIIns::Instance().reportLog(STR_VH_LIVE_OPERATE, eLogRePortK_LivePlugInUnit, L"PlugInUnit", body);
   emit sigClicked();
}


void ToolBarWidget::SlotCameraClicked() {
   STRU_MAINUI_CLICK_CONTROL loControl;
   loControl.m_eType = control_AddCamera;
   QPoint globalPoint = m_pCameraBtnMTW->mapToGlobal(QPoint(0, 0));
   loControl.m_globalX = globalPoint.x();
   loControl.m_globalY = globalPoint.y() + m_pCameraBtnMTW->height();
   loControl.m_dwExtraData = 1;
   SingletonMainUIIns::Instance().PostCRMessage(MSG_MAINUI_CLICK_CONTROL, &loControl, sizeof(STRU_MAINUI_CLICK_CONTROL));
   emit sigClicked();
   emit sigClickedCamera();
}

void ToolBarWidget::showStreamStatus() {
   VH::CComPtr<ICommonData> pCommonData;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));

   //没有推流 或 没有信息
   if (!pCommonData->GetPublishState() || -1 == m_iRtmpStreamCount) {
      return;
   }

   if (m_pStreamStatus) {
      m_pStreamStatus->show();
   }
}

void ToolBarWidget::hideStreamStatus() {
   if (m_pStreamStatus) {
      m_pStreamStatus->hide();
   }
}

void ToolBarWidget::UpdateStreamStateUI(QString qStrStreamState, int iStreamCnt) {
   m_pStreamStatus->setText(qStrStreamState);
   if (iStreamCnt != m_iRtmpStreamCount) {
      m_iRtmpStreamCount = iStreamCnt;
      int weight = 150;
      int height = 150 * m_iRtmpStreamCount;
      m_pStreamStatus->resize(weight, height);
      int x = this->width() - weight;
      int y = ((QWidget*)(this->parent()))->height() - height - this->height();
      m_pStreamStatus->move(x, y);
   }
}



void ToolBarWidget::SetSpeakerMute(bool bMute) {
}

//void ToolBarWidget::cameraBtnEnter() {
//   VH::CComPtr<ISettingLogic> pSettingLogic;
//   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IMainUI, IID_ISettingLogic, pSettingLogic, return, ASSERT(FALSE));
//   m_bCameraShow = pSettingLogic->IsCameraShow();
//}

void ToolBarWidget::multiMediaEnter() {
  // m_bMultiMediaShow = ((VhallLiveMainDlg*)parent())->IsMultiMediaShow();
}

//void ToolBarWidget::desktopShareEnter() {
//  // m_bDesktopShow = ((VhallLiveMainDlg*)parent())->IsDesktopShow();
//}
//void ToolBarWidget::ShowLiveTool(bool ok) {
//   if (m_pLiveTool) {
//      if (ok) {
//         m_pLiveTool->show();
//      } else {
//         m_pLiveTool->hide();
//      }
//   }
//}

//void ToolBarWidget::ShowMemberListBtn(bool show) {
//   if (show) {
//      if (m_pbtnMemberList) {
//         m_pbtnMemberList->show();
//      }
//   }
//   else {
//      if (m_pbtnMemberList) {
//         m_pbtnMemberList->hide();
//      }
//   }
//}



void ToolBarWidget::Slot_RecordMicValue() {
   m_bRecordMicValue = true;
}


//void ToolBarWidget::SetMemberListShow(bool bShow /*= true*/)
//{
//	if (bShow && NULL!=m_pbtnMemberList)
//	{
//		m_pbtnMemberList->show();
//	}
//	else if(NULL != m_pbtnMemberList)
//	{
//		m_pbtnMemberList->hide();
//	}
//}


void ToolBarWidget::SetVoiceTranslateBtnState(bool open) {
   if (open) {
      mpVoiceTranslate->setChecked(true);
   }
   else {
      mpVoiceTranslate->setChecked(false);
   }
}

void ToolBarWidget::AddVoiceTranslate() {
	if (mpVoiceTranslate != NULL) {
		//mpVoiceTranslate->setChecked(true);
		//mpVoiceTranslate->changeImage(":/CtrlBtn/img/CtrlButton/openVoiceTranslate.png");
		mpVoiceTranslate->show();
		mpVoiceTranslateLabel->show();
	}
}

void ToolBarWidget::RemoveVoiceTranslate() {
	if (mpVoiceTranslate != NULL) {
		//mpVoiceTranslate->changeImage(":/CtrlBtn/img/CtrlButton/openVoiceTranslate.png");
		//mpVoiceTranslate->setChecked(true);
		mpVoiceTranslate->hide();
		mpVoiceTranslateLabel->hide();
	}
}

void ToolBarWidget::SetStreamButtonStatus(bool isActive)
{
	if (isActive) {
		m_bRecordMicValue = true;
	}
	else if (m_pMicValueTimer) {
		m_pMicValueTimer->stop();
		m_bRecordMicValue = false;
	}
}

void ToolBarWidget::micBtnEnter() {
	//m_bMicShow = !m_pMicSliderUI->isHidden();
}

void ToolBarWidget::micBtnClicked() {
	VH::CComPtr<ICommonData> pCommonData;
	DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
	pCommonData->ReportEvent("mkf00");

	VH::CComPtr<IOBSControlLogic> pOBSControlLogic;
	DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IOBSControl, IID_IOBSControlLogic, pOBSControlLogic, return, ASSERT(FALSE));
	int value = 100 * pOBSControlLogic->GetMicVolunm();
	STRU_MAINUI_MUTE loMute;
	loMute.m_eType = mute_Mic;
	eLogRePortK iMicOpen = eLogRePortK_MicOpen;

	if (value == 0) {
		SetMicVolumn(iOldMicVal);
		loMute.m_bMute = false;
	}
	else
	{
		iOldMicVal = value;
		SetMicVolumn(0);
		loMute.m_bMute = true;
		iMicOpen = eLogRePortK_MicMute;
	}
	SetMicMute(loMute.m_bMute);
	SingletonMainUIIns::Instance().PostCRMessage(
		MSG_MAINUI_MUTE, &loMute, sizeof(loMute));
	QJsonObject body;
	SingletonMainUIIns::Instance().reportLog(STR_VH_LIVE_OPERATE, iMicOpen, L"MicOpen", body);
	TRACE6("%s mic value:%d\n", __FUNCTION__, value);

	emit sigClicked();
}

void ToolBarWidget::SetMicMute(bool bMute) {
	//if (!bMute) {
	//	m_pMicButton->setChecked(true);
	//	//m_pMicButton->changeImage(":/CtrlBtn/img/CtrlButton/mic_0.png");
	//}
	//else {
	//	m_pMicButton->setChecked(false);
	//	//m_pMicButton->changeImage(":/CtrlBtn/img/CtrlButton/mic_close.png");
	//}

	m_pMicButton->setChecked(!bMute);
	emit sigMicStateChange(!bMute);
	m_bMicMute = bMute;
	m_pMicButton->repaint();
}

void ToolBarWidget::UpdateVolumn(int v /*StartButton* pbtn (m_pStartLiveBtn)*/) {

	if (m_bMicMute) return;
	if (m_bRecordMicValue) {
		TRACE6("%s mic value:%d\n", __FUNCTION__, v);
		m_bRecordMicValue = false;
	}
	
	m_pMicButton->repaint();
}

void ToolBarWidget::SetMicVolumn(int v) {
	VH::CComPtr<IOBSControlLogic> pOBSControlLogic;
	DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IOBSControl, IID_IOBSControlLogic, pOBSControlLogic, return, ASSERT(FALSE));
	float value = v;
	value /= 100.0f;
	pOBSControlLogic->SetMicVolunm(value);
	SetMicMute(value == 0);
	m_pMicButton->setChecked(value>0);


	if (m_pLiveMainDlg) {
		if(value > 0)
			m_pLiveMainDlg->FadeOutTip(MIC_IS_OPENED, TipsType_Success);
		else
			m_pLiveMainDlg->FadeOutTip(MIC_IS_CLOSED, TipsType_Success);
	}

	emit sigMicStateChange(value > 0);
}