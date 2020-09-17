#include <QBoxLayout>
#include <QPainter>
#include <QPixmap>
#include <QBitmap>
#include <QMouseEvent>
#include "BriefBar_Widget.h"
#include "src/tool_button/tool_button.h"
BriefBarWidget::BriefBarWidget(QWidget *parent) 
   : QDialog(parent)
{
   setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
   setAttribute(Qt::WA_TranslucentBackground);
   setAutoFillBackground(true);

   mStartBtn = new ToolButton(":/toolWidget/briefStart", this);
   mMicBtn = new ToolButton(":/toolWidget/mic", this);
   mSettingBtn = new ToolButton(":/toolWidget/setting", this);
   mReturnBtn = new ToolButton(":/toolWidget/text", this);

   QHBoxLayout *BriefBarLayout = new QHBoxLayout(this);
   BriefBarLayout->addWidget(mStartBtn, 0, Qt::AlignCenter);
   BriefBarLayout->addWidget(mMicBtn, 0, Qt::AlignCenter);
   BriefBarLayout->addWidget(mSettingBtn, 0, Qt::AlignCenter);
   BriefBarLayout->addWidget(mReturnBtn, 0, Qt::AlignCenter);
   setLayout(BriefBarLayout);

   connect(mStartBtn, SIGNAL(clicked()), this, SLOT(emitStartPublish()));
   connect(mReturnBtn, SIGNAL(clicked()), this, SLOT(Return()));

   setWindowFlags(Qt::FramelessWindowHint);
   pix.load(":/toolWidget/briefBarBack", 0, Qt::AvoidDither | Qt::ThresholdDither | Qt::ThresholdAlphaDither);
   resize(pix.size());
   setMask(QBitmap(pix.mask()));

   mouse_press = false;
}

BriefBarWidget::~BriefBarWidget()
{

}

void BriefBarWidget::paintEvent(QPaintEvent *){
   //QPainter painter(this);
   //painter.setRenderHint(QPainter::Antialiasing, true);
   //painter.setPen(QPen(QColor(67, 67, 67), 1));
   //painter.setBrush(QColor(38, 38, 38));
   //painter.drawRoundedRect(rect(), 20.0, 20.0);

   QPainter painter(this);
   painter.drawPixmap(0, 0, QPixmap(":/toolWidget/briefBarBack"));
}

void BriefBarWidget::mousePressEvent(QMouseEvent *event)
{
   if (event->button() == Qt::LeftButton){
      mouse_press = true;
   }
   move_point = event->globalPos() - pos();
}

void BriefBarWidget::mouseReleaseEvent(QMouseEvent *)
{
   mouse_press = false;
}

void BriefBarWidget::mouseMoveEvent(QMouseEvent *event)
{
   if (mouse_press){
      QPoint move_pos = event->globalPos();
      move(move_pos - move_point);
   }
}

void BriefBarWidget::emitStartPublish()
{
   emit StartPublish();
}

void BriefBarWidget::Return(){
   accept();
}