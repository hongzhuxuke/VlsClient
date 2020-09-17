#include "LibCefViewWdg.h"
#include "VhallUI_define.h"
#include <QApplication>
#include <QDesktopWidget>
#include <QGuiApplication>
#include <QScreen> 
#include "DebugTrace.h"
#include "ConfigSetting.h"
#include "vhallwaiting.h"
#include "pathmanager.h"

LibCefViewWdg::LibCefViewWdg(MsgCallbackDelegate* delegate, bool isLiveListView,QWidget *parent)
    : QWidget(parent)
   , mDelegate(delegate)
   , mbIsListView(isLiveListView)
{
   this->setStyleSheet("QWidget{background-color: rgba(67, 67, 67, 255);}");
   setAttribute(Qt::WA_TranslucentBackground);
   setAutoFillBackground(true);
   mShowTimer = new QTimer(this);
   connect(mShowTimer,SIGNAL(timeout()),this,SLOT(slot_OnChnageWebView()));
   mShowTimer->setSingleShot(true);

   QDesktopWidget* desktopWidget = QApplication::desktop();
   int index = desktopWidget->screenNumber(this);
   int screen_logDpi = 0;
   int screen_phyDpi = 0;
   QList<QScreen *> screenlist = QGuiApplication::screens();
   for (int i = 0; i < screenlist.size(); i++) {
      QScreen * curQScreen = screenlist.at(i);
      if (curQScreen) {
         connect(curQScreen, SIGNAL(physicalSizeChanged(const QSizeF &)), this, SLOT(slot_physicalSizeChanged(const QSizeF &)));
         connect(curQScreen, SIGNAL(physicalDotsPerInchChanged(qreal)), this, SLOT(slot_physicalDotsPerInchChanged(qreal)));
         connect(curQScreen, SIGNAL(logicalDotsPerInchChanged(qreal)), this, SLOT(slot_logicalDotsPerInchChanged(qreal)));
      }
      screen_logDpi = screenlist.at(i)->logicalDotsPerInchX();
      screen_phyDpi = screenlist.at(i)->physicalDotsPerInchX();
      double screen_dipSize = (double)(screen_logDpi)/(double)(screen_phyDpi);
      TRACE6("%s  width %d height %d screen_dipSize %f \n", __FUNCTION__,  screen_logDpi, screen_phyDpi, screen_dipSize);
   }

   QString dpi = ConfigSetting::ReadString(CPathManager::GetConfigPath(), GROUP_DEFAULT, DESKTOP_DIP, "1");
   if (!dpi.isEmpty()) {
      mDpi = dpi.toDouble();
   }
}

LibCefViewWdg::~LibCefViewWdg()
{
   if (mDocCefView) {
      //mDocCefView->CloseWebView(mDocIndex);
      vhall_paas::DestoryVHWebViewObject(mDocCefView);
      mDocCefView = nullptr;
   }
}

void LibCefViewWdg::InitLibCef() {
   if (mDocCefView == nullptr) {
      mDocCefView = vhall_paas::GetVHWebViewObject();
      if (mDocCefView) {
         mDocCefView->InitlibCef();
      }
   }
}

void LibCefViewWdg::Create(QString url) {
   mUrl = url + "&view_type=cef";
   TRACE6("%s mUrl:%s\n", __FUNCTION__, mUrl.toStdString().c_str());
}

void LibCefViewWdg::ChangeViewSize(int width, int height) {
   double screen_dipSize = ConfigSetting::GetCurrentScreenDPISize(CPathManager::GetConfigPath(), this);
   TRACE6("%s %s width %d height %d screen_dipSize %f \n", __FUNCTION__, mUrl.toStdString().c_str(), width, height, screen_dipSize);
   if (mDocCefView) {
      mDocCefView->ChangeWebViewSize(mDocIndex, width * screen_dipSize, height * screen_dipSize);
   }
}

void LibCefViewWdg::CloseWeb() {
   if (mDocCefView) {
      mDocCefView->CloseAll();
   }
}

