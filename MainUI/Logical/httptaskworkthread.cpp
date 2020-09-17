#include "StdAfx.h"
#include "httptaskworkthread.h"
#include "Msg_CommonToolKit.h"
#include "pathmanager.h"
#include "ConfigSetting.h"
#include "DebugTrace.h"
#include "IPluginWebServer.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QEventLoop>
#include <QNetworkProxy>
#include <QtNetwork/QAuthenticator>

#define MAX_HTTP_TIME_OUT 10000

HttpTaskWorkThread::HttpTaskWorkThread(QObject *parent)
   : QThread(parent)
{
   mHttpManager = new QNetworkAccessManager;

}

HttpTaskWorkThread::~HttpTaskWorkThread()
{
   if (mHttpManager) {
      delete mHttpManager;
      mHttpManager = NULL;
   }
   TRACE6("%s delete mHttpManager end\n", __FUNCTION__);
   if (mTaskParam) {
      delete mTaskParam;
      mTaskParam = NULL;
   }
   TRACE6("%s delete mTaskParam end\n", __FUNCTION__);
   if (mLoopEvent) {
      delete mLoopEvent;
      mLoopEvent = NULL;
   }
   TRACE6("%s delete  end\n", __FUNCTION__);
}

void HttpTaskWorkThread::run() {
   QString url = QString::fromWCharArray(mTaskParam->m_wzRequestUrl);

   if (true/*url.contains("cut-record")*/) {
      mLoopEvent = new QEventLoop;
      QTimer timer;
      QString qsConfPath = CPathManager::GetConfigPath();
      QNetworkProxy proxy;
      QString ip;
      int port;
      int is_http_proxy = ConfigSetting::ReadInt(qsConfPath, GROUP_DEFAULT, PROXY_OPEN, 0);
      if (is_http_proxy) {
         ip = ConfigSetting::ReadString(qsConfPath, GROUP_DEFAULT, PROXY_HOST, "");
         port = ConfigSetting::ReadInt(qsConfPath, GROUP_DEFAULT, PROXY_PORT, 80);
         mUserName = ConfigSetting::ReadString(qsConfPath, GROUP_DEFAULT, PROXY_USERNAME, "");
         mUserPwd = ConfigSetting::ReadString(qsConfPath, GROUP_DEFAULT, PROXY_PASSWORD, "");
         proxy.setHostName(ip);
         proxy.setPort(port);
         proxy.setType(QNetworkProxy::HttpProxy);
      }
      else {
         proxy.setType(QNetworkProxy::NoProxy);
      }

      if (mHttpManager && mLoopEvent) {
         mHttpManager->setProxy(proxy);
         mHttpTimeOut.start(MAX_HTTP_TIME_OUT);
         QObject::connect(&mHttpTimeOut, SIGNAL(timeout()), mLoopEvent, SLOT(quit()));
         bool bConn = QObject::connect(mHttpManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(slot_eventLoopQuit(QNetworkReply*)));
         QObject::connect(mHttpManager, SIGNAL(authenticationRequired(QNetworkReply *, QAuthenticator *)), this, SLOT(slot_authenticationRequired(QNetworkReply *, QAuthenticator *)));
         QObject::connect(mHttpManager, SIGNAL(proxyAuthenticationRequired(const QNetworkProxy &, QAuthenticator *)), this, SLOT(slot_proxyAuthenticationRequired(const QNetworkProxy &, QAuthenticator *)));

         QString urlStr = QString::fromWCharArray(mTaskParam->m_wzRequestUrl);
         QUrl url = QUrl(urlStr);
         QNetworkRequest req = QNetworkRequest(url);
         QNetworkReply *reply = mHttpManager->get(req);
         QObject::connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slot_networkReplyErr(QNetworkReply::NetworkError)), Qt::ConnectionType::DirectConnection);
         QObject::connect(reply, SIGNAL(finished()), mLoopEvent, SLOT(quit()), Qt::ConnectionType::DirectConnection);

         TRACE6("%s Url %s\n", __FUNCTION__, urlStr.toStdString().c_str());
         mLoopEvent->exec();
         mHttpTimeOut.stop();
         if (reply) {
            QString qStrResult = reply->readAll();
            int errCode = reply->error();
            TRACE6("%s errCode %d  Url %s qStrResult:%s\n", __FUNCTION__, errCode, urlStr.mid(512).toStdString().c_str(), qStrResult.mid(512).toStdString().c_str());
            mTaskParam->m_bIsSuc = TRUE;
            mTaskParam->m_i64UserData = errCode;
            wstring strResult = qStrResult.mid(0, DEF_MAX_HTTP_URL_LEN).toStdWString();
            wcscpy_s(mTaskParam->m_wzUrlData, strResult.c_str());
            STRU_HTTPCENTER_HTTP_RS aoRs;
            aoRs = *mTaskParam;
            SingletonMainUIIns::Instance().PostCRMessage(MSG_HTTPCENTER_HTTP_RS, &aoRs, sizeof(aoRs));
            reply->deleteLater();
         }
         else {
            TRACE6("%s reply is null Url %s\n", __FUNCTION__, urlStr.toStdString().c_str());
            STRU_HTTPCENTER_HTTP_RS aoRs;
            mTaskParam->m_bIsSuc = TRUE;
            mTaskParam->m_i64UserData = -1;
            aoRs = *mTaskParam;
            SingletonMainUIIns::Instance().PostCRMessage(MSG_HTTPCENTER_HTTP_RS, &aoRs, sizeof(aoRs));
         }
      }
   }
   else {
      QString urlStr = QString::fromWCharArray(mTaskParam->m_wzRequestUrl);
      IPluginWebServer *httpClient = GetHttpServerInstance();
      if (httpClient) {
         char *outData = NULL;
         int nOutLen = 0;
         bool bSuc = false;
         TRACE6("ExecuteRemoteTask %s\n", urlStr.toStdString().c_str());

         outData = httpClient->ClientHttpGet(urlStr.toStdString().c_str(), nOutLen, bSuc);
         if (outData) {
            QString qStrResult = QByteArray(outData, nOutLen);
            int errCode = 0;
            TRACE6("%s errCode:%d\n", __FUNCTION__, bSuc);
            mTaskParam->m_bIsSuc = TRUE;
            mTaskParam->m_i64UserData = errCode;
            wcscpy_s(mTaskParam->m_wzUrlData, qStrResult.toStdWString().c_str());
            STRU_HTTPCENTER_HTTP_RS aoRs;
            aoRs = *mTaskParam;
            SingletonMainUIIns::Instance().PostCRMessage(MSG_HTTPCENTER_HTTP_RS, &aoRs, sizeof(aoRs));
         }
         else {
            TRACE6("HttpTaskWorkThread::ExecuteRemoteTask Reply NULL\n");
            STRU_HTTPCENTER_HTTP_RS aoRs;
            mTaskParam->m_bIsSuc = TRUE;
            mTaskParam->m_i64UserData = -1;
            aoRs = *mTaskParam;
            SingletonMainUIIns::Instance().PostCRMessage(MSG_HTTPCENTER_HTTP_RS, &aoRs, sizeof(aoRs));
         }
         if (outData != NULL) {
            delete[]outData;
         }
      }
   }
}

