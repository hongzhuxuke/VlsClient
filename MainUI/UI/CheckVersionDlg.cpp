#include "stdafx.h"
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QJsonParseError>
#include <QMessageBox>
#include <QPainter>
#include <QTimer>
#include <QDir>
#include <atlstr.h>
#include "CheckVersionDlg.h"
#include "TitleWidget.h"
#include "ConfigSetting.h"
#include "AlertTipsDlg.h"
#include "vhupdatetips.h"
#include "pathManage.h"
#include "pathmanager.h"
#include "VhallNetWorkInterface.h"


#if _MSC_VER >= 1600  
#pragma execution_character_set("utf-8")  
#endif  
#define VersionHttpAPI QString("http://e.vhall.com/api/client/v1/util/current-version")
#define MengZhuVersionHttpAPI QString("http://e.vhall.com/api/client/v1/util/current-version")
IA64	GetCurVersion(const wchar_t* exeFile, std::wstring &);
void	SplitString(const string& str, const string& sKey, vector<string>& vecResult, BOOL bUrl);
static QString gCheckVerMsg;
static int gCheckCode = 0;

const QEvent::Type CustomEvent_CheckVer = (QEvent::Type)40100;

CCheckVersionDlg::CCheckVersionDlg(QDialog *parent, bool bUpdata)
   : CBaseDlg(parent)
   , m_bMengZhu(bUpdata)
   , mChkVersionAccessManager(NULL)
   , mTimeoutTimer(NULL)
   , mStartCheckTimer(NULL)
   , mDownloadAccessManager(NULL)
   , mChkVersionReply(NULL)
   , mDownloadReply(NULL)
   , mFile(NULL) {
   TRACE6("%s setWindowFlags\n", __FUNCTION__);
   this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint | Qt::Tool | Qt::WindowStaysOnTopHint);
   setAttribute(Qt::WA_TranslucentBackground);
   TRACE6("%s setAttribute\n", __FUNCTION__);
   setAutoFillBackground(false);
   TRACE6("%s 1\n", __FUNCTION__);

   QVBoxLayout *main_layout = new QVBoxLayout(this);
   TRACE6("%s \n", __FUNCTION__);
   QString toolConfPath = CPathManager::GetAppDataPath() + QString::fromStdWString(VHALL_TOOL_CONFIG);
   int hideLogo = ConfigSetting::ReadInt(toolConfPath, GROUP_DEFAULT, KEY_VHALL_LOGO_HIDE, 0);
   if (hideLogo == 1) {
      QString imagePath = QString::fromStdWString(GetAppPath()) + "splash.png";
      this->pixmap = QPixmap(imagePath);
   }
   else {
      QString imagePath = CPathManager::GetSplashImagePath();
      this->pixmap = QPixmap(imagePath);
   }

   if (pixmap.size().width() > 600 && pixmap.size().height() > 366) {
      this->setFixedSize(QSize(600, 366));
   }
   else {
      this->setFixedSize(pixmap.size());
   }

   TRACE6("%s 2\n", __FUNCTION__);

   mProcessBar = new VHProcessBar(this);
   mProcessBar->setForceUpdate(mIsForceUpdata);
   connect(mProcessBar, &VHProcessBar::SigSuspensionLoad, this, &CCheckVersionDlg::slotSuspensionLoad);
   mProcessBar->SetValue(0);

   WCHAR				szApp[1024 * 4] = L"";
   GetModuleFileNameW(NULL, szApp, 1024 * 4);
   TRACE6("%s szApp:%ws 3\n", __FUNCTION__, szApp);
   if (bUpdata) {
      GetCurVersion(szApp, mCurrVersionW);
   }
   TRACE6("%s 1111111\n", __FUNCTION__);
   this->installEventFilter(this);
   this->setToolTip("");
   TRACE6("%s 4\n", __FUNCTION__);
}

