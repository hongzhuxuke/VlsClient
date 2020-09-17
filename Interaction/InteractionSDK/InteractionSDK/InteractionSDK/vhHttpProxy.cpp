#include "vhHttpProxy.h"
#include <QTimer>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkProxy>
#include <QtNetwork/QAuthenticator>
#include <QEventLoop>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkCookie>
#include <QNetworkCookieJar>
#include "DebugTrace.h"

InternalHttpRequest::InternalHttpRequest(QObject* obj) :QObject(obj)
, mpTimerLoopTimeOut(NULL){
}

InternalHttpRequest::~InternalHttpRequest() {
   TRACE6("%s start\n", __FUNCTION__);
	if (mpTimerLoopTimeOut) {
		if (mpTimerLoopTimeOut->isActive()){
			mpTimerLoopTimeOut->stop();
		}
		//mpTimerLoopTimeOut->deleteLater();
        delete mpTimerLoopTimeOut;
		mpTimerLoopTimeOut = NULL;
	}

   if (mpPostTimer) {
      mpPostTimer->stop();
      //mpPostTimer->deleteLater();
      delete mpPostTimer;
      mpPostTimer = NULL;
   }

   if (m_pVHPostEventLoop) {
      m_pVHPostEventLoop->quit();
      //m_pVHPostEventLoop->deleteLater();
      delete m_pVHPostEventLoop;
      m_pVHPostEventLoop = NULL;
   }

   if (m_pGetEventLoop && m_pVHGetManager) {
      //m_pEventLoop->deleteLater();
      delete m_pGetEventLoop;
      m_pGetEventLoop = NULL;
   }
   TRACE6("%s end\n", __FUNCTION__);
}

bool InternalHttpRequest::VHGetRet2(const QString& url, int time_out, QByteArray &ba, int& errCode, bool notTimeOut, QString io) {
   QString strTimeTemp = QDateTime::currentDateTime().toString("yyyyMMddhhmmssz");
   QNetworkProxy proxy;
   if (m_bEnableProxy) {
      proxy.setHostName(m_ip);
      proxy.setPort(m_port);
      proxy.setType(QNetworkProxy::HttpProxy);
   } else {
      proxy.setType(QNetworkProxy::NoProxy);
      QNetworkProxy::setApplicationProxy(proxy);
   }
   if (m_pVHGetManager == NULL) {
      m_pVHGetManager = new QNetworkAccessManager(this);
      if (m_pVHGetManager != NULL) {
         QObject::connect(m_pVHGetManager, SIGNAL(authenticationRequired(QNetworkReply *, QAuthenticator *)), this, SLOT(slot_authenticationRequired(QNetworkReply *, QAuthenticator *)));
         QObject::connect(m_pVHGetManager, SIGNAL(proxyAuthenticationRequired(const QNetworkProxy &, QAuthenticator *)), this, SLOT(slot_proxyAuthenticationRequired(const QNetworkProxy &, QAuthenticator *)));
      }
   }
   QNetworkCookieJar m_pCookieJar;
   if (m_pVHGetManager != NULL) {
      m_pVHGetManager->setProxy(proxy);
   }
   if (m_pGetEventLoop == NULL) {
      m_pGetEventLoop = new QEventLoop(this);
      QObject::connect(m_pVHGetManager, SIGNAL(finished(QNetworkReply *)), m_pGetEventLoop, SLOT(quit()));
	  QObject::connect(m_pVHGetManager, SIGNAL(sslErrors(QNetworkReply *, const QList<QSslError> &)), m_pGetEventLoop, SLOT(quit()));
   }  

   QNetworkRequest request(url);
   if (notTimeOut) {
      //request.setRawHeader("Cookie", io.toStdString().c_str());
   }

   QNetworkReply *reply = NULL;
   if (m_pVHGetManager){
      reply = m_pVHGetManager->get(request);
   }

   if (NULL == mpTimerLoopTimeOut)
   {
	   mpTimerLoopTimeOut = new QTimer(this);
	   if (NULL != mpTimerLoopTimeOut)
	   {
		   QObject::connect(mpTimerLoopTimeOut, SIGNAL(timeout()), m_pGetEventLoop, SLOT(quit()));
		   mpTimerLoopTimeOut->setSingleShot(true);
	   }
   }
     
   if (NULL != mpTimerLoopTimeOut)
   {
	   if (time_out > 0)
	   {
		   mpTimerLoopTimeOut->start(time_out);
	   }
	   else
	   {
		   mpTimerLoopTimeOut->start(10000);
	   }
   }
    
   if (m_pGetEventLoop) {
	   //TRACE6("%s pEventLoop->exec %s  %s \n", __FUNCTION__, strTimeTemp.toStdString().c_str(), url.mid(0, 1204).toStdString().c_str());
      m_pGetEventLoop->exec();
   }

   if (mpTimerLoopTimeOut->isActive())
	mpTimerLoopTimeOut->stop();

   //TRACE6(" *****  %s pEventLoop->exec  QUIT %s  ***** %s \n", __FUNCTION__, strTimeTemp.toStdString().c_str(), url.mid(0, 1204).toStdString().c_str());
   if (reply != NULL) {
      reply->abort();
      reply->close();
      errCode = reply->error();
      ba = reply->readAll();
      TRACE6("%s errCode:%d \n", __FUNCTION__, errCode);
      reply->deleteLater();
      reply = NULL;
   }
   return true;
}

