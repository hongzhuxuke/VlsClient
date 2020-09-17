#ifndef TIMERCLOCK_H
#define TIMERCLOCK_H

#include <QLabel>
#include <QTimer>
#include <QTime>

class QDateTime;
class QTime;
class QTimer;
class TimerClock : public QLabel
{
      Q_OBJECT
   public:
      explicit TimerClock(QWidget *parent = 0);
      ~TimerClock();
      bool GetCurrentDateTime(QDateTime &dateTime);
      void SetCurrentDataTime(QDateTime &dateTime);
      int GetDisplayDateTime();
      void Clear();

      void InitStartTime(int initLiveTime);
   signals:

   private slots:
      void updateClock();

   private:
      QString zeroClock;
      QTimer* m_pQTimer = NULL;
      qint64 hisTime;
      qint64 displayTime;
      QDateTime mStartDateTime;
      int mLiveTime = 0;

   public:
      void startTimer();
      void stopTimer();

};

#endif // TIMERCLOCK_H
