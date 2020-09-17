#ifndef H_WEBINARIDWDG_H
#define H_WEBINARIDWDG_H

#include "CWidget.h"
#include "ui_webinarIdWdg.h"
#include <QFont>

class webinarIdWdg : public CWidget
{
	Q_OBJECT

public:
	webinarIdWdg(QWidget *parent = Q_NULLPTR);
	~webinarIdWdg();
	void SetId(const QString& id);
	QString strStreamid();
signals:
    void sig_copy();
protected:
private slots:
    void slot_ClickedCopy();
private:

private:
	Ui::webinarIdWdg ui;

};

#endif //H_WEBINARIDWDG_H