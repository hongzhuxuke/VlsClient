#pragma once

#include <QDialog>
#include "ui_DesktopShareSelectWdg.h"
#include "DesktopSharePreviewItem.h"
#include "VHMonitorCapture.h"
#include <vector>
#include <QTimer>
#include <QMouseEvent>
#include <QKeyEvent>
#include "NoiseTooltip.h"
#include "DesktopWarnningTips.h"

class DesktopShareSelectWdg : public QDialog
{
   Q_OBJECT

public:
   DesktopShareSelectWdg(QWidget *parent = Q_NULLPTR);
   ~DesktopShareSelectWdg();
   void InsertDesktopPreviewItem(std::vector<VHD_WindowInfo>& moniters);
   void StopCapture();
   
   VHD_WindowInfo GetCurrentWindowInfo();
   int GetCurrentIndex();
private slots:
   void slot_itemPressed(QListWidgetItem*);
   void slot_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);
   void slot_AcceptToCaptureScreen();
   void slot_StartCapture();
   void slot_SelectPrev();
   void slot_SelectNext();
   void slot_OnClose();

protected:
   void mousePressEvent(QMouseEvent *);
   void mouseMoveEvent(QMouseEvent *);
   void mouseReleaseEvent(QMouseEvent *);
   void keyPressEvent(QKeyEvent *e);
   bool eventFilter(QObject *obj, QEvent *ev);
private:
   Ui::DesktopShareSelectWdg ui;
   bool m_bIsEnableMove = true;
   QPoint mPressPoint;
   bool mIsMoved = false;
   QTimer *mCaptureTimer;
   DesktopWarnningTips* m_pNoiseTips = NULL;
   int mMonitorIndex;
   VHD_WindowInfo mMoniterInfo;
};
