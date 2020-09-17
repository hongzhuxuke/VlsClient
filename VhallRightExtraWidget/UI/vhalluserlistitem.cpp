#include "vhalluserlistitem.h"
#include "ui_vhalluserlistitem.h"
#include <QDebug>
#include "VhallRightExtraWidgetIns.h"
#include "Msg_VhallRightExtraWidget.h"
#include "ICommonData.h"
#include <QJsonObject>

VhallUserListItem::VhallUserListItem(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::VhallUserListItem) ,
    m_gagType(VhallShowType_Allow) ,
    m_kickType(VhallShowType_Allow) {
    
    ui->setupUi(this);
    m_btnGag = new ToolButton(":/extern/18.png",this);
    connect(m_btnGag,SIGNAL(sigClicked()),this,SLOT(GagClicked()));
    m_btnGag->setFocusPolicy(Qt::NoFocus);

    m_btnKick = new ToolButton(":/extern/19.png",this);
    connect(m_btnKick,SIGNAL(sigClicked()),this,SLOT(KickClicked()));
    m_btnKick->setFocusPolicy(Qt::NoFocus);


    ui->exLayout->addWidget(m_btnGag);
    ui->exLayout->addWidget(m_btnKick);

    
    ui->label->setStyleSheet("color:rgb(200,200,200)");
    mbOpera = true;
}

VhallUserListItem::~VhallUserListItem() {
    delete ui;
}
void VhallUserListItem::SetUserInfo(VhallAudienceUserInfo &info,int pageType) {
   //set userName with "maxlength style"
   //QString name_str = QString::fromLocal8Bit("??????????????????????????????");
   QString name_str = info.userName;
   QString name_elide = geteElidedText(ui->label->font(), name_str, 134);
   ui->label->setText(name_elide);
   ui->label->setToolTip(name_str);
   setStyleSheet(
      "QToolTip {\
         border: 0px solid;\
         background: rgb(0,0,0);\
      }"
   );
   userInfo = info;
   ShowGag(VhallShowType_Hide, pageType);
   ShowKick(VhallShowType_Hide, pageType);
   if(info.role==USER_HOST) {
      ui->labelRole->setPixmap(QPixmap(":/extern/20.png"));
   }
   else if(info.role==USER_GUEST) {
      ui->labelRole->setPixmap(QPixmap(":/extern/21.png"));
   }
   else if(info.role==USER_ASSISTANT) {
      ui->labelRole->setPixmap(QPixmap(":/extern/22.png"));
   }
   else if(info.role==USER_USER) {
      ui->labelRole->hide();
      ShowGag(VhallShowType_Allow, pageType);
      ShowKick(VhallShowType_Allow, pageType);
   }
}