int CCheckVersionDlg::exec() {
   TRACE6("%s\n", __FUNCTION__);
   //if (NULL == mStartCheckTimer){
   //   mStartCheckTimer = new QTimer(this);
   //   if (NULL != mStartCheckTimer){
   //      connect(mStartCheckTimer, SIGNAL(timeout()), this, SLOT(slot_startCheck()));
   //      mStartCheckTimer->setSingleShot(true);
   //   }
   //}

   //if (NULL != mStartCheckTimer){
   //   mStartCheckTimer->start(300);
   //}
   QString chkVerUrl;
   QString qsConfPath = CPathManager::GetToolConfigPath();
   if (!m_bMengZhu) {
      chkVerUrl = ConfigSetting::ReadString(qsConfPath, GROUP_DEFAULT, KEY_CHK_VER_URL, VersionHttpAPI);
   }
   else {
      chkVerUrl = ConfigSetting::ReadString(qsConfPath, GROUP_DEFAULT, KEY_CHK_VER_URL, MengZhuVersionHttpAPI);
   }
   TRACE6("%s url:%s\n", __FUNCTION__, chkVerUrl.toStdString().c_str());
   QString configPath = CPathManager::GetConfigPath();
   QString proxyHost;
   QString proxyUsr;
   QString proxyPwd;
   int proxyPort;
   int is_http_proxy = ConfigSetting::ReadInt(configPath, GROUP_DEFAULT, PROXY_OPEN, 0);
   if (is_http_proxy) {
      TRACE6("%s is_http_proxy:%d\n", __FUNCTION__, is_http_proxy);
      proxyHost = ConfigSetting::ReadString(configPath, GROUP_DEFAULT, PROXY_HOST, "");
      proxyPort = ConfigSetting::ReadInt(configPath, GROUP_DEFAULT, PROXY_PORT, 80);
      proxyUsr = ConfigSetting::ReadString(configPath, GROUP_DEFAULT, PROXY_USERNAME, "");
      proxyPwd = ConfigSetting::ReadString(configPath, GROUP_DEFAULT, PROXY_PASSWORD, "");
   }
   HTTP_GET_REQUEST httpRequest(chkVerUrl.toStdString());
   httpRequest.SetHttpPost(true);
   QObject* obj = this;
   GetHttpManagerInstance()->HttpGetRequest(httpRequest, [&, obj](const std::string& msg, int code, const std::string userData) {
      TRACE6("%s code:%d msg:%s\n", __FUNCTION__, code, msg.c_str());
      gCheckVerMsg = QString::fromStdString(msg);
      gCheckCode = code;
      QApplication::postEvent(obj,new QEvent(CustomEvent_CheckVer));
   });
   return QDialog::exec();
}

void CCheckVersionDlg::customEvent(QEvent* event){
   if (event && event->type() == CustomEvent_CheckVer) {
      int ret = CHECKVER_JSON_SUCCESS;
      QByteArray bytes = gCheckVerMsg.toUtf8();
      if ((ret = parseJson(&bytes)) != CHECKVER_JSON_SUCCESS) {
         Sleep(3000);
         this->close();
      }
      else {
         TRACE6("CCheckVersionDlg::VersionReplayFinished will CheckVersion");
         CheckVersion();
      }
   }
}

void CCheckVersionDlg::slot_startCheck()
{
   mStartCheckTimer->stop();
   TRACE6("%s new QVBoxLayout \n", __FUNCTION__);

   if (NULL == mTimeoutTimer){
      mTimeoutTimer = new QTimer(this);
      if (NULL != mTimeoutTimer){
         connect(mTimeoutTimer, SIGNAL(timeout()), this, SLOT(VersionCheckTimeout()));
      }
   }

   if (NULL != mTimeoutTimer){
      mTimeoutTimer->start(10000);
   }

   if (NULL == mChkVersionAccessManager){
      mChkVersionAccessManager = new QNetworkAccessManager(this);
      if (NULL != mChkVersionAccessManager){
         connect(mChkVersionAccessManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(VersionReplayFinished(QNetworkReply*)));
         connect(mChkVersionAccessManager, SIGNAL(authenticationRequired(QNetworkReply *, QAuthenticator *)), this, SLOT(slot_authenticationRequired(QNetworkReply *, QAuthenticator *)));
         connect(mChkVersionAccessManager, SIGNAL(proxyAuthenticationRequired(const QNetworkProxy &, QAuthenticator *)), this, SLOT(slot_proxyAuthenticationRequired(const QNetworkProxy &, QAuthenticator *)));
         TRACE6("%s new mChkVersionAccessManager \n", __FUNCTION__);
      }
   }

   GetNewVersion();
   TRACE6("%s\n", __FUNCTION__);
   QUrl url = mRequest.url();

   if (NULL != mChkVersionAccessManager){
      SetNetworkProxy(mChkVersionAccessManager);
      mChkVersionReply = mChkVersionAccessManager->get(mRequest);
   }
   TRACE6("%s mChkVersionAccessManager->get(mRequest) url:%s\n", __FUNCTION__, url.toString().toStdString().c_str());
}

