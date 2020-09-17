#ifndef _VHDIALOG_H_
#define _VHDIALOG_H_
#include <QDialog>
#include "cbasedlg.h"

class VHDialog : public CBaseDlg {
   Q_OBJECT

public:
   VHDialog(QWidget *parent = 0);
   ~VHDialog();
   static void WillClose();
   static void WillShow(void *);
   static int VHDialogShowCount(); 
   static void Raise();
   
protected:
   virtual void showEvent(QShowEvent *);
   virtual void hideEvent(QHideEvent *);
   virtual void keyPressEvent(QKeyEvent *);
   virtual void OnSave();
private:
   static int mShowCount;
   static VHDialog *mCurrent;
};
#endif