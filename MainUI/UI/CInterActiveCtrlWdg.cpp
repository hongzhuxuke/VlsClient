#include "CInterActiveCtrlWdg.h"
#include "Msg_MainUI.h"
#include "MainUIIns.h"
#include <ICommonData.h >
#include "VH_Macro.h"
#include "CRPluginDef.h"
#include "ConfigSetting.h"

//#define TO_SPEAK "QPushButton{border-image:url(:/interactivity/img/interactivity/reply_yes.png);}\
//QPushButton:hover{border-image:url(:/interactivity/img/interactivity/reply_yes_hover.png);}\
//QPushButton:pressed{border-image:url(:/interactivity/img/interactivity/reply_yes.png);}\
//QPushButton:disabled{border-image: url(:/interactivity/img/interactivity/reply_yes_unenable.png);}"
//
//#define OFF_SPEAK "QPushButton{border-image:url(:/interactivity/img/interactivity/leave_speak.png);}\
//QPushButton:hover{border-image:url(:/interactivity/img/interactivity/leave_speak_hover.png);}\
//QPushButton:pressed{border-image:url(:/interactivity/img/interactivity/leave_speak.png);}\
//QPushButton:disabled{border-image: url(:/interactivity/img/interactivity/leave_speak_unenable.png);}"


CInterActiveCtrlWdg::CInterActiveCtrlWdg(QWidget *parent)
: QWidget(parent)
, mRecordState(eRecordState_Stop){
   ui.setupUi(this);
   this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint | Qt::Tool | Qt::WindowStaysOnTopHint);
   InitBtnState(true);
   InitConnect();
   DisMouseTracking(); 

   mpStartLiveBtnTimer = new QTimer(this);
   if (mpStartLiveBtnTimer) {
       mpStartLiveBtnTimer->setSingleShot(true);
       connect(mpStartLiveBtnTimer, SIGNAL(timeout()), this, SLOT(slot_OnStartLiveTimeOut()));
   }

}

CInterActiveCtrlWdg::~CInterActiveCtrlWdg() {

}

void CInterActiveCtrlWdg::InitBtnState(bool bHost) {
	if (bHost) {
		ui.pushButton_startLive->show();
	} else {
		ui.pushButton_startLive->hide();
	}
	ui.pushButton_Camera->changeImage(":/CtrlBtn/img/CtrlButton/camera_open.png");
	ui.pushButton_mic->changeImage(":/CtrlBtn/img/CtrlButton/mic_0.png");
	ui.pushButton_player->changeImage(":/CtrlBtn/img/CtrlButton/player_open.png");
	//ui.pushButton_screenShare->changeImage(":/CtrlBtn/img/CtrlButton/desktop_share.png");
	ui.pushButton_startLive->changeImage(":/toolWidget/startLive");
	ui.pushButton_cameraList->changeImage(":/CtrlBtn/img/CtrlButton/arrow.png");
	ui.pushButton_micList->changeImage(":/CtrlBtn/img/CtrlButton/arrow.png");
	ui.pushButton_playerList->changeImage(":/CtrlBtn/img/CtrlButton/arrow.png");
	//ui.pushButton_plugin->changeImage(":/CtrlBtn/img/CtrlButton/live_plugin.png");
	//mCurLiveState = false;
	//ui.pushButton_startLive->setToolTip(QString::fromWCharArray(L"开始直播"));
	//ui.pushButton_startLive->setStyleSheet(TOOLTIPQSS);//"QToolTip{color:rgb(128,128,128);}"
	//ui.label->setText("00:00:00");
	//ui.pushButton_LeaveActive->setToolTip(QString::fromWCharArray(L"退出直播"));
	ui.btnRecord->changeImage(":/CtrlBtn/img/CtrlButton/record.png");
	ui.pushButton_playFile->changeImage(":/CtrlBtn/img/CtrlButton/insertMediaFile.png");
	//录制
	ui.btnRecord->hide();
	ui.pushButton_toSpeak->hide();
	//ui.btnMemberList->changeImage(":/CtrlBtn/img/CtrlButton/MemberList.png");
	//ui.btnMemberList->changeImage(":/CtrlBtn/img/CtrlButton/btnmember.png");
	//SetEixtUnseeHandsUpList(false);
	//ui.btnMemberList->hide();
	ui.labMember->hide();
}

