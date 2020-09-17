#include "StdAfx.h"
#include "HttpReqLogic.h"
#include "Msg_CommonToolKit.h"
#include "pathmanager.h"
#include "ConfigSetting.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QEventLoop>
#include <QNetworkProxy>
#include <QtNetwork/QAuthenticator>
#include <QTimer>

HttpReqLogic::HttpReqLogic(QObject *parent)
: QObject(parent)
, m_bIsAlive(FALSE)
, m_hEvent(NULL){

}

HttpReqLogic::~HttpReqLogic(void) {
   if (m_hEvent != NULL) {
      CloseHandle(m_hEvent);
      m_hEvent = NULL;
   }
   if (m_qNetAccessMgr) {
      delete m_qNetAccessMgr;
      m_qNetAccessMgr = NULL;
   }

   if (m_hEixtThreadEvent) {
      CloseHandle(m_hEixtThreadEvent);
      m_hEixtThreadEvent = NULL;
   }
}

int HttpReqLogic::Create() {
   if (m_bIsAlive) {
      return TRUE;
   }
   m_qNetAccessMgr = new QNetworkAccessManager();
   if (m_qNetAccessMgr) {
      connect(m_qNetAccessMgr, SIGNAL(authenticationRequired(QNetworkReply *, QAuthenticator *)), this, SLOT(slot_authenticationRequired(QNetworkReply *, QAuthenticator *)));
      connect(m_qNetAccessMgr, SIGNAL(proxyAuthenticationRequired(const QNetworkProxy &, QAuthenticator *)), this, SLOT(slot_proxyAuthenticationRequired(const QNetworkProxy &, QAuthenticator *)));
   }
   m_bIsAlive = TRUE;
   CBaseThread loBaseThread;
   //创建工作线程轮询URL请求
   loBaseThread.BeginThread(HttpReqLogic::LoopReqThread, this);
   return TRUE;
}

void HttpReqLogic::Destroy() {
   m_bIsAlive = FALSE;
   if (m_hEvent) {
      ::SetEvent(m_hEvent);
   }
   if (m_hEixtThreadEvent) {
      WaitForSingleObject(m_hEixtThreadEvent, 60000);
   }
}

unsigned int HttpReqLogic::LoopReqThread(void * apParam) {
   HttpReqLogic* pThis = static_cast<HttpReqLogic*>(apParam);
   if (pThis) {
      pThis->LoopReqWork();
   }
   return 0;
}

unsigned int HttpReqLogic::LoopReqWork() {
   TRACE4("HttpReqLogic::LoopReqWork %lu\n",GetCurrentThreadId());
   //创建事件句柄(自动重置)
   m_hEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
   m_hEixtThreadEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
   if (m_hEvent) {
      while (m_bIsAlive) {
         //取得一个任务
         STRU_HTTPCENTER_HTTP_RQ oReq;
         if (GetReqTask(oReq)) {
            //执行请求
            ExecuteRemoteTask(oReq);
            continue;
         }
         else {
            ::WaitForSingleObject(m_hEvent, 1000);
         }
      }
   }
   if (m_hEixtThreadEvent) {
      ::SetEvent(m_hEixtThreadEvent);
   }
   return 0;
}

