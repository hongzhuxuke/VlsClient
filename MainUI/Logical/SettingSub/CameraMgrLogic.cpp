#include "StdAfx.h"
#include "CameraMgrLogic.h"
#include "CameraMgrUI.h"
#include "CameraItemUi.h"
#include "CameraModifyUI.h"
#include "ConfigSetting.h"
#include "IDeviceManager.h"
#include "IDShowPlugin.h"
#include "IDeckLinkDevice.h"
#include "IOBSControlLogic.h"
#include "Msg_MainUI.h"
#include "Msg_OBSControl.h"
#include "Msg_CommonToolKit.h"
#include "dshowcapture.hpp"
#include <QScreen>

CameraMgrLogic::CameraMgrLogic(void)
   : m_pCameraMgrUI(NULL)
   , m_pCameraModifyUI(NULL)
   , m_bShowState(FALSE) {

}

CameraMgrLogic::~CameraMgrLogic(void) {

}

BOOL CameraMgrLogic::Create() {
   do {
      if (!CreateMgrUI()) {
         break;
      }
      if (!CreateModifyUI()) {
         break;
      }
      mDesktopCameraRender[0] = new DesktopCameraRenderWdg();
      mDesktopCameraRender[0]->hide();
      mDesktopCameraRender[1] = new DesktopCameraRenderWdg();
      mDesktopCameraRender[1]->hide();
      ShowVideo();
      return TRUE;
   } while (FALSE);
   return FALSE;
}

void CameraMgrLogic::Destroy() {
   if (mDesktopCameraRender[0]) {
      delete mDesktopCameraRender[0];
      mDesktopCameraRender[0] = nullptr;
   }
   if (mDesktopCameraRender[1]) {
      delete mDesktopCameraRender[1];
      mDesktopCameraRender[1] = nullptr;
   }
   //销毁添加界面
   DestroyMgrUI();
   //销毁修改界面
   DestroyModifyUI();
   m_mutex.lock();
   m_CameraItemMap.clear();
   m_mutex.unlock();
}

void CameraMgrLogic::DealMessage(DWORD adwMessageID, void* apData, DWORD adwLen) {
   switch (adwMessageID) {
      //点击控制
   case MSG_MAINUI_CLICK_CONTROL:
      DealClickControl(apData, adwLen);
      break;
      //显示摄像头列表
   case MSG_MAINUI_DESKTOP_SHOW_CAMERALIST:
      DealShowDesktopCameraList(apData, adwLen);
      break;
      //添加摄像头
   case MSG_MAINUI_MGR_CAMERA:
      DealMgrCamera(apData, adwLen);
      break;
      //修改摄像头
   case MSG_MAINUI_MODIFY_CAMERA:
      DealModifyCamera(apData, adwLen);
      break;
      //修改保存
   case MSG_MAINUI_MODIFY_SAVE:
      DealModifySave(apData, adwLen);
      break;
      //设备全屏
   case MSG_MAINUI_CAMERA_FULL:
      DealCameraFull(apData, adwLen);
      break;
      //设备插拔
   case MSG_MAINUI_DEVICE_CHANGE:
      DealDeviceChange(apData, adwLen);
      break;
      //获取焦点
   case MSG_MAINUI_SETTING_FOCUSIN:
      DealFocusIn(apData, adwLen);
      break;
      //设备删除
   case MSG_MAINUI_CAMERA_DELETE:
      DealDeviceDelete(apData, adwLen);
      break;
      //开始桌面共享
   case MSG_OBSCONTROL_DESKTOPSHARE:
      DealDesktopShare(apData, adwLen);
      break;
   case MSG_MAINUI_DO_CLOSE_MAINWINDOW:
      DealStopLive();
      break;
   default:
      break;
   }
}

void CameraMgrLogic::DealClickControl(void* apData, DWORD adwLen) {
   DEF_CR_MESSAGE_DATA_DECLARE_CA(STRU_MAINUI_CLICK_CONTROL, loMessage, return);
   switch (loMessage.m_eType) {
      //添加摄像头
   case control_AddCamera:
      ShowCameraAddUI(loMessage.m_globalX, loMessage.m_globalY);
      break;
   case control_CloseApp:
      m_pCameraMgrUI->hide();
      break;
   default:
      break;
   }
}

