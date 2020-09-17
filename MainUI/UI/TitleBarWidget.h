#ifndef TITLEBARWIDGET_H
#define TITLEBARWIDGET_H

#include <CWidget.h>
#include <CLabel.h>

class QLabel;
class TitleButton;
class ToolButton;
//class MainWidget;
class PlayerUI;
class QHBoxLayout;
class QVBoxLayout;
class CTimeButton;
class QPushButton;
class webinarIdWdg;
class ButtonWdg;

class TitleBarWidget : public CWidget {
   Q_OBJECT
public:
   explicit TitleBarWidget(QWidget *parent = 0);
   ~TitleBarWidget();

public:
	QPoint RecordPos();
	int iStartLiveX();
	int iStartLiveWidth();
	int iStartLiveHeight();
	void SetStreamButtonStatus(bool isActive);
	void SetRecordState(const int& eState);
	bool isStreamStatusActive();

	void setRecordBtnShow(const int iDisplay);
	int GetRecordState();
	bool IsRecordBtnhide();

   void SetControlRightWidget(bool);
   bool Create(const bool& bLivingWdg);
   void Destroy();
   void AddImageWidget(QWidget *);
   void RemoveImageWidget(QWidget *);
   static bool MouseIsPressed();
   void HideLogo(bool);
   void HideSetting();
   void ShowSharedButton();
   void SetParentWidget(QWidget *w){ m_parentWidget = w;}
   void ShowRefreshButton(const bool& bShow = true);
   void ShowWebinarName(bool show);
   void SetWebinarName(QString name,QString id);
   void ClearWebinarName();
   void StartWithActiveLive(int liveTime);
   void ClearTimeClock();
   void ReInit();
   void StartTimeClock(bool bStart);
   long GetLiveStartTime();
   void SetMaxIcon(bool bMax = true);
   void SetPressedPoint(QPoint pos);
signals:   
   void sigMousePressed();
   //void sigSharedBtnClicked();
   void sigToClose();
   void sigRefresh();
   void sigClicked();
   void sigMaxClicked();
   void sigCopy();
   void sigMin();
   void sigMouseMoved();
public slots:
   void setMoveEnabled(bool);
   void onMenuClick();
   void onMinClick();
   void slotMaxClick();
   void onCloseClick();

private slots:
	void SlotLiveClicked();
	void slotEnableRecordBtn(bool enable);
	void slotEnableRecordBtnTimeOut();
	void slotRecordClicked();
    void slot_OnStartLiveTimeOut();
	//void slotRecordEnter();
    void Slot_CopyID();
protected:
   void mousePressEvent(QMouseEvent *);
   void mouseMoveEvent(QMouseEvent *);
   void mouseReleaseEvent(QMouseEvent *);

private:
   QPoint m_qPressPoint;
   bool m_bIsMoveEnabled;
   static bool m_bIsPressed;

   QLabel *m_pVersionTitle = NULL;   //标题
   CLabel *m_pWebinarName = NULL; //活动名称（组成：活动名称+ID(123123)）
   webinarIdWdg* mpWebinarIdWdg = NULL;

   TitleButton *m_pBtnSetting = NULL; //设置   
   TitleButton *m_pBtnRefresh = NULL;  //刷新      
   TitleButton *m_pBtnMin = NULL;			//最小化 
   TitleButton* mpBtnMax = NULL;			//最大化
   TitleButton *m_pBtnClose = NULL;		//关闭

   //TitleButton *m_pBtnShared = NULL;//分享

   QHBoxLayout *m_pTitleLayout = NULL;
   QHBoxLayout *m_pImageLayout = NULL;
   bool m_bControlRightWidget = false;
   QWidget *m_parentWidget = NULL;
   
   //来源 toolBar
   CTimeButton* m_pStartLiveBtn = NULL;					//开始直播
   //ToolButton *mPRecordBtn = NULL;	//录制
   ButtonWdg* mPRecordBtnWdg = NULL;
   int mRecordState; //录制状态
   QTimer* mpBtnRecordEnableTime = NULL;
   QTimer* mpStartLiveBtnTimer = NULL;

   bool m_bRecordShow = false;
   
   //CTimeButton *TimeButton;
};

#endif // TITLEBARWIDGET_H
