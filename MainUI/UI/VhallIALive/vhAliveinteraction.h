#ifndef H_VHALIVEINTERACTION_H
#define H_VHALIVEINTERACTION_H

#include <QObject>
#include <QMutex>
#include <windows.h>
#include "IInteractionClient.h"
#include <QMap>
#include <QDebug>
#include <QJsonObject>
#include <QThread>
#include <atomic>
#include "VHWebSocketInterface.h"
//通知事件
class VHAliveInteraction :public QObject, public Delegate, public WebSocketCallBackInterface
{
    Q_OBJECT
public:
    //WebSocketCallBackInterface  socketio callback
    virtual void OnOpen();
    virtual void OnFail();
    virtual void OnReconnecting();
    virtual void OnReconnect(unsigned, unsigned);
    virtual void OnClose();
    virtual void OnSocketOpen(std::string const& nsp);
    virtual void OnSocketClose(std::string const& nsp);
    virtual void OnRecvChatCtrlMsg(const char* msgType, const char* msg);
    virtual void OnRecvAllMsg(const char*, int length) {};
    //Delegate http long polling call back
    virtual void onConnected(Event* event);
    virtual void onMessaged(Event* event);
    virtual void onError(Event* event);

    explicit VHAliveInteraction(bool enableProxy = false, QObject *parent = 0);
    virtual ~VHAliveInteraction();

    void SetEventReciver(QObject *obj);

    BOOL Create();
    void Destory();
    BOOL Init(Key_Data &);

    virtual void MessageRQ(MsgRQType eMsgType, RQData* vData = NULL);
    //socketIO 回调
    virtual void SetMessageSocketIOCallBack(InteractionMsgCallBack);
    void SetHttpProxy(bool enable, const char *proxy_ip, const char* proxy_port, const char *proxy_username, const char *proxy_password);
    bool isInit() {return m_bInit;}

    static QJsonObject MakeEventObject(Event *e);
    static void ParseJSon(const MsgRQType& type, const QString& qsJson, Event& oEvent);
signals:
    void SigMsg(QJsonObject);
    void SigInit(QByteArray);
    void SigUnInit();
    void SigUnInitFinished();

private:
    void InitSocketThread(QByteArray);
    void UnInitSocketThread();


private:
   IInteractionClient *m_clinet = NULL;
   std::atomic_bool m_bInit = false;
   QMutex m_mutex;
   InteractionMsgCallBack m_socketIOCallBack;
   Key_Data mKeyData;
   bool mEnableProxy;
   QObject* mEventReciver = nullptr;
   VHWebSocketInterface* mpVHWebSocketInterface = nullptr;     //socketIo/WebSocket库
   IInteractionClient* mWebSocketMsgParamClient = nullptr;   //只用OnHttp接口解析cmd/flash消息
};

bool AnalysisEventObject(QJsonObject &obj,Event &e);

#endif // H_VHALIVEINTERACTION_H
