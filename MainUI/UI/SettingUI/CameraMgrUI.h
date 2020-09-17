#ifndef CameraMgrUi_H
#define CameraMgrUi_H
#include "ui_CameraMgrUI.h"
#include <QDialog>
#include "VHDialog.h"
#include "VH_ConstDeff.h"
#include "CameraItemUi.h"
class TitleWidget;
class QLabel;
class CameraMgrUI : public VHDialog {
   Q_OBJECT

public:
   CameraMgrUI(QDialog *parent = 0);
   ~CameraMgrUI();

   bool Create();
   void Destroy();
   void setSettingShow(const bool& bShow = true)
   {
	   mbShowSetting = bShow;
   }
   
   void AddItem(CameraItem* item);
   void ClearItem();
   CameraItem *FindItem(DeviceInfo);
   void SetFullScreenShow(const bool& bShow = true);
   //Œﬁ…Ë±∏
   void ShowNoDeivice(bool bShow);

   void CenterWindow(QWidget* parent);
   
   void SetItemIsFullScreen(DeviceInfo info,bool isFullScreen);

   void RefreshItemStatus(enum_checkbox_status status);

   void RefreshItemStatus();

   void SetItemUnchecked();

   void SelectedItemSetFullScreen();

   int SelectedItemCount();

private slots:
   void OnClose();

protected:
   void paintEvent(QPaintEvent *);
   void showEvent(QShowEvent *);
   void hideEvent(QHideEvent *);
   virtual void focusOutEvent(QFocusEvent *);
   virtual void enterEvent(QEvent *);
   virtual void leaveEvent(QEvent *);

private:
   void UnSelectedItemEnable(bool);
private:
   Ui::CameraMgrUI ui;
   TitleWidget *mTitleBar = NULL;
   QLabel* m_pNoDeviceText = NULL;
   std::vector<CameraItem*> m_oItemList;
   QPixmap mPixmap;

   int m_iHeight;
   bool m_bShow = false;
   bool m_bEnter = false;
   bool mbShowSetting = true; 
};

#endif // CameraMgrUi_H
