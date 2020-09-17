#pragma once
#include <QDialog>
#include <QWidget>
#include "ui_DeviceTestingWdg.h"
#include "VhallUI_define.h"
#include "LiveEventInterface.h"
#include "VhallIALiveSettingDlg.h"
#include <thread>
#include <QMovie>
#include <QKeyEvent>
#include "pub.Const.h"
#include "MainUIIns.h"
#include "Msg_MainUI.h"
#include "Msg_CommonToolKit.h"
#include "Msg_OBSControl.h"
#include "Msg_VhallRightExtraWidget.h"
#include "ICommonData.h"
#include "IOBSControlLogic.h"
#include "IDeviceManager.h"
#include "IDShowPlugin.h"
#include "IDeckLinkDevice.h"
#include "TypeDeff.h"
#include "MediaDefs.h"
#include "VH_ConstDeff.h"
#include "CRPluginDef.h"
#include "pub.Const.h"
#include "SystemSettingDlg.h"
#include "IPlayerEngine.h"


enum DEVICE_PAGE_INDEX {
	DEVICE_CAMERA = 0,
	DEVICE_MIC,
	DEVICE_SPEAKER,
	DEVICE_RESULT,
};

class DevInfo {
public:
   std::string mDevId;
   int index;
};

Q_DECLARE_METATYPE(DevInfo)

class DeviceTestingWdg : public QDialog
{
	Q_OBJECT

public:
	DeviceTestingWdg(int liveType = eLiveType_VhallActive, QWidget *parent = Q_NULLPTR);
	~DeviceTestingWdg();

   void SetRole(bool is_host);
	void StartPreView();
   void HandlePreviewCamera(bool success,int code);
   static void ThreadProFun(void *);
   void ProcessFun();
   void SetMicVolume(int value);



protected:
   void mousePressEvent(QMouseEvent *);
   void mouseMoveEvent(QMouseEvent *);
   void mouseReleaseEvent(QMouseEvent *);
   void keyPressEvent(QKeyEvent *e);

private:
   void RtcLiveDevice();
   void VideoLiveDevice();

   void RtcLiveStartPreView();
   void VideoLiveStartPreView();

   void VideoLiveStopPreview();

   void AddCapture();

   void RtcLiveGetDevices();
   void VideoLiveGetDevice();

   void GetVideoFormat(DeviceInfo cameraDevInfo);

private slots:
	void slot_Testing();
	void on_cmbcamera_currentIndexChanged(int index);
   void slot_DesktopCaptureValueChanged(int value);
	void slot_TestScuess();
   void slot_currentChanged(int);
	void slot_TestFailed();
	void slot_TestPlay();
   void slot_PlayFileTimeout();
	void slot_CurrentMicSelectChanged(int);
	void slot_CurrentPlayerSelectChanged(int);
   void slot_CurrentCameraFormatChanged(int index);
   void slot_CheckRendData();
   void slot_CurrentMicHasVolume();
	void slot_ReDevTesting();
	void slot_GoClass();
	void slot_Help();
   void slot_Close();

protected:
   virtual void customEvent(QEvent* e);
private:
	Ui::DeviceTestingWdg ui;
   int mLiveType;

	bool mbTestCamera = false;
	bool mbTestMic = false;
	bool mbTestSpeaker = false;
   std::thread *mDevThread = nullptr;

   QList<VhallLiveDeviceInfo> playerDevList;
   QList<VhallLiveDeviceInfo> cameraDevList;
   QList<VhallLiveDeviceInfo> micDevList;
   QMovie *mQMovie = nullptr;

   bool m_bIsEnableMove = true;
   QPoint mPressPoint;
   bool mIsMoved = false;
   bool mIsPlayFile = false;
   QTimer *mPlayingTimer = nullptr;

   int     m_iCalCursorPos;
   bool    m_bLeftPress;
   QRect   m_rtPreGeometry;
   QPoint  m_ptViewMousePos;

   //摄像设备列表
   DeviceList m_CameraItemMap;
   DeviceList m_oMicList;
   DeviceList m_oSpeakerList;
   DeviceInfo m_currentMic;
   DeviceInfo m_currentSpeaker;
   //帧率列表
   FrameInfoList frameInfoList;
   CameraSettingUIResolutionList m_resolutionList;
   //当前帧率
   FrameInfo currentFrameInfo;
   DeviceInfo m_LastCameraDevInfo;

   QTimer* mCheckRendDataTimer = nullptr;
   vlive::IPlayerEngine* mIPlayerEngine = nullptr;
};
