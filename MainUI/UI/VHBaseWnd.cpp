#include "VHBaseWnd.h"

VHBaseWnd::VHBaseWnd(QWidget *parent)
    : QWidget(parent)
{
}

VHBaseWnd::~VHBaseWnd()
{
}

void VHBaseWnd::mousePressEvent(QMouseEvent *event) {
   if (event) {
      mPressPoint = this->pos() - event->globalPos();
      mIsMoved = true;
   }
}

void VHBaseWnd::mouseMoveEvent(QMouseEvent *event) {
   if (event && (event->buttons() == Qt::LeftButton) && mIsMoved) {
      this->move(event->globalPos() + mPressPoint);
      this->raise();
   }
}

void VHBaseWnd::mouseReleaseEvent(QMouseEvent *) {
   if (mIsMoved) {
      mIsMoved = false;
   }
}
