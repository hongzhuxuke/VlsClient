#ifndef VHALLAUDIENCEWIDGET_H
#define VHALLAUDIENCEWIDGET_H

#include <QWidget>
#include <QAbstractButton>
#include <QPaintEvent>
#include "vhalluserinfomanager.h"

namespace Ui {
class VhallAudienceWidget;
}

class VhallAudienceWidget : public QWidget
{
    Q_OBJECT

public:
    explicit VhallAudienceWidget(QWidget *parent = 0);
    ~VhallAudienceWidget();
    void SyncOnlineList(VhallAudienceUserInfoList &specialList,VhallAudienceUserInfoList &onlineList,VhallAudienceUserInfoList &gagList);
    void SyncGagList(VhallAudienceUserInfoList &);
    void SyncKickList(VhallAudienceUserInfoList &);
    void SetOnlineNumber(int);
    void SetGagNumber(int);
    void SetKickNumber(int);

    void RemoveOnlineList(VhallAudienceUserInfo &info);
    void AppendOnlineList(VhallAudienceUserInfo &info);
    
    void RemoveGagList(VhallAudienceUserInfo &info);
    void AppendGagList(VhallAudienceUserInfo &info);

    void RemovekickList(VhallAudienceUserInfo &info);
    void AppendkickList(VhallAudienceUserInfo &info);

    void ChangeOnlineList(VhallAudienceUserInfo &info);
    void ChangeGagList(VhallAudienceUserInfo &info);
    void ChangekickList(VhallAudienceUserInfo &info);
    int Count();
    void TabPressed();
    void Clear();
    void SetIsHost(bool ok);
    void setSelfInfo(VhallAudienceUserInfo& info);
    void ReleaseSelf();
signals:
    void SigSendMsg(MsgRQType eMsgType, RQData vData);
    void SendTest();
private slots:
    void on_buttonGroup_buttonClicked(QAbstractButton * button);
private:
    void BtnClicked(QAbstractButton * button);
    void SendMsg(MsgRQType eMsgType, RQData vData);

private:
   Ui::VhallAudienceWidget *ui;
   bool mbHost;
   VhallAudienceUserInfo* m_pSelfInfo = NULL;
};

#endif // VHALLAUDIENCEWIDGET_H
