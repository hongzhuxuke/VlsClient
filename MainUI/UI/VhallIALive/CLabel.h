#pragma once

#include <QLabel>

class CLabel : public QLabel
{
	Q_OBJECT

public:

	CLabel(QWidget *parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());
	CLabel(const QString &text, QWidget *parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());
	~CLabel();
protected:
	void paintEvent(QPaintEvent *event);
};
