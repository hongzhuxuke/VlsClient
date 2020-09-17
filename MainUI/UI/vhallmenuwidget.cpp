#include "vhallmenuwidget.h"
#include "ui_vhallmenuwidget.h"
#include <QPainter>
#include <QDebug>
#include <QMouseEvent>
#include <QListWidgetItem>
#include <QListWidget>
#include <QScrollBar>
#include <QDir>
#include <QFileInfo>
#include <vector>
#include "TitleWidget.h"
#include "ConfigSetting.h"
using namespace std;

VHallMenuWidget::VHallMenuWidget(QWidget *parent) :
QWidget(parent),
mIsLoadEnable(false),
ui(new Ui::VHallMenuWidget) ,
mIsFileCheck(false)
{
   ui->setupUi(this);
   TitleWidget *titleBar=new TitleWidget(PLAY_FILE_LIST);
//   connect(titleBar,SIGNAL(closeWidget()),this,SLOT(close()));
   titleBar->hideCloseBtn();
   titleBar->SetMoveEnable(false);


   
   ui->gridLayoutTitle->addWidget(titleBar);
   this->setFocusPolicy(Qt::StrongFocus);
   this->setWindowFlags(Qt::FramelessWindowHint | Qt::SplashScreen | Qt::WindowStaysOnTopHint);
   this->setAttribute(Qt::WA_TranslucentBackground);
   this->setAutoFillBackground(false);
   this->setMaxItemCountShow(5);
   noneTip = "";   
   
   ui->listWidget->setStyleSheet(
      "QListWidget{background-color:rgb(38,38,38);}"
      "QListView::item:focus"
      "{"
      "color: rgb(153, 153, 153);border-image:null;"
      "}"
      "QListView::item:hover {"
      "color: rgb(153, 153, 153);border-image:null;"
      "}"

   );

   ui->listWidget->verticalScrollBar()->setStyleSheet(
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
      "background: rgb(38,38,38);"
      "}"
      "QScrollBar::add-page:vertical {"
      "background: rgb(38,38,38);"
      "}");
   
   Load();
}

VHallMenuWidget::~VHallMenuWidget() {
   Save();
   delete ui;
}
void VHallMenuWidget::DrawCenterHandstandTriangle(QPainter *p,QRect rect,QColor color){
    QPolygon po;
    po.append(QPoint(rect.x()+rect.width()/2-rect.height(),rect.y()));
    po.append(QPoint(rect.x()+rect.width()/2+rect.height(),rect.y()));
    po.append(QPoint(rect.x()+rect.width()/2,rect.y()+rect.height()));
    p->setBrush(QBrush(color));
    p->setPen(color);
    p->drawPolygon(po);
}
void VHallMenuWidget::closeEvent(QCloseEvent *e) {
   QWidget::closeEvent(e);
   emit SigClose();
}

void VHallMenuWidget::paintEvent(QPaintEvent *e) {
   if(!this->mBackgroundPixmap.isNull()){      
      QPainter painter(this);
      painter.drawPixmap(rect(),this->mBackgroundPixmap);
   }
   else {
      QWidget::paintEvent(e);
   }

}
void VHallMenuWidget::focusInEvent(QFocusEvent *e) {
   QWidget::focusInEvent(e);
}
void VHallMenuWidget::leaveEvent(QEvent *e) {
   enterType = false;
   QWidget::leaveEvent(e);
   this->close();
}
void VHallMenuWidget::enterEvent(QEvent *e) {
   enterType = true;
   QWidget::enterEvent(e);
}
void VHallMenuWidget::focusOutEvent(QFocusEvent *e) {

   QWidget::focusOutEvent(e);

   if (!enterType) {
      this->close();
   }

   QWidget *focusWidget = this->focusWidget();
   bool focusThis = false;

   while (focusWidget) {
      if (focusWidget == this) {
         focusThis = true;
         break;
      }

      focusWidget = focusWidget->parentWidget();
   }

   if (!focusThis) {
      this->close();
   } else {
      this->setFocus();
   }
}
void VHallMenuWidget::showEvent(QShowEvent *e) {
   enterType = false;
   QWidget::showEvent(e);
   this->setFocus();
}
void VHallMenuWidget::clear() {
   ui->listWidget->clear();
   Save();
}
void VHallMenuWidget::append(VHallMenuWidgetItem *itemWidget) {
   QListWidgetItem *item = new QListWidgetItem();
   ui->listWidget->insertItem(ui->listWidget->count(), item);
   item->setSizeHint(itemWidget->size());
   ui->listWidget->setItemWidget(item, itemWidget);
   Save();
}
void VHallMenuWidget::Flush(QStringList fileList) {
   VHallMenuWidgetItem *item=currentItem();
   QString currentFile="";
   if(item) {
      currentFile=item->getData(0).toString();
   }
   
   clear();
   for(int i=0;i<fileList.count();i++){
      VHallMenuWidgetItem * item=addItem(fileList[i]);
      if(item) {
         if(fileList[i]==currentFile){
            setSelected(item);
         }
      }
   }
}

