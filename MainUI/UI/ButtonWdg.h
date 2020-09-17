#ifndef H_BUTTONWDG_H
#define H_BUTTONWDG_H

#include "CWidget.h"
//#include "ui_ButtonWdg.h"
#include <QVBoxLayout> 
#include <QLabel> 
class  CIconFontLabel : public QLabel
{
public:

   CIconFontLabel(QWidget *parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());
   CIconFontLabel(const QString &text, QWidget *parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());
   ~CIconFontLabel();
   void SetPaintPoint(const bool& bPaint = true);
protected:
   void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
private:
   bool mbPoint = false;
};

class ButtonWdg : public CWidget
{
   Q_OBJECT
public:
   ButtonWdg(QWidget *parent = Q_NULLPTR);
   ~ButtonWdg();

   enum eFontDirection
   {
      eFontDirection_Up = 0,
      eFontDirection_Right = 1,
      eFontDirection_Underside = 2,
      eFontDirection_Left = 3,

   };

   void Create(eFontDirection efont, const int& iSpac/* = 0*/, const int&iTextFontPix, const int& iIconFontPix /*= 20*/, const QString normal = QString(), const QString clicked = QString(), const QString disable = QString());
   void SetBorderWidth(const int& iW) { miBorderW = iW; }
   virtual void setFontText(const QString& strText, Qt::Alignment flag = Qt::AlignCenter);
   void setIconAlignment(Qt::Alignment flag = Qt::AlignCenter);
   void setIconSheet(const QString& strNormal, const QString& strHover, const QString& disable = QString()) ;
   void setIconTextSize(const int& iW, const int& iH);
   void setIconSheetStyle(const QString& strNormal, const QString& strHover, const QString& borderColor = QString());
   void setBorderColor(const QString& nomalColor, const QString& hoverColor, const int& iBorder = 1);
   void setRaduis(const bool& bRaduis = false);
   void SetBtnEnable(bool enable);
   void SetClicked(bool click);
   void SetPaintPoint(const bool& bPaint = true);
   void SetHoverState(bool hover);
   QString GetRecordState();
   void SetIconFixedHeight(int height);
signals:
   void btnClicked();

protected:
   bool eventFilter(QObject *, QEvent *);

protected:
   void initStyleSheet();
   void hoverStyleSheet();

   QBoxLayout *mpLayout = nullptr;
   //QHBoxLayout *mpHLayout = nullptr;
   CIconFontLabel *mpLabIcon = nullptr;
   QLabel *mpLabFont = nullptr;
   QLabel *mpLabSpace = nullptr;
   bool mbEnter = false;
   int miBorderW = 0;

   QString mStrNormal = "color: #ECECEC;"; //background-color:#ff0000;
   QString mStrHover = "color: #FC5659;";//background-color:#00ff00;

    //QString mStrNormal = "background-color:#ff0000";
    //QString mStrHover = "background-color:#00ff00";

   QString mstrBorderNormal = "border-color:#ECECEC;";//ECECEC
   QString mstrBorderHover = "border-color:#FC5659;";
   QString mstrDisAbleColor = "color:#7c7c7c;";

   int miConFontPix = 20;
   int miTextFontPix = 12;
   bool mBRaduis = false;
   eFontDirection miDirection = eFontDirection_Up;
   bool mbEnable = true;

   bool mbClicked = false;

   QString mNormalSheet;
   QString mClickedSheet;
   QString mDisableSheet;
};

class BtnMenuItem :public ButtonWdg
{
   Q_OBJECT
public:
   BtnMenuItem(QWidget *parent = Q_NULLPTR);
   ~BtnMenuItem();
   void SetOpType(const int&  iOpType);
   virtual void setFontText(const QString& strText, const int& textLenth, Qt::Alignment flag = Qt::AlignCenter);
signals:
   void Clicked(const int& iType);
   void sigEnter(const int& iType, const int& iX, const int& iy);
   //void sigLeave(const int& iType);
protected:
   bool eventFilter(QObject *, QEvent *);

private slots:
   void slotClicked();
private:
   int miOpType;
};
#endif//H_BUTTONWDG_H