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
   *  ����HttpServer����
   *  ����ֵ��0 �ɹ�������ʧ��
   **/
   virtual int StartHttpServer(const char* ip, const int port) = 0;
   /*
   *  ֹͣHttpServer����,�ȴ������߳��˳�֮�󷵻ء�
   *  ����ֵ��0 �ɹ�������ʧ��
   **/
   virtual int StopHttpServer() = 0;
   /*
   *  ע�����HTTP���ݻص�
   **/
   virtual void RegisterHttpDateCallback(HttpRequestData) = 0;
   /*
   *  ��HttpUrl�����ַ�����ת��  
   *  ������bSpaceToPlus  true ��url�пո�ת�ɡ�+��
                          false��url�пո�ת��%20.
   **/
   virtual char *HttpUrlEncode(const char* url,const int Len,bool bSpaceToPlus) = 0;

   virtual char* ClientHttpGet(const char* url, int &nOutLen, bool &bSuc) = 0;
};

PLUGINWEBSERVER_EXPORT IPluginWebServer* GetHttpServerInstance();
