#ifndef H_MENUWDG_H
#define H_MENUWDG_H

#include <QDialog>
#include "ui_BtnMenuWdg.h"
#include <QMap>

class BtnMenuItem;

class BtnMenuWdg : public QDialog
{
	Q_OBJECT

public:
	BtnMenuWdg(QWidget *parent = Q_NULLPTR);
	~BtnMenuWdg();
	void show(int x, int y);
	void SetShowItemCount(const int& iCount);

	enum ePlaceType
	{
		ePlaceType_Top = 0,
		ePlaceType_Mid = 1,
		ePlaceType_Bottom = 2,
	};
	void addItem(const int& iOpType, const int&  eType );
	void SetItemShow(const int& iOpType, const bool& bShow = true);
	//void SetItemEnabled(const int& iOpType, const bool& bShow = true);
    void SetEnable(const int& iOpType, bool enable);

	enum eOptype
	{
		eOptype_Member = 0,
		eOptype_Chat,
		eOptype_Interactive
	};

signals:
	void sigClicked(const int& iOpType);
	void sigEnter(const int& iType, const int& iX, const int& iy);
protected:
	void showEvent(QShowEvent *);
	virtual void focusOutEvent(QFocusEvent *);
	virtual void enterEvent(QEvent *);
	virtual void leaveEvent(QEvent *);
private slots:

private:
private:
	Ui::BtnMenuWdg ui;
	bool m_bEnter = false;
	QMap<int, BtnMenuItem*> mlistMenuItem;
	int miCount = 0;
};

#endif H_MENUWDG_H