void CInterActiveCtrlWdg::InitConnect() {
	connect(ui.pushButton_Camera, SIGNAL(sigClicked()), this, SLOT(Slot_OnClickedCameraCtrlBtn()));
	connect(ui.pushButton_mic, SIGNAL(sigClicked()), this, SLOT(Slot_OnClickedMicCtrlBtn()));
	connect(ui.pushButton_player, SIGNAL(sigClicked()), this, SLOT(Slot_OnClickedPlayerCtrlBtn()));
	//connect(ui.pushButton_screenShare, SIGNAL(sigClicked()), this, SLOT(Slot_OnClickedScreenShareCtrlBtn()));
	connect(ui.pushButton_startLive, SIGNAL(sigClicked()), this, SLOT(Slot_OnClickedStartLiveCtrlBtn()));
	//connect(ui.pushButton_LeaveActive, SIGNAL(clicked()), this, SLOT(Slot_OnClickedLeaveActiveBtn()));
	connect(ui.pushButton_cameraList, SIGNAL(sigClicked()), this, SLOT(Slot_OnClickedShowCameraListBtn()));
	connect(ui.pushButton_micList, SIGNAL(sigClicked()), this, SLOT(Slot_OnClickedShowMicListBtn()));
	connect(ui.pushButton_playerList, SIGNAL(sigClicked()), this, SLOT(Slot_OnClickedShowPlayerBtn()));
	connect(ui.pushButton_mic, SIGNAL(sigEnter()), this, SLOT(Slot_OnMicBtnEnter()));
	connect(ui.pushButton_mic, SIGNAL(sigLeave()), this, SLOT(Slot_OnMicBtnLeave()));
	connect(ui.pushButton_player, SIGNAL(sigEnter()), this, SLOT(Slot_OnPlayerBtnEnter()));
	connect(ui.pushButton_player, SIGNAL(sigLeave()), this, SLOT(Slot_OnPlayerBtnLeave()));
	//connect(ui.pushButton_plugin, SIGNAL(clicked()), this, SLOT(Slot_LiveToolClicked()));
	//connect(ui.btnRecord, SIGNAL(sigEnter()), this, SLOT(slotRecordEnter()));
	connect(ui.btnRecord, SIGNAL(sigClicked()), this, SLOT(slotRecordClicked()));
	connect(ui.pushButton_playFile, SIGNAL(sigClicked()), this, SLOT(slotPlayFileClicked()));
	//connect(ui.btnMemberList, SIGNAL(sigClicked()), this, SIGNAL(sigMemberListClicked()));
	connect(ui.pushButton_toSpeak, SIGNAL(clicked()), this, SIGNAL(sigToSpeakClicked()));
}

void CInterActiveCtrlWdg::SetHideDevList() {
	ui.pushButton_cameraList->hide();
	ui.pushButton_micList->hide();
	ui.pushButton_playerList->hide();
}

void CInterActiveCtrlWdg::ChageSpacerNomal(bool bNomal/* = false*/)
{
	if (bNomal)//正常
	{
		ui.hztLiveTool->changeSize(20, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);
		ui.spacerPlayer->changeSize(20, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);
		ui.pushButton_cameraList->hide();
		ui.pushButton_micList->hide();
	}
	else
	{
		ui.hztLiveTool->changeSize(10, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);
		ui.spacerPlayer->changeSize(10, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);
		ui.pushButton_cameraList->show();
		ui.pushButton_micList->show();
	}
	
}

void CInterActiveCtrlWdg::OnOpenCamera(bool bOpen) {
   if (bOpen) {
      ui.pushButton_Camera->changeImage(":/CtrlBtn/img/CtrlButton/camera_open.png");
   } else {
      ui.pushButton_Camera->changeImage(":/CtrlBtn/img/CtrlButton/camera_close.png");
   }
}

void CInterActiveCtrlWdg::SetEnablePlguinsBtn(bool enable) {
   //ui.pushButton_plugin->setEnabled(enable);
}

void CInterActiveCtrlWdg::SetBackGroundColor(QString styleSheet) {
	this->setStyleSheet(styleSheet);
}

void CInterActiveCtrlWdg::ShowToSpeakBtn(bool show) {
	//if (show) {
	//	ui.pushButton_toSpeak->show();
	//}
	//else {
	//	ui.pushButton_toSpeak->hide();
	//}
}	

void CInterActiveCtrlWdg::SetToSpeakEnble(bool bEnble /*= true*/)
{
	ui.pushButton_toSpeak->setEnabled(bEnble);
}

void CInterActiveCtrlWdg::SetToSpeakBtnState(bool toSpeak) {
	if (toSpeak) {
		ui.pushButton_toSpeak->setText(APPLY_SPEAK);
        mbIsSpeaking = false;
	}
	else {
		ui.pushButton_toSpeak->setText(DOWNMIC);
        mbIsSpeaking = true;
	}
    this->update();
}

