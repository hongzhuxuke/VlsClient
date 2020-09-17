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
#include <QApplication>
#include <QKeyEvent>
#include <QtNetwork\QNetworkProxy>
#include <string>
#include <vector>
#include <QPropertyAnimation>
#include <QDir>
#include "CRPluginDef.h"
#include "ICommonData.h"
#include "VH_Macro.h"
#include "Msg_MainUI.h"
#include "ConfigSetting.h"
#include "vhproxytest.h"
#include "AlertTipsDlg.h"
#include "title_button.h"
#include "vhnetworkproxy.h"
#include "vhnetworktips.h"
#include "vhsqllite.h"
#include "DebugTrace.h"
#include "pathManage.h"
#include "pathManager.h"
#include "pub.Struct.h"
#include "pub.Const.h"
#include "IPluginWebServer.h"
#include "MainUIIns.h"
#include "VhallNetWorkInterface.h"
#include "PicturePreviewWdg.h"
#include <QDesktopWidget>

VhallLoginWidget::VhallLoginWidget(QWidget *parent) :
   QWidget(parent),
   ui(new Ui::VhallLoginWidget) {
   ui->setupUi(this);

   this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint | Qt::Dialog);
   setAttribute(Qt::WA_TranslucentBackground);
   setAutoFillBackground(false);
   setWindowTitle(LOGIN);

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

   ui->lineEdit_account->show();
   ui->lineEdit_password->show();
   ui->lineEdit_phone->show();
   ui->lineEdit_verify_code->show();

   ui->tip_phone->clear();
   ui->tip_verify_code->clear();
   ui->tip_account->clear();
   ui->tip_password->clear();

   ////µ±µÇÂ¼¿ò¡¢ÃÜÂë¿ò¡¢ÊÖ»úºÅÊäÈë¿òÓÐÎÄ×ÖÊ±£¬Í¼±ê¸ßÁÁ£¬Ã»ÓÐÎÄ×ÖÊ±£¬Í¼±ê²»¸ßÁÁ
   connect(ui->lineEdit_account, SIGNAL(textChanged(const QString &)), this, SLOT(on_lineEidt_account_textChanged(const QString &)));
   connect(ui->lineEdit_password, SIGNAL(textChanged(const QString &)), this, SLOT(on_lineEdit_password_textChanged(const QString &)));
   connect(ui->lineEdit_phone, SIGNAL(textChanged(const QString &)), this, SLOT(on_lineEdit_phone_textChanged(const QString &)));

   QPixmap bg_pixmap = QPixmap(":/login/img/loginDialog/login_background.png");
   QPixmap bg_pixmap_shadow = QPixmap(":/login/img/loginDialog/login_background_shadow.png");
   pixmap = bg_pixmap_shadow;
   QPainter p(&pixmap);
   p.drawPixmap(bg_pixmap.rect(), bg_pixmap);

   ui->login_account->setStyleSheet(QString::fromWCharArray(L"QLabel{color:#FF3333;font: 14pt \"Î¢ÈíÑÅºÚ\";}"));
   ui->login_phone->setStyleSheet(QString::fromWCharArray(L"QLabel{color:#FEFEFE;font: 14pt \"Î¢ÈíÑÅºÚ\";}QLabel:hover{color:#DDDDDD;}"));
   ui->stackedWidget->setCurrentIndex(0);
   ui->BtnClose->loadPixmap(":/sysButton/close_button");
   ui->BtnClose->setToolTip(QStringLiteral("¹Ø±Õ´°¿Ú"));
   ui->BtnClose->setStyleSheet(TOOLTIPQSS);
   ui->BtnClose->setFocusPolicy(Qt::NoFocus);
   connect(ui->BtnClose, SIGNAL(clicked()), this, SLOT(Slot_ExitApp()));

   ui->pushButton_min->loadPixmap(":/sysButton/min_button");
   ui->pushButton_min->setToolTip(QStringLiteral("×îÐ¡»¯"));
   ui->pushButton_min->setStyleSheet(TOOLTIPQSS);
   ui->pushButton_min->setFocusPolicy(Qt::NoFocus);
   connect(ui->pushButton_min, SIGNAL(clicked()), this, SLOT(Slot_MinApp()));

   QString chkVerUrl;
   mConfigPath = CPathManager::GetConfigPath();
   QString token = ConfigSetting::ReadString(mConfigPath, GROUP_DEFAULT, USER_AUTH_TOKEN, "");
   SetAuthToken(token);

   connect(&mTimer, SIGNAL(timeout()), this, SLOT(timeout()));
   setFocusPolicy(Qt::NoFocus);
   QString toolConfPath = CPathManager::GetAppDataPath() + QString::fromStdWString(VHALL_TOOL_CONFIG);
   int hideLogo = ConfigSetting::ReadInt(toolConfPath, GROUP_DEFAULT, KEY_VHALL_LOGO_HIDE, 0);
   if (hideLogo == 1) {
      ui->widget_vhallLogo->hide();
   }
   else {
      QString logoPath = CPathManager::GetLogoImagePath();
      QString sheet = QString("border-image:url(\"%1\");").arg(logoPath);
      ui->widget_vhallLogo->setStyleSheet(sheet);
   }

   int hide_user_register = ConfigSetting::ReadInt(toolConfPath, GROUP_DEFAULT, KEY_VHALL_USER_REGISTER_HIDE, 0);
   if (hide_user_register == 1) {
      ui->login_register->hide();
      ui->label_7->hide();
   }
   int hide_forget_pwd = ConfigSetting::ReadInt(toolConfPath, GROUP_DEFAULT, KEY_VHALL_USER_FORGET_PWD, 0);
   if (hide_forget_pwd == 1) {
      ui->login_forgot_password->hide();
      ui->label_7->hide();
   }
}

