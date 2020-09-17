#include "CPluginWebServer.h"
#include <stdlib.h>
#include <stdio.h>
#include <QDebug>
#include <windows.h>
#include <ShlObj.h>
#include <tchar.h>
#include "DebugTrace.h"


CPluginWebServer* CPluginWebServer::m_pInstance = NULL;
HttpRequestData CPluginWebServer::m_pHttpDataCb = NULL;
struct event_base* CPluginWebServer::m_pHttpBase = NULL;
struct evhttp *CPluginWebServer::m_pHttpServer = NULL;
HANDLE CPluginWebServer::m_pEvent = NULL;
QMutex CPluginWebServer::mHttpServerMutex;

struct http_request_get {
   struct evhttp_uri *uri = NULL;
   struct event_base *base = NULL;
   struct evhttp_connection *cn = NULL;
   struct evhttp_request *req = NULL;
   char *requestData = NULL;
   int nDataLen = 0;
   int nCode = -1;
};

void http_requset_get_cb(struct evhttp_request *req, void *arg)
{
   struct http_request_get *http_req_get = (struct http_request_get *)arg;
   if (req) {
      switch (req->response_code)
      {
      case HTTP_OK:
      {
         struct evbuffer* buf = evhttp_request_get_input_buffer(req);
         size_t len = evbuffer_get_length(buf);
         http_req_get->requestData = new char[len + 1];
         memset(http_req_get->requestData, 0, len + 1);
         memcpy(http_req_get->requestData, evbuffer_pullup(buf, -1), len);
         http_req_get->nDataLen = len;
         http_req_get->nCode = 0;
         event_base_loopexit(http_req_get->base, 0);
         break;
      }
      default:
         event_base_loopexit(http_req_get->base, 0);
         return;
      }
   }
   else {
      event_base_loopexit(http_req_get->base, 0);
   }
}

CPluginWebServer::CPluginWebServer() {
   m_pEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
   InitWebServerDebugTrace(L"PluginWebServer", 6, false);
}

CPluginWebServer::~CPluginWebServer() {
   DestoryHttpServer();
   bool isThreadRuning = isRunning();
   TRACE6("%s CPluginWebServer threadRuning:%d \n", __FUNCTION__, isThreadRuning);
   if (isThreadRuning) {
      quit();
      if (m_pHttpBase) {
         StopHttpServer();
      }
      if (m_pEvent) {
         TRACE6("%s CPluginWebServer quit thread \n", __FUNCTION__);
         WaitForSingleObject(m_pEvent, 5000);
      }
   }
   int count = 0;
   while (isRunning() && count < 5) {
      TRACE6("%s wait thread exit\n", __FUNCTION__);
      quit();
      Sleep(1000);
      count++;
   }
   m_pInstance = NULL;
   if (m_pEvent) {
      CloseHandle(m_pEvent);
   }
   TRACE6("%s CPluginWebServer release ok\n", __FUNCTION__);
}

int CPluginWebServer::InitHttpServer() {
   return 0;
}

int CPluginWebServer::StartHttpServer(const char* ip, const int port) {
   TRACE6("%s InitHttpServer\n", __FUNCTION__);
   if (InitHttpServer() != 0) {
      return ERR_INIT;
   }
   TRACE6("%s event_base_new\n", __FUNCTION__);
   m_pHttpBase = event_base_new();
   if (m_pHttpBase == NULL) {
      return ERR_NEW_EVENTBASE;
   }
   TRACE6("%s evhttp_new\n", __FUNCTION__);
   m_pHttpServer = evhttp_new(m_pHttpBase);
   if (m_pHttpServer == NULL) {
      DestoryHttpServer();
      return ERR_NEW_HTTPSERVER;
   }
   TRACE6("%s evhttp_bind_socket\n", __FUNCTION__);
   int ret = evhttp_bind_socket(m_pHttpServer, ip, port);
   if (ret != 0) {
      DestoryHttpServer();
      return ERR_BIND_SOCKET;
   }
   TRACE6("%s evhttp_set_gencb\n", __FUNCTION__);
   evhttp_set_gencb(m_pHttpServer, HttpRequestCallBack, NULL);
   this->start();
   TRACE6("%s start thread\n", __FUNCTION__);
   return 0;
}
     
