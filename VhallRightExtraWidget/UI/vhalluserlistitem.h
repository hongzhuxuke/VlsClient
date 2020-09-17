#ifndef VHALLUSERLISTITEM_H
#define VHALLUSERLISTITEM_H

#include <QWidget>
#include "vhalluserinfomanager.h"
#include "ToolButton.h"
namespace Ui {
class VhallUserListItem;
}

class VhallUserListItem : public QWidget
{
    Q_OBJECT

public:
    explicit VhallUserListItem(QWidget *parent = 0);
    ~VhallUserListItem();
    void SetUserInfo(VhallAudienceUserInfo &,int pageType);
    void ShowGag(VhallShowType,int pageType);
    void ShowKick(VhallShowType, int pageType);
    bool HasUserInfo(const VhallAudienceUserInfo &);
    VhallAudienceUserInfo & GetUserInfo() ;
    void SetOperator(bool,int pageType);
private:
   QString geteElidedText(QFont font, QString str, int MaxWidth);
protected:
    void enterEvent(QEvent *);
    void leaveEvent(QEvent *);
    virtual void contextMenuEvent(QContextMenuEvent *);

public slots:
   void GagClicked();
   void KickClicked();

private:
    Ui::VhallUserListItem *ui;
    ToolButton *m_btnGag;
    ToolButton *m_btnKick;

    VhallShowType m_gagType;
    VhallShowType m_kickType;
    VhallAudienceUserInfo userInfo;
    bool mbOpera;
};

#endif // VHALLUSERLISTITEM_H
