#pragma once

#include <QWidget>
#include <QKeyEvent>
#include <QTimer>
#include "ui_FullDocWdg.h"
#include "FullDocEscTipWdg.h"

class FullDocWdg : public QWidget
{
   Q_OBJECT

public:
   FullDocWdg(QWidget *parent = Q_NULLPTR);
   ~FullDocWdg();

   void InsertDoc(QWidget* wdg);
   void RemoveDoc(QWidget* wdg);

private slots:
   void slot_HideEscTip();

protected:
   void showEvent(QShowEvent *event);
   virtual void keyPressEvent(QKeyEvent *event);
   virtual void keyReleaseEvent(QKeyEvent *event);
   //virtual void mousePressEvent(QMouseEvent *event);
   //virtual void mouseReleaseEvent(QMouseEvent *event);
   //virtual void mouseDoubleClickEvent(QMouseEvent *event);
   //virtual void keyReleaseEvent(QKeyEvent *event);
   //virtual void enterEvent(QEvent *event);
   //virtual void leaveEvent(QEvent *event);
signals:
   void sig_PressKeyEsc();
   void sig_FullDocShow();

private:
   Ui::FullDocWdg ui;
   QWidget* mWebView = nullptr;
   FullDocEscTipWdg* mFullDocEscTipWdg = nullptr;
   QTimer* mHideFullDocEscTipWdgTimer = nullptr;
};
