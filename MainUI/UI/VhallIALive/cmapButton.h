#pragma once
#include <QToolButton>
class CMapButton : public QToolButton
{
	Q_OBJECT

public:
	CMapButton(QWidget *parent = Q_NULLPTR);
	~CMapButton();

	void setHoverStyle(const QString & str);
	void setLeaveStyle(const QString & str);

	void setMapButton(CMapButton* pbtn);
	void setEnabled(bool enable);
   bool bEnter() {return m_bEnter;}

   void EnterButton();
   void LeaveButton();
signals:
	void sig_hover();
	void sig_leave();
   void sigfocusOut();

public slots:
	void slot_hover();
	void slot_leave();

protected:
	void enterEvent(QEvent *event);
	void leaveEvent(QEvent *event);
   void focusOutEvent(QFocusEvent *);
private slots:


private:
	QString mStrHoverStyle;
	QString mStrLeaveStyle;
   bool m_bEnter = false;
   CMapButton* mpBtn = nullptr;
};
