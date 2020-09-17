#ifndef INVITEJOINWIDGET_H
#define INVITEJOINWIDGET_H

#include <QDialog>
#include <QPaintEvent>
#include <QPixmap>
#include <QTimer>
#include "cbasedlg.h"

#include "ui_InviteJoinWidget.h"
#include "title_button.h"

#define MAX_TIME_OUT 10000

class InviteJoinWidget : public CBaseDlg
{
   Q_OBJECT

public:
   InviteJoinWidget(QWidget *parent = 0);
   ~InviteJoinWidget();
   void CenterWindow(QWidget* parent);
	void InitTimeOut(int timeout);

   void StartTimer();

   int GetLeftTime();

private slots:
   void Slot_GuestAgreeHostInvite(); 
   void Slot_GuestRefuseHostInvite();
   void Slot_ShowTimeOut();

signals:
   void Sig_GuestAgreeHostInvite();
   void Sig_GuestRefuseHostInvite();
protected:
   void paintEvent(QPaintEvent *);
   bool eventFilter(QObject *o, QEvent *e);
   void showEvent(QShowEvent *);
private:
   Ui::InviteJoinWidget ui;
   QPixmap mpixmap;
   TitleButton *m_pCloseBtn =  NULL;
   QPoint pressPoint;
   QPoint startPoint;

   QTimer *m_pTimer = NULL;
   int nTimeCount;
	int mnMaxTimeOut = 0;
   bool mbIsStartTimer = false;
};

#endif // INVITEJOINWIDGET_H
