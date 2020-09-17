#include "MemberBackgroud.h"

MemberBackgroud::MemberBackgroud(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
}

MemberBackgroud::~MemberBackgroud()
{
}

void MemberBackgroud::SetBackGroud(QString image, QString notice) {
   ui.label_limit_image->setStyleSheet(image);
   ui.label_limit_notice->setText(notice);
}