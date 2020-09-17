#ifndef CRECORDDLG_H
#define CRECORDDLG_H

#include "VHDialog.h"
#include "ui_RecordDlg.h"

class QVBoxLayout;
class ToolButton;
class ButtonWdg;

////开始/结束 按钮
//enum eStartOrStopState
//{
//	eRecordBtnState_Start = 0,	//开始状态
//	eRecordBtnState_Stop          //停止
//};

//当前按钮 暂停/恢复
enum eSuspendOrRecovery
{
	eSuspendOrRecovery_Suspend = 0,  //暂停录制
	eSuspendOrRecovery_Recovery,      //恢复录制
};

enum eCurOp
{
	eCurOp_Stop = 0,		//停止录制
	eCurOp_Suspend,		//暂停录制
	eCurOp_Recovery		//恢复录制
};

class CRecordDlg : public VHDialog {
   Q_OBJECT
public:
   explicit CRecordDlg(QWidget *parent = 0);
   ~CRecordDlg();
   bool Create(const bool& isLive = true);
   void Destroy();
   void SetRecordState(const int& eState);
   
signals:
   void sigClicked(int iOp);
public slots:
	//void show();
	//void hide();
protected:
   //void paintEvent(QPaintEvent *);
   void showEvent(QShowEvent *);
   virtual void focusOutEvent(QFocusEvent *);
   virtual void enterEvent(QEvent *);
   virtual void leaveEvent(QEvent *);
private slots:
	void slotStopRecordClicke();
	void slotSuspendOrRecoveryClicke();
private:
	Ui::RecordDlg ui;
   //QVBoxLayout* m_pMainLayout;
   //QPixmap mPixmap;
   bool m_bEnter = false;

   //ToolButton* mPStopRecordBtn;     //开始/结束  录制
   //ToolButton* mPSuspendOrRecovery; // 暂停/恢复  录制

   //ButtonWdg* mPStopRecordBtn = nullptr;			//开始/结束  录制
   //ButtonWdg* mPSuspendOrRecovery = nullptr;	// 暂停/恢复  录制
   eSuspendOrRecovery mSuspendOrRecovery;		//当前按钮状态
   bool mbLive = true;//true 直播  false 互动
  };

#endif // CRECORDDLG_H