VhallLoginWidget::~VhallLoginWidget() {
   mTimer.stop();
   delete ui;
}
bool VhallLoginWidget::eventFilter(QObject *o, QEvent *e) {
   if (o == ui->login_account) {
      int currentIndex = ui->stackedWidget->currentIndex();
      if (e->type() == QEvent::MouseButtonPress) {
         if (currentIndex == 0) {
            ui->login_account->setStyleSheet(QString::fromWCharArray(L"QLabel{color:#FF3333;font: 14pt \"Î¢ÈíÑÅºÚ\";}QLabel:hover{color:#FF3333;}"));
         }
         else {
            ui->login_account->setStyleSheet(QString::fromWCharArray(L"QLabel{color:#FEFEFE;font: 14pt \"Î¢ÈíÑÅºÚ\";}QLabel:hover{color:#DDDDDD;}"));
         }
         this->repaint();
      }
      else if (e->type() == QEvent::MouseButtonRelease) {
         if (ui->BtnClose->isEnabled()) {
            ui->login_account->setStyleSheet(QString::fromWCharArray(L"QLabel{color:#FF3333;font: 14pt \"Î¢ÈíÑÅºÚ\";}"));
            ui->login_phone->setStyleSheet(QString::fromWCharArray(L"QLabel{color:#FEFEFE;font: 14pt \"Î¢ÈíÑÅºÚ\";}QLabel:hover{color:#DDDDDD;}"));
            ui->stackedWidget->setCurrentIndex(0);
         }
         this->repaint();
      }
   }
   else if (o == ui->login_phone) {
      if (e->type() == QEvent::MouseButtonPress) {
         int currentIndex = ui->stackedWidget->currentIndex();
         if (currentIndex == 1) {
            ui->login_phone->setStyleSheet(QString::fromWCharArray(L"QLabel{color:#FF3333;font: 14pt \"Î¢ÈíÑÅºÚ\";}QLabel:hover{color:#FF3333;}"));
         }
         else {
            ui->login_phone->setStyleSheet(QString::fromWCharArray(L"QLabel{color:#FEFEFE;font: 14pt \"Î¢ÈíÑÅºÚ\";}QLabel:hover{color:#DDDDDD;}"));
         }
         this->repaint();
      }
      else if (e->type() == QEvent::MouseButtonRelease) {
         if (ui->BtnClose->isEnabled()) {
            ui->login_phone->setStyleSheet(QString::fromWCharArray(L"QLabel{color:#FF3333;font: 14pt \"Î¢ÈíÑÅºÚ\";}"));
            ui->login_account->setStyleSheet(QString::fromWCharArray(L"QLabel{color:#FEFEFE;font: 14pt \"Î¢ÈíÑÅºÚ\";}QLabel:hover{color:#DDDDDD;}"));
            ui->stackedWidget->setCurrentIndex(1);
         }

         this->repaint();
      }
   }
   else if (o == ui->login_forgot_password) {
      if (e->type() == QEvent::MouseButtonPress) {
         ui->login_forgot_password->setStyleSheet(QString::fromWCharArray(L"QLabel{color:#626567;}QLabel:hover{color:#626567;}"));
         this->repaint();
      }
      else if (e->type() == QEvent::MouseButtonRelease) {
         ui->login_forgot_password->setStyleSheet(QString::fromWCharArray(L"QLabel{color:#626567;}QLabel:hover{color:#FEFEFE;}"));
         this->repaint();

         QString domain = ConfigSetting::ReadString(CPathManager::GetToolConfigPath(), GROUP_DEFAULT, KEY_URL_DOMAIN, domainUrl);
         QDesktopServices::openUrl(QUrl(domain + "/auth/forgot"));
      }
   }
   else if (o == ui->login_register) {
      if (e->type() == QEvent::MouseButtonPress) {
         ui->login_register->setStyleSheet(QString::fromWCharArray(L"QLabel{color:#626567;}QLabel:hover{color:#626567;}"));
         this->repaint();
      }
      else if (e->type() == QEvent::MouseButtonRelease) {
         ui->login_register->setStyleSheet(QString::fromWCharArray(L"QLabel{color:#626567;}QLabel:hover{color:#FEFEFE;}"));
         this->repaint();

         QString domain = ConfigSetting::ReadString(CPathManager::GetToolConfigPath(), GROUP_DEFAULT, KEY_URL_DOMAIN, domainUrl);
         QDesktopServices::openUrl(QUrl(domain + "/auth/register"));
      }
   }
   else if (o == this) {
      if (e->type() == QEvent::MouseButtonPress) {
         this->pressPoint = this->cursor().pos();
         this->startPoint = this->pos();
      }
      else if (e->type() == QEvent::MouseMove) {
         int dx = this->cursor().pos().x() - this->pressPoint.x();
         int dy = this->cursor().pos().y() - this->pressPoint.y();
         this->move(this->startPoint.x() + dx, this->startPoint.y() + dy);
      }
      else if (e->type() == QEvent::MouseButtonRelease) {
         QWidget* parent_widget = this;
         QPoint pos = parent_widget->pos();
         QRect rect = QApplication::desktop()->availableGeometry(pos);
         QRect wnd_rect = parent_widget->frameGeometry();
         if (pos.y() > rect.height() - 150) {
            pos.setY(rect.height() - 150);
            parent_widget->move(pos);
         }
         else if (pos.y() < rect.y()) {
            parent_widget->move(pos + QPoint(0, +100));
         }
         else if (wnd_rect.x() >= rect.x() + rect.width() - 100) {
            parent_widget->move(QPoint(wnd_rect.x(), wnd_rect.y()) + QPoint(-100, 0));
         }
         else if (wnd_rect.x() + this->width() < rect.x() + 100) {
            parent_widget->move(QPoint(wnd_rect.x(), wnd_rect.y()) + QPoint(100, 0));
         }
      }
   }
   else if (o == ui->checkBox) {
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
      }
      else if (e->type() == QEvent::Leave) {
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
      }
      else if (e->type() == QEvent::MouseButtonPress) {
         this->repaint();
      }
      else if (e->type() == QEvent::MouseButtonRelease) {
         this->repaint();
      }
   }
   else if (o == ui->login_btn) {
      if (e->type() == QEvent::MouseButtonRelease) {
         if (ui->login_btn->isEnabled()) {
            Login();
         }
      }
   }
   else if (o == ui->login_get_verify_code) {
      if (e->type() == QEvent::MouseButtonRelease) {
         if (ui->BtnClose->isEnabled() && ui->login_get_verify_code->isEnabled()) {
            GetVerifyCode();
         }
      }
   }
   else if (o == ui->label_proxy) {
      if (e->type() == QEvent::MouseButtonRelease) {
         slot_OnClickedProxySet();
      }
   }
   else if (o == ui->label_interactive) {
      if (e->type() == QEvent::MouseButtonRelease) {
         ShowJoinActiveWnd();
      }
   }
   else if (o == ui->lineEdit_verify_code) {
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
      strLogInUId = account;
      loginByAccount = true;
      if (account.isEmpty()) {
         ui->tip_account->clear();
         ui->tip_password->clear();
         ui->tip_phone->clear();
         ui->tip_verify_code->clear();
         ui->tip_account->setText(IINPUT_ACCOUNT);
         mLoginFailedReason = "";
         LoginFailed();
         return;
      }
      if (password.isEmpty()) {
         ui->tip_account->clear();
         ui->tip_password->clear();
         ui->tip_phone->clear();
         ui->tip_verify_code->clear();
         ui->tip_password->setText(INPUT_PWD);
         mLoginFailedReason = "";
         LoginFailed();
         return;
      }
   }
   else {
      QString phoneNumber = ui->lineEdit_phone->text();
      QString verifyCode = ui->lineEdit_verify_code->text();
      strLogInUId = phoneNumber;
      if (phoneNumber.isEmpty()) {
         ui->tip_account->clear();
         ui->tip_password->clear();
         ui->tip_phone->clear();
         ui->tip_verify_code->clear();
         ui->tip_phone->setText(QString::fromWCharArray(L"ÇëÊäÈëÊÖ»úºÅ"));
         mLoginFailedReason = "";
         LoginFailed();
         return;
      }
      if (!IsPhoneNumberEffective(phoneNumber)) {
         ui->tip_account->clear();
         ui->tip_password->clear();
         ui->tip_phone->clear();
         ui->tip_verify_code->clear();
         ui->tip_phone->setText(PHONE_NUM_FORMAT_ERR);
         mLoginFailedReason = "";
         LoginFailed();
         return;
      }
      if (verifyCode.isEmpty()) {
         ui->tip_account->clear();
         ui->tip_password->clear();
         ui->tip_phone->clear();
         ui->tip_verify_code->clear();
         ui->tip_verify_code->setText(CHECK_CODE_ERR);
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
   QString domain = ConfigSetting::ReadString(CPathManager::GetToolConfigPath(), GROUP_DEFAULT, KEY_URL_DOMAIN, domainUrl);
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
   }
   else {
      url = domain + "/api/vhallassistant/auth/phonelogin?phone=%0&password=%1";
      url = url.arg(ui->lineEdit_phone->text()).arg(ui->lineEdit_verify_code->text());
   }

   //´úÀíÅäÖÃ
   QString proxyHost;
   QString proxyUsr;
   QString proxyPwd;
   int proxyPort;
   int is_http_proxy = ConfigSetting::ReadInt(mConfigPath, GROUP_DEFAULT, PROXY_OPEN, 0);
   if (is_http_proxy) {
      TRACE6("%s is_http_proxy:%d\n", __FUNCTION__, is_http_proxy);
      proxyHost = ConfigSetting::ReadString(mConfigPath, GROUP_DEFAULT, PROXY_HOST, "");
      proxyPort = ConfigSetting::ReadInt(mConfigPath, GROUP_DEFAULT, PROXY_PORT, 80);
      proxyUsr = ConfigSetting::ReadString(mConfigPath, GROUP_DEFAULT, PROXY_USERNAME, "");
      proxyPwd = ConfigSetting::ReadString(mConfigPath, GROUP_DEFAULT, PROXY_PASSWORD, "");
   }
   QObject *recvObj = this;
   HTTP_GET_REQUEST request(url.toStdString(), loginByAccount == true ? "1" : "0");
   GetHttpManagerInstance()->HttpGetRequest(request, [&, loginByAccount, recvObj](const std::string& msg, int code, const std::string userData) {
      if (recvObj) {
         if (loginByAccount) {
            QCoreApplication::postEvent(recvObj, new QHttpResponeEvent(CustomEvent_LoginByAccount, code, QString::fromStdString(msg)));
         }
         else {
            QCoreApplication::postEvent(recvObj, new QHttpResponeEvent(CustomEvent_LoginByPhone, code, QString::fromStdString(msg)));
         }
      }
   });
}

