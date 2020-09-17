#include "RtcGetDeviceList.h"
#include <QList>

std::atomic_bool RtcGetDeviceList::bManagerThreadRun = false;
HANDLE RtcGetDeviceList::threadHandle = nullptr;
#include "WebRtcSDKInterface.h"

using namespace vlive;
RtcGetDeviceList::RtcGetDeviceList() {
   bManagerThreadRun = true;
   mGetDevThread = new std::thread(RtcGetDeviceList::ThreadProcess, this);
   threadHandle = ::CreateEvent(NULL, FALSE, FALSE, NULL);
}

RtcGetDeviceList::~RtcGetDeviceList() {

   bManagerThreadRun = false;
   if (threadHandle) {
      ::SetEvent(threadHandle);
   }
   if (mGetDevThread) {
      mGetDevThread->join();
      delete mGetDevThread;
      mGetDevThread = nullptr;
   }
   if (threadHandle) {
      CloseHandle(threadHandle);
      threadHandle = nullptr;
   }
}

void RtcGetDeviceList::SetSubscribe(DevicelistSubscribe* sub) {
   mDevSub = sub;
};

DWORD WINAPI RtcGetDeviceList::ThreadProcess(LPVOID p) {
   while (bManagerThreadRun) {
      DWORD ret = WaitForSingleObject(threadHandle, INFINITE);
      if (p && bManagerThreadRun) {
         RtcGetDeviceList* sdk = (RtcGetDeviceList*)(p);
         if (sdk) {
            sdk->ProcessTask();
         }
      }
   }
   return 0;
}

void RtcGetDeviceList::ProcessTask() {
   QList<VhallLiveDeviceInfo> playerDevList;
   QList<VhallLiveDeviceInfo> cameraDevList;
   QList<VhallLiveDeviceInfo> micDevList;

   std::list<vhall::VideoDevProperty> cameraList;
   GetWebRtcSDKInstance()->GetCameraDevices(cameraList);
   std::list<vhall::VideoDevProperty>::iterator it = cameraList.begin();
   while (it != cameraList.end()) {
      VhallLiveDeviceInfo info(QString::fromStdString(it->mDevId), QString::fromStdString(it->mDevName), it->mIndex);
      cameraDevList.push_back(info);
      it++;
   }

   std::list<vhall::AudioDevProperty> micList;
   GetWebRtcSDKInstance()->GetMicDevices(micList);
   std::list<vhall::AudioDevProperty>::iterator itMic = micList.begin();
   while (itMic != micList.end()) {
      VhallLiveDeviceInfo info(QString::fromStdWString(itMic->mDevGuid), QString::fromStdWString(itMic->mDevName), itMic->mIndex);
      micDevList.push_back(info);
      itMic++;
   }

   std::list<vhall::AudioDevProperty> playerList;
   GetWebRtcSDKInstance()->GetPlayerDevices(playerList);
   std::list<vhall::AudioDevProperty>::iterator itPlayer = playerList.begin();
   while (itPlayer != playerList.end()) {
      VhallLiveDeviceInfo info(QString::fromStdWString(itPlayer->mDevGuid), QString::fromStdWString(itPlayer->mDevName), itPlayer->mIndex);
      playerDevList.push_back(info);
      itPlayer++;
   }

   if (mDevSub) {
      mDevSub->OnDevicelistNotify(micDevList, cameraDevList, playerDevList, mRequestType);
   }
}

void RtcGetDeviceList::SetRequestType(int type) {
   mRequestType = type;
   if (threadHandle) {
      ::SetEvent(threadHandle);
   }
}
