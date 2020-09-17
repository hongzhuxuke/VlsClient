#include "AreaCloseWdg.h"
#include <QPainter>

AreaCloseWdg::AreaCloseWdg(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::Tool);
    connect(ui.pushButton, SIGNAL(clicked()), this, SLOT(slot_closeAreaShare()));
    setAttribute(Qt::WA_TranslucentBackground, true);
}

AreaCloseWdg::~AreaCloseWdg()
{
}

void AreaCloseWdg::SetTextTips(QString content) {
   ui.pushButton->setText(content);
}

void AreaCloseWdg::slot_closeAreaShare() {
    emit sig_clicked();
}

void AreaCloseWdg::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.fillRect(this->rect(), QColor(255, 255, 255, 1));  //QColor最后一个参数80代表背景的透明度
}