void CameraMgrLogic::DealDeviceChange(void* apData, DWORD adwLen) {
   DEF_CR_MESSAGE_DATA_DECLARE_CA(STRU_MAINUI_DEVICE_CHANGE, loMessage, return);
   //插入
   if (loMessage.m_bAdd) {
//      DShowLog(DShowLogType_Level1_USBHot, DShowLogLevel_Info, L"[DEVICE] INSERT %s\n", loMessage.m_wzDeviceID);
      Sleep(1000);
   }
   else {
//      DShowLog(DShowLogType_Level1_USBHot, DShowLogLevel_Info, L"[DEVICE] REMOVE %s\n", loMessage.m_wzDeviceID);
   }

   QJsonObject body;
   body["vib"] = loMessage.m_bAdd;
   SingletonMainUIIns::Instance().reportLog(STR_VH_LIVE_OPERATE, eLogRePortK_USBCamera_OP, L"USBCamera_OP", body);

   VH::CComPtr<IOBSControlLogic> pObsControlLogic = NULL;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IOBSControl, IID_IOBSControlLogic, pObsControlLogic, return, ASSERT(FALSE));

   VH::CComPtr<IMainUILogic> pMainUILogic;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IMainUI, IID_IMainUILogic, pMainUILogic, return, ASSERT(FALSE));
   if (!pMainUILogic->IsLoadUrlFinished()) {
      TRACE6("%s loading url ,can not showDeviceList\n", __FUNCTION__);
      return;
   }
   pObsControlLogic->DeviceRecheck();
   m_deviceReset = true;
}

