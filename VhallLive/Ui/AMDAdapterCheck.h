#pragma once

#include <QWidget>
#include "cbasedlg.h"
#include "ui_AMDAdapterCheck.h"

class AMDAdapterCheck : public CBaseDlg
{
    Q_OBJECT

public:
    AMDAdapterCheck(QString name,QWidget *parent = Q_NULLPTR);
    ~AMDAdapterCheck();

private:
    Ui::AMDAdapterCheck ui;
};
