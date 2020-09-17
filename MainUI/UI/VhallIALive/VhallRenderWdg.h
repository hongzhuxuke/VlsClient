#pragma once

#include <QWidget>
#include <QTimer>
#include <QMouseEvent>
#include "VideoRenderReceiver.h"
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLBuffer>
#include <QTimer>
#include <mutex>
#include "ui_VhallRenderWdg.h"
#include "VhallRenderMaskWdg.h"
#include "MemberCtrlMaskWdg.h"
#include <QImage>
#include <QPixmap>
#include <mutex>
#include <atomic>
#include <QMovie>
#define SMALL_RENDER_VIEW_SIZE	QSize(175, 133)
#define MAIN_RENDER_VIEW_SIZE	QSize(294, 243)

#define MAIN_RENDER_VIEW_WIDTH 320
#define MAIN_RENDER_VIEW_HEIGHT 240

#define SMALL_RENDER_VIEW_WIDTH 175
#define SMALL_RENDER_VIEW_HEIGHT 133

enum LostRateLevel {
   LostRateLevel_High = 0,   //高丢包
   LostRateLevel_Middle = 1, //中丢包
   LostRateLevel_Low = 2     //低丢包
};

enum AudioLevel {
   AudioLevel_1 = 0,
   AudioLevel_2 = 1,
   AudioLevel_3 = 2,
   AudioLevel_4 = 3,
   AudioLevel_5 = 4,
};

class VhallIALiveSettingDlg;

class UIFrameReciver {
public:
   virtual void VideoFrame(const unsigned char* video, int length, int width, int height) = 0;
};


class RTCVideoReciver : public vhall::VideoRenderReceiveInterface {
public:
   virtual void ReceiveVideo(const unsigned char* video, int length, int width, int height);
   void SetReiver(UIFrameReciver* obj);
private:
   UIFrameReciver* mUIFrameReciver = nullptr;
   std::mutex mReciverMutex;
   std::atomic_bool mbRun;
};

class VhallRenderWdg : public QWidget, public UIFrameReciver
{
   Q_OBJECT
public:
   VhallRenderWdg(QWidget *parent = Q_NULLPTR);
   ~VhallRenderWdg();

   
   void VideoFrame(const unsigned char* video, int length, int width, int height);
   /***
   *
   * 参数说明：
   *  userId：参会者id
   *  userName：参会者昵称
   *  userRole：参会者角色
   *  mInRoomRole：登录用户角色
   *  bShowInMainView: 是否显示在大窗中
   *  loginUid： 登录用户id
   ****/
   void InitUserInfo(const QString& userId, const QString& userName, const QString& userRole, const QString& mInRoomRole, bool bShowInMainView, const QString& loginUid);
   void SetUserStreamInfo(QString stream_id, int steam_type);
   bool IsTeacherPos();
   void SetTeacherPos(bool set);
   QString GetStreamId();
   HWND GetRenderWndID();
   void SetViewState(int state);
   int GetViewState();
   void ResetViewSize(bool isHost);
   void SwitchCenterSize(int width, int height);
   bool IsHostRole();
   QString GetUserRole() {
      return mRenderRoleName;
   }

   std::shared_ptr<RTCVideoReciver> GetVideoReciver();
   void SetVhallIALiveSettingDlg(VhallIALiveSettingDlg* settingDlg);
   void ShowVideoLoading(bool show);
   void SetSize(int width, int height);
   void SetUserCameraState(bool open);
   void SetUserMicState(bool open);
   bool GetUserCameraState();
   void SetUserToSpeaker(bool enable);
   void SetUserToShowInMainView(bool enable);
   void ShowSwitchLayoutBtn(bool show = false);
   void ShowSetMainViewBtn(bool show);
   void ShowSetSpeakBtn(bool show);
   void ShowSetKickOutBtn(bool show);
   QString GetUserID();
   QString GetNickName();
   bool GetUserMicState();
   void SetEnableUpdateRendWnd(bool enable);

