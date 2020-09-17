#ifndef VHINTERACTION_H
#define VHINTERACTION_H

#include <QObject>
#include <QMutex>
#include <windows.h>
#include "IInteractionClient.h"
#include <QMap>
#include <QDebug>
#include <QJsonObject>
#include <QThread>
//通知事件
class VHInteraction :public QObject, public Delegate 
{
    Q_OBJECT
public:
    explicit VHInteraction(QObject *parent = 0);
    
    virtual ~VHInteraction();

    BOOL Create();
    
    void Destory();

    BOOL Init(Key_Data &);

    void UnInit();
    
    virtual void onConnected(Event* event);
    
    virtual void onMessaged(Event* event);
    
    //virtual void onDisconnected(Event* event);
    
    virtual void onError(Event* event);
    
    virtual void MessageRQ(MsgRQType eMsgType, RQData* vData = NULL);
    //socketIO 回调
    virtual void SetMessageSocketIOCallBack(InteractionMsgCallBack);
    
    //websocket 回调
    virtual void SetMessageWebSocketCallBack(InteractionMsgCallBack);

    void SetHttpProxy(bool enable, const char *proxy_ip, const char* proxy_port, const char *proxy_username, const char *proxy_password);

signals:
    void SigMsg(QJsonObject);
    void SigInit(QByteArray);
    void SigOnConnected();
    void SigUnInit();
    void SigUnInitFinished();

private:
    void InitSocketThread(QByteArray);
    void UnInitSocketThread();

private:
   IInteractionClient *m_clinet = NULL;
   bool m_bInit = false;
   QMutex m_mutex;
   InteractionMsgCallBack m_socketIOCallBack;
   InteractionMsgCallBack m_websocketCallBack;
   long m_roomId ;
};

bool AnalysisEventObject(QJsonObject &obj,Event &e);

#endif // VHINTERACTION_H