// 添加和删除摄像设备
void CameraMgrLogic::DealMgrCamera(void* apData, DWORD adwLen) {
   if (!m_pCameraMgrUI) {
      return;
   }
   VH::CComPtr<IOBSControlLogic> pObsControlLogic = NULL;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IOBSControl, IID_IOBSControlLogic, pObsControlLogic, return, ASSERT(FALSE));

   m_mutex.lock();
   DeviceList::iterator &itor = m_CameraItemMap.begin();
   while (itor != m_CameraItemMap.end()) {
      //发送添加摄像头消息给OBSCONTROL
      STRU_OBSCONTROL_ADDCAMERA  loAddCamera;
      loAddCamera.m_deviceInfo = *itor;
      CameraItem  *item = m_pCameraMgrUI->FindItem(*itor);
      if (!item) {
         itor++;
         continue;
      }
      //---------是否为全屏---------
      //在添加摄像头的时候，如果没有选择全屏，则为右下角
      if (item->IsFullChecked()) {
         loAddCamera.m_PosType = enum_PosType_fullScreen;
      }
      else {
         loAddCamera.m_PosType = enum_PosType_auto;
      }
      int index = item->GetItemIndex();
      //设备选中
      if (item->IsDeviceChecked()) {
         bool isFullScreen = false;
         if (!pObsControlLogic->GetGraphicsDeviceInfoExist(loAddCamera.m_deviceInfo, isFullScreen)) {
            //添加
            loAddCamera.m_dwType = device_operator_add;
            loAddCamera.m_renderHwnd = (mDesktopCameraRender[0]->GetDevId().isEmpty() || mDesktopCameraRender[0]->GetDevId() == QString::fromStdWString(loAddCamera.m_deviceInfo.m_sDeviceID))
               ? mDesktopCameraRender[0]->LockVideo(QString::fromStdWString(loAddCamera.m_deviceInfo.m_sDeviceID), index, mIsDesktop)
               : mDesktopCameraRender[1]->LockVideo(QString::fromStdWString(loAddCamera.m_deviceInfo.m_sDeviceID), index, mIsDesktop);
         }
         else {
            //修改
            if (isFullScreen != (loAddCamera.m_PosType == enum_PosType_fullScreen)) {
               loAddCamera.m_dwType = device_operator_modify;
            }
         }
         VH::CComPtr<ISettingLogic> pSettingLogic;
         DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IMainUI, IID_ISettingLogic, pSettingLogic, return);
         pSettingLogic->SetUsedDevice(true, loAddCamera.m_deviceInfo);
      }
      else {
         //删除
         loAddCamera.m_dwType = device_operator_del;
         VH::CComPtr<IOBSControlLogic> pObsControlLogic = NULL;
         DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IOBSControl, IID_IOBSControlLogic, pObsControlLogic, return);
         pObsControlLogic->DealAddCameraSync(&loAddCamera, sizeof(STRU_OBSCONTROL_ADDCAMERA));
         if (mDesktopCameraRender[0]->GetDevId() == QString::fromStdWString(loAddCamera.m_deviceInfo.m_sDeviceID)) {
            mDesktopCameraRender[0]->LockVideo("", -1, false);
         }
         else if (mDesktopCameraRender[1]->GetDevId() == QString::fromStdWString(loAddCamera.m_deviceInfo.m_sDeviceID)) {
            mDesktopCameraRender[1]->LockVideo("", -1, false);
         }
         VH::CComPtr<ISettingLogic> pSettingLogic;
         DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IMainUI, IID_ISettingLogic, pSettingLogic, return);
         pSettingLogic->SetUsedDevice(false, loAddCamera.m_deviceInfo);
      }
      itor++;
   }

   m_mutex.unlock();
}
//修改摄像头
void CameraMgrLogic::DealModifyCamera(void* apData, DWORD adwLen) {
   if (!m_pCameraModifyUI) {
      return;
   }
   DEF_CR_MESSAGE_DATA_DECLARE_CA(DeviceInfo, deviceInfo, return);
   VH::CComPtr<IOBSControlLogic> pObsControlLogic = NULL;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IOBSControl, IID_IOBSControlLogic, pObsControlLogic, return, ASSERT(FALSE));

   //初始化摄像头管理
   InitCameraMgrUI();
   m_pCameraModifyUI->BindDevice(deviceInfo);
   int deviceCount = pObsControlLogic->GetGraphicsDeviceInfoCount();
   for (int i = 0; i < deviceCount; i++) {
      DeviceInfo info;
      DataSourcePosType posType;
      if (pObsControlLogic->GetGraphicsDeviceInfo(info, posType, i)) {
         if (info == deviceInfo) {
            m_pCameraModifyUI->SetPosType(posType);
         }
         else {
            m_pCameraModifyUI->RemoveDeviceInfo(info);
         }
      }
   }
   m_pCameraModifyUI->CenterWindow((QWidget*)m_pCameraModifyUI->parent());
   m_pCameraModifyUI->setModal(true);
   m_pCameraModifyUI->setVisible(true);
}
//修改摄像头
void CameraMgrLogic::DealModifySave(void* apData, DWORD adwLen) {
   DEF_CR_MESSAGE_DATA_DECLARE_CA(STRU_MAINUI_DEVICE_MODIFY, loMessage, return);

   VH::CComPtr<IOBSControlLogic> pObsControlLogic = NULL;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IOBSControl, IID_IOBSControlLogic, pObsControlLogic, return, ASSERT(FALSE));
   pObsControlLogic->ModifyDeviceSource(
      loMessage.srcDevice,
      loMessage.desDevice,
      loMessage.posType);
}

void CameraMgrLogic::ShowCameraAddUI(int ix, int iy) {
   bool bFullScreen = false;
   VH::CComPtr<IOBSControlLogic> pObsControlLogic = NULL;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IOBSControl, IID_IOBSControlLogic, pObsControlLogic, return, ASSERT(FALSE));
   VH::CComPtr<IMainUILogic> pMainUILogic;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IMainUI, IID_IMainUILogic, pMainUILogic, return, ASSERT(FALSE));
   if (!pMainUILogic->IsLoadUrlFinished()) {
      TRACE6("%s loading url ,can not showDeviceList\n", __FUNCTION__);
      return;
   }

   bool isHasMonitor = pObsControlLogic->IsHasSource(SRC_MONITOR);
   if (isHasMonitor) {
      return;
   }
   showCameraList(ix, iy, true);
}

