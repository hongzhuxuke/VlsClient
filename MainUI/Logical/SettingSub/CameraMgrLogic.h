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

   // ������Ϣ
   void DealMessage(DWORD adwMessageID, void* apData, DWORD adwLen);
   // ��ʾ��Ƶ
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
   // ����Click����
   void DealClickControl(void* apData, DWORD adwLen);

   // �����������ͷ
   void DealMgrCamera(void* apData, DWORD adwLen);

   // �����޸�����ͷ
   void DealModifyCamera(void* apData, DWORD adwLen);

   // �����޸ı���
   void DealModifySave(void* apData, DWORD adwLen);

   // �����豸ѡ��
   void DealCameraSelect(void* apData, DWORD adwLen);

   // �����豸ȫ��
   void DealCameraFull(void* apData, DWORD adwLen);

   //����ͷ���
   void DealDeviceChange(void* apData, DWORD adwLen);

   //���������ڵõ�����
   void DealFocusIn(void* apData, DWORD adwLen);

   //�����豸ɾ��
   void DealDeviceDelete(void* apData, DWORD adwLen);

   void DealDesktopShare(void* apData, DWORD adwLen);

   void DealStopLive();
private:
	void showCameraList(int ix, int iy, const bool bFullScreen);
   // ��ʾ�������
   void ShowCameraAddUI(int ix, int iy);

   // ������ӽ���
   BOOL CreateMgrUI();
   
   // ������ӽ���
   void DestroyMgrUI();

   // �����޸Ľ���
   BOOL CreateModifyUI();

   // �����޸Ľ���
   void DestroyModifyUI();

   // ��ȡ�豸�б�
   void ObtainDeviceList(DeviceList& deviceList);

   // ��ʼ������ͷ����
   bool InitCameraMgrUI();

   // ��������ͷ����UI
   void UpdateCameraMgrUI();

   // �б���ʾ����(ѡ�������������ã�С��������������)
   void ListShowCheck();

   // �����豸ѡ���
   //void DealOptionSave(void* apData, DWORD adwLen);

   //������ʾ����ͷ�б�
   void DealShowDesktopCameraList(void* apData, DWORD adwLen);
private:
   CameraMgrUI* m_pCameraMgrUI = nullptr;              //��ӽ���
   CameraModifyUI* m_pCameraModifyUI = nullptr;        //�޸Ľ���
   CameraSettingUI* m_pCameraSettingUI = nullptr;      //����ͷ�߼�ѡ��
   VHDesktopVideo *m_pCameraVideo = NULL;
   DesktopCameraRenderWdg* mDesktopCameraRender[2] = {nullptr};
   
   DeviceList m_CameraItemMap;
   QMutex m_mutex;
   BOOL m_bShowState;
   bool m_deviceReset = true;
   bool mIsDesktop = false;
};

#endif //__CAMERA_MGR_LOGIC__H_INCLUDE__
