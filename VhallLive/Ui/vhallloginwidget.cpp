#include "vhallloginwidget.h"
#include "ui_vhallloginwidget.h"
#include <QDesktopServices>
#include <QUrl>
#include <QDebug>
#include <string>
#include <iostream>
#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>
#include <QKeyEvent>
#include <QtNetwork\QNetworkProxy>

#include <string>
#include <vector>

#include "ConfigSetting.h"
#include "vhproxytest.h"
#include "AlertDlg.h"
#include "title_button.h"
#include "vhnetworkproxy.h"
#include "vhnetworktips.h"
#include "vhsqllite.h"
#include "DebugTrace.h"
#include "pathManage.h"
#include "pathManager.h"
#include "pub.Struct.h"
#include "pub.Const.h"
#include "Msg_MainUI.h"
#include "RuntimeInstance.h"
#include "IPluginWebServer.h"
#include "cinteractiveloginwdg.h"

//std::wstring GetAppPath();

VhallLoginWidget::VhallLoginWidget(QDialog *parent) :
CBaseDlg(parent),
ui(new Ui::VhallLoginWidget) {
   ui->setupUi(this);

   this->setWindowFlags(Qt::FramelessWindowHint
                        | Qt::WindowMinimizeButtonHint
                        /*| Qt::WindowStaysOnTopHint*/
                        | Qt::Dialog
                        );
   setAttribute(Qt::WA_TranslucentBackground);
   setAutoFillBackground(false);
   setWindowTitle(QString::fromWCharArray(L"µ«¬º"));

   this->installEventFilter(this);
   ui->login_account->installEventFilter(this);
   ui->login_phone->installEventFilter(this);
   ui->login_forgot_password->installEventFilter(this);
   ui->login_register->installEventFilter(this);
   ui->login_btn->installEventFilter(this);
   ui->login_get_verify_code->installEventFilter(this);
   ui->lineEdit_verify_code->installEventFilter(this);
   ui->checkBox->installEventFilter(this);
   ui->label_proxy->installEventFilter(this);
   ui->label_interactive->installEventFilter(this);

   ui->tip_phone->clear();
   ui->tip_verify_code->clear();
   ui->tip_account->clear();
   ui->tip_password->clear();

   //µ±µ«¬ºøÚ°¢√‹¬ÎøÚ°¢ ÷ª˙∫≈ ‰»ÎøÚ”–Œƒ◊÷ ±£¨Õº±Í∏ﬂ¡¡£¨√ª”–Œƒ◊÷ ±£¨Õº±Í≤ª∏ﬂ¡¡
   connect(ui->lineEdit_account, SIGNAL(textChanged(const QString &)), this, SLOT(on_lineEidt_account_textChanged(const QString &)));
   connect(ui->lineEdit_password, SIGNAL(textChanged(const QString &)), this, SLOT(on_lineEdit_password_textChanged(const QString &)));
   connect(ui->lineEdit_phone, SIGNAL(textChanged(const QString &)), this, SLOT(on_lineEdit_phone_textChanged(const QString &)));
   //ui->checkBox->installEventFilter(this);

   QPixmap bg_pixmap = QPixmap(":/login/img/loginDialog/login_background.png");
   QPixmap bg_pixmap_shadow = QPixmap(":/login/img/loginDialog/login_background_shadow.png");
   pixmap = bg_pixmap_shadow;
   QPainter p(&pixmap);
   p.drawPixmap(bg_pixmap.rect(), bg_pixmap);


   ui->login_account->setStyleSheet(QString::fromWCharArray(L"QLabel{color:#FF3333;font: 14pt \"Œ¢»Ì—≈∫⁄\";}"));
   ui->login_phone->setStyleSheet(QString::fromWCharArray(L"QLabel{color:#FEFEFE;font: 14pt \"Œ¢»Ì—≈∫⁄\";}QLabel:hover{color:#DDDDDD;}"));
   ui->stackedWidget->setCurrentIndex(0);

   //ui->label->setStyleSheet(QString::fromWCharArray(L"QLabel{color:#d5d8d7;font: 10pt \"Œ¢»Ì—≈∫⁄\";}"));

   //TitleButton *m_pBtnClose = new TitleButton(this);;//πÿ±’
   ui->BtnClose->loadPixmap(":/sysButton/close_button");
   ui->BtnClose->setToolTip(tr("πÿ±’¥∞ø⁄"));
   ui->BtnClose->setFocusPolicy(Qt::NoFocus);

   connect(ui->BtnClose, SIGNAL(clicked()), this, SLOT(close()));
   //ui->layout_title_content->addWidget(m_pBtnClose);

   QString chkVerUrl;
   //std::wstring confPath = GetAppPath() + CONFIGPATH;
   mConfigPath = CPathManager::GetConfigPath();
   QString token = ConfigSetting::ReadString(mConfigPath, GROUP_DEFAULT, USER_AUTH_TOKEN, "");
   SetAuthToken(token);

   connect(&mTimer, SIGNAL(timeout()), this, SLOT(timeout()));
   mTimer.start(800);
   setFocusPolicy(Qt::NoFocus);
   QString toolConfPath = CPathManager::GetAppDataPath() + QString::fromStdWString(VHALL_TOOL_CONFIG);
   int hideLogo = ConfigSetting::ReadInt(toolConfPath, GROUP_DEFAULT, KEY_VHALL_LOGO_HIDE, 0);
   if (hideLogo == 1) {
      ui->widget_vhallLogo->hide();
   }

   mInterActiveLoginWdg = new CInteractiveLoginWdg();
   if (mInterActiveLoginWdg){
      mInterActiveLoginWdg->hide();
   }
}

