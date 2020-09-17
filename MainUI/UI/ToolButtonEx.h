#ifndef ToolButtonExEX_H
#define ToolButtonExEX_H
#include <QToolButton>
class QPainter;
class QMouseEvent;

class ToolButtonEx : public QToolButton
{
   Q_OBJECT
public:

   ToolButtonEx(QString pic_name, QWidget *parent = 0);
   ~ToolButtonEx();
   void setShowNormal(bool);

   //???????任??
   void changeImage(QString pic_name);

   //禁用
   void SetDisabled(bool bDisabled);
signals:
   void showVolumeSlider();
   void hideVolumeSlider();
   void sigClicked();
   void sigPressed();
   void sigEnter();
   void sigLeave();
protected:
   void enterEvent(QEvent *);
   void leaveEvent(QEvent *);
   void mousePressEvent(QMouseEvent *event);
   void mouseReleaseEvent(QMouseEvent *event);
   void paintEvent(QPaintEvent *);

public:
   //enum: status of title button
   enum ButtonStatus{ NORMAL, ENTER, PRESS, DISABLED };
   ButtonStatus mStatus;
   QPixmap mPixmap;

   int mBtnWidth;
   int mBtnHeight;
   bool mMousePressed = false;
};

#endif //ToolButtonExEX_H
