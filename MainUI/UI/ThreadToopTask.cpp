#include "ThreadToopTask.h"
#include "VhallUI_define.h"
#include <QThread>
#include <iostream>
#include "VH_Macro.h"
#include "Msg_MainUI.h"
#include "Msg_OBSControl.h"
#include "IOBSControlLogic.h"
#include "MainUIIns.h"
#include "CRPluginDef.h"
#include "DebugTrace.h"
#include "VHComPtr.h"
#include "IDeviceManager.h"

using std::cout;
using std::endl;

WorkTask::WorkTask(QObject* event_recv_obj)
{
   mRecvObj = event_recv_obj;
   this->setAutoDelete(true);
}

WorkTask::~WorkTask()
{

}

//线程真正执行的内容
void WorkTask::run()
{
   switch (mWorkType)
   {
   case CustomEvent_CreateObsLogic: {
      VH::CComPtr<IOBSControlLogic> pObsControlLogic = NULL;
      DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IOBSControl, IID_IOBSControlLogic, pObsControlLogic, return, ASSERT(FALSE));
      pObsControlLogic->Create();
      VH::CComPtr<IDeviceManager> pDeviceManager;
      DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IOBSControl, IID_IDeviceManager, pDeviceManager, return, ASSERT(FALSE));
      TRACE6(" pDeviceManager->Create()\n");
      pDeviceManager->Create();
      TRACE6(" pDeviceManager->Create() end\n");
      VH::CComPtr<ISettingLogic> pSettingLogic;
      DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IMainUI, IID_ISettingLogic, pSettingLogic, return);
      pSettingLogic->GetDevices();
      if (mRecvObj) {
         QApplication::postEvent(mRecvObj, new QEvent(CustomEvent_CreateObsLogic));
      }
      break;
   }
   case CustomEvent_INIT_DEVICE: {
      TRACE6("%s CustomEvent_INIT_DEVICE\n",__FUNCTION__);
      VH::CComPtr<IOBSControlLogic> pOBSControlLogic;
      DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IOBSControl, IID_IOBSControlLogic, pOBSControlLogic, return, ASSERT(FALSE));
      pOBSControlLogic->InitCapture();
      pOBSControlLogic->SetAudioCapture(true);
      TRACE6("%s CustomEvent_INIT_DEVICE end\n", __FUNCTION__);
      break;
   }
   case CustomEvent_ADD_CAMERA: {
      VH::CComPtr<IOBSControlLogic> pObsControlLogic = NULL;
      DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IOBSControl, IID_IOBSControlLogic, pObsControlLogic, return);
      pObsControlLogic->DealAddCameraSync(&mAddCamera, sizeof(STRU_OBSCONTROL_ADDCAMERA));
      break;
   }
   default:
      break;
   }
}


void WorkTask::SetWorkType(int type, void *appdata) {
   mWorkType = type;
   if (appdata != nullptr) {
      switch (type)
      {
      case CustomEvent_ADD_CAMERA: {
         mAddCamera = *(STRU_OBSCONTROL_ADDCAMERA*)(appdata);
         break;
      }
      default:
         break;
      }
   }

}


ThreadToopTask::ThreadToopTask(QObject *parent)
    : QObject(parent)
{
   mThreadPool = new QThreadPool(this);
   if (mThreadPool) {
      mThreadPool->setMaxThreadCount(1);
   }
   mRecvObj = parent;
}

ThreadToopTask::~ThreadToopTask()
{
}


void ThreadToopTask::DoWork(int type, void* appdata) {
   if (mThreadPool) {
      WorkTask* work = new WorkTask(mRecvObj);
      work->SetWorkType(type, appdata);
      mThreadPool->start(work);
   }
}

void ThreadToopTask::ClearAllWork() {
   if (mThreadPool) {
      mThreadPool->clear();
   }
}
