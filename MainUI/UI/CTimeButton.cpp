#include "CTimeButton.h"
//#include "../Unility/Unility.h"
#include "priConst.h"
#include <QPaintEvent> 
#include <QStyleOption> 
#include "pub.Const.h"
#include <QDateTime>
#include "DebugTrace.h"

#define QSS_BLIVED "CTimeButton{\
	font: 14px \"Î¢ÈíÑÅºÚ\";\
	   color: #FFFFFF;\
	   background:#FC5659;\
      border-radius:13px;\
	}\
	CTimeButton:hover{\
	background:#ff7b78;\
	}\
	CTimeButton: pressed\
	{\
		background:#D22727;\
	}"

#define QSS_BLIVING  "CTimeButton\
	{\
	font: 14px \"Î¢ÈíÑÅºÚ\";\
	color: #FFFFFF;\
	background:#FC5659;\
	border-radius:13px; \
		}\
		CTimeButton:hover{ \
		background:#ff7b78; \
	}\
	CTimeButton : pressed\
	{\
	background:#ff7b78; \
	}"

CTimeButton::CTimeButton(QWidget *parent /*= Q_NULLPTR*/)
   :QPushButton(parent)
   , mIsStarting(false)
{
   QString strQss = TOOLTIPQSS;
   strQss += QSS_BLIVED;
   this->setStyleSheet(strQss);
   setAttribute(Qt::WA_AlwaysShowToolTips, false);
}

CTimeButton::~CTimeButton()
{

}

void CTimeButton::InitStartTime(int initLiveTime) {
   mLiveTime = initLiveTime;
}

void CTimeButton::updateLiveStatus(bool liveStatus){
   TRACE6("%s liveStatus %d\n",__FUNCTION__, liveStatus);
   mIsStarting = liveStatus;
   if (mIsStarting){
      QString strQss = TOOLTIPQSS;
      strQss += QSS_BLIVING;
      this->setStyleSheet(strQss);
      startClassTime();
      setHoverDisplayText(QStringLiteral("½áÊøÖ±²¥"));
   }
   else
   {
      QString strQss = TOOLTIPQSS;
      strQss += QSS_BLIVED;
      this->setStyleSheet(strQss);
      setHoverDisplayText(QStringLiteral("¿ªÊ¼Ö±²¥"));
      if (nullptr != mpClassTime) {
         mpClassTime->stop();
         TRACE6("%s mpClassTime->stop()\n", __FUNCTION__);
      }
   }
   repaint();
   update();
}

void CTimeButton::Clear() {
   miClassingTime = 0;
   mLiveTime = 0;
}

bool CTimeButton::GetLiveStatus()
{
   return mIsStarting;
}

void CTimeButton::setHoverDisplayText(const QString& strText)
{
   mstrDisplayText = strText;
   if (mstrDisplayText.isEmpty())
   {
      setText("00:00:00");
      setEnabled(false);
   }
   else {
      setText(strText);
      setEnabled(true);
   }
}

void CTimeButton::enterEvent(QEvent *event)
{
   mbMouseHoverOn = true;
   if (!mstrDisplayText.isEmpty()) {
      setText(mstrDisplayText);
   }
   QPushButton::enterEvent(event);
}

void CTimeButton::leaveEvent(QEvent *event)
{
   mbMouseHoverOn = false;
   if (mIsStarting) {
      dispalyTime();
   }
   QPushButton::leaveEvent(event);
}

void CTimeButton::startClassTime()
{
   TRACE6("%s startClassTime\n", __FUNCTION__);
   if (mIsStarting) {
      miClassingTime = 0;
      if (nullptr == mpClassTime) {
         mpClassTime = new QTimer(this);
         connect(mpClassTime, &QTimer::timeout, this, &CTimeButton::slot_UpdateClassTime);
      }
      if (nullptr != mpClassTime)
         TRACE6("%s mpClassTime start\n", __FUNCTION__);
         mpClassTime->start(1000);
   }
}

void CTimeButton::dispalyTime()
{
   int iTime = mLiveTime + miClassingTime;
   int iSecond = iTime % 60;
   int iTemp = iTime / 60;
   int iBranch = iTemp % 60;
   int iHour = iTemp / 60;
   mLiveTotalTime = iTime;
   if (!mbMouseHoverOn || mstrDisplayText.trimmed().isEmpty()) {
      setText(QString("%1:%2:%3").arg(iHour).arg(iBranch, 2, 10, QLatin1Char('0')).arg(iSecond, 2, 10, QLatin1Char('0')));
   }
}

void CTimeButton::slot_UpdateClassTime()
{
   if (mLastLiveTime == 0 || miClassingTime == 0) {
      miClassingTime++;
      mLastLiveTime = QDateTime::currentDateTime().toTime_t();
   }
   else {
      miClassingTime = QDateTime::currentDateTime().toTime_t() - mLastLiveTime;
   }
   dispalyTime();
}

long CTimeButton::GetClassTime() {
   return mLiveTotalTime;
}