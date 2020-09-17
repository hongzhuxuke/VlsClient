#include "MenuWdg.h"
#include "MenuItem.h"
#include <QGraphicsDropShadowEffect> 

MenuWdg::MenuWdg(QWidget *parent /*= Q_NULLPTR*/)
	:QDialog(parent)
{
	ui.setupUi(this);
	//this->setWindowOpacity(0.8);
	//this->setFocusPolicy(Qt::StrongFocus);
	//setAttribute(Qt::WA_TranslucentBackground);
	//setAutoFillBackground(false);
	//this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint | Qt::Tool | Qt::WindowStaysOnTopHint);
	//this->setPalette(Qt::transparent);
   this->setFocusPolicy(Qt::StrongFocus);
   this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint | Qt::Tool | Qt::WindowStaysOnTopHint);
   setAttribute(Qt::WA_TranslucentBackground);
   setAutoFillBackground(true);
	

	//mpEffect = new QGraphicsDropShadowEffect(this);
	//mpEffect->setOffset(4, 6);
	//mpEffect->setColor(QColor(0, 0, 0, 40));
	//mpEffect->setBlurRadius(10);
	//ui.widget->setGraphicsEffect(mpEffect);
}

MenuWdg::~MenuWdg()
{
	if (nullptr != mpEffect)
	{
		delete mpEffect;
		mpEffect = nullptr;
	}
	QMap<int, MenuItem*>::iterator it = mlistMenuItem.begin();
	for (; it!= mlistMenuItem.end(); it++)
	{
		MenuItem* pTem = it.value();
		if (nullptr != pTem)
		{
			delete pTem;
			pTem = nullptr;
			
		}
	}
	mlistMenuItem.clear();
}

void MenuWdg::SetShowItemCount(const int& iCount)
{
	miCount = iCount; 
	QMargins margins = ui.verticalLayout_2->contentsMargins();
	setFixedHeight(30 * miCount + 10);
}

void MenuWdg::Show(int x, int y)
{
	move(x, y);
	setFocus(); 	
   show();
}

void MenuWdg::addItem(const QString& strText,
	const QString& strTextHover,
	const QString& strTextLeave,
	const int& iOpType,
	const QString& strIcoHover,
	const QString& strIcoLeave)
{
	MenuItem* pItem = nullptr; 
	pItem = new MenuItem(this);

	if (nullptr != pItem)
	{
		ui.verticalLayout_2->addWidget(pItem);
		connect(pItem, &MenuItem::sigClicked, this, &MenuWdg::sigClicked);
		connect(pItem, &MenuItem::sigClicked, this, &MenuWdg::close);
		mlistMenuItem.insert(iOpType, pItem);

		pItem->setHoverStyle(strIcoHover, MenuItem::eBtnId_icon);
		pItem->setLeaveStyle(strIcoLeave, MenuItem::eBtnId_icon);

		pItem->setHoverStyle(strTextHover, MenuItem::eBtnId_text);
		pItem->setLeaveStyle(strTextLeave, MenuItem::eBtnId_text);

		pItem->SetText(strText);
		pItem->SetOpType(iOpType);
	}
}

void MenuWdg::SetItemShow(const int& iOpType, const bool& bShow/* = true*/){
	QMap<int, MenuItem*>::iterator it = mlistMenuItem.find(iOpType);
	if(it != mlistMenuItem.end() && nullptr != it.value())
		bShow? it.value()->show(): it.value()->hide();
}

void MenuWdg::showEvent(QShowEvent *e) {
   activateWindow();
	setFocus();
	QDialog::showEvent(e);
}

void MenuWdg::enterEvent(QEvent *e) {
	QDialog::enterEvent(e);
	m_bEnter = true;
}
void MenuWdg::leaveEvent(QEvent *e) {
	QDialog::leaveEvent(e);
	m_bEnter = false;
   this->close();
}

void MenuWdg::focusOutEvent(QFocusEvent *e) {
	if (!m_bEnter) {
		hide();
		this->close();
		return;
	}

	QWidget *focusWidget = this->focusWidget();
	bool focusThis = false;

	while (focusWidget) {
		if (focusWidget == this) {
			focusThis = true;
			break;
		}

		focusWidget = focusWidget->parentWidget();
	}

	if (!focusThis) {
		this->close();
		m_bEnter = false;
	}
	else {
		this->setFocus();
	}
}
