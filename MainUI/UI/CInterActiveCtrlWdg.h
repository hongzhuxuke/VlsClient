#ifndef CINTERACTIVECTRLWDG_H
#define CINTERACTIVECTRLWDG_H

#include <QWidget>
#include "ui_CInterActiveCtrlWdg.h"

class CInterActiveCtrlWdg : public QWidget {
   Q_OBJECT

public:
    CInterActiveCtrlWdg(QWidget *parent = 0);
    ~CInterActiveCtrlWdg();
    void InitBtnState(bool bHost);
    void InitConnect();
    void SetHideDevList();
    void ShowTop(bool top);

    void ShowToSpeakBtn(bool show);
    void SetToSpeakBtnState(bool toSpeak);
    void OnOpenCamera(bool bOpen);
    void OnOpenMic(bool bOpen);
    void OnOpenPlayer(bool bOpen);
    void OnOpenScreenShare(bool bOpen);
    void ShowStartLiveBtn(bool show);
    void OnStartLive(bool start);
    void OnSetLiveTime(const QString& time);

    void HideTimeCount();

    void SetShowMemberListBtn(bool show);
    void SetShowPluginBtn(bool show);

    //QString GetLiveTime();
    QPoint GetMicBtnGlobalPos();
    QPoint GetPlayBtnGlobalPos();
    QPoint GetMicListBtnGlobalPos();
    QPoint GetPlayListBtnGlobalPos();
    QPoint GetCameraListBtnGlobalPos();
    int GetCameraListBtnWidth();
    int GetMicListBtnWidth();
    int GetPlayListBtnWidth();
    bool GetToSpeakBtnState();
    void SetMicListBtnEnable(bool bEnable);
    void SetFullState(bool isFull);
    void ShowLiveTool(bool);
    int GetRecordState();
    void SetRecordState(const int iState);
    //QPoint GetRecordBtnPos();
    //int GetRecordBtnWidth();
    //void setRecordBtnShow(const int iDisplay);
    //bool  IsRecordBtnhide();
    void SetBackGroundColor(QString styleSheet);

    void ShowMemberListBtn();
    //void SetEixtUnseeHandsUpList(bool bExit);

    //void SetEnableToSpeakBtn(bool enable);

    void ChageSpacerNomal(bool bNomal = false);
    void EnableStartLive(bool enable);
    void SetEnablePlguinsBtn(bool enable);
    void SetToSpeakEnble(bool bEnble = true);
signals:
   void Sig_OnClickedCameraCtrlBtn();
   void Sig_OnClickedMicCtrlBtn();
   void Sig_OnClickedPlayerCtrlBtn();
   void Sig_OnClickedScreenShareCtrlBtn();
   void Sig_OnClickedStartLiveCtrlBtn();
   //void Sig_OnClickedLeaveActiveBtn();
   void Sig_OnClickedShowCameraListBtn();
   void Sig_OnClickedShowMicListBtn();
   void Sig_OnClickedShowPlayerBtn();
   void Sig_OnMicBtnEnter();
   void Sig_OnMicBtnLeave();
   void Sig_OnPlayerBtnEnter();
   void Sig_OnPlayerBtnLeave();
   void Sig_MouseEnterCtrlWdg();
   void SigCurRecordState(int iCurState);
   void SigStopLive();
   void sig_OnPlayFileClicked();
   void sigMemberListClicked();
	void sigToSpeakClicked();
public slots:
	void Slot_LiveToolClicked();
protected:
   virtual void mouseMoveEvent(QMouseEvent *);
   void DisMouseTracking();
   void EnableMouseTracking();   

private slots:
    void slot_OnStartLiveTimeOut();
   void Slot_OnClickedCameraCtrlBtn();
   void Slot_OnClickedMicCtrlBtn();
   void Slot_OnClickedPlayerCtrlBtn();
   void Slot_OnClickedScreenShareCtrlBtn();
   void Slot_OnClickedStartLiveCtrlBtn();
   //void Slot_OnClickedLeaveActiveBtn();
   void Slot_OnClickedShowCameraListBtn();
   void Slot_OnClickedShowMicListBtn();
   void Slot_OnClickedShowPlayerBtn();
   void Slot_OnMicBtnEnter();
   void Slot_OnMicBtnLeave();
   void Slot_OnPlayerBtnEnter();
   void Slot_OnPlayerBtnLeave();
   
   //void slotRecordEnter();
   void slotRecordClicked();
   void slotEnableRecordBtn(bool enable);
   void slotEnableRecordBtnTimeOut();
   void slotPlayFileClicked();
private:
   Ui::CInterActiveCtrlWdg ui;
   bool m_bIsFull = false;
   
   int mRecordState; //录制状态
   QTimer* mpBtnRecordEnableTime = NULL;
   //bool mCurLiveState;//当前直播状态 True 正在直播
   bool mbIsSpeaking = false;
   QTimer* mpStartLiveBtnTimer = NULL;
};

#endif // CINTERACTIVECTRLWDG_H