void VHallMenuWidget::on_listWidget_itemClicked(QListWidgetItem *item) {
   FileCheck();
   QWidget *w = ui->listWidget->itemWidget(item);
   if(!w)
   {
      return ;
   }
   VHallMenuWidgetItem *itemWidget = dynamic_cast<VHallMenuWidgetItem *>(w);
   if(itemWidget->getUnEnable())
   {
      return ;
   }
   
   for (int i = 0; i < ui->listWidget->count(); i++) {
      QListWidgetItem *itemTemp = ui->listWidget->item(i);
      if (itemTemp) {
         QWidget *w = ui->listWidget->itemWidget(itemTemp);
         if (w) {
            VHallMenuWidgetItem *itemWidget = dynamic_cast<VHallMenuWidgetItem *>(w);
            if (itemWidget) {
               if (itemWidget->getUnEnable())  {
                  continue;
               }
               if (itemTemp == item) {
                  itemWidget->setSelect(true);
                  emit this->vHallMenuWidgetChecked(itemWidget);
               } else {
                  itemWidget->setSelect(false);
               }
            }
         }
      }
   }

   Save();

   this->repaint();

}
void VHallMenuWidget::on_listWidget_currentRowChanged(int currentRow) {
   FileCheck();
   QListWidgetItem *item = ui->listWidget->item(currentRow);
   if (item) {
      QWidget *w = ui->listWidget->itemWidget(item);
      if (w) {
         VHallMenuWidgetItem *itemWidget = dynamic_cast<VHallMenuWidgetItem *>(w);
         if (itemWidget->getUnEnable()) {
            return ;
         }
      }
   }


   for (int i = 0; i < ui->listWidget->count(); i++) {
      QListWidgetItem *item = ui->listWidget->item(i);
      if (item) {
         QWidget *w = ui->listWidget->itemWidget(item);
         if (w) {
            VHallMenuWidgetItem *itemWidget = dynamic_cast<VHallMenuWidgetItem *>(w);
            if (itemWidget) {
               if (i == currentRow) {
                  itemWidget->setSelect(true);
               } else {
                  itemWidget->setSelect(false);
               }
            }
         }
      }
   }
   Save();

   this->repaint();
}
void VHallMenuWidget::leftDownMoveShow(int x, int y) {
   QSize size = this->size();
   if (ui->listWidget->count() == 0) {
      this->resize(this->width(), 100);
      isNoneTip = true;
   } else {
      isNoneTip = false;
      QListWidgetItem *item = ui->listWidget->item(0);
      QWidget *w = ui->listWidget->itemWidget(item);
      if (w) {
         int h = w->height();
         int width = w->width();
         int count = ui->listWidget->count();
         if (count > this->maxItemCountShow) {
            //ui->listWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
            int vbarWidth = ui->listWidget->verticalScrollBar()->width();
            if (vbarWidth > 20) {
               vbarWidth = 20;
            }

            //
            //size.setWidth(width + vbarWidth);
            //size.setHeight(h*this->maxItemCountShow);
            
         } else {
            //ui->listWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
            //size.setWidth(width);
            //size.setHeight(h*count);
         }

         //qDebug() << "VHallMenuWidget::leftDownMoveShow" << size;
         //this->setMaximumSize(size);
         //this->setMinimumSize(size);
         //this->setMaximumSize(size);
         //this->setMinimumSize(size);
      } else {
         return;
      }
   }

   this->move(x, y - size.height()+12);
   //this->raise();
   this->show();
   Save();
   this->repaint();
}
void VHallMenuWidget::setMaxItemCountShow(int n) {
   this->maxItemCountShow = n;
}
void VHallMenuWidget::setNoneTip(QString noneTip) {
   this->noneTip = noneTip;
}
int VHallMenuWidget::count() {
   return ui->listWidget->count();
}
VHallMenuWidgetItem *VHallMenuWidget::item(int i) {
   VHallMenuWidgetItem *vItem = NULL;
   if (i >= ui->listWidget->count()) {
      return NULL;
   }

   QListWidgetItem *item = ui->listWidget->item(i);
   if (item) {
      QWidget *w = ui->listWidget->itemWidget(item);
      if (w) {
         vItem = dynamic_cast<VHallMenuWidgetItem *>(w);
      }
   }

   return vItem;
}
QString VHallMenuWidget::itemString(int i) {
   QString str = QString();
   VHallMenuWidgetItem *vItem = this->item(i);
   if (vItem) {
      str = vItem->getData(0).toString();
   }

   return str;
}
VHallMenuWidgetItem *VHallMenuWidget::Find(QString str) {
   for (int i = 0; i < this->count(); i++) {
      VHallMenuWidgetItem *item = this->item(i);
      if (item) {
         if (item->getData(0).toString() == str) {
            return item;
         }
      }
   }
   return NULL;
}
void VHallMenuWidget::setIsFileCheck(bool ok)
{
   this->mIsFileCheck=ok;
}
bool VHallMenuWidget::setItems(QStringList fileList,int currntIndex) {
   VHallMenuWidgetItem *item=this->currentItem();
   QString filename="";
   bool ret=false;
   
   if(item) {
      filename=item->getData(0).toString();
   }

   this->clear();
   
   for_each(fileList.begin(),fileList.end(),[&](const QString &file) mutable {
      this->addItem(file);
   });

   for(int i=0;i<this->count();i++){
      VHallMenuWidgetItem *item=this->item(i);
      if(item) {
         QString currentFilename=item->getData(0).toString();
         if(currentFilename==fileList[currntIndex]){
            this->setSelected(item);
            //if(filename!=currentFilename)
            {
               emit this->vHallMenuWidgetChecked(item);
            }
            break;
         }
      }
   }

   return fileList[currntIndex]!=filename;
}

