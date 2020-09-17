#ifndef CTabItemDlg_H
#define CTabItemDlg_H


#include "ui_CTabItemDlg.h"
#include <QCoreApplication>
#include <QMouseEvent>

enum {
   NORMAL = 0,
   TOPLEFT = 11,
   TOP = 12,
   TOPRIGHT = 13,
   LEFT = 21,
   CENTER = 22,
   RIGHT = 23,
   BUTTOMLEFT = 31,
   BUTTOM = 32,
   BUTTOMRIGHT = 33
};

#define FRAMESHAPE 10    


class QGraphicsDropShadowEffect;
class CTabItemDlg: public QDialog {
    Q_OBJECT

public:
   CTabItemDlg(QWidget *parent = 0);
   ~CTabItemDlg();
   QObjectList GetChildren();
   void SetTitle(const QString& strTitle);
   void AddCenterWdg(QWidget* pWdg);
   void RemoveCenterWdg(QWidget* pWdg);
   void SetMaxSizeState();
   void setCursorShape(int CalPos);    //设置鼠标对应位置的形状
   void SetBtnShow(bool is_show_fresh,bool is_show_max);
private slots:
   void slot_OnFresh();

signals:
	void sig_doubleClickedTitleBar();  //被双击时发射的信号
   void sig_clickMaxBtn();
   void sig_OnFresh();

protected:
	bool event(QEvent *);
   void showEvent(QShowEvent *e);

protected:
    void mousePressEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void leaveEvent(QEvent *event);
    bool eventFilter(QObject *obj, QEvent *event);
    void keyPressEvent(QKeyEvent *e);
    bool m_bIsEnableMove = true;

private:
    QPoint mPressPoint;
    bool mIsMoved = false;

	Ui::CTabItemDlg ui;
   bool mbIsMax = false;
   int     m_iCalCursorPos;
   bool    m_bLeftPress;
   QRect   m_rtPreGeometry;
   QPoint  m_ptViewMousePos;
   QString mTitleStr;
};

#endif // CTabItemDlg_H