void HttpReqLogic::ExecuteRemoteTask(STRU_HTTPCENTER_HTTP_RQ aoReq) {
   QString qsConfPath = CPathManager::GetConfigPath();
   QNetworkProxy proxy;
   int is_http_proxy = ConfigSetting::ReadInt(qsConfPath, GROUP_DEFAULT, PROXY_OPEN, 0);
   if (is_http_proxy) {
      m_ip = ConfigSetting::ReadString(qsConfPath, GROUP_DEFAULT, PROXY_HOST, "");
      m_port = ConfigSetting::ReadInt(qsConfPath, GROUP_DEFAULT, PROXY_PORT, 80);
      m_user = ConfigSetting::ReadString(qsConfPath, GROUP_DEFAULT, PROXY_USERNAME, "");
      m_pwd = ConfigSetting::ReadString(qsConfPath, GROUP_DEFAULT, PROXY_PASSWORD, "");
      proxy.setHostName(m_ip);
      proxy.setPort(m_port);
      proxy.setType(QNetworkProxy::HttpProxy);
   } else {
      proxy.setType(QNetworkProxy::NoProxy);
   }
   if (m_qNetAccessMgr){
      m_qNetAccessMgr->setProxy(proxy);
      QEventLoop loop;
      QObject::connect(m_qNetAccessMgr, SIGNAL(finished(QNetworkReply*)), &loop, SLOT(quit()), Qt::DirectConnection);
      QString urlStr = QString::fromWCharArray(aoReq.m_wzRequestUrl);
      QUrl url = QUrl(urlStr);
      QNetworkRequest req = QNetworkRequest(url);
      QByteArray host = url.host().toUtf8();
      QByteArray port = QString::number(url.port()).toUtf8();
      req.setRawHeader("HOST", url.host().toUtf8());
      req.setRawHeader("PORT", QString::number(url.port()).toUtf8());  
      QNetworkReply *reply = m_qNetAccessMgr->get(req);
      TRACE6("ExecuteRemoteTask Url %s\n", QString::fromWCharArray(aoReq.m_wzRequestUrl).toStdString().c_str());
      QTimer timeOut;
      connect(&timeOut, SIGNAL(timeout()),&loop, SLOT(quit()));
      timeOut.setSingleShot(true);
      timeOut.start(5000);
      loop.exec();
      if (reply) {
         QString qStrResult = reply->readAll();
         int errCode = reply->error();
         if (!reply->isFinished()) {
            reply->abort();
         }
         STRU_HTTPCENTER_HTTP_RS aoRs;
         aoRs.m_bIsSuc = TRUE;
         aoRs.m_dwPluginId = aoReq.m_dwPluginId;
         aoRs.m_i64UserData = aoRs.m_i64UserData;
         wcsncpy(aoRs.m_wzRequestUrl, aoReq.m_wzRequestUrl, DEF_MAX_HTTP_URL_LEN);
         wcsncpy(aoRs.m_wzUrlData, qStrResult.toStdWString().c_str(), DEF_MAX_HTTP_URL_LEN);
         SingletonCommonToolKitIns::Instance().PostCRMessage(MSG_HTTPCENTER_HTTP_RS, &aoRs, sizeof(aoRs));
         reply->deleteLater();
      }
      else {
         TRACE6("ExecuteRemoteTask Reply NULL\n");
      }
      QObject::disconnect(m_qNetAccessMgr, SIGNAL(finished(QNetworkReply*)), &loop, SLOT(quit()));
      m_qNetAccessMgr->clearAccessCache();
      TRACE6("%s end\n",__FUNCTION__);
   }
}

void HttpReqLogic::AddReqTask(const STRU_HTTPCENTER_HTTP_RQ& aoReq) {
   CCriticalAutoLock loLock(m_oReqTaskLock);
   //添加到请求队列
   m_oHttpReqList.push_back(aoReq);
   if (m_hEvent) {
      ::SetEvent(m_hEvent);
   }
}

//取得一个待下载的任务
BOOL HttpReqLogic::GetReqTask(STRU_HTTPCENTER_HTTP_RQ& aoReq) {
   CCriticalAutoLock loLock(m_oReqTaskLock);
   if (m_oHttpReqList.size() < 1) {
      return FALSE;
   }

   aoReq = m_oHttpReqList.front();
   m_oHttpReqList.pop_front();
   return TRUE;
}

void HttpReqLogic::slot_authenticationRequired(QNetworkReply *, QAuthenticator *auth) {
   if (auth && !auth->isNull()) {
      auth->setUser(m_user);
      auth->setPassword(m_pwd);
   }
}

void HttpReqLogic::slot_proxyAuthenticationRequired(const QNetworkProxy &, QAuthenticator *auth) {
   if (auth && !auth->isNull()) {
      auth->setUser(m_user);
      auth->setPassword(m_pwd);
   }
}
