#pragma once

#include <QTimer>
#include <QWidget>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include "ui_ShowTipsItem.h"
#include "VhallUI_define.h"

#define ProFileChange_ShowTimeOut   60000  
#define Normal_ShowTimeOut   5000  

class ShowTipsItem : public QWidget
{
    Q_OBJECT

public:
    ShowTipsItem(qint64 index, QWidget *parent = Q_NULLPTR);
    ~ShowTipsItem();

    void SetTipsMsg(const QString& msg, int showTimeout, TipsType type);
    void UpdateChangeProfileState(const QString& msg);
    qint64 CurrentIndex();
    TipsType GetTipsType();
private slots:
    void slot_ClickedClose();
    void slot_close();
    void slot_remove();
    void slot_ChangeProfile();
signals:
    void sig_closeItem(qint64);
    void sig_changeProfile(int curProfile);
    void sig_close();
private:
    Ui::ShowTipsItem ui;
    qint64 mIndex;
    QTimer* mCloseTimerPtr;
    TipsType mTipsType;
    QGraphicsOpacityEffect* m_pGraphicsOpacityEffect = nullptr;
    QGraphicsOpacityEffect* m_plabelTopGraphicsOpacityEffect = nullptr;
    QGraphicsOpacityEffect* m_plabelBottomGraphicsOpacityEffect = nullptr;
    QPropertyAnimation* m_pNameAnimation = nullptr;
    QPropertyAnimation* m_pLableTopAnimation = nullptr;
    QPropertyAnimation* m_pLableBottomNameAnimation = nullptr;
};
