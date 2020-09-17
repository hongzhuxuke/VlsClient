#include "vhallspeakerlist.h"
#include "ui_vhallspeakerlist.h"

#include "CRPluginDef.h"
#include "MainUIIns.h"
#include "Msg_VhallRightExtraWidget.h"
//#include "IVhallRightExtraWidgetLogic.h"
#include "DebugTrace.h"
#include "pathmanager.h"
#include "ConfigSetting.h"


#include <QListView>
#include <QDebug>
#include <QListWidgetItem>

VhallSpeakerList::VhallSpeakerList(QWidget *parent) :
QWidget(parent),
ui(new Ui::VhallSpeakerList) {
   ui->setupUi(this);

   //connect(ui->pushButtonJoin, SIGNAL(clicked()), this, SLOT(Slot_JoinActivity()));
   //connect(ui->pushButtonCancelJoin, SIGNAL(clicked()), this, SLOT(Slot_CancelJoinActivity()));

   ///ui->pushButtonCancelJoin->hide();
   //ui->pushButtonJoin->hide();

   m_pTimer = new QTimer(this);
   if (m_pTimer != NULL) {
      connect(m_pTimer, SIGNAL(timeout()), this, SLOT(Slot_ButtonDisableTimeOut()));
   }

   //ui->comboBox->setView(new QListView());
   //qDebug()<<"VhallSpeakerList::VhallSpeakerList "<<ui->comboBox->size();

   this->m_pSpeakerButton = new QToolButton();
   this->m_pSpeakerButton->setMinimumSize(67, 22);
   this->m_pSpeakerButton->setMaximumSize(67, 22);

   this->m_pSpeakerButton->setStyleSheet(
      "QToolButton{border-image: url(\":/interactivity/toSpeake\");}"
      "QToolButton:hover{border-image: url(\":/interactivity/toSpeakeHover\");}"
      "QToolButton:pressed {border-image: url(\":/interactivity/toSpeake\");}"
      );
   ui->listWidget->setStyleSheet("QAbstractItemView::item{ border:0px;}");
   connect(m_pSpeakerButton, SIGNAL(clicked()), this, SLOT(Slot_SpeakerButtonClicked()));
   m_pSpeakerButton->setEnabled(false);
   this->m_pSpeakerButton->setStyleSheet("QToolButton{border-image: url(\": /interactivity/img/interactivity/to_speake_disable.png\");}");
}

VhallSpeakerList::~VhallSpeakerList() {
   delete ui;
   if (m_pSpeakerButton) {
      delete m_pSpeakerButton;
      m_pSpeakerButton = NULL;
   }
}

void VhallSpeakerList::SetRole(bool Master) {
   if (Master) {
      this->m_pSpeakerButton->hide();
   } else {
      this->m_pSpeakerButton->show();
   }
   m_bMaster = Master;

   QString toolConfPath = CPathManager::GetAppDataPath() + QString::fromStdWString(VHALL_TOOL_CONFIG);
   int hideMemberList = ConfigSetting::ReadInt(toolConfPath, GROUP_DEFAULT, KEY_VHALL_HIDE_MEMBER_LIST, 0);
   if (!m_bMaster) {
      if (hideMemberList == 0) {
         ui->layout_speak->addWidget(this->m_pSpeakerButton);
      } else {
         ui->topSpeekBtnLayOut->addWidget(this->m_pSpeakerButton);
      }
   }
}

void VhallSpeakerList::OnUpdateJoinState(bool offLine) {
   if (bGuest) {
      if (offLine) {
         //ui->pushButtonCancelJoin->hide();
         //ui->pushButtonJoin->show();
      } else {
         //ui->pushButtonCancelJoin->show();
         //ui->pushButtonJoin->hide();
      }
   }
}

void VhallSpeakerList::Slot_JoinActivity() {
   //点击上麦后，启动定时器，超时时间内不允许再次点击。
   if (m_pTimer) {
      m_pTimer->start(TIMER_OUT_BUTTON_DISABLE);
   }

   m_pSpeakerButton->setEnabled(false);
   emit Sig_JoinActivity();
}

