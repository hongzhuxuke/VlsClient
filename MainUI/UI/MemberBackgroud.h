#pragma once

#include <QWidget>
#include "ui_MemberBackgroud.h"

class MemberBackgroud : public QWidget
{
    Q_OBJECT

public:
    MemberBackgroud(QWidget *parent = Q_NULLPTR);
    ~MemberBackgroud();
    void SetBackGroud(QString image,QString notice);
private:
    Ui::MemberBackgroud ui;
};
