#ifndef LIVETOOLDLG_H
#define LIVETOOLDLG_H
#include "ui_LiveToolDlg.h"
#include "ui_LiveToolWdg.h"
#include "VHDialog.h"
#include "CWidget.h"
#include "priConst.h"


//    1: �ĵ�
//    2���װ�
//    3���ʾ�
//    4���齱
//    5��ǩ��
//    6������


//�����ĵ�ҳ����ʾ����
enum eDocCtrlType
{
    eDocCtrlType_Doc = 1,  // �ĵ�
    eDocCtrlType_Broad = 2,          // �װ�
    eDocCtrlType_questionnaire = 3,  // �ʾ�    
    eDocCtrlType_Luck = 4,           //  �齱     
    eDocCtrlType_Signin = 5,         //  ǩ��      
    eDocCtrlType_Questions = 6,      //  �ʴ�     
    eDocCtrlType_Hide_Doc = 7,       //    7������ʾ�ĵ����װ�
    eDocCtrlType_Show_Doc = 8,       //    8����ʾ�ĵ����װ�
    eDocCtrlType_DocNormal_Size = 9, //    9���ĵ���С��
    eDocCtrlType_Close_Tool = 10,    //    10���رջ�������
    eDocCtrlType_RedPacket = 11,     //   11�����ͺ��������ʾ
};

enum eDocToolCtrlType
{
    eDocToolCtrlType_CloseTool = 0,  // �ر��ĵ�����
    eDocToolCtrlType_OpenTool = 1,  // ���ĵ�����
};

//����������ʾ���ȼ�
enum eOpType
{
    eOpType_Questions = 1,//  �ʴ�      1
    eOpType_Signin = 2,//  ǩ��      2
    eOpType_Luck = 3,//  �齱     3
    eOpType_questionnaire = 4,// �ʾ�     4
    eOpType_envelopes = 5//  ���      5
};

class BtnMenuItem;
class LiveToolDlg : public VHDialog {
   Q_OBJECT
public:
   LiveToolDlg(QDialog *parent = 0);
   ~LiveToolDlg();

   bool Create();
   void Destroy();

signals:
	void sigClicked(const int& iOpType);
private slots:

protected:
	//void paintEvent(QPaintEvent *);
	void showEvent(QShowEvent *);
	virtual void focusOutEvent(QFocusEvent *);
	virtual void enterEvent(QEvent *);
	virtual void leaveEvent(QEvent *);
private:

	void addLayOut(BtnMenuItem*  pItem, const int& index);
private:

   Ui::LiveToolDlg ui;
   bool m_bEnter = false;
   QList<int> mOpIdList;
   QList<BtnMenuItem*> mListItemBtn;
};


class LiveToolWdg : public CWidget {
	Q_OBJECT

public:
	LiveToolWdg(QWidget *parent = 0);
	~LiveToolWdg();

	bool Create();
	void Destroy();

signals:
	void sigClicked(const int& iOpType);
private slots:

protected:
private:

	void addLayOut(BtnMenuItem*  pItem);
private:

	Ui::LiveToolWdg ui;
	bool m_bEnter = false;
	QList<int> mOpIdList;
	QList<BtnMenuItem*> mListItemBtn;
};
#endif // LIVETOOLDLG_H
