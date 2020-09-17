#pragma once

#include <QWidget>
#include "ui_DesktopSharePreviewItem.h"
#include "VHMonitorCapture.h"
#include <QPixmap>
#include <QPainter>
class DesktopSharePreviewItem : public QWidget
{
    Q_OBJECT

public:
    DesktopSharePreviewItem(QWidget *parent = Q_NULLPTR);
    ~DesktopSharePreviewItem();

    void SetDesktopInfo(int monitorIndex, VHD_WindowInfo& moniters);
    void SetSelectItemSheet(bool select);
    bool GetSelectState();
    void StartCaptrue();
    VHD_WindowInfo GetCurrentWindowInfo();
    int GetCurrentIndex();
protected:
   void paintEvent(QPaintEvent *);
private:
    Ui::DesktopSharePreviewItem ui;
    int mMonitorIndex;
    VHD_WindowInfo mMoniterInfo;
    QPixmap mCaptrueImage;
    bool mbIsSelectd;
    BYTE* bitData = NULL;
    HDC mCaptureDC = nullptr;
    HDC mMemDC = nullptr;
};
