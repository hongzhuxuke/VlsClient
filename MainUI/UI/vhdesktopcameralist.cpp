#include "stdafx.h"

#include "vhdesktopcameralist.h"
#include "ui_vhdesktopcameralist.h"
#include <vhdesktopcameralistitem.h>
#include <QPixmap>
#include <QPainter>
#include <QDebug>

#include "Msg_MainUI.h"
#include "MainUILogic.h"
#include "IOBSControlLogic.h"

#define HEAD mBodyHead
#define BODY mBodyShow
#define TAIL mBodyTail

#define HEADSHOW mBodyHead
#define BODYSHOW mBodyShow
#define TAILSHOW mBodyTail

VHDesktopCameraList::VHDesktopCameraList(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::VHDesktopCameraList)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::SplashScreen | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);
    this->setAutoFillBackground(true);
    
    tipsLabel = new QLabel(this);
    tipsLabel->setText(QString::fromWCharArray(L"未检测到视频设备"));
    tipsLabel->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
    ui->verticalLayout->addWidget(tipsLabel);

    ui->widget_head->setMaximumHeight(HEADSHOW);
    ui->widget_head->setMinimumHeight(HEADSHOW);
    ui->widget_tail->setMaximumHeight(TAILSHOW);
    ui->widget_tail->setMinimumHeight(TAILSHOW);
}
void VHDesktopCameraList::SetPixmap(QPixmap p) {
   pixmap = p;
   pixmap_head = pixmap.copy(0,0,pixmap.width(),HEAD).scaledToHeight(HEADSHOW, Qt::SmoothTransformation);
   pixmap_body = pixmap.copy(0,HEAD,pixmap.width(),BODY).scaledToHeight(BODYSHOW, Qt::SmoothTransformation);
   pixmap_tail = pixmap.copy(0,pixmap.height()-TAIL,pixmap.width(),TAIL).scaledToHeight(TAILSHOW, Qt::SmoothTransformation);
   
   this->resize(pixmap.size());
   this->setMinimumWidth(pixmap.width());
   this->setMaximumWidth(pixmap.width());
}
void VHDesktopCameraList::SetPixmap(QString str){
    pixmap = QPixmap(str);
    pixmap_head = pixmap.copy(0,0,pixmap.width(),HEAD).scaledToHeight(HEADSHOW, Qt::SmoothTransformation);
    pixmap_body = pixmap.copy(0,HEAD,pixmap.width(),BODY).scaledToHeight(BODYSHOW, Qt::SmoothTransformation);
    pixmap_tail = pixmap.copy(0,pixmap.height()-TAIL,pixmap.width(),TAIL).scaledToHeight(TAILSHOW, Qt::SmoothTransformation);

    this->resize(pixmap.size());
    this->setMinimumWidth(pixmap.width());
    this->setMaximumWidth(pixmap.width());
}
void VHDesktopCameraList::SetTipsLabelText(QString txt) {
   tipsLabel->setText(txt);
}
VHDesktopCameraList::~VHDesktopCameraList()
{
    delete ui;
}
void VHDesktopCameraList::paintEvent(QPaintEvent *e){
    QPixmap a=QPixmap(this->size());
    a.fill(Qt::transparent);

    QPainter p(&a);

    p.drawPixmap(0,0,pixmap.width(),HEADSHOW,pixmap_head);

    int count = this->mCurrentWidget.count();
    if(count<1) {
        count = 1;
    }

    for(int i=0;i<count; i ++) {
        p.drawPixmap(0,HEADSHOW+BODYSHOW*i,pixmap.width(),BODYSHOW,pixmap_body);
    }
    p.drawPixmap(0,HEADSHOW+BODYSHOW*count,pixmap.width(),TAILSHOW,pixmap_tail);

    QPainter p2(this);
    p2.drawPixmap(this->rect(),a);
}
void VHDesktopCameraList::AddItemInterActivityItem(QString str,DeviceInfo data,int index) {
   VHDesktopCameraListItem *item = new VHDesktopCameraListItem();
   item->setMaximumHeight(BODYSHOW);
   item->setMinimumHeight(BODYSHOW);
   item->SetData(str, data, index);
   this->mCurrentWidget.append(item);
   item->SetToggle(false);
   ui->verticalLayout->addWidget(item);
   connect(item,SIGNAL(SigClicked(bool,DeviceInfo)),this,SIGNAL(SigItemClicked(bool,DeviceInfo)));
   connect(item,SIGNAL(SigClicked(bool,DeviceInfo)),this,SLOT(SlotItemClicked(bool,DeviceInfo)));
   if(checkedCount >= maxCount) {
      item->SetEnabled(false);
   }

}
void VHDesktopCameraList::SlotItemClicked(bool ok,DeviceInfo device){
   if(ok) {
      checkedCount++;
   }
   else {
      checkedCount--;
   }
   
   for(int i = 0 ;i<mCurrentWidget.count();i++) {
      VHDesktopCameraListItem *item = dynamic_cast<VHDesktopCameraListItem *>(mCurrentWidget[i]);
      if(item) {
         if(!item->IsChecked()) {
				QJsonObject body;
				body["vdn"] = QString::fromWCharArray(device.m_sDeviceDisPlayName);//vdn:设备用于显示的名字
				body["vn"] = QString::fromWCharArray(device.m_sDeviceName);
				body["vid"] = QString::fromWCharArray(device.m_sDeviceID);
				body["vt"] = device.m_sDeviceType;
				body["ac"] = maxCount > checkedCount ? "Use" : "Cancle";
            item->SetEnabled(maxCount > checkedCount);
				SingletonMainUIIns::Instance().reportLog(L"interaction_camerachoice", eLogRePortK_Interaction_CameraChoice, body);
         }
      }
   }

}

