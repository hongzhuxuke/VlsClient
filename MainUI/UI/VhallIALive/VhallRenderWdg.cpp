#include "VhallRenderWdg.h"
#include "VhallUI_define.h"
#include <QDebug>
#include "DebugTrace.h"
#include "arraysize.h"
#include "vhalluserinfomanager.h"
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QDebug>
#include <QtWinExtras>
#include "pathManage.h"
#include "ConfigSetting.h"
#include "pathmanager.h"
#include "WebRtcSDKInterface.h"
#include <QDateTime>
#include <windows.h>
#include "MemberCtrlMaskWdg.h"
#include "VhallIALiveSettingDlg.h"
#pragma  comment(lib,"Msimg32.lib") //TransparentBlt函数包含的库文件

#define VERTEXIN 0
#define TEXTUREIN 1
using namespace vlive;

void RTCVideoReciver::ReceiveVideo(const unsigned char* video, int length, int width, int height) {
   std::unique_lock<std::mutex> lock(mReciverMutex);
   if (mUIFrameReciver && mbRun) {
      mUIFrameReciver->VideoFrame(video, length, width, height);
   }

}
void RTCVideoReciver::SetReiver(UIFrameReciver* obj){
   std::unique_lock<std::mutex> lock(mReciverMutex);
   obj == nullptr ? mbRun = false : mbRun = true;
   mUIFrameReciver = obj;
}

