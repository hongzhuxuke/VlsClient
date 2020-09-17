#include "DesktopCameraCtrl.h"

#define SWITCH_OPEN  "border-image: url(:/button/images/kaiqi.png);"
#define SWITCH_CLOSE "border-image: url(:/button/images/guanbi.png);"

DesktopCameraCtrl::DesktopCameraCtrl(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint | Qt::Tool | Qt::WindowStaysOnTopHint);
    connect(ui.pushButton, SIGNAL(clicked()), this, SLOT(OnSwitchCamera()));
    setAttribute(Qt::WA_TranslucentBackground);
    setAutoFillBackground(false);
}

DesktopCameraCtrl::~DesktopCameraCtrl()
{
}


void DesktopCameraCtrl::SetCameraName(QString name) {
   if (name.length() > 23){
      name = name.mid(0, 20);
      name += "...";
   }
   ui.label->setText(name);
}

void DesktopCameraCtrl::SetDesktopCameraWndState(bool show) {
   mbIsExistFloatWnd = show;
   if (mbIsExistFloatWnd) {
      ui.pushButton->setStyleSheet(SWITCH_OPEN);
   }
   else {
      ui.pushButton->setStyleSheet(SWITCH_CLOSE);
   }
}

void DesktopCameraCtrl::OnSwitchCamera() {
   emit sig_ClickedSwitchBtn();
}

void DesktopCameraCtrl::leaveEvent(QEvent *event) {
   hide();
}