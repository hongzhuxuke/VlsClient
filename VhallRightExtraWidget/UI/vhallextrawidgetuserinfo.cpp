#include "vhallextrawidgetuserinfo.h"
#include "ui_vhallextrawidgetuserinfo.h"
#include "pub.Const.h"
VhallExtraWidgetUserInfo::VhallExtraWidgetUserInfo(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::VhallExtraWidgetUserInfo) {
    ui->setupUi(this);  
    m_ImageLabel = new VhallNetworkImageLabel(this);
    ui->gridLayoutImage->addWidget(m_ImageLabel);
    connect(this,SIGNAL(SigSetUserImage(const QString &)),this,SLOT(SlotSetUserImage(const QString &)));    
    //connect(this,SIGNAL(SigSetUserName(const QString &)),this,SLOT(SlotSetUserName(const QString &)));
    connect(m_ImageLabel,SIGNAL(SigUserImageFinished(QPixmap &)),this,SIGNAL(SigUserImageFinished(QPixmap &)));
    

	setStyleSheet(TOOLTIPQSS);
    //setStyleSheet("background-color:red;");
}  

VhallExtraWidgetUserInfo::~VhallExtraWidgetUserInfo() {
    delete ui;
}
void VhallExtraWidgetUserInfo::SlotSetUserName(const QString &name) {
   QString name_str = name;
   QString name_elide = geteElidedText(ui->label->font(), name_str, 118);
   ui->label->setText(name_elide);
   ui->label->setToolTip(name_str);
}
//void VhallExtraWidgetUserInfo::SetUserName(const QString &name) {
//   //emit SigSetUserName(name);
//	SlotSetUserName(name);
//}
void VhallExtraWidgetUserInfo::SetUserImage(const QString &img) {   
   emit this->SigSetUserImage(img);
}
void VhallExtraWidgetUserInfo::SlotSetUserImage(const QString &img) {
   //m_ImageLabel->SetImage("http://f.hiphotos.baidu.com/baike/w%3D268%3Bg%3D0/sign=304ad8f708f41bd5da53eff269e1e6f6/d439b6003af33a87a4043fc6c05c10385243b584.jpg");
   m_ImageLabel->SetImage(img);
}

QString VhallExtraWidgetUserInfo::geteElidedText(QFont font, QString str, int MaxWidth)
{
   QFontMetrics fontWidth(font);
   int width = fontWidth.width(str);
   if (width >= MaxWidth)
   {
      str = fontWidth.elidedText(str, Qt::ElideRight, MaxWidth);
   }
   return str;
}

QString VhallExtraWidgetUserInfo::GetNickName()
{
	return ui->label->toolTip().trimmed();
}