VhallRenderWdg::VhallRenderWdg(QWidget *parent)
   : QWidget(parent)
{
   ui.setupUi(this);
   mpMemberCtrlWdg = new MemberCtrlMaskWdg(this);
   if (mpMemberCtrlWdg) {
      mpMemberCtrlWdg->hide();
      connect(mpMemberCtrlWdg, SIGNAL(sig_clickedCamera(bool)), this, SLOT(slot_OnClickedCamera(bool)));
      connect(mpMemberCtrlWdg, SIGNAL(sig_clickedMic(bool)), this, SLOT(slot_OnClickedMic(bool)));
      connect(mpMemberCtrlWdg, SIGNAL(sig_clickedSetMainView()), this, SLOT(slot_OnClickedSetMainView()));
      connect(mpMemberCtrlWdg, SIGNAL(sig_clickedKickOffRoom()), this, SLOT(slot_OnClickedKickOffRoom()));
      connect(mpMemberCtrlWdg, SIGNAL(sig_clickedSetToSpeaker()), this, SLOT(slot_OnClickedSetToSpeaker()));
      connect(mpMemberCtrlWdg, SIGNAL(sig_leave()), this, SLOT(slot_MemberCtrlLeave()));
      connect(mpMemberCtrlWdg, SIGNAL(sig_switchView()), this, SLOT(slot_SwitchView()));
   }
   mpVhallRenderMaskWdg = new VhallRenderMaskWdg(this);
   if (mpVhallRenderMaskWdg) {
      mpVhallRenderMaskWdg->hide();
   }

   ZeroMemory(&bmi_, sizeof(bmi_));
   bmi_.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
   bmi_.bmiHeader.biPlanes = 1;
   bmi_.bmiHeader.biBitCount = 32;
   bmi_.bmiHeader.biCompression = BI_RGB;
   bmi_.bmiHeader.biWidth = 1;
   bmi_.bmiHeader.biHeight = -1;
   bmi_.bmiHeader.biSizeImage = 1 * 1 * (bmi_.bmiHeader.biBitCount >> 3);

   bool ret = mImage.load(":/check/images/device_check/icon_maikefeng_1.bmp");
   mImage = mImage.scaled(16,16, Qt::KeepAspectRatio, Qt::FastTransformation);
   mAudioLevel_1_Pixmap = QPixmap::fromImage(mImage);
   ret = mImage.load(":/check/images/device_check/icon_maikefeng_2.bmp");
   mImage = mImage.scaled(16, 16, Qt::KeepAspectRatio, Qt::FastTransformation);
   mAudioLevel_2_Pixmap = QPixmap::fromImage(mImage);
   ret = mImage.load(":/check/images/device_check/icon_maikefeng_3.bmp");
   mImage = mImage.scaled(16, 16, Qt::KeepAspectRatio, Qt::FastTransformation);
   mAudioLevel_3_Pixmap = QPixmap::fromImage(mImage);
   ret = mImage.load(":/check/images/device_check/icon_maikefeng_4.bmp");
   mImage = mImage.scaled(16, 16, Qt::KeepAspectRatio, Qt::FastTransformation);
   mAudioLevel_4_Pixmap = QPixmap::fromImage(mImage);
   ret = mImage.load(":/check/images/device_check/icon_maikefeng_5.bmp");
   mImage = mImage.scaled(16, 16,Qt::KeepAspectRatio, Qt::FastTransformation);
   mAudioLevel_5_Pixmap = QPixmap::fromImage(mImage);
   ret = mImage.load(":/check/images/device_check/icon_maikefeng_0.bmp");
   mImage = mImage.scaled(16, 16, Qt::KeepAspectRatio, Qt::FastTransformation);
   mAudioLevel_0_Pixmap = QPixmap::fromImage(mImage);

   mAudioLevel_0_BitMap = QtWin::toHBITMAP(mAudioLevel_0_Pixmap);
   mAudioLevel_1_BitMap = QtWin::toHBITMAP(mAudioLevel_1_Pixmap);
   mAudioLevel_2_BitMap = QtWin::toHBITMAP(mAudioLevel_2_Pixmap);
   mAudioLevel_3_BitMap = QtWin::toHBITMAP(mAudioLevel_3_Pixmap);
   mAudioLevel_4_BitMap = QtWin::toHBITMAP(mAudioLevel_4_Pixmap);
   mAudioLevel_5_BitMap = QtWin::toHBITMAP(mAudioLevel_5_Pixmap);

   ret = mImage.load(":/check/images/device_check/icon_net_low.bmp");
   mImage = mImage.scaled(16, 16, Qt::KeepAspectRatio, Qt::FastTransformation);
   mLostRate_H_Pixmap = QPixmap::fromImage(mImage);
   ret = mImage.load(":/check/images/device_check/icon_net_m.bmp");
   mImage = mImage.scaled(16, 16, Qt::KeepAspectRatio, Qt::FastTransformation);
   mLostRate_M_Pixmap = QPixmap::fromImage(mImage);
   ret = mImage.load(":/check/images/device_check/icon_net_h.bmp");
   mImage = mImage.scaled(16, 16, Qt::KeepAspectRatio, Qt::FastTransformation);
   mLostRate_L_Pixmap = QPixmap::fromImage(mImage);


   mLostRate_H_BitMap = QtWin::toHBITMAP(mLostRate_H_Pixmap);
   mLostRate_M_BitMap = QtWin::toHBITMAP(mLostRate_M_Pixmap);
   mLostRate_L_BitMap = QtWin::toHBITMAP(mLostRate_L_Pixmap);

   connect(&mTimerInfo, SIGNAL(timeout()), this, SLOT(slot_OnGetPushInfo()));

   mRendWnd = (HWND)(ui.widget_rendView->winId());
   mNoCameraWnd = (HWND)(ui.widget_ctrl->winId());

   mRTCVideoReciver = std::make_shared<RTCVideoReciver>();
   mRTCVideoReciver->SetReiver(this);

   mbIsRecvData = true;
   mRecvDataTime = QDateTime::currentDateTime().toMSecsSinceEpoch();

   mQMovie = new QMovie(":/check/images/device_check/loading.gif");
   ui.label_gif->setMovie(mQMovie);
   ui.label_gif->hide();
   ui.label_loadText->hide();
}

