#ifndef H_STATEWDG_H
#define H_STATEWDG_H

#include <QWidget>
#include "ui_StateWdg.h"

class StateWdg : public QWidget
{
    Q_OBJECT

public:
    StateWdg(QWidget *parent = Q_NULLPTR);
    ~StateWdg();
	void setExceedHigh(const bool& bEx = false);
	void setTitleText(const QString& strText);
	void setText(const QString& strText);
	void setTextWidth(const int& lw, const int& rw);
	void setIcon(const QChar ch);

private:
	Ui::StateWdg ui;
};

#endif // !H_STATEWDG_H
