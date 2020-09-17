#include <QPainter>
#include <QToolTip>
#include "volumedisplay_wiget.h"

VolumeDisplayWiget::VolumeDisplayWiget(QString picPath, int maxValue, int maxNum, QWidget *parent)
   : QLabel(parent)
{
   mPixmap.load(picPath);
   mBtnWidth = mPixmap.width() / (maxNum + 1);
   mBtnHeight = mPixmap.height();
   setFixedSize(mBtnWidth, mBtnHeight);

   mMaxNum = maxNum;
   mMaxValue = maxValue;
   mVolume = 0;
   mStatus = 0;
   mToolTip = "";
}

VolumeDisplayWiget::~VolumeDisplayWiget()
{

}

void VolumeDisplayWiget::updateDisplay(int volumeValue)
{
   mVolume = volumeValue;
   translateVolume();
   update();
}

void VolumeDisplayWiget::updateTooltip(QString tp){
   mToolTip = tp;
}

void VolumeDisplayWiget::translateVolume()
{
   mStatus = mMaxNum;
   if (mVolume <= 0){
      mStatus = 0;
      return;
   }
   if (mVolume >= mMaxValue){
      mStatus = mMaxNum;
      return;
   }

   int stepValue = mMaxValue / mMaxNum;
   int step = mVolume / stepValue;
   mStatus = step + 1;
}

void VolumeDisplayWiget::paintEvent(QPaintEvent *)
{
   QPainter painter(this);
   painter.drawPixmap(rect(), mPixmap.copy(mBtnWidth * mStatus, 0, mBtnWidth, mBtnHeight));
}

void VolumeDisplayWiget::enterEvent(QEvent *){
   mbEnter = true;
   //QToolTip::showText(QCursor::pos(), mToolTip);
}

void VolumeDisplayWiget::leaveEvent(QEvent *){
   mbEnter = false;

   //QToolTip::hideText();
}