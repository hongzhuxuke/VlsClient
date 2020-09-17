#pragma once

#include <QDialog>
#include <QMutex>
#include <QTimer>
#include <QThread>
#include "ui_VhallIALiveSettingDlg.h"
#include "VhallUI_define.h"
#include "WebRtcSDKInterface.h"
#include "vlive_def.h"
#include "VhallRenderWdg.h"
#include "..\..\..\vhall_live_core\libdshowcapture\source\dshow-base.hpp"

class VhallWaiting;
class VhallIALiveSettingDlg;
class NoiseTooltip;
//class VhallIALive;

enum SettingPageIndex {
   video_page = 0,
   audio_page = 1,
   ai_page = 2,
   layout_page = 3,
   about_feedback = 4,
   aboutUs_page = 5,
   system_page = 6,
};

enum CloseType {
   CloseWithSave = 0,
   CloseNone = 1,
   CloseWithSaveLayoutMode = 2,
};

class ReleaseThread :public QThread {
   Q_OBJECT
public:
   ReleaseThread(QObject *parent = NULL);
   ~ReleaseThread();

   void SetSubscribe(VhallIALiveSettingDlg* sub, CloseType type);

protected:
   virtual void run();
   VhallIALiveSettingDlg *mParamPtr = NULL;
   CloseType mCloseType;
};

#define High_Pro_Tips           QString::fromStdWString(L"高清")
#define Standard_Pro_Tips       QString::fromStdWString(L"标清")
#define Fluency_Pro_Tips        QString::fromStdWString(L"流畅")
#define NO_DEVICE               QString::fromStdString("do_dev")

class VhallIALiveSettingDlg : public QDialog, public UIFrameReciver
{
   Q_OBJECT

public:
   VhallIALiveSettingDlg(QWidget *parent = Q_NULLPTR, bool host = false, QString definition = "");
   ~VhallIALiveSettingDlg();
   void VideoFrame(const unsigned char* video, int length, int width, int height);

   void VideoFrameFromUsing(const unsigned char* video, int length, int width, int height);
   void RegisterListener(DeviceChangedListener* listener);
   void RegisterPlayMediaFileObj(IPlayMediaFileObj* playMediaFile);
   void SetCurrentDeviceInfo(const QList<VhallLiveDeviceInfo>& micList, const QString& curMicId, const QList<VhallLiveDeviceInfo>& cameraList, const QString& curCameraId, const QList<VhallLiveDeviceInfo>& playerList, const QString& curPlayerId);
   void SerVersion(const QString& version);
   void HandleReleaseStream(int type);
   LAYOUT_MODE GetLayOutMode();
   void SetLayOutMode(LAYOUT_MODE mode);
   void SetLastLayoutMode();
   void SetCopyrightInfo(QString qsAppName, QString qsCopyright);
   void SetStartLiving(bool start);
   void ResetProfile(int index);
   void UpdateStartCaptureInfo(QString guid, int videoProfile);

   void PreviewCameraCallback(bool succuss);
   bool CheckPicEffectiveness(const QString filePath);
   void ShowDesktopCaptrue(bool show);
   void OnOpenPlayer(bool bOpen);
   void setPlayerVolume(const int& iValue);
   bool IsOpenCameraSuccess();
   void SetCamerCaptureFailed();
   static VideoProfileIndex GetPushStreamProfileByStreamType(int streamType);
   static VideoProfileIndex GetPushPicStreamProfile(VideoProfileIndex camera_profile);
   static VideoProfileIndex GetPlayMediaFileProfile(QString file);
   static BroadCastVideoProfileIndex GetBroadCastVideoProfileIndex();


signals:
   void sig_PlayerBtnClicked();
   void sigVolumnChanged(int);
   void sigProfileChanged();

public slots:
   void slot_SelectTiledMode();
protected:
   virtual void customEvent(QEvent*);
   virtual void showEvent(QShowEvent *event);
   virtual bool eventFilter(QObject *obj, QEvent *event);
   void mousePressEvent(QMouseEvent *event);
   void mouseMoveEvent(QMouseEvent *event);
   void mouseReleaseEvent(QMouseEvent *);