void VhallSpeakerList::Slot_CancelJoinActivity() {
   if (m_pTimer) {
      m_pTimer->start(TIMER_OUT_BUTTON_DISABLE);
   }
   m_pSpeakerButton->setEnabled(false);
   emit Sig_CancelJoinActivity();
}
void VhallSpeakerList::Slot_SpeakerButtonClicked() {
   QJsonObject body;
   if (!m_bSpeakerStatus) {
      Slot_JoinActivity();
      body["ac"] = "OnWheat";
   } else {
      Slot_CancelJoinActivity();
      body["ac"] = "UnderWheat";
   }
   SingletonMainUIIns::Instance().reportLog(L"interaction_horonwheat", eLogRePortK_Interaction_HorOnWheat, body);
}

void VhallSpeakerList::Slot_ButtonDisableTimeOut() {
   m_pSpeakerButton->setEnabled(true);
}

void VhallSpeakerList::OnJoinActiveSuc(bool suc) {
   TRACE6("%s %d\n", __FUNCTION__, suc);
   m_pTimer->stop();
   Slot_ButtonDisableTimeOut();
   if (suc) {
      m_bSpeakerStatus = true;
      this->m_pSpeakerButton->setStyleSheet(
         "QToolButton{border-image: url(\":/interactivity/offSpeake\");}"
         "QToolButton:hover{border-image: url(\":/interactivity/offSpeakeHover\");}"
         "QToolButton:pressed {border-image: url(\":/interactivity/offSpeake\");}"
         );
   } else {
      m_bSpeakerStatus = false;
      this->m_pSpeakerButton->setStyleSheet(
         "QToolButton{border-image: url(\":/interactivity/toSpeake\");}"
         "QToolButton:hover{border-image: url(\":/interactivity/toSpeakeHover\");}"
         "QToolButton:pressed {border-image: url(\":/interactivity/toSpeake\");}"
         );
   }
}

void VhallSpeakerList::OnLeaveActiveSuc(bool suc) {
   TRACE6("%s %d\n", __FUNCTION__, suc);
   m_pTimer->stop();
   Slot_ButtonDisableTimeOut();
   if (suc) {
      m_bSpeakerStatus = false;
      this->m_pSpeakerButton->setStyleSheet(
         "QToolButton{border-image: url(\":/interactivity/toSpeake\");}"
         "QToolButton:hover{border-image: url(\":/interactivity/toSpeakeHover\");}"
         "QToolButton:pressed {border-image: url(\":/interactivity/toSpeake\");}"
         );

   } else {
      m_bSpeakerStatus = true;
      this->m_pSpeakerButton->setStyleSheet(
         "QToolButton{border-image: url(\":/interactivity/offSpeake\");}"
         "QToolButton:hover{border-image: url(\":/interactivity/offSpeakeHover\");}"
         "QToolButton:pressed {border-image: url(\":/interactivity/offSpeake\");}"
         );
   }
}
void VhallSpeakerList::Remove(QString id) {
   for (int i = 0; i < ui->listWidget->count(); i++) {
      QListWidgetItem *item = ui->listWidget->item(i);
      QWidget *w = ui->listWidget->itemWidget(item);
      if (w) {
         VhallSpeakerListItem *itemWidget = dynamic_cast<VhallSpeakerListItem *>(w);
         if (itemWidget) {
            if (itemWidget->GetId() == id) {
               delete itemWidget;
               delete item;
            }
         }
      }
   }

}
//void VhallSpeakerList::Append(QString id, QString name, bool master) {
//   if (GetItem(id)) {
//      return;
//   }
//
//   VH::CComPtr<IVhallRightExtraWidgetLogic> pVhallRightExtraWidget = NULL;
//   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IVhallRightExtraWidget, IID_IVhallRightExtraWidgetLogic, pVhallRightExtraWidget);
//   if (pVhallRightExtraWidget != NULL) {
//      wchar_t wname[512] = { 0 };
//      //if (pVhallRightExtraWidget->GetUserName(id.toUtf8().data(), wname)) 
//	  {
//         name = QString::fromWCharArray(wname);
//      }
//   }
//
//   VhallSpeakerListItem *itemWidget = new VhallSpeakerListItem(ui->listWidget);
//   QListWidgetItem *item = new QListWidgetItem(ui->listWidget);
//   itemWidget->setId(id);
//   itemWidget->setName(name);
//   itemWidget->setMaster(master);
//   item->setSizeHint(QSize(0, 40));
//   if (!m_bMaster) {
//      itemWidget->SetOffVisiable(false);
//   } else if (mUserId != id) {
//      itemWidget->SetOffVisiable(true);
//   } else {
//      itemWidget->SetOffVisiable(false);
//   }
//
//   if (master && mUserId == id) {
//      ui->listWidget->insertItem(0, item);
//   } else {
//      ui->listWidget->insertItem(ui->listWidget->count(), item);
//   }
//
//   ui->listWidget->setItemWidget(item, itemWidget);
//   connect(itemWidget, SIGNAL(SigItemClicked(QString)), this, SIGNAL(Sig_ItemClicked(QString)));
//   auto itorCamera = mCameraIcons.find(id);
//   if (itorCamera != mCameraIcons.end()) {
//      itemWidget->setCameraIcon(itorCamera.value());
//      mCameraIcons.erase(itorCamera);
//   }
//   auto itorMic = mMicIcons.find(id);
//   if (itorMic != mMicIcons.end()) {
//      itemWidget->setMicIcon(itorMic.value());
//      mMicIcons.erase(itorMic);
//   }
//}

