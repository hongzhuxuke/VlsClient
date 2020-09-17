#include "CLabel.h"
#include <QStyleOption> 
#include <QPainter> 

CLabel::~CLabel()
{
}

CLabel::CLabel(QWidget *parent/* = Q_NULLPTR*/, Qt::WindowFlags f/* = Qt::WindowFlags()*/)
	:QLabel(parent, f )
{

}

CLabel::CLabel(const QString &text, QWidget *parent /*= Q_NULLPTR*/, Qt::WindowFlags f /*= Qt::WindowFlags()*/)
	: QLabel(text, parent, f)
{

}

void CLabel::paintEvent(QPaintEvent *event)
{
	QStyleOption opt; // 需要头文件#include <QStyleOption>  
	opt.init(this);
	QPainter p(this); // 需要头文件#include <QPainter>  
	style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
	QLabel::paintEvent(event);
}
