#pragma once

#include <QWidget>
#include <QMouseEvent>
#include "ui_VhallLiveTitle.h"
#include "vhallextrawidgetuserinfo.h"
class  ButtonWdg;
class CRecordDlg;

enum TitleType
{
	eVhallIALiveNone = 0,
	eVhallIALive = 1,
	eVhallIALiveSetting = 2,
	eVhallIALiveGuest = 3,
};

class VhallLiveTitle : public CWidget
{
	Q_OBJECT

public:
    VhallLiveTitle(QWidget *parent = Q_NULLPTR);
    ~VhallLiveTitle();
    QPoint RecordPos();
    void RecordStateSucce();
    void SetRecordState(const int& eState);
    void setRecordBtnShow(const int iDisplay);
    bool IsRecordBtnhide();

    void SetTitleType(TitleType type);
    void SetLiveInfo(const QString& streamID, const QString& roomName, const QString& userName);
    void SetEnableCloseBtn(bool enable);
    void SetHideMaxBtn(bool hide);
    void SetMaxIcon(bool bMax = true);
    void ShowStartLiveBtn(bool show);
    void SetToSpeakEnble(bool bEnble /*= true*/);
    void SetToSpeakBtnState(bool toSpeak);
    bool GetToSpeakBtnState();
    void StartWithActiveLive(int liveTime);
    void ReInit();
    void ClearTimeClock();
    void StartTimeClock(bool bStart);
    long GetLiveStartTime();
    void SetStreamButtonStatus(bool isActive);
    long GetLiveTime();
    void stopHandsUpCountDown();
    void startToSpeakCoutDown();
    //设置活动直播 或 结束状态
    void SetLivingState(bool start);
    void SetPressedPoint(QPoint pos);
signals:
    void sig_ClickedClose();
    void sig_ClickedFresh();
    void sig_ClickedFullSize();
    void sig_ClickedMinSize();
    void sig_ClickedSetting();
    void sig_ClickedShare();
    void sig_OnCopy();
    void Sig_OnClickedStartLiveCtrlBtn();
    void sigToSpeakClicked();
    void sigRecordType(const int& iRecordType);
    void sigFadeOutTip(const QString& strtips);
    void sigCancelClicked();
    void sigMouseMoved();
    void sigApplyTimeout();
private slots:
    void slot_OnClose();
    void slot_OnFresh();
    void slot_OnFullSize();
    void slot_OnMinSize();
    void slot_OnSetting();
    void slot_OnShare();
    void slot_OnCopy();
    void slotEnableRecordBtn(bool enable);
    void slotRecordClicked();
    void slotEnableRecordBtnTimeOut();
    void Slot_OnClickedStartLiveCtrlBtn();
    void slot_OnStartLiveTimeOut();
    void slot_RecordStateChanged(int iOp);
    //void SlotLiveClicked();
    void Slot_HandsUp();
    void slot_CountDown();
protected:
    void mousePressEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);

private:

    Ui::VhallLiveTitle ui;
    private:
    void createRecordDlg();
    QPoint mPressPoint;
    bool mIsMoved = false;
    bool m_bIsEnableMove = true;
    VhallExtraWidgetUserInfo *mUserInfo = NULL;
	
    QPixmap mPixmap;
    ButtonWdg* mpRecordBtnWdg = NULL;
    //bool m_bRecordShow = false;
    bool m_bRecordShow = false;
    QTimer* mpBtnRecordEnableTime = NULL;
    int mRecordState; //录制状态
    QTimer* mpStartLiveBtnTimer = NULL;
    bool mbIsSpeaking = false;
    CRecordDlg* mPRecordDlg = NULL;
    int miNewRecordRs ;

    QTimer*  mpCountDownTimer = nullptr;
    int miCountDownTime = 0;

    bool mbIsLiving = false;
};
