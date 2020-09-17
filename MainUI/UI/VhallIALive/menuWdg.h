#pragma once

#include <QDialog>
#include "ui_MenuWdg.h"
#include <QMap>

class MenuItem;
class QGraphicsDropShadowEffect;
class MenuWdg : public QDialog
{
	Q_OBJECT

public:
	MenuWdg(QWidget *parent = Q_NULLPTR);
	~MenuWdg();
	void Show(int x, int y);
	void SetShowItemCount(const int& iCount);
	void addItem(const QString& strText, 
		const QString& strTextHover,
		const QString& strTextLeave,
		const int& iOpType,
		const QString& strIcoHover = "",
		const QString& strIcoLeave = "");
		void SetItemShow(const int& iOpType, const bool& bShow = true);
signals:
	void sigClicked(const int& iOpType);

protected:
	void showEvent(QShowEvent *);
	virtual void focusOutEvent(QFocusEvent *);
	virtual void enterEvent(QEvent *);
	virtual void leaveEvent(QEvent *);
private slots:

private:
private:
	Ui::menuWdg ui;
	bool m_bEnter = false;
	QMap<int, MenuItem*> mlistMenuItem;
	int miCount = 0;
	QGraphicsDropShadowEffect* mpEffect = nullptr;
};
