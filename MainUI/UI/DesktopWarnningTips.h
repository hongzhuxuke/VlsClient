#pragma once

#include <QWidget>
#include "ui_DesktopWarnningTips.h"

class DesktopWarnningTips : public QWidget
{
    Q_OBJECT

public:
    DesktopWarnningTips(QWidget *parent = Q_NULLPTR);
    ~DesktopWarnningTips();

private:
    Ui::DesktopWarnningTips ui;
};
