#pragma once

#include <QDialog>
#include "ui_NoticeSendWdg.h"
//#include "../Unility/Unility.h"
#include <QEvent>
#include <QCloseEvent>

class NoticeShowWdg;

class NoticeSendWdg : public QDialog
{
    Q_OBJECT

public:
    NoticeSendWdg(QWidget *parent = Q_NULLPTR);
    ~NoticeSendWdg();
	void AppendMsg(QString msg, const QString& strTime);

private slots:
    void slot_sendMsg();
    void slot_textChanged();
    void onSliderChanged(int);

private:
    Ui::NoticeSendWdg ui;
};
