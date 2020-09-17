#ifndef FullScreenLabel_H
#define FullScreenLabel_H

#include <QLabel>
#include <QPixmap>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QKeyEvent>

class FullScreenLabel : public QLabel {
   Q_OBJECT
public:
   explicit FullScreenLabel(QWidget *parent = 0);
   ~FullScreenLabel();
   void SetFullScreenPixmap(QPixmap);

   bool Create();
   void Destroy();
signals:
   void sigAreaSelect(int, int, int, int);
public slots:
protected:
   void paintEvent(QPaintEvent *);
   void mouseMoveEvent(QMouseEvent *ev);
   void mousePressEvent(QMouseEvent *ev);
   void mouseReleaseEvent(QMouseEvent *ev);
   void leaveEvent(QEvent *);
   void keyPressEvent(QKeyEvent *ev);
private:
   QPixmap m_qPixmap;
   QPoint m_qStartPos;
   QPoint m_qEndPos;
   bool moved;
};

#endif // FullScreenLabel_H