void VhallLoginWidget::LoginFailed() {
   //loginLogReport();
   Tip(LOGIN_ERR, mLoginFailedReason);
   ui->BtnClose->setEnabled(true);
   this->repaint();
}

void VhallLoginWidget::Tip(QString title, QString msg) {
   ui->tip_login_failed->setText(msg);
}
void VhallLoginWidget::timeout() {
   uint lastTime = ConfigSetting::ReadInt(mConfigPath, GROUP_DEFAULT, GET_VERIFY_CODE_TIME, 0);
   uint currentTime = QDateTime::currentDateTime().toTime_t();

   if (lastTime + 60 > currentTime) {
      ui->login_get_verify_code->setEnabled(false);
      uint diff = currentTime - lastTime;
      diff = 60 - diff;
      qDebug() << "timeout" << diff;
      ui->login_get_verify_code->setText(QString::number(diff) + LOGIN_LATER);
   }
   else {
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

void	SplitString(const string& str, const string& sKey, vector<string>& vecResult, BOOL bUrl) {
   string::size_type	npos = 0;
   string::size_type	nPrev = 0;

   while (true) {
      nPrev = npos;
      npos = str.find(sKey, npos);
      if (nPrev != 0 && bUrl == TRUE) {
         //if it is url, not check the url's key
         npos = string::npos;
      }
      if (npos != string::npos) {
         if (npos != nPrev) {
            vecResult.push_back(str.substr(nPrev, npos - nPrev));
         }
         npos += sKey.size();
         if (npos == nPrev)
            break;
      }
      else {
         if (nPrev < str.size())
            vecResult.push_back(str.substr(nPrev));
         break;
      }
   }
}
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
   //loginLogReport(true);
   mTimer.stop();
   ui->login_get_verify_code->setText(QString::fromWCharArray(L""));
   ui->login_get_verify_code->setEnabled(true);
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
            }
            else {
               TRACE6("VhallLoginWidget::LoginSuccessed Insert Faile!\n");
            }
         }
         else if (currentCount == 1) {
            query.exec(QString("SELECT version FROM t_user_record WHERE user_account = '%1'").arg(account));
            TRACE6("VhallLoginWidget::LoginSuccessed Select Last Version\n");
            while (query.next()) {
               QString lastVersion = query.value(0).toString();
               unsigned long long lastVerionI = GetLongVersion(lastVersion.toStdString());
               if (lastVerionI > 0 && iv > lastVerionI) {
                  TRACE6("VhallLoginWidget::LoginSuccessed Current(%s) < Last(%s)\n", cv.toUtf8().data(), lastVersion.toUtf8().data());
                  query.exec(QString("UPDATE t_user_record SET version = '%1' WHERE user_account = '%2'").arg(cv).arg(account));
                  mShowTeaching = true;
               }
               else {
                  TRACE6("VhallLoginWidget::LoginSuccessed Current(%s) < Last(%s)\n", cv.toUtf8().data(), lastVersion.toUtf8().data());
               }
               break;
            }
         }
      }
      else {
         TRACE6("VhallLoginWidget::LoginSuccessed Select failed!\n");
         if (query.exec("CREATE TABLE t_user_record (user_account varchar(255),version varchar(255))")) {
            TRACE6("VhallLoginWidget::LoginSuccessed Create Table Successed!\n");
            query.exec(QString("INSERT INTO t_user_record VALUES ('%1','%2')").arg(account).arg(cv));
            mShowTeaching = true;
         }
         else {
            TRACE6("VhallLoginWidget::LoginSuccessed Create Table Failed!\n");
         }
      }
      qDebug() << "VhallLoginWidget::LoginSuccessed()" << iv << cv;
   }
   else {
      TRACE6("VhallLoginWidget::LoginSuccessed Init Failed!\n");
   }
   ConfigSetting::writeValue(mConfigPath, GROUP_DEFAULT, GET_VERIFY_CODE_TIME, QDateTime::currentDateTime().toTime_t() - 60);
   SetAuthToken(token);
   QString toolConfig = CPathManager::GetToolConfigPath();
   int teaching = ConfigSetting::ReadInt(toolConfig, GROUP_DEFAULT, TEACHING_FLAG, 0);
   if (teaching == 0) {
      mShowTeaching = true;
      ConfigSetting::writeValue(toolConfig, GROUP_DEFAULT, TEACHING_FLAG, 1);
   }
   emit sig_Login();
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
      ui->tip_phone->setText(INPUT_PHONE);
      return;
   }

   if (!IsPhoneNumberEffective(phoneNumber)) {
      ui->tip_account->clear();
      ui->tip_password->clear();
      ui->tip_phone->clear();
      ui->tip_verify_code->clear();
      ui->tip_phone->setText(PHONE_NUM_FORMAT_ERR);
      return;
   }

   ui->login_get_verify_code->setText(RE_GET_CODE);
   ui->login_get_verify_code->setEnabled(false);
   repaint();

   QString domain = ConfigSetting::ReadString(CPathManager::GetToolConfigPath(), GROUP_DEFAULT, KEY_URL_DOMAIN, domainUrl);
   QString url = domain + "/api/vhallassistant/auth/verifycode?phone=" + ui->lineEdit_phone->text();

   //´úÀíÅäÖÃ
   QString proxyHost;
   QString proxyUsr;
   QString proxyPwd;
   int proxyPort;
   int is_http_proxy = ConfigSetting::ReadInt(mConfigPath, GROUP_DEFAULT, PROXY_OPEN, 0);
   if (is_http_proxy) {
      TRACE6("%s is_http_proxy:%d\n", __FUNCTION__, is_http_proxy);
      proxyHost = ConfigSetting::ReadString(mConfigPath, GROUP_DEFAULT, PROXY_HOST, "");
      proxyPort = ConfigSetting::ReadInt(mConfigPath, GROUP_DEFAULT, PROXY_PORT, 80);
      proxyUsr = ConfigSetting::ReadString(mConfigPath, GROUP_DEFAULT, PROXY_USERNAME, "");
      proxyPwd = ConfigSetting::ReadString(mConfigPath, GROUP_DEFAULT, PROXY_PASSWORD, "");
   }
   HTTP_GET_REQUEST request(url.toStdString());
   GetHttpManagerInstance()->HttpGetRequest(request, [&](const std::string& msg, int code, const std::string userData) {
      QCoreApplication::postEvent(this, new QHttpResponeEvent(CustomEvent_VerifyCode, code, QString::fromStdString(msg)));
   });
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
   }
   else {
      Login();
   }
}
void VhallLoginWidget::on_lineEdit_password_returnPressed() {
   Login();
}
void VhallLoginWidget::on_lineEdit_phone_returnPressed() {
   if (ui->lineEdit_password->text().isEmpty()) {
      ui->lineEdit_password->setFocus();
   }
   else {
      Login();
   }

}
void VhallLoginWidget::on_lineEdit_verify_code_returnPressed() {
   Login();
}