VHallMenuWidgetItem * VHallMenuWidget::addItem(QString filename) {   
   FileCheck();
   if (this->Find(filename) != NULL) {
      return NULL;
   }

   VHallMenuWidgetItem *item = new VHallMenuWidgetItem();
   item->setData(0, QVariant(filename));
   //item->doResize(200, item->height());
   item->doResize(268, 40);

   QFileInfo info;
   info.setFile(filename);
   item->setText(info.fileName());

   if (this->count() == 0) {
      item->setSelect(true);
   } else {
      item->setSelect(false);
   }

   item->setDeleteBtnShow();
   connect(item, SIGNAL(sigDeleteItem(VHallMenuWidgetItem *)), this, SLOT(slotItemDelete(VHallMenuWidgetItem *)));
   this->append(item);
   Save();

   this->repaint();
   return item;
}
VHallMenuWidgetItem *VHallMenuWidget::currentItem() {
   for (int i = 0; i < this->count(); i++) {
      VHallMenuWidgetItem *item = this->item(i);
      if (item) {
         if(item->getUnEnable())
         {
            continue;
         }
         
         if (item->getIsSelect()) {
            return item;
         }
      }
   }
   return NULL;
}
VHallMenuWidgetItem *VHallMenuWidget::nextItem()
{
   FileCheck();
   int index=this->count();
   for (int i = 0; i < this->count(); i++) {
      VHallMenuWidgetItem *item = this->item(i);
      if (item) {
         if(item->getUnEnable())
         {
            continue;
         }
         
         if (item->getIsSelect()) {
            index=i;
            item->setSelect(false);
         }
         
         if(i>index)
         {
            item->setSelect(true);
            return item;
         }
      }
   }
   
   for (int i = 0; i < this->count(); i++) {
      VHallMenuWidgetItem *item = this->item(i);
      if (item) {
         if(item->getUnEnable())
         {
            continue;
         }
        
         item->setSelect(true);
         return item;
      }
   }

   return NULL;
}
void VHallMenuWidget::FileCheck()
{
   if(!mIsFileCheck)
   {
      return ;
   }

   int checkedIndex=-1;
   for (int i = 0; i < this->count(); i++) {
      VHallMenuWidgetItem *item = this->item(i);
      if (item) {
         QString filename=item->getData(0).toString();
         QFileInfo info;
         info.setFile(filename);
         if(!info.exists())
         {
            item->setUnEnable(true);
            if(item->getIsSelect())
            {
               checkedIndex=i+1;
               item->setSelect(false);
            }
         }
         else
         {
            item->setUnEnable(false);
            if (i == checkedIndex) {
               item->setSelect(true);
            }
         }
      }
   }
   if(checkedIndex==this->count())
   {
      for (int i = 0; i < this->count(); i++) {
         VHallMenuWidgetItem *item = this->item(i);
         if(item)
         {
            QString filename=item->getData(0).toString();
            QFileInfo info;
            info.setFile(filename);
            if(info.exists())
            {
               item->setSelect(true);
               return ;
            }

         }
      }
   }
}
int VHallMenuWidget::ItemIndex(VHallMenuWidgetItem *item) {
   if (!item) {
      return -1;
   }

   int index = -1;
   for (int i = 0; i < this->count(); i++) {
      VHallMenuWidgetItem *tmp = this->item(i);
      if (tmp) {
         if (tmp == item) {
            index = i;
            break;
         }
      }
   }
   return index;
}
void VHallMenuWidget::LoadPixmap(QString pixmapName) {
   this->mBackgroundPixmap.load(pixmapName);

   if(!this->mBackgroundPixmap.isNull()) {
      this->setFixedSize(this->mBackgroundPixmap.size());
   }
}

