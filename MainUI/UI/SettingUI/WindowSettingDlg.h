#ifndef ADDSOFTWARE_DIALOG_H
#define ADDSOFTWARE_DIALOG_H

#include "ui_WindowSettingUI.h"
#include "VHDialog.h"
#include "ToolButton.h"
class TitleWidget;
class QHBoxLayout;
class WindowSettingDlg : public VHDialog {
   Q_OBJECT
public:
   explicit WindowSettingDlg(QDialog *parent = 0);
   ~WindowSettingDlg();

   bool Create();
   void Destroy();

   void AddDeviceItem(wchar_t* wzDeviceName, QVariant dwUserData);

   bool GetCurItem(QVariant& UserData);

   //清空软件源列表
   void Clear();

   void CenterWindow(QWidget* parent);

   bool GetComptible();
private slots:
   void OnSave();
   void slotReFlush();
//protected:
//   void paintEvent(QPaintEvent *);

private:
   Ui::WindowSettingUI ui;
   
private:
   TitleWidget* m_pTitleBar;
   QHBoxLayout* m_pMainLayout;
   ToolButton * m_pFlushBtn = NULL;
};

#endif // ADDSOFTWARE_DIALOG_H
