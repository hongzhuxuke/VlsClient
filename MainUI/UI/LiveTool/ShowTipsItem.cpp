#include "ShowTipsItem.h"
#include <QPropertyAnimation>
#include "ConfigSetting.h"
#include "DebugTrace.h"

ShowTipsItem::ShowTipsItem(qint64 index, QWidget *parent)
    : QWidget(parent)
    , mIndex(index)
    , mCloseTimerPtr(nullptr)
{
    ui.setupUi(this);
    ui.pushButton_change->hide();
    ui.label_profileTips->hide();
    ui.pushButton_bitrateCtrl->hide();
    mCloseTimerPtr = new QTimer(this);
    if (mCloseTimerPtr) {
        mCloseTimerPtr->setSingleShot(true);
        connect(mCloseTimerPtr, SIGNAL(timeout()), this, SLOT(slot_close()));
    }
    connect(ui.pushButton_closeItem, SIGNAL(clicked()), this, SLOT(slot_ClickedClose()));
    connect(ui.pushButton_change, SIGNAL(clicked()), this, SLOT(slot_ChangeProfile()));
    connect(ui.pushButton_bitrateCtrl, SIGNAL(clicked()), this, SLOT(slot_ChangeProfile()));

    m_pGraphicsOpacityEffect = new QGraphicsOpacityEffect(this);
    if (m_pGraphicsOpacityEffect) {
        m_pGraphicsOpacityEffect->setOpacity(1);
        ui.widget->setGraphicsEffect(m_pGraphicsOpacityEffect);
        m_pNameAnimation = new QPropertyAnimation(m_pGraphicsOpacityEffect, "opacity", this);
        if (m_pNameAnimation) {
            m_pNameAnimation->setEasingCurve(QEasingCurve::Linear);
            m_pNameAnimation->setDuration(1000);
            connect(m_pNameAnimation, SIGNAL(finished()), this, SLOT(slot_remove()));
        }
    }
}

ShowTipsItem::~ShowTipsItem()
{
    if (m_pGraphicsOpacityEffect) {
        delete m_pGraphicsOpacityEffect;
        m_pGraphicsOpacityEffect = nullptr;
    }
    if (m_pNameAnimation) {
        delete m_pNameAnimation;
        m_pNameAnimation = nullptr;
    }
}

qint64 ShowTipsItem::CurrentIndex() {
    return mIndex;
}

void ShowTipsItem::slot_ChangeProfile() {
    if (mCloseTimerPtr) {
        mCloseTimerPtr->stop();
    }
    ui.label_stateTips->hide();
    ui.pushButton_closeItem->hide();
    ui.pushButton_change->hide();
    ui.label_profileTips->hide();
    if (mTipsType == TipsType_ChangeProfile_Standard) {
        ui.label_tipMsg->setText(STD_LEVEL_CHANGING);
    }
    else if (mTipsType == TipsType_ChangeProfile_Fluency) {
        ui.label_tipMsg->setText(FLU_LEVEL_CHANGING);
    }
    emit sig_changeProfile(mTipsType);
    if (mTipsType == TipsType_Bitrate_Down) {
       slot_ClickedClose();
    }
}

void ShowTipsItem::slot_ClickedClose() {
    slot_close();
}

void ShowTipsItem::slot_close() {
    if (mTipsType == TipsType_ChangeProfile_Standard || mTipsType == TipsType_ChangeProfile_Fluency) {
        emit sig_close();
        emit sig_closeItem(mIndex);
    }
    else {
        if (m_pNameAnimation) {
            ui.label->hide();
            ui.label_2->hide();
            m_pNameAnimation->setStartValue(1);
            m_pNameAnimation->setEndValue(0);
            m_pNameAnimation->start(QAbstractAnimation::KeepWhenStopped);
        }
    }
}

void ShowTipsItem::slot_remove() {
    emit sig_closeItem(mIndex);
}

void ShowTipsItem::UpdateChangeProfileState(const QString& msg) {
    ui.label_tipMsg->setText(msg);
    if (mCloseTimerPtr) {
        mCloseTimerPtr->start(Normal_ShowTimeOut);
    }
}

