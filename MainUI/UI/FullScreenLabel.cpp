#include "FullScreenLabel.h"
#include <QPainter>
#include <QImage>

#include "MainUIIns.h"
#include "Msg_OBSControl.h"
FullScreenLabel::FullScreenLabel(QWidget *parent) :
QLabel(parent) {
   setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint | Qt::WindowStaysOnTopHint);
}

FullScreenLabel::~FullScreenLabel() {

}

void FullScreenLabel::paintEvent(QPaintEvent *e) {
   
   if (this->m_qPixmap.isNull()) {
      return;
   }

   QPainter p(this);
   QPixmap showPixmap = this->m_qPixmap;
   QImage maskImage = QImage(showPixmap.size(), QImage::Format_ARGB32);
   maskImage.fill(QColor(0, 0, 0, 150));
   int x1, x2, y1, y2;
   x1 = m_qStartPos.x();
   x2 = m_qEndPos.x();
   y1 = m_qStartPos.y();
   y2 = m_qEndPos.y();

   if (x1 > x2) {
      int tmp = x1;
      x1 = x2;
      x2 = tmp;
   }

   if (y1 > y2) {
      int tmp = y1;
      y1 = y2;
      y2 = tmp;
   }

   for (int x = x1; x <= x2; x++) {
      for (int y = y1; y <= y2; y++) {
         if (x >= 0 && x <= maskImage.width() && y >= 0 && y <= maskImage.height()) {
            maskImage.setPixel(x, y, 0x0);
         }
      }
   }

   QPainter p2(&showPixmap);
   p2.drawImage(showPixmap.rect(), maskImage);
   QPen pen;
   pen.setWidth(2);
   pen.setColor(QColor(Qt::green));
   p2.setPen(pen);
   p2.drawRect(x1, y1, x2 - x1, y2 - y1);
   p.drawPixmap(this->rect(), showPixmap);
	QWidget::paintEvent(e);
}
void FullScreenLabel::mouseMoveEvent(QMouseEvent *ev) {
   QWidget::mouseMoveEvent(ev);
   this->m_qEndPos = ev->pos();
   moved = true;
   this->repaint();
}
void FullScreenLabel::mousePressEvent(QMouseEvent *ev) {
   QWidget::mousePressEvent(ev);
   this->m_qStartPos = ev->pos();
}
void FullScreenLabel::mouseReleaseEvent(QMouseEvent *ev) {
   QWidget::mouseReleaseEvent(ev);

   int x1, x2, y1, y2;
   x1 = m_qStartPos.x();
   x2 = m_qEndPos.x();
   y1 = m_qStartPos.y();
   y2 = m_qEndPos.y();

   if (x1 > x2) {
      int tmp = x1;
      x1 = x2;
      x2 = tmp;
   }

   if (y1 > y2) {
      int tmp = y1;
      y1 = y2;
      y2 = tmp;
   }

   STRU_OBSCONTROL_SHAREREGION ShareRegion;
   if (x2 - x1 <= 5 || y2 - y1 <= 5 || !moved) {    
      ShareRegion.m_rRegionRect.left = 0;
      ShareRegion.m_rRegionRect.top = 0;
      ShareRegion.m_rRegionRect.right = 0;
      ShareRegion.m_rRegionRect.bottom = 0;

   }
   else {
      ShareRegion.m_rRegionRect.left = x1;
      ShareRegion.m_rRegionRect.top = y1;
      ShareRegion.m_rRegionRect.right = x2;
      ShareRegion.m_rRegionRect.bottom = y2;
      SingletonMainUIIns::Instance().PostCRMessage(MSG_OBSCONTROL_SHAREREGION, 
         &ShareRegion, sizeof(STRU_OBSCONTROL_SHAREREGION));
   }

   this->close();
}
void FullScreenLabel::SetFullScreenPixmap(QPixmap pixmap) {
   moved = false;
   m_qPixmap = pixmap;
   this->m_qStartPos = QPoint(0, 0);
   this->m_qEndPos = QPoint(0, 0);
   this->repaint();
}

bool FullScreenLabel::Create() {
   return true;
}

void FullScreenLabel::Destroy() {

}

void FullScreenLabel::leaveEvent(QEvent *e) {
   QWidget::leaveEvent(e);
   emit this->sigAreaSelect(0, 0, 0, 0);
   this->close();
}
void FullScreenLabel::keyPressEvent(QKeyEvent *ev) {
   QWidget::keyPressEvent(ev);
   if (ev->key() == Qt::Key_Escape) {
      emit this->sigAreaSelect(0, 0, 0, 0);
      this->close();
   }
}