VhallSpeakerListItem *VhallSpeakerList::GetItem(QString id) {
   for (int i = 0; i < ui->listWidget->count(); i++) {
      QListWidgetItem *item = ui->listWidget->item(i);
      QWidget *w = ui->listWidget->itemWidget(item);
      if (w) {
         VhallSpeakerListItem *itemWidget = dynamic_cast<VhallSpeakerListItem *>(w);
         if (itemWidget) {
            if (itemWidget->GetId() == id) {
               return itemWidget;
            }
         }
      }
   }
   return NULL;
}

void VhallSpeakerList::SetCameraIcon(QString id, bool ok) {
   VhallSpeakerListItem *itemWidget = GetItem(id);
   if (itemWidget) {
      itemWidget->setCameraIcon(ok);
   } else {
      mCameraIcons[id] = ok;
   }
}
bool VhallSpeakerList::SetMicIcon(QString id, bool ok) {
   bool bRemix = false;
   VhallSpeakerListItem *itemWidget = GetItem(id);
   if (itemWidget) {
      bool bCurMixState = itemWidget->GetMicState();
      if (bCurMixState != ok) {
         bRemix = true;
      }
      itemWidget->setMicIcon(ok);
   } else {
      mMicIcons[id] = ok;
   }
   return bRemix;
}
//void VhallSpeakerList::RefreshOnlineList() {
//   qDebug() << "#######VhallSpeakerList::RefreshOnlineList()#######";
//   VH::CComPtr<IVhallRightExtraWidgetLogic> pVhallRightExtraWidget = NULL;
//   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IVhallRightExtraWidget, IID_IVhallRightExtraWidgetLogic, pVhallRightExtraWidget);
//   if (!pVhallRightExtraWidget) {
//      return;
//   }
//
//   for (int i = 0; i < ui->listWidget->count(); i++) {
//      QListWidgetItem *item = ui->listWidget->item(i);
//      QWidget *w = ui->listWidget->itemWidget(item);
//      if (w) {
//         VhallSpeakerListItem *itemWidget = dynamic_cast<VhallSpeakerListItem *>(w);
//         if (itemWidget) {
//            wchar_t name[256] = { 0 };
//            QString id = itemWidget->GetId();
//            //if (pVhallRightExtraWidget->GetUserName(id.toUtf8().data(), name)) 
//			{
//               itemWidget->setName(QString::fromWCharArray(name));
//               qDebug() << "VhallSpeakerList::RefreshOnlineList() " << id << QString::fromWCharArray(name);
//            }
//         }
//      }
//   }
//}

