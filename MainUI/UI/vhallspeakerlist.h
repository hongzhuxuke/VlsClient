#ifndef VHALLSPEAKERLIST_H
#define VHALLSPEAKERLIST_H

#include <QWidget>
#include <QTimer>
#include <QMap>

#include "ToolButton.h"
#include "vhallspeakerlistitem.h"
#define TIMER_OUT_BUTTON_DISABLE 30000    //设置发起上麦、下麦操作超时时间。

namespace Ui {
class VhallSpeakerList;
}

class VhallSpeakerList : public QWidget
{
    Q_OBJECT

public:
    explicit VhallSpeakerList(QWidget *parent = 0);
    ~VhallSpeakerList();

    void Clear();

    //初始状态，如果为嘉宾则显示上麦按钮。
    void SetRole(bool Master);
    bool IsExistSpeak(const QString& id);

    void OnJoinActiveSuc(bool suc);
    void OnLeaveActiveSuc(bool suc);

    //void Append(QString id,QString name , bool master);
    void SetCameraIcon(QString id,bool );

    bool SetMicIcon(QString id,bool );
    bool GetMicState(const QString id);
    void Remove(QString id);
    //void RefreshOnlineList();
    void SetUserId(QString id){mUserId = id;}
    QList<QString> GetSpeakListUsers();
    void SetPushState(bool pushing);
    QMap<QString, bool> GetAllOpenMicUsers();
    
public:
    //设置上下麦回调按键状态。
    void OnUpdateJoinState(bool offLine);
private:
   VhallSpeakerListItem *GetItem(QString id);
private slots:
   void Slot_JoinActivity();
   void Slot_CancelJoinActivity();
   void Slot_ButtonDisableTimeOut();
   void Slot_SpeakerButtonClicked();
      
signals:
   void Sig_JoinActivity();
   void Sig_CancelJoinActivity();
   void Sig_ItemClicked(QString);
private:
    Ui::VhallSpeakerList *ui;
    bool bGuest = false;

    QTimer *m_pTimer = NULL;
    QToolButton *m_pSpeakerButton = NULL;
    bool m_bSpeakerStatus = false;
    bool m_bMaster = false;

    QMap<QString,bool> mCameraIcons;
    QMap<QString,bool> mMicIcons;
    QString mUserId;
    bool m_bPushing = false;
};

#endif // VHALLSPEAKERLIST_H