   const BITMAPINFO& bmi() const { return bmi_; }
   const uint8_t* image() const { return image_.get(); }

   int GetSubStreamErrorCount();
   void SetSubStreamError();
private slots:
   void slot_OnClickedCamera(bool);
   void slot_OnClickedMic(bool);
   void slot_OnClickedSetMainView();
   void slot_OnClickedKickOffRoom();
   void slot_OnClickedSetToSpeaker();
   void slot_MemberCtrlLeave();
   void slot_OnGetPushInfo();
   void slot_SwitchView();
signals:
   void sig_setInMainView(QString, QString);
   void sig_setToSpeaker(QString, QString);
   void sig_ClickedCamera(QString, bool);
   void sig_ClickedMic(QString, bool);
   void sig_NotToSpeak(QString);//下麦
   void sig_SwitchView();

protected:
   virtual void enterEvent(QEvent *event);
   virtual void leaveEvent(QEvent *event);
   virtual void showEvent(QShowEvent *event);

   void RenderIcon();
   void InitNickNamePic();
private:
   QString mRenderRoleName;  //显示区域用户角色
   QString mRenderUserId;	  //显示区域用户ID
   QString mRenderUserName;  //显示区域用户昵称

   HWND mRenderWndID = NULL;

   bool mIsMainView = false;	//当前窗体是否显示在主窗体。
   QString mLoginRoomRole;			//当前登录用户在房间中的角色。
   QString mLoginUid;         //登陆当前房间的用户id

   Ui::VhallRenderWdg ui;
   MemberCtrlMaskWdg *mpMemberCtrlWdg = NULL;
   VhallRenderMaskWdg *mpVhallRenderMaskWdg = NULL;
   int mnStateType = 0;
   bool mbCameraOpen = true;
   bool mbMicOpen = true;
   QMovie* mQMovie = nullptr;
   QString mStreamId;
   int mStreamType;

   QTimer mTimerInfo;

   BITMAPINFO bmi_;
   std::unique_ptr<uint8_t[]> image_;
   QImage mImage;
   QPixmap mPixmap;

   QPixmap mAudioLevel_0_Pixmap;
   QPixmap mAudioLevel_1_Pixmap;
   QPixmap mAudioLevel_2_Pixmap;
   QPixmap mAudioLevel_3_Pixmap;
   QPixmap mAudioLevel_4_Pixmap;
   QPixmap mAudioLevel_5_Pixmap;

   HBITMAP mAudioLevel_0_BitMap;
   HBITMAP mAudioLevel_1_BitMap;
   HBITMAP mAudioLevel_2_BitMap;
   HBITMAP mAudioLevel_3_BitMap;
   HBITMAP mAudioLevel_4_BitMap;
   HBITMAP mAudioLevel_5_BitMap;

   QPixmap mLostRate_H_Pixmap;
   QPixmap mLostRate_M_Pixmap;
   QPixmap mLostRate_L_Pixmap;

   HBITMAP mLostRate_H_BitMap;
   HBITMAP mLostRate_M_BitMap;
   HBITMAP mLostRate_L_BitMap;

   HBITMAP mNickNameBitMap;
   int mNickNameWidth;
   int mNickNameHeight;
   std::atomic_int mAudioLevel = AudioLevel_5;   //1格：0-0.04  2格：0.04-0.16   3格：0.16-0.36  4格：0.36-0.64  5格：0.64-1
   std::atomic_int mLoastRate = LostRateLevel_Low;    //优0-3%    良 4%-9%   差：>10%

   HWND mRendWnd;
   HWND mNoCameraWnd;

   std::shared_ptr<RTCVideoReciver> mRTCVideoReciver = nullptr;
   std::atomic_bool mbIsRecvData = false;
   std::atomic_ullong mRecvDataTime = 0;

   int mLostPactRate = 0;
   VhallIALiveSettingDlg *mVhallIALiveSettingDlg = nullptr;

   QString mStreamID;
   std::atomic<int> mSubStreamErrorCount = 0;
};
