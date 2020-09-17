#pragma once

#include <QWidget>
#include "ui_CameraTeachTips.h"

class CameraTeachTips : public QWidget
{
    Q_OBJECT

public:
    CameraTeachTips(QWidget *parent = Q_NULLPTR);
    ~CameraTeachTips();

private slots:
    void slot_OnClose();

private:
    Ui::CameraTeachTips ui;
};
