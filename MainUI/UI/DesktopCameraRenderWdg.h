#pragma once

#include <QWidget>
#include "ui_DesktopCameraRenderWdg.h"
#include <QMouseEvent>

class AreaCloseWdg;

class DesktopCameraRenderWdg : public QWidget
{
   enum {
      NORMAL = 0,
      TOPLEFT = 11,
      TOP = 12,
      TOPRIGHT = 13,
      LEFT = 21,
      CENTER = 22,
      RIGHT = 23,
      BUTTOMLEFT = 31,
      BUTTOM = 32,
      BUTTOMRIGHT = 33
   };
   Q_OBJECT

public:
   DesktopCameraRenderWdg(QWidget *parent = Q_NULLPTR);
   ~DesktopCameraRenderWdg();
   void StartRender();
   void SetCloseCamera(bool close);
   void ReSize();
   void ShowCloseBtn();
   HWND LockVideo(QString deviceID, int index,bool);
   void UnlockVideo(HWND);
   void ClearInfo();
   void CheckShow();   
   QString GetDevId();
   void SetCurrentIndex(int index);
protected:
   void mousePressEvent(QMouseEvent *);
   void mouseMoveEvent(QMouseEvent *);
   void mouseReleaseEvent(QMouseEvent *);
   void leaveEvent(QEvent *event);
   bool eventFilter(QObject *obj, QEvent *event);
   void showEvent(QShowEvent *e);
   void hideEvent(QHideEvent *);
private:
   void setCursorShape(int CalPos);    //设置鼠标对应位置的形状
signals:
   void sig_closeDesktopCameraRender();

private slots:
   void slot_closeRender();
private:
   QPoint mPressPoint;
   bool mIsMoved = false;

   bool m_bIsEnableMove = true;
   bool mbIsMax = false;
   int     m_iCalCursorPos;
   bool    m_bLeftPress;
   QRect   m_rtPreGeometry;
   QPoint  m_ptViewMousePos;
   //AreaCloseWdg *mCloseBtn;
   QString mDeviceID;
   int mIndex;
private:
   Ui::DesktopCameraRenderWdg ui;
};
