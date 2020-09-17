#ifndef CPAINTLABEL_H
#define CPAINTLABEL_H
#include "priConst.h"

#include <QLabel>

class CPaintLabel: public QLabel
{
	Q_OBJECT
public:
	CPaintLabel(QWidget *parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());
	CPaintLabel(const QString &text, QWidget *parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());
	~CPaintLabel();
	void SetHandsUpState(eHandsUpState  eState);
signals:
	void sig_DoubleClicked();
protected:
	void paintEvent(QPaintEvent *);
	void mouseDoubleClickEvent(QMouseEvent *event);
	eHandsUpState mState = eHandsUpState_No;
};

#endif // !CPAINTLABEL_H