void VhallUserListItem::ShowGag(VhallShowType type,int pageType) {  
   if(!mbOpera) {
      return ;
   }

   bool bEnableMemberManage = false;
   int liveType = 0;
   VH::CComPtr<ICommonData> pCommonData = NULL;
   DEF_GET_INTERFACE_PTR(SingletonVhallRightExtraWidgetIns, PID_ICommonToolKit, IID_ICommonData, pCommonData);
   if (pCommonData) {
      liveType = pCommonData->GetLiveType();
      bEnableMemberManage = pCommonData->GetMembersManagerState();
      QString joinUid = pCommonData->GetUserID();
      if (liveType == eLiveType_TcLoginActive && bEnableMemberManage) {
         if (userInfo.role.compare(USER_HOST) != 0 && userInfo.userId.compare(joinUid) != 0) {
            if (pageType == ePageType_ChatForbid) {
               type = VhallShowType_Prohibit;
            } else if (pageType == ePageType_KickOut) {
               type = VhallShowType_Hide;
            }else if(pageType == ePageType_OnLineUser){
               if (type == VhallShowType_Hide) {
                  type = VhallShowType_Allow;
               }
            }
         }
      }
   }

   m_gagType = type;
   switch(m_gagType) {
      case VhallShowType_Allow:
         m_btnGag->show();
         m_btnGag->changeImage(":/extern/18.png");
         break;
      case VhallShowType_Prohibit:
         m_btnGag->show();
         m_btnGag->changeImage(":/extern/32.png");
         break;
      case VhallShowType_Hide:
         m_btnGag->hide();
         break;
      default:
         break;
   } 
}
void VhallUserListItem::ShowKick(VhallShowType type,int pageType) {
   if(!mbOpera) {
      return ;
   }
   bool bEnableMemberManage = false;
   int liveType = 0;
   VH::CComPtr<ICommonData> pCommonData = NULL;
   DEF_GET_INTERFACE_PTR(SingletonVhallRightExtraWidgetIns, PID_ICommonToolKit, IID_ICommonData, pCommonData);
   if (pCommonData) {
      liveType = pCommonData->GetLiveType();
      bEnableMemberManage = pCommonData->GetMembersManagerState();
      QString joinUid = pCommonData->GetUserID();
      if (liveType == eLiveType_TcLoginActive && bEnableMemberManage) {
         if (userInfo.role.compare(USER_HOST) != 0 && userInfo.userId.compare(joinUid) != 0) {
            if (pageType == ePageType_KickOut) {
               type = VhallShowType_Prohibit;
            } else if (pageType == ePageType_ChatForbid) {
               type = VhallShowType_Hide;
            }
            else if (pageType == ePageType_OnLineUser) {
               if (type == VhallShowType_Hide) {
                  type = VhallShowType_Allow;
               }
            }
         }
      }
   }

   m_kickType = type;
   switch(m_kickType) {
      case VhallShowType_Allow:
         m_btnKick->show();
         m_btnKick->changeImage(":/extern/19.png");
         break;
      case VhallShowType_Prohibit:
         m_btnKick->show();
         m_btnKick->changeImage(":/extern/33.png");
         break;
      case VhallShowType_Hide:
         m_btnKick->hide();
         break;
      default:
         break;
   } 

}
//---test---now
void VhallUserListItem::GagClicked() {
   qDebug()<<"VhallUserListItem::GagClicked";
   RQData oData;
	
	QJsonObject body;

   if(m_gagType == VhallShowType_Allow) {//½ûÑÔ
      oData.m_eMsgType=e_RQ_UserProhibitSpeakOneUser;
		body["ac"] = "Gag";
   }
   else {//È¡Ð¦½ûÑÔ
      oData.m_eMsgType=e_RQ_UserAllowSpeakOneUser;
		body["ac"] = "GagCancel";
   }

	
	body["id"] = QString::fromWCharArray(oData.m_oUserInfo.m_szUserID);
	SingletonVhallRightExtraWidgetIns::Instance().reportLog(L"Interaction_GagOrCancle", eLogRePortK_Interaction_GagOrCancle, body);
   
	userInfo.userId.toWCharArray(oData.m_oUserInfo.m_szUserID);
   userInfo.userName.toWCharArray(oData.m_oUserInfo.m_szUserName);
   
   SingletonVhallRightExtraWidgetIns::Instance().PostCRMessage(
      MSG_VHALLRIGHTEXTRAWIDGET_SENDMSG,&oData,sizeof(RQData));
}
void VhallUserListItem::KickClicked() {
	QJsonObject body;

   RQData oData;
   if(m_kickType== VhallShowType_Allow) {
		body["ac"] = "ShotOff";
      oData.m_eMsgType=e_RQ_UserKickOutOneUser;
   }
   else {
		body["ac"] = "ShotOffCancle";
      oData.m_eMsgType=e_RQ_UserAllowJoinOneUser;
   }
   
	body["id"] = QString::fromWCharArray(oData.m_oUserInfo.m_szUserID);
	SingletonVhallRightExtraWidgetIns::Instance().reportLog(L"Interaction_ShotOffOrCancle", eLogRePortK_Interaction_ShotOffOrCancle, body);

   userInfo.userId.toWCharArray(oData.m_oUserInfo.m_szUserID);
   userInfo.userName.toWCharArray(oData.m_oUserInfo.m_szUserName);

   SingletonVhallRightExtraWidgetIns::Instance().PostCRMessage(
      MSG_VHALLRIGHTEXTRAWIDGET_SENDMSG,&oData,sizeof(RQData));
}

