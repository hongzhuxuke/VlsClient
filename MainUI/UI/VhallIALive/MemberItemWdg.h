#ifndef H_MEMBERITEMWDG_H
#define H_MEMBERITEMWDG_H


#include "CWidget.h"
#include "ui_OnlineItemWdg.h"
#include "vhalluserinfomanager.h"
#include "IInteractionClient.h"
#include <QObject>

class MenuWdg;
class CAliveDlg;
class QTimer;
class QListWidgetItem;
class CMemberItem : public CWidget
{
	Q_OBJECT
public:
	CMemberItem(QWidget *parent = Q_NULLPTR);
	~CMemberItem();
	void setUpper(QPushButton*, QLabel*);
	void setProhibitSpeaking(QPushButton*, QLabel*);
	void setKickOut(QPushButton*, QLabel*);
	virtual void SetUserInfo(const VhallAudienceUserInfo& info) = 0;

	virtual void SetUpperStates(const bool& bUpper = true);
   virtual void SetSpeakerState(bool mainSpeaker);
   virtual void RecvUserApplySpeak(bool apply);

	bool UpperStates();
   bool IsHandsUp();
	void SetProhibitChatStates(const bool& bProhibit = true);
	bool ProhibitChatStates();
	void SetKickOutStates(const bool& bKickOut = true);
	bool KickOutStates();

	bool isCurrentUser(const QString& strRole);

    bool IsSpeaking();

	virtual QString GetId();
	virtual QString GetName() = 0;
	virtual QString GetRole();
signals:
	void sig_OperationUser(RQData& RqData);
	void sig_btnUpper_clicked(const QString& strId,const bool& bOperation = true);

private slots:
   void slot_CheckHandsUpTimeout();
protected:
	void btnUpperClicked(const bool& checked );
	void btnProhibitSpeakingClicked(const bool& checked);
	void btnbtnKickOutClicked(const bool& checked);

	void  sendOperatRQ(const MsgRQType& type);
	QString mStrId;
	QString mRole;
    int mDevType = 2;   //0其他 1 移动 2 pc
	QPushButton* mpBtnUpper = NULL;
	//QPushButton* mpBtnProhibitSpeaking = NULL;
	//QPushButton* mpBtnKickOut = NULL;

	QLabel* mpLabUpper = NULL;
	QLabel* mpLabProhibitSpeaking = NULL;
	QLabel* mpLabKickOut = NULL;
   bool mbIsSpeaking = false;
   bool mbIsHandsUp = false;
   QTimer *mCheckSpeakTimer = nullptr;

};


/*在线列表成员项*/
class COnlineItemWdg : public CMemberItem
{
	Q_OBJECT

public:
	COnlineItemWdg(QObject* event_reciver, QWidget *parent = Q_NULLPTR);
	COnlineItemWdg(QObject* event_reciver, COnlineItemWdg* pitem,QWidget *parent = Q_NULLPTR);
	~COnlineItemWdg();

   void SetAlive(CAliveDlg* pAlive);
   void ShowMoreBtn(bool show);
   void SetMainView(QString id, bool main);
   void ShowToSpeakBtn(bool show);
	virtual void SetUserInfo(const VhallAudienceUserInfo& info);
	virtual QString GetName();
	void SetUpperStates(const bool& bUpper = true);
	void SetItem(QListWidgetItem* pItem) {
		mpItem = pItem;
	}
   bool IsUserHandsUp();
   void LoadImage(QString file);
	void SetEnableWheat(const bool& bEnable);
	void SetSelfUpperWheat(const bool& bUpper = true);
   void FlushBtnState();
	int GetWeight();
	int CalcWeight();
   void UpdataHostMoreBtn();
   void ShowSetmainSpeaker(bool show);
   void StopCheckUpperTimeout();
   void DownLoadHeadImage(QString image_path);
	bool operator < (const COnlineItemWdg&);
	bool operator <= (const COnlineItemWdg&);
	bool operator > (const COnlineItemWdg&);
	bool operator >= (const COnlineItemWdg&);
	bool operator == (const COnlineItemWdg&) ;

	friend bool operator< (const COnlineItemWdg& obj1, const COnlineItemWdg& obj2)
	{
		return  obj1.mIWeight < obj2.mIWeight;
	}

	enum eOptype
	{
		eOptype_Forbidden = 0,
		eOptype_CancleForbidden,
		eOptype_KickOut,
		eOptype_CancleKickOut,
		eOptype_setMain
	};

	int m_iStaticId;
signals:
	void sig_selfUpper(const bool& bUpper);
   void sig_setMainSpeaker(QString join_uid, QString role);
	void sig_AgreeUpper(const QString& strId);
   void sig_DownLoadAvatar(QString join_id, QString path);
protected:
	void enterEvent(QEvent *event);
	void leaveEvent(QEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
   void customEvent(QEvent* event);
private slots:
	void on_btnSelfUpper_clicked(bool checked = false);
	void on_btnUpper_clicked(bool checked = false);
   void on_pushButton_setMain_clicked(bool checked = false);
	void slot_upperTimeOut();
	void slotCliecked(const int& iOpType);
	void on_btnMoreOp_clicked(bool checked /*= false*/);
private:
	void createAction();

private:

	Ui::OnlineItemWdg ui;
	int mIWeight;
	QTimer* mpUpperTime = NULL;
	QListWidgetItem* mpItem = NULL;
   bool mbEnableSetMainSpeaker = false;
	bool mbHostEnableWheat = false;//启用/禁用 -主持人自己  上/下麦 功能
	bool mbSelfUpper = false;		//自己上下麦操作
   CAliveDlg* mpAlive = NULL;
	MenuWdg* mPMoreOpMenu = nullptr;
   QObject* mEventReciver = nullptr;
};

#endif//H_MEMBERITEMWDG_H