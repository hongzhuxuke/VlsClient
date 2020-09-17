#include "LiveContentCtrl.h"
#include "webinarIdWdg.h"
#include "pub.Const.h"
#include "ConfigSetting.h"

#define SwitchBtn       "QPushButton{border-image:url(:/button/images/switch.png);}QPushButton:hover{border-image:url(:/button/images/switch_hover.png);}"
#define MicMuteBtn      "QPushButton{border-image:url(:/button/images/mic_mute.png);}QPushButton:hover{border-image:url(:/button/images/mic_mute_hover.png);}"
#define MicBtn          "QPushButton{border-image:url(:/button/images/mic.png);}QPushButton:hover{border-image:url(:/button/images/mic_hover.png);}"


LiveContentCtrl::LiveContentCtrl(QWidget *parent)
    : CWidget(parent)
{
    ui.setupUi(this);
	//this->setWindowOpacity(0.8);
	this->setWindowFlags(Qt::FramelessWindowHint |Qt::WindowMinimizeButtonHint);
	setAttribute(Qt::WA_TranslucentBackground);
	setAutoFillBackground(false);
   setPalette(Qt::transparent);

	ui.pushButton_ChangeView->setStyleSheet(SwitchBtn);
	ui.pushButton_ChangeView->setToolTip(CHANGE_LEVEL);
	connect(ui.pushButton_ChangeView, SIGNAL(clicked(bool)), this, SLOT(slot_OnChangeView()));

	ui.pushButton_mic->setStyleSheet(MicBtn);
	ui.pushButton_mic->setToolTip(MIC_DEV);
	connect(ui.pushButton_mic, SIGNAL(clicked()), this, SLOT(slot_OnMicClicked()));
}

LiveContentCtrl::~LiveContentCtrl()
{
}


void LiveContentCtrl::slot_SetMicState(const bool & bOpen)
{
	bOpen? ui.pushButton_mic->setStyleSheet(MicBtn): ui.pushButton_mic->setStyleSheet(MicMuteBtn);
}

void LiveContentCtrl::SetMicShow(const bool& bShow/* = true*/)
{
	bShow ? ui.pushButton_mic->show() : ui.pushButton_mic->hide();
}

void LiveContentCtrl::enterEvent(QEvent *event) {
    show();
}

void LiveContentCtrl::leaveEvent(QEvent *event) {
    hide();
}

void LiveContentCtrl::slot_OnChangeView() {
    hide();
    emit sig_ChangeView();
}

void LiveContentCtrl::slot_OnMicClicked() {
    emit sig_MicClicked();
}