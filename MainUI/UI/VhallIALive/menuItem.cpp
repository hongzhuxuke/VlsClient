#include "MenuItem.h"

MenuItem::MenuItem(QWidget *parent /*= Q_NULLPTR*/)
	:QWidget(parent)
{
	ui.setupUi(this);
	this->setFocusPolicy(Qt::StrongFocus);
	/*setAttribute(Qt::WA_TranslucentBackground);
	setAutoFillBackground(false);
	this->setWindowFlags(Qt::FramelessWindowHint | Qt::SplashScreen);*/

	ui.tbtnIcon->setMapButton(ui.tBtnText);
}

MenuItem::~MenuItem()
{

}


void MenuItem::enterEvent(QEvent *event)
{
   ui.tBtnText->EnterButton();
}

void MenuItem::leaveEvent(QEvent *event)
{
   ui.tBtnText->LeaveButton();
}

void MenuItem::setHoverStyle(const QString & str, eBtnId id)
{
	if (eBtnId_icon == id)
	{
		ui.tbtnIcon->setHoverStyle(str);
	}
	else
	{
		ui.tBtnText->setHoverStyle(str);
	}
}

void MenuItem::SetText(const QString & str)
{
	ui.tBtnText->setText(str);
}

void MenuItem::setLeaveStyle(const QString & str, eBtnId id)
{
	if (eBtnId_icon == id)
	{
		ui.tbtnIcon->setLeaveStyle(str);
	}
	else
	{
		ui.tBtnText->setLeaveStyle(str);
	}
}

void MenuItem::SetOpType(const int& iOpType) {
	miOpType = iOpType;
}

void MenuItem::on_tbtnIcon_clicked(bool checked /*= false*/)
{
	emit sigClicked(miOpType);
}

void MenuItem::on_tBtnText_clicked(bool checked /*= false*/)
{
	emit sigClicked(miOpType);
}