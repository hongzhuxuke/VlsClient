#include "cdesktopmodechatparentwdg.h"

CDesktopModeChatParentWdg::CDesktopModeChatParentWdg(QWidget *parent)
    : QWidget(parent)
{
   ui.setupUi(this);
   this->setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowSystemMenuHint | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint);
   this->setFixedSize(QSize(300, 600));
}

CDesktopModeChatParentWdg::~CDesktopModeChatParentWdg()
{

}

void CDesktopModeChatParentWdg::showEvent(QShowEvent *event) {
   this->setAttribute(Qt::WA_Mapped);
   QWidget::showEvent(event);
}
