#include "VhallLiveListViewDlg.h"
#include <QApplication>
#include <QDesktopWidget>
#include <QLabel>
#include <QPushButton>
#include <QtNetwork/QNetworkProxy>
#include <QVBoxLayout>
#include <QPropertyAnimation>
#include "TitleBarWidget.h"
#include "DebugTrace.h"
#include "ConfigSetting.h"
#include "vhallwaiting.h"
#include "pathmanager.h"
#include "pub.Const.h"
#include "pub.Struct.h"
#include "Msg_MainUI.h"
#include "MainUIIns.h"
#include <QScreen>
#include <QDesktopWidget>
#include <QDesktopServices>
#include <QProcess>

VhallLiveListViewDlg::VhallLiveListViewDlg(QWidget *parent) :
   QWidget(parent),
   m_pCenterLayout(NULL),
   m_pTitle(NULL) {
   ui.setupUi(this);
   setStyleSheet(QLatin1String("background-color:rgb(28,28,28); "));
   setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowSystemMenuHint | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint);

   m_pCenterLayout = new QVBoxLayout(this);
   m_pCenterLayout->setSpacing(0);
   m_pCenterLayout->setContentsMargins(0, 0, 0, 0);
   m_pTitle = new TitleBarWidget(this);
   m_pCenterLayout->addWidget(m_pTitle);
   connect(this, SIGNAL(SigJsCode(QString)), this, SLOT(SlotJsCode(QString)));
   setMinimumSize(QSize((898), (602)));
   setMaximumSize(QSize((898), (602)));
   this->installEventFilter(this);
}

VhallLiveListViewDlg::~VhallLiveListViewDlg() {
   if (m_exitWidget) {
      delete m_exitWidget;
      m_exitWidget = NULL;
   }
   ReleaseWebEngineView();
}

void VhallLiveListViewDlg::Destory() {

}

void VhallLiveListViewDlg::ReleaseWebEngineView() {

}

void VhallLiveListViewDlg::OnWebViewLoadEnd() {

}

void VhallLiveListViewDlg::Load(QString url, QObject *obj) {
   m_obj = obj;
   m_liveUrl = url;
   TRACE6("%s  m_liveUrl:%s\n", __FUNCTION__, m_liveUrl.toStdString().c_str());
   AddWebEngineView();
}

void VhallLiveListViewDlg::loadFinished(bool ok) {
   TRACE6("%s ok:%d\n", __FUNCTION__, ok);
   m_bFinished = true;
   TRACE6("%s loadFinished end\n", __FUNCTION__);
}

void VhallLiveListViewDlg::loadFinished(QNetworkReply *reply) {
   if (reply) {
      int nErr = reply->error();
      if (nErr != 0) {
         TRACE6("%s err:%d\n", __FUNCTION__, nErr);
      }
   }
}

void VhallLiveListViewDlg::CenterWindow(QWidget* parent) {
   int x = 0;
   int y = 0;
   if (NULL == parent) {
      const QRect rect = QApplication::desktop()->availableGeometry();
      x = rect.left() + (rect.width() - width()) / 2;
      y = rect.top() + (rect.height() - height()) / 2;
   }
   else {
      QPoint point(0, 0);
      point = parent->mapToGlobal(point);
      x = point.x() + (parent->width() - width()) / 2;
      y = point.y() + (parent->height() - height()) / 2;
   }
   move(x, y);
}


void VhallLiveListViewDlg::OnRecvMsg(std::string fun_name, std::string msg) {
   if (fun_name == "JsCallQtStartLive") {
      CustomRecvMsgEvent* event = new CustomRecvMsgEvent(CustomEvent_JsCallQtStartLive, QString::fromStdString(msg));
      QApplication::postEvent(m_obj, event);
   }
   else if (fun_name == "JsCallQtShareTo") {
      CustomRecvMsgEvent* event = new CustomRecvMsgEvent(CustomEvent_JsCallQtShareTo, QString::fromStdString(msg));
      QApplication::postEvent(m_obj, event);
   }
   else if (fun_name == "JsCallQtJoinActivity") {
      CustomRecvMsgEvent* event = new CustomRecvMsgEvent(CustomEvent_JsCallQtJoinActivity, QString::fromStdString(msg));
      QApplication::postEvent(m_obj, event);
   }
   else if (fun_name == "JsCallQtOpenUrl") {
      //创建直播
      CustomRecvMsgEvent* event = new CustomRecvMsgEvent(CustomEvent_JsCallQtOpenUrl, QString::fromStdString(msg));
      QApplication::postEvent(m_obj, event);
   }
   else if (fun_name == "JsCallQtStartVoiceLive") {
      CustomRecvMsgEvent* event = new CustomRecvMsgEvent(CustomEvent_JsCallQtStartVoiceLive, QString::fromStdString(msg));
      QApplication::postEvent(m_obj, event);
   }
}

