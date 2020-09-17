#pragma once

#include <QObject>
#include <QApplication>
#include <QObject>
#include <QRunnable>
#include <QThreadPool>
#include "Msg_OBSControl.h"

class WorkTask : public QObject, public QRunnable
{
   Q_OBJECT

public:
   WorkTask(QObject* event_recv_obj);
   ~WorkTask();

   void SetWorkType(int type, void* appdata = nullptr);
protected:
   void run();

signals:
   //注意！要使用信号，采用QObejct 和 QRunnable多继承，记得QObject要放在前面
   void mySignal();

private:
   QObject* mRecvObj = nullptr;
   int mWorkType = 0;
   STRU_OBSCONTROL_ADDCAMERA mAddCamera;
};


class ThreadToopTask : public QObject
{
   Q_OBJECT

public:
    ThreadToopTask(QObject *parent);
    ~ThreadToopTask();

   void DoWork(int type,void* appdata = nullptr);
   void ClearAllWork();
private:
   QThreadPool *mThreadPool = nullptr;
   QObject* mRecvObj = nullptr;
};
