#pragma once

#include <QWidget>
#include "ui_FullDocEscTipWdg.h"

class FullDocEscTipWdg : public QWidget
{
    Q_OBJECT

public:
    FullDocEscTipWdg(QWidget *parent = Q_NULLPTR);
    ~FullDocEscTipWdg();

private:
    Ui::FullDocEscTipWdg ui;
};
