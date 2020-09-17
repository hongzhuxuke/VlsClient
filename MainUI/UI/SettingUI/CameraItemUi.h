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
   //�豸ID
   void SetCameraDevice(DeviceInfo);

   //����ѡ��
   void SetChecked(bool bSelected);
   //�Ƿ�ѡ�е�ǰ�豸
   bool IsDeviceChecked();

   //����ȫ��ѡ��
   void SetFullChecked(bool bSelected);
   void SetFullScreenShow(const bool& bShow = true);
   //�Ƿ�ѡ��ȫ��
   bool IsFullChecked();

   //����
   void SetDisabled(bool bDisable);

   //���ø߼�����
   void SetSettingDisabled(bool bDisable);

   bool Equal(DeviceInfo);

   void SetItemIndex(int index);
   int GetItemIndex();
   void ShowSwitchButton(bool show);

protected:
   bool eventFilter(QObject *, QEvent *);

private slots:
   //�߼�����
   void OnCameraSetting();

   //�豸ѡ��/��ѡ��
   void OnCameraSelect();

   //�豸ȫ��
   void OnCameraFull();

   //�л������豸
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
