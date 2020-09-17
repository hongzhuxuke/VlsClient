#ifndef H_MEMBERLISTWDG_H
#define H_MEMBERLISTWDG_H

#include <QWidget>
#include <QMouseEvent> 
#include <QMap>
#include <QString>
#include "ui_MemberListWdg.h"
#include "vhalluserinfomanager.h"
#include "pub.Const.h"
#include "priConst.h"
#include "MemberItemWdg.h"

class COnlineItemWdg;
class CAliveDlg;
class QTimer;
class QPainter;

bool compareOnlineData(const COnlineItemWdg& pwdg1, const COnlineItemWdg& pwdg2);

template<class T>
void AddUserList(const UserList& userList, const bool& bExtend, const bool& bForAll, T* obj, bool(T::*temF)(const VhallAudienceUserInfo &, const bool&, const bool&));

class MemberListWdg : public QWidget
{
   Q_OBJECT

public:
   MemberListWdg(QWidget *parent = Q_NULLPTR);
   ~MemberListWdg();

   void InitLiveTypeInfo(bool isHost, bool is_rtc_live);
   void RecvUserListMsg(Event& event);
   void SetCurrentListTab(const eTabMember& tabM);
   void NtfUserOffLine(const QString& strId, int iUserCount);
   void NtfUserOnline(const VhallAudienceUserInfo &Info);
   void SetAlive(CAliveDlg* pAlive);
   void setHansUpStates(const bool& bChecked);
   void SetHansUpShow(const bool& bshow);
   bool GetHansUpShow();
   void clearData();
   void setUserCount(int iCount = -1);
   bool IsUserHandsUp(QString id);
   void SetEnableWheat(const bool& bEnable);
   QString GetUserNickName(const QString& strId);
   void UpdataOnLineUserHandsUpState(const QString& strId, bool handsUp);
   void ResetSpeakState();
   void ResetSpeakState(QListWidget* listWidget);
   //bUpper : true 显示上麦， false  显示下麦
   void SetSelfUpperWheat(const bool& bUpper = true);
   long long GetShowUserMember();
   void ShowSetmainSpeaker(const QString& join_uid);
   void FlushBtnState(const QString& join_uid);
   void HandleDownLoadHead(QEvent *event);

   void DealSwitchHansUp(bool open);
   void CancelHandsUpIcon();

   bool addRaiseHandsMember(const VhallAudienceUserInfo &info, const bool& bextend = true, const bool& bForAll = true);
   void SetUpperStatesById(const QString& strId, const bool& bUpper);//true 显示下麦 false显示邀请上麦
signals:
   void sig_selfUpper(const bool& bUpper);
   void sig_AgreeUpper(const QString& strId);
   void signal_Refresh();
   void signal_Upper(const QString& strId, const bool& bOperation = true);
   void sigAllowRaiseHands(const bool& checked);
   void sig_OperationUser(RQData& RqData);
   void sig_ReqApointPageUserList(const int& iApoint);
   void sig_Hide();
   void sig_setMainSpeaker(QString join_uid, QString role);
   void sig_ClickedHandsUpBtn();
public slots:
   void show(const QRect& rect);
   void slot_Upper(const QString& strId, const bool& bUpper);
   void slot_SearchSignalMember();
protected:
   //virtual void paintEvent(QPaintEvent *);

