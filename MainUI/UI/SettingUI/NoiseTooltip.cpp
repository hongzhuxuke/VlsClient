#include "NoiseTooltip.h"

NoiseTooltip::NoiseTooltip(QWidget *parent)
    : QWidget(parent)
{
   ui.setupUi(this);

   setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
   setAttribute(Qt::WA_TranslucentBackground);
   this->setAutoFillBackground(true);
   ui.textEditTips->setReadOnly(true);
	//ui.textEditTips->setText(QStringLiteral("当前输入音量小于噪音闸时，麦克风输出音量自动静音；当音量输入音量大于噪音闸时，音量大于噪音闸的声音会被传输出去。"));
}

NoiseTooltip::~NoiseTooltip()
{

}

void NoiseTooltip::setText(const QString& str)
{
	ui.textEditTips->setText(str);
}
