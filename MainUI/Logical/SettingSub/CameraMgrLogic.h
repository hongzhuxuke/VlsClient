#ifndef __CAMERA_MGR_LOGIC__H_INCLUDE__
#define __CAMERA_MGR_LOGIC__H_INCLUDE__

#pragma once
#include "Msg_OBSControl.h"
#include "vhdesktopvideo.h"
#include "vhdesktopcameralist.h"
#include <map>
#include <vector>
#include <QMutex>
#include "DesktopCameraRenderWdg.h"

using namespace std;

class CameraModifyUI;
class CameraMgrUI;
class CameraItem;
class CameraSettingUI;
class CameraMgrLogic {
public:
   CameraMgrLogic(void);
   ~CameraMgrLogic(void);

public:
   BOOL Create();
   void Destroy();

   // 处理消息
   void DealMessage(DWORD adwMessageID, void* apData, DWORD adwLen);
   // 显示视频
   void ShowVideo();
   void HideVideo();

   void *LockVideo(QString deviceID = QString(),int index = -1);
   void UnlockVideo(void *v);
   void ResetLockVideo();
   void SetDesktopShareState(bool);
public:
   bool IsCameraShow();
   void ResetCameraWndPos(QString name);

public:
   // 处理Click控制
   void DealClickControl(void* apData, DWORD adwLen);

   // 处理管理摄像头
   void DealMgrCamera(void* apData, DWORD adwLen);

   // 处理修改摄像头
   void DealModifyCamera(void* apData, DWORD adwLen);

   // 处理修改保存
   void DealModifySave(void* apData, DWORD adwLen);

   // 处理设备选中
   void DealCameraSelect(void* apData, DWORD adwLen);

   // 处理设备全屏
   void DealCameraFull(void* apData, DWORD adwLen);

   //摄像头插拔
   void DealDeviceChange(void* apData, DWORD adwLen);

   //处理主窗口得到焦点
   void DealFocusIn(void* apData, DWORD adwLen);

   //处理设备删除
   void DealDeviceDelete(void* apData, DWORD adwLen);

   void DealDesktopShare(void* apData, DWORD adwLen);

   void DealStopLive();
private:
	void showCameraList(int ix, int iy, const bool bFullScreen);
   // 显示管理界面
   void ShowCameraAddUI(int ix, int iy);

   // 创建添加界面
   BOOL CreateMgrUI();
   
   // 销毁添加界面
   void DestroyMgrUI();

   // 创建修改界面
   BOOL CreateModifyUI();

   // 销毁修改界面
   void DestroyModifyUI();

   // 获取设备列表
   void ObtainDeviceList(DeviceList& deviceList);

   // 初始化摄像头管理
   bool InitCameraMgrUI();

   // 更新摄像头管理UI
   void UpdateCameraMgrUI();

   // 列表显示规则(选中两个其他禁用，小于两个其他开启)
   void ListShowCheck();

   // 处理设备选项保存
   //void DealOptionSave(void* apData, DWORD adwLen);

   //处理显示摄像头列表
   void DealShowDesktopCameraList(void* apData, DWORD adwLen);
private:
   CameraMgrUI* m_pCameraMgrUI = nullptr;              //添加界面
   CameraModifyUI* m_pCameraModifyUI = nullptr;        //修改界面
   CameraSettingUI* m_pCameraSettingUI = nullptr;      //摄像头高级选项
   VHDesktopVideo *m_pCameraVideo = NULL;
   DesktopCameraRenderWdg* mDesktopCameraRender[2] = {nullptr};
   
   DeviceList m_CameraItemMap;
   QMutex m_mutex;
   BOOL m_bShowState;
   bool m_deviceReset = true;
   bool mIsDesktop = false;
};

#endif //__CAMERA_MGR_LOGIC__H_INCLUDE__
