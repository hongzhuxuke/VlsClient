#include <QObject>
#include <QPainter>
#include <QMouseEvent>
#include "ToolButtonEx.h"
#include <QFile>
ToolButtonEx::ToolButtonEx(QString pic_name, QWidget *parent)
:QToolButton(parent) {
   //�����ı���ɫ
   QPalette text_palette = palette();
   text_palette.setColor(QPalette::ButtonText, QColor(230, 230, 230));
   setPalette(text_palette);

   //�����ı�����
   QFont &text_font = const_cast<QFont &>(font());
   text_font.setWeight(QFont::Bold);
   setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

   //����ͼ��
   mPixmap.load(pic_name);
   mBtnWidth = mPixmap.width() / 4;
   mBtnHeight = mPixmap.height();
   setIcon(mPixmap);
   setIconSize(mPixmap.size());
   setFixedSize(mBtnWidth, mBtnHeight);
   setAutoRaise(true);
   setStyleSheet("background:transparent;");

   mStatus = NORMAL;
   mMousePressed = false;
}

ToolButtonEx::~ToolButtonEx() {

}

void ToolButtonEx::enterEvent(QEvent *) {
   if (mStatus == DISABLED) {
      return;
   }
   mStatus = ENTER;
   emit sigEnter();
   update();
}

void ToolButtonEx::leaveEvent(QEvent *) {
   if (mStatus == DISABLED) {
      return;
   }
   mStatus = NORMAL;
   update();
}
void ToolButtonEx::setShowNormal(bool ok) {
   if (ok) {
      mStatus = NORMAL;
   } else {
      mStatus = ENTER;
   }
   emit sigLeave();
   update();
   this->repaint();

}
void ToolButtonEx::mousePressEvent(QMouseEvent *event) {
   if (mStatus == DISABLED) {
      return;
   }
   if (event->button() == Qt::LeftButton) {
      emit sigPressed();
      mMousePressed = true;
      mStatus = PRESS;
      update();
   } else {
   }
}

void ToolButtonEx::mouseReleaseEvent(QMouseEvent *) {
   if (mStatus == DISABLED) {
      return;
   }
   if (mMousePressed) {
      mMousePressed = false;
      mStatus = ENTER;
      update();
      this->repaint();
      emit sigClicked();
   }
}

void ToolButtonEx::paintEvent(QPaintEvent *) {
   QPainter painter(this);
   painter.drawPixmap(rect(), mPixmap.copy(mBtnWidth * mStatus, 0, mBtnWidth, mBtnHeight));
}

void ToolButtonEx::changeImage(QString pic_name) {
   mPixmap.load(pic_name);
   mBtnWidth = mPixmap.width() / 3;
   mBtnHeight = mPixmap.height();
   setFixedSize(mBtnWidth, mBtnHeight);
}

void ToolButtonEx::SetDisabled(bool bDisabled) {
   if (bDisabled) {
      mStatus = DISABLED;
   } else {
      mStatus = NORMAL;
   }
   setDisabled(bDisabled);
}
