#ifndef VHALLPROCESSSLIDER_H
#define VHALLPROCESSSLIDER_H

#include <QSlider>
#include <QMouseEvent>
#include <QPoint>
#include <QMutex>
class VHallProcessSlider : public QSlider {
   Q_OBJECT
public:
   explicit VHallProcessSlider(QWidget *parent = 0);
   explicit VHallProcessSlider(Qt::Orientation orientation, QWidget *parent = 0);
   void setMaxValue(int);
   int getMaxValue();
   void setValue(int);
   void reActive();
signals:
   void sigValueChange(int);
   public slots:
   void priSlotValueChanged(int);
protected:
   void mousePressEvent(QMouseEvent *);
   void mouseReleaseEvent(QMouseEvent *);
   void mouseMoveEvent(QMouseEvent *);
private:
   QPoint pressPoint;
   QMutex sliderMutex;
   bool sliderPressed = false;
   bool isActive = true;
   int moveValue;
};

#endif // VHALLPROCESSSLIDER_H