BOOL CameraMgrLogic::CreateMgrUI() {
   VH::CComPtr<IMainUILogic> pMainUILogic;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IMainUI, IID_IMainUILogic, pMainUILogic, return FALSE, ASSERT(FALSE));
   QWidget* pMainUI = NULL;
   pMainUILogic->GetMainUIWidget((void**)&pMainUI);
   //添加文字窗口创建
   m_pCameraMgrUI = new CameraMgrUI((QDialog*)pMainUI);
   if (NULL == m_pCameraMgrUI || !m_pCameraMgrUI->Create()) {
      ASSERT(FALSE);
      return FALSE;
   }
   return TRUE;
}

void CameraMgrLogic::DestroyMgrUI() {
}

BOOL CameraMgrLogic::CreateModifyUI() {
   VH::CComPtr<IMainUILogic> pMainUILogic;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IMainUI, IID_IMainUILogic, pMainUILogic, return FALSE, ASSERT(FALSE));

   QWidget* pMainUI = NULL;
   pMainUILogic->GetMainUIWidget((void**)&pMainUI);

   //修改界面创建
   m_pCameraModifyUI = new CameraModifyUI((QDialog*)pMainUI);
   if (NULL == m_pCameraModifyUI || !m_pCameraModifyUI->Create()) {
      ASSERT(FALSE);
      return FALSE;
   }

   return TRUE;
}

void CameraMgrLogic::DestroyModifyUI() {
}

//获得设备列表
void CameraMgrLogic::ObtainDeviceList(DeviceList& deviceList) {
   deviceList.clear();
   VH::CComPtr<IDeviceManager> pDeviceManager;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IOBSControl, IID_IDeviceManager, pDeviceManager, return, ASSERT(FALSE));
   pDeviceManager->GetVedioDevices(deviceList);
   SyncDeviceList(deviceList);
}

