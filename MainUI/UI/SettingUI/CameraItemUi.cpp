#include "stdafx.h"
#include "CameraItemUi.h"
#include "MainUIIns.h"
#include "ToolButton.h"

#include "Msg_MainUI.h"
#include "IDShowPlugin.h"
#include "ICommonData.h"
#include "IOBSControlLogic.h"

#include <QDebug>

#define CHECK_ICON "QCheckBox {font:12px bold; color:rgb(153, 153, 153);}\
            QCheckBox:disabled{font:12px bold; color:rgb(142, 142, 142); }   \
            QCheckBox::indicator{width:15px; height:15px;}\
            QCheckBox::indicator:checked{border-image:url(:/sysButton/checkbox) 0 30 0 15; } \
            QCheckBox::indicator:unchecked{border-image:url(:/sysButton/checkbox) 0 45 0 0; }\
            QCheckBox::indicator:checked:disabled{border-image:url(:/sysButton/checkbox) 0 0 0 45; }\
            QCheckBox::indicator:unchecked:disabled{border-image:url(:/sysButton/checkbox) 0 15 0 30; }"

#define CHECK_HIDE_ICON "QCheckBox {font:12px bold; color:rgb(153, 153, 153);}\
            QCheckBox:disabled{font:12px bold; color:rgb(142, 142, 142); }   \
            QCheckBox::indicator{width:0px;}\
            QCheckBox::indicator:checked{border-image:url(:/sysButton/checkbox) 0 30 0 15; } \
            QCheckBox::indicator:unchecked{border-image:url(:/sysButton/checkbox) 0 45 0 0; }\
            QCheckBox::indicator:checked:disabled{border-image:url(:/sysButton/checkbox) 0 0 0 45; }\
            QCheckBox::indicator:unchecked:disabled{border-image:url(:/sysButton/checkbox) 0 15 0 30; }"



#define SWITCH_OPEN  "border-image: url(:/button/images/kaiqi.png);"
#define SWITCH_CLOSE "border-image: url(:/button/images/guanbi.png);"


CameraItem::CameraItem(QWidget *parent /*= 0*/) 
	: CWidget(parent)
	,itemIndex(-1){
   ui.setupUi(this);

   m_pBtnSetting = new ToolButton(":/sysButton/settingBtn", this);

   QHBoxLayout* pLayout = (QHBoxLayout*)layout();
   if (pLayout) {
      pLayout->addWidget(m_pBtnSetting, 0, Qt::AlignLeft);
   }
   connect(m_pBtnSetting, SIGNAL(sigClicked()), this, SLOT(OnCameraSetting()));
   connect(ui.checkBox_camera, SIGNAL(clicked()), this, SLOT(OnCameraSelect()));
   connect(ui.checkBox_fullScreen, SIGNAL(clicked()), this, SLOT(OnCameraFull()));
   connect(ui.pushButton_hideRender, SIGNAL(clicked()), this, SLOT(OnSwitchCamera()));

   ui.checkBox_camera->installEventFilter(this);
   ui.checkBox_fullScreen->installEventFilter(this);
   m_pBtnSetting->installEventFilter(this);
  
   ui.pushButton_hideRender->hide();
   mSwitchClickedTimer = new QTimer(this);
   if (mSwitchClickedTimer) {
      mSwitchClickedTimer->setSingleShot(true);
      connect(mSwitchClickedTimer, SIGNAL(timeout()), this, SLOT(OnSwitchCamerClicked()));
   }
}

CameraItem::~CameraItem() {

}

bool CameraItem::Create() {
   return true;
}

void CameraItem::Destroy() {
}

void CameraItem::setSettingShow(const bool& bShow /*= true*/)
{
	if (NULL == m_pBtnSetting) return;
	bShow ? m_pBtnSetting->show() : m_pBtnSetting->hide();
}

void CameraItem::SetCameraDevice(DeviceInfo device) {
	QString str = QString::fromStdWString(device.m_sDeviceDisPlayName);
	this->setToolTip(str);

	if (str.length() > 23)
	{
		str = str.mid(0, 20); 
		str += "...";
	}
	else
	{

	}
   ui.checkBox_camera->setText(str);
   int num = GetDeviceDisplayNumber(device);
   m_Device = device;
}