bool CCheckVersionDlg::eventFilter(QObject *o, QEvent *e) {
   if (o == this) {
      if (e->type() == QEvent::MouseButtonPress) {
         this->pressPoint = this->cursor().pos();
         this->startPoint = this->pos();
      }
      else if (e->type() == QEvent::MouseMove) {
         int dx = this->cursor().pos().x() - this->pressPoint.x();
         int dy = this->cursor().pos().y() - this->pressPoint.y();
         this->move(this->startPoint.x() + dx, this->startPoint.y() + dy);
      }
   }
   return QWidget::eventFilter(o, e);
}

CCheckVersionDlg::~CCheckVersionDlg() {
   Release();
}

void CCheckVersionDlg::Release() {
   if (NULL != mStartCheckTimer){
      if (mStartCheckTimer->isActive()){
         mStartCheckTimer->stop();
      }
      delete mStartCheckTimer;
      mStartCheckTimer = NULL;
   }

   if (NULL != mTimeoutTimer){
      if (mTimeoutTimer->isActive()){
         mTimeoutTimer->stop();
      }
      delete mTimeoutTimer;
      mTimeoutTimer = NULL;
   }

   if (NULL != mChkVersionAccessManager){
      disconnect(mChkVersionAccessManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(VersionReplayFinished(QNetworkReply*)));
      disconnect(mChkVersionAccessManager, SIGNAL(authenticationRequired(QNetworkReply *, QAuthenticator *)), this, SLOT(slot_authenticationRequired(QNetworkReply *, QAuthenticator *)));
      disconnect(mChkVersionAccessManager, SIGNAL(proxyAuthenticationRequired(const QNetworkProxy &, QAuthenticator *)), this, SLOT(slot_proxyAuthenticationRequired(const QNetworkProxy &, QAuthenticator *)));
      mChkVersionAccessManager->deleteLater();
      mChkVersionAccessManager = NULL;
   }

   if (NULL != mDownloadAccessManager){
      disconnect(mDownloadAccessManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(DownloadReplyFinished(QNetworkReply*)));
      disconnect(mDownloadAccessManager, SIGNAL(authenticationRequired(QNetworkReply *, QAuthenticator *)), this, SLOT(slot_authenticationRequired(QNetworkReply *, QAuthenticator *)));
      disconnect(mDownloadAccessManager, SIGNAL(proxyAuthenticationRequired(const QNetworkProxy &, QAuthenticator *)), this, SLOT(slot_proxyAuthenticationRequired(const QNetworkProxy &, QAuthenticator *)));
      mDownloadAccessManager->deleteLater();
      mDownloadAccessManager = NULL;
   }

   if (NULL != mProcessBar){
      mProcessBar->deleteLater();
      mProcessBar = NULL;
   }

   if (NULL != mFile){
      mFile->deleteLater();
      mFile = NULL;
   }
}