void CameraMgrLogic::DealCameraSelect(void* apData, DWORD adwLen) {
   if (!m_pCameraMgrUI) {
      return;
   }

   DEF_CR_MESSAGE_DATA_DECLARE_CA(STRU_MAINUI_CHECKSTATUS, loMessage, return);
   if (m_pCameraMgrUI->SelectedItemCount() == 0) {
      m_pCameraMgrUI->SelectedItemSetFullScreen();
   }

   VH::CComPtr<IOBSControlLogic> pObsControlLogic = NULL;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IOBSControl, IID_IOBSControlLogic, pObsControlLogic, return, ASSERT(FALSE));
   DeviceList::iterator &itor = m_CameraItemMap.find(loMessage.m_deviceInfo);
   if (itor != m_CameraItemMap.end()) {
      //发送添加摄像头消息给OBSCONTROL
      STRU_OBSCONTROL_ADDCAMERA  loAddCamera;
      loAddCamera.m_deviceInfo = *itor;
      CameraItem  *item = m_pCameraMgrUI->FindItem(*itor);
      if (item == nullptr) {
         return;
      }
      //---------是否为全屏---------
      //在添加摄像头的时候，如果没有选择全屏，则为右下角
      if (item->IsFullChecked()) {
         loAddCamera.m_PosType = enum_PosType_fullScreen;
      }
      else {
         loAddCamera.m_PosType = enum_PosType_auto;
      }

      int itemIndex = item->GetItemIndex();
      //设备选中
      if (item->IsDeviceChecked()) {
         bool isFullScreen = false;
         //添加
         loAddCamera.m_dwType = device_operator_add;
         loAddCamera.m_renderHwnd = (mDesktopCameraRender[0]->GetDevId().isEmpty() || mDesktopCameraRender[0]->GetDevId() == QString::fromStdWString(loAddCamera.m_deviceInfo.m_sDeviceID))
            ? mDesktopCameraRender[0]->LockVideo(QString::fromStdWString(loAddCamera.m_deviceInfo.m_sDeviceID), itemIndex, mIsDesktop)
            : mDesktopCameraRender[1]->LockVideo(QString::fromStdWString(loAddCamera.m_deviceInfo.m_sDeviceID), itemIndex, mIsDesktop);

         VH::CComPtr<IMainUILogic> pMainUILogic;
         DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IMainUI, IID_IMainUILogic, pMainUILogic, return, ASSERT(FALSE));
         pMainUILogic->RunTask(CustomEvent_ADD_CAMERA, &loAddCamera);

         VH::CComPtr<ISettingLogic> pSettingLogic;
         DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IMainUI, IID_ISettingLogic, pSettingLogic, return);
         pSettingLogic->SetUsedDevice(true, loAddCamera.m_deviceInfo);

         //if (!pObsControlLogic->GetGraphicsDeviceInfoExist(loAddCamera.m_deviceInfo, isFullScreen)) {
         //   //添加
         //   loAddCamera.m_dwType = device_operator_add;
         //   loAddCamera.m_renderHwnd = (mDesktopCameraRender[0]->GetDevId().isEmpty() || mDesktopCameraRender[0]->GetDevId() == QString::fromStdWString(loAddCamera.m_deviceInfo.m_sDeviceID))
         //      ? mDesktopCameraRender[0]->LockVideo(QString::fromStdWString(loAddCamera.m_deviceInfo.m_sDeviceID), itemIndex, mIsDesktop)
         //      : mDesktopCameraRender[1]->LockVideo(QString::fromStdWString(loAddCamera.m_deviceInfo.m_sDeviceID), itemIndex, mIsDesktop);

         //   VH::CComPtr<IMainUILogic> pMainUILogic;
         //   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IMainUI, IID_IMainUILogic, pMainUILogic, return , ASSERT(FALSE));
         //   pMainUILogic->RunTask(CustomEvent_ADD_CAMERA,&loAddCamera);
         //}
         //else {
         //   //修改
         //   if (isFullScreen != (loAddCamera.m_PosType == enum_PosType_fullScreen)) {
         //      loAddCamera.m_dwType = device_operator_modify;
         //      VH::CComPtr<IMainUILogic> pMainUILogic;
         //      DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IMainUI, IID_IMainUILogic, pMainUILogic, return, ASSERT(FALSE));
         //      pMainUILogic->RunTask(CustomEvent_ADD_CAMERA, &loAddCamera);
         //   }
         //}
      }
      else {
         //删除
         loAddCamera.m_dwType = device_operator_del;
         VH::CComPtr<IMainUILogic> pMainUILogic;
         DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IMainUI, IID_IMainUILogic, pMainUILogic, return, ASSERT(FALSE));
         pMainUILogic->RunTask(CustomEvent_ADD_CAMERA, &loAddCamera);

         VH::CComPtr<ISettingLogic> pSettingLogic;
         DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IMainUI, IID_ISettingLogic, pSettingLogic, return);
         pSettingLogic->SetUsedDevice(false, loAddCamera.m_deviceInfo);
      }
      //itor++;
   }
   m_pCameraMgrUI->RefreshItemStatus(loMessage.status);
}

void CameraMgrLogic::DealCameraFull(void* apData, DWORD adwLen) {
   DEF_CR_MESSAGE_DATA_DECLARE_CA(STRU_MAINUI_DEVICE_FULL, loMessage, return);
   STRU_OBSCONTROL_ADDCAMERA  loAddCamera;
   loAddCamera.m_deviceInfo = loMessage.m_deviceInfo;
   loAddCamera.m_PosType = loMessage.posType;
   loAddCamera.m_dwType = device_operator_modify;

   VH::CComPtr<IMainUILogic> pMainUILogic;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IMainUI, IID_IMainUILogic, pMainUILogic, return, ASSERT(FALSE));
   pMainUILogic->RunTask(CustomEvent_ADD_CAMERA, &loAddCamera);
}

