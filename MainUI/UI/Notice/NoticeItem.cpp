#include "NoticeItem.h"
#include "webinarIdWdg.h"

NoticeItem::NoticeItem(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
}

NoticeItem::~NoticeItem()
{
}

int NoticeItem::InsertNoticeMsg(QString msg, const QString& strTime) {
    QString text = QStringLiteral("[公告] %1").arg(msg.simplified());
    ui.textEdit->setText(text);

    QFont font;
    font.setFamily(QStringLiteral("微软雅黑"));
    font.setPointSize(9);
    QFontMetrics fm(font);
    QRect rec = fm.boundingRect(text);
    //字符串所占的像素宽度,高度
    int textWidth = rec.width();
    int textHeight = rec.height();

    int count = textWidth / 278;
    if (count == 0) {
        count = 1;
    }
    else if (textWidth > count * 278) {
        count += 1;
    }
    int newHeight = 55 + count * 22;
    this->setFixedHeight(newHeight);
    ui.textEdit->setFixedHeight(count * 22 + 10);
    ui.label_data->setText(strTime);
    return newHeight;
}
