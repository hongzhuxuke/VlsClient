#include "CScreenShareToolWgd.h"

#include <QPropertyAnimation>
#include <QApplication>
#include <QDesktopWidget>
#include <QDebug>

#include "MainUIIns.h"
#include "Msg_MainUI.h"
#include "DebugTrace.h"
#include "BtnMenuWdg.h"


CScreenShareToolWgd::CScreenShareToolWgd(QWidget *parent)
: QWidget(parent) ,
mbTencentSDK(true){
   ui.setupUi(this);
   this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint | Qt::Tool | Qt::WindowStaysOnTopHint);
   connect(ui.pushButton_mic, SIGNAL(sigClicked()), this, SLOT(Slot_OnClickedMicBtn()));
   connect(ui.pushButton_stopShare, SIGNAL(sigClicked()), this, SLOT(Slot_OnClickedStopShareBtn()));
   connect(ui.pushButton_camera, SIGNAL(sigClicked()), this, SLOT(Slot_OnClickedCameraBtn()));
   connect(ui.pushButton_player, SIGNAL(sigClicked()), this, SLOT(Slot_OnClickedPlayerBtn()));
   connect(ui.pushButton_startLive, SIGNAL(sigClicked()), this, SLOT(Slot_OnClickedStartLive()));
   connect(ui.tBtnQuikTool, SIGNAL(sigClicked()), this, SLOT(on_tBtnQuikTool_clicked()));
   
   ui.pushButton_mic->changeImage(":/CtrlBtn/img/CtrlButton/mic_close.png");
   ui.pushButton_stopShare->changeImage(":/CtrlBtn/img/CtrlButton/stop_sharing.png");
   ui.pushButton_camera->changeImage(":/CtrlBtn/img/CtrlButton/camera_close.png");
   ui.pushButton_player->changeImage(":/CtrlBtn/img/CtrlButton/player_open.png");
   ui.pushButton_startLive->changeImage(":/CtrlBtn/img/CtrlButton/start_live.png");
   ui.tBtnQuikTool->changeImage(":/CtrlBtn/img/CtrlButton/QuickTool.png");

   mPMoreOpMenu = new BtnMenuWdg(this);
   mPMoreOpMenu->addItem(BtnMenuWdg::eOptype_Member);
   mPMoreOpMenu->addItem(BtnMenuWdg::eOptype_Chat);
   mPMoreOpMenu->addItem(BtnMenuWdg::eOptype_Interactive);
   connect(mPMoreOpMenu, &BtnMenuWdg::sigClicked, this, &CScreenShareToolWgd::slotMenuCliecked);

   setAttribute(Qt::WA_TranslucentBackground);
}

CScreenShareToolWgd::~CScreenShareToolWgd() {

}

void CScreenShareToolWgd::slotMenuCliecked(const int& iOpType)
{
	switch (iOpType)
	{
	case BtnMenuWdg::eOptype_Member:
		emit sigMemberClicked();
		//SlotShowMember();
		break;
	case	BtnMenuWdg::eOptype_Chat:
		slotChatClicked();
		break;
	case BtnMenuWdg::eOptype_Interactive:
	default:
		emit sigInteractiveClicked();
		break;
	}
}

void CScreenShareToolWgd::enterEvent(QEvent *) {
   if (!m_bEnterEvent) {
      QPropertyAnimation *pAnimation = new QPropertyAnimation(this, "pos");
      pAnimation->setDuration(500);
      QRect rect = QApplication::desktop()->availableGeometry();
      int n = this->pos().ry();
      pAnimation->setStartValue(QPoint((rect.width() - this->width()) / 2, this->pos().ry()));
      pAnimation->setEndValue(QPoint((rect.width() - this->width()) / 2, 0));
      pAnimation->start(QAbstractAnimation::DeleteWhenStopped);
      m_bEnterEvent = true;
   }
}

void CScreenShareToolWgd::leaveEvent(QEvent *) {
   if (m_bEnterEvent) {
      QPropertyAnimation *pAnimation = new QPropertyAnimation(this, "pos");
      pAnimation->setDuration(500);
      int n = this->pos().ry();
      QRect rect = QApplication::desktop()->availableGeometry();
      pAnimation->setStartValue(QPoint((rect.width() - this->width()) / 2, this->pos().ry()));
      pAnimation->setEndValue(QPoint((rect.width() - this->width()) / 2, -70));
      pAnimation->start(QAbstractAnimation::DeleteWhenStopped);
      m_bEnterEvent = false;
   }
}

