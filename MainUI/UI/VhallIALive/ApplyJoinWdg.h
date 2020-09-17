#ifndef APPLYJOINWIDGET_H
#define APPLYJOINWIDGET_H

#include <QDialog>
#include <QPaintEvent>
#include <QPixmap>
#include <QTimer>

#include "ui_ApplyJoinWdg.h"
#include "title_button.h"
#include "cbasedlg.h"


class ApplyJoinWdg : public CBaseDlg
{
   Q_OBJECT

public:
   ApplyJoinWdg(QWidget *parent = 0);
   ~ApplyJoinWdg();


   //virtual void Notice(const CSTDMapParam &);
   //virtual void Notice(const CSTDMapParamPtr &);


   void CenterWindow(QWidget* parent);
   void InitTimeOut(int timeout);

   QString GetUid();
   void SetUid(const QString& strUid,const QString nickName);

public slots:
   void hide();
signals:
	void Sig_HostAgreeApply();
	void Sig_HostRefuseApply();
	void sig_Destroy(QString id);
	void sig_AgreeUpper(const QString& strId);
	void sig_RejectUpper(const QString& strId);
private slots:
   void Slot_HostAgreeApply();
   void Slot_HostRefuseApply();
   void Slot_ShowTimeOut();
protected:
   void paintEvent(QPaintEvent *);
   bool eventFilter(QObject *o, QEvent *e);
   void showEvent(QShowEvent *);
private:

   Ui::ApplyJoinWdg ui;
   QPixmap mpixmap;
   TitleButton *m_pCloseBtn =  NULL;
   QPoint pressPoint;
   QPoint startPoint;
   bool mbPressed = false;

   QTimer *m_pTimer = NULL;
   int nTimeCount;
	int mnMaxTimeOut = 0;

	QString mStrUid = "";
};

#endif // APPLYJOINWIDGET_H