void VhallSpeakerList::Clear() {
   ui->listWidget->clear();
}

QList<QString> VhallSpeakerList::GetSpeakListUsers() {
   QList<QString>  members;
   for (int i = 0; i < ui->listWidget->count(); i++) {
      QListWidgetItem *item = ui->listWidget->item(i);
      QWidget *w = ui->listWidget->itemWidget(item);
      if (w) {
         VhallSpeakerListItem *itemWidget = dynamic_cast<VhallSpeakerListItem *>(w);
         if (itemWidget) {
            members.push_back(itemWidget->GetId());
         }
      }
   }
   return members;
}

bool VhallSpeakerList::IsExistSpeak(const QString& id) {
   for (int i = 0; i < ui->listWidget->count(); i++) {
      QListWidgetItem *item = ui->listWidget->item(i);
      QWidget *w = ui->listWidget->itemWidget(item);
      if (w) {
         VhallSpeakerListItem *itemWidget = dynamic_cast<VhallSpeakerListItem *>(w);
         if (itemWidget) {
            if (itemWidget->GetId() == id) {
               return true;
            }
         }
      }
   }
   return false;
}

void VhallSpeakerList::SetPushState(bool pushing) {
   //主播如果停止推流则上麦按键不可点击。
   if (pushing && !m_bPushing) {
      m_bPushing = true;
      m_pSpeakerButton->setEnabled(true);

      this->m_pSpeakerButton->setStyleSheet(
         "QToolButton{border-image: url(\":/interactivity/toSpeake\");}"
         "QToolButton:hover{border-image: url(\":/interactivity/toSpeakeHover\");}"
         "QToolButton:pressed {border-image: url(\":/interactivity/toSpeake\");}"
         );
   } else if (!pushing) {
      m_bPushing = false;
      m_pSpeakerButton->setEnabled(false);
      this->m_pSpeakerButton->setStyleSheet(
         "QToolButton{border-image: url(\":/interactivity/img/interactivity/to_speake_disable.png\");}"
         "QToolButton:hover{border-image: url(\":/interactivity/img/interactivity/to_speake_disable.png\");}"
         "QToolButton:pressed {border-image: url(\":/interactivity/img/interactivity/to_speake_disable.png\");}"
         );
   }
}


QMap<QString, bool> VhallSpeakerList::GetAllOpenMicUsers() {
   QMap<QString, bool> micStateMap;
   for (int i = 0; i < ui->listWidget->count(); i++) {
      QListWidgetItem *item = ui->listWidget->item(i);
      if (item) {
         QWidget *w = ui->listWidget->itemWidget(item);
         if (w) {
            VhallSpeakerListItem *itemWidget = dynamic_cast<VhallSpeakerListItem *>(w);
            if (itemWidget && itemWidget->GetMicState()) {
               micStateMap[itemWidget->GetId()] = itemWidget->GetMicState();
            }
         }
      }
   }
   return micStateMap;
}

bool VhallSpeakerList::GetMicState(const QString id) {
   bool bOpenMic = false;
   for (int i = 0; i < ui->listWidget->count(); i++) {
      QListWidgetItem *item = ui->listWidget->item(i);
      if (item) {
         QWidget *w = ui->listWidget->itemWidget(item);
         if (w) {
            VhallSpeakerListItem *itemWidget = dynamic_cast<VhallSpeakerListItem *>(w);
            if (itemWidget && itemWidget->GetId().compare(id) == 0) {
               bOpenMic = itemWidget->GetMicState();
            }
         }
      }
   }
   return bOpenMic;
}