void CScreenShareToolWgd::Slot_OnClickedMicBtn() {
   emit Sig_ClickedMicBtn();
}

void CScreenShareToolWgd::Slot_OnClickedStopShareBtn() {
   emit Sig_ClickedStopShareBtn();
}

void CScreenShareToolWgd::Slot_OnClickedCameraBtn() {
   emit Sig_ClickedCameraBtn();
}

void CScreenShareToolWgd::Slot_OnClickedPlayerBtn() {
   emit Sig_ClickedPlayerBtn();
}

void CScreenShareToolWgd::showEvent(QShowEvent *) {
   QPropertyAnimation *pAnimation = new QPropertyAnimation(this, "pos");
   pAnimation->setDuration(2000);
   QRect rect = QApplication::desktop()->availableGeometry();
   pAnimation->setStartValue(QPoint((rect.width() - this->width()) / 2, 0));
   pAnimation->setEndValue(QPoint((rect.width() - this->width()) / 2, -70));
   pAnimation->start(QAbstractAnimation::DeleteWhenStopped);
}


void CScreenShareToolWgd::SetMicState(bool open) {
   if (open) {
      ui.pushButton_mic->changeImage(":/CtrlBtn/img/CtrlButton/mic_0.png");
   } else {
      ui.pushButton_mic->changeImage(":/CtrlBtn/img/CtrlButton/mic_close.png");
   }
}

void CScreenShareToolWgd::SetCameraState(bool open) {
   if (open) {
      ui.pushButton_camera->changeImage(":/CtrlBtn/img/CtrlButton/camera_open.png");
   } else {
      ui.pushButton_camera->changeImage(":/CtrlBtn/img/CtrlButton/camera_close.png");
   }
}

void CScreenShareToolWgd::SetStartLive(bool start) {
   if (start) {
      ui.pushButton_startLive->changeImage(":/CtrlBtn/img/CtrlButton/stop_live.png");
   } else {
      ui.pushButton_startLive->changeImage(":/CtrlBtn/img/CtrlButton/start_live.png");
   }
}

void CScreenShareToolWgd::SetPlayerState(bool open) {
	mbPlayerOpenState = open;
   if (open) {
      ui.pushButton_player->changeImage(":/CtrlBtn/img/CtrlButton/player_open.png");
   } else {
      ui.pushButton_player->changeImage(":/CtrlBtn/img/CtrlButton/player_close.png");
   }
}

bool CScreenShareToolWgd::GetPlayerState()
{
	return mbPlayerOpenState;
}

void CScreenShareToolWgd::setTencentSDK(bool bTen/* = true*/)
{
	mbTencentSDK = bTen;
}

void CScreenShareToolWgd::Slot_OnClickedStartLive() {
   emit Sig_ClickedStartLiveBtn();
}

void CScreenShareToolWgd::SetHostUser(bool bHost) {
   m_bHost = bHost;
   if (m_bHost) {
	   ui.pushButton_startLive->show();
	   ui.line_startLive->show();
	   ui.line_startLive->setMinimumSize(QSize(1, 0));
	   ui.line_startLive->setMaximumSize(QSize(1, 16777215));
   } else {
	   ui.pushButton_startLive->hide();
	   ui.line_startLive->hide();
	   ui.line_startLive->setMinimumSize(QSize(0, 0));
	   ui.line_startLive->setMaximumSize(QSize(0, 0));
   }
}

void CScreenShareToolWgd::slotChatClicked()
{
	TRACE6(" %s ", __FUNCTION__);
	if (mbTencentSDK) {
		bool bIntertactive = true;
		//SingletonMainUIIns::Instance().PostCRMessage(MSG_MAINUI_DESKTIO_SHOW_CHAR, NULL, NULL);
	}
	emit Sig_ChatClicked();
}

void CScreenShareToolWgd::on_tBtnQuikTool_clicked()
{
	TRACE6(" %s ", __FUNCTION__);
	mPMoreOpMenu->SetItemShow(BtnMenuWdg::eOptype_Member, true);
	mPMoreOpMenu->SetItemShow(BtnMenuWdg::eOptype_Chat, true);
	mPMoreOpMenu->SetItemShow(BtnMenuWdg::eOptype_Interactive, true);

	mPMoreOpMenu->SetShowItemCount(3);

	QPoint pos = mapToGlobal(ui.tBtnQuikTool->pos());
	int iX = pos.x();
	int iY = pos.y();

	mPMoreOpMenu->show(iX, iY + ui.tBtnQuikTool->height());
}
