#include "ButtonWdg.h"
#include <QPainter>
#include <QFontDatabase> 
#include <QEvent> 
#include <DebugTrace.h>
#include <QDebug>
//#include "pathmanager.h"
#include <QDir>
#include "webinarIdWdg.h"

CIconFontLabel::CIconFontLabel(QWidget *parent/* = Q_NULLPTR*/, Qt::WindowFlags f/* = Qt::WindowFlags()*/)
   :QLabel(parent, f)
{

}

CIconFontLabel::CIconFontLabel(const QString &text, QWidget *parent /*= Q_NULLPTR*/, Qt::WindowFlags f /*= Qt::WindowFlags()*/)
   : QLabel(text, parent, f)
{

}

CIconFontLabel::~CIconFontLabel()
{

}

void CIconFontLabel::SetPaintPoint(const bool& bPaint /*= true*/)
{
   mbPoint = bPaint;
   repaint();
}

void CIconFontLabel::paintEvent(QPaintEvent *event)
{
   Q_UNUSED(event)
      QString strText = this->text();
   QPainter painter(this);
   int iW = this->width();
   int iH = this->height();

   //painter.drawRect(0, 0, iW, iH);

   painter.drawText(0, 0, iW, iH, alignment(), strText);

   if (mbPoint)
   {
      painter.setPen(QPen(QColor(252, 86, 89), 4, Qt::DashLine));//
      painter.setBrush(QBrush(Qt::red, Qt::SolidPattern));//
      //painter.setBrush(Qt::SolidPattern);
      painter.drawEllipse(0.65*iW, 0.3*iH, 2, 2);//画圆 
      //painter.drawPoint(0.6*iW, 0.3*iH);
   }
   QLabel::paintEvent(event);
}


ButtonWdg::ButtonWdg(QWidget *parent /*= Q_NULLPTR*/)
   :CWidget(parent)
{
   //setupUi(this);
}

ButtonWdg::~ButtonWdg()
{
   if (nullptr != mpLayout)
   {
      delete mpLayout;
      mpLayout = nullptr;
   }

   if (nullptr != mpLayout)
   {
      delete mpLayout;
      mpLayout = nullptr;
   }

   if (nullptr != mpLabIcon)
   {
      delete mpLabIcon;
      mpLabIcon = nullptr;
   }

   if (nullptr != mpLabFont)
   {
      delete mpLabFont;
      mpLabFont = nullptr;
   }
}

