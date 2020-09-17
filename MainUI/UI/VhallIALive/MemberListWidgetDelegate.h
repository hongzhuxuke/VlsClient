#pragma once
#include <QObject>
#include <QTimer>
#include <QStyledItemDelegate>
#include "vhalluserinfomanager.h"
#include "IInteractionClient.h"

#define SHOW_MORE_MENU     Qt::UserRole + 1
#define SHOW_CHAT_FORBID   Qt::UserRole + 2
#define SHOW_MAIN_VIEW     Qt::UserRole + 3
#define SHOW_KICK_OUT      Qt::UserRole + 5
#define SHOW_PHONE         Qt::UserRole + 6
#define SHOW_SPEAK_STATE   Qt::UserRole + 7
#define SHOW_HANDSUP_TIME  Qt::UserRole + 8
#define SHOW_HEAD_IMAGE    Qt::UserRole + 9


#define STATE_HIDE_HANDS_UP  0
#define STATE_HANDS_UP  1
#define STATE_SPEAKING  2
#define STATE_STAR      3

enum eOptype
{
   eOptype_Forbidden = 0,
   eOptype_CancleForbidden,
   eOptype_KickOut,
   eOptype_CancleKickOut,
   eOptype_setMain,
};

class MenuWdg;
class CAliveDlg;

class MemberListWidgetDelegate : public QStyledItemDelegate
{
   Q_OBJECT

public:
    MemberListWidgetDelegate(QObject *parent);
    ~MemberListWidgetDelegate();

private:
   //决定如何绘图
   virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
   //决定单元格的推荐大小
   virtual QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;

private slots:
   void slotCliecked(const int& iOpType);
   void slot_upperTimeOut();
   void Slot_HandsUpTimeout();
private:
   void btnProhibitSpeakingClicked(const bool& checked);
   void btnbtnKickOutClicked(const bool& checked); 
   void SetMainView();
   void sendOperatRQ(const MsgRQType& type);
   void ClickedMenu(QString id, QString name,QPoint pos,int showChatForbid, int showKickOu);
   void SpeakState(QString speak_state);
signals:
   void sig_selfUpper(const bool& bUpper);
   void sig_setMainSpeaker(QString join_uid, QString role);
   void sig_AgreeUpper(const QString& strId);
   void sig_DownLoadAvatar(QString join_id, QString path);
   void sig_OperationUser(RQData& RqData);
   void sig_btnUpper_clicked(const QString& strId, const bool& bOperation = true);

protected:
   bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index);

private:
   MenuWdg* mPMoreOpMenu = nullptr;
   QString mStrUserId;
   QString mRoleName;
   QString mNickName;
   VhallAudienceUserInfo mUserInfo;
   int mIWeight;
   CAliveDlg* mpAlive = nullptr;
   QTimer* mpUpperTime = NULL;
   QTimer* mpHandsUpCheckTimer = NULL;
   bool mIsShowHandsUp = false;
   bool mIsEnableInvite = true;
   bool mIsEnterSpeak = false;
   bool mIsEnterItem = false;
};
