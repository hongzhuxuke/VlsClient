#include <QPainter>
#include <QDesktopWidget>
#include "vhdesktoptip.h"
#include "ui_vhdesktoptip.h"
#include <QDebug>
VHDesktopTip::VHDesktopTip(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::VHDesktopTip)
{
    ui->setupUi(this);
    this->setWindowFlags(
        Qt::FramelessWindowHint |
        Qt::WindowMinimizeButtonHint |
        Qt::Tool|
        Qt::WindowStaysOnTopHint);
    this->setAttribute(Qt::WA_TranslucentBackground);
    this->setAutoFillBackground(true);
    connect(&this->timer,SIGNAL(timeout()),this,SLOT(timeout()));
    connect(this,SIGNAL(SigStop()),this,SLOT(SlotStop()));
}

VHDesktopTip::~VHDesktopTip()
{
    delete ui;
}
void VHDesktopTip::CenterWindow(QWidget * parent){
   int x = 0;
   int y = 0;
   if (NULL == parent) {
      const QRect rect = QApplication::desktop()->availableGeometry();
      x = rect.left() + (rect.width() - width()) / 2;
      y = rect.top() + 100;
   } else {
      QPoint point(0, 0);
      point = parent->mapToGlobal(point);
      x = point.x() + (parent->width() - width()) / 2;
      y = point.y() + (parent->height() - height()) / 2;
   }
   move(x, y);
}
void VHDesktopTip::Tip(QString str){
    ui->label->setText(str);
    CenterWindow(NULL);
    this->show();
    if(this->timer.isActive()) {
        this->timer.stop();
    }

    this->timerCount=0;
    this->setWindowOpacity(1.0f);
    this->timer.start(100);
}
void VHDesktopTip::SlotStop(){
   this->timer.stop();
   this->hide();
}

void VHDesktopTip::Stop(){
   emit this->SigStop();
}
void VHDesktopTip::SetPixmap(QString file){
    this->pixmap=QPixmap(file);
    this->resize(pixmap.size());
}

void VHDesktopTip::paintEvent(QPaintEvent *){
    QPainter p(this);
    p.drawPixmap(this->rect(),this->pixmap);
}
void VHDesktopTip::timeout(){
    this->timerCount++;
    if(this->timerCount>=30) {
        this->timer.stop();
        this->hide();
    }
    else if(this->timerCount>=20) {
        qreal opacity=1.0f;
        opacity-=0.1f*(this->timerCount-20);
        this->setWindowOpacity(opacity);
    }
}

