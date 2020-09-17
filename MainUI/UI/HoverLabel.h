#ifndef HOVERLABEL_H
#define HOVERLABEL_H

#include <QWidget>
#include <QLabel>
class HoverLabel : public QLabel
{
      Q_OBJECT
   public:
      explicit HoverLabel(QWidget *parent = 0);
      ~HoverLabel();

   protected:
      void enterEvent(QEvent *);
      void leaveEvent(QEvent *);

   signals:
      void showDetail();
      void hideDetail();

   public slots:
};

#endif // HOVERLABEL_H
