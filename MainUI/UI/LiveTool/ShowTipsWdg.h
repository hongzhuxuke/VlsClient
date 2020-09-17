#pragma once

#include <QWidget>
#include <QPaintEvent>
#include "ui_ShowTipsWdg.h"
#include "ShowTipsItem.h"

#define AddMaxItemCount 3

class ShowTipsWdg : public QWidget
{
    Q_OBJECT

public:
    ShowTipsWdg(QWidget *parent = Q_NULLPTR);
    ~ShowTipsWdg();

    void AddTips(QString &msg,int itemWidth,int showTimeout, TipsType type = TipsType_Normal);
    void ResizeWidth(int width);

    void ChangeProfileStatus(TipsType type, QString msg);
    void ClearAllTips();
private slots:
    void slot_removeItem(qint64);
    void slot_clickedClose();
    void slot_changeProfile(int curProfile);

protected:
    void paintEvent(QPaintEvent *event);

signals:
    void sig_removeItem();
    void sig_closeItem();
    void sig_changeProfile(int);
private:
    Ui::ShowTipsWdg ui;
};