VhallRenderWdg::~VhallRenderWdg(){
   if (mQMovie) {
      mQMovie->stop();
      delete mQMovie;
   }
   mRTCVideoReciver->SetReiver(nullptr);
   mRTCVideoReciver.reset();
   TRACE6("VhallRenderWdg::~VhallRenderWdg delete render uid:%s\n", mRenderUserId.toStdString().c_str());
   TRACE6("VhallRenderWdg::~VhallRenderWdg mpMemberCtrlWdg end\n");
   if (mpVhallRenderMaskWdg) {
      delete mpVhallRenderMaskWdg;
      mpVhallRenderMaskWdg = NULL;
   }

   if (mpMemberCtrlWdg) {
      disconnect(mpMemberCtrlWdg, SIGNAL(sig_clickedCamera(bool)), this, SLOT(slot_OnClickedCamera(bool)));
      disconnect(mpMemberCtrlWdg, SIGNAL(sig_clickedMic(bool)), this, SLOT(slot_OnClickedMic(bool)));
      disconnect(mpMemberCtrlWdg, SIGNAL(sig_clickedSetMainView()), this, SLOT(slot_OnClickedSetMainView()));
      disconnect(mpMemberCtrlWdg, SIGNAL(sig_clickedKickOffRoom()), this, SLOT(slot_OnClickedKickOffRoom()));
      disconnect(mpMemberCtrlWdg, SIGNAL(sig_clickedSetToSpeaker()), this, SLOT(slot_OnClickedSetToSpeaker()));
      delete mpMemberCtrlWdg;
      mpMemberCtrlWdg = NULL;
   }
   ::DeleteObject(mAudioLevel_0_BitMap);
   ::DeleteObject(mAudioLevel_1_BitMap);
   ::DeleteObject(mAudioLevel_2_BitMap);
   ::DeleteObject(mAudioLevel_3_BitMap);
   ::DeleteObject(mAudioLevel_4_BitMap);
   ::DeleteObject(mAudioLevel_5_BitMap);

   ::DeleteObject(mLostRate_H_BitMap);
   ::DeleteObject(mLostRate_M_BitMap);
   ::DeleteObject(mLostRate_L_BitMap);
   TRACE6("VhallRenderWdg::~VhallRenderWdg mpVhallRenderMaskWdg end\n");
}

void VhallRenderWdg::SetSize(int width, int height) {
   if (width == bmi_.bmiHeader.biWidth && -height == bmi_.bmiHeader.biHeight) {
      return;
   }

   bmi_.bmiHeader.biWidth = width;
   bmi_.bmiHeader.biHeight = -height;
   bmi_.bmiHeader.biSizeImage = width * height *
      (bmi_.bmiHeader.biBitCount >> 3);
}


std::shared_ptr<RTCVideoReciver> VhallRenderWdg::GetVideoReciver() {
   return mRTCVideoReciver;
}

void VhallRenderWdg::SetVhallIALiveSettingDlg(VhallIALiveSettingDlg* settingDlg) {
   mVhallIALiveSettingDlg = settingDlg;
}

void VhallRenderWdg::ShowVideoLoading(bool show) {
   if (show) {
      if (mQMovie) {
         mQMovie->start();
         ui.label_gif->show();
      }
      ui.label_loadText->show();
   }
   else {
      if (mQMovie) {
         mQMovie->stop();
      }
      ui.label_gif->hide();
      ui.label_loadText->hide();
   }
}

void VhallRenderWdg::VideoFrame(const unsigned char* video, int input_length, int input_width, int input_height) {
   mSubStreamErrorCount = 0;
   if (mVhallIALiveSettingDlg && !mVhallIALiveSettingDlg->isHidden()) {
      mVhallIALiveSettingDlg->VideoFrameFromUsing(video, input_length, input_width, input_height);
   }
   SetSize(input_width, input_height);
   mbIsRecvData = true;
   mRecvDataTime = QDateTime::currentDateTime().toMSecsSinceEpoch();
   HWND wnd_ = mRendWnd;
   RECT rc;
   ::GetClientRect(wnd_, &rc);

   const BITMAPINFO& bmi = this->bmi();
   int height = abs(bmi.bmiHeader.biHeight);
   int width = bmi.bmiHeader.biWidth;
   double screen_dipSize = ConfigSetting::GetCurrentScreenDPISize(CPathManager::GetConfigPath(), this);
   const uint8_t* image = video;
   if (image != NULL) {
      auto mWindowDC = ::GetDC(wnd_);
      HDC dc_mem = ::CreateCompatibleDC(mWindowDC);
      ::SetStretchBltMode(dc_mem, HALFTONE);

      // Set the map mode so that the ratio will be maintained for us.
      HDC all_dc[] = { mWindowDC, dc_mem };
      for (int i = 0; i < arraysize(all_dc); ++i) {
         SetMapMode(all_dc[i], MM_ISOTROPIC);
         SetWindowExtEx(all_dc[i], width, height, NULL);
         SetViewportExtEx(all_dc[i], rc.right, rc.bottom, NULL);
      }

      HBITMAP bmp_mem = ::CreateCompatibleBitmap(mWindowDC, rc.right, rc.bottom);
      HGDIOBJ bmp_old = ::SelectObject(dc_mem, bmp_mem);

      POINT logical_area = { rc.right, rc.bottom };
      DPtoLP(mWindowDC, &logical_area, 1);

      HBRUSH brush = ::CreateSolidBrush(RGB(0, 0, 0));
      RECT logical_rect = { 0, 0, logical_area.x, logical_area.y };
      ::FillRect(dc_mem, &logical_rect, brush);
      ::DeleteObject(brush);

      int x = (logical_area.x / 2) - (width / 2);
      int y = (logical_area.y / 2) - (height / 2);

      StretchDIBits(dc_mem, x , y , width, height,0, 0, width, height, image, &bmi, DIB_RGB_COLORS, SRCCOPY);
      BitBlt(mWindowDC, 0, 0, logical_area.x, logical_area.y, dc_mem, 0, 0, SRCCOPY);
      RenderIcon();

      ::SelectObject(dc_mem, bmp_old);
      ::DeleteObject(bmp_mem);
      ::DeleteDC(dc_mem);
      ::ReleaseDC(wnd_, mWindowDC);
   }
}