VhallLoginWidget::~VhallLoginWidget() {
   if (mInterActiveLoginWdg) {
      delete mInterActiveLoginWdg;
      mInterActiveLoginWdg = NULL;
   }
   mTimer.stop();
   delete ui;
}
bool VhallLoginWidget::eventFilter(QObject *o, QEvent *e) {
   if (o == ui->login_account) {
      int currentIndex = ui->stackedWidget->currentIndex();
      if (e->type() == QEvent::MouseButtonPress) {
         if (currentIndex == 0) {
            ui->login_account->setStyleSheet(QString::fromWCharArray(L"QLabel{color:#FF3333;font: 14pt \"Œ¢»Ì—≈∫⁄\";}QLabel:hover{color:#FF3333;}"));
         } else {
            ui->login_account->setStyleSheet(QString::fromWCharArray(L"QLabel{color:#FEFEFE;font: 14pt \"Œ¢»Ì—≈∫⁄\";}QLabel:hover{color:#DDDDDD;}"));
         }
         this->repaint();
      } else if (e->type() == QEvent::MouseButtonRelease) {
         if (ui->BtnClose->isEnabled()) {
            ui->login_account->setStyleSheet(QString::fromWCharArray(L"QLabel{color:#FF3333;font: 14pt \"Œ¢»Ì—≈∫⁄\";}"));
            ui->login_phone->setStyleSheet(QString::fromWCharArray(L"QLabel{color:#FEFEFE;font: 14pt \"Œ¢»Ì—≈∫⁄\";}QLabel:hover{color:#DDDDDD;}"));
            ui->stackedWidget->setCurrentIndex(0);
         }
         this->repaint();
      }
   } else if (o == ui->login_phone) {
      if (e->type() == QEvent::MouseButtonPress) {
         int currentIndex = ui->stackedWidget->currentIndex();
         if (currentIndex == 1) {
            ui->login_phone->setStyleSheet(QString::fromWCharArray(L"QLabel{color:#FF3333;font: 14pt \"Œ¢»Ì—≈∫⁄\";}QLabel:hover{color:#FF3333;}"));
         } else {
            ui->login_phone->setStyleSheet(QString::fromWCharArray(L"QLabel{color:#FEFEFE;font: 14pt \"Œ¢»Ì—≈∫⁄\";}QLabel:hover{color:#DDDDDD;}"));
         }
         this->repaint();
      } else if (e->type() == QEvent::MouseButtonRelease) {
         if (ui->BtnClose->isEnabled()) {
            ui->login_phone->setStyleSheet(QString::fromWCharArray(L"QLabel{color:#FF3333;font: 14pt \"Œ¢»Ì—≈∫⁄\";}"));
            ui->login_account->setStyleSheet(QString::fromWCharArray(L"QLabel{color:#FEFEFE;font: 14pt \"Œ¢»Ì—≈∫⁄\";}QLabel:hover{color:#DDDDDD;}"));
            ui->stackedWidget->setCurrentIndex(1);
         }

         this->repaint();
      }
   } else if (o == ui->login_forgot_password) {

      if (e->type() == QEvent::MouseButtonPress) {
         ui->login_forgot_password->setStyleSheet(QString::fromWCharArray(L"QLabel{color:#626567;}QLabel:hover{color:#626567;}"));
         this->repaint();
      } else if (e->type() == QEvent::MouseButtonRelease) {
         ui->login_forgot_password->setStyleSheet(QString::fromWCharArray(L"QLabel{color:#626567;}QLabel:hover{color:#FEFEFE;}"));
         this->repaint();

         QString domain = ConfigSetting::ReadString(mConfigPath, GROUP_DEFAULT, KEY_URL_DOMAIN, "http://e.vhall.com");
         QDesktopServices::openUrl(QUrl(domain + "/auth/forgot"));
      }
   } else if (o == ui->login_register) {
      if (e->type() == QEvent::MouseButtonPress) {
         ui->login_register->setStyleSheet(QString::fromWCharArray(L"QLabel{color:#626567;}QLabel:hover{color:#626567;}"));
         this->repaint();
      } else if (e->type() == QEvent::MouseButtonRelease) {
         ui->login_register->setStyleSheet(QString::fromWCharArray(L"QLabel{color:#626567;}QLabel:hover{color:#FEFEFE;}"));
         this->repaint();

         QString domain = ConfigSetting::ReadString(mConfigPath, GROUP_DEFAULT, KEY_URL_DOMAIN, "http://e.vhall.com");
         QDesktopServices::openUrl(QUrl(domain + "/auth/register"));
      }
   } else if (o == this) {
      if (e->type() == QEvent::MouseButtonPress) {
         this->pressPoint = this->cursor().pos();
         this->startPoint = this->pos();
      } else if (e->type() == QEvent::MouseMove) {
         int dx = this->cursor().pos().x() - this->pressPoint.x();
         int dy = this->cursor().pos().y() - this->pressPoint.y();
         this->move(this->startPoint.x() + dx, this->startPoint.y() + dy);
      }
   } else if (o == ui->checkBox) {
      if (e->type() == QEvent::Enter) {
         QString styleSheet =
            "QCheckBox{"
            "color:#626567"
            "}"
            "QCheckBox:hover{"
            "color:#b8b8b8;"
            "}"
            "QCheckBox:pressed{"
            "color:#626567"
            "}"
            "QCheckBox:checked{"
            "color:#b8b8b8;"
            "}"
            "QCheckBox::indicator:unchecked{"
            "border-image:url(:/login/img/loginDialog/login_auto_hover.png);"
            "width:12px;"
            "height:12px;"
            "}"
            "QCheckBox::indicator:checked{"
            "border-image:url(:/login/img/loginDialog/login_auto_checked.png);"
            "background:transparent;"
            "width:12px;"
            "height:12px;"
            "}"
            ;
         ui->checkBox->setStyleSheet(styleSheet);
         repaint();
      } else if (e->type() == QEvent::Leave) {
         QString styleSheet =
            "QCheckBox{"
            "color:#626567"
            "}"
            "QCheckBox:hover{"
            "color:#b8b8b8;"
            "}"
            "QCheckBox:pressed{"
            "color:#626567"
            "}"
            "QCheckBox:checked{"
            "color:#b8b8b8;"
            "}"
            "QCheckBox::indicator:unchecked{"
            "border-image:url(:/login/img/loginDialog/login_auto.png);"
            "width:12px;"
            "height:12px;"

            "}"
            "QCheckBox::indicator:checked{"
            "border-image:url(:/login/img/loginDialog/login_auto_checked.png);"
            "background:transparent;"
            "width:12px;"
            "height:12px;"
            "}"
            ;

         ui->checkBox->setStyleSheet(styleSheet);
         repaint();
      } else if (e->type() == QEvent::MouseButtonPress) {
         this->repaint();
      } else if (e->type() == QEvent::MouseButtonRelease) {
         this->repaint();
      }
   } else if (o == ui->login_btn) {
      if (e->type() == QEvent::MouseButtonRelease) {
         if (ui->login_btn->isEnabled()) {
            Login();
         }
      }
   } else if (o == ui->login_get_verify_code) {
      if (e->type() == QEvent::MouseButtonRelease) {
         if (ui->BtnClose->isEnabled() && ui->login_get_verify_code->isEnabled()) {
            GetVerifyCode();
         }
      }
   } else if (o == ui->label_proxy) {
      if (e->type() == QEvent::MouseButtonRelease) {
         slot_OnClickedProxySet();
      }
   } else if (o == ui->label_interactive) {
      if (e->type() == QEvent::MouseButtonRelease) {
         ShowJoinActiveWnd();
      }
   } else if (o == ui->lineEdit_verify_code) {
      if (e->type() == QEvent::KeyPress) {
         QKeyEvent *k = dynamic_cast<QKeyEvent *>(e);

         if (k) {
            switch (k->key()) {
            case Qt::Key_Backspace:

            case Qt::Key_Enter:
            case Qt::Key_Delete:

            case Qt::Key_Home:
            case Qt::Key_End:

            case Qt::Key_Left:
            case Qt::Key_Up:
            case Qt::Key_Right:
            case Qt::Key_Down:
            case Qt::Key_Tab:
            case Qt::Key_0:
            case Qt::Key_1:
            case Qt::Key_2:
            case Qt::Key_3:
            case Qt::Key_4:
            case Qt::Key_5:
            case Qt::Key_6:
            case Qt::Key_7:
            case Qt::Key_8:
            case Qt::Key_9:
            case Qt::Key_Control:

               break;
            default:
               k->ignore();
               return true;

               break;
            }
         }
      }
   }

   return QWidget::eventFilter(o, e);
}
void VhallLoginWidget::paintEvent(QPaintEvent *) {
   QPainter painter(this);
   painter.drawPixmap(rect(), this->pixmap);
}
void VhallLoginWidget::Login() {
   m_startLogInTime = QDateTime::currentDateTime();
   QString strLogInUId;
   ui->BtnClose->setEnabled(false);
   this->repaint();

   mLoginFailedReason = "";
   bool loginByAccount = false;

   if (ui->stackedWidget->currentIndex() == 0) {
      QString account = ui->lineEdit_account->text();
      QString password = ui->lineEdit_password->text();
      ConfigSetting::writeValue(mConfigPath, GROUP_DEFAULT, LOG_ID_BASE, account);
      strLogInUId = account;
      loginByAccount = true;
      if (account.isEmpty()) {
         ui->tip_account->clear();
         ui->tip_password->clear();
         ui->tip_phone->clear();
         ui->tip_verify_code->clear();
         ui->tip_account->setText(QString::fromWCharArray(L"«Î ‰»Î’À∫≈"));
         mLoginFailedReason = "";
         LoginFailed();
         return;
      }
      if (password.isEmpty()) {
         ui->tip_account->clear();
         ui->tip_password->clear();
         ui->tip_phone->clear();
         ui->tip_verify_code->clear();
         ui->tip_password->setText(QString::fromWCharArray(L"«Î ‰»Î√‹¬Î"));
         mLoginFailedReason = "";
         LoginFailed();
         return;
      }
   }
   else {
      QString phoneNumber = ui->lineEdit_phone->text();
      QString verifyCode = ui->lineEdit_verify_code->text();
      strLogInUId = phoneNumber;
      ConfigSetting::writeValue(mConfigPath, GROUP_DEFAULT, LOG_ID_BASE, phoneNumber);
      if (phoneNumber.isEmpty()) {
         ui->tip_account->clear();
         ui->tip_password->clear();
         ui->tip_phone->clear();
         ui->tip_verify_code->clear();
         ui->tip_phone->setText(QString::fromWCharArray(L"«Î ‰»Î ÷ª˙∫≈"));
         mLoginFailedReason = "";
         LoginFailed();
         return;
      }
      if (!IsPhoneNumberEffective(phoneNumber)) {
         ui->tip_account->clear();
         ui->tip_password->clear();
         ui->tip_phone->clear();
         ui->tip_verify_code->clear();
         ui->tip_phone->setText(QString::fromWCharArray(L" ÷ª˙∫≈∏Ò Ω¥ÌŒÛ"));
         mLoginFailedReason = "";
         LoginFailed();
         return;
      }
      if (verifyCode.isEmpty()) {
         ui->tip_account->clear();
         ui->tip_password->clear();
         ui->tip_phone->clear();
         ui->tip_verify_code->clear();
         ui->tip_verify_code->setText(QString::fromWCharArray(L"—È÷§¬Î¥ÌŒÛ"));
         mLoginFailedReason = "";
         LoginFailed();
         return;
      }
   }

   ui->tip_account->clear();
   ui->tip_password->clear();
   ui->tip_phone->clear();
   ui->tip_verify_code->clear();
   Tip("", mLoginFailedReason);
   repaint();
   QString domain = ConfigSetting::ReadString(mConfigPath, GROUP_DEFAULT, KEY_URL_DOMAIN, "");
   QString url;

   QString pwd = ui->lineEdit_password->text();
   m_pPluginHttpServer = GetHttpServerInstance();
   if (m_pPluginHttpServer && !pwd.isEmpty()) {
      char *encodeUrl = m_pPluginHttpServer->HttpUrlEncode(pwd.toStdString().c_str(), pwd.toStdString().length(), false);
      if (encodeUrl != NULL) {
         pwd = QString(encodeUrl);
         free(encodeUrl);
         encodeUrl = NULL;
      }
   }
   if (loginByAccount) {
      url = domain + "/api/vhallassistant/auth/login?account=%0&password=%1";
      url = url.arg(ui->lineEdit_account->text()).arg(pwd);
   } else {
      url = domain + "/api/vhallassistant/auth/phonelogin?phone=%0&password=%1";
      url = url.arg(ui->lineEdit_phone->text()).arg(ui->lineEdit_verify_code->text());
   }

   //¥˙¿Ì≈‰÷√
   int is_http_proxy = ConfigSetting::ReadInt(mConfigPath, GROUP_DEFAULT, PROXY_OPEN, 0);
   if (is_http_proxy) {
      QString host = ConfigSetting::ReadString(mConfigPath, GROUP_DEFAULT, PROXY_HOST, "");
      int port = ConfigSetting::ReadInt(mConfigPath, GROUP_DEFAULT, PROXY_PORT, 80);
      QString usr = ConfigSetting::ReadString(mConfigPath, GROUP_DEFAULT, PROXY_USERNAME, "");
      QString pwd = ConfigSetting::ReadString(mConfigPath, GROUP_DEFAULT, PROXY_PASSWORD, "");
      QNetworkProxy proxy;
      proxy.setType(QNetworkProxy::HttpProxy);
      proxy.setHostName(host);
      proxy.setPort(port);
      proxy.setUser(usr);
      proxy.setPassword(pwd);
      QNetworkProxy::setApplicationProxy(proxy);
   } else {
      QNetworkProxy proxy;
      proxy.setType(QNetworkProxy::NoProxy);
      QNetworkProxy::setApplicationProxy(proxy);
   }

   QByteArray ba;
   HttpRequest req;
   if (req.VHGetRet(url, 10000, ba)) {//µ«¬Ω–≈œ¢—È÷§
      qDebug() << "LoginEnd Successed" << ba;
      loginLogReport(true);
      QJsonDocument doc = QJsonDocument::fromJson(ba);
      QJsonObject obj = doc.object();
      int code = obj["code"].toInt();
      QString msg = obj["msg"].toString();
      if (code == 200) {
         QJsonObject data = obj["data"].toObject();
         mPluginsUrl = data["plugins_url"].toString();
         mlistUrl = data["list_url"].toString();
         mtoken = data["token"].toString();
         mUserName = data["nick_name"].toString();
         mUserImage = "http:" + data["avatar"].toString();
         qDebug() << "VhallLoginWidget::Login() " << mPluginsUrl << mlistUrl << mtoken;
         if (loginByAccount) {
            QJsonObject obj;
            obj["account"] = ui->lineEdit_account->text();
            if (ui->checkBox->isChecked()) {
               obj["password"] = ui->lineEdit_password->text();
            }
            QJsonDocument doc;
            doc.setObject(obj);
            mAuthToken = doc.toJson(QJsonDocument::Compact).toBase64();
         } else {
            mAuthToken = "";
         }
         LoginSuccessed(loginByAccount ? ui->lineEdit_account->text() : ui->lineEdit_phone->text());
         return;
      } else {
         //’À∫≈µ«¬º
         if (loginByAccount) {
            if (code / 100 == 4) {
               ui->tip_account->setText(msg);
            } else if (code / 100 == 5) {
               ui->tip_password->setText(msg);
            } else {
               ui->tip_login_failed->setText(msg);
               mLoginFailedReason = msg;
            }
         }
         // ÷ª˙∫≈£¨—È÷§¬Î
         else {
            if (code / 100 == 4) {
               ui->tip_phone->setText(msg);
            } else if (code / 100 == 5) {
               ui->tip_verify_code->setText(msg);
            } else {
               ui->tip_login_failed->setText(msg);
               mLoginFailedReason = msg;
            }
         }
      }
      //µ«¬Ω≥…π¶  strLogInUId
   } else {
      VHNetworkTips tips(this);
      tips.exec();

      if (tips.ProxyConfigure()) {
         VHNetworkProxy proxy(this);
         proxy.exec();
      }

      mLoginFailedReason = QString::fromWCharArray(L"µ«¬º ß∞‹");
   }
   LoginFailed();
}

