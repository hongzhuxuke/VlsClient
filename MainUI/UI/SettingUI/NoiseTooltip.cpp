#include "NoiseTooltip.h"

NoiseTooltip::NoiseTooltip(QWidget *parent)
    : QWidget(parent)
{
   ui.setupUi(this);

   setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
   setAttribute(Qt::WA_TranslucentBackground);
   this->setAutoFillBackground(true);
   ui.textEditTips->setReadOnly(true);
	//ui.textEditTips->setText(QStringLiteral("��ǰ��������С������բʱ����˷���������Զ�����������������������������բʱ��������������բ�������ᱻ�����ȥ��"));
}

NoiseTooltip::~NoiseTooltip()
{

}

void NoiseTooltip::setText(const QString& str)
{
	ui.textEditTips->setText(str);
}
