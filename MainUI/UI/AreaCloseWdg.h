#pragma once

#include <QWidget>
#include <QPaintEvent>
#include "ui_AreaCloseWdg.h"

class AreaCloseWdg : public QWidget
{
    Q_OBJECT

public:
    AreaCloseWdg(QWidget *parent = Q_NULLPTR);
    ~AreaCloseWdg();

    void SetTextTips(QString content);

private slots:
    void slot_closeAreaShare();
signals:
    void sig_clicked();

protected:
    void paintEvent(QPaintEvent *event);
private:
    Ui::AreaCloseWdg ui;
};
