#ifndef H_INTERACTIONALIVECLIENT_H
#define H_INTERACTIONALIVECLIENT_H
			
#include "CriticalSection.h"
#include <atomic>
#include <time.h>
#include <mutex>
#include <QEventLoop>
#include <QNetworkAccessManager>
#include <QTimer>
#include <QNetworkReply>  
#include "vhHttpProxy.h"
#include "pri.Inter.const.h"
#include <QTextStream>
#include <QDateTime>
#include <QFile>

class InteractionAliveClient;
class QNetworkAccessManager;

//#include <fstream>
//#include <time.h>
//
//using namespace std;
//void add_log(const char *logfile, const char *fmt, ...);

//互动客户端接口
class InteractionAliveClient : public IInteractionClient{
public:

   InteractionAliveClient();
   virtual ~InteractionAliveClient();
   
   //初始化
   BOOL Init(Delegate* pDelegate, Key_Data& oKeyData);

   virtual void RegisterDelegate(Delegate* pDelegate);
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
   virtual void SetProxyAddr(const bool& enable, const char *proxy_ip, const char* proxy_port, const char *proxy_username, const char *proxy_password);
   virtual void OnHttpMsg(const QString& cmd, const QString& value);
public:
   void PubSocketIOSendMsg(QString &);

private:
	BOOL InitHttp();
	void UnInitHttp();

	BOOL InitSocketIO();
	void UnInitSocketIO();

   BOOL InitKeepLiveThread();
   void UnInitKeepLiveThread();

	//Http
	static unsigned int HttpReqThread(void * apParam);
	unsigned int HttpReqWork();
	void AddHttpTask(HttpReq& aoReq);
	BOOL GetHttpTask(HttpReq& aoReq);
	BOOL ExecuteHttpTask(HttpReq& oTask);
	void ParseJSon(const MsgRQType& type, const QString& qsJson, Event& oEvent);

	//Socket.io
	static unsigned int SocketIOReqThread(void * apParam);
	unsigned int SocketIOHttpReqWork();

   static unsigned int KeepLiveThread(void *Param);
   unsigned int KeepLiveThreadWork();


	BOOL ExecuteCallBackTask(Event& oTask);
	void CreateAnnouncement(Event& oEvent, const QJsonObject& obj, QString& msgSocketIO);
	void CreateKickout(Event& oEvent, const QJsonObject& obj);
	void CreatKickoutrestore(Event& oEvent, const QJsonObject& obj);
	void CreatDisablechat(Event& oEvent, const QJsonObject& obj);
	void CreatPermitchat(Event& oEvent, const QJsonObject& obj);
	void CreatForbidchat(Event& oEvent, const QJsonObject& obj,const QString& strValue);
	void CreatWhiteBoard(/*Event& oEvent, const QJsonObject& obj, */const QString& strValue);
	void CreatSwitchHandsup(Event& oEvent, const QJsonObject& obj);
	void CreatHandsUp(Event& oEvent, const QJsonObject& obj);
    void CreatSetDefinition(Event& oEvent, const QJsonObject& obj);
	void CreateReplyInvite(Event& oEvent, const QJsonObject& obj);
	void CreateSwitchDevice(Event& oEvent, const QJsonObject& obj);
	void CreateNotSpeak(Event& oEvent, const QJsonObject& obj);
	void CreateSendInvite(Event& oEvent, const QJsonObject& obj);
	void CreateAddSpeaker(Event& oEvent, const QJsonObject& obj);
    void CreateAgreeSpeaker(Event& oEvent, const QJsonObject& obj);
	void CreateSetMainShow(Event& oEvent, const QJsonObject& obj);
	void CreateSetMainSpeaker(Event& oEvent, const QJsonObject& obj);
	void CreatePublishStart(Event& oEvent, const QJsonObject& obj);
	void CreateOver(Event& oEvent, const QJsonObject& obj);
	//void CreateAddSpeak(Event& oEvent, const QJsonObject& obj);
	void CreateRejectSpeak(Event& oEvent, const QJsonObject& obj);
	//void CreateWhiteBoard(Event& oEvent, const QJsonObject& obj);
	void analysisUseInfo(Event& oEvent, const QJsonObject& obj);
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
   InteractionMsgCallBack m_socketIOCallback;

   InternalHttpRequest m_httpSocketIOProxy;
   InternalHttpRequest m_httpKeepLive;
   InternalHttpRequest m_httpTaskProxy;
   InternalHttpRequest quitRequest;
   QString mURL;
   QString mToken;
   std::atomic_int mIndex;
   QString mCookio;
   QMutex mKeepLiveMutex;
};
#endif // H_INTERACTIONALIVECLIENT_H