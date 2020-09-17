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
   //�豸��
   void SetCameraNum(QString qsNum);
   void SetPosType(DataSourcePosType);
   void CenterWindow(QWidget* parent);

   //���ø߼�����
   void SetSettingDisabled(bool bDisable);
private:
    bool GetItemInfo(int,DeviceInfo&);
private slots:
   void OnSave();
   void OnClose();
   //�߼�����
   void OnCameraSetting();

   //�л��豸
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