void VhallLoginWidget::LoginFailed() {
   loginLogReport();
   Tip(QString::fromWCharArray(L"µ«¬º ß∞‹"), mLoginFailedReason);
   ui->BtnClose->setEnabled(true);
   this->repaint();
}

void VhallLoginWidget::Tip(QString title,QString msg) {
#if 0
   AlertDlg *alertDlg = new AlertDlg(msg, false, this);
   alertDlg->SetTitle(title);
   alertDlg->exec();
   delete alertDlg;
#else
   ui->tip_login_failed->setText(msg);
#endif
}
void VhallLoginWidget::timeout() {
   uint lastTime = ConfigSetting::ReadInt(mConfigPath, GROUP_DEFAULT, GET_VERIFY_CODE_TIME, 0);
   uint currentTime = QDateTime::currentDateTime().toTime_t();

   if (lastTime + 60 > currentTime) {
      ui->login_get_verify_code->setEnabled(false);
      uint diff = currentTime - lastTime;
      diff = 60 - diff;
      qDebug() << "timeout" << diff;
      ui->login_get_verify_code->setText(QString::number(diff) + QString::fromWCharArray(L"s∫Û÷ÿ–¬ªÒ»°"));
   } else {
      ui->login_get_verify_code->setEnabled(true);
      ui->login_get_verify_code->setText("");
   }

   repaint();
}
QString VhallLoginWidget::GetAuthToken() {
   return mAuthToken;
}
bool VhallLoginWidget::IsPhoneNumberEffective(QString phoneNumber) {
   bool ok = false;
   qulonglong number = phoneNumber.toULongLong(&ok);
   if (!ok) {
      return false;
   }

   QString tmp = QString::number(number);
   if (tmp.toULongLong() != phoneNumber.toULongLong()) {
      return false;
   }

   if (phoneNumber.length() != 11) {
      return false;
   }

   return true;
}

