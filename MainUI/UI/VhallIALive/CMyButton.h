#pragma once

#include <QObject>
#include <QPushButton>
#include <QEvent>

class CMyButton : public QPushButton
{
   Q_OBJECT

public:
    CMyButton(QWidget *parent);
    ~CMyButton();

protected:
   void enterEvent(QEvent *);
   void leaveEvent(QEvent *);

signals:
   void sig_Enter();
   void sig_Leave();
};
