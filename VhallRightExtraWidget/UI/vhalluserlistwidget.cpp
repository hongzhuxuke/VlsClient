#include "vhalluserlistwidget.h"
#include "ui_vhalluserlistwidget.h"
#include <QListWidgetItem>
#include <QScrollBar>
#include <QWheelEvent>
#include "Msg_VhallRightExtraWidget.h"
#include "VhallRightExtraWidgetIns.h"
#include "DebugTrace.h"


VhallUserListWidget::VhallUserListWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::VhallUserListWidget) {
   ui->setupUi(this);

   ui->listWidget->setStyleSheet(
	   "QAbstractItemView::item {border:0px;color: rgb(153, 153, 153);background-color: rgb(255, 255, 255);}"
      "QAbstractItemView::item:focus {color: rgb(153, 153, 153);background-color: rgb(93,93,93);}"
      "QAbstractItemView::item:hover {color: rgb(153, 153, 153);background-color: rgb(93,93,93);}"
      "QListView::item:focus"
      "{"
      "color: rgb(153, 153, 153);border-image:null;"
      "}"
      "QListView::item:hover {"
      "color: rgb(153, 153, 153);border-image:null;"
      "}"
      "QListWidget{background-color:rgb(20,20,20);outline:0px;}"
      "QScrollBar:vertical"
      "{"
      "width:4px;"
      "margin:0px,0px,0px,0px;"
      "}"
      "QScrollBar::handle:vertical"
      "{"
      "width:4px;"
      "background:rgb(205,205,205);"
      "border-radius:2px;"
      "min-height:20;"
      "}"
      "QScrollBar::handle:vertical:hover"
      "{"
      "width:4px;"
      "background:rgb(180,180,180);"
      "border-radius:2px;"
      "min-height:20;"
      "}"
      "QScrollBar::add-line:vertical"
      "{"
      "height:0px;"
      "width:4px;"
      "subcontrol-position:bottom;"
      "}"
      "QScrollBar::sub-line:vertical"
      "{"
      "height:0px;"
      "width:4px;"
      "subcontrol-position:top;"
      "}"
      "QScrollBar::sub-page:vertical {"
      "background: rgb(20,20,20);"
      "}"
      "QScrollBar::add-page:vertical {"
      "background: rgb(20,20,20);"
      "}"
   );   
}
VhallUserListWidget::~VhallUserListWidget() {
    delete ui;
}
void VhallUserListWidget::Append(VhallAudienceUserInfo &info,bool bOpera,int pageType) {
   for (int i = 0; i<ui->listWidget->count(); i++) {
      QListWidgetItem *tmpItem = ui->listWidget->item(i);
      if (tmpItem) {
         QWidget *w = ui->listWidget->itemWidget(tmpItem);
         if (w) {
            VhallUserListItem *tmpItemWidget = dynamic_cast<VhallUserListItem *>(w);
            if(!bOpera) {
               tmpItemWidget->SetOperator(bOpera, pageType);
            }
            else {
               if(tmpItemWidget->GetUserInfo().role!="host")
               {
                  tmpItemWidget->SetOperator(bOpera, pageType);
               }
            }
            
            if (tmpItemWidget) {
               VhallAudienceUserInfo tmpUserInfo = tmpItemWidget->GetUserInfo();
               if (tmpUserInfo == info) {
                  return ;
               }
            }
         }
      }   
   }

   QListWidgetItem *item = new QListWidgetItem();
   VhallUserListItem *itemWidget = new VhallUserListItem();

   itemWidget->SetUserInfo(info,mPageType);
   itemWidget->SetOperator(bOpera, pageType);
   item->setSizeHint(QSize(0,itemWidget->height()));
   itemWidget->ShowGag(isGagHide ? VhallShowType_Hide : info.gagType, mPageType);
   itemWidget->ShowKick(isShotHide ? VhallShowType_Hide : info.kickType, mPageType);
   TRACE6("%s info.role == USER_USER id:%s role:%s name:%s\n", __FUNCTION__,
	   info.userId.toStdString().c_str(), info.role.toStdString().c_str(), info.userName.toStdString().c_str());

   if(info.role == USER_USER) {
      TRACE6("%s %ld addItem  info.role == USER_USER\n", __FUNCTION__, __threadid());
      ui->listWidget->addItem(item);
      ui->listWidget->setItemWidget(item,itemWidget);
   }
   else {
      for (int i = 0; i < ui->listWidget->count(); i++) {
         QListWidgetItem *tmpItem = ui->listWidget->item(i);
         if (tmpItem) {
            QWidget *w = ui->listWidget->itemWidget(tmpItem);
            if (w) {
               VhallUserListItem *tmpItemWidget = dynamic_cast<VhallUserListItem *>(w);
               if (!bOpera) {
                  tmpItemWidget->SetOperator(bOpera, pageType);
               } else if (tmpItemWidget->GetUserInfo().role != "host") {
                  tmpItemWidget->SetOperator(bOpera, pageType);
               }

               if (tmpItemWidget) {
                  VhallAudienceUserInfo tmpUserInfo = tmpItemWidget->GetUserInfo();
                  if (!(info < tmpUserInfo)) {
                     TRACE6("%s %ld insertItem i=%d\n", __FUNCTION__, __threadid(), i);
                     ui->listWidget->insertItem(i, item);
                     ui->listWidget->setItemWidget(item, itemWidget);
                     return;
                  }
               }
            }
         }
      }
      TRACE6("%s %ld add user\n", __FUNCTION__,__threadid());
      ui->listWidget->addItem(item);
      ui->listWidget->setItemWidget(item,itemWidget);
   }
}
void VhallUserListWidget::Remove(VhallAudienceUserInfo &info) {
   for(int i=0;i<ui->listWidget->count();i++) {
      QListWidgetItem *item = ui->listWidget->item(i);
      if(item) {
         QWidget *w=ui->listWidget->itemWidget(item);
         if(w) {
            VhallUserListItem *itemWidget = dynamic_cast<VhallUserListItem *>(w);
            if(itemWidget->HasUserInfo(info)) {
               TRACE6("%s info.role == USER_USER id:%s role:%s name:%s\n", __FUNCTION__,
				   info.userId.toStdString().c_str(), info.role.toStdString().c_str(), info.userName.toStdString().c_str());
               delete item;
               return ;
            }
         }
      }
   }
}
void VhallUserListWidget::Change(VhallAudienceUserInfo &info) {
   for(int i=0;i<ui->listWidget->count();i++) {
      QListWidgetItem *item = ui->listWidget->item(i);
      if(item) {
         QWidget *w=ui->listWidget->itemWidget(item);
         if(w) {
            VhallUserListItem *itemWidget = dynamic_cast<VhallUserListItem *>(w);
            if(itemWidget->HasUserInfo(info)) {
               itemWidget->ShowGag(isGagHide ? VhallShowType_Hide : info.gagType, mPageType);
               itemWidget->ShowKick(isShotHide ? VhallShowType_Hide : info.kickType, mPageType);
               if (isShotHide == true || info.kickType == VhallShowType_Hide) {
                  qDebug() << "HideKick";
               }
            }
         }
      }
   }
}