void VhallLoginWidget::on_lineEidt_account_textChanged(const QString & text) {
   if (text.isEmpty()) {
      ui->label_account_login->setStyleSheet("background:url(:/login/img/loginDialog/login_user.png)");
   }
   else {
      ui->label_account_login->setStyleSheet("background:url(:/login/img/loginDialog/login_user_high.png)");
   }
}

void VhallLoginWidget::on_lineEdit_phone_textChanged(const QString& text) {
   if (text.isEmpty()) {
      ui->label_phoneNum_login->setStyleSheet("background:url(:/login/img/loginDialog/login_user.png)");
   }
   else {
      ui->label_phoneNum_login->setStyleSheet("background:url(:/login/img/loginDialog/login_user_high.png)");
   }
}

void VhallLoginWidget::on_lineEdit_password_textChanged(const QString& text) {
   if (text.isEmpty()) {
      ui->label_login_password->setStyleSheet("background:url(:/login/img/loginDialog/login_password.png)");
   }
   else {
      ui->label_login_password->setStyleSheet("background:url(:/login/img/loginDialog/login_password_high.png)");
   }
}

void VhallLoginWidget::SetVersion(const QString& strVersion) {
   int iPos = strVersion.lastIndexOf(".");
   ui->labVesion->setText("V" + strVersion.mid(0, iPos));
}