bool CInterActiveCtrlWdg::GetToSpeakBtnState() {
   return mbIsSpeaking;
}

void CInterActiveCtrlWdg::OnOpenMic(bool bOpen) {
   if (bOpen) {
      ui.pushButton_mic->changeImage(":/CtrlBtn/img/CtrlButton/mic_0.png");
   } else {
      ui.pushButton_mic->changeImage(":/CtrlBtn/img/CtrlButton/mic_close.png");
   }
   this->update();
}

void CInterActiveCtrlWdg::OnOpenPlayer(bool bOpen) {
   if (bOpen) {
      ui.pushButton_player->changeImage(":/CtrlBtn/img/CtrlButton/player_open.png");
   } else {
      ui.pushButton_player->changeImage(":/CtrlBtn/img/CtrlButton/player_close.png");
   }
   this->update();
}

void CInterActiveCtrlWdg::OnOpenScreenShare(bool bOpen) {
   //if (bOpen) {
   //   ui.pushButton_screenShare->changeImage(":/CtrlBtn/img/CtrlButton/desktop_share.png");
   //} else {
   //   ui.pushButton_screenShare->changeImage(":/CtrlBtn/img/CtrlButton/desktop_share.png");
   //}
}

void CInterActiveCtrlWdg::ShowStartLiveBtn(bool show) {
	if (show) {
		ui.pushButton_startLive->show();
	}
	else {
		ui.pushButton_startLive->hide();
	}
}

void CInterActiveCtrlWdg::OnStartLive(bool start) {
	//mCurLiveState = start;
   if (start) {
      ui.pushButton_startLive->changeImage(":/toolWidget/stopLive");
      //ui.pushButton_startLive->setToolTip(QStringLiteral("结束直播"));
   } else {
	   emit SigStopLive();
      ui.pushButton_startLive->changeImage(":/toolWidget/startLive");
      //ui.pushButton_startLive->setToolTip(QStringLiteral("开始直播"));
   }
}

void CInterActiveCtrlWdg::Slot_OnClickedCameraCtrlBtn() {
   emit Sig_OnClickedCameraCtrlBtn();
}

void CInterActiveCtrlWdg::Slot_OnClickedMicCtrlBtn() {
   emit Sig_OnClickedMicCtrlBtn();
}

void CInterActiveCtrlWdg::Slot_OnClickedPlayerCtrlBtn() {
   emit Sig_OnClickedPlayerCtrlBtn();
}

void CInterActiveCtrlWdg::Slot_OnClickedScreenShareCtrlBtn() {
   emit Sig_OnClickedScreenShareCtrlBtn();
}

void CInterActiveCtrlWdg::Slot_OnClickedStartLiveCtrlBtn() {
    if(mpStartLiveBtnTimer){
        ui.pushButton_startLive->setEnabled(false);
        mpStartLiveBtnTimer->start(2000);
    }
    emit Sig_OnClickedStartLiveCtrlBtn();
}

//void CInterActiveCtrlWdg::Slot_OnClickedLeaveActiveBtn() {
//   emit Sig_OnClickedLeaveActiveBtn();
//}

void CInterActiveCtrlWdg::Slot_OnClickedShowCameraListBtn() {
   emit Sig_OnClickedShowCameraListBtn();
}

void CInterActiveCtrlWdg::Slot_OnClickedShowMicListBtn() {
   emit Sig_OnClickedShowMicListBtn();
}

void CInterActiveCtrlWdg::Slot_OnClickedShowPlayerBtn() {
   emit Sig_OnClickedShowPlayerBtn();
}

void CInterActiveCtrlWdg::Slot_OnMicBtnEnter() {
   emit Sig_OnMicBtnEnter();
}
void CInterActiveCtrlWdg::Slot_OnMicBtnLeave() {
   emit Sig_OnMicBtnLeave();
}
void CInterActiveCtrlWdg::Slot_OnPlayerBtnEnter() {
   emit Sig_OnPlayerBtnEnter();
}
void CInterActiveCtrlWdg::Slot_OnPlayerBtnLeave() {
   emit Sig_OnPlayerBtnLeave();
}

void CInterActiveCtrlWdg::OnSetLiveTime(const QString& time) {
   //ui.label->setText(time);
}

void CInterActiveCtrlWdg::HideTimeCount() {
   // ui.label->hide();
}

