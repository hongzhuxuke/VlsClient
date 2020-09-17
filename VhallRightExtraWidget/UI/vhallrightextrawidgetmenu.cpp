#include "vhallrightextrawidgetmenu.h"
#include "ui_vhallrightextrawidgetmenu.h"
#include <QDebug>
#include "pub.Const.h"
#include <QJsonObject>
#include "VhallRightExtraWidgetIns.h"
#include "Msg_VhallRightExtraWidget.h"
#include "ICommonData.h"

#define BTNSTYLESHEET \
"QToolButton{"\
	"background-color:rgb(28,28,28);"\
	"color:rgb(255,245,245)"\
"}"\
"QToolButton:hover{"\
	"background-color:rgb(255,51,51);"\
"}"\

VhallRightExtraWidgetMenu::VhallRightExtraWidgetMenu(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::VhallRightExtraWidgetMenu) {
    ui->setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint | Qt::Popup);
    m_btnGroup = new QButtonGroup(this);
    connect(m_btnGroup,SIGNAL(buttonClicked(QAbstractButton *)),this,SLOT(BtnClicked(QAbstractButton *)));
}

VhallRightExtraWidgetMenu::~VhallRightExtraWidgetMenu() {
    delete ui;
}


void VhallRightExtraWidgetMenu::BtnClicked(QAbstractButton *b){
   if(!b) {
      return ;
   }

	QJsonObject body;
	body["ac"] = b->text();
	SingletonVhallRightExtraWidgetIns::Instance().reportLog(L"RightExtraChoice",eLogRePortK_RightExtraChoice, body);

   emit this->SigBtnClicked(b->text());
   hide();
}

int VhallRightExtraWidgetMenu::Refresh(QStringList &names){
   for (int i = 0 ; i < m_btns.count(); i++) {
      ui->layoutMenu->removeWidget(m_btns[i]);
      m_btnGroup->removeButton(m_btns[i]);
      delete m_btns[i];
   }

   m_btns.clear();
   int height = 0;
   for (int i=0 ; i < names.count(); i++) {
      //针对全局设置，如果是没有公告权限，隐藏公告按键。
      bool bWebinar = true;
      VH::CComPtr<ICommonData> pCommonData = NULL;
      DEF_GET_INTERFACE_PTR(SingletonVhallRightExtraWidgetIns, PID_ICommonToolKit, IID_ICommonData, pCommonData);
      if (pCommonData) {
         bWebinar = pCommonData->GetWebinarNoticeState();
         if (!bWebinar && names[i].compare(WEBINAR_NOTICE) == 0) {
            if (names.count() == 1) {
               break;
            } else {
               continue;
            }
         }
      }
      QToolButton *b=new QToolButton();
      b->setFixedSize(104,30);
      b->setText(names[i]);
      b->setStyleSheet(BTNSTYLESHEET);
      b->setAutoRaise(true);
      m_btnGroup->addButton(b);
      ui->layoutMenu->addWidget(b);
      m_btns.append(b);
      height+=30;
   }
   
   this->resize(this->width(),height);
   return height;
}
