#include "vhallchatwidget.h"
#include "ui_vhallchatwidget.h"
#include "VhallRightExtraWidgetIns.h"
#include "Msg_VhallRightExtraWidget.h" 
#include <QSpacerItem>
#include <QDesktopServices>
#include <QMessageBox>
#include <QWebEngineProfile>
#include "VhallRightExtraWidgetIns.h"
#include "Msg_VhallRightExtraWidget.h"
#include "ICommonData.h"
#include "Logging.h"
#include "DebugTrace.h"

#if _MSC_VER >= 1600  
#pragma execution_character_set("utf-8")  
#endif  

VhallChatWidget::VhallChatWidget(QWidget *parent) :
QWidget(parent),
ui(new Ui::VhallChatWidget) {
   ui->setupUi(this);
   m_initForbidchat = 99;
   connect(ui->textEdit, SIGNAL(textChanged()), this, SLOT(slotsTextChanged()));
   ui->textEdit->installEventFilter(this);
   ui->textEdit->setContextMenuPolicy(Qt::NoContextMenu);
   this->setFocusPolicy(Qt::NoFocus);
   m_bHost = true;
}

VhallChatWidget::~VhallChatWidget() {
   if (m_pWebChannel && m_pWebEngineView) {
      m_pWebEngineView->page()->setWebChannel(NULL);
      delete m_pWebChannel;
      m_pWebChannel = NULL;
   }
   if (m_pWebEngineView) {
      ui->verticalLayout_web->removeWidget(m_pWebEngineView);
      m_pWebEngineView->stop();

      delete m_pWebEngineView;
      m_pWebEngineView = NULL;
   }
   delete ui;
}

// 初始化js交互
void VhallChatWidget::addObjectToJs() {
   //ui->chatWebView->page()->mainFrame()->addToJavaScriptWindowObject("MainWindow", this);
}


void VhallChatWidget::slot_loadFinished(bool ok) {
   mIsLoadUrlFinished = true;
}

void VhallChatWidget::Refresh() {
   if (mIsLoadUrlFinished) {
      mIsLoadUrlFinished = false;
      RegisterWebEngineView();
   }
   mIsLoadUrlFinished = false;
   ui->textEdit->clear();
}

void VhallChatWidget::SetIsHost(bool bHost) {
   m_bHost = bHost;
   Refresh();
}

//初始化基本信息，有外部調用
void VhallChatWidget::InitUserInfo(QString userName, QString userImageUrl, QString userId) {
   m_userName = userName;
   m_userImageUrl = userImageUrl;
   m_userId = userId;
   LoadUserInfo(userName, userImageUrl, userId);
}
//初始化roomid和url
void VhallChatWidget::setRoomId(QString roomId, char *filterurl, bool forbidchat) {
   this->m_roomId = roomId;
   m_filterurl = QString::fromUtf8(filterurl);
   m_initForbidchat = forbidchat ? 1 : 0;
   qDebug() << "VhallChatWidget::setRoomId:" << roomId << m_filterurl << m_initForbidchat;
}

//收到消息，然後調用js
void VhallChatWidget::appendChatMsg(QString msg) {
   msg.replace("\"", "\\\"");
   msg.replace("\'", "\\\'");
   QString method = QString("appendChatMsg(\'%1\')").arg(msg);
   if (method.length()<2048)
   {
	   TRACE6("%s - %s \n", __FUNCTION__, method.toStdString().c_str());
   }
   else
   {
	   TRACE6("%s - %s \n", __FUNCTION__, method.mid(0, 2048).toStdString().c_str());
   }
   
   if (m_pWebEngineView) {
      m_pWebEngineView->page()->runJavaScript(method);
   }

   if (m_pWebEngineView&&m_pWebEngineView->isVisible())
   {
	   TRACE6("%s - QWebEngineView isVisible  \n", __FUNCTION__);
   }
   else
   {
	   TRACE6("%s -  QWebEngineView NOT Visible  \n", __FUNCTION__);
   }
}

