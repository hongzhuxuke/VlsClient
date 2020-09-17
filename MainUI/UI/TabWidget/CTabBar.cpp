//CTabBar.cpp
#include "CTabBar.h"
#include <QtGui>

CTabBar::CTabBar(QWidget *parent) :QTabBar(parent), mPressFlg(false){
}


void CTabBar::slot_DoubleClicked(){
   emit sig_tabDrag(currentIndex(), pos(), false);
   qDebug() << "CTabBar::slot_DoubleClicked" << endl;
}

void CTabBar::mousePressEvent(QMouseEvent *event){
   if (event->button() == Qt::LeftButton){
      mPressFlg = true;
      qDebug() << "CTabBar::mousePressEvent" << endl;
   }
   QTabBar::mousePressEvent(event);
}

void CTabBar::mouseReleaseEvent(QMouseEvent *event){
   if (event->button() == Qt::LeftButton && mPressFlg && !mDoubleClicked){
      if (!tabRect(currentIndex()).contains(event->pos())){
         emit sig_tabDrag(currentIndex(), event->pos(),true);
         qDebug() << "CTabBar::mouseReleaseEvent" << endl;
      }
      else {
         emit sig_clickedTabDrag(currentIndex(), event->pos());
         qDebug() << "CTabBar::mouseReleaseEvent" << endl;
      }
   }
   mPressFlg = false;
   mDoubleClicked = false;
   QTabBar::mouseReleaseEvent(event);
}

void CTabBar::mouseDoubleClickEvent(QMouseEvent *event){
   mDoubleClicked = true;
   emit sig_tabDrag(currentIndex(), event->pos(),false);
   qDebug() << "CTabBar::mouseDoubleClickEvent" << endl;
   QTabBar::mouseDoubleClickEvent(event);
}