#pragma once

#include <QWidget>
#include <QMouseEvent>

class VHBaseWnd : public QWidget
{
   Q_OBJECT

public:
    VHBaseWnd(QWidget *parent);
    ~VHBaseWnd();
protected:
   void mousePressEvent(QMouseEvent *);
   void mouseMoveEvent(QMouseEvent *);
   void mouseReleaseEvent(QMouseEvent *);

private:
   QPoint mPressPoint;
   bool mIsMoved = false;
};