//收到“全體禁言”消息
void VhallChatWidget::recv_forbidAll() {
   QString method = QString("forbidAll()");
   if (m_pWebEngineView) {
      m_pWebEngineView->page()->runJavaScript(method);
      m_initForbidchat = 1;
   }
}
void VhallChatWidget::recv_cancelForbidAll() {
   QString method = QString("cancelForbidAll()");
   if (m_pWebEngineView) {
      m_pWebEngineView->page()->runJavaScript(method);
      m_initForbidchat = 0;
   }
}
//发送全体禁言消息
int VhallChatWidget::post_forbidAll(bool aim) {
   MsgRQType type = aim ? e_RQ_UserProhibitSpeakAllUser : e_RQ_UserAllowSpeakAllUser;
   RQData oData;
   oData.m_eMsgType = type;
   SingletonVhallRightExtraWidgetIns::Instance().PostCRMessage(MSG_VHALLRIGHTEXTRAWIDGET_SENDMSG, &oData, sizeof(RQData));
   if (type == e_RQ_UserProhibitSpeakAllUser) {
      recv_forbidAll();
   }
   else if (type == e_RQ_UserAllowSpeakAllUser) {
      recv_cancelForbidAll();
   }
   return 0;
}

void VhallChatWidget::open_filter_browser() {
   //qDebug()<<" VhallChatWidget::open_filter_browser"<<m_roomId;
   //QString url = QString("http:\/\/t.vhall.com\/webinar\/chatfilter\/%1").arg(m_roomId);
   QDesktopServices::openUrl(QUrl(m_filterurl));
}

int VhallChatWidget::forbid_someone(QString user_id, bool aim) {
   //QMessageBox::information(this, "提示", QString("%1, 禁言=%2").arg(user_id).arg(aim));
   MsgRQType type = aim ? e_RQ_UserProhibitSpeakOneUser : e_RQ_UserAllowSpeakOneUser;

   RQData oData;
   oData.m_eMsgType = type;
   user_id.toWCharArray(oData.m_oUserInfo.m_szUserID);
   SingletonVhallRightExtraWidgetIns::Instance().PostCRMessage(
      MSG_VHALLRIGHTEXTRAWIDGET_SENDMSG, &oData, sizeof(RQData));

   return 0;
}
int VhallChatWidget::kickout_someone(QString user_id, bool aim) {
   //QMessageBox::information(this, "提示", QString("%1, 踢出=%2").arg(user_id).arg(aim));
   MsgRQType type = aim ? e_RQ_UserKickOutOneUser : e_RQ_UserAllowJoinOneUser;

   RQData oData;
   oData.m_eMsgType = type;
   user_id.toWCharArray(oData.m_oUserInfo.m_szUserID);
   SingletonVhallRightExtraWidgetIns::Instance().PostCRMessage(
      MSG_VHALLRIGHTEXTRAWIDGET_SENDMSG, &oData, sizeof(RQData));

   return 0;
}
//被js循环调用，初始化pageinfo
QString VhallChatWidget::get_init() {
   QString ret;
   if (m_userName.length() == 0)
      return "";
   else {
      //针对全局设置，如果是没有公告权限，隐藏公告按键。
      bool bShowChatfilter = true;
      bool bShowChatForbidAll = true;
      int hostType = eHostType_Standard;
      int liveType = eLiveType_Live;
      VH::CComPtr<ICommonData> pCommonData = NULL;
      DEF_GET_INTERFACE_PTR(SingletonVhallRightExtraWidgetIns, PID_ICommonToolKit, IID_ICommonData, pCommonData);
      if (pCommonData) {
         liveType = pCommonData->GetLiveType();
         hostType = pCommonData->GetLoginUserHostType();
         if (hostType == eHostType_Standard) {
            //无极版用户(标准版)，聊天列表区域不显示【聊天过滤】按钮
            bShowChatfilter = false;
         }
         else if (liveType == eLiveType_TcLoginActive) {
            bShowChatfilter = pCommonData->IsShowChatFilterBtn();
            bShowChatForbidAll = pCommonData->IsShowChatForbidBtn();
         }
      }
      TRACE6("%s bShowChatfilter:%d bShowChatForbidAll:%d\n", __FUNCTION__, bShowChatfilter, bShowChatForbidAll);
      QString name = m_userName;
      name = name.replace("\"", "\\\"");
      ret = QString("{\"join_uid\":\"%1\",\"user_avatar\":\"%2\",\"join_uname\":\"%3\",\"role\":\"%4\",\"chat_filter_hide\":\"%5\",\"chat_forbid_hide\":\"%6\"}").arg(m_userId).arg(m_userImageUrl).arg(name).arg(m_bHost ? "host" : "guest").arg(bShowChatfilter ? "show" : "hide").arg(bShowChatForbidAll ? "show" : "hide");
   }
   return ret;
}
//被js循环调用，初始化“全体禁言”按钮状态
QString VhallChatWidget::get_initForbStatus() {
   QString ret = QString("%1").arg(m_initForbidchat);
   return ret;
}

