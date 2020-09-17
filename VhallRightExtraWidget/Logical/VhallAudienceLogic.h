#ifndef _VHALLAUDIENCELOGIC_H_
#define _VHALLAUDIENCELOGIC_H_
#include <QObject>
#include "IVhallRightExtraWidgetLogic.h"
#include "vhallaudiencewidget.h"
#include <QTimer>
#include <QMutex>
#define MAX_ONLINE_LIST 50
//观众列表
class VhallAudienceLogic:public QObject  {
   Q_OBJECT

public:
   explicit VhallAudienceLogic(QObject *parent = NULL);
   ~VhallAudienceLogic(void);

public:
   BOOL Create();
   void Destroy();
   // 处理消息
   void DealMessage(DWORD adwMessageID, void* apData, DWORD adwLen);
   // 接收消息
   void RecvMsg(Event &);
   // 更新在线列表人数
   void SyncOnlineListNumber();
   // 更新在线列表
   void SyncOnlineList();
   // 更新禁言列表
   void SyncGagList();
   // 更新踢出列表
   void SyncKickList();
   
   // 移出单个用户
   void RemoveOnlineUser(VhallAudienceUserInfo &);
   // 添加单个用户
   void AppendOnlineUser(VhallAudienceUserInfo &);
   // 单个用户状态改变
   void ChangeModeOnlineUser(VhallAudienceUserInfo & );

   // 移出单个用户
   void RemoveGagUser(VhallAudienceUserInfo &);
   // 添加单个用户
   void AppendGagUser(VhallAudienceUserInfo &);
   // 单个用户状态改变
   void ChangeModeGagUser(VhallAudienceUserInfo &);

   // 移出单个用户
   void RemoveKickUser(VhallAudienceUserInfo &);
   // 添加单个用户
   void AppendKickUser(VhallAudienceUserInfo &);
   // 单个用户状态改变
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
   //特殊身份列表 主持人、嘉宾、助理
   VhallAudienceUserInfoList m_specialList;
   //在线列表
   VhallAudienceUserInfoList m_onlineList;
   //禁言列表
   VhallAudienceUserInfoList m_gagList;
   //踢出列表
   VhallAudienceUserInfoList m_kickList;

   QTimer m_timer;   
   int m_sumPageCount = 0;
   int m_currentPage = 0;
   int m_sumOnlineNumbers = 0;
   int m_timeClicked = 0;
   bool m_bActiveMessage = false;
};

#endif //_VHALLAUDIENCELOGIC_H_