void CPluginWebServer::HttpRequestCallBack(struct evhttp_request *req, void *arg) {
   //int nRequestType = evhttp_request_get_command(req);
   //const char *httpUrl = evhttp_request_get_uri(req);
   //curl http://127.0.0.1:80/check_your_status?user=Summer&passwd=12345678
   if (req == NULL)  {
      return;
   }
   const struct evhttp_uri* httpUrl = evhttp_request_get_evhttp_uri(req);
   if (httpUrl == NULL) {
      return;
   }
   const char *param = evhttp_uri_get_query(httpUrl);
   if (param == NULL) {
      return;
   }  

   char *decodeURI = evhttp_decode_uri(param);
   if (decodeURI != NULL) {
      int req_len = strlen(decodeURI);
      //struct evkeyvalq *headers = NULL;
      //struct evbuffer *req_evb = NULL;
      //int ret = 0;
      //int req_len = 0;
      char *req_buf = NULL;
      //headers = evhttp_request_get_input_headers(req);
      //req_evb = evhttp_request_get_input_buffer(req);
      //req_len = evbuffer_get_length(req_evb);

      req_buf = (char*)malloc(req_len + 1);
      if (!req_buf) {
         return;
      }
      memset(req_buf, 0, req_len + 1);
      memcpy(req_buf, decodeURI, req_len);
      //ret = evbuffer_remove(req_evb, req_buf, req_len);
      //if (ret != req_len) {
      //}
      req_buf[req_len] = 0;
      if (m_pHttpDataCb) {
         m_pHttpDataCb(req_buf, req_len);
         evhttp_send_reply(req, HTTP_OK, "OK", NULL);
      }
      free(req_buf);
      free(decodeURI);
   }
   
   //evbuffer_free(req_evb);
}

int CPluginWebServer::StopHttpServer() {
	int nRet = -1;
   if (m_pHttpBase != NULL){
      TRACE6("%s event_base_loopbreak \n", __FUNCTION__);
      nRet = event_base_loopbreak(m_pHttpBase);
   }
   TRACE6("%s event_base_loopbreak :%d \n", __FUNCTION__, nRet);
   return nRet;
}

void CPluginWebServer::RegisterHttpDateCallback(HttpRequestData cb) {
   m_pHttpDataCb = cb;
}

void CPluginWebServer::InitWebServerDebugTrace(std::wstring awstrDllName, int aiTraceLevel, bool abUseSameLog/* = false*/) {
   wchar_t lwzDllPath[MAX_PATH + 1] = { 0 };

   SHGetFolderPathW(NULL, CSIDL_MYDOCUMENTS, NULL, SHGFP_TYPE_CURRENT, lwzDllPath);
   wcscat_s(lwzDllPath, _MAX_PATH, L"\\VHallHelper");

   //首先设置日志打印选项
   CDebugTrace::SetTraceLevel(aiTraceLevel);

   CDebugTrace::SetTraceOptions(CDebugTrace::GetTraceOptions() \
      | CDebugTrace::Timestamp & ~CDebugTrace::LogLevel \
      & ~CDebugTrace::FileAndLine | CDebugTrace::AppendToFile\
      | CDebugTrace::PrintToConsole);

   //生成TRACE文件名
   SYSTEMTIME loSystemTime;
   GetLocalTime(&loSystemTime);

   wchar_t lwzLogFileName[255] = { 0 };

   if (abUseSameLog) {
      wsprintf(lwzLogFileName, L"%s%s%s_%4d_%02d_%02d_%02d_%02d%s", lwzDllPath, L"\\vhlog\\", L"GGLogFile", loSystemTime.wYear, loSystemTime.wMonth, loSystemTime.wDay, loSystemTime.wHour, loSystemTime.wMinute, L".log");
   }
   else {
      wsprintf(lwzLogFileName, L"%s%s%s_%4d_%02d_%02d_%02d_%02d%s", lwzDllPath, L"\\vhlog\\", awstrDllName.c_str(), loSystemTime.wYear, loSystemTime.wMonth, loSystemTime.wDay, loSystemTime.wHour, loSystemTime.wMinute, L".log");
   }

   CDebugTrace::SetLogFileName(lwzLogFileName);
}

