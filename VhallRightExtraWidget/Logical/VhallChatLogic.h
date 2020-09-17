#ifndef _VHALLCHATLOGIC_H_
#define _VHALLCHATLOGIC_H_
#include <QObject>
#include "IVhallRightExtraWidgetLogic.h"
#include "vhallchatwidget.h"
#include "IInteractionClient.h"
//����
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
   // ��ʼ��������Ϣ
   void InitUserInfo(QString userName,QString userImageUrl,QString userId);
   // ������Ϣ
   void DealMessage(DWORD adwMessageID, void* apData, DWORD adwLen);
   // ������Ϣ
   void RecvMsg(Event &);
   // ������Ϣ
   //void SendMsg(MsgRQType eMsgType, RQData vData);
   //����roomid
   void setRoomId(QString,char *,bool);
   //�л�ʱ�������ڲ���������
   void hideRightMouseMenuFunc();

   void SetReloadChatHtml(bool bReload);
   bool IsLoadUrlFinished();
private:
   VhallChatWidget m_chatWidget;
   QString m_userId;

};

#endif //_VHALLCHATLOGIC_H_
