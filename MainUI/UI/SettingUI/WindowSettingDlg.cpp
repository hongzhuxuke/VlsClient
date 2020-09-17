#include "stdafx.h"
#include <QBoxLayout>
#include <QPainter>
#include <QDesktopWidget>
#include "TitleWidget.h"
#include "WindowSettingDlg.h"
#include "Msg_MainUI.h"
#include "InteractAPIManager.h"
#include "ICommonData.h"

WindowSettingDlg::WindowSettingDlg(QDialog *parent)
: VHDialog(parent)
, m_pTitleBar(NULL)
, m_pMainLayout(NULL) {
   ui.setupUi(this);
   setFixedSize(342, 182);
   setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
   setAttribute(Qt::WA_TranslucentBackground);
   setAutoFillBackground(true);
   connect(ui.saveBtn, SIGNAL(clicked()), this, SLOT(OnSave()));
}

WindowSettingDlg::~WindowSettingDlg() {
}

bool WindowSettingDlg::Create() {
   m_pTitleBar = new TitleWidget(tr("软件源"), NULL);
   if (NULL == m_pTitleBar) {
      ASSERT(FALSE);
      return false;
   }
   connect(m_pTitleBar, SIGNAL(closeWidget()), this, SLOT(close()));
   ui.gridLayoutTitle->addWidget(m_pTitleBar);
   m_pFlushBtn = new ToolButton(":/windowSetting/img/windowSetting/flush.png");
   ui.gridLayoutBtn->addWidget(m_pFlushBtn);   
   connect(m_pFlushBtn, SIGNAL(sigClicked()), this, SLOT(slotReFlush()));
   return true;
}
void WindowSettingDlg::slotReFlush() {

   STRU_MAINUI_CLICK_CONTROL loControl;
   loControl.m_eType = (enum_control_type)(control_WindowSrc);

   SingletonMainUIIns::Instance().PostCRMessage(
      MSG_MAINUI_CLICK_CONTROL, &loControl, sizeof(STRU_MAINUI_CLICK_CONTROL));

   VH::CComPtr<ICommonData> pCommonData;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
   pCommonData->ReportEvent("rjys03");
}
void WindowSettingDlg::Destroy() {
   //if (NULL != m_pMainLayout) {
   //   delete m_pMainLayout;
   //   m_pMainLayout = NULL;
   //}

   //if (NULL != m_pTitleBar) {
   //   delete m_pTitleBar;
   //   m_pTitleBar = NULL;
   //}
}

void WindowSettingDlg::AddDeviceItem(wchar_t* wzDeviceName, QVariant UserData) {
   ui.mCmbWindowList->addItem(QString::fromWCharArray(wzDeviceName), UserData);
}

bool WindowSettingDlg::GetCurItem(QVariant& UserData) {
   UserData = ui.mCmbWindowList->itemData(ui.mCmbWindowList->currentIndex());
   return true;
}

void WindowSettingDlg::OnSave() {
    InteractAPIManager apiManager(nullptr);
    apiManager.HttpSendChangeWebWatchLayout(1);
    SingletonMainUIIns::Instance().PostCRMessage(MSG_MAINUI_CLICK_ADD_WNDSRC, NULL, 0);
    hide();
    //accept();
}

////添加圆角，配合标题栏圆角
//void WindowSettingDlg::paintEvent(QPaintEvent *) {
//   QPainter painter(this);
//   painter.setRenderHint(QPainter::Antialiasing, true);
//   painter.setPen(QPen(QColor(54, 54, 54), 1));
//   painter.setBrush(QColor(38, 38, 38));
//   painter.drawRoundedRect(rect(), 4.0, 4.0);
//}

void WindowSettingDlg::Clear() {
   ui.mCmbWindowList->clear();
   ui.ckb_compatible->setChecked(false);
}

void WindowSettingDlg::CenterWindow(QWidget* parent) {
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
bool WindowSettingDlg::GetComptible(){
   return ui.ckb_compatible->isChecked();
}