void VHallMenuWidget::setSelected(VHallMenuWidgetItem *item) {
   if (!item) {
      return;
   }

   for (int i = 0; i < this->count(); i++) {
      VHallMenuWidgetItem *tmp = this->item(i);
      if (tmp) {
         if (tmp == item) {
            ui->listWidget->setCurrentRow(i);
            tmp->setSelect(true);
         } else {
            tmp->setSelect(false);
         }
      }
   }
}
void VHallMenuWidget::deleteItem(int i) {
   if (i < 0 || i >= this->count()) {
      return;
   }

   QListWidgetItem *item = ui->listWidget->item(i);
   delete item;
   Save();
}
void VHallMenuWidget::slotItemDelete(VHallMenuWidgetItem *item) {
   if (!item) {
      return;
   }

   int toDeleteItempos = -1;
   for (int i = 0; i < this->count(); i++) {
      VHallMenuWidgetItem *itemtmp = this->item(i);
      if (itemtmp == item) {
         if (item->getIsSelect()) {
            toDeleteItempos = i;

         }
         deleteItem(i);
         break;
      }
   }

   if (toDeleteItempos<0) {
      return;
   }

   if (this->count()>0) {
      if (toDeleteItempos >= this->count()) {
         toDeleteItempos = 0;
      }

      VHallMenuWidgetItem *itemNext = this->item(toDeleteItempos);
      if (itemNext) {
         this->setSelected(itemNext);
      }
      
      emit this->vHallMenuWidgetChecked(itemNext);
   }
   else {
      emit this->vHallMenuWidgetChecked(NULL);
   }

}
void VHallMenuWidget::Load() {
   if (!this->mIsLoadEnable) {
      return;
   }
   this->mSaveMutex.lock();
   QJsonArray mediaListJsonArray = MediaListFileRead();
   for (int i = 0; i < mediaListJsonArray.count(); i++) {
      QString filename = mediaListJsonArray[i].toString();
      this->addItem(filename);
   }
   this->mSaveMutex.unlock();
   FileCheck();
}
void VHallMenuWidget::Save() {
   if (!this->mIsLoadEnable) {
      return;
   }
   if (this->mSaveMutex.tryLock()) {
      QJsonArray array;
      for (int i = 0; i < this->count(); i++) {
         VHallMenuWidgetItem *item = this->item(i);
         if (item) {
            QString filename = item->getData(0).toString();
            array.append(filename);
         }
      }
      MediaListFileWrite(array);
      this->mSaveMutex.unlock();
   }
}
QString VHallMenuWidget::GetMediaListFileCacheFile() {
   return QDir::tempPath() + "/media.list";
   //return QApplication::applicationDirPath() + "/media.list";
}
void VHallMenuWidget::MediaListFileWrite(QJsonArray value) {
   QString filename = GetMediaListFileCacheFile();
   QFile f;
   f.setFileName(filename);
   if (f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
      QJsonDocument doc;
      doc.setArray(value);
      QByteArray ba = doc.toJson();
      f.write(ba);
      f.close();
   }
}
QJsonArray VHallMenuWidget::MediaListFileRead() {
   QJsonArray array = QJsonArray();
   QString filename = GetMediaListFileCacheFile();
   QFile f;
   f.setFileName(filename);
   if (f.open(QIODevice::ReadOnly)) {
      QByteArray ba = f.readAll();
      f.close();
      QJsonDocument doc = QJsonDocument::fromJson(ba);
      array = doc.array();
   }

   return array;
}
void VHallMenuWidget::setLoadEnable(bool ok) {
   this->mIsLoadEnable = ok;
   if (this->mIsLoadEnable) {
      this->Load();
   }
}