void CameraItem::SetChecked(bool bSelected) {
   ui.checkBox_camera->setChecked(bSelected);
   if (bSelected) {
      ui.pushButton_hideRender->setStyleSheet(SWITCH_OPEN);
   }
   else {
      ui.pushButton_hideRender->setStyleSheet(SWITCH_CLOSE);
   }
}

bool CameraItem::IsDeviceChecked() {
   return ui.checkBox_camera->isChecked();
}

void CameraItem::SetFullChecked(bool bSelected) {
   ui.checkBox_fullScreen->setChecked(bSelected);
}

void CameraItem::SetFullScreenShow(const bool& bShow/* = true*/)
{
   bShow ? ui.checkBox_fullScreen->show() : ui.checkBox_fullScreen->hide();
   if (bShow) {
      //显示全屏
      ui.pushButton_hideRender->hide();
      ui.checkBox_camera->setStyleSheet(CHECK_ICON);
   }
   else {
      ui.pushButton_hideRender->show();
      ui.checkBox_camera->setStyleSheet(CHECK_HIDE_ICON);
   }
}

bool CameraItem::IsFullChecked() {
   return ui.checkBox_fullScreen->isChecked();
}

void CameraItem::OnSwitchCamerClicked(){
   ui.pushButton_hideRender->setEnabled(true);
}

//配置摄像头
void CameraItem::OnCameraSetting() {
   STRU_MAINUI_CAMERACTRL loCameraCtrl;
   loCameraCtrl.m_Device = m_Device;
   SingletonMainUIIns::Instance().PostCRMessage(
      MSG_MAINUI_CAMERA_SETTING, &loCameraCtrl, sizeof(loCameraCtrl));
   
   //隐藏摄像设备管理窗口
   QWidget* pParent = (QWidget*)parent();
   if (pParent) {
      pParent->hide();
   }

   VH::CComPtr<ICommonData> pCommonData;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
   pCommonData->ReportEvent("sxsb04");
}

void CameraItem::OnSwitchCamera(){
   if (ui.checkBox_camera->isChecked()) {
      ui.checkBox_camera->setChecked(false);
      qDebug() << "close camera";
   }
   else {
      ui.checkBox_camera->setChecked(true);
      qDebug() << "open camera";
   }
   ui.pushButton_hideRender->setEnabled(false);
   if (mSwitchClickedTimer) {
      mSwitchClickedTimer->start(1000);
   }
   OnCameraSelect();
}

//选择摄像头，用于在渲染区添加或减少
void CameraItem::OnCameraSelect() {
   VH::CComPtr<IOBSControlLogic> pObsControlLogic = NULL;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IOBSControl, IID_IOBSControlLogic, pObsControlLogic, return, ASSERT(FALSE));

   if(pObsControlLogic->GetGraphicsDeviceInfoCount() >=2 && ui.checkBox_camera->isChecked()) {
      qDebug()<<"CameraItem::OnCameraSelect() GetGraphicsDeviceInfoCount>=2 return "<<pObsControlLogic->GetGraphicsDeviceInfoCount();
      return ;
   }

   VH::CComPtr<ISettingLogic> pSettingLogic;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IMainUI, IID_ISettingLogic, pSettingLogic, return);
   
   
   STRU_MAINUI_CHECKSTATUS loCameraCtrl;
   if (ui.checkBox_camera->isChecked()) {
      loCameraCtrl.status = checkbox_status_checked;
      ui.pushButton_hideRender->setStyleSheet(SWITCH_OPEN);
      qDebug() << "SWITCH_OPEN";
   } else {
      //取消时，同步取消“全屏”框
      ui.checkBox_fullScreen->setChecked(false);
      loCameraCtrl.status = checkbox_status_unchecked;
      ui.pushButton_hideRender->setStyleSheet(SWITCH_CLOSE);
      qDebug() << "SWITCH_CLOSE";
   }
   ui.pushButton_hideRender->repaint();

	QJsonObject body;
	body["vdn"] = QString::fromWCharArray(m_Device.m_sDeviceDisPlayName);
	body["vn"] = QString::fromWCharArray(m_Device.m_sDeviceName);
	body["vid"] = QString::fromWCharArray(m_Device.m_sDeviceID);
	body["vt"] = m_Device.m_sDeviceType;
	body["vb"] = QString(ui.checkBox_camera->isChecked()?"Open":"Close");
	body["ac"] = (IsFullChecked() ? "Max" : "Normal");
	SingletonMainUIIns::Instance().reportLog(STR_VH_LIVE_OPERATE, eLogRePortK_Camera_State, L"Camera_State", body);
   loCameraCtrl.m_deviceInfo = m_Device;
   pSettingLogic->DealCameraSelect(&loCameraCtrl, sizeof(STRU_MAINUI_CHECKSTATUS));
}

