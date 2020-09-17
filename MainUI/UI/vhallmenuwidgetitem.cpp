#include "vhallmenuwidgetitem.h"
#include "ui_vhallmenuwidgetitem.h"
#include <QPainter>
#include <QDebug>
#include <QFile>
#include <QDateTime>
#include <QFileInfo>
#include "pub.Const.h"

int VHallLog(const char * format, ...) {
   return 0;
#define LOGBUGLEN 1024
   char logBuf[LOGBUGLEN] = { 0 };
   va_list arg_ptr;
   va_start(arg_ptr, format);
   //int nWrittenBytes = sprintf_s(logBuf, format, arg_ptr);
   int nWrittenBytes = vsnprintf_s(logBuf, LOGBUGLEN, format, arg_ptr);
   va_end(arg_ptr);
   QFile f;
   f.setFileName("E:/vhall_debug.txt");
   if (f.open(QIODevice::Append)) {
      f.write(QDateTime::currentDateTime().toString("[yyyy-MM-dd HH:mm:ss.zzz]").toLocal8Bit());
      f.write(logBuf, nWrittenBytes);
      f.write("\r\n");
      f.close();
   }
   return nWrittenBytes;
}

QString VHallMenuWidgetItem::styleSheetNormal = "\
QLabel{\
background-color:transparent;\
color:#999999;\
}\
QLabel:hover{\
color:white\
}";
QString VHallMenuWidgetItem::styleSheetSelect = "\
QLabel{\
background-color:transparent;\
color:#F64840;\
}\
QLabel:hover{\
color:red\
}";
QString VHallMenuWidgetItem::styleSheetUnEnable="\
QLabel{\
background-color:transparent;\
color:gray;\
}\
QLabel:hover{\
color:gray\
}";



VHallMenuWidgetItem::VHallMenuWidgetItem(QWidget *parent) :
QWidget(parent),
ui(new Ui::VHallMenuWidgetItem),
isSelect(false) ,
mIsUnEnable(false)
{
   ui->setupUi(this);
   ui->exitBtn->hide();
   this->setStyleSheet(VHallMenuWidgetItem::styleSheetNormal);
   ui->exitBtn->loadPixmap(":/sysButton/deleteItem");
   connect(ui->exitBtn, SIGNAL(clicked()), this, SLOT(exitBtnClicked()));
   this->setStyleSheet("background-color:transparent");
}
void VHallMenuWidgetItem::setSelect(bool ok) {
   if (ok) {
      this->setStyleSheet(VHallMenuWidgetItem::styleSheetSelect);
   } else {
      this->setStyleSheet(VHallMenuWidgetItem::styleSheetNormal);
   }
   isSelect = ok;
   if(this->mIsUnEnable)
   {
      this->setStyleSheet(VHallMenuWidgetItem::styleSheetUnEnable);
   }
   ui->label->setStyleSheet(TOOLTIPQSS);
   this->repaint();
}
void VHallMenuWidgetItem::setUnEnable(bool ok)
{
   this->mIsUnEnable=ok;
   if(this->mIsUnEnable)
   {
      this->setStyleSheet(VHallMenuWidgetItem::styleSheetUnEnable);
   }
   else
   {
      if(this->isSelect)
      {
         this->setStyleSheet(VHallMenuWidgetItem::styleSheetSelect);
      }
      else
      {
         this->setStyleSheet(VHallMenuWidgetItem::styleSheetNormal);
      }
   }
}
bool VHallMenuWidgetItem::getUnEnable()
{
   return this->mIsUnEnable;
}
bool VHallMenuWidgetItem::getIsSelect() {
   return this->isSelect;
}
void VHallMenuWidgetItem::setDeleteBtnShow() {
   ui->exitBtn->show();
}
VHallMenuWidgetItem::~VHallMenuWidgetItem() {
   delete ui;
}
void VHallMenuWidgetItem::setText(QString str) {
   ui->label->setToolTip(str);
   str = ui->label->fontMetrics().elidedText(str, Qt::ElideRight,250);
   ui->label->setText(str);
}
void VHallMenuWidgetItem::paintEvent(QPaintEvent *e) {
   QWidget::paintEvent(e);
   QPainter p(this);
   int leftInterval = 2;
   int rightInterval = 2;
   int y = this->height() - 1;
   QColor lineColor = QColor(70, 70, 70, 255);
   p.setPen(lineColor);
   QFont f;
   f.setPixelSize(1);
   p.setFont(f);
   p.drawLine(leftInterval, y, this->width() - rightInterval - leftInterval, y);
}
void VHallMenuWidgetItem::setData(int index, QVariant vdata) {
   this->data[index] = vdata;
}
QVariant VHallMenuWidgetItem::getData(int index) {
   return this->data[index];
}
void VHallMenuWidgetItem::setMaxWidth(int width) {
   this->resize(width, this->height());
}
void VHallMenuWidgetItem::doResize(int w, int h) {
   QSize size = QSize(w, h);
   this->setMinimumSize(size);
   this->setMaximumSize(size);
   this->setMinimumSize(size);
   this->setMaximumSize(size);
}
void VHallMenuWidgetItem::exitBtnClicked() {
   emit this->sigDeleteItem(this);
}
void VHallMenuWidgetItem::setLabelCenter() {
   ui->label->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
}
