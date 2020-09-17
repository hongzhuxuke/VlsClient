#include "VhallRenderMaskWdg.h"
#include <QPainter>
#include "pathmanager.h"
#include "vhalluserinfomanager.h"
#include "pub.Const.h"
VhallRenderMaskWdg::VhallRenderMaskWdg(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	setAttribute(Qt::WA_TranslucentBackground, true);
	setWindowFlags(Qt::FramelessWindowHint| Qt::WindowStaysOnTopHint);
	ui.label_roleLogo->hide();
}

VhallRenderMaskWdg::~VhallRenderMaskWdg()
{

}

void VhallRenderMaskWdg::SetRoleName(const QString &role) {
   //if (role.compare(USER_HOST) == 0) {
   //   ui.label_roleLogo->setStyleSheet("border-image:url(:/vhallActive/img/vhallactive/host_2.png);");
   //}
   //else if (role.compare(USER_GUEST) == 0) {
   //   ui.label_roleLogo->setStyleSheet("border-image:url(:/vhallActive/img/vhallactive/guest_2.png);");
   //}
   //else if (role.compare(USER_USER) == 0) {
   //   ui.label_roleLogo->setStyleSheet("border-image:url(:/vhallActive/img/vhallactive/member_2.png);");
   //}
}

void VhallRenderMaskWdg::SetUserName(const QString &userName) {
   ui.label_userName->setText(CPathManager::GetString(userName, 14));
   ui.label_userName->setToolTip(userName);
   ui.label_userName->setStyleSheet(TOOLTIPQSS);
}

void VhallRenderMaskWdg::SetResetSize(bool mainView) {
   if (mainView) {
      ui.label_roleLogo->setFixedSize(QSize(20, 22));
      this->setFixedWidth(320);
   }
   else {
      ui.label_roleLogo->setFixedSize(QSize(10, 11));
      this->setFixedWidth(175);
   }
}