void VhallLiveListViewDlg::OnWebViewDestoryed() {

}

bool VhallLiveListViewDlg::Create() {
   m_pTitle->Create(false);
   m_pTitle->HideSetting();

   m_pTitle->SetParentWidget(this);
   m_pTitle->ShowRefreshButton();
   connect(m_pTitle, SIGNAL(sigRefresh()), this, SLOT(slot_reloadURL()));
   connect(m_pTitle, SIGNAL(sigMin()), this, SLOT(slot_OnShowMin()));

   m_userInfo = new VhallExtraWidgetUserInfo(this);
   m_userInfo->installEventFilter(this);
   m_pTitle->AddImageWidget(m_userInfo);

   m_exitWidget = new VhallExitWidget(NULL);
   connect(m_exitWidget, SIGNAL(clicked()), this, SLOT(ToExit()));

   QLabel *label = new QLabel(this);
   label->resize(896, 40);
   label->move(1, 0);

   label->setStyleSheet("background-color:transparent;");
   m_pTitle->raise();
   return true;
}

void VhallLiveListViewDlg::SetUserName(QString &name) {
   if (m_userInfo) {
      m_userInfo->SlotSetUserName(name);
   }
}

void VhallLiveListViewDlg::SetUserImage(QString &imageUrl) {
   if (m_userInfo) {
      m_userInfo->SetUserImage(imageUrl);
   }
}

bool VhallLiveListViewDlg::eventFilter(QObject *o, QEvent *e) {
   if (o == m_userInfo) {
      if (e->type() == QEvent::MouseButtonRelease) {
         if (m_exitWidget) {
            QPoint p = m_userInfo->mapToGlobal(QPoint(-5, 35));
            m_exitWidget->move(p);
            m_exitWidget->show();
         }
      }
   }
   else if (o == this) {
      if (e->type() == QEvent::Move
         || e->type() == QEvent::Show
         || e->type() == QEvent::ShowToParent
         || e->type() == QEvent::WindowActivate
         || e->type() == QEvent::ActivationChange
         ) {
         if (m_pwaiting) {
            VhallWaiting *w = dynamic_cast<VhallWaiting *>(m_pwaiting);
            if (w) {
               w->Repos();
            }
         }
      }
   }
   return QWidget::eventFilter(o, e);
}

bool VhallLiveListViewDlg::nativeEvent(const QByteArray &eventType, void *message, long *result) {
   MSG* msg = reinterpret_cast<MSG*>(message);
   if (msg) {
      switch (msg->wParam)
      {
      case SC_CLOSE: {
         if (msg->hwnd == (HWND)this->winId()) {
            ToExitAppWithAnimation();
            return true;
         }
         break;
      }
      case WM_DPICHANGED: {
         int a = 0;
         a = 0;
         break;
      }
      default:
         break;
      }
   }
   return QWidget::nativeEvent(eventType, message, result);
}

void VhallLiveListViewDlg::executeJSCode(QString method) {
   qDebug() << "executeJSCode " << method;
   emit SigJsCode(method);
}

void VhallLiveListViewDlg::SlotJsCode(QString method) {
   if (mListCefView) {
      mListCefView->AppCallJsFunction(method);
   }
}

void VhallLiveListViewDlg::ToExit() {
   qDebug() << "VhallWebView::ToExit";
   TRACE6("VhallLiveListViewDlg::ToExit.\n");
   emit exitClicked(true);
}

void VhallLiveListViewDlg::slot_reloadURL() {
   if (mListCefView) {
      mListCefView->LoadUrl(m_liveUrl);
   }
}

void VhallLiveListViewDlg::slot_OnShowMin() {
   this->showMinimized();
}

