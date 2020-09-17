#pragma once

#include <QWidget>
#include "ui_DesktopCameraCtrl.h"

class DesktopCameraCtrl : public QWidget
{
    Q_OBJECT

public:
    DesktopCameraCtrl(QWidget *parent = Q_NULLPTR);
    ~DesktopCameraCtrl();

    void SetCameraName(QString name);
    void SetDesktopCameraWndState(bool show);

private slots:
   void OnSwitchCamera();

protected:
   void leaveEvent(QEvent *event);

signals:
   void sig_ClickedSwitchBtn();
   
private:
    Ui::DesktopCameraCtrl ui;
    bool mbIsExistFloatWnd = false;
};
