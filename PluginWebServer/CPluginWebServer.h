#ifndef CPLUGINWEBSERVER_H
#define CPLUGINWEBSERVER_H

#include <QObject>

#include "IPluginWebServer.h"
#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/http.h>
#include <event2/http_struct.h>    
#include <event2/keyvalq_struct.h> 
#include <QThread>
#include <QMutex>


class CPluginWebServer :public IPluginWebServer, public QThread {
public:
   static CPluginWebServer* GetInstance();

   virtual int StartHttpServer(const char* ip, const int port);
   virtual int StopHttpServer();
   virtual void RegisterHttpDateCallback(HttpRequestData);
   virtual char *HttpUrlEncode(const char* url, const int Len, bool bSpaceToPlus);
   virtual char* ClientHttpGet(const char* url, int &nOutLen, bool &bSuc);

protected:
   virtual void run();
private:
   static unsigned int RunHttpSeverRecvThread(void *);
   CPluginWebServer();
   virtual ~CPluginWebServer();
   int InitHttpServer();
   static void DestoryHttpServer();

   int start_url_request(struct http_request_get *http_req, int req_get_flag);
   void InitWebServerDebugTrace(std::wstring awstrDllName, int aiTraceLevel, bool abUseSameLog/* = false*/);
private:
   static void HttpRequestCallBack(struct evhttp_request *req, void *arg);
   static struct event_base* m_pHttpBase;
   static struct evhttp *m_pHttpServer;
   static HttpRequestData m_pHttpDataCb;
   static  QMutex mHttpServerMutex;
   static CPluginWebServer* m_pInstance;
   static HANDLE m_pEvent;
};

#endif // CPLUGINWEBSERVER_H