void VhallRenderWdg::InitUserInfo(const QString& userId, const QString& userName, const QString& userRole, const QString& inRoomRole, bool bShowInMainView, const QString& loginUid) {
   mLoginUid = loginUid;
   mLoginRoomRole = inRoomRole;
   mRenderRoleName = userRole;
   mRenderUserId = userId;
   mRenderUserName = userName;
   TRACE6("VhallRenderWdg::InitUserInfo create render uid:%s loginUid:%s\n", userId.toStdString().c_str(), loginUid.toStdString().c_str());
   if (mpVhallRenderMaskWdg) {
      mpVhallRenderMaskWdg->SetUserName(userName);
      mpVhallRenderMaskWdg->SetRoleName(userRole);
   }
   InitNickNamePic();
   //当前登录用户是主持人，根据上麦用户角色判断是否显示对应设置图标。
   if (mLoginRoomRole.compare(USER_HOST) == 0 && mpMemberCtrlWdg) {
      if (userRole.compare(USER_HOST) == 0 && bShowInMainView) {
         mpMemberCtrlWdg->SetCurrentUserRole(Host_InMainView);
      }
      else if (userRole.compare(USER_HOST) == 0 && !bShowInMainView) {
         mpMemberCtrlWdg->SetCurrentUserRole(Host_InListView);
      }

      if (userRole.compare(USER_GUEST) == 0 && bShowInMainView) {
         mpMemberCtrlWdg->SetCurrentUserRole(Guest_InMainView);
      }
      else if (userRole.compare(USER_GUEST) == 0 && !bShowInMainView) {
         mpMemberCtrlWdg->SetCurrentUserRole(Guest_InListView);
      }

      if (userRole.compare(USER_USER) == 0 && bShowInMainView) {
         mpMemberCtrlWdg->SetCurrentUserRole(Other_InMainView);
      }
      else if (userRole.compare(USER_USER) == 0 && !bShowInMainView) {
         mpMemberCtrlWdg->SetCurrentUserRole(Other_InListView);
      }
   }
   else if (mpMemberCtrlWdg) { //其他身份。
      mpMemberCtrlWdg->ShowMainView(false);
      mpMemberCtrlWdg->ShowMainSpeaker(false);
      //自己才显示下麦按键
      if (userId.compare(mLoginUid) == 0) {
         mpMemberCtrlWdg->ShowToKickOutRoom(true);
      }
      else {
         mpMemberCtrlWdg->ShowToKickOutRoom(false);
      }
   }
}

bool VhallRenderWdg::IsTeacherPos() {
   return mIsMainView;
}

void VhallRenderWdg::SetTeacherPos(bool set) {
   mIsMainView = set;
   if (mpMemberCtrlWdg) {
      mpMemberCtrlWdg->ShowSwitchLayoutBtn(set);
   }
}

void VhallRenderWdg::SetUserStreamInfo(QString stream_id, int steam_type) {
   mStreamId = stream_id;
   mStreamType = steam_type;
}

QString VhallRenderWdg::GetStreamId() {
   return mStreamId;
}

int VhallRenderWdg::GetSubStreamErrorCount() {
   return mSubStreamErrorCount;
}
void VhallRenderWdg::SetSubStreamError() {
   mSubStreamErrorCount++;
   if (mSubStreamErrorCount > 10) {
      mSubStreamErrorCount = 10;
   }
}

void VhallRenderWdg::SetEnableUpdateRendWnd(bool enable) {
   TRACE6("%s uid:%s enable:%d\n", __FUNCTION__, mRenderUserId.toStdString().c_str(), enable);
   ui.widget_rendView->setUpdatesEnabled(enable);
}

