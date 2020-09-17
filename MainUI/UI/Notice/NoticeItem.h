#pragma once

#include <QWidget>
#include "ui_NoticeItem.h"

class NoticeItem : public QWidget
{
    Q_OBJECT

public:
    NoticeItem(QWidget *parent = Q_NULLPTR);
    ~NoticeItem();

    int InsertNoticeMsg(QString msg, const QString& strTime);

private:
    Ui::NoticeItem ui;
};
