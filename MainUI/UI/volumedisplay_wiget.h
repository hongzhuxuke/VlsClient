#ifndef VOLUMEDISPLAY_WIGET_H
#define VOLUMEDISPLAY_WIGET_H

#include <QWidget>
#include <Qlabel>

class VolumeDisplayWiget : public QLabel
{
   Q_OBJECT

public:
   explicit VolumeDisplayWiget(QString picPath, int maxValue, int mMaxNum, QWidget *parent = 0);
    ~VolumeDisplayWiget();

    void updateDisplay(int volumeValue);
    void updateTooltip(QString tp);

protected:
   void paintEvent(QPaintEvent *);
   void enterEvent(QEvent *);
   void leaveEvent(QEvent *);

private:
    //emun  all status of volume
   //enum VolumeStatus { SILENT, VOLUME1, VOLUME2, VOLUME3, VOLUME4, VOLUME5, VOLUME6, VOLUME7, VOLUME8, VOLUME9, VOLUME10 };
   int mStatus;
   int mVolume;
   QPixmap mPixmap;
   int mBtnWidth;
   int mBtnHeight;

   int mMaxNum;
   int mMaxValue;
   QString mToolTip;
public:
   bool mbEnter = false;

private:
   void translateVolume();
};

#endif // VOLUMEDISPLAY_WIGET_H
