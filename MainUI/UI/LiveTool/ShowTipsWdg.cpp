#include "ShowTipsWdg.h"
#include <QPainter>
#include <QDateTime>

ShowTipsWdg::ShowTipsWdg(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
    setAttribute(Qt::WA_TranslucentBackground, true);
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint | Qt::Dialog);
}

ShowTipsWdg::~ShowTipsWdg()
{
}

void ShowTipsWdg::ResizeWidth(int width) {
    setFixedWidth(width);
    for (int i = 0; i < ui.verticalLayout->count(); i++) {
        QLayoutItem *item = ui.verticalLayout->itemAt(i);
        if (item && item->widget()) {
            ShowTipsItem* tips = dynamic_cast<ShowTipsItem*>(item->widget());
            if (tips) {
                tips->setFixedWidth(width);
            }
        }
    }
}

void ShowTipsWdg::ClearAllTips() {
   while (ui.verticalLayout->count() > 0) {
      QLayoutItem *item = ui.verticalLayout->itemAt(0);
      if (item && item->widget()) {
         ShowTipsItem* tips = dynamic_cast<ShowTipsItem*>(item->widget());
         if (tips) {
            ui.verticalLayout->removeItem(item);
            delete item;
            delete tips;
         }
      }
   }
}

void ShowTipsWdg::ChangeProfileStatus(TipsType type, QString msg) {
    for (int i = 0; i < ui.verticalLayout->count(); i++) {
        QLayoutItem *item = ui.verticalLayout->itemAt(i);
        if (item && item->widget()) {
            ShowTipsItem* tips = dynamic_cast<ShowTipsItem*>(item->widget());
            if (tips->GetTipsType() == type) {
                tips->UpdateChangeProfileState(msg);
                return;
            }
        }
    }
}

void ShowTipsWdg::AddTips(QString &msg, int itemWidth, int showTimeout, TipsType type) {
    for (int i = ui.verticalLayout->count(); i > 0; i--) {
        QLayoutItem *item = ui.verticalLayout->itemAt(i - 1);
        if (item && item->widget()) {
            ShowTipsItem* tips = dynamic_cast<ShowTipsItem*>(item->widget());
            if (tips->GetTipsType() == TipsType_ChangeProfile_Standard || tips->GetTipsType() == TipsType_ChangeProfile_Fluency || 
                tips->GetTipsType() == TipsType_SystemBusy || tips->GetTipsType() == TipsType_OnlyOne_Context || tips->GetTipsType() == TipsType_MIC_NOTICE) {
                //存在切换提示，不重新插入
                if (type == TipsType_ChangeProfile_Standard || type == TipsType_ChangeProfile_Fluency 
                    || type == TipsType_SystemBusy || type == TipsType_OnlyOne_Context || type == TipsType_Bitrate_Down) {
                    return;
                }
                if (type == TipsType_MIC_NOTICE) {
                   tips->SetTipsMsg(msg, showTimeout, type);
                   return;
                }
                else {
                    break;
                }
            }
            else if (ui.verticalLayout->count() >= AddMaxItemCount) {
                ui.verticalLayout->removeItem(item);
                delete item;
                delete tips;
                break;
            }
        }
    }

    QDateTime dataTime;
    qint64 curTime = dataTime.currentMSecsSinceEpoch();
    ShowTipsItem *widget = new ShowTipsItem(curTime,this);
    if (widget) {
        widget->SetTipsMsg(msg, showTimeout, type);
        ui.verticalLayout->addWidget(widget);
        connect(widget, SIGNAL(sig_closeItem(qint64)), this, SLOT(slot_removeItem(qint64)));
        connect(widget, SIGNAL(sig_close()), this, SLOT(slot_clickedClose()));
        connect(widget, SIGNAL(sig_changeProfile(int)), this, SLOT(slot_changeProfile(int)));
        if (ui.verticalLayout->count() == 1) {
            setFixedHeight(ui.verticalLayout->count() * 48);
        }
        else if (ui.verticalLayout->count() == 2) {
            setFixedHeight(ui.verticalLayout->count() * 48 + 6);
        }
        else if (ui.verticalLayout->count() == 3) {
            setFixedHeight(ui.verticalLayout->count() * 48 + 12);
        }
    }
}


void ShowTipsWdg::slot_clickedClose() {
    emit sig_closeItem();
}

void ShowTipsWdg::slot_changeProfile(int type_value) {
    emit sig_changeProfile(type_value);
    if (type_value == TipsType_Bitrate_Down) {
       emit sig_closeItem();
    }
}

void ShowTipsWdg::slot_removeItem(qint64 index) {
    for (int i = 0; i < ui.verticalLayout->count(); i++) {
        QLayoutItem *item = ui.verticalLayout->itemAt(i);
        if (item && item->widget()) {
            ShowTipsItem* tips = dynamic_cast<ShowTipsItem*>(item->widget());
            if (tips->CurrentIndex() == index) {
                ui.verticalLayout->removeItem(item); 
                delete item;
                delete tips;
                break;
            }
        }
    }
    if (ui.verticalLayout->count() == 0) {
        hide();
    }
    if (ui.verticalLayout->count() == 1) {
        setFixedHeight(ui.verticalLayout->count() * 48);
    }
    else if (ui.verticalLayout->count() == 2) {
        setFixedHeight(ui.verticalLayout->count() * 48 + 6);
    }
    else if (ui.verticalLayout->count() == 3) {
        setFixedHeight(ui.verticalLayout->count() * 48 + 12);
    }
    emit sig_removeItem();
}


void ShowTipsWdg::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.fillRect(this->rect(), QColor(0, 0, 0, 0));  //QColor最后一个参数80代表背景的透明度
}