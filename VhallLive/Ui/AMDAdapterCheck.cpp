#include "stdafx.h"
#include "AMDAdapterCheck.h"

AMDAdapterCheck::AMDAdapterCheck(QString name,QWidget *parent)
    : CBaseDlg(parent)
{
    ui.setupUi(this);
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    this->setAutoFillBackground(true);
    connect(ui.pushButton_accept, SIGNAL(clicked()), this, SLOT(accept()));
    QString notice = QStringLiteral("显卡名称:%1\n\n检测到当前显卡驱动版本过低，可能影响\n程序正常使用，建议更新至最新的显卡驱\n动版本之后重试。").arg(name);
    ui.label->setText(notice);
}

AMDAdapterCheck::~AMDAdapterCheck()
{
}