// 从列表获取状态，用于右键菜单
/*1为已禁言，0为未禁言*/
int VhallChatWidget::getForbidStatus(QString userID) {
   VH::CComPtr<IVhallRightExtraWidgetLogic> pVhallRightExtraWidget;
   DEF_GET_INTERFACE_PTR(SingletonVhallRightExtraWidgetIns, PID_IVhallRightExtraWidget, IID_IVhallRightExtraWidgetLogic, pVhallRightExtraWidget, return 0);
   wchar_t wuid[1024] = { 0 };
   userID.toWCharArray(wuid);
   bool ret = pVhallRightExtraWidget->IsUserGag(wuid);
   return ret ? 1 : 0;
}
// 从列表获取状态，用于右键菜单
/*1为已踢出，1为未踢出*/
int VhallChatWidget::getKickoutStatus(QString userID) {
   VH::CComPtr<IVhallRightExtraWidgetLogic> pVhallRightExtraWidget;
   DEF_GET_INTERFACE_PTR(SingletonVhallRightExtraWidgetIns, PID_IVhallRightExtraWidget, IID_IVhallRightExtraWidgetLogic, pVhallRightExtraWidget, return 0);
   wchar_t wuid[1024] = { 0 };
   userID.toWCharArray(wuid);

   bool ret = pVhallRightExtraWidget->IsUserKick(wuid);
   return ret ? 1 : 0;
}

// 回车事件，发送文本到js
bool VhallChatWidget::eventFilter(QObject *obj, QEvent *e) {
   if (obj == ui->textEdit) {
      if (e->type() == QEvent::KeyPress) {
         QKeyEvent *event = static_cast<QKeyEvent*>(e);
         if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) {
            QString pText = ui->textEdit->toPlainText().trimmed();
            if (pText.isEmpty()) {
               ui->textEdit->clear();
               ui->textEdit->setFocus();
               return true;
            } else {
               pText.replace("\"", "\\\"");
               doSendMsg(pText);
               ui->textEdit->clear();
            }

            return true;
         }
      } else if (e->type() == QEvent::FocusIn) {
         if (ui->textEdit->toPlainText().length() == 0) {
            ui->textEdit->setPlaceholderText("");
         }
      } else if (e->type() == QEvent::FocusOut) {
         ui->textEdit->setPlaceholderText(QString::fromWCharArray(L"说点儿什么吧，按“回车键”发送"));
      } else
      if (e->type() == QEvent::FocusOut) {
         QWidget *w = qApp->focusWidget();
         if (w == NULL) {
            TRACE6("VhallChatWidget::eventFilter FocusOut ,focusWidget is NULL\n");
         } else {
            TRACE6("VhallChatWidget::eventFilter FocusOut ,focusWidget is [%s][%d][%d][%d][%d]\n",
                   w->objectName().toUtf8().data(),
                   w->x(),
                   w->y(),
                   w->width(),
                   w->height()
                   );

         }
      }
   }
   return QWidget::eventFilter(obj, e);
}

//調用js中的本地顯示和發出
void VhallChatWidget::doSendMsg(QString msg) {
   QString method = QString("preSendChatMsg(\"%1\")").arg(msg);
   if (m_pWebEngineView) {
      m_pWebEngineView->page()->runJavaScript(method);
   }
}

//向外發送消息
int VhallChatWidget::sendChatMsg(QString text) {
   //QMessageBox::information(this, "提示", text);

   RQData oData;
   oData.m_eMsgType = e_RQ_UserChat;
   strcpy(oData.m_wzText, text.toUtf8().data());

   SingletonVhallRightExtraWidgetIns::Instance().PostCRMessage(
      MSG_VHALLRIGHTEXTRAWIDGET_SENDMSG, &oData, sizeof(RQData));

   return 0;
}

