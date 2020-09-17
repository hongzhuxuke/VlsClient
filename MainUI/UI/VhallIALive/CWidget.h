#pragma once

#include <QWidget>

class CWidget : public QWidget
{
	Q_OBJECT

public:
	CWidget(QWidget *parent);
	~CWidget();
protected:
	void paintEvent(QPaintEvent *event);
};
