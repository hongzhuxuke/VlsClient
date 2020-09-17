#ifndef CSCREENSHARETOOLWGD_H
#define CSCREENSHARETOOLWGD_H

#include <QWidget>
#include "ui_CScreenShareToolWgd.h"

#define DEF_WDG_WIDTH   67

class QPropertyAnimation;
class BtnMenuWdg;

class CScreenShareToolWgd : public QWidget {
   Q_OBJECT

public:
   CScreenShareToolWgd(QWidget *parent = 0);
   ~CScreenShareToolWgd();

   void SetHostUser(bool bHost);

   void SetMicState(bool open);
   void SetCameraState(bool open);
   void SetStartLive(bool start);
   void SetPlayerState(bool open);
   bool GetPlayerState();

   void setTencentSDK(bool bTen = true);
signals:
	void Sig_ClickedMicBtn();
	void Sig_ClickedStopShareBtn();
	void Sig_ClickedCameraBtn();
	void Sig_ClickedPlayerBtn();
	void Sig_ClickedStartLiveBtn();
	void Sig_ChatClicked();

	//void sigChatClicked();
	void sigMemberClicked();
	void sigInteractiveClicked();

public slots:
   void Slot_OnClickedPlayerBtn();
protected:
   virtual void enterEvent(QEvent *);
   virtual void leaveEvent(QEvent *);
   virtual void showEvent(QShowEvent *);
private slots:
   void Slot_OnClickedMicBtn();
   void Slot_OnClickedStopShareBtn();
   void Slot_OnClickedCameraBtn();
   
   void Slot_OnClickedStartLive();
   void on_tBtnQuikTool_clicked();
   void slotChatClicked();
   void slotMenuCliecked(const int& iOpType);

private:
   Ui::CScreenShareToolWgd ui;
   bool m_bEnterEvent = false;
   bool m_bHost = true;
   bool mbPlayerOpenState = false;
   bool mbTencentSDK;
   BtnMenuWdg* mPMoreOpMenu = nullptr;
};

#endif // CSCREENSHARETOOLWGD_H
