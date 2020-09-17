#ifndef VHALLUSERLISTWIDGET_H
#define VHALLUSERLISTWIDGET_H

#include <QWidget>
#include <QTimer>

#include "vhalluserlistitem.h"

namespace Ui {
class VhallUserListWidget;
}

class VhallUserListWidget : public QWidget
{
    Q_OBJECT

public:
    explicit VhallUserListWidget(QWidget *parent = 0);
    ~VhallUserListWidget();
    void Append(VhallAudienceUserInfo &,bool bOpera,int pageType);
    void Remove(VhallAudienceUserInfo &);
    void Change(VhallAudienceUserInfo &);
    void ClearExceptParamter(VhallAudienceUserInfo* SelfInfo, bool bHost,int pageType);
    int Count();
    void SetMonitorVerticalBar(bool);
    void SetGagHide(bool);
    void SetShotHide(bool);
    void SetIsHost(bool,int pageType);
    void SetPageListType(int);
protected:
    bool eventFilter(QObject *,QEvent *);
signals:
    void GetNewPage(int);
    void Sig_MouseRightUserListItem(QString id, QString role);

public slots:
    void ListValueChangeSlots(int);
    void timeout();
private:
    Ui::VhallUserListWidget *ui;
    QTimer m_timer;
    int m_pageControl;
    bool isGagHide = false;
    bool isShotHide = false;
    int mPageType = 0;
};

#endif // VHALLUSERLISTWIDGET_H
