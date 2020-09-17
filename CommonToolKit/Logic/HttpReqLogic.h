#ifndef __HTTP_REQ_LOGIC__H_INCLUDE__
#define __HTTP_REQ_LOGIC__H_INCLUDE__

#pragma once
#include <QObject>
#include <atomic>
#include <windows.h>
#include <atomic>
#include "CriticalSection.h"

class STRU_HTTPCENTER_HTTP_RQ;
class QNetworkAccessManager;
class QNetworkProxy;
class QNetworkReply;
class QAuthenticator;

class HttpReqLogic: public QObject
{
   Q_OBJECT
public:
   HttpReqLogic(QObject *parent = NULL);
	~HttpReqLogic(void);

   int Create();
   void Destroy();

   //添加请求任务
   void AddReqTask(const STRU_HTTPCENTER_HTTP_RQ& aoReq);

private slots:
   void slot_authenticationRequired(QNetworkReply *, QAuthenticator *);
   void slot_proxyAuthenticationRequired(const QNetworkProxy &, QAuthenticator *);
private:
   static unsigned int LoopReqThread(void * apParam);
   unsigned int LoopReqWork();

   void ExecuteRemoteTask(STRU_HTTPCENTER_HTTP_RQ aoReq);

   //获取一个待请求的任务
   int GetReqTask(STRU_HTTPCENTER_HTTP_RQ& aoReq);

private:
   CCriticalSection		m_oReqTaskLock;	      //任务请求队列锁
   std::list<STRU_HTTPCENTER_HTTP_RQ> m_oHttpReqList;
   std::atomic_int m_bIsAlive;
   HANDLE m_hEvent = NULL;
   HANDLE m_hEixtThreadEvent = NULL;
   QNetworkAccessManager* m_qNetAccessMgr = nullptr;

   QString m_ip;
   int m_port;
   QString m_user;
   QString m_pwd;
};

#endif //__HTTP_REQ_LOGIC__H_INCLUDE__