bool InternalHttpRequest::VHPostRet(const QString& url, /*int timeout, QByteArray &ba,*/ QString &json,QString cookis) {
   QTimer timer;
   QUrl urlTemp(url);
   QNetworkRequest network(urlTemp);
   QNetworkRequest request(url);

   QNetworkProxy proxy;
   if (m_bEnableProxy) {
       proxy.setHostName(m_ip);
       proxy.setPort(m_port);
       proxy.setType(QNetworkProxy::HttpProxy);
   }
   else {
       proxy.setType(QNetworkProxy::NoProxy);
       QNetworkProxy::setApplicationProxy(proxy);
   }

   if (!cookis.isEmpty()) { 
      QString io = "io=" + cookis;
      request.setRawHeader("Cookie", io.toStdString().c_str());
      request.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/json"));
   }

   if (m_pVHPostEventLoop == NULL) {
      m_pVHPostEventLoop = new QEventLoop(this);
   }     
   if (mpPostTimer == NULL) {
      mpPostTimer = new QTimer(this);
      if (mpPostTimer) {
         QObject::connect(mpPostTimer, SIGNAL(timeout()), m_pVHPostEventLoop, SLOT(quit()));
         mpPostTimer->setSingleShot(true);
      }
   }

   QNetworkAccessManager postManager;// = new QNetworkAccessManager(this);
   postManager.setProxy(proxy);
   QObject::connect(&postManager, SIGNAL(authenticationRequired(QNetworkReply *, QAuthenticator *)), this, SLOT(slot_authenticationRequired(QNetworkReply *, QAuthenticator *)));
   QObject::connect(&postManager, SIGNAL(proxyAuthenticationRequired(const QNetworkProxy &, QAuthenticator *)), this, SLOT(slot_proxyAuthenticationRequired(const QNetworkProxy &, QAuthenticator *)));
   if (m_pVHPostEventLoop) {
      QObject::connect(&postManager, SIGNAL(finished(QNetworkReply *)), m_pVHPostEventLoop, SLOT(quit()));
      QObject::connect(&postManager, SIGNAL(sslErrors(QNetworkReply*, const QList<QSslError>&)), m_pVHPostEventLoop, SLOT(quit()));
   }
   TRACE6("%s json:%s \n", __FUNCTION__, json.toStdString().c_str());
   if (mpPostTimer) {
      mpPostTimer->start(HTTP_LONG_POLLING_HEART_TIME / 2);
   }
   CCriticalAutoLock cs(mNetWorkManagerCS);
   QNetworkReply *reply = postManager.post(request, QByteArray(json.toStdString().c_str()));
   m_pVHPostEventLoop->exec();
   int err = -1;
   if (reply) {
      QByteArray arry = reply->readAll();
      err = reply->error();
      std::string resp(arry.data(), arry.size());
      reply->abort();
      reply->deleteLater();
   }
   if (mpPostTimer) {
      mpPostTimer->stop();
   }

   QObject::disconnect(&postManager, SIGNAL(authenticationRequired(QNetworkReply *, QAuthenticator *)), this, SLOT(slot_authenticationRequired(QNetworkReply *, QAuthenticator *)));
   QObject::disconnect(&postManager, SIGNAL(proxyAuthenticationRequired(const QNetworkProxy &, QAuthenticator *)), this, SLOT(slot_proxyAuthenticationRequired(const QNetworkProxy &, QAuthenticator *)));
   if (m_pVHPostEventLoop) {
      QObject::disconnect(&postManager, SIGNAL(finished(QNetworkReply *)), m_pVHPostEventLoop, SLOT(quit()));
      QObject::disconnect(&postManager, SIGNAL(sslErrors(QNetworkReply*, const QList<QSslError>&)), m_pVHPostEventLoop, SLOT(quit()));
   }
   
   TRACE6("%s err:%d\n", __FUNCTION__, err);
   return true;
}

void InternalHttpRequest::slot_authenticationRequired(QNetworkReply *replay, QAuthenticator *auth) {
   if (auth && !auth->isNull()) {
      auth->setUser(m_user);
      auth->setPassword(m_pwd);
   }
}

void InternalHttpRequest::slot_proxyAuthenticationRequired(const QNetworkProxy &proxy, QAuthenticator *auth) {
   if (auth && !auth->isNull()) {
      auth->setUser(m_user);
      auth->setPassword(m_pwd);
   }
}

void InternalHttpRequest::SetHttpProxyInfo(bool enable,QString ip, unsigned short port, QString userName, QString password) {
   m_ip = ip;
   m_port = port;
   m_user = userName;
   m_pwd = password;
   m_bEnableProxy = enable;
}

void InternalHttpRequest::ExitLoop() {
   if (m_pGetEventLoop) {
      m_pGetEventLoop->quit();
   }
   if (m_pVHPostEventLoop) {
      m_pVHPostEventLoop->quit();
   }
   if (mpPostTimer) {
      mpPostTimer->stop();
   }
}

void InternalHttpRequest::StopSendPing() {
   if (mpPostTimer) {
      mpPostTimer->stop();
   }

   if (m_pVHPostEventLoop) {
      m_pVHPostEventLoop->quit();
   }
}

void InternalHttpRequest::Release() {
   if (m_pVHGetManager != NULL) {
      delete m_pVHGetManager;
      m_pVHGetManager = NULL;
   }

   if (m_pGetEventLoop) {
      delete m_pGetEventLoop;
      m_pGetEventLoop = NULL;
   }
}