void VhallLoginWidget::SetAuthToken(QString token) {
   if (token == "") {
      return;
   }

   QJsonDocument doc = QJsonDocument::fromJson(QByteArray::fromBase64(token.toUtf8()));
   QJsonObject obj = doc.object();
   QString account = obj["account"].toString();
   QString password = obj["password"].toString();

   if (!account.isEmpty() && !password.isEmpty()) {
      ui->checkBox->setChecked(true);
   }

   ui->lineEdit_account->setText(account);
   ui->lineEdit_password->setText(password);
}

void	SplitString(const std::string& str, const std::string& sKey, std::vector<std::string>& vecResult, BOOL bUrl);
unsigned long long GetLongVersion(std::string sVersion) {
   int				i = 0;
   std::vector<std::string>	vers;
   unsigned long long 			i64Ret = 0;
   SplitString(sVersion, ".", vers, false);
   if (vers.size() != 4)
      return 0;
   for (i = 0; i < 4; i++) {
      if (i == 0)
         i64Ret = atol(vers[i].c_str());
      else {
         i64Ret = i64Ret << 16;
         i64Ret |= atol(vers[i].c_str());
      }
   }
   return i64Ret;
}

unsigned long long GetCurVersion(QString file, QString &currentVersion) {
   unsigned long long 				_i64Ver = 0;
   DWORD				dwVerHnd = 0;
   VS_FIXEDFILEINFO*	pFileInfo = NULL;
   unsigned int		dwInfoSize = 0;
   char	*			pVersionInfo = NULL;
   wchar_t  exeFile[1024] = { 0 };
   file.toWCharArray(exeFile);
   DWORD				dwVerInfoSize = GetFileVersionInfoSizeW(exeFile, &dwVerHnd);

   if (dwVerInfoSize) {
      pVersionInfo = new char[dwVerInfoSize];

      GetFileVersionInfoW(exeFile, dwVerHnd, dwVerInfoSize, pVersionInfo);
      VerQueryValue(pVersionInfo, (LPTSTR)("\\"), (void**)&pFileInfo, &dwInfoSize);

      currentVersion = QString("%1.%2.%3.%4")
         .arg(HIWORD(pFileInfo->dwFileVersionMS))
         .arg(LOWORD(pFileInfo->dwFileVersionMS))
         .arg(HIWORD(pFileInfo->dwFileVersionLS))
         .arg(LOWORD(pFileInfo->dwFileVersionLS));

      _i64Ver = pFileInfo->dwProductVersionMS;
      _i64Ver = _i64Ver << 32;
      _i64Ver |= pFileInfo->dwProductVersionLS;

      delete[] pVersionInfo;
      pVersionInfo = NULL;
   }
   return _i64Ver;
}

