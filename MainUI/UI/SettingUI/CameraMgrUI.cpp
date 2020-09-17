#include "stdafx.h"
#include "CameraMgrUI.h"
#include "MainUIIns.h"
#include <QPainter>
#include <QLabel>
#include <QDesktopWidget>
#include <QDebug>
#include <QStyleOption>

#include "TitleWidget.h"

#include "Msg_MainUI.h"
#include "ICommonData.h"
#include "IOBSControlLogic.h"

CameraMgrUI::CameraMgrUI(QDialog *parent /*= 0*/)
: VHDialog(parent)
, m_pNoDeviceText(NULL) {
   ui.setupUi(this);
   this->setFocusPolicy(Qt::StrongFocus);
   this->setWindowFlags(Qt::FramelessWindowHint | Qt::SplashScreen);
   this->setMouseTracking(true);
}

CameraMgrUI::~CameraMgrUI() {

}

bool CameraMgrUI::Create() {
   //ui.Layout_DeviceItem->setContentsMargins(10, 10, 15, 0);
   m_iHeight = 22;
   return true;
}

void CameraMgrUI::Destroy() {
}

void CameraMgrUI::AddItem(CameraItem* item) {
	if (NULL == item) return;
   m_iHeight = m_iHeight + 40;
   m_oItemList.push_back(item);
   item->setSettingShow(mbShowSetting);
   
   //QSizePolicy sizePolicy3(QSizePolicy::Expanding, QSizePolicy::Expanding);
   //sizePolicy3.setHorizontalStretch(0);
   //sizePolicy3.setVerticalStretch(0);
   //sizePolicy3.setHeightForWidth(item->sizePolicy().hasHeightForWidth());
   //item->setSizePolicy(sizePolicy3);

   ui.Layout_DeviceItem->addWidget((QWidget*)item);
   setFixedHeight(m_iHeight);
}

void CameraMgrUI::SetItemIsFullScreen(DeviceInfo info, bool isFullScreen) {
   CameraItem *item = FindItem(info);
   if (item) {
      item->SetChecked(true);
      item->SetFullChecked(isFullScreen);
   }
}
void CameraMgrUI::RefreshItemStatus(enum_checkbox_status status) {
   if (status == checkbox_status_checked) {
      int count = SelectedItemCount();
      qDebug()<<"CameraMgrUI::RefreshItemStatus"<<count;
      if (count == 1) {
         //SelectedItemSetFullScreen();
         VH::CComPtr<ICommonData> pCommonData;
         DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
         pCommonData->ReportEvent("sxsb01");
      } else if (count >= 2) {
         UnSelectedItemEnable(false);
         VH::CComPtr<ICommonData> pCommonData;
         DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
         pCommonData->ReportEvent("sxsb02");
      }
   } else {
      if (SelectedItemCount() < 2) {
         UnSelectedItemEnable(true);
      }
   }
}

void CameraMgrUI::RefreshItemStatus() {
   int count = SelectedItemCount();
   if (count >= 2) {
      UnSelectedItemEnable(false);
   } else {
      UnSelectedItemEnable(true);
   }
}
void CameraMgrUI::SetItemUnchecked() {
   for(auto itor =m_oItemList.begin();itor!=m_oItemList.end();itor++) {
      (*itor)->SetChecked(false);
      (*itor)->SetFullChecked(false);
   }
}

