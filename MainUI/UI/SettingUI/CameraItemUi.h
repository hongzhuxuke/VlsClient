#ifndef CameraItem_H
#define CameraItem_H
#include "ui_CameraItemUi.h"
#include "VH_ConstDeff.h"
#include "CWidget.h"

class ToolButton;
class CameraItem : public CWidget {
   Q_OBJECT

public:
   CameraItem(QWidget *parent = 0);
   ~CameraItem();

   bool Create();
   void Destroy();
   void setSettingShow(const bool& bShow = true);
   //设备ID
   void SetCameraDevice(DeviceInfo);

   //设置选中
   void SetChecked(bool bSelected);
   //是否选中当前设备
   bool IsDeviceChecked();

   //设置全屏选中
   void SetFullChecked(bool bSelected);
   void SetFullScreenShow(const bool& bShow = true);
   //是否选中全屏
   bool IsFullChecked();

   //禁用
   void SetDisabled(bool bDisable);

   //禁用高级设置
   void SetSettingDisabled(bool bDisable);

   bool Equal(DeviceInfo);

   void SetItemIndex(int index);
   int GetItemIndex();
   void ShowSwitchButton(bool show);

protected:
   bool eventFilter(QObject *, QEvent *);

private slots:
   //高级设置
   void OnCameraSetting();

   //设备选中/不选中
   void OnCameraSelect();

   //设备全屏
   void OnCameraFull();

   //切换开关设备
   void OnSwitchCamera();

   void OnSwitchCamerClicked();
private:
   Ui::CameraItem ui;
   DeviceInfo m_Device;
   ToolButton* m_pBtnSetting = NULL;
   int itemIndex;
   QTimer *mSwitchClickedTimer = nullptr;
};

#endif // CameraItem_H