void VhallLoginWidget::LoginSuccessed(QString account) {
   this->accept();
   QDir::setCurrent(qApp->applicationDirPath());

   QString token = GetAuthToken();
   if (token != "") {
      ConfigSetting::writeValue(mConfigPath, GROUP_DEFAULT, USER_AUTH_TOKEN, token);
   }

   QString cv;
   //accept version
   unsigned long long iv = GetCurVersion(qApp->applicationFilePath(), cv);

   VHSqlLite sql;
   if (sql.Init("vhall.db")) {

      TRACE6("VhallLoginWidget::LoginSuccessed Init Successed!\n");

      QSqlQuery query = sql.GetQuery();
      QString sqlStr = QString("SELECT count(*) FROM t_user_record where user_account='%1'").arg(account);
      qDebug() << "[SQL]" << sqlStr;

      if (query.exec(sqlStr)) {
         TRACE6("VhallLoginWidget::LoginSuccessed Select Successed!\n");

         int currentCount = -1;
         while (query.next()) {
            currentCount = query.value(0).toInt();
            break;
         }

         TRACE6("VhallLoginWidget::LoginSuccessed Select Count = %d!\n", currentCount);
         if (currentCount == 0) {
            if (query.exec(QString("INSERT INTO t_user_record VALUES ('%1','%2')").arg(account).arg(cv))) {
               TRACE6("VhallLoginWidget::LoginSuccessed Insert Successed!\n");
               mShowTeaching = true;
            } else {
               TRACE6("VhallLoginWidget::LoginSuccessed Insert Faile!\n");
            }
         } else if (currentCount == 1) {
            query.exec(QString("SELECT version FROM t_user_record WHERE user_account = '%1'").arg(account));
            TRACE6("VhallLoginWidget::LoginSuccessed Select Last Version\n");

            while (query.next()) {
               QString lastVersion = query.value(0).toString();
               unsigned long long lastVerionI = GetLongVersion(lastVersion.toStdString());

               if (lastVerionI > 0 && iv > lastVerionI) {
                  TRACE6("VhallLoginWidget::LoginSuccessed Current(%s) < Last(%s)\n", cv.toUtf8().data(), lastVersion.toUtf8().data());
                  query.exec(QString("UPDATE t_user_record SET version = '%1' WHERE user_account = '%2'").arg(cv).arg(account));
                  mShowTeaching = true;
               } else {
                  TRACE6("VhallLoginWidget::LoginSuccessed Current(%s) < Last(%s)\n", cv.toUtf8().data(), lastVersion.toUtf8().data());
               }
               break;
            }
         }
      } else {
         TRACE6("VhallLoginWidget::LoginSuccessed Select failed!\n");
         if (query.exec("CREATE TABLE t_user_record (user_account varchar(255),version varchar(255))")) {
            TRACE6("VhallLoginWidget::LoginSuccessed Create Table Successed!\n");
            query.exec(QString("INSERT INTO t_user_record VALUES ('%1','%2')").arg(account).arg(cv));
            mShowTeaching = true;
         } else {
            TRACE6("VhallLoginWidget::LoginSuccessed Create Table Failed!\n");
         }
      }

      qDebug() << "VhallLoginWidget::LoginSuccessed()" << iv << cv;
   } else {
      TRACE6("VhallLoginWidget::LoginSuccessed Init Failed!\n");
   }

   ConfigSetting::writeValue(mConfigPath, GROUP_DEFAULT, GET_VERIFY_CODE_TIME, QDateTime::currentDateTime().toTime_t() - 60);
}

