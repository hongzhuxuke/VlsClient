#include "stdafx.h"
#include "RecordDlg.h"
#include "ToolButton.h"

#include "Msg_MainUI.h"

#include <QPainter>
#include <QPushButton>
#include "ButtonWdg.h"

//#define QSS_NOMAL "color: #FFFFFF; \
//		background-color:rgba(67,67,67,255); "
//
//#define QSS_HOVER "color: #FFFFFF; \
//		background-color:rgba(44,44,44,255); "

#define QSS_NOMAL "color: #FFFFFF;"
#define QSS_HOVER "color: #FFFFFF;"


#define BOERDER_NOMAL "border-color:rgba(67,67,67,1);background-color:rgba(67,67,67,255); " //;
#define BOERDER_HOVER "border-color:rgba(44,44,44,1);background-color:rgba(44,44,44,255); " //
#define RecordNormal "border-image:url(:/button/images/record.png);"
#define RecordNormalHover "border-image:url(:/button/images/record_hover.png);"
#define RecordPauseNormal "border-image:url(:/button/images/record_pause.png);"
#define RecordPauseHover "border-image:url(:/button/images/record_pause_hover.png);"
#define RecordStop "border-image:url(:/button/images/record_stop.png);"
#define RecordStopHover "border-image:url(:/button/images/record_stop_hover.png);"

CRecordDlg::CRecordDlg(QWidget *parent)
	: VHDialog(parent),
	mSuspendOrRecovery(eSuspendOrRecovery_Suspend){
	ui.setupUi(this);

   this->setFocusPolicy(Qt::StrongFocus);
   this->setWindowFlags(Qt::FramelessWindowHint | Qt::SplashScreen);
   setAttribute(Qt::WA_TranslucentBackground);
   this->setFixedSize(112, 72);
}

CRecordDlg::~CRecordDlg() {

}

bool CRecordDlg::Create(const bool& isLive) {
	mbLive = isLive;
	ui.StopRecordBtn->Create(ButtonWdg::eFontDirection_Right, 4, 12, 6, RecordStop, RecordStopHover, RecordStop);
	ui.SuspendOrRecovery->Create(ButtonWdg::eFontDirection_Right, 4, 12, 6, RecordPauseNormal, RecordPauseHover, RecordPauseNormal);
   ui.StopRecordBtn->setIconTextSize(5, 6);
   ui.SuspendOrRecovery->setIconTextSize(5, 6);

	ui.StopRecordBtn->SetBorderWidth(1);
	ui.SuspendOrRecovery->SetBorderWidth(1);
	ui.StopRecordBtn->setRaduis(false);
	ui.SuspendOrRecovery->setRaduis(false);

	ui.StopRecordBtn->setBorderColor(BOERDER_NOMAL, BOERDER_HOVER);
	ui.SuspendOrRecovery->setBorderColor(BOERDER_NOMAL, BOERDER_HOVER);

	ui.StopRecordBtn->setFixedWidth(112);
	ui.SuspendOrRecovery->setFixedWidth(112);
	ui.SuspendOrRecovery->setBorderColor(BOERDER_NOMAL, BOERDER_HOVER);
	ui.StopRecordBtn->setIconSheetStyle(QSS_NOMAL, QSS_HOVER);
	ui.SuspendOrRecovery->setIconSheetStyle(QSS_NOMAL, QSS_HOVER);
	connect(ui.StopRecordBtn, &ButtonWdg::btnClicked, this, &CRecordDlg::slotStopRecordClicke);
	connect(ui.SuspendOrRecovery, &ButtonWdg::btnClicked, this, &CRecordDlg::slotSuspendOrRecoveryClicke);

	ui.StopRecordBtn->setFontText(tr("½áÊø"), Qt::AlignVCenter | Qt::AlignLeft);
	ui.StopRecordBtn->setIconAlignment(Qt::AlignVCenter | Qt::AlignRight);
   return true;
}

void CRecordDlg::Destroy() {
   //if (NULL != m_pMainLayout) {
   //   delete m_pMainLayout;
   //   m_pMainLayout = NULL;
   //}
}

//void CRecordDlg::paintEvent(QPaintEvent *) {
//   QPainter painter(this);
//   painter.drawPixmap(rect(), mPixmap.copy(0, 0, mPixmap.width(), mPixmap.height()));
//}

void CRecordDlg::enterEvent(QEvent *e) {
   QDialog::enterEvent(e);
   m_bEnter = true;
}
void CRecordDlg::leaveEvent(QEvent *e) {
   QDialog::leaveEvent(e);
   m_bEnter = false;
}

