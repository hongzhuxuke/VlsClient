#ifndef H_CSUSPENSIONDLG_H
#define H_CSUSPENSIONDLG_H


#include <QDialog>
#include <QMouseEvent> 

#include "ui_suspensionDlg.h"

class CSuspensionDlg : public QDialog
{
	Q_OBJECT

public:
	CSuspensionDlg(QWidget *parent = Q_NULLPTR);
	~CSuspensionDlg();
	void appentWdg(QWidget* pwd);
	void removeWdg(QWidget* pwd);
	void removeWdg();
signals:
	void SigClose();
protected:
	virtual void mouseMoveEvent(QMouseEvent *event);
	virtual void mousePressEvent(QMouseEvent *event);
	virtual void mouseReleaseEvent(QMouseEvent *event);
	virtual void showEvent(QShowEvent *event);
private slots:
	void on_btnClose_clicked(bool checked = false);
private:
	
	Ui::suspensionDlg ui;
	bool m_bTitlePressed = false;
	QPoint pressPoint;
	QPoint startPoint;
	QWidget* mpWdg = NULL;
};
#endif//H_CSUSPENSIONDLG_H