void ButtonWdg::Create(eFontDirection efont, const int& iSpac, const int&iTextFontPix, const int& iIconFontPix, const QString normal, const QString clicked, const QString disable)
{
   mpLabIcon = new CIconFontLabel(this);
   mpLabFont = new QLabel(this);
   mpLabSpace = new QLabel(this);

   mNormalSheet = normal;
   mClickedSheet = clicked;
   mDisableSheet = disable;

   miConFontPix = iIconFontPix;
   miTextFontPix = iTextFontPix;

   mpLabIcon->setObjectName(QStringLiteral("mpLabIcon"));
   mpLabFont->setObjectName(QStringLiteral("mpLabFont"));
   this->setObjectName("btnwdg");

   miDirection = efont;
   switch (efont)
   {
   case ButtonWdg::eFontDirection_Up: {
      mpLayout = new QVBoxLayout(this);
      mpLabSpace->setFixedHeight(iSpac);
      mpLabSpace->setText("");
      mpLayout->addWidget(mpLabFont);
      mpLayout->addWidget(mpLabSpace);
      mpLayout->addWidget(mpLabIcon);
      this->setLayout(mpLayout);
      break;
   }
   case ButtonWdg::eFontDirection_Right: {
      mpLayout = new QHBoxLayout(this);
      QSpacerItem* horizontalSpacer = new QSpacerItem(16, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
      mpLayout->addItem(horizontalSpacer);
      mpLayout->addWidget(mpLabIcon);
      mpLayout->addWidget(mpLabSpace);
      mpLayout->addWidget(mpLabFont);
      QSpacerItem* horizontalSpacer_2 = new QSpacerItem(16, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
      mpLayout->addItem(horizontalSpacer_2);
      this->setLayout(mpLayout);
      mpLabSpace->setFixedWidth(iSpac);
      mpLabSpace->setText("");
      break;
   }
   case ButtonWdg::eFontDirection_Underside: {
      mpLabSpace->setFixedHeight(iSpac);
      mpLabSpace->setText("");
      mpLayout = new QVBoxLayout(this);
      QWidget* widget = new QWidget(this);
      widget->setObjectName(QStringLiteral("widget"));
      QHBoxLayout* horizontalLayout = new QHBoxLayout(widget);
      horizontalLayout->setSpacing(0);
      horizontalLayout->setContentsMargins(11, 11, 11, 11);
      horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
      horizontalLayout->setContentsMargins(0, 0, 0, 0);
      QSpacerItem* horizontalSpacer = new QSpacerItem(16, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
      horizontalLayout->addItem(horizontalSpacer);
      horizontalLayout->addWidget(mpLabIcon);
      QSpacerItem* horizontalSpacer_2 = new QSpacerItem(16, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
      horizontalLayout->addItem(horizontalSpacer_2);

      mpLayout->addWidget(widget);
      mpLayout->addWidget(mpLabSpace);
      mpLayout->addWidget(mpLabFont);
      this->setLayout(mpLayout);
      break;
   }
   case ButtonWdg::eFontDirection_Left:
      mpLayout = new QHBoxLayout(this);
      mpLayout->addWidget(mpLabFont);
      mpLayout->addWidget(mpLabSpace);
      mpLayout->addWidget(mpLabIcon);
      this->setLayout(mpLayout);
      mpLabSpace->setFixedWidth(iSpac);
      mpLabSpace->setText("");
      break;
   default:
      break;
   }

   mpLayout->setSpacing(0);
   mpLayout->setObjectName(QStringLiteral("horizontalLayout"));
   mpLayout->setContentsMargins(0, 0, 0, 0);

   //QFont font = webinarIdWdg::GetIconfont(miConFontPix);
   //mpLabIcon->setFont(font);

   QFont Textfont;
   Textfont.setFamily(QString::fromUtf8("\345\276\256\350\275\257\351\233\205\351\273\221"));//微软雅黑
   //Textfont.setPointSize(miTextFontPix);
   Textfont.setPixelSize(miTextFontPix);
   mpLabFont->setFont(Textfont);

   initStyleSheet();
   mpLabFont->installEventFilter(this);
   mpLabIcon->installEventFilter(this);
   mpLabSpace->installEventFilter(this);
}

void ButtonWdg::SetIconFixedHeight(int height) {
   //if (mpLabIcon) {
   //   mpLabIcon->setFixedHeight(height);
   //}
}

QString  ButtonWdg::GetRecordState() {
   if (mpLabFont) {
      return mpLabFont->text();
   }
   return QString();
}

void ButtonWdg::setFontText(const QString& strText, Qt::Alignment flag)
{
   if (nullptr != mpLabFont)
   {
      mpLabFont->setText(strText);
      mpLabFont->setAlignment(flag);
   }

   ////create后  设置本控件尺寸  再计算文字位置
   if (ButtonWdg::eFontDirection_Right == miDirection && miBorderW > 0)
   {
      int width = this->width();
      int FontP = mpLabFont->font().pixelSize();
      int strLen = strText.trimmed().length();
      if (FontP > 0)
      {
         int iTextW = FontP * strLen/*mpLabFont->fontMetrics().boundingRect(strText.trimmed()).width()*/;
         int iSpacing = mpLayout->spacing();
         int iIconWidth = 0.5*width - 0.5*iTextW + 0.5*miConFontPix - 0.5*iSpacing;
         //mpLabIcon->setFixedWidth(iIconWidth);
         initStyleSheet();
      }

   }

}

void ButtonWdg::setIconAlignment(Qt::Alignment flag)
{
   if (nullptr != mpLabIcon) {
      mpLabIcon->setAlignment(flag);
   }
}

void ButtonWdg::setIconTextSize(const int& iW, const int& iH){
   if (mpLabIcon) {
      mpLabIcon->setFixedSize(iW, iH);
   }
}

void ButtonWdg::setIconSheet(const QString& strNormal, const QString& strHover, const QString& disable) {
   mNormalSheet = strNormal;
   mClickedSheet = strHover;
   mDisableSheet = disable;
   initStyleSheet();
}

void ButtonWdg::setIconSheetStyle(const QString& strNormal, const QString& strHover, const QString& borderColor/* = QString()*/)
{
   mStrNormal = strNormal;
   mStrHover = strHover;
   if (!borderColor.isEmpty()) {
      mstrBorderNormal = borderColor;
   }
}

void ButtonWdg::setBorderColor(const QString& nomalColor, const QString& hoverColor, const int& iBorder)
{
   miBorderW = iBorder;
   mstrBorderNormal = nomalColor;
   mstrBorderHover = hoverColor;
}

void ButtonWdg::setRaduis(const bool& bRaduis/* = false*/)
{
   mBRaduis = bRaduis;
}

void ButtonWdg::SetBtnEnable(bool enable) {
   this->setEnabled(enable);
   mbEnable = enable;
   if (mbEnable) {
      if (mbClicked) {
         hoverStyleSheet();
      }
      else {
         initStyleSheet();
      }
   }
   else {
      initStyleSheet();
   }

}

void ButtonWdg::SetClicked(bool click) {
   mbClicked = click;
   if (!click) {
      initStyleSheet();
   }
}

void ButtonWdg::SetPaintPoint(const bool& bPaint /*= true*/)
{
   if (nullptr != mpLabIcon){
      mpLabIcon->SetPaintPoint(bPaint);
   }
}

void ButtonWdg::SetHoverState(bool hover) {
   if (hover) {
      hoverStyleSheet();
   }
   else {
      initStyleSheet();
   }
   repaint();
}

bool ButtonWdg::eventFilter(QObject *o, QEvent *e) {
   if (e->type() == QEvent::Enter) {
      hoverStyleSheet();
      mbEnter = true;
   }
   else if (e->type() == QEvent::Leave) {
      if (mbClicked) {
         hoverStyleSheet();
      }
      else {
         initStyleSheet();
      }
      mbEnter = false;
   }
   else if (e->type() == QEvent::MouseButtonPress || e->type() == QEvent::MouseButtonDblClick) {
      qDebug() << " mbEnter " << mbEnter;
      if (mbEnter && mbEnable)
      {
         setFocus();
         emit btnClicked();
      }
   }
   return QWidget::eventFilter(o, e);
}

void ButtonWdg::initStyleSheet()
{
   QString strIcon = QString("font:%1px;").arg(miTextFontPix);
   if (mbEnable) {
      mpLabFont->setStyleSheet(strIcon + mStrNormal);
      //mpLabIcon->setStyleSheet(mStrNormal);
      strIcon = QString("font:%1px;").arg(miConFontPix);
      if (mpLabIcon) {
         mpLabIcon->setStyleSheet(mNormalSheet);
      }
   }
   else {
      mpLabFont->setStyleSheet(strIcon + mstrDisAbleColor);
      //mpLabIcon->setStyleSheet(mStrNormal);
      strIcon = QString("font:%1px;").arg(miConFontPix);
      if (mpLabIcon) {
         mpLabIcon->setStyleSheet(mDisableSheet);
      }
   }

   if (miBorderW > 0){
      QString str = this->objectName();
      QString strQSS = "";
      int iHeight;
      mBRaduis ? iHeight = 0.5* this->height() : iHeight = 0;
      if (mbEnable) {
         strQSS += QString("ButtonWdg{border: %1px solid;  border-radius: %3px;%2}").arg(miBorderW).arg(mstrBorderNormal).arg(iHeight);
      }
      else {
         strQSS += QString("ButtonWdg{border: %1px solid;  border-radius: %3px;background-color: rgb(67, 67, 67);%2}").arg(miBorderW).arg(mstrBorderNormal).arg(iHeight);
      }
      this->setStyleSheet(strQSS);
   }

}

void ButtonWdg::hoverStyleSheet()
{
   QString strIcon = QString("font:%1px;").arg(miTextFontPix);
   if (mbEnable) {
      mpLabFont->setStyleSheet(strIcon + mStrHover);
      strIcon = QString("font:%1px;").arg(miConFontPix);
      if (mpLabIcon) {
         mpLabIcon->setStyleSheet(mClickedSheet);
      }
   }
   else {
      mpLabFont->setStyleSheet(strIcon + mstrDisAbleColor);
      strIcon = QString("font:%1px;").arg(miConFontPix);
      if (mpLabIcon) {
         mpLabIcon->setStyleSheet(mDisableSheet);
      }
   }

   if (miBorderW > 0)
   {
      QString strQSS = "";
      int iHeight;
      mBRaduis ? iHeight = 0.5* this->height() : iHeight = 0;
      if (mbEnable)
         strQSS += QString("ButtonWdg{border: %1px solid;  border-radius: %3px;%2 }").arg(miBorderW).arg(mstrBorderHover).arg(iHeight);
      else
         return;// strQSS += QString("#btnwdg{border: %1px solid; %2 border-radius: %3px; };").arg(miBorderW).arg(mstrBorderNormal).arg(iHeight);
      this->setStyleSheet(strQSS);
   }
}


////////////////////////////////////////////
////////////////////////////////////////////
////////////////////////////////////////////
BtnMenuItem::BtnMenuItem(QWidget *parent /*= Q_NULLPTR*/)
   :ButtonWdg(parent)
{
   setAttribute(Qt::WA_TranslucentBackground);
   setAutoFillBackground(false);

   connect(this, &BtnMenuItem::btnClicked, this, &BtnMenuItem::slotClicked);
}

BtnMenuItem::~BtnMenuItem()
{

}

void BtnMenuItem::SetOpType(const int&  iOpType){
   miOpType = iOpType;
}

bool BtnMenuItem::eventFilter(QObject *o, QEvent * e){
   if (e->type() == QEvent::Enter) {

      QPoint tempPos = this->pos();
      QPoint posTemp = mapToGlobal(tempPos);

      int iX = posTemp.x() + this->width();
      int iY = posTemp.y();

      emit sigEnter(miOpType, iX, iY);
   }
   else if (e->type() == QEvent::Leave) {
      //emit sigLeave(miOpType);
   }
   return ButtonWdg::eventFilter(o, e);
}

void BtnMenuItem::slotClicked(){
   emit Clicked(miOpType);
}

void BtnMenuItem::setFontText(const QString& strText, const int& strLen, Qt::Alignment flag)
{
   if (nullptr != mpLabFont){
      mpLabFont->setText(strText);
      mpLabFont->setAlignment(flag);
   }

   ////create后  设置本控件尺寸  再计算文字位置
   if (ButtonWdg::eFontDirection_Right == miDirection && miBorderW > 0){
      int width = this->width();
      int FontP = mpLabFont->font().pixelSize();
      //int strLen = strText.trimmed().length();
      if (FontP > 0){
         int iTextW = FontP * strLen/*mpLabFont->fontMetrics().boundingRect(strText.trimmed()).width()*/;
         int iSpacing = mpLayout->spacing() + mpLabSpace->width();
         int iIconWidth = 0.5*width - 0.5*iTextW + 0.5*miConFontPix - 0.5*iSpacing;
         if (mpLabIcon) {
            mpLabIcon->setFixedWidth(iIconWidth);
         }
         initStyleSheet();
      }
   }
}
