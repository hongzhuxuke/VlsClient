#include "HoverLabel.h"

HoverLabel::HoverLabel(QWidget *parent) : QLabel(parent)
{

}

HoverLabel::~HoverLabel()
{

}

void HoverLabel::enterEvent(QEvent *)
{
   emit showDetail();
}

void HoverLabel::leaveEvent(QEvent *)
{
   emit hideDetail();
}
