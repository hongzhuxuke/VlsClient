#include "FullDocWdg.h"
#include <QDesktopWidget>
#include <windows.h>


FullDocWdg::FullDocWdg(QWidget *parent)
   : QWidget(parent){
   ui.setupUi(this);

   setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
   setFocusPolicy(Qt::FocusPolicy::StrongFocus);
   mHideFullDocEscTipWdgTimer = new QTimer(this);
   if (mHideFullDocEscTipWdgTimer) {
      connect(mHideFullDocEscTipWdgTimer,SIGNAL(timeout()),this,SLOT(slot_HideEscTip()));
   }
   mFullDocEscTipWdg = new FullDocEscTipWdg(this);
}

FullDocWdg::~FullDocWdg(){
   if (mHideFullDocEscTipWdgTimer) {
      mHideFullDocEscTipWdgTimer->stop();
   }
}

void FullDocWdg::InsertDoc(QWidget* wdg) {
   if (wdg) {
      ui.horizontalLayout->addWidget(wdg);
      this->setMouseTracking(true);
      mWebView = wdg;

      mWebView->activateWindow();
      mWebView->setFocus();
      //this->grabKeyboard();
   }
   if (mFullDocEscTipWdg) {
      mFullDocEscTipWdg->show();
      QRect rect = QApplication::desktop()->screenGeometry(this);
      mFullDocEscTipWdg->move(rect.x() + (this->width() - mFullDocEscTipWdg->width()) / 2,100);
   }
   if (mHideFullDocEscTipWdgTimer) {
      mHideFullDocEscTipWdgTimer->setSingleShot(true);
      mHideFullDocEscTipWdgTimer->start(3000);
   }
}

void FullDocWdg::RemoveDoc(QWidget* wdg) {
   if (wdg) {
      ui.horizontalLayout->removeWidget(wdg);   
      this->setMouseTracking(false);
      mWebView = nullptr;
      //this->releaseKeyboard();
   }
   slot_HideEscTip();
}

void FullDocWdg::slot_HideEscTip() {
   if (mFullDocEscTipWdg) {
      mFullDocEscTipWdg->hide();
   }
   if (mWebView) {
      mWebView->show();
      mWebView->activateWindow();
      mWebView->setFocus();
   }
   mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
}

void FullDocWdg::showEvent(QShowEvent *event) {
   this->setAttribute(Qt::WA_Mapped);
   QWidget::showEvent(event);    
   raise();

   activateWindow();
   setFocus();

   if (mWebView) {
      mWebView->raise();
      mWebView->activateWindow();
      mWebView->setFocus();
   }
   mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
}

void FullDocWdg::keyPressEvent(QKeyEvent *event) {
   switch (event->key())
   {
   case Qt::Key_Escape:
      emit sig_PressKeyEsc();
      break;
   default:
      if (mWebView) {
         mWebView->setFocus();
         mWebView->raise();
         mWebView->activateWindow();
      }
      QWidget::keyPressEvent(event);
   }
}

void FullDocWdg::keyReleaseEvent(QKeyEvent *event) {
   switch (event->key())
   {
   case Qt::Key_Escape:
      emit sig_PressKeyEsc();
      break;
   default:
      if (mWebView) {
         mWebView->raise();
         mWebView->activateWindow();
         mWebView->repaint();
      }
      QWidget::keyPressEvent(event);
   }
}
