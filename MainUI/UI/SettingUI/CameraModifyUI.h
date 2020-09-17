#ifndef CameraModifyUi_H
#define CameraModifyUi_H
#include "ui_CameraModifyUI.h"
#include <QDialog>
#include <string>
#include "VHDialog.h"
#include "VH_ConstDeff.h"
using namespace std;

class TitleWidget;
class CameraModifyUI : public VHDialog {
   Q_OBJECT

public:
   CameraModifyUI(QDialog *parent = 0);
   ~CameraModifyUI();

   bool Create();
   void Destroy();
   void BindDevice(DeviceInfo);

   void AddItem(DeviceInfo);
   void Clear();
   void RemoveDeviceInfo(DeviceInfo &);
   //设备号
   void SetCameraNum(QString qsNum);
   void SetPosType(DataSourcePosType);
   void CenterWindow(QWidget* parent);

   //禁用高级设置
   void SetSettingDisabled(bool bDisable);
private:
    bool GetItemInfo(int,DeviceInfo&);
private slots:
   void OnSave();
   void OnClose();
   //高级设置
   void OnCameraSetting();

   //切换设备
   void OnChgDevice(int nIndex);

protected:
   void paintEvent(QPaintEvent *);

private:
   Ui::CameraModifyUI ui;
   TitleWidget *mTitleBar;
   DeviceInfo m_Device;
   DataSourcePosType m_posType;
};

#endif // CameraModifyUi_H
