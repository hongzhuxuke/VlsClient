#ifndef _VHALLNOTICEINFOLOGIC_H_
#define _VHALLNOTICEINFOLOGIC_H_
#include <QObject>
#include "IVhallRightExtraWidgetLogic.h"
#include "vhallnoticeinfowidget.h"
#include "IInteractionClient.h"

//聊天
class VhallNoticeInfoLogic  : public QObject{
   Q_OBJECT
      
public:
   explicit VhallNoticeInfoLogic(QObject *parent = 0);
   ~VhallNoticeInfoLogic(void);
   void RecvAnnouncement(QString,QString);
public:
   BOOL Create();
   void Destroy();
   // 处理消息
   void DealMessage(DWORD adwMessageID, void* apData, DWORD adwLen);
   // 接收消息
   void RecvMsg(Event &);
   // 发送消息
   void SendMsg(MsgRQType eMsgType, RQData vData);

   void Refresh(bool bInit = false);

   void ShowNoticeWnd();

   bool IsLoadUrlFinished();
signals:
   void SigSendMsg(MsgRQType eMsgType, RQData vData);
private:
   VhallNoticeInfoWidget m_noticeInfoWidget;
};

#endif //_VHALLNOTICEINFOLOGIC_H_