void VhallLoginWidget::loginLogReport(bool bResult) {
   QJsonObject body;
   body["_r"] = bResult;//£¨¿Í»§¶ËIP£©
   body["tt"] = m_startLogInTime.msecsTo(QDateTime::currentDateTime());

   wstring version;
   VH::CComPtr<ICommonData> pCommonData;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
   if (pCommonData) {
      pCommonData->GetCurVersion(version);
   }

   OSVERSIONINFO osver = { sizeof(OSVERSIONINFO) };
   GetVersionEx(&osver);
   QString logInfo = QString("log_in&start_mode=0&version=%1").arg(QString::fromStdWString(version).toStdString().c_str());
   SingletonMainUIIns::Instance().reportLog(logInfo.toStdWString().c_str(), eLogRePortK_LogIn, L"LogIn", body);
}

void VhallLoginWidget::slot_OnClickedProxySet() {
   VHNetworkProxy proxy(this);
   proxy.exec();
}

void VhallLoginWidget::ShowJoinActiveWnd() {
   emit sig_PwdLogin();
}

bool VhallLoginWidget::IsPwdLogin() {
   return mbIsPwdLogin;
}

void VhallLoginWidget::Slot_ExitApp() {
   ui->horizontalLayout_12->removeItem(ui->verticalLayout_3);
   ui->horizontalLayout_12->removeItem(ui->gridLayout);
   setMinimumSize(0, 0);
   QPropertyAnimation* closeAnimation = new QPropertyAnimation(this, "geometry");
   closeAnimation->setStartValue(geometry());
   closeAnimation->setEndValue(QRect(geometry().x(), geometry().y() /*+ height() / 2*/, width(), 0));
   closeAnimation->setDuration(150);
   connect(closeAnimation, SIGNAL(finished()), this, SLOT(slot_CloseAndExitApp()));
   closeAnimation->start(QAbstractAnimation::DeleteWhenStopped);
}