void VHDesktopCameraList::AddItem(QString str,DeviceInfo data,int index){
    VHDesktopCameraListItem *item = new VHDesktopCameraListItem();
    item->setMaximumHeight(BODYSHOW);
    item->setMinimumHeight(BODYSHOW);
    item->SetData(str, data, index);
    this->mCurrentWidget.append(item);
    ui->verticalLayout->addWidget(item);
}
void VHDesktopCameraList::Clear(){
    for(int i=0;i<this->mCurrentWidget.size();i++) {
        delete this->mCurrentWidget[i];
    }
    this->mCurrentWidget.clear();
    checkedCount = 0;
}
void VHDesktopCameraList::Resize(){
    int size = HEADSHOW +BODYSHOW*(mCurrentWidget.count()>1?mCurrentWidget.count():1)+TAILSHOW;
    this->resize(this->pixmap.width(),size);
    this->repaint();
}
void VHDesktopCameraList::Show() {
    this->tipsLabel->setVisible(this->mCurrentWidget.count() == 0);
    this->setFocus();
    this->show();
    Resize();
}
void VHDesktopCameraList::focusOutEvent(QFocusEvent *e){
    this->close();
}
void VHDesktopCameraList::SetChecked(DeviceInfo &dInfo){
   
   for(int i = 0 ;i<mCurrentWidget.count();i++) {
      VHDesktopCameraListItem *item = dynamic_cast<VHDesktopCameraListItem *>(mCurrentWidget[i]);
      if(item) {
         if(item->SetChecked(dInfo)){
            checkedCount ++ ;
         }
      }
   }
   
   for(int i = 0 ;i<mCurrentWidget.count();i++) {
      VHDesktopCameraListItem *item = dynamic_cast<VHDesktopCameraListItem *>(mCurrentWidget[i]);
      if(item) {
         if(!item->IsChecked()) {
            item->SetEnabled(maxCount > checkedCount);
         }
      }
   }

}
void VHDesktopCameraList::FlushEnableStatus(){
   int checkedCount = 0 ;
   VH::CComPtr<IOBSControlLogic> pObsControlLogic = NULL;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IOBSControl, IID_IOBSControlLogic, pObsControlLogic, return , ASSERT(FALSE));
   checkedCount = pObsControlLogic->GetGraphicsDeviceInfoCount();
   if(checkedCount>=2) {
      for(int i = 0 ;i<mCurrentWidget.count();i++) {
         VHDesktopCameraListItem *item = dynamic_cast<VHDesktopCameraListItem *>(mCurrentWidget[i]);
         if(item) {
            item->SetEnabled(item->IsChecked());
         }
      }
   }
   else {
      for(int i = 0 ;i<mCurrentWidget.count();i++) {
         VHDesktopCameraListItem *item = dynamic_cast<VHDesktopCameraListItem *>(mCurrentWidget[i]);
         if(item) {
            item->SetEnabled(true);
         }
      }

   }
}

void VHDesktopCameraList::RemoveDevice(QString deviceID) {
   for (int i = 0; i < mCurrentWidget.count(); i++) {
      VHDesktopCameraListItem *item = dynamic_cast<VHDesktopCameraListItem *>(mCurrentWidget[i]);
      if (item && item->GetDeviceID().compare(deviceID) == 0) {
        item->SetChecked(false);
        break;
      }
   }
}
void VHDesktopCameraList::SetBodyShow(int h,int b,int t) {
   mBodyHead = h;
   mBodyShow = b;
   mBodyTail=t;
      
   ui->widget_head->setMinimumHeight(h);
   ui->widget_head->setMaximumHeight(h);
   ui->widget_tail->setMinimumHeight(t);
   ui->widget_tail->setMaximumHeight(t);
}
void VHDesktopCameraList::SetBodySide(int l,int r) {
   ui->gridLayout_2->setContentsMargins(l,0,r,0);
}

QString VHDesktopCameraList::GetCheckedDeviceID() {
   for (int i = 0; i < mCurrentWidget.count(); i++) {
      VHDesktopCameraListItem *item = dynamic_cast<VHDesktopCameraListItem *>(mCurrentWidget[i]);
      if (item && item->IsChecked()) {
         return item->GetDeviceID();
      }
   }
   return QString();
}