HWND VhallRenderWdg::GetRenderWndID() {
   HWND id = (HWND)(ui.widget_rendView->winId());
   return id;
}

void VhallRenderWdg::SetViewState(int state) {
   switch (state) {
   case RenderView_NoJoin: {
      ui.label_viewState->setStyleSheet("border-image:url(:/interactivity/img/vhallactive/head_normal.png);");
      ui.label_viewState->show();
      ui.stackedWidget->setCurrentIndex(1);
      break;
   }
   case RenderView_NoCamera: {
      ui.label_viewState->setStyleSheet("border-image: url(:/vhallActive/img/vhallactive/camera_close.png);");
      ui.label_viewState->show();
      ui.stackedWidget->setCurrentIndex(1);
      break;
   }
   default: {
      ui.stackedWidget->setCurrentIndex(0);
      mTimerInfo.start(1000);
      break;
   }
   }
   mnStateType = state;
   TRACE6("%s uid:%s mnStateType:%d \n", __FUNCTION__, mRenderUserId.toStdString().c_str(), mnStateType);
}

void VhallRenderWdg::SwitchCenterSize(int width, int height) {
   this->setMaximumHeight(height);
   this->setMaximumWidth(width);
   this->setMinimumHeight(height);
   this->setMinimumWidth(width);
   this->setFixedWidth(width);
   this->setFixedHeight(height);
   this->resize(width, height);
   if (mpMemberCtrlWdg) {
      mpMemberCtrlWdg->setMaximumHeight(height);
      mpMemberCtrlWdg->setMaximumWidth(width);
      mpMemberCtrlWdg->setMinimumHeight(height);
      mpMemberCtrlWdg->setMinimumWidth(width);
      mpMemberCtrlWdg->setFixedWidth(width);
      mpMemberCtrlWdg->setFixedHeight(height);
      mpMemberCtrlWdg->resize(width, height);
   }
}

void VhallRenderWdg::ResetViewSize(bool isHostView) {
   if (mpVhallRenderMaskWdg) {
      mpVhallRenderMaskWdg->SetResetSize(isHostView);
   }

   if (isHostView) {
      //ui.label_roleLogo->setFixedSize(QSize(20, 22));
      this->setMaximumHeight(MAIN_RENDER_VIEW_HEIGHT);
      this->setMaximumWidth(MAIN_RENDER_VIEW_WIDTH);
      this->setMinimumHeight(MAIN_RENDER_VIEW_HEIGHT);
      this->setMinimumWidth(MAIN_RENDER_VIEW_WIDTH);
      this->setFixedWidth(MAIN_RENDER_VIEW_WIDTH);
      this->setFixedHeight(MAIN_RENDER_VIEW_HEIGHT);
      this->resize(MAIN_RENDER_VIEW_WIDTH, MAIN_RENDER_VIEW_HEIGHT);
      if (mpMemberCtrlWdg) {
         mpMemberCtrlWdg->setMaximumHeight(MAIN_RENDER_VIEW_HEIGHT);
         mpMemberCtrlWdg->setMaximumWidth(MAIN_RENDER_VIEW_WIDTH);
         mpMemberCtrlWdg->setMinimumHeight(MAIN_RENDER_VIEW_HEIGHT);
         mpMemberCtrlWdg->setMinimumWidth(MAIN_RENDER_VIEW_WIDTH);
         mpMemberCtrlWdg->setFixedWidth(MAIN_RENDER_VIEW_WIDTH);
         mpMemberCtrlWdg->setFixedHeight(MAIN_RENDER_VIEW_HEIGHT);
         mpMemberCtrlWdg->resize(MAIN_RENDER_VIEW_WIDTH, MAIN_RENDER_VIEW_HEIGHT);
      }
   }
   else {
      this->setMaximumHeight(SMALL_RENDER_VIEW_HEIGHT);
      this->setMaximumWidth(SMALL_RENDER_VIEW_WIDTH);
      this->setMinimumHeight(SMALL_RENDER_VIEW_HEIGHT);
      this->setMinimumWidth(SMALL_RENDER_VIEW_WIDTH);
      this->setFixedWidth(SMALL_RENDER_VIEW_WIDTH);
      this->setFixedHeight(SMALL_RENDER_VIEW_HEIGHT);
      this->resize(SMALL_RENDER_VIEW_WIDTH, SMALL_RENDER_VIEW_HEIGHT);
      if (mpMemberCtrlWdg) {
         mpMemberCtrlWdg->setMaximumHeight(SMALL_RENDER_VIEW_HEIGHT);
         mpMemberCtrlWdg->setMaximumWidth(SMALL_RENDER_VIEW_WIDTH);
         mpMemberCtrlWdg->setMinimumHeight(SMALL_RENDER_VIEW_HEIGHT);
         mpMemberCtrlWdg->setMinimumWidth(SMALL_RENDER_VIEW_WIDTH);
         mpMemberCtrlWdg->setFixedWidth(SMALL_RENDER_VIEW_WIDTH);
         mpMemberCtrlWdg->setFixedHeight(SMALL_RENDER_VIEW_HEIGHT);
         mpMemberCtrlWdg->resize(SMALL_RENDER_VIEW_WIDTH, SMALL_RENDER_VIEW_HEIGHT);
      }
   }
}