CPluginWebServer* CPluginWebServer::GetInstance() {
   CPluginWebServer *pIns = NULL;
   QMutexLocker l(&mHttpServerMutex);
   if (m_pInstance == NULL) {
      m_pInstance = new CPluginWebServer();
   }
   pIns = m_pInstance;
   return pIns;
}

unsigned int CPluginWebServer::RunHttpSeverRecvThread(void *) {
   TRACE6("%s enter thread\n", __FUNCTION__);
   int nRet = event_base_dispatch(m_pHttpBase);
   if (m_pEvent) {
      TRACE6("%s SetEvent\n", __FUNCTION__);
      ::SetEvent(m_pEvent);
   }
   TRACE6("%s leave thread:%d\n", __FUNCTION__, nRet);
   return nRet;
}

void CPluginWebServer::DestoryHttpServer() {
   TRACE6("%s \n", __FUNCTION__);
   if (m_pHttpServer) {
      evhttp_free(m_pHttpServer);
      m_pHttpServer = NULL;
      TRACE6("%s evhttp_free ok\n", __FUNCTION__);
   }
   if (m_pHttpBase) {
      TRACE6("%s WaitForSingleObject m_pEvent\n", __FUNCTION__);
      WaitForSingleObject(m_pEvent, 30000);
      event_base_free(m_pHttpBase);
      m_pHttpBase = NULL;
   }
   TRACE6("%s DestoryHttpServer end\n", __FUNCTION__);
   
}

void CPluginWebServer::run() {
   RunHttpSeverRecvThread(this);
}

char * CPluginWebServer::HttpUrlEncode(const char* url, const int Len, bool bSpaceToPlus) {
   if (url == NULL) {
      return NULL;
   }
   return evhttp_uriencode(url, Len, bSpaceToPlus);
}


struct http_request_get *http_request_new(struct event_base* base, const char *url, int req_get_flag, const char *content_type, const char* data)
{
   struct http_request_get *http_req_get = new http_request_get;
   //http_req_get->req->output_headers = NULL;
   TRACE6("%s url: %s", __FUNCTION__,url);
   if (http_req_get != NULL) {
	   TRACE6("%s pre evhttp_uri_parse", __FUNCTION__);
      http_req_get->uri = evhttp_uri_parse(url);
	  TRACE6("%s evhttp_uri_parse later", __FUNCTION__);
      http_req_get->base = base;
	  TRACE6("%s end", __FUNCTION__);
   }
   return http_req_get;
}


void http_request_free(struct http_request_get *http_req_get, int req_get_flag)
{
   qDebug() << "http_request_free";
   evhttp_connection_free(http_req_get->cn);
   qDebug() << "evhttp_connection_free";
   evhttp_uri_free(http_req_get->uri);
   qDebug() << "evhttp_uri_free";
   //evhttp_request_free(http_req_get->req);
   qDebug() << "evhttp_request_free";
   if (http_req_get->requestData) {
      delete []http_req_get->requestData;
      qDebug() << "http_req_get->requestData";
   }
   if (req_get_flag == REQUEST_GET_FLAG) {
      free(http_req_get);
   }
   http_req_get = NULL;
}