   void SetSize(int width, int height);
   void RenderWnd(HWND wnd, const unsigned char* video, int length, int width, int height);
   int GetLevelFromSlider(int value);
private slots:
   void slot_OnSelectAudioPage();
   void slot_OnSelectVideoPage();
   void slot_OnSelectAIPage();
   void slot_OnSelectAboutUsPage();
   void slot_OnChangeLayoutPage();
   void slot_OnClickedApply();
   void slot_OnClickedOK();
   void slot_OnClickedClose();
   void slot_SelectPushPic();
   void slot_OpenPushPic(bool checked);
   void slot_RadioCamera(bool checked);
   void slot_OpenDesktopAudio(bool checked);
   void slot_OnMicMuteClick();
   void slot_OnSpeakerVolumeChange(int value);
   void slot_SelectplayCapture(int index);
   void slot_CurrentCameraSelectChanged(int index);
   void slot_CurrentBeautyCameraSelectChanged(int index);
   void slot_CurrentMicSelectChanged(int index);
   void slot_CurrentPlayerSelectChanged(int index);
   void slot_CurrentDesktopProfileChanged(int index);
   void OnBeautyValueChange(int value);
   void OnDesktopEnhanceChanged(int);
   void OnMaxLength();
   void slot_OnAdvancedSetting();
   void slot_localDefinitionChanged(int index);
   void slot_OnChangeFeedBackPage();
   void slot_SelectFloatMode();
   void slot_OnSelectSystemPage();
   void slot_SelectGridMode();

   void HandleGetCamera(QString camerId);
   void HandleGetCameraSuc();
   void HandleGetCameraErr();
   void HandleGetCaptureFrame();
   void on_chkbDesktopAudio_stateChanged(int state);

   void LoadPrivewPic(const QString picPath);
   IBaseFilter* GetDeviceByValue(const IID &enumType, const WCHAR* lpType, const WCHAR* lpName, const WCHAR* lpType2, const WCHAR* lpName2);
private:
   Ui::VhallIALiveSettingDlg ui;

   QString mCurCameraDevID;
   QString mCurMicDevID;
   QString mCurPlayerID;
   QString mCapturePlayUid;

   DeviceChangedListener* mpDevChangedListener = NULL;
   bool mbInitSuc = false;
   bool mbEnableUserSelectCamera = false;
   bool mbGetCameraEnd = true;
   std::atomic_bool mbGetCameraSuccess = false;

   int mLastCameraIndex = -1;
   int mLastMicIndex = -1;
   int mLastPlayerIndex = -1;

   QDialog *mpExitWaiting = NULL;
   QDialog *mpSaveWaiting = NULL;
   ReleaseThread mReleaseThread;
   IPlayMediaFileObj* mpMediaFile = NULL;
   LAYOUT_MODE meLayoutMode = LAYOUT_MODE_TILED;
   LAYOUT_MODE meLastLayoutMode = LAYOUT_MODE_TILED;
   static bool mbIsHost;
   NoiseTooltip* m_pNoiseTips = NULL;

   bool mbIsStartLiving;
   int mLocalVideoDefinition;
   QString mRemoteVideoDefinition;

   QString mStartCaptureDevId;
   int mStartCaptureVideoProfile;
   //std::list<vhall::VideoDevProperty> camerDevList;

   std::list<vlive::CameraDetailsInfo> camerDevList;

   int mCapturePlayIndex;
   int mCapturePlayValue = -1;

   QWidget *parentWdg = nullptr;
   QRect mNormalRect;

   QPoint mPressPoint;
   bool mIsMoved = false;
   bool m_bIsEnableMove = true;

   std::shared_ptr<RTCVideoReciver> mRTCVideoReciver = nullptr;
   BITMAPINFO bmi_;
   std::unique_ptr<uint8_t[]> image_;
   HWND mPlayView = nullptr;
   HWND mBeautyView = nullptr;
   std::atomic_bool mbRecvData = false;
   int mLastLevel = 0;
};


