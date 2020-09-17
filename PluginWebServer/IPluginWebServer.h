#pragma once
#include <QtCore/qglobal.h>

#ifdef PLUGINWEBSERVER_LIB
# define PLUGINWEBSERVER_EXPORT Q_DECL_EXPORT
#else
# define PLUGINWEBSERVER_EXPORT Q_DECL_IMPORT
#endif

typedef void(*HttpRequestData)(const char *data, int dataLen);

// (default)    
#define HTTP_CONTENT_TYPE_URL_ENCODED   "application/x-www-form-urlencoded"       
// (use for files: picture, mp3, tar-file etc.)                                            
#define HTTP_CONTENT_TYPE_FORM_DATA     "multipart/form-data"                     
// (use for plain text)    
#define HTTP_CONTENT_TYPE_TEXT_PLAIN    "text/plain"    

#define REQUEST_POST_FLAG               2    
#define REQUEST_GET_FLAG                3  

enum WEB_SERVER_ERRCODE {
   ERR_NONO = 0,
   ERR_INIT = 1,
   ERR_NEW_EVENTBASE = 2,
   ERR_NEW_HTTPSERVER = 3,
   ERR_BIND_SOCKET = 4,
   ERR_DISPATCH = 5,
   ERR_THREAD_RUN = 6,
};

class PLUGINWEBSERVER_EXPORT IPluginWebServer {
public:
   virtual ~IPluginWebServer() {};
   /*
   *  启动HttpServer服务
   *  返回值：0 成功，其它失败
   **/
   virtual int StartHttpServer(const char* ip, const int port) = 0;
   /*
   *  停止HttpServer服务,等待接收线程退出之后返回。
   *  返回值：0 成功，其它失败
   **/
   virtual int StopHttpServer() = 0;
   /*
   *  注册接收HTTP数据回调
   **/
   virtual void RegisterHttpDateCallback(HttpRequestData) = 0;
   /*
   *  对HttpUrl特殊字符进行转义  
   *  参数：bSpaceToPlus  true ：url中空格转成“+”
                          false：url中空格转成%20.
   **/
   virtual char *HttpUrlEncode(const char* url,const int Len,bool bSpaceToPlus) = 0;

   virtual char* ClientHttpGet(const char* url, int &nOutLen, bool &bSuc) = 0;
};

PLUGINWEBSERVER_EXPORT IPluginWebServer* GetHttpServerInstance();