//初始化摄像头列表
bool CameraMgrLogic::InitCameraMgrUI() {
   if (!m_pCameraMgrUI) {
      return false;
   }

   if (m_deviceReset) {
      m_deviceReset = false;
      m_mutex.lock();
      m_CameraItemMap.clear();
      ObtainDeviceList(m_CameraItemMap);
      m_mutex.unlock();
   }

   m_pCameraMgrUI->ClearItem();
   m_pCameraModifyUI->Clear();
   m_pCameraMgrUI->resize(m_pCameraMgrUI->width(), 100);

   std::vector<DeviceInfo> deviceInfoVector;
   m_mutex.lock();
   DeviceList::iterator &itor = m_CameraItemMap.begin();
   deviceInfoVector.resize(m_CameraItemMap.size());
   while (itor != m_CameraItemMap.end()) {
      int num = GetDeviceDisplayNumber(*itor);
      if (num >= 0 && num < m_CameraItemMap.size()) {
         deviceInfoVector[num] = *itor;
      }
      itor++;
   }
   m_mutex.unlock();

   for (int i = 0; i < deviceInfoVector.size(); i++) {
      CameraItem* item = new CameraItem(m_pCameraMgrUI);
      if (!item) {
         ASSERT(FALSE);
         return false;
      }
      item->SetCameraDevice(deviceInfoVector[i]);
      item->SetItemIndex(i);
      m_pCameraMgrUI->AddItem(item);
      m_pCameraModifyUI->AddItem(deviceInfoVector[i]);
   }

   //设备不存在文本显示
   m_pCameraMgrUI->ShowNoDeivice(0 == m_CameraItemMap.size() ? true : false);
   return true;
}

void CameraMgrLogic::DealDeviceDelete(void* apData, DWORD adwLen) {
   DEF_CR_MESSAGE_DATA_DECLARE_CA(STRU_MAINUI_DELETECAMERA, loMessage, return);
   for (int i = 0; i < 2; i++) {
      mDesktopCameraRender[i]->UnlockVideo(loMessage.hwnd);
   }
}

void CameraMgrLogic::DealStopLive() {
   if (mDesktopCameraRender[0]) {
      mDesktopCameraRender[0]->ClearInfo();
   }
   if (mDesktopCameraRender[1]) {
      mDesktopCameraRender[1]->ClearInfo();
   }
}

void CameraMgrLogic::DealDesktopShare(void* apData, DWORD adwLen) {
   if (apData == NULL) { //停止桌面共享
      mIsDesktop = false;
      if (mDesktopCameraRender[0]) {
         mDesktopCameraRender[0]->hide();
      }
      if (mDesktopCameraRender[1]) {
         mDesktopCameraRender[1]->hide();
      }
      if (m_pCameraMgrUI) {
         m_pCameraMgrUI->hide();
      }
   }
   else {
      //开始桌面共享
      DEF_CR_MESSAGE_DATA_DECLARE_CA(VHD_WindowInfo, loMessage, return);
      //QRect screen_rect;
      //QDesktopWidget* desktopWidget = QApplication::desktop();
      //if (desktopWidget) {
      //   QList<QScreen *> screenlist = QGuiApplication::screens();
      //   for (int i = 0; i < screenlist.size(); i++) {
      //      if (QString::fromStdWString(loMessage.name) == screenlist.at(i)->name()) {
      //         screen_rect = screenlist.at(i)->availableGeometry();
      //         break;
      //      }
      //   }
      //}

      //mIsDesktop = true;
      //if (mDesktopCameraRender[0]) {
      //   mDesktopCameraRender[0]->ReSize();
      //   if (mDesktopCameraRender[0]->isHidden()) {
      //      mDesktopCameraRender[0]->move(screen_rect.x() + screen_rect.width() - mDesktopCameraRender[0]->width() - 50, 70);
      //   }
      //   mDesktopCameraRender[0]->CheckShow();
      //}
      //if (mDesktopCameraRender[1]) {
      //   mDesktopCameraRender[1]->ReSize();
      //   if (mDesktopCameraRender[1]->isHidden()) {
      //      mDesktopCameraRender[1]->move(screen_rect.x() + screen_rect.width() - mDesktopCameraRender[1]->width() - 50, 400);
      //   }
      //   mDesktopCameraRender[1]->CheckShow();
      //};
      ResetCameraWndPos(QString::fromStdWString(loMessage.name));
   }
}

