#ifndef H_BUTTONMODTEXTWDG_H
#define H_BUTTONMODTEXTWDG_H

#include "CWidget.h"
//#include "ui_ButtonWdg.h"
#include <QVBoxLayout> 
#include <QLabel> 

class ButtonModTextWdg : public CWidget
{
	Q_OBJECT
public:
	ButtonModTextWdg(QWidget *parent = Q_NULLPTR);
	~ButtonModTextWdg();

	//enum eFontDirection
	//{
	//	eFontDirection_Up = 0,
	//	eFontDirection_Right = 1,
	//	eFontDirection_Underside = 2,
	//	eFontDirection_Left = 3,
	//	
	//};

	void Create(const int& iSpac/* = 0*/, const int& iFrontPix ,const int& iBackPix, const int& iTextFontPix/*= 20*/);
   void SetFrontIconImage(const QString normal, const QString clicked, const QString disable);
   void SetBackIconImage(const QString normal, const QString clicked, const QString disable);
   void SetFrontIconFixedSize(int width,int height);
   void SetBackIconFixedSize(int width, int height);
	void setFontText(const QString& strText, Qt::Alignment flag = Qt::AlignCenter);
	void setIconAlignment(Qt::Alignment flag = Qt::AlignCenter);
	void setBackIconText(const QChar& ch, Qt::Alignment flag = Qt::AlignCenter);

	void setIconSheetStyle(const QString& strNormal, const QString& strHover);
	void setBorderColor(const QString& nomalColor, const QString& hoverColor);
signals:
	void btnClicked();

protected:
	bool eventFilter(QObject *, QEvent *);

private:
	void initStyleSheet();
	void hoverStyleSheet();

	QBoxLayout *mpLayout = nullptr;
	//QHBoxLayout *mpHLayout = nullptr;
	QLabel *mpFrontLabIcon = nullptr;
	QLabel *mpLabFont = nullptr;
	QLabel *mpBackLabIcon = nullptr;

	bool mbEnter = false;
	//int miBorderW = 0;
	QString mStrNormal = "color:#ECECEC;";
	QString mStrHover = "color:#FC5659;";
	QString mstrBorderNormal = "#ECECEC;";
	QString mstrBorderHover = "#FC5659;";
	int miFrontPix = 20;
	int miBackPix = 12;
	int miTextFontPix;


   QString mNormalSheet;
   QString mClickedSheet;
   QString mDisableSheet;

   QString mBackIconNormalSheet;
   QString mBackIconClickedSheet;
   QString mBackIconDisableSheet;
};

#endif//H_BUTTONMODTEXTWDG_H