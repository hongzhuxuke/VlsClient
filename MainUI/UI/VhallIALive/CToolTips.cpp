#include "CToolTips.h"

CToolTips::CToolTips(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
}

CToolTips::~CToolTips()
{
}

void CToolTips::SetText(QString text) {
	ui.label_tips->setText(text);
   repaint();
}

void CToolTips::setSuspension()
{
	setWindowFlags(Qt::Tool | Qt::FramelessWindowHint);
	setAttribute(Qt::WA_TranslucentBackground);
}