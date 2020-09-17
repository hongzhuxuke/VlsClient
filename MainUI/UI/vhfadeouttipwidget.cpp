#include "vhfadeouttipwidget.h"
#include <QGridLayout>
#include <QPainter>
#include <QDebug>
#define TIMETICK 40
VHFadeOutTipWidget::VHFadeOutTipWidget(QWidget *parent) :
   QWidget(NULL) {
	setWindowFlags(Qt::FramelessWindowHint | Qt::Tool | Qt::WindowStaysOnTopHint);
   setAttribute(Qt::WA_TranslucentBackground);
   setAutoFillBackground(false);
   
   QGridLayout *layout = new QGridLayout(this);
   m_tipLabel = new QLabel (this);
   m_tipLabel->setStyleSheet("color:rgb(150,150,150);");
   m_tipLabel->setAlignment(Qt::AlignCenter);
   layout->addWidget(m_tipLabel);
   setLayout(layout);
   mPoint = QPoint(0,0);
   m_timer = new QTimer(this);
   connect(m_timer,SIGNAL(timeout()),this,SLOT(timeout()));
   m_globalSize = QSize(898,34);
}

VHFadeOutTipWidget::~VHFadeOutTipWidget() {
   if (m_timer) {
      m_timer->stop();
   }

}

void VHFadeOutTipWidget::SetSize(int w,int h) {
   m_globalSize = QSize(w,h);
}

void VHFadeOutTipWidget::rePos(/*QWidget* mainWdg*/) {
   //int xPoint = 0;
   //int yPoint = 0;
   //if (mainWdg != NULL) {
   //   xPoint = mainWdg->x();
   //   yPoint = mainWdg->y();
   //}
   raise();
   this->move(/*xPoint + */mPoint.x(), /*yPoint + */mPoint.y());
   //int iWidth = 0;
   //QPoint pos = mainWdg->GetTipPoint(iWidth);
   //move(mPoint.x(), mPoint.y() );
   resize(m_globalSize);
}

void VHFadeOutTipWidget::Tip(/*QWidget* mainWdg,*/QPoint p, QString str, const int& iW) {
   if (str.isEmpty()&& m_tipLabel->text().trimmed().isEmpty()) {
      return;
   }else if (m_tipLabel && !str.isEmpty()) {
      m_tipLabel->setText(str);
   }
   SetSize(iW, m_globalSize.height());
   mPoint = p;
   rePos(/*mainWdg*/);
   show();
   if(m_timer) {
      m_timer->stop();
      m_closeCount = 0;
      setWindowOpacity(1.0f);
      m_timer->start(TIMETICK);
   }
}

void VHFadeOutTipWidget::Stop(){
   if(m_timer) {
      m_timer->stop();
   }
   hide();
}

void VHFadeOutTipWidget::paintEvent(QPaintEvent *e) {
   QPainter painter(this);
   painter.setRenderHint(QPainter::Antialiasing, true);
   painter.fillRect(this->rect(), QColor(67, 71, 74, 230));
}

void VHFadeOutTipWidget::timeout() {
   m_closeCount+=TIMETICK;
   if(m_closeCount<3000) {
      return ;
   }
   else if(m_closeCount<4000) {
      float opacity=1.0f;
      opacity=4000;
      opacity-=m_closeCount;
      opacity/=1000;
      setWindowOpacity(opacity);
   }
   else {
      m_timer->stop();
      m_closeCount=0;
      close();
   }
} 

void VHFadeOutTipWidget::mousePressEvent(QMouseEvent *event) {
   if (m_pParentWdg) {
      m_pParentWdg->raise();
      this->show();
      this->raise();
   }
}

