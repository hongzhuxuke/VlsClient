#ifndef CENTERNOTICEWDG_H
#define CENTERNOTICEWDG_H

#include <QWidget>
#include "ui_CenterNoticeWdg.h"
#include <QPixmap>

class CCenterNoticeWdg : public QWidget
{
    Q_OBJECT

public:
    CCenterNoticeWdg(QWidget *parent = 0);
    ~CCenterNoticeWdg();

    void SetNoticeType(int type);
    void SetPhotoPixmap(const QPixmap& pix);
    void SetIsFullState(bool full);

protected:
   virtual void mouseMoveEvent(QMouseEvent *);
   virtual void contextMenuEvent(QContextMenuEvent *);
   virtual void mouseDoubleClickEvent(QMouseEvent *);
   void EnableMouseTracking();
   void DisableMouseTracking();
signals:
   void Sig_MouseMoveInCenterNoticeWdg();
   void Sig_ContextMenuEvent();
   void Sig_MouseDoubleClicked();
private:
    Ui::CCenterNoticeWdg ui;
    QPixmap m_PhotoPixmap;
    bool m_bIsFullWnd  = false;
};

#endif // CENTERNOTICEWDG_H