void VhallRenderWdg::showEvent(QShowEvent *event) {
   int a;
   a = 0;
}

bool VhallRenderWdg::IsHostRole() {
   if (mRenderRoleName.compare(USER_HOST) == 0) {
      return true;
   }
   return false;
}

void VhallRenderWdg::SetUserCameraState(bool open) {
   if (mpMemberCtrlWdg) {
      mpMemberCtrlWdg->SetCameraState(open);
      if (open) {
         SetViewState(RenderView_None);
      }
      else {
         SetViewState(RenderView_NoCamera);
      }
      mbCameraOpen = open;
      TRACE6("%s uid:%s open:%d\n", __FUNCTION__, mRenderUserId.toStdString().c_str(), open);
   }
}

bool VhallRenderWdg::GetUserCameraState() {
   return mbCameraOpen;
}

bool VhallRenderWdg::GetUserMicState() {
   return mbMicOpen;
}

void VhallRenderWdg::SetUserMicState(bool open) {
   if (mpMemberCtrlWdg) {
      mpMemberCtrlWdg->SetMicState(open);
      mbMicOpen = open;
   }
   TRACE6("%s open:%d\n", __FUNCTION__, open);
}
void VhallRenderWdg::SetUserToSpeaker(bool enable) {

}
void VhallRenderWdg::SetUserToShowInMainView(bool enable) {

}
void VhallRenderWdg::ShowSetMainViewBtn(bool show) {
   if (mpMemberCtrlWdg) {
      mpMemberCtrlWdg->ShowMainView(show);
   }
}

void VhallRenderWdg::ShowSwitchLayoutBtn(bool show) {
   if (mpMemberCtrlWdg) {
      mpMemberCtrlWdg->ShowSwitchLayoutBtn(show);
   }
}

void VhallRenderWdg::ShowSetSpeakBtn(bool show) {
   if (mpMemberCtrlWdg) {
      mpMemberCtrlWdg->ShowMainSpeaker(show);
   }
}
void VhallRenderWdg::ShowSetKickOutBtn(bool show) {
   if (mpMemberCtrlWdg) {
      mpMemberCtrlWdg->ShowToKickOutRoom(show);
   }
}

QString VhallRenderWdg::GetUserID() {
   return mRenderUserId;
}

QString VhallRenderWdg::GetNickName() {
   return mRenderUserName;
}

void VhallRenderWdg::enterEvent(QEvent *event) {
   if (mpMemberCtrlWdg && (mLoginRoomRole.compare(USER_HOST) == 0 || mLoginUid.compare(mRenderUserId) == 0 || mIsMainView)) {
      if (mLoginRoomRole.compare(USER_GUEST) == 0) {
         if (mLoginUid.compare(mRenderUserId) != 0) {
            mpMemberCtrlWdg->HideDeviceCtrl(true);
         }
         else {
            mpMemberCtrlWdg->HideDeviceCtrl(false);
         }
      }
      mpMemberCtrlWdg->show();
   }
   //if (mpVhallRenderMaskWdg) {
   //   mpVhallRenderMaskWdg->show();
   //   mpVhallRenderMaskWdg->move(0, this->height() - mpVhallRenderMaskWdg->height());
   //}
   QWidget::enterEvent(event);
}

int VhallRenderWdg::GetViewState() {
   return mnStateType;
}

