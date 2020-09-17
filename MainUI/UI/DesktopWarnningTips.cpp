#include "DesktopWarnningTips.h"

DesktopWarnningTips::DesktopWarnningTips(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
    this->setWindowFlags(
       Qt::FramelessWindowHint |
       Qt::WindowMinimizeButtonHint |
       Qt::Tool);
    this->setAttribute(Qt::WA_TranslucentBackground);
    this->setAutoFillBackground(true);
   
}

DesktopWarnningTips::~DesktopWarnningTips()
{
}
