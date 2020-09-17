#include "webinarIdWdg.h"
#include <QFontDatabase> 
#include <DebugTrace.h>
#include <QClipboard>

webinarIdWdg::webinarIdWdg(QWidget *parent /*= Q_NULLPTR*/)
	:CWidget(parent)
{
	ui.setupUi(this);
    connect(ui.pushButton, SIGNAL(clicked()), this, SLOT(slot_ClickedCopy()));

}

webinarIdWdg::~webinarIdWdg()
{

}

void webinarIdWdg::SetId(const QString& id)
{
	ui.labId->setText(id);
}

QString webinarIdWdg::strStreamid()
{
	return ui.labId->text().trimmed();
}

void webinarIdWdg::slot_ClickedCopy() {
    QClipboard *clipborad = QApplication::clipboard();
    if (clipborad){
        QString id = ui.labId->text();
        clipborad->setText(id);
        emit sig_copy();
    }
}