void CRecordDlg::focusOutEvent(QFocusEvent *e) {
   QDialog::focusOutEvent(e);
   if (!m_bEnter) {
      this->close();
      return;
   }
   QWidget *focusWidget = this->focusWidget();
   bool focusThis = false;

   while (focusWidget) {
      if (focusWidget == this) {
         focusThis = true;
         break;
      }

      focusWidget = focusWidget->parentWidget();
   }

   if (!focusThis) {
      this->close();
      m_bEnter = false;
   } else {
      this->setFocus();
   }
}

void CRecordDlg::showEvent(QShowEvent *e) {
   setFocus();
   VHDialog::showEvent(e);
}

void CRecordDlg::slotStopRecordClicke() {
	QJsonObject body;
	body["rs"] = "StopRecord";
	SingletonMainUIIns::Instance().reportLog(STR_VH_LIVE_OPERATE, eLogRePortK_StartRecord, L"StopRecord", body);


	if (mbLive)
	{
		STRU_MAINUI_CLICK_CONTROL loControl;
		loControl.m_eType = (enum_control_type)(control_StartRecord);
		loControl.m_dwExtraData = false;  //false ½áÊøÂ¼ÖÆ true¿ªÊ¼Â¼ÖÆ
		SingletonMainUIIns::Instance().PostCRMessage(
			MSG_MAINUI_CLICK_CONTROL, &loControl, sizeof(STRU_MAINUI_CLICK_CONTROL));
	}

   hide();
   emit sigClicked(eCurOp_Stop);
}

void CRecordDlg::slotSuspendOrRecoveryClicke() {
	QJsonObject body;
	if (eSuspendOrRecovery_Suspend == mSuspendOrRecovery)//ÔÝÍ£Â¼ÖÆ
	{
		body["rs"] = "Suspend";
		emit sigClicked(eCurOp_Suspend);
	} 
	else//»Ö¸´Â¼ÖÆ
	{
		body["rs"] = "Recovery";
		emit sigClicked(eCurOp_Recovery);
	}

	
	SingletonMainUIIns::Instance().reportLog(STR_VH_LIVE_OPERATE, eLogRePortK_StartRecord, L"Suspend", body);
	if (mbLive)
	{
		STRU_MAINUI_CLICK_CONTROL loControl;
		loControl.m_eType = (enum_control_type)(control_RecordSuspendOrRecovery);
		loControl.m_dwExtraData = mSuspendOrRecovery;            //0 ÔÝÍ£ 1»Ö¸´Â¼ÖÆ
		SingletonMainUIIns::Instance().PostCRMessage(
			MSG_MAINUI_CLICK_CONTROL, &loControl, sizeof(STRU_MAINUI_CLICK_CONTROL));
	}
	hide();
}

void CRecordDlg::SetRecordState(const int& eState)
{
	if (eRecordState_Recording == eState){//µ±Ç°Â¼ÖÆ×´Ì¬£º Â¼ÖÆÖÐ
		ui.SuspendOrRecovery->setFontText(tr("ÔÝÍ£"), Qt::AlignVCenter | Qt::AlignLeft);
		ui.SuspendOrRecovery->setIconAlignment(Qt::AlignVCenter | Qt::AlignRight);
      ui.SuspendOrRecovery->setIconSheet(RecordPauseNormal, RecordPauseHover, RecordPauseNormal);
		mSuspendOrRecovery = eSuspendOrRecovery_Suspend;
	}
	else if (eRecordState_Stop == eState)
	{
		ui.SuspendOrRecovery->setFontText(tr("ÔÝÍ£"), Qt::AlignVCenter | Qt::AlignLeft);
		ui.SuspendOrRecovery->setIconAlignment(Qt::AlignVCenter | Qt::AlignRight);
      ui.SuspendOrRecovery->setIconSheet(RecordPauseNormal, RecordPauseHover, RecordPauseNormal);
	}
	else
	{
		mSuspendOrRecovery = eSuspendOrRecovery_Recovery;
		ui.SuspendOrRecovery->setFontText(tr("»Ö¸´"), Qt::AlignVCenter | Qt::AlignLeft);//6pt
		ui.SuspendOrRecovery->setIconAlignment(Qt::AlignVCenter | Qt::AlignRight);
      ui.SuspendOrRecovery->setIconSheet(RecordNormal, RecordNormalHover, RecordNormal);
	}

}