#include "vhdesktopcameralistitem.h"
#include "ui_vhdesktopcameralistitem.h"
#include <QPainter>
#include <QDebug>

#include "MainUIIns.h"
#include "IOBSControlLogic.h"
#include "Msg_OBSControl.h"
#include "CRPluginDef.h"

VHDesktopCameraListItem::VHDesktopCameraListItem(QWidget *parent) :
    QWidget(parent),
    itemIndex(-1),
    ui(new Ui::VHDesktopCameraListItem) {
    ui->setupUi(this);
    ui->widget->installEventFilter(this);
    ui->checkBox->installEventFilter(this);
    connect(ui->checkBox,SIGNAL(toggled(bool)),this,SLOT(checkBox_toggled(bool)));
}

VHDesktopCameraListItem::~VHDesktopCameraListItem() {
    delete ui;
}
void VHDesktopCameraListItem::SetData(QString str, DeviceInfo dInfo, int index) {
    this->txt=str;
    this->deviceInfo = dInfo;
    this->itemIndex = index;
}
void VHDesktopCameraListItem::paintEvent(QPaintEvent *){
   
    QPainter p(this);
    QPen pen;
    
    if(this->isEnabled()) {
      pen.setColor(QColor(150,150,150));
    }
    else {
       pen.setColor(QColor(80,80,80));
    }
    
    p.setPen(pen);
    p.drawText(20,9,this->width()-10,this->height(),0,this->txt);
}
bool VHDesktopCameraListItem::eventFilter(QObject *o, QEvent *e){
    if(o==ui->widget) {
        if(e->type()==QEvent::MouseButtonRelease) {
            if(this->isEnabled()) {
               ui->checkBox->setChecked(!ui->checkBox->isChecked());               
               //Toggle(true);
            }
        }
    }
    else if(o==ui->checkBox) {
       if(e->type()==QEvent::MouseButtonRelease) {         
         if(this->isEnabled()) {
            //Toggle(false);
         }
       }
    }
    
    return QWidget::eventFilter(o,e);
}
bool VHDesktopCameraListItem::SetChecked(DeviceInfo &dInfo){
   bool ret = false;
   disconnect(ui->checkBox,SIGNAL(toggled(bool)),this,SLOT(checkBox_toggled(bool)));
   if(this->deviceInfo==dInfo) {
      ui->checkBox->setChecked(true);
      ret = true;
   }
   connect(ui->checkBox,SIGNAL(toggled(bool)),this,SLOT(checkBox_toggled(bool)));
   return ret;
}

void VHDesktopCameraListItem::SetChecked(bool bCheck){
   disconnect(ui->checkBox, SIGNAL(toggled(bool)), this, SLOT(checkBox_toggled(bool)));
   qDebug() << "VHDesktopCameraListItem::SetChecked" << bCheck << " " << ui->checkBox->isChecked();
   ui->checkBox->setChecked(false);
   connect(ui->checkBox, SIGNAL(toggled(bool)), this, SLOT(checkBox_toggled(bool)));
   Toggle(true);
}

bool VHDesktopCameraListItem::IsChecked(){
   return ui->checkBox->isChecked();
}
void VHDesktopCameraListItem::SetEnabled(bool ok){
   this->setEnabled(ok);
}
void VHDesktopCameraListItem::checkBox_toggled(bool checked){
   qDebug()<<"VHDesktopCameraListItem::on_checkBox_toggled"<<checked<<ui->checkBox->isChecked();
   if(m_bToToggle){
      Toggle(true);
   }
   else{
      emit this->SigClicked(checked,deviceInfo);
   }
}
void VHDesktopCameraListItem::Toggle(bool ok){
   VH::CComPtr<ISettingLogic> pSettingLogic;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IMainUI, IID_ISettingLogic, pSettingLogic, return);

   STRU_OBSCONTROL_ADDCAMERA loAddCamera;
   loAddCamera.m_deviceInfo = deviceInfo;

   bool done = ui->checkBox->isChecked();
   if(!ok){
      done = !done;
   }
	QJsonObject body;
   if(done) {
      //添加
      loAddCamera.m_dwType = device_operator_add;
      loAddCamera.m_renderHwnd = (HWND)pSettingLogic->LockVideo(QString::fromStdWString(loAddCamera.m_deviceInfo.m_sDeviceID),itemIndex);
      loAddCamera.m_PosType = emum_PosType_rightDown;

      if(loAddCamera.m_renderHwnd==NULL) {
         ui->checkBox->setChecked(false);
         return ;
      }

		body["vdn"] = QString::fromStdWString(loAddCamera.m_deviceInfo.m_sDeviceDisPlayName);		//摄像设备用于显示的名字
		body["vn"] = QString::fromStdWString(loAddCamera.m_deviceInfo.m_sDeviceName);//摄像设备内部名
		body["vid"] = QString::fromStdWString(loAddCamera.m_deviceInfo.m_sDeviceID);//设备的Id
		body["vt"] = loAddCamera.m_deviceInfo.m_sDeviceType;//设备的类型
		body["vb"] = QString("Open");//开启/关闭操作
		

      VH::CComPtr<IOBSControlLogic> pObsControlLogic = NULL;
      DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IOBSControl, IID_IOBSControlLogic, pObsControlLogic, return);
      pObsControlLogic->DealAddCameraSync(&loAddCamera,sizeof(STRU_OBSCONTROL_ADDCAMERA));

      //VH::CComPtr<ISettingLogic> pSettingLogic = NULL;
      //DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IMainUI, IID_ISettingLogic, pSettingLogic, return);
      //pSettingLogic->DeviceAdd(&loAddCamera,sizeof(STRU_OBSCONTROL_ADDCAMERA));
   }
   else {
      //删除
      loAddCamera.m_dwType = device_operator_del;
		body["vdn"] = QString::fromStdWString(loAddCamera.m_deviceInfo.m_sDeviceDisPlayName);		//摄像设备用于显示的名字
		body["vn"] = QString::fromStdWString(loAddCamera.m_deviceInfo.m_sDeviceName);//摄像设备内部名
		body["vid"] = QString::fromStdWString(loAddCamera.m_deviceInfo.m_sDeviceID);//设备的Id
		body["vt"] = loAddCamera.m_deviceInfo.m_sDeviceType;//设备的类型
		body["vb"] = QString("Close");//开启/关闭操作

      VH::CComPtr<IOBSControlLogic> pObsControlLogic = NULL;
      DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IOBSControl, IID_IOBSControlLogic, pObsControlLogic, return);
      pObsControlLogic->DealAddCameraSync(&loAddCamera,sizeof(STRU_OBSCONTROL_ADDCAMERA));

      //VH::CComPtr<ISettingLogic> pSettingLogic;
      //DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IMainUI, IID_ISettingLogic, pSettingLogic, return);
      //pSettingLogic->DeviceAdd(&loAddCamera,sizeof(STRU_OBSCONTROL_ADDCAMERA));
   }

	SingletonMainUIIns::Instance().reportLog(L"desktopshare_camera", eLogRePortK_DesktopShare_Camera, body);
}

QString VHDesktopCameraListItem::GetDeviceID() {
   return QString::fromStdWString(this->deviceInfo.m_sDeviceID);
}

DeviceInfo* VHDesktopCameraListItem::GetDeviceInfo()
{
	return &deviceInfo;
}

