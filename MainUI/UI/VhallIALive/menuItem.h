#pragma once

#include <QWidget>
#include "ui_MenuItem.h"

class MenuItem : public QWidget
{
	Q_OBJECT

public:
	MenuItem(QWidget *parent = Q_NULLPTR);
	~MenuItem();

	enum eBtnId
	{
		eBtnId_icon,
		eBtnId_text,
	};

	void setHoverStyle(const QString & str, eBtnId id);
	void setLeaveStyle(const QString & str, eBtnId id);
	void SetOpType(const int& iOpType);
	void SetText(const QString & str);
signals:
	void sigClicked(const int& iOpType);
protected:
   void enterEvent(QEvent *event);
   void leaveEvent(QEvent *event);

private slots:
	void on_tbtnIcon_clicked(bool checked = false);
	void on_tBtnText_clicked(bool checked = false);
private:
private:
	Ui::menuItem ui;

	bool m_bEnter = false;
	int miOpType;
};
