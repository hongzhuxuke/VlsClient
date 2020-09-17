#include "stdafx.h"
#include "cinteractiveloginwdg.h"
#include "title_button.h"
#include "ConfigSetting.h"
#include "pathManager.h"
#include "IPluginWebServer.h"
#include "vhproxytest.h"

#define LEDT_NICKNAME_LENTH 30

CInteractiveLoginWdg::CInteractiveLoginWdg(QDialog *parent)
: CBaseDlg(parent) {
   ui.setupUi(this);
   ui.activityId->setValidator(new QIntValidator(0, 999999999));
   ui.label->setOpenExternalLinks(true);
   setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint | Qt::WindowStaysOnTopHint);
   setAttribute(Qt::WA_TranslucentBackground);
   setAutoFillBackground(false);


   m_pBtnClose = new TitleButton(this);
   m_pBtnClose->loadPixmap(":/sysButton/close_button");
   m_pBtnClose->setToolTip(QString::fromWCharArray(L"关闭窗口"));
   ui.layout_close->addWidget(m_pBtnClose);

   connect(m_pBtnClose, SIGNAL(clicked()), this, SLOT(Slot_Close()));
   connect(ui.btn_enter, SIGNAL(clicked()), this, SLOT(Slot_JoinInteractive()));
   connect(ui.ledtNick, SIGNAL(textEdited(const QString &)),this,SLOT(Slot_NickNameEdited(const QString&)));
   m_pBtnClose->setFocusPolicy(Qt::NoFocus);

   ui.widget_title->installEventFilter(this);
   ui.activityId->setPlaceholderText(QString::fromWCharArray(L"请输入直播ID"));
   this->setWindowTitle(QString::fromWCharArray(L"嘉宾连麦"));
   pixmap = QPixmap(":/interactivity/joinActivity");
   QString styleSheet = "QToolButton{border-image: url(\":/interactivity/enterLive\");}"
      "QToolButton:hover{border-image: url(\":/interactivity/enterLiveHover\");}"
      "QToolButton:pressed{border-image: url(\":/interactivity/enterLive\");}";

   ui.btn_enter->setStyleSheet(styleSheet);

   m_pWaiting = new VhallWaiting(this);
   m_pWaiting->SetPixmap(":/interactivity/cb");
   m_pWaiting->Append(":/interactivity/c1");
   m_pWaiting->Append(":/interactivity/c2");
   m_pWaiting->Append(":/interactivity/c3");
   m_pWaiting->Append(":/interactivity/c4");
   m_pWaiting->Append(":/interactivity/c5");
   m_pWaiting->Append(":/interactivity/c6");
   m_pWaiting->Append(":/interactivity/c7");
   m_pWaiting->Append(":/interactivity/c8");
   m_pWaiting->Append(":/interactivity/c9");
   m_pWaiting->Append(":/interactivity/c10");
   m_pWaiting->Append(":/interactivity/c11");
   m_pWaiting->Append(":/interactivity/c12");
}

CInteractiveLoginWdg::~CInteractiveLoginWdg() {

}

void CInteractiveLoginWdg::Slot_Close() {
   reject();
}

bool CInteractiveLoginWdg::eventFilter(QObject *obj, QEvent *e) {
   if (ui.widget_title == obj) {
      if (e->type() == QEvent::MouseButtonPress) {
         this->pressPoint = this->cursor().pos();
         this->startPoint = this->pos();
      } else if (e->type() == QEvent::MouseMove) {
         int dx = this->cursor().pos().x() - this->pressPoint.x();
         int dy = this->cursor().pos().y() - this->pressPoint.y();
         this->move(this->startPoint.x() + dx, this->startPoint.y() + dy);
      }
   } else if (this == obj) {
      if (e->type() == QEvent::Move) {
         m_pWaiting->Repos();
      }
   }

   return QWidget::eventFilter(obj, e);
}

void CInteractiveLoginWdg::paintEvent(QPaintEvent *event) {
   QPainter painter(this);
   painter.drawPixmap(rect(), this->pixmap);
   QWidget::paintEvent(event);
}

void CInteractiveLoginWdg::closeEvent(QCloseEvent *) {
   Slot_Close();
}

