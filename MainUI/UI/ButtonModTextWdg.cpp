#include "ButtonModTextWdg.h"
#include <QPainter>
#include <QFontDatabase> 
#include <QEvent> 
#include <DebugTrace.h>
//#include "pathmanager.h"
#include <QDir>
#include "webinarIdWdg.h"

ButtonModTextWdg::ButtonModTextWdg(QWidget *parent /*= Q_NULLPTR*/)
	:CWidget(parent)
{
	//setupUi(this);
}

ButtonModTextWdg::~ButtonModTextWdg()
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

	if (nullptr != mpFrontLabIcon)
	{
		delete mpFrontLabIcon;
		mpFrontLabIcon = nullptr;
	}

	if (nullptr != mpBackLabIcon)
	{
		delete mpBackLabIcon;
		mpBackLabIcon = nullptr;
	}

	if (nullptr != mpLabFont)
	{
		delete mpLabFont;
		mpLabFont = nullptr;
	}
}

void ButtonModTextWdg::Create(const int& iSpac, const int& iFrontPix, const int& iBackPix,  const int& iTextFontPix)
{
	mpFrontLabIcon = new QLabel(this);
	mpLabFont = new QLabel(this);
	mpBackLabIcon = new QLabel(this);

	miFrontPix = iFrontPix;
	miBackPix = iBackPix;
	miTextFontPix = iTextFontPix;

	mpFrontLabIcon->setObjectName(QStringLiteral("mpLabIcon"));
	mpLabFont->setObjectName(QStringLiteral("mpLabFont"));
	mpBackLabIcon->setObjectName(QStringLiteral("mpLabIcon"));

	mpLayout = new QHBoxLayout(this);
	mpLayout->addWidget(mpFrontLabIcon);
	mpLayout->addWidget(mpLabFont);
	mpLayout->addWidget(mpBackLabIcon);
	this->setLayout(mpLayout);

	mpLayout->setSpacing(iSpac);
	mpLayout->setObjectName(QStringLiteral("horizontalLayout"));
	mpLayout->setContentsMargins(0, 0, 0, 0);

	//QFont font = webinarIdWdg::GetIconfont(miFrontPix);
	//mpFrontLabIcon->setFont(font);
	//QFont font1 = webinarIdWdg::GetIconfont(miBackPix);
	//mpBackLabIcon->setFont(font1);

	mpLabFont->installEventFilter(this);
	mpFrontLabIcon->installEventFilter(this);
	mpBackLabIcon->installEventFilter(this);

	initStyleSheet();
}

void ButtonModTextWdg::SetFrontIconImage(const QString normal, const QString clicked, const QString disable) {
   mNormalSheet = normal;
   mClickedSheet = clicked;
   mDisableSheet = disable;
   initStyleSheet();
}
void ButtonModTextWdg::SetBackIconImage(const QString normal, const QString clicked, const QString disable) {
   mBackIconNormalSheet = normal;
   mBackIconClickedSheet = clicked;
   mBackIconDisableSheet = disable;
   initStyleSheet();
}

void ButtonModTextWdg::SetFrontIconFixedSize(int width, int height) {
   if (mpFrontLabIcon) {
      mpFrontLabIcon->setFixedSize(width,height);
   }
}

void ButtonModTextWdg::SetBackIconFixedSize(int width, int height) {
   if (mpBackLabIcon) {
      mpBackLabIcon->setFixedSize(width, height);
   }
}

void ButtonModTextWdg::setFontText(const QString& strText, Qt::Alignment flag)
{
	if (nullptr != mpLabFont)
	{
		mpLabFont->setText(strText);
		mpLabFont->setAlignment(flag);
	}
}

void ButtonModTextWdg::setIconAlignment(Qt::Alignment flag)
{
	if (nullptr != mpFrontLabIcon){
		TRACE6("%s *********************\n", __FUNCTION__);
		mpFrontLabIcon->setAlignment(flag);
	}
}

void ButtonModTextWdg::setBackIconText(const QChar& ch, Qt::Alignment flag/* = Qt::AlignCenter*/)
{
	if (nullptr != mpBackLabIcon)
	{
		TRACE6("%s *********************\n", __FUNCTION__);
		//mpBackLabIcon->setText(ch);
		//int a = mpBackLabIcon->font().pointSize();
		mpBackLabIcon->setAlignment(flag);
	}
}

void ButtonModTextWdg::setIconSheetStyle(const QString& strNormal, const QString& strHover)
{
	mStrNormal = strNormal;
	mStrHover = strHover;
	initStyleSheet();
}

void ButtonModTextWdg::setBorderColor(const QString& nomalColor, const QString& hoverColor)
{
	//miBorderW = 1;
	mstrBorderNormal = nomalColor;
	mstrBorderHover = hoverColor;
	initStyleSheet();
}

bool ButtonModTextWdg::eventFilter(QObject *o, QEvent *e) {
	if (e->type() == QEvent::Enter) {
		hoverStyleSheet();
		mbEnter = true;
	}
	else if (e->type() == QEvent::Leave) {
		initStyleSheet();
		mbEnter = false;
	}
	else if (e->type() == QEvent::MouseButtonPress) {
		if (mbEnter)
		{
			setFocus();
			emit btnClicked();
		}
	}
	else if (e->type() == QEvent::MouseButtonRelease)
	{
	}

	return QWidget::eventFilter(o, e);
}

void ButtonModTextWdg::initStyleSheet()
{
	QString strIcon = QString("font:%1px;").arg(miTextFontPix);
	mpLabFont->setStyleSheet(strIcon+mStrNormal);
	mpFrontLabIcon->setStyleSheet(mNormalSheet);
	mpBackLabIcon->setStyleSheet(mBackIconNormalSheet);
}

void ButtonModTextWdg::hoverStyleSheet()
{
	QString strIcon = QString("font:%1px;").arg(miTextFontPix);
	mpLabFont->setStyleSheet(strIcon+ mStrHover);
	mpFrontLabIcon->setStyleSheet(mClickedSheet);
	mpBackLabIcon->setStyleSheet(mBackIconClickedSheet);
}