void CCheckVersionDlg::StartDownload() {
   if (!mDownloadLink.isEmpty() && !mDownloadLink.isNull()) {

#ifndef _DEBUG
      QUrl downloadURL("http:" + mDownloadLink);
#else
      QUrl downloadURL("http:" + mDownloadLink);
#endif

      fileInfo = downloadURL.path();

      TRACE6("CCheckVersionDlg::StartDownload will download file to %s\n", fileInfo.fileName().toStdString().c_str());
      mDownloadFilePath = mDownloadFilePath.fromStdWString(GetAppDataPath() + L"updates\\");
      TRACE6("%s mDownloadFilePath  to %s\n", __FUNCTION__, mDownloadFilePath.toStdString().c_str());
      QDir updateDir(mDownloadFilePath);
      if (updateDir.exists() == false) {
         bool mksuc = updateDir.mkpath(mDownloadFilePath);
         TRACE6("%s mksuc %d\n", __FUNCTION__, mksuc);
      }
      mDownloadFilePath += "VhallUpdate.exe";
      TRACE6("%s mDownloadFilePath  to %s\n", __FUNCTION__, mDownloadFilePath.toStdString().c_str());
      mFile = new QFile(mDownloadFilePath);
      mFile->open(QIODevice::WriteOnly | QIODevice::Truncate);

      mRequest.setUrl(downloadURL);
      TRACE6("CCheckVersionDlg::StartDownload downloadURL %s\n", downloadURL.toString().toStdString().c_str());
      if (NULL == mDownloadAccessManager){
         mDownloadAccessManager = new QNetworkAccessManager(this);
         TRACE6("%s new mDownloadAccessManager \n", __FUNCTION__);
         if (NULL != mDownloadAccessManager){
            connect(mDownloadAccessManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(DownloadReplyFinished(QNetworkReply*)));
            connect(mDownloadAccessManager, SIGNAL(authenticationRequired(QNetworkReply *, QAuthenticator *)), this, SLOT(slot_authenticationRequired(QNetworkReply *, QAuthenticator *)));
            connect(mDownloadAccessManager, SIGNAL(proxyAuthenticationRequired(const QNetworkProxy &, QAuthenticator *)), this, SLOT(slot_proxyAuthenticationRequired(const QNetworkProxy &, QAuthenticator *)));
         }
      }

      SetNetworkProxy(mDownloadAccessManager);
      mDownloadReply = mDownloadAccessManager->get(mRequest);
      if (mDownloadReply) {
         connect(mDownloadReply, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(onDownloadProgress(qint64, qint64)));
         connect((QObject *)mDownloadReply, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
      }
      this->mProcessBar->show();
   }
}

void CCheckVersionDlg::DownloadReplyFinished(QNetworkReply *reply2) {
   this->mProcessBar->hide();
   QVariant status_code = reply2->attribute(QNetworkRequest::HttpStatusCodeAttribute);
   if (reply2->error() == QNetworkReply::NoError) {
      TRACE6("CheckVersion() download client success [%s]", mDownloadLink.toStdString().c_str());
      mFile->close();
      accept();
   }
   else {
      qDebug() << "CCheckVersionDlg::DownloadReplyFinished:" << reply2->errorString();
      TRACE6("CheckVersion() download client failed [%s]", mDownloadLink.toStdString().c_str());
      close();
   }
   reply2->deleteLater();
}

void CCheckVersionDlg::onDownloadProgress(qint64 bytesSent, qint64 bytesTotal) {
   if (bytesTotal > 0) {
      qint64 persent = bytesSent * 100;
      persent /= bytesTotal;
      if (NULL != mProcessBar && !mProcessBar->GetSuspension()) {
         mProcessBar->SetValue(persent);
      }   
   }
}

void CCheckVersionDlg::onReadyRead() {

   if (NULL != mFile && NULL != mDownloadReply)
      mFile->write(mDownloadReply->readAll());
}

void CCheckVersionDlg::GetNewVersion() {
   TRACE6("%s \n", __FUNCTION__);
   QString chkVerUrl;
   QString qsConfPath = CPathManager::GetToolConfigPath();
   if (!m_bMengZhu) {
      chkVerUrl = ConfigSetting::ReadString(qsConfPath, GROUP_DEFAULT, KEY_CHK_VER_URL, VersionHttpAPI);
   }
   else {
      chkVerUrl = ConfigSetting::ReadString(qsConfPath, GROUP_DEFAULT, KEY_CHK_VER_URL, MengZhuVersionHttpAPI);
   }

   QUrl versionUrl(chkVerUrl);
   mRequest.setUrl(versionUrl);
   TRACE6("%s url %s\n", __FUNCTION__, chkVerUrl.toStdString().c_str());
}

void CCheckVersionDlg::slotSuspensionLoad(){
   if (mDownloadReply) {
      disconnect(mDownloadReply, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(onDownloadProgress(qint64, qint64)));
      disconnect((QObject *)mDownloadReply, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
   }

   if (NULL != mFile){
      mFile->close();
      mFile->deleteLater();
   }

   mbSuspensionLoad = true;
   reject();
}

void CCheckVersionDlg::VersionCheckTimeout() {
   if (mCheckMutex.tryLock()) {
      mTimeoutTimer->stop();
      bool isShowAlert = !mIsChecked;
      mIsChecked = true;
      mCheckMutex.unlock();
      this->close();
   }
}

int CCheckVersionDlg::VersionReplayFinished(QNetworkReply *reply) {
   int ret = CHECKVER_JSON_SUCCESS;
   if (mCheckMutex.tryLock()) {
      bool isShowAlert = !mIsChecked;
      mIsChecked = true;
      mCheckMutex.unlock();
      if (true) {
         mTimeoutTimer->stop();
         QVariant status_code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
         if (reply->error() == QNetworkReply::NoError) {
            QByteArray bytes = reply->readAll();
            if ((ret = parseJson(&bytes)) != CHECKVER_JSON_SUCCESS) {
               Sleep(3000);
               this->close();
               return ret;
            }
            else {
               TRACE6("CCheckVersionDlg::VersionReplayFinished will CheckVersion");
               CheckVersion();
            }
         }
         else {
            TRACE6("%s reply->error() %d\n", __FUNCTION__, reply->error());
            Sleep(3000);
            this->close();
            return CHECKVER_DATA_ERROR;
         }
      }
   }

   reply->deleteLater();
   return ret;

}
int CCheckVersionDlg::parseJson(QByteArray *bytes) {
   QJsonParseError error;
   QJsonDocument jsonDocument = QJsonDocument::fromJson(*bytes, &error);
   if (error.error == QJsonParseError::NoError) {
      if (jsonDocument.isObject()) {
         QVariantMap result = jsonDocument.toVariant().toMap();
         int code = result["code"].toInt();
         QString msg = result["msg"].toString();
         if (code == 200) {
            QVariantMap dataMap = result["data"].toMap();
            mNewVersion = (dataMap["version"].toString()).toStdString();
            mDownloadLink = dataMap["download_link"].toString();
            mDownloadTips = dataMap["description"].toString();
            TRACE6("%s new:%s\n", __FUNCTION__, mNewVersion.c_str());
            QString forceUpdata = dataMap["force_upgrade"].toString();
            TRACE6("%s forceUpdata :%s\n", __FUNCTION__, forceUpdata.toStdString().c_str());
            if (forceUpdata.compare("1") == 0) {
               mIsForceUpdata = true;
            }
            return CHECKVER_JSON_SUCCESS;
         }
         else {
            return CHECKVER_JSON_ERROR;
         }
      }
      else {
         return CHECKVER_JSON_ERROR;
      }
   }
   else {
      return CHECKVER_JSON_ERROR;
   }
}

int CCheckVersionDlg::CheckVersion() {
   string newVerStr = this->mNewVersion;
   WCHAR				szApp[1024 * 4] = L"";
   IA64				i64NewVersion = 0;
   IA64				i64CurVersion = 0;
   GetModuleFileNameW(NULL, szApp, 1024 * 4);
   i64CurVersion = GetCurVersion(szApp, mCurrVersionW);

   if (newVerStr.empty() || newVerStr.length() == 0) {
      TRACE6("newVerStr.empty() || newVerStr.length() == 0\n");
      Sleep(3000);
      this->close();
      return -1;
   }
   else {
      i64NewVersion = GetLongVersion(newVerStr);
      TRACE6("%s i64NewVersion:%I64d  i64CurVersion:%I64d\n", __FUNCTION__, i64NewVersion, i64CurVersion);
      if (i64NewVersion > i64CurVersion) {
         TRACE6("A new version is detected\n");
         if (!CanSkipUpdate) {
            this->StartDownload();
            return UPDATE_CHECK_VER;
         }
         else {
            VHUpdateTips tips(NULL, mIsForceUpdata);
            tips.SetVersion(QString::fromStdString(this->mNewVersion));
            tips.SetTip(this->mDownloadTips);
            ConfigSetting::MoveWindow(&tips);
            this->setWindowOpacity(0);
            if (tips.exec() == QDialog::Accepted) {
               this->StartDownload();
               return UPDATE_CHECK_VER;
            }
            else {
               TRACE6("Choose to skip the software update\n");
               this->accept();
               this->setWindowOpacity(100);
            }
         }
      }
      else {
         TRACE6("%s accept\n", __FUNCTION__);
         this->accept();
      }
   }
   return SUCCESS_CHECK_VER;
}
QString CCheckVersionDlg::GetDownloadFilePath() {
   return mDownloadFilePath;
}

IA64	CCheckVersionDlg::GetLongVersion(string sVersion) {
   int				i = 0;
   vector<string>	vers;
   IA64			i64Ret = 0;
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
wstring CCheckVersionDlg::GetVersion() {
   return mCurrVersionW;
}

QString CCheckVersionDlg::GetQVersion() {
   return QString::fromStdWString(mCurrVersionW);
}

IA64	 CCheckVersionDlg::GetCurVersion(const wchar_t* exeFile, std::wstring &currentVersion) {
   IA64				_i64Ver = 0;
   DWORD				dwVerHnd = 0;
   VS_FIXEDFILEINFO*	pFileInfo = NULL;
   unsigned int		dwInfoSize = 0;
   char	*			pVersionInfo = NULL;
   DWORD				dwVerInfoSize = GetFileVersionInfoSizeW(exeFile, &dwVerHnd);

   if (dwVerInfoSize) {
      pVersionInfo = new char[dwVerInfoSize];
      GetFileVersionInfoW(exeFile, dwVerHnd, dwVerInfoSize, pVersionInfo);
      VerQueryValue(pVersionInfo, (LPTSTR)("\\"), (void**)&pFileInfo, &dwInfoSize);

      CString szAppVer;
      szAppVer.Format(_T("%d.%d.%d.%d"), HIWORD(pFileInfo->dwFileVersionMS), LOWORD(pFileInfo->dwFileVersionMS),
         HIWORD(pFileInfo->dwFileVersionLS), LOWORD(pFileInfo->dwFileVersionLS));
      currentVersion = (LPCWSTR)szAppVer;

      _i64Ver = pFileInfo->dwProductVersionMS;
      _i64Ver = _i64Ver << 32;
      _i64Ver |= pFileInfo->dwProductVersionLS;

      delete[] pVersionInfo;
      pVersionInfo = NULL;
   }
   return _i64Ver;
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
void CCheckVersionDlg::paintEvent(QPaintEvent *) {
   QPainter painter(this);
   if (!m_bSplashHide) {
      painter.drawPixmap(rect(), this->pixmap);
   }
}

void CCheckVersionDlg::slot_authenticationRequired(QNetworkReply *reply, QAuthenticator *auth) {
   SetProxyUserInfo(auth);
}

void CCheckVersionDlg::slot_proxyAuthenticationRequired(const QNetworkProxy &, QAuthenticator *auth) {
   SetProxyUserInfo(auth);
}

void CCheckVersionDlg::SetProxyUserInfo(QAuthenticator *auth) {
   if (auth && !auth->isNull()) {
      QString qsConfPath = CPathManager::GetConfigPath();
      QString usr = ConfigSetting::ReadString(qsConfPath, GROUP_DEFAULT, PROXY_USERNAME, "");
      QString pwd = ConfigSetting::ReadString(qsConfPath, GROUP_DEFAULT, PROXY_PASSWORD, "");
      auth->setUser(usr);
      auth->setPassword(pwd);
      TRACE6("%s proxy_user:%s  proxy_pwd:%s\n", __FUNCTION__, usr.toStdString().c_str(), pwd.toStdString().c_str());
   }
}

void CCheckVersionDlg::SetNetworkProxy(QNetworkAccessManager* manager) {
   QNetworkProxy proxy;
   QString configPath = CPathManager::GetConfigPath();
   int isOpenProxy = ConfigSetting::ReadInt(configPath, GROUP_DEFAULT, PROXY_OPEN, 0);
   if (isOpenProxy == 1) {
      QString ip = ConfigSetting::ReadString(configPath, GROUP_DEFAULT, PROXY_HOST, "");
      int port = ConfigSetting::ReadInt(configPath, GROUP_DEFAULT, PROXY_PORT, 80);
      proxy.setHostName(ip);
      proxy.setPort(port);
      proxy.setType(QNetworkProxy::HttpProxy);
   }
   else {
      proxy.setType(QNetworkProxy::NoProxy);
   }
   manager->setProxy(proxy);
}


