#ifndef BRIEFBARWIDGET_H
#define BRIEFBARWIDGET_H

#include <QDialog>

class QPixmap;
class ToolButton;
class BriefBarWidget : public QDialog
{
      Q_OBJECT
   public:
      explicit BriefBarWidget(QWidget *parent = 0);
      ~BriefBarWidget();

   signals:
      void StartPublish();
   public slots :
      void emitStartPublish();
      void Return();

   protected:
      void paintEvent(QPaintEvent *);
      void mousePressEvent(QMouseEvent *event);
      void mouseReleaseEvent(QMouseEvent *event);
      void mouseMoveEvent(QMouseEvent *event);

   private:
      ToolButton *mStartBtn;
      ToolButton *mMicBtn;
      ToolButton *mSettingBtn;
      ToolButton *mReturnBtn;
      QPixmap pix;
      bool mouse_press;
      QPoint move_point;
};

#endif // BRIEFBARWIDGET_H