int CameraMgrUI::SelectedItemCount() {
   //VH::CComPtr<IOBSControlLogic> pObsControlLogic = NULL;
   //DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IOBSControl, IID_IOBSControlLogic, pObsControlLogic, return 0, ASSERT(FALSE));
   //return pObsControlLogic->GetGraphicsDeviceInfoCount();

   int count = 0;
   for (auto itor = m_oItemList.begin(); itor != m_oItemList.end(); itor++) {
      if ((*itor)->IsDeviceChecked()) {
         count++;
      }
   }
   return count;
}
void CameraMgrUI::UnSelectedItemEnable(bool enabled) {
   for (auto itor = m_oItemList.begin(); itor != m_oItemList.end(); itor++) {
      if (!(*itor)->IsDeviceChecked()) {
         (*itor)->SetDisabled(!enabled);
      }
   }
}
void CameraMgrUI::SelectedItemSetFullScreen() {
   for (auto itor = m_oItemList.begin(); itor != m_oItemList.end(); itor++) {
      if ((*itor)->IsDeviceChecked()) {
         (*itor)->SetFullChecked(true);
      }
   }
}


void CameraMgrUI::ShowNoDeivice(bool bShow) {
   if (bShow) {
      if (!m_pNoDeviceText) {
         m_pNoDeviceText = new QLabel(this);
      }
      m_pNoDeviceText->setFixedHeight(40);
      m_pNoDeviceText->setText("ÔÝÎÞÉè±¸...");
      m_pNoDeviceText->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
      m_pNoDeviceText->setStyleSheet("color:#969696;font:16px;");
      ui.Layout_DeviceItem->addWidget((QWidget*)m_pNoDeviceText);
   } else {
      if (m_pNoDeviceText) {
         ui.Layout_DeviceItem->removeWidget((QWidget*)m_pNoDeviceText);
         m_pNoDeviceText->deleteLater();
         m_pNoDeviceText = NULL;
      }
   }
}

void CameraMgrUI::OnClose() {
   SingletonMainUIIns::Instance().PostCRMessage(MSG_MAINUI_MGR_CLOSEWND, NULL, 0);
   hide();
}

void CameraMgrUI::paintEvent(QPaintEvent *) {
   QStyleOption  opt;
   opt.init(this);
   QPainter  p(this);
   style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void CameraMgrUI::CenterWindow(QWidget* parent) {
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

void CameraMgrUI::ClearItem() {
   std::vector<CameraItem*>::iterator itor = m_oItemList.begin();
   while (itor != m_oItemList.end()) {
      ui.Layout_DeviceItem->removeWidget((QWidget*)*itor);
      ((QWidget*)*itor)->deleteLater();
      itor++;
   }
   m_oItemList.clear();
   m_iHeight = 22;
}

void CameraMgrUI::SetFullScreenShow(const bool& bShow)
{
	for (auto itor = m_oItemList.begin(); itor != m_oItemList.end(); itor++) {
		(*itor)->SetFullScreenShow(bShow);
	}
}

CameraItem *CameraMgrUI::FindItem(DeviceInfo deviceInfo) {
   for (auto itor = m_oItemList.begin(); itor != m_oItemList.end(); itor++) {
      if ((*itor)->Equal(deviceInfo)) {
         CameraItem *item = *itor;
         return item;
      }
   }
   return NULL;
}

void CameraMgrUI::enterEvent(QEvent *e) {
   QDialog::enterEvent(e);
   m_bEnter = true;
}
void CameraMgrUI::leaveEvent(QEvent *e) {
   QDialog::leaveEvent(e);
   m_bEnter = false;
   this->close();
}

void CameraMgrUI::focusOutEvent(QFocusEvent *e) {
   QDialog::focusOutEvent(e);
   if (!m_bEnter) {
      this->close();
      return;
   }
   QWidget *focusWidget = this->focusWidget();
   bool focusThis = false;

   while (focusWidget) {
      if (focusWidget == this) {
         focusThis = true;
         break;
      }

      focusWidget = focusWidget->parentWidget();
   }

   if (!focusThis) {
      this->close();
      m_bEnter = false;
   } else {
      this->setFocus();
   }
}

void CameraMgrUI::showEvent(QShowEvent *e) {
   setFocus();
   VHDialog::showEvent(e);
}
void CameraMgrUI::hideEvent(QHideEvent *e) {
   VHDialog::hideEvent(e);
}
