#include "vhallaudiencewidget.h"
#include "ui_vhallaudiencewidget.h"
#include <QDebug>
#include <QPainter>

#include <QKeyEvent>

#include "Msg_VhallRightExtraWidget.h"
#include "VhallRightExtraWidgetIns.h"

#define BUTTON_UP_STYLE    "\
                              QToolButton , QToolButton:pressed, QToolButton:hover{ \
                              background-color:rgb(26,26,26);\
                              font-size : 12px;\
                              font-family : Î¢ÈíÑÅºÚ; \
                              color :  rgb(200,200,200);} \
                              "
#define BUTTON_DOWN_STYLE    "\
                                 QToolButton , QToolButton:pressed, QToolButton:hover{ \
                                 background-color:rgb(50,50,50);\
                                 font-size : 12px;\
                                 font-family : Î¢ÈíÑÅºÚ; \
                                 color : rgb(240,240,240);} \
                                 "

VhallAudienceWidget::VhallAudienceWidget(QWidget *parent) :
QWidget(parent),
ui(new Ui::VhallAudienceWidget),
m_pSelfInfo(NULL) {
   ui->setupUi(this);
   BtnClicked(ui->btn_online);
   SetOnlineNumber(0);
   SetGagNumber(0);
   SetKickNumber(0);
   ui->widgetOnline->SetMonitorVerticalBar(true);
   ui->widgetShot->SetGagHide(true);
   ui->widgetGag->SetShotHide(true);
   this->installEventFilter(this);
   mbHost = true;

   ui->widgetOnline->SetPageListType(ePageType_OnLineUser);
   ui->widgetShot->SetPageListType(ePageType_KickOut);
   ui->widgetGag->SetPageListType(ePageType_ChatForbid);
}
void VhallAudienceWidget::SyncOnlineList(VhallAudienceUserInfoList &specialList, VhallAudienceUserInfoList &onlineList, VhallAudienceUserInfoList &gagList) {
   if (m_pSelfInfo != NULL) {
      ui->widgetOnline->ClearExceptParamter(m_pSelfInfo, mbHost, ePageType_OnLineUser);
   } else {
      ui->widgetOnline->ClearExceptParamter(NULL, mbHost, ePageType_OnLineUser);
   }
   
   for (int i = 0; i < specialList.count(); i++) {
      ui->widgetOnline->Append(specialList[i], mbHost,ePageType_OnLineUser);
   }
   for (int i = 0; i < gagList.count(); i++) {
      ui->widgetOnline->Append(gagList[i], mbHost, ePageType_OnLineUser);
   }
   for (int i = 0; i < onlineList.count(); i++) {
      ui->widgetOnline->Append(onlineList[i], mbHost, ePageType_OnLineUser);
   }
}
void VhallAudienceWidget::SyncGagList(VhallAudienceUserInfoList &gagList) {
   ui->widgetOnline->ClearExceptParamter(NULL, mbHost,ePageType_OnLineUser);
   for (int i = 0; i < gagList.
        count(); i++) {
      VhallAudienceUserInfo_st &info = gagList[i];
      ui->widgetGag->Append(info, mbHost, ePageType_ChatForbid);
   }
}
void VhallAudienceWidget::SyncKickList(VhallAudienceUserInfoList &kickList) {
   ui->widgetOnline->ClearExceptParamter(NULL, mbHost, ePageType_OnLineUser);
   for (int i = 0; i < kickList.
        count(); i++) {
      VhallAudienceUserInfo_st &info = kickList[i];
      ui->widgetShot->Append(info, mbHost,ePageType_KickOut);
   }
}

int VhallAudienceWidget::Count() {
   return ui->widgetOnline->Count();
}