void VhallUserListWidget::ClearExceptParamter(VhallAudienceUserInfo* SelfInfo, bool bHost, int pageType) {
   TRACE6("%s %ld\n",__FUNCTION__,__threadid());
   ui->listWidget->clear();
   if (NULL != SelfInfo) {
      Append(*SelfInfo, bHost, pageType);
   }
}
int VhallUserListWidget::Count() {
   return ui->listWidget->count();
}
void VhallUserListWidget::SetMonitorVerticalBar(bool ok) {
   if(ok) {

      connect(&m_timer,SIGNAL(timeout()),this,SLOT(timeout()));
      connect(ui->listWidget->verticalScrollBar(),SIGNAL(valueChanged(int)),this,SLOT(ListValueChangeSlots(int)));
      ui->listWidget->installEventFilter(this);
   }
}
void VhallUserListWidget::timeout() {
   m_timer.stop();
   SingletonVhallRightExtraWidgetIns::Instance().PostCRMessage(
      MSG_VHALLRIGHTEXTRAWIDGET_GETNEWPAGEONLINELIST,&m_pageControl,sizeof(int));  
}

void VhallUserListWidget::ListValueChangeSlots(int v){
   if(ui->listWidget->verticalScrollBar()->maximum() == 0) {
      m_pageControl = CHANGE_PAGE ;
      return ;
   }
   if(v == ui->listWidget->verticalScrollBar()->maximum()){
      m_pageControl = NEXT_PAGE;
   }
   else if(v==0) {
      m_pageControl = PREV_PAGE;
   }
   else {
      m_pageControl = NULL_PAGE;
   }
}
bool VhallUserListWidget::eventFilter(QObject *obj,QEvent *e) {
   if(obj == ui->listWidget) {
      if(e->type()==QEvent::Wheel) {
         QWheelEvent *we=dynamic_cast<QWheelEvent *>(e);
         if(we) {
            if(m_pageControl != NULL_PAGE) {
               if(CHANGE_PAGE == m_pageControl) {
                  if(we->delta() > 0) {
                     m_pageControl = PREV_PAGE;
                  }
                  else {
                     m_pageControl = NEXT_PAGE;
                  }
               }
               m_timer.start(1000);
            }
         }
      }
   }
   return QWidget::eventFilter(obj,e);
}

void VhallUserListWidget::SetGagHide(bool ok) {
   isGagHide = ok;
}
void VhallUserListWidget::SetShotHide(bool ok) {
   isShotHide = ok;
}
void VhallUserListWidget::SetIsHost(bool ok, int pageType) {
   for (int i = 0; i<ui->listWidget->count(); i++) {
      QListWidgetItem *tmpItem = ui->listWidget->item(i);
      if (tmpItem) {
         QWidget *w = ui->listWidget->itemWidget(tmpItem);
         if (w) {
            VhallUserListItem *tmpItemWidget = dynamic_cast<VhallUserListItem *>(w);
            tmpItemWidget->SetOperator(ok, pageType);
         }
      }   
   }
}

void VhallUserListWidget::SetPageListType(int pageType) {
   mPageType = pageType;
}
