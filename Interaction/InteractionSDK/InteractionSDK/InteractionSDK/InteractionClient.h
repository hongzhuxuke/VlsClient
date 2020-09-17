#ifndef H_INTERACTIONCLIENT_H
#define H_INTERACTIONCLIENT_H

#include "CriticalSection.h"
//#include "sio_client.h"
//#include "../lib/libwebsockets.h"

#include <atomic>
#include <time.h>
#include <mutex>
#include <QEventLoop>
#include <QNetworkAccessManager>
#include <QTimer>
#include <QNetworkReply>  
#include "vhHttpProxy.h"
#include "pri.Inter.const.h"



class QString;
class InteractionClient;
class QNetworkAccessManager;

//互动客户端接口
class InteractionClient : public IInteractionClient{
public:

   InteractionClient();
   virtual ~InteractionClient();
   
   //初始化
   BOOL Init(Delegate* pDelegate, Key_Data& oKeyData);
   //反初始化
   void UnInit();
   //连接服务器()
   void ConnectServer();
   //断开链接
   void DisConnect();
   //发送请求
   void MessageRQ(const MsgRQType& eMsgType, RQData* vData = NULL);
   //socketIO 回调
   void SetMessageSocketIOCallBack(InteractionMsgCallBack);
   //websocket 回调
   void SetMessageWebSocketCallBack(InteractionMsgCallBack);


   virtual void SetProxyAddr(const bool&  enable, const char *proxy_ip, const char* proxy_port, const char *proxy_username, const char *proxy_password);
private:
   BOOL InitHttp();
   void UnInitHttp();

   BOOL InitSocketIO();
   void UnInitSocketIO();

   BOOL InitWebSocket();
   void UnInitWebSocket();
   
   BOOL InitKeepLiveThread();
   void UnInitKeepLiveThread();

   //Http
   static unsigned int HttpReqThread(void * apParam);
   unsigned int HttpReqWork();
   void AddHttpTask(HttpReq& aoReq);
   BOOL GetHttpTask(HttpReq& aoReq);
   BOOL ExecuteHttpTask(HttpReq& oTask);
   void ParseJSon(MsgRQType type, QString qsJson, Event& oEvent);

   //Socket.io
   static unsigned int SocketIOReqThread(void * apParam);
   unsigned int SocketIOHttpReqWork();

   //WebSocket
   static unsigned int WebSocketThread(void * apParam);
   unsigned int WebSocketLoopWork();

   void OnHttpMsg(QString cmd, QString value);
   void ParseWebsocketData(char* data);
   BOOL ExecuteCallBackTask(Event& oTask);
   
   static unsigned int KeepLiveThread(void *Param);
   unsigned int KeepLiveThreadWork();

public:
   void PubSocketIOSendMsg(QString &);
   void PubWebSocketSendMsg(QString &);


private:
   //Http
   CCriticalSection		m_oHttpTaskLock;	         //任务请求队列锁
   HANDLE m_hHttpEvent;
   HttpReqList m_oHttpList;

   std::atomic<bool> m_bHttpLoopRun = true;
   std::atomic<bool> m_bKeepLiveHeartBeatRun = true;
   //保护锁，保证和调用者同步
   CCriticalSection		m_oSafeLock;	         
   Delegate* m_pDelegate;
   Key_Data m_oKeyData;
   CCriticalSection		m_oHttpProxyInfoLock;
   HttpProxyInfo        m_httpProxyInfo;

   std::mutex m_socketIOMutex;
   std::mutex m_websocketMutex;
   InteractionMsgCallBack m_socketIOCallback;
   InteractionMsgCallBack m_websocketCallback;

   InternalHttpRequest m_httpSocketIOProxy;
   InternalHttpRequest m_httpWebSocketProxy;
   InternalHttpRequest m_httpTaskProxy;
   InternalHttpRequest m_httpKeepLive;
   QString mURL;
   QString mToken;
   int mIndex;
   QString mCookio;
   QMutex mKeepLiveMutex;

};

#endif // H_INTERACTIONALIVECLIENT_H