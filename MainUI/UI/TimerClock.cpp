#include <QDateTime>
#include "TimerClock.h"
#include <windows.h>
#include <QDebug>

TimerClock::TimerClock(QWidget *parent)
   : QLabel(parent)
{
   this->setStyleSheet("color:#969696;font:10px;");

   zeroClock = QTime(0, 0, 0, 0).toString("  hh:mm:ss");
   this->setText(zeroClock);
   m_pQTimer = new QTimer();
   m_pQTimer->setInterval(1000);
   connect(m_pQTimer, SIGNAL(timeout()), this, SLOT(updateClock()));
   displayTime=0;
   hisTime=0;
}

TimerClock::~TimerClock()
{
	if (NULL != m_pQTimer)
	{
		delete m_pQTimer;
		m_pQTimer = NULL;
	}
}
bool TimerClock::GetCurrentDateTime(QDateTime &dateTime)
{
   qDebug()<<"TimerClock::GetCurrentDateTime()";

   if(m_pQTimer)
   {
      if(m_pQTimer->isActive())
      {
         dateTime=this->mStartDateTime;
         return true;
      }
   }
   return false;
}
void TimerClock::SetCurrentDataTime(QDateTime &dateTime)
{
   qDebug()<<"TimerClock::SetCurrentDataTime()";

   this->mStartDateTime=dateTime;
}
int TimerClock::GetDisplayDateTime()
{
   qDebug()<<"TimerClock::GetDisplayDateTime()";

   int h=displayTime/1000/3600;
   int m=displayTime/1000/60%60;
   int s=displayTime/1000%60;

   return h*3600+m*60+s;
}
void TimerClock::startTimer()
{

   qDebug()<<"TimerClock::startTimer()";

   if(m_pQTimer->isActive()) {      

      return ;
   }
   
	m_pQTimer->start();
	mStartDateTime = QDateTime::currentDateTime();
}

void TimerClock::stopTimer()
{
   qDebug()<<"TimerClock::stopTimer()";

	m_pQTimer->stop();
	//this->setText(zeroClock);
	hisTime=displayTime;
}

void TimerClock::updateClock()
{
   qint64 msTime=mStartDateTime.msecsTo(QDateTime::currentDateTime())+hisTime*1000 + mLiveTime * 1000;
   qDebug()<<"TimerClock::updateClock()"<<msTime;
   
   int h=msTime/1000/3600;
   int m=msTime/1000/60%60;
   int s=msTime/1000%60;

   char tmp[128]={0};
   sprintf(tmp,"  %02d:%02d:%02d",h,m,s);
   this->setText(tmp);
   displayTime=msTime/1000;
}
void TimerClock::Clear(){
   displayTime = 0;
   hisTime = 0;
   this->setText("  00:00:00");
}

void TimerClock::InitStartTime(int initLiveTime) {
    mLiveTime = initLiveTime;
}

