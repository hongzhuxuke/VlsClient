#ifndef CDESKTOPMODECHATPARENTWDG_H
#define CDESKTOPMODECHATPARENTWDG_H

#include <QWidget>
#include "ui_cdesktopmodechatparentwdg.h"

class CDesktopModeChatParentWdg : public QWidget
{
    Q_OBJECT

public:
    CDesktopModeChatParentWdg(QWidget *parent = 0);
    ~CDesktopModeChatParentWdg();

protected:
   void showEvent(QShowEvent *event);

private:
    Ui::CDesktopModeChatParentWdg ui;
};

#endif // CDESKTOPMODECHATPARENTWDG_H