void VhallLiveListViewDlg::SlotRefresh() {
   TRACE6("%s active list url:%s\n", __FUNCTION__, m_liveUrl.toStdString().c_str());
   m_bInit = false;
   m_bFinished = false;
   slot_reloadURL();
   //QJsonObject body;
   //SingletonMainUIIns::Instance().reportLog(STR_VH_LIVE_OPERATE, eLogRePortK_LiveListRefresh, L"LiveListRefresh", body);
   TRACE6("%s SlotRefresh end\n", __FUNCTION__);
}

void VhallLiveListViewDlg::ShowTeaching() {
   if (!m_bShowTeaching) {
      return;
   }

   m_bShowTeaching = false;
   if (mListCefView) {
      VhallTeaching teaching(mListCefView, mListCefView->rect());
      connect(m_pTitle, SIGNAL(sigToClose()), &teaching, SLOT(close()));
      teaching.AddStep(QRect(19, 65, 858, 430), true,
         QRect(30, 490, 100, 75),
         QString(tr("获取Saas平台直播列表，点击开播，进入直播页面")),
         VhallTechingTip::Tip_Top,
         QString(":/teaching/BG_LEFT_TOP_LARGE"),
         QRect(415, 511, 100, 30),
         QString(tr("确定")));
      teaching.AddResource(0, QPoint(20, 70), QString(":/teaching/LIST"));
      teaching.Teaching();
   }
}

void VhallLiveListViewDlg::AddWebEngineView() {
   QString toolConfPath = CPathManager::GetAppDataPath() + QString::fromStdWString(VHALL_TOOL_CONFIG);
   int hideLogo = ConfigSetting::ReadInt(toolConfPath, GROUP_DEFAULT, KEY_VHALL_LOGO_HIDE, 0);
   QString newUrl = QString("%1&client_theme=%2").arg(m_liveUrl).arg(hideLogo);
   if (hideLogo == 1) {
      newUrl = newUrl + "&hide_share=1";
   }
   TRACE6("%s  url:%s\n", __FUNCTION__, newUrl.toStdString().c_str());
   m_liveUrl = newUrl;
   if (mListCefView == nullptr) {
      mListCefView = new LibCefViewWdg(this, true);
      if (mListCefView) {
         mListCefView->InitLibCef();
         mListCefView->Create(newUrl);
         m_pCenterLayout->addWidget(mListCefView);
      }
   }
}

QString VhallLiveListViewDlg::GetNickName() {
   QString strName;
   if (NULL != m_userInfo) {
      strName = m_userInfo->GetNickName();
   }
   return strName;
}

void VhallLiveListViewDlg::SetHttpProxyInfo(const bool bSetHttpProxy /*= false*/, const QString ip /*= QString()*/, const QString port /*= QString()*/, const QString userName /*= QString()*/, const QString pwd /*= QString()*/) {
   if (bSetHttpProxy) {
      m_ip = ip;
      m_port = m_port;
      m_userName = userName;
      m_pwd = pwd;
      proxy.setType(QNetworkProxy::HttpProxy);
      proxy.setHostName(m_ip);
      proxy.setPort(port.toInt());
      TRACE6("%s ip:%s port:%s user:%s pwd:%s\n", __FUNCTION__, ip.toStdString().c_str(), port.toStdString().c_str(), userName.toStdString().c_str(), pwd.toStdString().c_str());
      QNetworkProxy::setApplicationProxy(proxy);
   }
   else if (!bSetHttpProxy) {
      QNetworkProxy proxy;
      proxy.setType(QNetworkProxy::NoProxy);
      QNetworkProxy::setApplicationProxy(proxy);
   }
   m_bEnableHttpProxy = bSetHttpProxy;
}


void VhallLiveListViewDlg::ToExitAppWithAnimation() {
   if (m_pTitle) {
      m_pCenterLayout->removeWidget(m_pTitle);
      m_pTitle->setMinimumSize(0, 0);
   }

   setMinimumSize(0, 0);
   QPropertyAnimation* closeAnimation = new QPropertyAnimation(this, "geometry");
   closeAnimation->setStartValue(geometry());
   closeAnimation->setEndValue(QRect(geometry().x(), geometry().y(), width(), 0));
   closeAnimation->setDuration(150);
   connect(closeAnimation, SIGNAL(finished()), this, SLOT(slot_CloseAndExitApp()));
   closeAnimation->start(QAbstractAnimation::DeleteWhenStopped);
}

void VhallLiveListViewDlg::slot_CloseAndExitApp() {
   ReleaseWebEngineView();
   emit sig_ExitAppDirect();
}