void ShowTipsItem::SetTipsMsg(const QString& msg, int showTimeout, TipsType type) {
   TRACE6("%s msg %s\n",__FUNCTION__,msg.toStdString().c_str());
    ui.label_tipMsg->setText(msg);
    if (type == TipsType_Normal || type == TipsType_OnlyOne_Context || type == TipsType_MIC_NOTICE) {
        ui.pushButton_closeItem->hide();
        ui.label_stateTips->hide();
    }
    else if (type == TipsType_ChangeProfile_Standard) {
        ui.pushButton_closeItem->show();
        ui.label_stateTips->setStyleSheet("border-image: url(:/button/images/warning.png);");
        ui.label_stateTips->show();
        ui.label->setStyleSheet("background-color: rgb(253, 171, 87);");
        ui.label_2->setStyleSheet("background-color: rgb(253, 171, 87);");
        ui.widget->setStyleSheet("QWidget#widget{background-color: rgb(255, 232, 205);;border:0px;}");
        ui.label_tipMsg->setText(QString::fromWCharArray(L"当前电脑性能较差，建议"));
        ui.pushButton_change->show();
        ui.label_profileTips->setText(QString::fromWCharArray(L"至标清"));
        ui.label_profileTips->show();
        showTimeout = ProFileChange_ShowTimeOut;
    }
    else if (type == TipsType_Bitrate_Down) {
       ui.pushButton_closeItem->show();
       ui.label_stateTips->setStyleSheet("border-image: url(:/button/images/warning.png);");
       ui.label_stateTips->show();
       ui.label->setStyleSheet("background-color: rgb(253, 171, 87);");
       ui.label_2->setStyleSheet("background-color: rgb(253, 171, 87);");
       ui.widget->setStyleSheet("QWidget#widget{background-color: rgb(255, 232, 205);;border:0px;}");
       ui.label_tipMsg->setText(msg);
       ui.pushButton_change->hide();
       ui.pushButton_bitrateCtrl->show();
       ui.label_profileTips->hide();
       showTimeout = 5000;
    }
    else if (type == TipsType_ChangeProfile_Fluency) {
        ui.pushButton_closeItem->show();
        ui.label_stateTips->setStyleSheet("border-image: url(:/button/images/warning.png);");
        ui.label_stateTips->show();
        ui.label->setStyleSheet("background-color: rgb(253, 171, 87);");
        ui.label_2->setStyleSheet("background-color: rgb(253, 171, 87);");
        ui.widget->setStyleSheet("QWidget#widget{background-color: rgb(255, 232, 205);;border:0px;}");
        ui.label_tipMsg->setText(DEV_CPU_HIGH);
        ui.pushButton_change->show();
        ui.label_profileTips->setText(TO_FLU);
        ui.label_profileTips->show();
        showTimeout = ProFileChange_ShowTimeOut;
    }
    else if (type == TipsType_Success) {
        ui.label_stateTips->setStyleSheet("border-image: url(:/button/images/success.png);");
        ui.label_stateTips->show();
        ui.pushButton_closeItem->hide();
    }
    else if (type == TipsType_SystemBusy ) {
        ui.pushButton_closeItem->show();
        ui.label_stateTips->setStyleSheet("border-image: url(:/button/images/warning.png);");
        ui.label_stateTips->show();
        ui.label->setStyleSheet("background-color: rgb(253, 171, 87);");
        ui.label_2->setStyleSheet("background-color: rgb(253, 171, 87);");
        ui.widget->setStyleSheet("QWidget#widget{background-color: rgb(255, 232, 205);;border:0px;}");
        showTimeout = ProFileChange_ShowTimeOut;
    }
    else if (type == TipsType_Error || type == TipsType_MediaFilePlay) {
        ui.label_stateTips->setStyleSheet("border-image: url(:/button/images/warning.png);");
        ui.label_stateTips->show();
        ui.pushButton_closeItem->hide();
        ui.label->setStyleSheet("background-color: rgb(253, 171, 87);");
        ui.label_2->setStyleSheet("background-color: rgb(253, 171, 87);");
        ui.widget->setStyleSheet("QWidget#widget{background-color: rgb(255, 232, 205);;border:0px;}");
    }
    if (mCloseTimerPtr) {
        mCloseTimerPtr->start(showTimeout);
    }
    mTipsType = type;
}

TipsType ShowTipsItem::GetTipsType() {
    return mTipsType;
}