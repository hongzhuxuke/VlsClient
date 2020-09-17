#include "CMyButton.h"

CMyButton::CMyButton(QWidget *parent)
    : QPushButton(parent){
}

CMyButton::~CMyButton()
{
}

void CMyButton::enterEvent(QEvent *) {
   emit sig_Enter();
}

void CMyButton::leaveEvent(QEvent *) {
   emit sig_Leave();
}
