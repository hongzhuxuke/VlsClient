#pragma once

#include <QWidget>
#include "ui_CToolTips.h"

class CToolTips : public QWidget
{
	Q_OBJECT

public:
	CToolTips(QWidget *parent = Q_NULLPTR);
	~CToolTips();
	void setSuspension();
	void SetText(QString text);

private:
	Ui::CToolTips ui;
};