void CInterActiveCtrlWdg::SetShowMemberListBtn(bool show) {
    //if (show) {
    //    ui.btnMemberList->show();
    //    ui.labMember->show();
    //}
    //else {
    //    ui.btnMemberList->hide();
    //    ui.labMember->hide();
    //}
}

void CInterActiveCtrlWdg::SetShowPluginBtn(bool show) {
    //if (show) {
    //    ui.pushButton_plugin->show();
    //}
    //else {
    //    ui.pushButton_plugin->hide();
    //}
}

//QString CInterActiveCtrlWdg::GetLiveTime() {
//   return ui.label->text();
//}

QPoint CInterActiveCtrlWdg::GetMicBtnGlobalPos() {
   return ui.pushButton_mic->mapToGlobal(QPoint(15, 0));
}

QPoint CInterActiveCtrlWdg::GetPlayBtnGlobalPos() {
   return ui.pushButton_player->mapToGlobal(QPoint(18, 0));
}

QPoint CInterActiveCtrlWdg::GetMicListBtnGlobalPos() {
   return ui.pushButton_micList->mapToGlobal(QPoint(0, 0));
}
QPoint CInterActiveCtrlWdg::GetPlayListBtnGlobalPos() {
   return ui.pushButton_playerList->mapToGlobal(QPoint(0, 0));
}
QPoint CInterActiveCtrlWdg::GetCameraListBtnGlobalPos() {
   return ui.pushButton_cameraList->mapToGlobal(QPoint(0, 0));
}

int CInterActiveCtrlWdg::GetCameraListBtnWidth() {
   return ui.pushButton_cameraList->width();
}

int CInterActiveCtrlWdg::GetMicListBtnWidth() {
   return ui.pushButton_micList->width();
}

int CInterActiveCtrlWdg::GetPlayListBtnWidth() {
   return ui.pushButton_playerList->width();
}

void CInterActiveCtrlWdg::SetMicListBtnEnable(bool bEnable) {
   ui.pushButton_micList->setEnabled(bEnable);
}

void CInterActiveCtrlWdg::ShowTop(bool top) {
   if (top) {
      this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint | Qt::Tool | Qt::WindowStaysOnTopHint);
   } else {
      this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint);
   }
}

void CInterActiveCtrlWdg::mouseMoveEvent(QMouseEvent *) {
   if (m_bIsFull) {
      emit Sig_MouseEnterCtrlWdg();
   }
}

void CInterActiveCtrlWdg::SetFullState(bool isFull) {
   m_bIsFull = isFull;
   if (m_bIsFull){
      EnableMouseTracking();
   }
   else {
      DisMouseTracking();
   }
}

void CInterActiveCtrlWdg::DisMouseTracking() {
   this->setMouseTracking(false);
   //ui.label->setMouseTracking(false);
   ui.pushButton_Camera->setMouseTracking(false);
   ui.pushButton_cameraList->setMouseTracking(false);
   //ui.pushButton_LeaveActive->setMouseTracking(false);
   ui.pushButton_mic->setMouseTracking(false);
   ui.pushButton_micList->setMouseTracking(false);
   ui.pushButton_player->setMouseTracking(false);
   ui.pushButton_playerList->setMouseTracking(false);
   ui.pushButton_startLive->setMouseTracking(false);
   ui.widget_2->setMouseTracking(false);
   ui.widget_3->setMouseTracking(false);
   ui.widget_4->setMouseTracking(false);
   ui.widget->setMouseTracking(false);
   ui.widget_cameraList->setMouseTracking(false);
   ui.widget_micList->setMouseTracking(false);
   ui.widget_playerList->setMouseTracking(false);
}

void CInterActiveCtrlWdg::EnableMouseTracking() {
   this->setMouseTracking(true);
   //ui.label->setMouseTracking(true);
   ui.pushButton_Camera->setMouseTracking(true);
   ui.pushButton_cameraList->setMouseTracking(true);
   //ui.pushButton_LeaveActive->setMouseTracking(true);
   ui.pushButton_mic->setMouseTracking(true);
   ui.pushButton_micList->setMouseTracking(true);
   ui.pushButton_player->setMouseTracking(true);
   ui.pushButton_playerList->setMouseTracking(true);
   ui.pushButton_startLive->setMouseTracking(true);
   ui.widget_2->setMouseTracking(true);
   ui.widget_3->setMouseTracking(true);
   ui.widget_4->setMouseTracking(true);
   ui.widget->setMouseTracking(true);
   ui.widget_cameraList->setMouseTracking(true);
   ui.widget_micList->setMouseTracking(true);
   ui.widget_playerList->setMouseTracking(true);
}