//@某人，textedit插入文本
void VhallChatWidget::insertAtSomeone(QString uname) {
   ui->textEdit->insertPlainText(uname);
   ui->textEdit->moveCursor(QTextCursor::End);
   ui->textEdit->setFocus();
}

//@插入表情
void VhallChatWidget::insertArtist(QString artist) {
   ui->textEdit->insertPlainText(artist);
   ui->textEdit->moveCursor(QTextCursor::End);
   ui->textEdit->setFocus();
}

//限制輸入框字數
void VhallChatWidget::slotsTextChanged() {
   QString textContent = ui->textEdit->toPlainText();
   int length = textContent.count();
   int maxLength = 140; // 最大字符数
   if (length > maxLength) {
      int position = ui->textEdit->textCursor().position();
      QTextCursor textCursor = ui->textEdit->textCursor();
      textContent.remove(position - (length - maxLength), length - maxLength);
      ui->textEdit->setText(textContent);
      textCursor.setPosition(position - (length - maxLength));
      ui->textEdit->setTextCursor(textCursor);
   }
}

void VhallChatWidget::hideRightMouseMenuFunc() {
   QString method = QString("hideMenuFunc()");
   if (m_pWebEngineView) {
      m_pWebEngineView->page()->runJavaScript(method);
   }
}


void VhallChatWidget::RegisterWebEngineView() {
   if (m_pWebEngineView == NULL && m_pWebChannel == NULL) {
      m_pWebEngineView = new QWebEngineView(this);
      m_pWebChannel = new QWebChannel(this);
      if (m_pWebChannel && m_pWebEngineView) {
         m_pWebEngineView->setContextMenuPolicy(Qt::CustomContextMenu);
         QWebEngineSettings *pWebSettings = m_pWebEngineView->page()->settings();
         if (pWebSettings) {
            pWebSettings->setAttribute(QWebEngineSettings::AutoLoadImages, true);
            pWebSettings->setAttribute(QWebEngineSettings::JavascriptEnabled, true);
            pWebSettings->setAttribute(QWebEngineSettings::PluginsEnabled, true);
            pWebSettings->setAttribute(QWebEngineSettings::ScreenCaptureEnabled, true);
         }
         connect(m_pWebEngineView, SIGNAL(loadFinished(bool)), this, SLOT(slot_loadFinished(bool)));
         //如果不设置这就话，加载URL时会先显示以下白色背景。
         m_pWebEngineView->page()->setBackgroundColor(Qt::transparent);
         ui->verticalLayout_web->addWidget(m_pWebEngineView);
      }
      if (m_pWebChannel && m_pWebEngineView) {
         m_pWebChannel->registerObject("MainWindow", this);
         m_pWebEngineView->page()->setWebChannel(m_pWebChannel);
         m_pWebEngineView->page()->load(QUrl("qrc:/chat/chat_html/chat.html"));
      }
   }
}

void VhallChatWidget::LoadUserInfo(QString userName, QString userImageUrl, QString userId) {
   TRACE6("VhallChatWidget::InitUserInfo m_userName[%s] m_userImageUrl[%s] m_userId[%s]\n",
          m_userName.toLocal8Bit().data(),
          m_userImageUrl.toLocal8Bit().data(),
          m_userId.toLocal8Bit().data());
   QString method = QString("InitPageInfo(\"%1\", \"%2\", \"%3\")").arg(userName).arg(userImageUrl).arg(userId);
   if (m_pWebEngineView) {
      m_pWebEngineView->page()->runJavaScript(method);
   }
}

void VhallChatWidget::showEvent(QShowEvent *event) {
   if (m_pWebEngineView && m_bReloading) {
      m_bReloading = false;
      m_pWebEngineView->reload();
   }
}

void VhallChatWidget::SetReloadChatHtml(bool bReload) {
   m_bReloading = bReload;
}

bool VhallChatWidget::IsLoadUrlFinished() {
   return mIsLoadUrlFinished;
}



