#ifndef VHALLMENUWIDGETITEM_H
#define VHALLMENUWIDGETITEM_H

#include <QWidget>
#include <QPaintEvent>
#include <QMap>
#include <QVariant>
namespace Ui {
   class VHallMenuWidgetItem;
}

int VHallLog(const char * format, ...);

class VHallMenuWidgetItem : public QWidget {
   Q_OBJECT

public:
   explicit VHallMenuWidgetItem(QWidget *parent = 0);
   ~VHallMenuWidgetItem();
   void setMaxWidth(int);
   void setText(QString);
   void setSelect(bool);
   void setUnEnable(bool);
   bool getUnEnable();
   void setData(int, QVariant);
   QVariant getData(int);
   void doResize(int w, int h);
   bool getIsSelect();
   void setDeleteBtnShow();
   void setLabelCenter();
   public slots:
   void exitBtnClicked();
signals:
   void sigDeleteItem(VHallMenuWidgetItem *);
protected:
   void paintEvent(QPaintEvent *);
private:
   Ui::VHallMenuWidgetItem *ui;
   static QString styleSheetNormal;
   static QString styleSheetSelect;
   static QString styleSheetUnEnable;
   QMap <int, QVariant> data;
   bool isSelect;
   bool mIsUnEnable;
};

#endif // VHALLMENUWIDGETITEM_H