void VhallRenderWdg::leaveEvent(QEvent *event) {
   if (mpMemberCtrlWdg && (mLoginRoomRole.compare(USER_HOST) == 0 || mpMemberCtrlWdg && mLoginUid.compare(mRenderUserId) == 0)) {
      mpMemberCtrlWdg->hide();
      if (mnStateType == RenderView_NoJoin) {
         ui.label_viewState->show();
      }
      else if (mnStateType == RenderView_NoCamera) {
         ui.label_viewState->show();
      }
   }
   if (mnStateType != RenderView_NoJoin && mnStateType != RenderView_NoCamera) {
      ui.widget_rendView->show();
   }
   if (mpVhallRenderMaskWdg) {
      mpVhallRenderMaskWdg->hide();
   }
   QWidget::enterEvent(event);
}

void VhallRenderWdg::slot_OnClickedCamera(bool curState) {
   emit sig_ClickedCamera(mRenderUserId, curState);
}

void VhallRenderWdg::slot_OnClickedMic(bool curState) {
   emit sig_ClickedMic(mRenderUserId, curState);
}

void VhallRenderWdg::slot_OnClickedSetMainView() {
   emit sig_setToSpeaker(mRenderUserId, mRenderRoleName);
}

void VhallRenderWdg::slot_OnClickedKickOffRoom() {
   emit sig_NotToSpeak(mRenderUserId);
}

void VhallRenderWdg::slot_OnClickedSetToSpeaker() {
   emit sig_setInMainView(mRenderUserId, mRenderRoleName);
}

void VhallRenderWdg::slot_MemberCtrlLeave() {
   if (mpVhallRenderMaskWdg) {
      mpVhallRenderMaskWdg->hide();
   }
}

void VhallRenderWdg::slot_SwitchView() {
   emit sig_SwitchView();
}

void VhallRenderWdg::slot_OnGetPushInfo() {
   int level = GetWebRtcSDKInstance()->GetAudioLevel(mRenderUserId.toStdWString());
   level = level * 100 / 32767;
   if (level <= 0) {
      mAudioLevel = AudioLevel_1;
   }
   else if (level > 0 && level < 16) {
      mAudioLevel = AudioLevel_2;
   }
   else if (level >= 16 && level < 36) {
      mAudioLevel = AudioLevel_3;
   }
   else if (level >= 36 && level < 64) {
      mAudioLevel = AudioLevel_4;
   }
   else if (level >= 64 && level <= 100) {
      mAudioLevel = AudioLevel_5;
   }

   if (mLostPactRate == 0) {
      double lostRate = GetWebRtcSDKInstance()->GetVideoLostRate(mRenderUserId.toStdWString());
      if (lostRate >= 0.0 && lostRate <= 0.03) {
         mLoastRate = LostRateLevel_Low;
      }
      else if (lostRate > 0.03 && lostRate <= 0.09) {
         mLoastRate = LostRateLevel_Middle;
      }
      else if (lostRate > 0.09) {
         mLoastRate = LostRateLevel_High;
      }
   }
   mLostPactRate++;
   if (mLostPactRate == 2) {
      mLostPactRate = 0;
   }
   //TRACE6("%s level %d  lostRate %f\n",__FUNCTION__,level, lostRate);
   //if (mbIsRecvData) {
   //   //当1秒钟之内没有接收到数据，进行状态渲染。
   //   if (QDateTime::currentDateTime().toMSecsSinceEpoch() > mRecvDataTime + 3000) {
   //      RenderIcon();
   //   }
   //}
}

void VhallRenderWdg::InitNickNamePic() {
   //保存路径（包括文件名）
   QString toDir = QString::fromStdWString(GetAppDataPath()) + QString("image\\") + mRenderUserId + QString(".bmp");
   QFile fileTemp(toDir);
   fileTemp.remove();
   QString tempImagePath = toDir;
   //图片上的字符串，例如 HELLO
   if (mRenderUserName.length() > 8) {
      mRenderUserName = mRenderUserName.mid(0,7);
      mRenderUserName = mRenderUserName + QString("..");
   }
   QString imageText = mRenderUserName;
   QFont font;
   //设置显示字体的大小
   double screen_dipSize = ConfigSetting::GetCurrentScreenDPISize(CPathManager::GetConfigPath(), this);
   font.setPixelSize(12 * screen_dipSize);
   QFontMetrics fm(font);
   int charWidth = fm.width(imageText);
   charWidth = fm.boundingRect(imageText).width();
   //指定图片大小为字体的大小
   QSize size(charWidth + 8, 18 * screen_dipSize);
   //以ARGB32格式构造一个QImage
   QImage image(size, QImage::Format_ARGB32);
   //填充图片背景
   image.fill("white");

   //为这个QImage构造一个QPainter
   QPainter painter(&image);
   //设置画刷的组合模式CompositionMode_SourceOut这个模式为目标图像在上。
   //改变组合模式和上面的填充方式可以画出透明的图片。


   //改变画笔和字体
   QPen pen = painter.pen();
   pen.setColor(QColor(220, 220, 220));
   painter.setPen(pen);
   painter.setFont(font);

   //将Hello写在Image的中心
   painter.drawText(image.rect(), Qt::AlignCenter, imageText);
   //image.save(tempImagePath, "bmp", 100);
   QPixmap pixmap = QPixmap::fromImage(image);
   mNickNameWidth = pixmap.width();
   mNickNameHeight = pixmap.height();
   mNickNameBitMap = QtWin::toHBITMAP(pixmap);
}

