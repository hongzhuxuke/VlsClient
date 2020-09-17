#ifndef CRECORDDLG_H
#define CRECORDDLG_H

#include "VHDialog.h"
#include "ui_RecordDlg.h"

class QVBoxLayout;
class ToolButton;
class ButtonWdg;

////��ʼ/���� ��ť
//enum eStartOrStopState
//{
//	eRecordBtnState_Start = 0,	//��ʼ״̬
//	eRecordBtnState_Stop          //ֹͣ
//};

//��ǰ��ť ��ͣ/�ָ�
enum eSuspendOrRecovery
{
	eSuspendOrRecovery_Suspend = 0,  //��ͣ¼��
	eSuspendOrRecovery_Recovery,      //�ָ�¼��
};

enum eCurOp
{
	eCurOp_Stop = 0,		//ֹͣ¼��
	eCurOp_Suspend,		//��ͣ¼��
	eCurOp_Recovery		//�ָ�¼��
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

   //ToolButton* mPStopRecordBtn;     //��ʼ/����  ¼��
   //ToolButton* mPSuspendOrRecovery; // ��ͣ/�ָ�  ¼��

   //ButtonWdg* mPStopRecordBtn = nullptr;			//��ʼ/����  ¼��
   //ButtonWdg* mPSuspendOrRecovery = nullptr;	// ��ͣ/�ָ�  ¼��
   eSuspendOrRecovery mSuspendOrRecovery;		//��ǰ��ť״̬
   bool mbLive = true;//true ֱ��  false ����
  };

#endif // CRECORDDLG_H
