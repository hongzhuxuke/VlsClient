#ifndef VHDESKTOPSHARINGUI_H
#define VHDESKTOPSHARINGUI_H

#include "CWidget.h"
#include <QTimer>
#include <QDialog>
#include "ToolButton.h"
#include "volumedisplay_wiget.h"
#include "VHMonitorCapture.h"
#include "DesktopCameraCtrl.h"

class BtnMenuWdg;
//class NoiseTooltip;

namespace Ui {
   class VHDesktopSharingUI;
}

class VHDesktopSharingUI : public QDialog
{
   Q_OBJECT

public:
   explicit VHDesktopSharingUI(QDialog *parent = 0);
   ~VHDesktopSharingUI();
   void Show();
   void Close();
   void SetStreamButtonStatus(bool status);
   void SetMicMute(bool bMute);
   void SetCameraState(bool open);
   void SetShowChat(bool);
   void SetShowShareSelect(const bool show);
   void SetSetBtnShow(const bool& bShow = false);
   void SetPlayerShow(const bool& bShow = false);
   void SetLiveShow(const bool& bShow = true);
   QPoint GetCameraPos();
   int GetCameraHeight();
   void setEnablePlguin(bool enable);
   void SetStartLiveBtnEnable(bool enable);
   bool GetMicMute() { return mbMicMute; }
   bool GetStreamStatus() { return m_bStreamStatus; }
   void SetPlayerState(bool open);
   void SetLiveState(bool isVideoLive);
   void SetCurrentShareScreenInfo(const VHD_WindowInfo& info);
   VHD_WindowInfo GetCurrentShareWindowInfo();
   void SetFloatWndBtnState(bool show);
   void SetPushStreamLost(float lostRate);
   void SetMicVol(int value);
protected:
   void showEvent(QShowEvent *);
   void hideEvent(QHideEvent *);

private:
   void mouseEnter();

signals:
   void sigToStopDesktopSharing();
   //-----------------
   void sigShow();
   void sigClose();
   void sigChatClicked();
   void sigMemberClicked();
   void sigInteractiveClicked(bool isReload, const int& iX, const int& iy);
   void sigPlayer();
   void sigCameraClicked();
   void sigSettingClicked();
   void sigLiveClicked();
   void sigMicClicked();
   void sigShareSelect();
   void sigFloatSwitchBtn();

public slots:
   void slotShow();
   void slotClose();
   void SlottBtnQuikTool();
   void slot_OnClickedPlayer();
   void slot_OnClickedMic();

private slots:
   void on_btnEx_clicked(bool checked /*= false*/);
   void slotMenuCliecked(const int& iOpType);
   void slot_StopSharing();
   void slot_OnCheckSizeChanged();
   void slot_ClickedSwitchBtn();
   void slot_OnClickedCamera();
   void slot_OnSettingClicked();
   void slot_OnShowTimer();

private:
   Ui::VHDesktopSharingUI *ui;
   int m_timeOutCount = 0;
   bool m_bLeave = true;
   bool mbMicMute = false;
   bool m_bStreamStatus = false;
   BtnMenuWdg* mPMoreOpMenu = nullptr;
   bool mbShowChat = true;
   VHD_WindowInfo mCurrentWindowInfo;
   QTimer* mCheckScreenSizeChange = nullptr;
   DesktopCameraCtrl* mActiveLiveFloatCtrl = nullptr;
   bool mIsShow = false;

   QTimer *mShowUITimer = nullptr;
};

#endif // VHDESKTOPSHARINGUI_H