void LibCefViewWdg::LoadUrl(QString url) {
   mUrl = url + "&view_type=cef";
   TRACE6("%s mUrl:%s\n", __FUNCTION__, mUrl.toStdString().c_str());
   if (mDocCefView && mDocIndex >= 0) {
      mDocCefView->LoadUrl(mDocIndex, mUrl.toStdString().c_str());
      int w = this->width();
      int h = this->height();
      TRACE6("%s mUrl:%s\n", __FUNCTION__, mUrl.toStdString().c_str());
      double screen_dipSize = ConfigSetting::GetCurrentScreenDPISize(CPathManager::GetConfigPath(), this);
      TRACE6("%s %s width %d height %d screen_dipSize %f \n", __FUNCTION__, mUrl.toStdString().c_str(), width(), height(), screen_dipSize);
      /*mDocCefView->ChangeWebViewSize(mDocIndex, 1, 1);*/
      mDocCefView->ChangeWebViewSize(mDocIndex, this->width() * screen_dipSize, this->height() * screen_dipSize);
      TRACE6("%s mUrl:%s  mDocIndex %d\n", __FUNCTION__, mUrl.toStdString().c_str(), mDocIndex);
   }
}

void LibCefViewWdg::AppCallJsFunction(QString function) {
   if (mDocCefView) {
      mDocCefView->AppCallJsFunction(mDocIndex, function.toStdString().c_str());
   }
}

void  LibCefViewWdg::showEvent(QShowEvent *e) {
   if (!mbShown) {
      if (mDocCefView) {
         int w = this->width();
         int h = this->height();
         TRACE6("%s mUrl:%s\n", __FUNCTION__, mUrl.toStdString().c_str());
         double screen_dipSize = ConfigSetting::GetCurrentScreenDPISize(CPathManager::GetConfigPath(), this);
         TRACE6("%s %s width %d height %d screen_dipSize %f \n", __FUNCTION__, mUrl.toStdString().c_str(), width(), height(), screen_dipSize);
         int is_http_proxy = ConfigSetting::ReadInt(CPathManager::GetConfigPath(), GROUP_DEFAULT, PROXY_OPEN, 0);
         QString proxy_ip = ConfigSetting::ReadString(CPathManager::GetConfigPath(), GROUP_DEFAULT, PROXY_HOST, "");
         int proxy_port = ConfigSetting::ReadInt(CPathManager::GetConfigPath(), GROUP_DEFAULT, PROXY_PORT, 80);
         QString user = ConfigSetting::ReadString(CPathManager::GetConfigPath(), GROUP_DEFAULT, PROXY_USERNAME, "");
         QString pwd = ConfigSetting::ReadString(CPathManager::GetConfigPath(), GROUP_DEFAULT, PROXY_PASSWORD, "");
         if (is_http_proxy == 1 && !proxy_ip.isEmpty()) {
            std::string proxy_server = proxy_ip.toStdString()+std::string(":") + QString::number(proxy_port).toStdString();
            mDocCefView->SetProxy(true, proxy_server.c_str(), user.toStdString().c_str(), pwd.toStdString().c_str());
         }
         else {
            mDocCefView->SetProxy(false, "", "", "");
         }
         //mDocCefView->CreateWebView(this, (void *)this->winId(), w * screen_dipSize, h * screen_dipSize, mUrl.toStdString().c_str());
         if (mbIsListView) {
            mDocCefView->CreateWebView(this, (void *)this->winId(), 2, 2, mUrl.toStdString().c_str());
         }
         else {
            mDocCefView->CreateWebView(this, (void *)this->winId(), w * screen_dipSize, h * screen_dipSize, mUrl.toStdString().c_str());
         }
      }
      mbShown = true;
   }
}

void  LibCefViewWdg::resizeEvent(QResizeEvent *event) {
   if (mDocCefView) {
      double screen_dipSize = ConfigSetting::GetCurrentScreenDPISize(CPathManager::GetConfigPath(), this);
      TRACE6("%s %s width %d height %d screen_dipSize %f \n", __FUNCTION__, mUrl.toStdString().c_str(), width(), height(), screen_dipSize);
      mDocCefView->ChangeWebViewSize(mDocIndex, this->width() * screen_dipSize, this->height() * screen_dipSize);
   }
}

/*创建WebView成功时的回调。
*参数：id 对应创建webview的id, url 创建webview时对应加载的url地址。
*用户根据返回的id，可操作指定的webview*/
void LibCefViewWdg::OnHandleCreateWebView(int id, const char* url) {
   if (url != nullptr && !mUrl.isEmpty() && QString(url).contains(mUrl) && this && mDocCefView) {
      mDocIndex = id;
      CustomRecvMsgEvent* event = new CustomRecvMsgEvent(CustomEvent_LibCefLoadFinished);
      QApplication::postEvent(this, event);
   }
}