void VhallUserListItem::enterEvent(QEvent *e){
   QWidget::enterEvent(e);
   ui->label->setStyleSheet("color:rgb(255,255,255)");
}
void VhallUserListItem::leaveEvent(QEvent *e){
   QWidget::leaveEvent(e);   
   ui->label->setStyleSheet("color:rgb(200,200,200)");
}
bool VhallUserListItem::HasUserInfo(const VhallAudienceUserInfo &info) {
   //qDebug() << "VhallUserListItem::HasUserInfo "<<userInfo.userId<<info.userId;
   return userInfo==info;
}
VhallAudienceUserInfo& VhallUserListItem::GetUserInfo() {
   return userInfo;
}
void VhallUserListItem::SetOperator(bool bOpera,int pageType){
   mbOpera = bOpera;
   bool bEnableMemberManage = false;
   int liveType = 0;
   VhallShowType type = VhallShowType_Allow;
   VH::CComPtr<ICommonData> pCommonData = NULL;
   DEF_GET_INTERFACE_PTR(SingletonVhallRightExtraWidgetIns, PID_ICommonToolKit, IID_ICommonData, pCommonData);
   if (pCommonData) {
      liveType = pCommonData->GetLiveType();
      bEnableMemberManage = pCommonData->GetMembersManagerState();
      QString loginID = pCommonData->GetUserID();
      if (liveType == eLiveType_TcLoginActive && bEnableMemberManage) {
         if (userInfo.role.compare(USER_HOST) != 0 && userInfo.userId.compare(loginID) != 0) {
            if (pageType == ePageType_ChatForbid) {
               m_btnGag->setVisible(true);
               m_btnKick->setVisible(false);
            }
            else if (pageType == ePageType_KickOut) {
               m_btnGag->setVisible(false);
               m_btnKick->setVisible(true);
            }
            else {
               m_btnGag->setVisible(true);
               m_btnKick->setVisible(true);
               mbOpera = true;
            }
         }
         else {
            m_btnGag->setVisible(false);
            m_btnKick->setVisible(false);
         }
      }
      else {
         if (pageType == ePageType_ChatForbid) {
            m_btnGag->setVisible(true);
            m_btnKick->setVisible(false);
         } else if (pageType == ePageType_KickOut) {
            m_btnGag->setVisible(false);
            m_btnKick->setVisible(true);
         } else {
            m_btnGag->setVisible(bOpera);
            m_btnKick->setVisible(bOpera);
         }
      }
   }
}

QString VhallUserListItem::geteElidedText(QFont font, QString str, int MaxWidth)
{
   QFontMetrics fontWidth(font);
   int width = fontWidth.width(str);
   if (width >= MaxWidth)
   {
      str = fontWidth.elidedText(str, Qt::ElideRight, MaxWidth);
   }
   return str;
}

void VhallUserListItem::contextMenuEvent(QContextMenuEvent *) {
   SelectUserInfo user;
   memset(&user,0,sizeof(UserInfo));
   wcscpy(user.userInfo.m_szUserID, this->userInfo.userId.toStdWString().c_str());
   wcscpy(user.userInfo.m_szRole, this->userInfo.role.toStdWString().c_str());
   wcscpy(user.userInfo.m_szUserName, this->userInfo.userName.toStdWString().c_str());
   user.selectType = SELECT_FROM_USER_LIST;
   SingletonVhallRightExtraWidgetIns::Instance().PostCRMessage(
      MSG_VHALLRIGHTEXTRAWIDGET_RIGHT_MOUSE_BUTTON_USER, &user, sizeof(SelectUserInfo));
}