void VhallLoginWidget::GetVerifyCode() {
   ui->tip_account->clear();
   ui->tip_password->clear();
   ui->tip_phone->clear();
   ui->tip_verify_code->clear();
   ui->tip_phone->clear();

   QString phoneNumber = ui->lineEdit_phone->text();
   if (phoneNumber.isEmpty()) {
      ui->tip_account->clear();
      ui->tip_password->clear();
      ui->tip_phone->clear();
      ui->tip_verify_code->clear();
      ui->tip_phone->setText(QString::fromWCharArray(L"«Î ‰»Î ÷ª˙∫≈"));
      return;
   }


   if (!IsPhoneNumberEffective(phoneNumber)) {
      ui->tip_account->clear();
      ui->tip_password->clear();
      ui->tip_phone->clear();
      ui->tip_verify_code->clear();
      ui->tip_phone->setText(QString::fromWCharArray(L" ÷ª˙∫≈∏Ò Ω¥ÌŒÛ"));

      return;
   }

   ui->login_get_verify_code->setEnabled(false);
   repaint();

   QString domain = ConfigSetting::ReadString(mConfigPath, GROUP_DEFAULT, KEY_URL_DOMAIN, "");
   QString url = domain + "/api/vhallassistant/auth/verifycode?phone=" + ui->lineEdit_phone->text();

   QByteArray ba;
   HttpRequest req;
   if (req.VHGetRet(url, 10000, ba)) {
      qDebug() << "VhallLoginWidget::GetVerifyCode() " << ba;
      QJsonDocument doc = QJsonDocument::fromJson(ba);
      QJsonObject obj = doc.object();
      int code = obj["code"].toInt();
      QString msg = obj["msg"].toString();
      if (code != 200) {
         ui->tip_phone->setText(msg);
      } else {
         ConfigSetting::writeValue(mConfigPath, GROUP_DEFAULT, GET_VERIFY_CODE_TIME, QDateTime::currentDateTime().toTime_t());
      }
   } else {
      Tip("", QString::fromWCharArray(L"«Î«Û≥¨ ±"));
   }
}
void VhallLoginWidget::GetVeryfyCodeEnd() {

}
void VhallLoginWidget::CheckAutoLogin() {

}
void VhallLoginWidget::on_checkBox_toggled(bool checked) {
   if (!checked) {
      ConfigSetting::writeValue(mConfigPath, GROUP_DEFAULT, USER_AUTH_TOKEN, "");
   }
}
void VhallLoginWidget::on_lineEdit_account_returnPressed() {
   if (ui->lineEdit_password->text().isEmpty()) {
      ui->lineEdit_password->setFocus();
   } else {
      Login();
   }
}
void VhallLoginWidget::on_lineEdit_password_returnPressed() {
   Login();
}
void VhallLoginWidget::on_lineEdit_phone_returnPressed() {
   if (ui->lineEdit_password->text().isEmpty()) {
      ui->lineEdit_password->setFocus();
   } else {
      Login();
   }

}
void VhallLoginWidget::on_lineEdit_verify_code_returnPressed() {
   Login();
}

