//CTabBar.h
#ifndef CTABBAR_H
#define CTABBAR_H
#include <QTabBar>
#include <QtGui>
 
class CTabBar :public QTabBar
{
	Q_OBJECT
public:
	CTabBar(QWidget* =0);
	virtual ~CTabBar(){};
	//void currentChanged();
signals:
	void sig_tabDrag(int, QPoint,bool drag);
   void sig_clickedTabDrag(int, QPoint);
public slots:
	void slot_DoubleClicked();
protected:
	void mousePressEvent(QMouseEvent *);
	void mouseReleaseEvent(QMouseEvent *);   //ͨ�������¼�ģ���tab���϶��Ķ���
	void mouseDoubleClickEvent(QMouseEvent *event);
private:
	bool mPressFlg;
	bool mDoubleClicked = false;
};
#endif
 