VhallAudienceWidget::~VhallAudienceWidget() {
   if (NULL != m_pSelfInfo) {
      delete m_pSelfInfo;
      m_pSelfInfo = NULL;
   }
   this->removeEventFilter(this);
   delete ui;
}
void VhallAudienceWidget::on_buttonGroup_buttonClicked(QAbstractButton * button) {
   BtnClicked(button);
}
void VhallAudienceWidget::BtnClicked(QAbstractButton * button) {
   QList<QAbstractButton *>	btns = ui->buttonGroup->buttons();
   for (int i = 0; i < btns.count(); i++) {
      QAbstractButton *btn = btns[i];
      if (btn == button) {
         btn->setStyleSheet(BUTTON_DOWN_STYLE);
      } else {
         btn->setStyleSheet(BUTTON_UP_STYLE);
      }
   }
   if (ui->btn_online == button) {
      ui->widgetLineSubLeft->hide();
      ui->widgetLineSubRight->show();
      ui->stackedWidget->setCurrentWidget(ui->page_online);
   } else if (ui->btn_gag == button) {
      ui->widgetLineSubLeft->hide();
      ui->widgetLineSubRight->hide();
      ui->stackedWidget->setCurrentWidget(ui->page_gag);
   } else if (ui->btn_shot == button) {
      ui->widgetLineSubLeft->show();
      ui->widgetLineSubRight->hide();
      ui->stackedWidget->setCurrentWidget(ui->page_shot);
   }
}
void VhallAudienceWidget::SetOnlineNumber(int n) {
   int count = ui->widgetOnline->Count();
   if (n < count) {
      n = count;
   }
   ui->btn_online->setText(QString::fromWCharArray(L"ÔÚÏß£¨%1£©").arg(n));
}
void VhallAudienceWidget::SetGagNumber(int n) {
   ui->btn_gag->setText(QString::fromWCharArray(L"½ûÑÔ£¨%1£©").arg(n));
}
void VhallAudienceWidget::SetKickNumber(int n) {
   ui->btn_shot->setText(QString::fromWCharArray(L"Ìß³ö£¨%1£©").arg(n));
}
void VhallAudienceWidget::RemoveOnlineList(VhallAudienceUserInfo &info) {
   ui->widgetOnline->Remove(info);
}
void VhallAudienceWidget::AppendOnlineList(VhallAudienceUserInfo &info) {

   if (info.kickType == VhallShowType_Hide) {
      qDebug() << "KickTypeError";
   }

   ui->widgetOnline->Append(info, mbHost,ePageType_OnLineUser);
}
void VhallAudienceWidget::RemoveGagList(VhallAudienceUserInfo &info) {
   ui->widgetGag->Remove(info);
}
void VhallAudienceWidget::AppendGagList(VhallAudienceUserInfo &info) {
   ui->widgetGag->Append(info, mbHost,ePageType_ChatForbid);
}
void VhallAudienceWidget::RemovekickList(VhallAudienceUserInfo &info) {
   ui->widgetShot->Remove(info);
}
void VhallAudienceWidget::AppendkickList(VhallAudienceUserInfo &info) {
   ui->widgetShot->Append(info, mbHost,ePageType_KickOut);
}
void VhallAudienceWidget::ChangeOnlineList(VhallAudienceUserInfo &info) {
   ui->widgetOnline->Change(info);
}
void VhallAudienceWidget::ChangeGagList(VhallAudienceUserInfo &info) {
   ui->widgetGag->Change(info);
}
void VhallAudienceWidget::ChangekickList(VhallAudienceUserInfo &info) {
   ui->widgetShot->Change(info);
}
void VhallAudienceWidget::TabPressed() {
   QJsonObject body;
   if (ui->stackedWidget->currentWidget() == ui->page_online) {
      body["ac"] = "Online";
      BtnClicked(ui->btn_gag);
   } else if (ui->stackedWidget->currentWidget() == ui->page_gag) {
      body["ac"] = "Gag";
      BtnClicked(ui->btn_shot);
   } else if (ui->stackedWidget->currentWidget() == ui->page_shot) {
      body["ac"] = "ShotOff";
      BtnClicked(ui->btn_online);
   }

   SingletonVhallRightExtraWidgetIns::Instance().reportLog(L"InteractionMemberList_TabChange", eLogRePortK_InteractionMemberList_Tab, body);

}
void VhallAudienceWidget::Clear() {
   ui->widgetGag->ClearExceptParamter(NULL, false,ePageType_ChatForbid);
   ui->widgetOnline->ClearExceptParamter(NULL, false,ePageType_OnLineUser);
   ui->widgetShot->ClearExceptParamter(NULL, false,ePageType_KickOut);
}
void VhallAudienceWidget::SetIsHost(bool ok) {
   ui->widgetGag->SetIsHost(ok,ePageType_ChatForbid);
   ui->widgetOnline->SetIsHost(ok,ePageType_OnLineUser);
   ui->widgetShot->SetIsHost(ok,ePageType_KickOut);
   mbHost = ok;
}

void VhallAudienceWidget::setSelfInfo(VhallAudienceUserInfo& info) {
   if (NULL == m_pSelfInfo){
      m_pSelfInfo = new VhallAudienceUserInfo(info);
   }
}

void VhallAudienceWidget::ReleaseSelf(){
   if (NULL != m_pSelfInfo){
      delete m_pSelfInfo;
      m_pSelfInfo = NULL;
    }
}
