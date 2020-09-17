#include "StateWdg.h"
#include "webinarIdWdg.h"

StateWdg::StateWdg(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
	ui.labIcone->setObjectName(QStringLiteral("mpLabIcon"));
	QFont font = webinarIdWdg::GetIconfont(12);
	ui.labIcone->setFont(font);
	ui.labTitle->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
	ui.labState->setAlignment(Qt::AlignVCenter| Qt::AlignLeft);

	QFont Textfont;
	Textfont.setFamily(QString::fromUtf8("\345\276\256\350\275\257\351\233\205\351\273\221"));//Î¢ÈíÑÅºÚ
	Textfont.setPixelSize(12);

	ui.labTitle->setFont(Textfont);
	ui.labState->setFont(Textfont);
}

StateWdg::~StateWdg()
{
}

void StateWdg::setExceedHigh(const bool& bEx /*= false*/)
{
	bEx ? ui.labIcone->show() : ui.labIcone->hide();
}

void StateWdg::setTitleText(const QString& strText)
{
	ui.labTitle->setText(strText.trimmed());
}

void StateWdg::setText(const QString& strText)
{
	ui.labState->setText(strText.trimmed());
}

void StateWdg::setTextWidth(const int& lw, const int& rw)
{
	ui.labTitle->setFixedWidth(lw);
}

void StateWdg::setIcon(const QChar ch)
{
	ui.labIcone->setStyleSheet("color: #FC5659;");
	ui.labIcone->setText(ch);
}