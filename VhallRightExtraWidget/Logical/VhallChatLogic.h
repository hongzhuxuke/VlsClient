#ifndef _VHALLCHATLOGIC_H_
#define _VHALLCHATLOGIC_H_
#include <QObject>
#include "IVhallRightExtraWidgetLogic.h"
#include "vhallchatwidget.h"
#include "IInteractionClient.h"
//聊天
class VhallChatLogic : public QObject  {
   Q_OBJECT
public:
   explicit VhallChatLogic(QObject *parent = NULL);
   ~VhallChatLogic(void);
   void Refresh();
   void SetIsHost(bool);
public:
   BOOL Create();
   void Destroy();
   // 初始化个人信息
   void InitUserInfo(QString userName,QString userImageUrl,QString userId);
   // 处理消息
   void DealMessage(DWORD adwMessageID, void* apData, DWORD adwLen);
   // 接收消息
   void RecvMsg(Event &);
   // 发送消息
   //void SendMsg(MsgRQType eMsgType, RQData vData);
   //设置roomid
   void setRoomId(QString,char *,bool);
   //切换时，隐藏内部部分内容
   void hideRightMouseMenuFunc();

   void SetReloadChatHtml(bool bReload);
   bool IsLoadUrlFinished();
private:
   VhallChatWidget m_chatWidget;
   QString m_userId;

};

#endif //_VHALLCHATLOGIC_H_