void CInteractiveLoginWdg::Slot_JoinInteractive() {
   mRoomID = ui.activityId->text().trimmed();
   if (mRoomID.length() != 9) {
      ui.label->setText(QString::fromWCharArray(L"请输入9位直播ID"));
      return;
   }

   mNickName = ui.ledtNick->text().trimmed();
   if (mNickName.isEmpty()) {
      ui.label->setText(QString::fromWCharArray(L"请输入参会昵称"));
      return;
   }

   mRoomPwd = ui.ledPassword->text().trimmed();
   if (mRoomPwd.isEmpty()) {
      ui.label->setText(QString::fromWCharArray(L"请输入参会口令"));
      return;
   }
   QString configPath = CPathManager::GetConfigPath();
   mDoMain = ConfigSetting::ReadString(configPath, GROUP_DEFAULT, KEY_URL_DOMAIN, "");
   QString clientUnique = ConfigSetting::ReadString(configPath, GROUP_DEFAULT, USER_PWD_LOGIN_CLIENT_UNIQUE, "");
   QString url;
   url = mDoMain + "/api/vhallassistant/auth/passwordlogin?webinar_id=%0&password=%1&nick_name=%2&client_unique=%3";
   url = url.arg(mRoomID).arg(mRoomPwd).arg(mNickName).arg(clientUnique);
   m_pWaiting->Show();
   QByteArray ba;
   HttpRequest req;
   if (req.VHGetRet(url, 10000, ba)) {//登陆信息验证
      QJsonDocument doc = QJsonDocument::fromJson(ba);
      QJsonObject obj = doc.object();
      int code = obj["code"].toInt();
      QString msg = obj["msg"].toString();
      if (code == 200) {
         QJsonObject data = obj["data"].toObject();
         mPluginsUrl = data["plugins_url"].toString();
         mlistUrl = data["list_url"].toString();
         mAuthToken = data["token"].toString();
         mUserName = data["nick_name"].toString();
         clientUnique = data["client_unique"].toString();
         if (!clientUnique.isEmpty()) {
            ConfigSetting::writeValue(configPath, GROUP_DEFAULT, USER_PWD_LOGIN_CLIENT_UNIQUE, clientUnique);
         }
         mUserImage = "http:" + data["avatar"].toString();
         accept();
         //if (IsEnableJoinActive()) {
         //   accept();
         //}
      } else {
         ui.label->setText(msg);
      }
   }
   else {
      ui.label->setText(QStringLiteral("进入互动连麦失败,请重试"));
   }
   m_pWaiting->Close();
   return;
}

void CInteractiveLoginWdg::Slot_NickNameEdited(const QString&text) {
   if (text.trimmed().length() > LEDT_NICKNAME_LENTH) {
      ui.label->setText(QString::fromWCharArray(L"参会昵称最多输入30个字符"));
   }
   ui.ledtNick->setMaxLength(LEDT_NICKNAME_LENTH);
   ui.ledtNick->setText(text.trimmed());
   ui.ledtNick->setMaxLength(32767);
   if (ui.ledtNick->text().trimmed().length() < LEDT_NICKNAME_LENTH) {
      ui.label->setText("");
   }
}

bool CInteractiveLoginWdg::IsEnableJoinActive() {
   IPluginWebServer *m_pPluginHttpServer = GetHttpServerInstance();
   if (m_pPluginHttpServer && !mNickName.isEmpty()) {
      char *encodeUrl = m_pPluginHttpServer->HttpUrlEncode(mNickName.toStdString().c_str(), mNickName.toStdString().length(), false);
      if (encodeUrl != NULL) {
         mNickName = QString(encodeUrl);
         free(encodeUrl);
         encodeUrl = NULL;
      }
   }
   //获取主持人在线状态、p判断加入的活动有效性、获取msg_token等参数。
   QString url = mDoMain + "/api/vhallassistant/webinar/join-webinar?webinar_id=" + mRoomID + "&nick_name=" + mNickName + "&password=" + mRoomPwd + "&params_verify_token=" + mAuthToken;   
   QByteArray dataByte;
   QString userId;
   QString userSig;

   VHStartParam param;
   memset(&param, 0, sizeof(VHStartParam));
   param.bLoadExtraRightWidget = true;
   param.bConnectToVhallService = false;

   HttpRequest req;
   if (!req.VHGetRet(url, 10000, dataByte)) {
      ui.label->setText(QString::fromWCharArray(L"进入互动连麦失败,请重试"));
      TRACE6("MainUILogic::SlotToJoinActivity Failed1\n");
      return false;
   }

   QJsonDocument doc = QJsonDocument::fromJson(dataByte);
   QJsonObject obj = doc.object();
   int code = obj["code"].toInt();
   if (code != 200) {
      QString msg = obj["msg"].toString();
      ui.label->setText(msg);
      return false;
   }
   return true;
}

QString CInteractiveLoginWdg::GetUserSig(const QString& msgToken) {
   QString userSig;
   QString url = mDoMain + "/api/client/v1/clientapi/gen-sig?token=" + msgToken;
   QByteArray dataByte;
   HttpRequest req;
   if (!req.VHGetRet(url, 10000, dataByte)) {
      return "";
   }

   QJsonDocument doc = QJsonDocument::fromJson(dataByte);
   QJsonObject obj = doc.object();
   int code = obj["code"].toInt();
   if (code != 200) {
      return "";
   }

   userSig = obj["data"].toString();
   return userSig;
}


QString CInteractiveLoginWdg::GetRoomID() {
   return mRoomID;
}

QString CInteractiveLoginWdg::GetRoomPwd() {
   return mRoomPwd;
}

QString CInteractiveLoginWdg::GetUserNickName() {
   return mUserName;
}

QString CInteractiveLoginWdg::GetToken() {
   return mAuthToken;
}

QString CInteractiveLoginWdg::GetHeadImageUrl() {
   return mUserImage;
}

QString CInteractiveLoginWdg::GetPluginUrl() {
   return mPluginsUrl;
}

