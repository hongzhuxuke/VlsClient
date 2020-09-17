#ifndef VHALLJOINACTIVITY_H
#define VHALLJOINACTIVITY_H

#include <QDialog>
#include <QPixmap>
#include <QPainter>
#include "cbasedlg.h"
#include "vhallwaiting.h"
namespace Ui {
class VhallJoinActivity;
}
class TitleButton;

#define  CHECK_PWD_LEN 6

class VhallJoinActivity : public CBaseDlg
{
    Q_OBJECT

public:
   explicit VhallJoinActivity(QWidget *parent = 0);
   ~VhallJoinActivity();
   QString GetActivityId();
   void CenterWindow(QWidget * parent);
	void setDefultNick(QString& strNickName);
   void CloseWaiting();
   void SetPwdLogin(bool pwdLogin);
signals:
   void SigJoinActivity(QString, QString, QString,bool);
   void SigClose();
public slots:
   void SlotJoinActivityEnd(bool,QString);
   void Slot_Close();
protected:   
   virtual bool eventFilter(QObject *o, QEvent *e);
   virtual void paintEvent(QPaintEvent *); 
   virtual void closeEvent(QCloseEvent *);
   virtual void showEvent(QShowEvent *);
private slots:
   void on_btn_enter_clicked();
   void on_activityId_returnPressed();
	void on_ledtNick_textEdited(const QString & text);
private:
   void ToJoin();
private:
   Ui::VhallJoinActivity *ui;
   TitleButton *m_pBtnClose = NULL;
   QPoint pressPoint;
   QPoint startPoint;   
   QPixmap pixmap; 
   VhallWaiting *m_pWaiting = NULL;
   bool mbPwdLogin = false;
};

#endif // VHALLJOINACTIVITY_H
