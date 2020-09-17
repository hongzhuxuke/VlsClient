#ifndef _VHALLAUDIENCELOGIC_H_
#define _VHALLAUDIENCELOGIC_H_
#include <QObject>
#include "IVhallRightExtraWidgetLogic.h"
#include "vhallaudiencewidget.h"
#include <QTimer>
#include <QMutex>
#define MAX_ONLINE_LIST 50
//�����б�
class VhallAudienceLogic:public QObject  {
   Q_OBJECT

public:
   explicit VhallAudienceLogic(QObject *parent = NULL);
   ~VhallAudienceLogic(void);

public:
   BOOL Create();
   void Destroy();
   // ������Ϣ
   void DealMessage(DWORD adwMessageID, void* apData, DWORD adwLen);
   // ������Ϣ
   void RecvMsg(Event &);
   // ���������б�����
   void SyncOnlineListNumber();
   // ���������б�
   void SyncOnlineList();
   // ���½����б�
   void SyncGagList();
   // �����߳��б�
   void SyncKickList();
   
   // �Ƴ������û�
   void RemoveOnlineUser(VhallAudienceUserInfo &);
   // ��ӵ����û�
   void AppendOnlineUser(VhallAudienceUserInfo &);
   // �����û�״̬�ı�
   void ChangeModeOnlineUser(VhallAudienceUserInfo & );

   // �Ƴ������û�
   void RemoveGagUser(VhallAudienceUserInfo &);
   // ��ӵ����û�
   void AppendGagUser(VhallAudienceUserInfo &);
   // �����û�״̬�ı�
   void ChangeModeGagUser(VhallAudienceUserInfo &);

   // �Ƴ������û�
   void RemoveKickUser(VhallAudienceUserInfo &);
   // ��ӵ����û�
   void AppendKickUser(VhallAudienceUserInfo &);
   // �����û�״̬�ı�
   void ChangeModeKickUser(VhallAudienceUserInfo & );

   bool IsUserGag(wchar_t *userId);

   bool IsUserKick(wchar_t *userId);
   
   void Refresh();

   void SetIsHost(bool);

	void ReleaseSelf();
public slots:
   void Timeout();
   void SlotTabPressed();
private:
   VhallAudienceWidget m_audienceWidget;
   //��������б� �����ˡ��α�������
   VhallAudienceUserInfoList m_specialList;
   //�����б�
   VhallAudienceUserInfoList m_onlineList;
   //�����б�
   VhallAudienceUserInfoList m_gagList;
   //�߳��б�
   VhallAudienceUserInfoList m_kickList;

   QTimer m_timer;   
   int m_sumPageCount = 0;
   int m_currentPage = 0;
   int m_sumOnlineNumbers = 0;
   int m_timeClicked = 0;
   bool m_bActiveMessage = false;
};

#endif //_VHALLAUDIENCELOGIC_H_