void VhallRenderWdg::RenderIcon() {
   HWND wnd_ = mRendWnd;
   if (mnStateType == RenderView_NoCamera) {
      wnd_ = mNoCameraWnd;
   }
   HBITMAP hBmp;
   HBITMAP hOldBmp;
   HDC lostDC = GetDC(wnd_);
   HDC hdcsource2 = CreateCompatibleDC(lostDC);
   if (mLoastRate == LostRateLevel_High) {
      hOldBmp = (HBITMAP)::SelectObject(hdcsource2, mLostRate_H_BitMap);
   }
   else if (mLoastRate == LostRateLevel_Middle) {
      hOldBmp = (HBITMAP)::SelectObject(hdcsource2, mLostRate_M_BitMap);
   }
   else if (mLoastRate == LostRateLevel_Low) {
      hOldBmp = (HBITMAP)::SelectObject(hdcsource2, mLostRate_L_BitMap);
   }

   double screen_dipSize = ConfigSetting::GetCurrentScreenDPISize(CPathManager::GetConfigPath(), this);
   ::TransparentBlt(lostDC, (this->width() * screen_dipSize - 26), (this->height() * screen_dipSize - 19), 16, 16, hdcsource2, 0, 0, 16, 16, RGB(0, 0, 255));
   ::SelectObject(hdcsource2, hOldBmp);

   HDC audioDC = GetDC(wnd_);
   HDC  hdcsource1 = CreateCompatibleDC(audioDC);
   if (mbMicOpen) {
      if (mAudioLevel == AudioLevel_1) {
         hOldBmp = (HBITMAP)::SelectObject(hdcsource1, mAudioLevel_1_BitMap);
      }
      else if (mAudioLevel == AudioLevel_2) {
         hOldBmp = (HBITMAP)::SelectObject(hdcsource1, mAudioLevel_2_BitMap);
      }
      else if (mAudioLevel == AudioLevel_3) {
         hOldBmp = (HBITMAP)::SelectObject(hdcsource1, mAudioLevel_3_BitMap);
      }
      else if (mAudioLevel == AudioLevel_4) {
         hOldBmp = (HBITMAP)::SelectObject(hdcsource1, mAudioLevel_4_BitMap);
      }
      else if (mAudioLevel == AudioLevel_5) {
         hOldBmp = (HBITMAP)::SelectObject(hdcsource1, mAudioLevel_5_BitMap);
      }
   }
   else {
      SelectObject(hdcsource1, mAudioLevel_0_BitMap);
   }
   ::TransparentBlt(audioDC, (this->width()* screen_dipSize - 50) , (this->height() * screen_dipSize - 19) , 16, 16, hdcsource1, 0, 0, 16, 16, RGB(0, 0, 255));
   ::SelectObject(hdcsource1, hOldBmp);


   HDC NickDC = GetDC(wnd_);
   HDC hdcsource3 = CreateCompatibleDC(NickDC);
   hOldBmp = (HBITMAP)::SelectObject(hdcsource3, mNickNameBitMap);
   ::TransparentBlt(NickDC, 0, (this->height() - 18) * screen_dipSize, mNickNameWidth, mNickNameHeight, hdcsource3, 0, 0, mNickNameWidth, mNickNameHeight, RGB(255, 255,255));
   ::SelectObject(hdcsource3, hOldBmp);
 

   ::DeleteDC(hdcsource1);
   ::DeleteDC(hdcsource2);
   ::DeleteDC(hdcsource3);
   ::DeleteDC(audioDC);
   ::DeleteDC(lostDC);
   ::DeleteDC(NickDC);
}


