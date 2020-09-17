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

//�����ͻ��˽ӿ�
class InteractionAliveClient : public IInteractionClient{
public:

   InteractionAliveClient();
   virtual ~InteractionAliveClient();
   
   //��ʼ��
   BOOL Init(Delegate* pDelegate, Key_Data& oKeyData);

   virtual void RegisterDelegate(Delegate* pDelegate);
   //����ʼ��
   void UnInit();
   //���ӷ�����()
   void ConnectServer();
   //�Ͽ�����
   void DisConnect();
   //��������
   void MessageRQ(const MsgRQType& eMsgType, RQData* vData = NULL);
   //socketIO �ص�
   void SetMessageSocketIOCallBack(InteractionMsgCallBack);
   //websocket �ص�
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
   CCriticalSection		m_oHttpTaskLock;	         //�������������
   HANDLE m_hHttpEvent;
   HttpReqList m_oHttpList;

   std::atomic<bool> m_bHttpLoopRun = true;
   std::atomic<bool> m_bKeepLiveHeartBeatRun = true;

   //����������֤�͵�����ͬ��
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