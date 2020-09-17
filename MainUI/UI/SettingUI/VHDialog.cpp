#include "VHDialog.h"
#include "IVedioPlayLogic.h"
#include "stdafx.h"
#include <QKeyEvent>
#include <QDebug>

int VHDialog::mShowCount=0;
VHDialog *VHDialog::mCurrent=NULL;

VHDialog::VHDialog(QWidget *parent)
: CBaseDlg(parent)
{
   setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
   setAttribute(Qt::WA_TranslucentBackground);
   setAutoFillBackground(true);
}
VHDialog::~VHDialog()
{

}


void VHDialog::Raise(){
   if(VHDialog::mCurrent) {
      VHDialog::mCurrent->raise();
      //qDebug()<<"VHDialog::Raise ";
   }
   else {
      //qDebug()<<"VHDialog::Raise Null ";
   }
} 
int VHDialog::VHDialogShowCount() {
   return mShowCount;
}

void VHDialog::WillShow(void *obj){
   mShowCount++;
}

void VHDialog::WillClose() {
   mShowCount--;
   if(mShowCount==0) {
      VHDialog::mCurrent=NULL;
   }
}

void VHDialog::showEvent(QShowEvent *e)
{
   
   raise();
   
   qDebug()<<"VHDialog::showEvent";
   VHDialog::mCurrent=this;

   WillShow(this);

	QDialog::showEvent(e);
}
void VHDialog::hideEvent(QHideEvent *e)
{
   
   qDebug()<<"VHDialog::hideEvent";
   
   WillClose();
	QDialog::hideEvent(e);
}
void VHDialog::OnSave()
{
   hide();
}
void  VHDialog::keyPressEvent(QKeyEvent *e)
{
   if ( e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return){
      OnSave();
   }
   else if (e->key() == Qt::Key_Escape) {
      return;
   }
   else
   {
      QDialog::keyPressEvent(e);
   }
}