void LibCefViewWdg::slot_OnChnageWebView() {
   if (mDocCefView) {
      double screen_dipSize = ConfigSetting::GetCurrentScreenDPISize(CPathManager::GetConfigPath(), this);
      TRACE6("%s url %s width %d height %d screen_dipSize %f \n", __FUNCTION__, mUrl.toStdString().c_str(), width(), height(), screen_dipSize);
      mDocCefView->ChangeWebViewSize(mDocIndex, this->width() * screen_dipSize, this->height() * screen_dipSize);
   }
   if (mDelegate) {
      mDelegate->OnWebViewLoadEnd();
   }
}

/*webview销毁事件通知*/
void LibCefViewWdg::OnHandleWebViewDestoryed(int id) {
   if (id == mDocIndex && mDelegate) {
      mDelegate->OnWebViewDestoryed();
   }
}

void LibCefViewWdg::OnRecvMsg(const int id, std::string fun_name, std::string msg) {
   TRACE6("%s fun_name:%s\n", __FUNCTION__, fun_name.c_str());
   if (id == mDocIndex && mDelegate) {
      TRACE6("%s msg:%s\n", __FUNCTION__, msg.c_str());
      TRACE6("%s url:%s\n", __FUNCTION__, mUrl.toStdString().c_str());
      mDelegate->OnRecvMsg(fun_name, msg);
   }
   TRACE6("%s fun_name:%s end\n", __FUNCTION__, fun_name.c_str());
}

void LibCefViewWdg::OnTitleChanged(const int id, std::string title_name) {
   if (id == mDocIndex && mDelegate) {
      //mDelegate->OnTitleChanged(id, title_name);
      CustomRecvMsgEvent* event = new CustomRecvMsgEvent(CustomEvent_CefOnTitleChanged,QString::fromStdString(title_name));
      QApplication::postEvent(this, event);
   }
}

void LibCefViewWdg::customEvent(QEvent* event) {
   if (event) {
      int typeValue = event->type();
      switch (typeValue) {
      case CustomEvent_LibCefLoadFinished: {
         //mShowTimer->start(1000);
         TRACE6("%s CustomEvent_LibCefLoadFinished %s\n", __FUNCTION__, mUrl.toStdString().c_str());
         slot_OnChnageWebView();
         break;
      }
      case CustomEvent_CefOnTitleChanged: {
         CustomRecvMsgEvent* custom_msg = dynamic_cast<CustomRecvMsgEvent*>(event);
         if (mDelegate) {
            mDelegate->OnTitleChanged(mDocIndex, custom_msg->msg.toStdString());
         }
         break;
      }
      case CustomEvent_CefOnRecvMsg: {
         CefMsgEvent* custom_msg = dynamic_cast<CefMsgEvent*>(event);
         if (mDelegate) {
            mDelegate->OnRecvMsg(custom_msg->mFunName.toStdString(), custom_msg->mCefMsgData.toStdString());
         }
         break;
      }
      default:
         break;
      }
   }
}

void LibCefViewWdg::enterEvent(QEvent *event) {
   if (event) {
      TRACE6("%s enterEvent\n", __FUNCTION__);
   }
}

void LibCefViewWdg::keyPressEvent(QKeyEvent *event) {
   switch (event->key())
   {
   case Qt::Key_Escape:
      emit sig_PressKeyEsc();
      break;
   default:
      QWidget::keyPressEvent(event);
   }
}

void LibCefViewWdg::keyReleaseEvent(QKeyEvent *event) {
   switch (event->key())
   {
   case Qt::Key_Escape:
      emit sig_PressKeyEsc();
      break;
   default:
      QWidget::keyPressEvent(event);
   }
}

void LibCefViewWdg::slot_physicalSizeChanged(const QSizeF &) {
   resizeEvent(nullptr);
}

void LibCefViewWdg::slot_physicalDotsPerInchChanged(qreal) {
   resizeEvent(nullptr);
}

void  LibCefViewWdg::slot_logicalDotsPerInchChanged(qreal) {
   resizeEvent(nullptr);
}

