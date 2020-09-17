#ifndef CAPTURESRCUI_H
#define CAPTURESRCUI_H

#include "VHDialog.h"
#include "ui_CaptureSrcUI.h"

class QHBoxLayout;
class ButtonWdg;
class CaptureSrcUI : public VHDialog {
   Q_OBJECT
public:
   explicit CaptureSrcUI(QDialog *parent = 0);
   ~CaptureSrcUI();
   void SetDesktopShareStatus(bool);
signals:
   void SigSetDesktopShareStatus(bool);
   void sig_hideCaptureSrcUi();

protected:
   void showEvent(QShowEvent *);
   virtual void focusOutEvent(QFocusEvent *);
   virtual void enterEvent(QEvent *);
   virtual void leaveEvent(QEvent *);

public:
   bool Create();
   void Destroy();

private slots:
   void OnDesktopClick();
   void OnSoftwareClick();
   void OnRegionClick();

private:
   Ui::CaptureSrcUI ui;
   bool m_bEnter = false;
};

#endif // CAPTURESRCUI_H