void CameraMgrLogic::ResetCameraWndPos(QString name) {
   QRect screen_rect;
   QDesktopWidget* desktopWidget = QApplication::desktop();
   if (desktopWidget) {
      QList<QScreen *> screenlist = QGuiApplication::screens();
      for (int i = 0; i < screenlist.size(); i++) {
         if (name == screenlist.at(i)->name()) {
            screen_rect = screenlist.at(i)->availableGeometry();
            break;
         }
      }
   }

   mIsDesktop = true;
   if (mDesktopCameraRender[0]) {
      mDesktopCameraRender[0]->ReSize();
      mDesktopCameraRender[0]->move(screen_rect.x() + screen_rect.width() - mDesktopCameraRender[0]->width() - 50, 70);
      mDesktopCameraRender[0]->CheckShow();
   }
   if (mDesktopCameraRender[1]) {
      mDesktopCameraRender[1]->ReSize();
      mDesktopCameraRender[1]->move(screen_rect.x() + screen_rect.width() - mDesktopCameraRender[1]->width() - 50, 400);
      mDesktopCameraRender[1]->CheckShow();
   };
}

bool CameraMgrLogic::IsCameraShow() {
   if (m_pCameraMgrUI) {
      return !m_pCameraMgrUI->isHidden();
   }
   return false;
}

void CameraMgrLogic::DealFocusIn(void* apData, DWORD adwLen) {
   if (m_pCameraMgrUI) {
      m_pCameraMgrUI->raise();
   }
}
void CameraMgrLogic::ShowVideo() {

}

void CameraMgrLogic::HideVideo() {

}

void CameraMgrLogic::showCameraList(int ix, int iy, const bool bFullScreen){
   if (NULL != m_pCameraMgrUI) {
      VH::CComPtr<IOBSControlLogic> pObsControlLogic = NULL;
      DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IOBSControl, IID_IOBSControlLogic, pObsControlLogic, return, ASSERT(FALSE));
      m_pCameraMgrUI->setSettingShow(false);
      if (InitCameraMgrUI()) {
         m_pCameraMgrUI->adjustSize();
      }
      m_pCameraMgrUI->SetFullScreenShow(bFullScreen);
      m_pCameraMgrUI->SetItemUnchecked();
      int deviceCount = pObsControlLogic->GetGraphicsDeviceInfoCount();
      for (int i = 0; i < deviceCount; i++) {
         DeviceInfo info;
         DataSourcePosType posType = enum_PosType_custom;
         if (pObsControlLogic->GetGraphicsDeviceInfo(info, posType, i)) {
            m_pCameraMgrUI->SetItemIsFullScreen(info, posType == enum_PosType_fullScreen);
         }
      }
      if (deviceCount > 0) {
         //刷新每项状态（禁用，选中等等）
         m_pCameraMgrUI->RefreshItemStatus();
      }
      //显示
      m_pCameraMgrUI->CenterWindow((QWidget*)m_pCameraMgrUI->parent());
      m_pCameraMgrUI->move(ix, iy);
      if (m_pCameraMgrUI->isHidden()) {
         m_pCameraMgrUI->show();
      }
      else {
         m_pCameraMgrUI->hide();
      }
   }
}

void CameraMgrLogic::DealShowDesktopCameraList(void* apData, DWORD adwLen) {
   DEF_CR_MESSAGE_DATA_DECLARE_CA(STRU_MAINUI_CLICK_CONTROL, loMessage, return);
   int ix = loMessage.m_globalX;
   int iy = loMessage.m_globalY;
   bool bFullScreen = false;
   showCameraList(ix, iy, bFullScreen);
}

void *CameraMgrLogic::LockVideo(QString strDeviceID, int index) {
   return (mDesktopCameraRender[0]->GetDevId().isEmpty() || mDesktopCameraRender[0]->GetDevId() == strDeviceID)
      ? mDesktopCameraRender[0]->LockVideo(strDeviceID, index, mIsDesktop)
      : mDesktopCameraRender[1]->LockVideo(strDeviceID, index, mIsDesktop);

}
void CameraMgrLogic::UnlockVideo(void *v) {
   STRU_MAINUI_DELETECAMERA param;
   param.hwnd = (HWND)v;
   SingletonMainUIIns::Instance().PostCRMessage(MSG_MAINUI_CAMERA_DELETE, &param, sizeof(STRU_MAINUI_DELETECAMERA));
}

void CameraMgrLogic::ResetLockVideo() {
   mDesktopCameraRender[0]->hide();
   mDesktopCameraRender[1]->hide();
}

void CameraMgrLogic::SetDesktopShareState(bool state) {
   mIsDesktop = state;
}