void CameraItem::SetDisabled(bool bDisable) {
   //ui.pushButton_hideRender->setEnabled(bDisable);
   ui.checkBox_camera->setDisabled(bDisable);
   ui.checkBox_fullScreen->setDisabled(bDisable);
   //ui.label_device->setDisabled(bDisable);
   m_pBtnSetting->setDisabled(bDisable);
   this->setDisabled(bDisable);
   if (ui.pushButton_hideRender->isHidden()) {
      if (bDisable) {
         ui.checkBox_camera->setStyleSheet(CHECK_ICON);
         ui.checkBox_fullScreen->setStyleSheet(CHECK_ICON);
      }
      else {
         ui.checkBox_camera->setStyleSheet(CHECK_ICON);
         ui.checkBox_fullScreen->setStyleSheet(CHECK_ICON);
      }
   }
   else {
      if (bDisable) {
         ui.checkBox_camera->setStyleSheet(CHECK_HIDE_ICON);
         ui.checkBox_fullScreen->setStyleSheet(CHECK_ICON);
      }
      else {
         ui.checkBox_camera->setStyleSheet(CHECK_HIDE_ICON);
         ui.checkBox_fullScreen->setStyleSheet(CHECK_ICON);
      }
   }
}

void CameraItem::SetSettingDisabled(bool bDisable) {
   m_pBtnSetting->setDisabled(bDisable);
}
bool CameraItem::Equal(DeviceInfo deviceInfo) {
   return this->m_Device == deviceInfo;
}

void CameraItem::OnCameraFull() {
   STRU_MAINUI_DEVICE_FULL loData;
   loData.m_deviceInfo = m_Device;
   loData.posType = IsFullChecked() ? enum_PosType_fullScreen : emum_PosType_rightDown;

	QJsonObject body;
	body["vdn"] = QString::fromWCharArray(m_Device.m_sDeviceDisPlayName);
	body["vn"] = QString::fromWCharArray(m_Device.m_sDeviceName);
	body["vid"] = QString::fromWCharArray(m_Device.m_sDeviceID);
	body["vt"] = m_Device.m_sDeviceType;
	body["ac"] = (IsFullChecked()?"Max":"Normal") ;
	SingletonMainUIIns::Instance().reportLog(STR_VH_LIVE_OPERATE, eLogRePortK_Camera_FullScreen, L"FullScreen", body);

   SingletonMainUIIns::Instance().PostCRMessage(
      MSG_MAINUI_CAMERA_FULL, &loData, sizeof(STRU_MAINUI_DEVICE_FULL));
}

void CameraItem::SetItemIndex(int index) {
   this->itemIndex = index;
}

int CameraItem::GetItemIndex() {
   return itemIndex;
}

void CameraItem::ShowSwitchButton(bool show) {
   if (show) {
      ui.pushButton_hideRender->show();
   }
   else {
      ui.pushButton_hideRender->hide();
   }
}

bool CameraItem::eventFilter(QObject *o, QEvent *e)
{
	if (e->type() == QEvent::Enter) {
		QString strQss = TOOLTIPQSS;
		strQss += "background-color:rgba(44,44,44,255);";
		setStyleSheet(strQss);
		//setStyleSheet("background-color:rgba(0,255,00,255);");
	}
	else if (e->type() == QEvent::Leave) {
		QString strQss = TOOLTIPQSS;
		strQss += "background-color:rgba(67,67,67,255);";
		setStyleSheet(strQss);
	}
	else if (e->type() == QEvent::MouseButtonPress) {

	}
	else if (e->type() == QEvent::MouseButtonRelease)
	{
	}

	return QWidget::eventFilter(o, e);

}