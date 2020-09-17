#pragma once

#include "CWidget.h"
#include <QEvent>
#include "ui_LiveContentCtrl.h"

class LiveContentCtrl : public CWidget
{
    Q_OBJECT

public:
    LiveContentCtrl(QWidget *parent = Q_NULLPTR);
    ~LiveContentCtrl();
	
	void SetMicShow(const bool& bShow = true);
public slots:
	void slot_SetMicState(const bool & bOpen = true);
private slots:
    void slot_OnChangeView();
    void slot_OnMicClicked();

signals:
    void sig_ChangeView();
	void sig_MicClicked();

protected:
    virtual void enterEvent(QEvent *event);
    virtual void leaveEvent(QEvent *event);
private:
    Ui::LiveContentCtrl ui;
    QWidget *mParentWdgPtr = nullptr;
};