   void customEvent(QEvent*);
   bool eventFilter(QObject *object, QEvent *event);

private slots:
   void slot_mouseMove();
   void slot_OnCheckMemberListTimeout();
   void on_btnClose_clicked(bool checked = false);
   void on_btnMin_clicked(bool checked = false);
   void on_btnOnLine_clicked(bool checked = false);
   void on_btnRaiseHands_clicked(bool checked = false);
   void on_btnProhibitSpeaking_clicked(bool checked = false);
   void on_btnKickOut_clicked(bool checked = false);
   void on_btnRefresh_clicked(bool checked = false);
   void on_tbtnAllowRaiseHands_clicked(bool checked = false);
   void slotEnableRefresh();
   void on_btnLookUp_clicked(bool checked /*= false*/);
   void on_btnClear_clicked(bool checked /*= false*/);
   void on_ledtLookUp_textChanged(const QString &text);
   void slot_HideFindMemberNotice();
   void slot_OnTextChanged(const QString &text);
public slots:
   void slot_OnLineMemberSliderChanged(int v);
   void slot_DownLoadPic(QString, QString);
   void slot_HostCheckHandsUpTimeout();
private:
   void StartCheckMemberList();
   void initUpper(COnlineItemWdg* pwd, QString id);
   void UpdatePageBtn(int page);
   void startRefreshUpdateTime();
   bool addOnlineMember(const VhallAudienceUserInfo &info, const bool& bSpecial = false, const bool& bForAll = true);
   void SortMemberItem(QListWidget* memberListWidget, const VhallAudienceUserInfo &info, QListWidgetItem* item);
   void LoadMemberHeadImage(const VhallAudienceUserInfo &info, QListWidgetItem* item);
   bool addForbidChatMember(const VhallAudienceUserInfo &inf, const bool& bForbid = true, const bool& bForAll = true);
   bool addKickOutMember(const VhallAudienceUserInfo &info, const bool& bKickOut = true, const bool& bForAll = true);
   void HandleSearchMember(Event& e);
   void HandleGetUserOnlineList(Event& e);
   void HandleGetUserSpecialList(Event& e);
   QListWidgetItem* getOnlineItemWdt(const QString& strId);
   void RemoveOnLineItemWdt(const QString& strId);
   void updateReplyInvite(const VhallAudienceUserInfo &info, const bool& bUpperSuc);
   void calcListNum();
   int calcWdgHeight(const int& iRowCount) const;
   void dealNotSpeak(const QString& strId);
   void addSpeaker(Event& e);
   void DealSwitchHandsUp(const Event& e);
   bool RemoveInfo(QListWidget *pListWdt, const QString &strId);
   bool IsExistInKickOutList(const QString &strId);
   bool IsExistUserInList(QListWidget* listWidget, const QString &strId);
   void clearInfoList(QListWidget *pListWdt);
   void ProhibitChat(const bool& bProhibit = true);
   void ProhibitChatWdg(QListWidget *pListWdt, const bool& bProhibit = true);
   void HandleSetMain(const Event& e);
   void RemoveFirstUserMembmer();
   void CheckShowMenu(QString insertUser, QListWidgetItem* insertItem);
   void CheckShowSpeakState(QString user_id, QString role_name, bool isSpeak, QListWidgetItem* insertItem);
private:
   Ui::MemberListWdg ui;

   bool mbIsHost = false;
   bool mbIsRtcLive = false;

   map<QString, QListWidgetItem*> mOnlineItemMap;        //在线展示列表
   map<QString, QListWidgetItem*> mForbidItemMap;        //禁言、踢出受限成员
   map<QString, QListWidgetItem*> mRaiseHandsItemMap;    //举手成员

   CAliveDlg* mpAlive = NULL;
   //QPixmap mPixmap;
   QTimer* mpRefreshTime = NULL; //刷新按钮定时器

   int miOnlineUserCont = 0;	//用户数量(普通用户+特殊用户)
   int mPageCont = 1;//总页数
   int mCurPage = 1; //当前显示的页数
   int mCurStartPageNum = 1;

   int mReqPage = 1;
   bool bChecked = false;
   QRect mNormalRect;
   QListWidget* mEnterMemberList = nullptr;
   QTimer *mFindMemberNotice = nullptr;
   QTimer *mMouseMoveTimer = nullptr;
   QTimer *mCheckHandsUpTimer = nullptr;
   bool mbIsEnter = false;
   bool mbOpenSearch = false;
   std::atomic<long long> mUserMemberCount = 0;
   QTimer* mCheckMemberListTimer = nullptr; 
};
#endif