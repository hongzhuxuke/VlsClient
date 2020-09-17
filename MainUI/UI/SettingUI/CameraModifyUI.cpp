#include "stdafx.h"
#include "CameraModifyUI.h"
#include "MainUIIns.h"
#include "TitleWidget.h"
#include <QPainter>
#include <QDesktopWidget>
#include "ToolButton.h"
#include "Msg_MainUI.h"

CameraModifyUI::CameraModifyUI(QDialog *parent /*= 0*/)
: VHDialog(parent) 
, mTitleBar(NULL){
   ui.setupUi(this);
   setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
   setAttribute(Qt::WA_TranslucentBackground);
   setAutoFillBackground(true);
   connect(ui.saveBtn, SIGNAL(clicked()), this, SLOT(OnSave()));
   connect(ui.comboBox_deviceName, SIGNAL(currentIndexChanged(int)), this, SLOT(OnChgDevice(int)));
}

CameraModifyUI::~CameraModifyUI() {

}

bool CameraModifyUI::Create() {
   mTitleBar = new TitleWidget(tr("修改摄像设备"), NULL);

   QVBoxLayout* pLayout = ui.verticalLayout_Title;
   if (pLayout) {
      pLayout->addWidget(mTitleBar, 0, Qt::AlignTop);
   }
   connect(mTitleBar, SIGNAL(closeWidget()), this, SLOT(OnClose()));

   ToolButton *settingBtn = new ToolButton(":/sysButton/settingBtn", this);
   connect(settingBtn, SIGNAL(sigClicked()), this, SLOT(OnCameraSetting()));
   ui.settingLayout->addWidget(settingBtn);
   return true;
}

void CameraModifyUI::Destroy() {

}
bool CameraModifyUI::GetItemInfo(int index,DeviceInfo &dInfo)
{
   if(index<0||index>=ui.comboBox_deviceName->count())
   {
      return false;
   }
   QVariant qItemData=ui.comboBox_deviceName->itemData(index);
   QString str= qItemData.toString();
   QByteArray ba = QByteArray::fromBase64(str.toLocal8Bit());
   dInfo = *(DeviceInfo *)ba.data();
   return true;
}

void CameraModifyUI::BindDevice(DeviceInfo info)
{
   m_Device=info;
   for (int i = 0; i<ui.comboBox_deviceName->count();i++) {
      DeviceInfo dInfo;
      if(GetItemInfo(i,dInfo))
      {
         if (dInfo==info)  {
            ui.comboBox_deviceName->setCurrentIndex(i);
            break;
         }
      }
   }
}
void CameraModifyUI::RemoveDeviceInfo(DeviceInfo &info)
{
   for (int i = 0; i<ui.comboBox_deviceName->count();i++) {
      DeviceInfo dInfo;
      if(GetItemInfo(i,dInfo))
      {
         if (dInfo==info)  {
            ui.comboBox_deviceName->removeItem(i);
            break;
         }
      }
   }
}


void CameraModifyUI::AddItem(DeviceInfo deviceInfo) {
   QVariant qItemData;
   QByteArray ba;
   ba.setRawData((const char *)&deviceInfo,sizeof(DeviceInfo));
   QString str = QString::fromLocal8Bit(ba.toBase64());
   qItemData.setValue(str);   
   ui.comboBox_deviceName->addItem(QString::fromWCharArray(deviceInfo.m_sDeviceDisPlayName),qItemData);
}

void CameraModifyUI::Clear() {
   ui.comboBox_deviceName->clear();
}

void CameraModifyUI::OnSave() {
   DeviceInfo desDevice;
   if(!GetItemInfo(ui.comboBox_deviceName->currentIndex(),desDevice))
   {
      hide();
      return;
   }
   
   STRU_MAINUI_DEVICE_MODIFY loMessage;
   loMessage.srcDevice=m_Device;
   loMessage.desDevice=desDevice;
   loMessage.posType= ui.checkBox_fullScreen->isChecked()?enum_PosType_fullScreen:m_posType==enum_PosType_fullScreen?enum_PosType_auto:m_posType;
   SingletonMainUIIns::Instance().PostCRMessage(
      MSG_MAINUI_MODIFY_SAVE, &loMessage,sizeof(STRU_MAINUI_DEVICE_MODIFY));

   hide();
}

void CameraModifyUI::OnClose() {
    hide();
}

void CameraModifyUI::paintEvent(QPaintEvent *) {
   QPainter painter(this);
   painter.setRenderHint(QPainter::Antialiasing, true);
   painter.setPen(QPen(QColor(54, 54, 54), 1));
   painter.setBrush(QColor(38, 38, 38));
   painter.drawRoundedRect(rect(), 4.0, 4.0);
}

void CameraModifyUI::OnCameraSetting() {
   STRU_MAINUI_CAMERACTRL loCameraCtrl;
   loCameraCtrl.m_Device=m_Device;
   SingletonMainUIIns::Instance().PostCRMessage(
      MSG_MAINUI_CAMERA_SETTING, &loCameraCtrl, sizeof(loCameraCtrl));
}

void CameraModifyUI::SetCameraNum(QString qsNum) {
   ui.label_deviceNumber->setText(qsNum);
}

void CameraModifyUI::OnChgDevice(int nIndex) {
   SetCameraNum(QString::fromStdWString(L"设备")
                + QString::number(nIndex + 1)
      + QString::fromStdWString(L":")
   );
}

void CameraModifyUI::CenterWindow(QWidget* parent) {
   int x = 0;
   int y = 0;
   if (NULL == parent) {
      const QRect rect = QApplication::desktop()->availableGeometry();
      x = rect.left() + (rect.width() - width()) / 2;
      y = rect.top() + (rect.height() - height()) / 2;
   } else {
      QPoint point(0, 0);
      point = parent->mapToGlobal(point);
      x = point.x() + (parent->width() - width()) / 2;
      y = point.y() + (parent->height() - height()) / 2;
   }
   move(x, y);
}

void CameraModifyUI::SetPosType(DataSourcePosType posType) {
   m_posType=posType;
   ui.checkBox_fullScreen->setChecked(m_posType==enum_PosType_fullScreen);
}

void CameraModifyUI::SetSettingDisabled(bool bDisable) {
   
}