void VhallLoginWidget::Slot_MinApp() {
   showMinimized();
}

void VhallLoginWidget::closeEvent(QCloseEvent *event) {
   if (event) {
      event->ignore();
   }
}

void VhallLoginWidget::customEvent(QEvent* event) {
   ConfigSetting::ContainsInDesktopIndex(this);
   if (event) {
      switch (event->type())
      {
      case CustomEvent_LoginByAccount:
      case CustomEvent_LoginByPhone: {
         HandleLogin(event);
         break;
      }
      case CustomEvent_VerifyCode: {
         HandleGetVerifyCode(event);
         break;
      }
      default:
         break;
      }
   }
}

void VhallLoginWidget::EnableClickedExitApp() {
   ui->BtnClose->setEnabled(true);
}

void VhallLoginWidget::slot_CloseAndExitApp() {
   TRACE6("%s \n", __FUNCTION__);
   emit sig_ExitApp();
}

void VhallLoginWidget::HandleLogin(QEvent* event) {
   QHttpResponeEvent* loginEvent = dynamic_cast<QHttpResponeEvent*>(event);
   if (loginEvent) {
      bool loginByAccount = event->type() == CustomEvent_LoginByAccount ? true : false;
      int code = loginEvent->mCode;
      QString msg = loginEvent->mData;
      if (code == 0 && msg.length() > 0) {
         QByteArray ba(msg.toStdString().c_str(), msg.length());
         QJsonDocument doc = QJsonDocument::fromJson(ba);
         QJsonObject obj = doc.object();
         int code = obj["code"].toInt();
         QString msg = obj["msg"].toString();
         if (code == 200) {
            QJsonObject data = obj["data"].toObject();
            LoginRespInfo loginInfo;
            loginInfo.plugins_url = data["plugins_url"].toString();
            loginInfo.interact_plugins_url = data["interact_plugins_url"].toString();
            loginInfo.list_url = data["list_url"].toString();
            loginInfo.token = data["token"].toString();
            loginInfo.nick_name = data["nick_name"].toString();
            loginInfo.avatar = "http:" + data["avatar"].toString();
            loginInfo.chat_url = data["chat_url"].toString();
            loginInfo.userid = data["name"].toString();
            loginInfo.phone = data["phone"].toString();
            loginInfo.phone = data["phone"].toString();
            loginInfo.sso_url = data["sso_url"].toString();
            VH::CComPtr<ICommonData> pCommonData;
            DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
            if (pCommonData) {
               pCommonData->SetLoginRespInfo(loginInfo);
            }
            if (!loginInfo.userid.isEmpty()) {
               ConfigSetting::writeValue(mConfigPath, GROUP_DEFAULT, LOG_ID_BASE, loginInfo.userid);
            }
            if (loginByAccount) {
               QJsonObject obj;
               obj["account"] = ui->lineEdit_account->text();
               if (ui->checkBox->isChecked()) {
                  obj["password"] = ui->lineEdit_password->text();
               }
               QJsonDocument doc;
               doc.setObject(obj);
               mAuthToken = doc.toJson(QJsonDocument::Compact).toBase64();
            }
            else {
               mAuthToken = "";
            }
            LoginSuccessed(loginByAccount ? ui->lineEdit_account->text() : ui->lineEdit_phone->text());
            return;
         }
         else {
            //ÕËºÅµÇÂ¼
            if (loginByAccount) {
               if (code / 100 == 4) {
                  ui->tip_account->setText(msg);
               }
               else if (code / 100 == 5) {
                  ui->tip_password->setText(msg);
               }
               else {
                  ui->tip_login_failed->setText(msg);
                  mLoginFailedReason = msg;
               }
            }
            //ÊÖ»úºÅ£¬ÑéÖ¤Âë
            else {
               if (code / 100 == 4) {
                  ui->tip_phone->setText(msg);
               }
               else if (code / 100 == 5) {
                  ui->tip_verify_code->setText(msg);
               }
               else {
                  ui->tip_login_failed->setText(msg);
                  mLoginFailedReason = msg;
               }
            }
         }
      }
      else {
         VHNetworkTips tips(this);
         tips.exec();
         if (tips.ProxyConfigure()) {
            VHNetworkProxy proxy(this);
            proxy.exec();
         }
         mLoginFailedReason = LOGIN_ERR;
      }
      LoginFailed();
   }
}

void VhallLoginWidget::HandleGetVerifyCode(QEvent* event) {
   QHttpResponeEvent* loginEvent = dynamic_cast<QHttpResponeEvent*>(event);
   if (loginEvent) {
      int code = loginEvent->mCode;
      QString msg = loginEvent->mData;
      if (code == 0 && msg.length() > 0) {
         QByteArray ba(msg.toStdString().c_str(), msg.length());
         QJsonDocument doc = QJsonDocument::fromJson(ba);
         QJsonObject obj = doc.object();
         int code = obj["code"].toInt();
         QString msg = obj["msg"].toString();
         if (code != 200) {
            ui->tip_phone->setText(msg);
         }
         else {
            mTimer.start(1000);
            ConfigSetting::writeValue(mConfigPath, GROUP_DEFAULT, GET_VERIFY_CODE_TIME, QDateTime::currentDateTime().toTime_t());
         }
      }
      else {
         Tip("", NETWORK_REQUEST_ERR);
      }
   }
}

