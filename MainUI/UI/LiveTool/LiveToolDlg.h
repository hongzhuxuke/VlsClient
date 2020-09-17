#ifndef LIVETOOLDLG_H
#define LIVETOOLDLG_H
#include "ui_LiveToolDlg.h"
#include "ui_LiveToolWdg.h"
#include "VHDialog.h"
#include "CWidget.h"
#include "priConst.h"


//    1: 文档
//    2：白板
//    3：问卷
//    4：抽奖
//    5：签到
//    6：答题


//控制文档页面显示内容
enum eDocCtrlType
{
    eDocCtrlType_Doc = 1,  // 文档
    eDocCtrlType_Broad = 2,          // 白板
    eDocCtrlType_questionnaire = 3,  // 问卷    
    eDocCtrlType_Luck = 4,           //  抽奖     
    eDocCtrlType_Signin = 5,         //  签到      
    eDocCtrlType_Questions = 6,      //  问答     
    eDocCtrlType_Hide_Doc = 7,       //    7：不显示文档、白板
    eDocCtrlType_Show_Doc = 8,       //    8：显示文档、白板
    eDocCtrlType_DocNormal_Size = 9, //    9：文档最小化
    eDocCtrlType_Close_Tool = 10,    //    10：关闭互动工具
    eDocCtrlType_RedPacket = 11,     //   11：发送红包工具显示
};

enum eDocToolCtrlType
{
    eDocToolCtrlType_CloseTool = 0,  // 关闭文档工具
    eDocToolCtrlType_OpenTool = 1,  // 打开文档工具
};

//互动工具显示优先级
enum eOpType
{
    eOpType_Questions = 1,//  问答      1
    eOpType_Signin = 2,//  签到      2
    eOpType_Luck = 3,//  抽奖     3
    eOpType_questionnaire = 4,// 问卷     4
    eOpType_envelopes = 5//  红包      5
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