int CPluginWebServer::start_url_request(struct http_request_get *http_req, int req_get_flag)
{
	TRACE6("%s start\n", __FUNCTION__);
   if (http_req->cn)
   { 
	   evhttp_connection_free(http_req->cn);
	   TRACE6("%s evhttp_connection_free \n", __FUNCTION__);
   }
      
   int port = evhttp_uri_get_port(http_req->uri);
   TRACE6("%s evhttp_uri_get_port \n", __FUNCTION__);
   http_req->cn = evhttp_connection_base_new(http_req->base, NULL,evhttp_uri_get_host(http_req->uri),(port == -1 ? 80 : port));
   TRACE6("%s evhttp_connection_base_new \n", __FUNCTION__);
   if (req_get_flag == REQUEST_GET_FLAG) {
      http_req->req = evhttp_request_new(http_requset_get_cb, http_req);
	  TRACE6("%s evhttp_request_new \n", __FUNCTION__);
   }

   if (req_get_flag == REQUEST_GET_FLAG) {
      const char *query = evhttp_uri_get_query(http_req->uri);
      const char *path = evhttp_uri_get_path(http_req->uri);
      size_t len = (query ? strlen(query) : 0) + (path ? strlen(path) : 0) + 1;
      char *path_query = NULL;
	  TRACE6("%s REQUEST_GET_FLAG \n", __FUNCTION__);
      if (len > 1) {
         path_query = new char[len + 1];// calloc(len, sizeof(char));
         sprintf(path_query, "%s?%s", path, query);
		 TRACE6("%s path_query %s \n", __FUNCTION__, path_query);
      }
      int nRet = evhttp_make_request(http_req->cn, http_req->req, EVHTTP_REQ_GET, path_query ? path_query : "/");
	  TRACE6("%s evhttp_make_request nRet %d\n", __FUNCTION__, nRet);
	  if (nRet != 0) {
          delete[]path_query;
		  TRACE6("%s  delete[]path_query   nRet\n", __FUNCTION__);
          return nRet;
      }
      delete[]path_query;
	  TRACE6("%s  delete[]path_query \n", __FUNCTION__);
   }
   /** Set the header properties */
   
   const char* szGetHost = evhttp_uri_get_host(http_req->uri);
   TRACE6("%s szGetHost %s \n", __FUNCTION__, szGetHost);
   if (NULL==http_req)
   {
	   TRACE6("%s NULL==http_req \n", __FUNCTION__);
   }
   if (NULL == http_req->req)
   {
	   TRACE6("%s NULL==http_req->req \n", __FUNCTION__);
   }
   if (NULL == http_req->req->output_headers)
   {
	   TRACE6("%s NULL== http_req->req->output_headers \n", __FUNCTION__);
   }
  
   TRACE6("%s pre evhttp_add_header \n", __FUNCTION__);
   int nRet = evhttp_add_header(http_req->req->output_headers, "Host", szGetHost);
   TRACE6("%s END \n", __FUNCTION__);
   return nRet;
}

char* CPluginWebServer::ClientHttpGet(const char* url,int &nOutLen,bool &bSuc) {
   char *outData = NULL;
   int req_get_flag = REQUEST_GET_FLAG;
   struct event_base* base = event_base_new();
   if (base == NULL) {
      TRACE6("%s event_base_new err\n", __FUNCTION__);
      return NULL;
   }
   TRACE6("%s event_base_new ok\n", __FUNCTION__);
   struct http_request_get *http_req = http_request_new(base, url, REQUEST_GET_FLAG, NULL, NULL);
   TRACE6("%s http_request_new ok\n", __FUNCTION__);
   if (http_req == NULL) {
        TRACE6("%s http_request_new err\n", __FUNCTION__);
        event_base_free(base);
      return NULL;
   }
   int nRet = start_url_request(http_req, REQUEST_GET_FLAG);
   if (nRet != 0) {
       TRACE6("%s start_url_request err\n", __FUNCTION__);
       event_base_free(base);
       return NULL;
   }
   TRACE6("%s start_url_request ok\n", __FUNCTION__);
   nRet = event_base_dispatch(base);
   if (nRet != 0) {
       TRACE6("%s event_base_dispatch err\n", __FUNCTION__);
       event_base_free(base);
       return NULL;
   }
   TRACE6("%s event_base_dispatch ok\n", __FUNCTION__);
   if (http_req) {
      bSuc = http_req->nCode != -1 ? true : false;
      if (http_req->requestData) {
         outData = new char[http_req->nDataLen + 1];
         if (outData) {
            memset(outData, 0, http_req->nDataLen + 1);
            memcpy(outData, http_req->requestData, http_req->nDataLen);
            nOutLen = http_req->nDataLen;
         }
         TRACE6("%s http_req ok\n", __FUNCTION__);
      }
   }
   http_request_free(http_req, REQUEST_GET_FLAG);
   event_base_free(base);
   TRACE6("%s outData ok\n", __FUNCTION__);
   return outData;
}

PLUGINWEBSERVER_EXPORT IPluginWebServer* GetHttpServerInstance() {
   return CPluginWebServer::GetInstance();
}


