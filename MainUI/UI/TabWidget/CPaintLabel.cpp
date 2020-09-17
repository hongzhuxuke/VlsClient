#include "CPaintLabel.h"
#include <QPainter>


CPaintLabel::CPaintLabel(QWidget *parent/* = Q_NULLPTR*/, Qt::WindowFlags f /*= Qt::WindowFlags()*/)
	:QLabel(parent, f)
{

}

CPaintLabel::CPaintLabel(const QString &text, QWidget *parent /*= Q_NULLPTR*/, Qt::WindowFlags f /*= Qt::WindowFlags()*/)
	: QLabel(text, parent,f)
{

}

CPaintLabel::~CPaintLabel()
{
	//int a = 0;
}

void CPaintLabel::SetHandsUpState(eHandsUpState  eState)
{
	mState = eState;
	update();
}

void CPaintLabel::paintEvent(QPaintEvent *e)
{
	QLabel::paintEvent(e);
	if (eHandsUpState_existence == mState)
	{

		QPainter painter(this);
		painter.setPen(QPen(QColor(252, 86, 89), 4, Qt::DashLine));//
		painter.setBrush(QBrush(Qt::red, Qt::SolidPattern));//
		//painter.setBrush(Qt::SolidPattern);
		int iW = this->width();
		int iH = this->height();
		painter.drawEllipse(2, 0.2*iH, 2, 2);//»­Ô² 
										 //painter.drawPoint(QPointF(58, 8));
	}
}

void CPaintLabel::mouseDoubleClickEvent(QMouseEvent *event)
{
	emit sig_DoubleClicked();
}
