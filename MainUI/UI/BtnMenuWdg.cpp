#include "BtnMenuWdg.h"
//#include "BtnMenuItem.h"
#include "ButtonWdg.h"

#define MEMBER		QStringLiteral("成    员")
#define CHAT				QStringLiteral("聊    天")
#define INTERACTIVE QStringLiteral("互动工具")

#define NORMALCOLOR  "border-color:rgba(0,0,0,0);background:rgba(0,0,0,0);"
#define HOVERCOLOR  "border-color:rgba(0,0,0,0);background:rgba(0,0,0,178);"

#define Member       "border-image:url(:/button/images/share_member.png);"
#define MemberHover   "border-image:url(:/button/images/share_member_hover.png);"
#define Chat        "border-image:url(:/button/images/share_chat.png);"
#define ChatHover    "border-image:url(:/button/images/share_chat_hover.png);"
#define PluginTool   "border-image:url(:/button/images/share_menu_tool.png);"
#define PluginToolHover   "border-image:url(:/button/images/share_menu_tool_hover.png);"
#define PluginToolDisable   "border-image:url(:/button/images/plugin_tool_disable.png);"


BtnMenuWdg::BtnMenuWdg(QWidget *parent /*= Q_NULLPTR*/)
	:QDialog(parent)
{
	ui.setupUi(this);
	this->setFocusPolicy(Qt::StrongFocus);
	setAttribute(Qt::WA_TranslucentBackground);
	setAutoFillBackground(false);
	this->setWindowFlags(Qt::FramelessWindowHint | Qt::SplashScreen);
}

BtnMenuWdg::~BtnMenuWdg()
{
	QMap<int, BtnMenuItem*>::iterator it = mlistMenuItem.begin();
	for (; it!= mlistMenuItem.end(); it++)
	{
		BtnMenuItem* pTem = it.value();
		if (nullptr != pTem)
		{
			delete pTem;
			pTem = nullptr;
			
		}
	}
	mlistMenuItem.clear();
}

void BtnMenuWdg::SetShowItemCount(const int& iCount)
{
	miCount = iCount;
	QMargins margins = ui.vLayout->contentsMargins();
	setFixedHeight(37* miCount+ margins.top() + margins.bottom());
}

void BtnMenuWdg::show(int x, int y)
{
	move(x, y);
	setFocus(); 	
    QDialog::show();
}

void BtnMenuWdg::addItem(const int& iOpType, const int&  eType)
{
	BtnMenuItem* pItem = nullptr;
	pItem = new BtnMenuItem(this);
	if (nullptr != pItem){
		switch (iOpType)
		{
		case eOptype_Member:
		{
         pItem->Create(BtnMenuItem::eFontDirection_Right, 10, 12, 14, Member, MemberHover, Member);
			pItem->setIconAlignment(Qt::AlignVCenter | Qt::AlignRight);
			pItem->setFontText(MEMBER, 4, Qt::AlignVCenter | Qt::AlignLeft);
         pItem->setIconTextSize(14,15);
		}
		break;
		case eOptype_Chat:
		{
         pItem->Create(BtnMenuItem::eFontDirection_Right, 10, 12, 14, Chat, ChatHover,Chat);
			pItem->setIconAlignment(Qt::AlignVCenter | Qt::AlignRight);
			pItem->setFontText(CHAT, 4, Qt::AlignVCenter | Qt::AlignLeft);
         pItem->setIconTextSize(14, 15);

		}
		break;
		case eOptype_Interactive:
		{
         pItem->Create(BtnMenuItem::eFontDirection_Right, 10, 12, 14, PluginTool, PluginToolHover, PluginToolDisable);
			pItem->setIconAlignment(Qt::AlignVCenter | Qt::AlignRight);
			pItem->setFontText(INTERACTIVE, 4, Qt::AlignVCenter | Qt::AlignLeft);
         pItem->setIconTextSize(14, 15);
		}
		break;
		default:
			break;
		}

      pItem->setFixedSize(94, 37);
      switch (eType)
      {
      case ePlaceType_Top:
         pItem->setBorderColor(NORMALCOLOR + QString("border: 0px solid;border-top-left-radius: 2px;border-top-right-radius: 2px;"),
            HOVERCOLOR + QString("border: 0px solid;border-top-left-radius: 2px;border-top-right-radius: 2px;"), 2);
         break;
      case ePlaceType_Bottom:
         pItem->setBorderColor(NORMALCOLOR + QString("border: 0px solid;border-bottom-left-radius: 2px;border-bottom-right-radius: 2px;"),
            HOVERCOLOR + QString("border: 0px solid;border-bottom-left-radius: 2px;border-bottom-right-radius: 2px;"), 2);
         break;
      default://ePlaceType_Mid
         pItem->setBorderColor(NORMALCOLOR, HOVERCOLOR, 2);
         break;
      }
      //pItem->setBorderColor(NORMALCOLOR, HOVERCOLOR, 2);

      ui.vLayout->addWidget(pItem);
      connect(pItem, &BtnMenuItem::Clicked, this, &BtnMenuWdg::sigClicked);
      connect(pItem, &BtnMenuItem::btnClicked, this, &BtnMenuWdg::close);
      connect(pItem, &BtnMenuItem::sigEnter, this, &BtnMenuWdg::sigEnter);
      mlistMenuItem.insert(iOpType, pItem);
      pItem->SetOpType(iOpType);

	}
}

void BtnMenuWdg::SetItemShow(const int& iOpType, const bool& bShow/* = true*/){
	QMap<int, BtnMenuItem*>::iterator it = mlistMenuItem.find(iOpType);
	if(nullptr != it.value())
		bShow? it.value()->show(): it.value()->hide();
}

//void BtnMenuWdg::SetItemEnabled(const int& iOpType, const bool& bShow/* = true*/)
//{
//	QMap<int, BtnMenuItem*>::iterator it = mlistMenuItem.find(iOpType);
//	if (nullptr != it.value())
//		it.value()->setEnabled(bShow);
//}

void BtnMenuWdg::SetEnable(const int& iOpType, bool enable) {
    QMap<int, BtnMenuItem*>::iterator it = mlistMenuItem.find(iOpType);
    if (nullptr != it.value()) {
        it.value()->SetBtnEnable(enable);
    }
}

void BtnMenuWdg::showEvent(QShowEvent *e) {
	setFocus();
	QDialog::showEvent(e);
}

void BtnMenuWdg::enterEvent(QEvent *e) {
	QDialog::enterEvent(e);
	m_bEnter = true;
}
void BtnMenuWdg::leaveEvent(QEvent *e) {
	QDialog::leaveEvent(e);
	m_bEnter = false;
}

void BtnMenuWdg::focusOutEvent(QFocusEvent *e) {
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
