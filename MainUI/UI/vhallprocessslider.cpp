#include "vhallprocessslider.h"
#include <QDebug>
#include <QtMath>
#define VALUEACC 1
#define VHallProcessSliderStyleSheet \
"VHallProcessSlider::add-page:Horizontal\
{    \
   background-color: #727272;\
   height:5px;\
}\
VHallProcessSlider::sub-page:Horizontal \
{\
   background-color:#F64840;\
   height:5px;\
}\
VHallProcessSlider::groove:Horizontal \
{\
   background:transparent;\
   height:6px;\
   margin: 0px 0px; \
   left: 0px; \
   right: 0px; \
}\
VHallProcessSlider::handle:Horizontal\
{\
   background:transparent;\
   border:0px;\
}\
"
int VHallLog(const char * format, ...) ;

VHallProcessSlider::VHallProcessSlider(QWidget *parent) :
   QSlider(parent)
{
   this->setPageStep(0);
   setMaxValue(0);
   QSlider::setValue(0);
   this->setStyleSheet(VHallProcessSliderStyleSheet);
   sliderPressed=false;
   isActive=true;
}
VHallProcessSlider::VHallProcessSlider(Qt::Orientation orientation, QWidget *parent):
   QSlider(orientation,parent)
{
   this->setPageStep(0);
   setMaxValue(0);
   QSlider::setValue(0);
   this->setStyleSheet(VHallProcessSliderStyleSheet);
   sliderPressed=false;
   
   isActive=true;
}
void VHallProcessSlider::setMaxValue(int value)
{
   setMaximum(value*VALUEACC);
}
int VHallProcessSlider::getMaxValue()
{
   return QSlider::maximum();
}
void VHallProcessSlider::reActive()
{
   sliderMutex.lock();
   isActive=true;
   sliderMutex.unlock();
}
void VHallProcessSlider::setValue(int value)
{
   sliderMutex.lock();
   
   VHallLog("VHallProcessSlider setValue(%d ) sliderPressed(%d) isActive(%d)"
      ,value,sliderPressed,isActive);
   if(!sliderPressed&&isActive)
   {
      QSlider::setValue(value*VALUEACC);
   }
   sliderMutex.unlock();
}
void VHallProcessSlider::priSlotValueChanged(int value)
{
   sliderMutex.lock();
   isActive=false;
   sliderMutex.unlock();

   emit this->sigValueChange(value/VALUEACC);
}
void VHallProcessSlider::mousePressEvent(QMouseEvent *e)
{
   sliderMutex.lock();
   sliderPressed=true;
   sliderMutex.unlock();

   QSlider::mousePressEvent(e);
   this->pressPoint=e->pos();
   this->moveValue=this->value();
   
}
void VHallProcessSlider::mouseMoveEvent(QMouseEvent *e)
{
   QSlider::mouseMoveEvent(e);   
   this->moveValue=this->value();
}
void VHallProcessSlider::mouseReleaseEvent(QMouseEvent *e)
{
   QSlider::mouseReleaseEvent(e);
   qDebug()<<"VHallProcessSlider::mouseReleaseEvent"<<this->pressPoint<<e->pos()<<this->width();
   qreal x=e->pos().x()-this->pressPoint.x();
   qreal y=e->pos().y()-this->pressPoint.y();
   qreal z=x*x+y*y;
   z=qSqrt(z);
   if(z<2)
   {
      x=e->x();
      if(x<0)
      {
         x=0;
      }
      if(x>this->width())
      {
         x=this->width();
      }

      qreal value=this->maximum();
      qDebug()<<value;
      value*=x;
      value/=this->width();
      QSlider::setValue(value);
      priSlotValueChanged(value);
   }
   else
   {
      priSlotValueChanged(this->moveValue);
   }
   sliderMutex.lock();
   sliderPressed=false;
   sliderMutex.unlock();
}
