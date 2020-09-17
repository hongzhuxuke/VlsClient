#include "suspensionDlg.h"


CSuspensionDlg::CSuspensionDlg(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	setWindowFlags(Qt::Window | Qt::FramelessWindowHint /*| Qt::WindowSystemMenuHint | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint*/);

}

CSuspensionDlg::~CSuspensionDlg()
{

}

void CSuspensionDlg::appentWdg(QWidget* pwd)
{
	if (NULL != pwd)
	{
		mpWdg = pwd;
		ui.verticalLayout->addWidget(pwd);
	}
	
}

void CSuspensionDlg::removeWdg(QWidget* pwd)
{
	if (NULL != mpWdg)
	{
		ui.verticalLayout->removeWidget(pwd);
	}
}

void CSuspensionDlg::removeWdg()
{
	if (NULL != mpWdg)
	{
		ui.verticalLayout->removeWidget(mpWdg);
	}
}

void CSuspensionDlg::mouseMoveEvent(QMouseEvent *event)
{
	if (m_bTitlePressed) {
		int dx = this->cursor().pos().x() - this->pressPoint.x();
		int dy = this->cursor().pos().y() - this->pressPoint.y();
		this->move(this->startPoint.x() + dx, this->startPoint.y() + dy);
	}
	QDialog::mouseMoveEvent(event);
}

void CSuspensionDlg::mousePressEvent(QMouseEvent *event)
{
	if (  ui.TiltleWdg	&& ui.TiltleWdg->rect().contains( event->pos() )) {
		this->pressPoint = this->cursor().pos();
		this->startPoint = this->pos();
		m_bTitlePressed = true;
	}
	QDialog::mousePressEvent(event);
}

void CSuspensionDlg::mouseReleaseEvent(QMouseEvent *event)
{
	m_bTitlePressed = false;
	QDialog::mouseReleaseEvent(event);
}

void CSuspensionDlg::showEvent(QShowEvent *event)
{
	this->setAttribute(Qt::WA_Mapped);
	QDialog::showEvent(event);
}

void CSuspensionDlg::on_btnClose_clicked(bool checked /*= false*/)
{
	removeWdg();
	close();
	emit SigClose();
}