void VhallLoginWidget::on_lineEidt_account_textChanged(const QString & text) {
   if (text.isEmpty()) {
      ui->label_account_login->setStyleSheet("background:url(:/login/img/loginDialog/login_user.png)");
   } else {
      ui->label_account_login->setStyleSheet("background:url(:/login/img/loginDialog/login_user_high.png)");
   }
}

void VhallLoginWidget::on_lineEdit_phone_textChanged(const QString& text) {
   if (text.isEmpty()) {
      ui->label_phoneNum_login->setStyleSheet("background:url(:/login/img/loginDialog/login_user.png)");
   } else {
      ui->label_phoneNum_login->setStyleSheet("background:url(:/login/img/loginDialog/login_user_high.png)");
   }
}

void VhallLoginWidget::on_lineEdit_password_textChanged(const QString& text) {
   if (text.isEmpty()) {
      ui->label_login_password->setStyleSheet("background:url(:/login/img/loginDialog/login_password.png)");
   } else {
      ui->label_login_password->setStyleSheet("background:url(:/login/img/loginDialog/login_password_high.png)");
   }
}

void VhallLoginWidget::SetVersion(const QString& strVersion) {
   int iPos = strVersion.lastIndexOf(".");
   ui->labVesion->setText("V" + strVersion.mid(0, iPos));
}