void CInterActiveCtrlWdg::Slot_LiveToolClicked()
{
	STRU_MAINUI_CLICK_CONTROL loControl;
	loControl.m_eType = control_LiveTool;
	SingletonMainUIIns::Instance().PostCRMessage(MSG_MAINUI_CLICK_CONTROL, &loControl, sizeof(STRU_MAINUI_CLICK_CONTROL));

}

void CInterActiveCtrlWdg::ShowLiveTool(bool ok)
{
	//if (ok) {
 //     ui.pushButton_plugin->show();
	//}
	//else {
 //     ui.pushButton_plugin->hide();
	//}
}


void CInterActiveCtrlWdg::slotRecordClicked()
{
	slotEnableRecordBtn(false);
	emit SigCurRecordState(mRecordState);
}

void CInterActiveCtrlWdg::slotEnableRecordBtnTimeOut() {
	slotEnableRecordBtn(true);
}

int CInterActiveCtrlWdg::GetRecordState()
{
	return mRecordState;
}

void CInterActiveCtrlWdg::SetRecordState(const int iState)
{
	mRecordState = iState;
	switch (mRecordState)
	{
		case eRecordState_Recording:{//录制中
			ui.btnRecord->changeImage(":/CtrlBtn/img/CtrlButton/recording.png");
		}
		break;
		case eRecordState_Suspend:{//录制暂停
			ui.btnRecord->changeImage(":/CtrlBtn/img/CtrlButton/recordSuspend.png");
		}
		break;
		default:{//停止录制
			ui.btnRecord->changeImage(":/CtrlBtn/img/CtrlButton/record.png");
		}
		break;
	}
	slotEnableRecordBtn(true);
}

//QPoint CInterActiveCtrlWdg::GetRecordBtnPos()
//{
//	return ui.btnRecord->mapToGlobal(QPoint(0, 0));
//}
//
//int CInterActiveCtrlWdg::GetRecordBtnWidth()
//{
//	return ui.btnRecord->width();
//}

//void CInterActiveCtrlWdg::setRecordBtnShow(const int iDisplay)
//{
//	if (eDispalyCutRecord_Show == iDisplay){
//		ui.btnRecord->show();
//	}
//	else{
//		ui.btnRecord->hide();
//	}
//}

void CInterActiveCtrlWdg::slotEnableRecordBtn(bool enable	/*= true*/)
{
	if (NULL == mpBtnRecordEnableTime)
	{
		mpBtnRecordEnableTime = new QTimer(this);
		connect(mpBtnRecordEnableTime, &QTimer::timeout, this, &CInterActiveCtrlWdg::slotEnableRecordBtnTimeOut);
	}

	ui.btnRecord->setEnabled(enable);

	if (enable)//启用
	{
		if (mpBtnRecordEnableTime&&mpBtnRecordEnableTime->isActive())
		{
			mpBtnRecordEnableTime->stop();
		}
	}
	else{//禁用  2s后启用
		mpBtnRecordEnableTime->start(2000);
	}
}

//bool CInterActiveCtrlWdg::IsRecordBtnhide()
//{
//	bool bHide = true;
//	if (NULL != ui.btnRecord)
//	{
//		bHide = (!ui.btnRecord->isVisible());
//	}
//	return bHide;
//}

void CInterActiveCtrlWdg::ShowMemberListBtn()
{
	//ui.btnMemberList->show();
	//ui.labMember->show();
}

//void CInterActiveCtrlWdg::SetEnableToSpeakBtn(bool enable) {
//   ui.pushButton_toSpeak->setEnabled(enable);
//   ui.pushButton_toSpeak->update();
//}

void CInterActiveCtrlWdg::EnableStartLive(bool enable) {
   ui.pushButton_startLive->setEnabled(enable);
}


//void CInterActiveCtrlWdg::SetEixtUnseeHandsUpList(bool bExit)
//{
//	//if (bExit)
//	//{
//	//	ui.btnMemberList->changeImage(":/CtrlBtn/img/CtrlButton/btnmemberWithHU.png");
//	//}
//	//else
//	//{
//	//	ui.btnMemberList->changeImage(":/CtrlBtn/img/CtrlButton/btnmember.png");
//	//}
//}

void CInterActiveCtrlWdg::slotPlayFileClicked() {
   emit sig_OnPlayFileClicked();
}

void CInterActiveCtrlWdg::slot_OnStartLiveTimeOut() {
    ui.pushButton_startLive->setEnabled(true);
}
