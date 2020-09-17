#ifndef VHALLMENUWIDGET_H
#define VHALLMENUWIDGET_H

#include <QWidget>
#include <QPaintEvent>
#include <QFocusEvent>
#include <QShowEvent>
#include <QListWidgetItem>
#include <QFile>
#include <QMutex>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonValue>
#include <QPixmap>
#include "vhallmenuwidgetitem.h"
namespace Ui {
   class VHallMenuWidget;
}

class VHallMenuWidget : public QWidget {
   Q_OBJECT

public:
   explicit VHallMenuWidget(QWidget *parent = 0);
   ~VHallMenuWidget();
   void setLoadEnable(bool);
   void clear();
   void append(VHallMenuWidgetItem *);
   void leftDownMoveShow(int x, int y);
   void setMaxItemCountShow(int n);
   void setNoneTip(QString);
   int count();
   VHallMenuWidgetItem *item(int i);
   void setIsFileCheck(bool);
   
   //----------------------------------------------------------

   VHallMenuWidgetItem *Find(QString);
   QString itemString(int i);
   VHallMenuWidgetItem * addItem(QString);
   bool setItems(QStringList,int);
   VHallMenuWidgetItem *currentItem();
   VHallMenuWidgetItem *nextItem();
   void deleteItem(int i);
   int ItemIndex(VHallMenuWidgetItem *);
   void setSelected(VHallMenuWidgetItem *);
   void LoadPixmap(QString);
   void Flush(QStringList);
public slots:
   void slotItemDelete(VHallMenuWidgetItem *);
signals:
   void vHallMenuWidgetChecked(VHallMenuWidgetItem *);
   void SigClose();
protected:
   virtual void closeEvent(QCloseEvent *);
   virtual void paintEvent(QPaintEvent *);
   virtual void focusInEvent(QFocusEvent *);
   virtual void focusOutEvent(QFocusEvent *);
   virtual void showEvent(QShowEvent *);
   virtual void leaveEvent(QEvent *);
   virtual void enterEvent(QEvent *);
private slots:
   void on_listWidget_currentRowChanged(int currentRow);
   void on_listWidget_itemClicked(QListWidgetItem *item);
private:
   void Load();
   void Save();
   QString GetMediaListFileCacheFile();
   void MediaListFileWrite(QJsonArray);
   QJsonArray MediaListFileRead();
   void FileCheck();   
   void DrawCenterHandstandTriangle(QPainter *p,QRect rect,QColor color);
private:
   Ui::VHallMenuWidget *ui;
   bool enterType = false;
   int maxItemCountShow;
   QString noneTip;
   bool isNoneTip = false;
   QMutex mSaveMutex;
   bool mIsLoadEnable;
   bool mIsFileCheck;
   QPixmap mBackgroundPixmap;
};

#endif // VHALLMENUWIDGET_H