void VhallLoginWidget::loginLogReport(bool bResult) {
   STRU_MAINUI_LOG log;
   swprintf_s(log.m_wzRequestUrl, DEF_MAX_HTTP_URL_LEN,
              L"key=%ws&k=%d",
              LOG_REPORT_KEY,
              eLogRePortK_LogIn);

   //QDateTime::currentDateTime();
   QJsonObject body;

   //body["t"] = m_startLogInTime.toString(STR_DATETIME);
   body["_r"] = bResult;//£®øÕªß∂ÀIP£©
   body["tt"] = m_startLogInTime.msecsTo(QDateTime::currentDateTime());
   QString json = CPathManager::GetStringFromJsonObject(body);
   strcpy_s(log.m_wzRequestJson, json.toLatin1().data());

   CSingletonRuntimeInstance::Instance().PostCRMessage(MSG_MAINUI_LOG, &log, sizeof(STRU_MAINUI_LOG));
}

void VhallLoginWidget::slot_OnClickedProxySet() {
   VHNetworkProxy proxy(this);
   proxy.exec();
}

void VhallLoginWidget::ShowJoinActiveWnd() {
   int accepted = 0;
   if (mInterActiveLoginWdg) {
      accepted = mInterActiveLoginWdg->exec();
      if (accepted == QDialog::Accepted) {
         mUserImage = mInterActiveLoginWdg->GetHeadImageUrl();
         mPluginsUrl = mInterActiveLoginWdg->GetPluginUrl();
         mAuthToken = mInterActiveLoginWdg->GetToken();
         mtoken = mAuthToken;
         mUserName = mInterActiveLoginWdg->GetUserNickName();
         mbIsPwdLogin = true;
         this->accept();
      }
   }
}

bool VhallLoginWidget::IsPwdLogin() {
   return mbIsPwdLogin;
}


