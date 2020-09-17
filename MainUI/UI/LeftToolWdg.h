#ifndef H_LEFTTOOLWDG_H
#define H_LEFTTOOLWDG_H

#include <QWidget>
#include "ui_LeftToolWdg.h"

enum ClickedBtn {
   ClickedBtn_Doc = 0,
   ClickedBtn_Whiteboard,
};

#define WhiteBoardICon  "QPushButton{border-image:url(:/button/images/borad.png);}"
#define WhiteBoardHoverICon  "QPushButton{border-image:url(:/button/images/borad_hover.png);}"
#define WhiteBoardDisableICon  "QPushButton{border-image: url(:/button/images/borad-disable.png);}"

#define WhiteBoardText "QPushButton{color: rgb(236, 236, 236);border:none;}"
#define WhiteBoardHoverText "QPushButton{color: rgb(252, 86, 89);border:none;}"
#define WhiteBoardTextDisable "QPushButton{color: rgb(153, 153, 153);border:none;}"

class LeftToolWdg : public CWidget
{
   Q_OBJECT

public:
   LeftToolWdg(QWidget *parent = Q_NULLPTR);
   ~LeftToolWdg();

   void SetDocBtnSelect();
   void SetWBBtnSelect();

   void ReInitDocAndWhiteBoradSheet();
   void ReInitDocSelectStatus();
   void ShowSharedButton(bool show = true);
   void GetPluginUrlToolDlgPos(int& x, int& y);
   void ShowWhiteboardBtn(const bool& bShow = true);
   void ShowDesktopButton(bool show);
   void SetDesktopShow(bool show);
   void ShowMemberButoon(bool show);
   void CheckStartMode();
   void SetDocBtnClicked(bool click);

   void setEnablePluginBtn(bool enable);
   void setEnableDesktopBtn(bool enable);
   void setEnableInsertVedio(bool enable);
   void setEnableDocAndWhiteBtn(bool enable);

   void SetEixtUnseeHandsUpList(bool bExit);
   void ShowInsertVedio(const bool& bShow = true);
   bool IsClickedWB();

private:
   void ClickedBtn(int doc);
public slots:
   void slot_OnClickedDoc();
signals:
   void SigSetDesktopSharing(bool);
   void sig_OnClickedDoc();
   void sig_OnClickedDesktop();
   void sig_OnClickedPluginUrl(const int& iX, const int& iY);
   //void sig_OnClickedShareUrl();
   void sigClicked();
   void sigSharedBtnClicked();
   void sig_OnClickedWhiteboard();
   void sigMemberBtnClicked();
   void sig_InsertVedio();
public slots:
   void SetDesktopSharing(bool bShow);
private slots:

   void slot_OnClickedWhiteBorad();
   void slot_OnClickedDesktop(bool);
   void slot_OnClickedPluginUrl();
   //void slot_OnClickedShareUrl(bool);
   void SlotSetDesktopSharing(bool);

   void SlotDesktopClicked();

private:
   bool IsFlashLive();
private:
   Ui::LeftToolWdg ui;
   bool m_bDesktopShow = false;
   bool m_bDesktopSharing = false;
   bool mbEnablePlguin = false;
   bool mbEnableDesktop = false;
   bool mbEnableDocAndWhite = false;
   bool mbClickedWhiteBoard = false;
};
#endif H_LEFTTOOLWDG_H