void HttpTaskWorkThread::SetTaskParam(STRU_HTTPCENTER_HTTP_RS& param) {
   mTaskParam = new STRU_HTTPCENTER_HTTP_RS();
   if (mTaskParam) {
      *mTaskParam = param;
   }
}

void HttpTaskWorkThread::QuitEventLoop() {
   if (mLoopEvent) {
      mLoopEvent->quit();
   }
}

void HttpTaskWorkThread::slot_authenticationRequired(QNetworkReply *, QAuthenticator *auth) {
   if (auth && !auth->isNull()) {
      auth->setUser(mUserName);
      auth->setPassword(mUserPwd);
   }
}

void HttpTaskWorkThread::slot_proxyAuthenticationRequired(const QNetworkProxy &, QAuthenticator *auth) {
   if (auth && !auth->isNull()) {
      auth->setUser(mUserName);
      auth->setPassword(mUserPwd);
   }
}

void HttpTaskWorkThread::slot_networkReplyErr(QNetworkReply::NetworkError) {
   QuitEventLoop();
}

void HttpTaskWorkThread::slot_eventLoopQuit(QNetworkReply*) {
   QuitEventLoop();
}

void HttpTaskWorkThread::SetHttpManger(QNetworkAccessManager* manager) {
   mHttpManager = manager;
}


