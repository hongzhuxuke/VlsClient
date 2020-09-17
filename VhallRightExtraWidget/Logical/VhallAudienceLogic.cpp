#include "VhallRightExtraWidgetIns.h"
#include "VhallAudienceLogic.h"
#include "Msg_VhallRightExtraWidget.h"
#include "IMainUILogic.h"
#include "DebugTrace.h"
#include <windows.h>
#include <QDebug>

VhallAudienceLogic::VhallAudienceLogic(QObject *parent) : QObject(parent) {
   connect(&m_timer,SIGNAL(timeout()),this,SLOT(Timeout()));
}
VhallAudienceLogic::~VhallAudienceLogic(void) {
   TRACE6("%s delete end\n", __FUNCTION__);
}
void VhallAudienceLogic::Timeout() {
   qDebug()<<"#############VhallAudienceLogic::Timeout()###########";
   if(m_timeClicked%10==0) {
      RQData oData;
      oData.m_eMsgType = e_RQ_UserOnlineList;
      //oData.m_iCurPage = m_currCount;
      SingletonVhallRightExtraWidgetIns::Instance().PostCRMessage(
            MSG_VHALLRIGHTEXTRAWIDGET_SENDMSG,&oData,sizeof(RQData));   
   }   

   m_timeClicked ++ ;
}
void VhallAudienceLogic::SlotTabPressed() {
   VH::CComPtr<IVhallRightExtraWidgetLogic> pVhallRightExtraWidget;
   DEF_GET_INTERFACE_PTR(SingletonVhallRightExtraWidgetIns, PID_IVhallRightExtraWidget, IID_IVhallRightExtraWidgetLogic, pVhallRightExtraWidget, return);
   if(pVhallRightExtraWidget->IsCurrentWidget(&m_audienceWidget)) {
      qDebug()<<"VhallAudienceLogic::SlotTabPressed";
      m_audienceWidget.TabPressed();
   }
}

BOOL VhallAudienceLogic::Create() {

   VH::CComPtr<IVhallRightExtraWidgetLogic> pVhallRightExtraWidget;
   DEF_GET_INTERFACE_PTR(SingletonVhallRightExtraWidgetIns, PID_IVhallRightExtraWidget, IID_IVhallRightExtraWidgetLogic, pVhallRightExtraWidget, return FALSE);
   pVhallRightExtraWidget->AppendModuleWidget(&m_audienceWidget,L"观众");
   //m_timer.start(1000);   
   return TRUE;
}
void VhallAudienceLogic::Destroy() {   
   m_timer.stop();   
   VH::CComPtr<IVhallRightExtraWidgetLogic> pVhallRightExtraWidget;
   DEF_GET_INTERFACE_PTR(SingletonVhallRightExtraWidgetIns, PID_IVhallRightExtraWidget, IID_IVhallRightExtraWidgetLogic, pVhallRightExtraWidget, return);
   pVhallRightExtraWidget->RemoveModuleWidget(&m_audienceWidget);
   TRACE6("%s delete Destroy end\n", __FUNCTION__);
}
// 处理消息
void VhallAudienceLogic::DealMessage(DWORD adwMessageID, void* apData, DWORD adwLen) {
   qDebug()<<"VhallAudienceLogic::DealMessage";
   if(MSG_VHALLRIGHTEXTRAWIDGET_GETNEWPAGEONLINELIST == adwMessageID) {
      DEF_CR_MESSAGE_DATA_DECLARE_CA(int, loMessage, return);
      RQData oData;
      oData.m_eMsgType = e_RQ_UserOnlineList;
      if(loMessage == NEXT_PAGE) {
         qDebug()<<"NEXT_PAGE";
         m_currentPage++;
         if(m_currentPage > m_sumPageCount) {
            m_currentPage = 1;
         }         
      }
      else if(loMessage == PREV_PAGE) {
         qDebug()<<"PREV_PAGE";
         m_currentPage--;
         if(m_currentPage < 1) {
            m_currentPage = m_sumPageCount;
         }
         if(m_currentPage <= 0) {
            m_currentPage = 1;
         }
      }
      oData.m_iCurPage = m_currentPage <= 0 ? 1 : m_currentPage;
      SingletonVhallRightExtraWidgetIns::Instance().PostCRMessage(
         MSG_VHALLRIGHTEXTRAWIDGET_SENDMSG,&oData,sizeof(RQData));  
      
      oData.m_eMsgType = e_RQ_UserProhibitSpeakList;
      SingletonVhallRightExtraWidgetIns::Instance().PostCRMessage(
         MSG_VHALLRIGHTEXTRAWIDGET_SENDMSG,&oData,sizeof(RQData));  

      oData.m_eMsgType = e_RQ_UserKickOutList;
      SingletonVhallRightExtraWidgetIns::Instance().PostCRMessage(
         MSG_VHALLRIGHTEXTRAWIDGET_SENDMSG,&oData,sizeof(RQData)); 

      oData.m_eMsgType = e_RQ_UserSpecialList;
      SingletonVhallRightExtraWidgetIns::Instance().PostCRMessage(
         MSG_VHALLRIGHTEXTRAWIDGET_SENDMSG,&oData,sizeof(RQData)); 

   
   }
}
void VhallAudienceLogic::SyncOnlineListNumber() {
   int sumNumber;
   if(m_bActiveMessage == false) {
      sumNumber = m_sumOnlineNumbers + m_specialList.count() + m_gagList.count();
   }   
   else {
      sumNumber = m_sumOnlineNumbers ;
   }
   
   //qDebug() << "######VhallAudienceLogic::SyncOnlineListNumber()###### "<<sumNumber;
   m_audienceWidget.SetOnlineNumber(sumNumber);
}

// 更新在线列表
void VhallAudienceLogic::SyncOnlineList() {
   m_audienceWidget.SyncOnlineList(m_specialList,m_onlineList,m_gagList);  
   SyncOnlineListNumber();
}
// 更新禁言列表
void VhallAudienceLogic::SyncGagList(){
   m_audienceWidget.SyncGagList(m_gagList);
   m_audienceWidget.SetGagNumber(m_gagList.count());
}
// 更新踢出列表
void VhallAudienceLogic::SyncKickList() {
   m_audienceWidget.SyncKickList(m_kickList);
   m_audienceWidget.SetKickNumber(m_kickList.count());
}
void VhallAudienceLogic::RemoveOnlineUser(VhallAudienceUserInfo &info) {
   if(info.role == USER_USER) {
      if(m_onlineList.Has(info.userId)) {
         m_onlineList.Remove(info);   
         //m_sumOnlineNumbers--;
      }
   }
   else {      
      m_specialList.Debug();
      m_specialList.Remove(info);
      m_specialList.Debug();
   }
   m_audienceWidget.RemoveOnlineList(info);
   SyncOnlineListNumber();
}
void VhallAudienceLogic::AppendOnlineUser(VhallAudienceUserInfo &info) {
   if(info.role == USER_USER) {
      if(!m_onlineList.pushBack(info)) {
         return ;
      } 
   }
   else {
      if(!m_specialList.append(info)) {
         return ;
      }      
   }
   m_audienceWidget.setSelfInfo(info);
   m_audienceWidget.AppendOnlineList(info);
   SyncOnlineListNumber();
}
//改变用户在在线列表中的状态
void VhallAudienceLogic::ChangeModeOnlineUser(VhallAudienceUserInfo &info) {
   m_audienceWidget.ChangeOnlineList(info);
}
// 移出单个用户
void VhallAudienceLogic::RemoveGagUser(VhallAudienceUserInfo &info){
   m_gagList.Remove(info);
   m_audienceWidget.RemoveGagList(info);
   m_audienceWidget.SetGagNumber(m_gagList.count());

   if (info.role==USER_USER) {
      if (m_onlineList.append(info)) {
         //m_sumOnlineNumbers++;
      }
   }
   else {
      m_specialList.append(info);
   }
}
// 添加单个用户
void VhallAudienceLogic::AppendGagUser(VhallAudienceUserInfo &info) {
   VhallShowType oldkickType = info.kickType;

   bool onlineRemove = false; 
   if(m_onlineList.Remove(info)) {
      m_audienceWidget.AppendOnlineList(info);
      //m_sumOnlineNumbers--;
      onlineRemove=true;
   }
   else if(m_specialList.Remove(info)){
      m_audienceWidget.AppendOnlineList(info);
   }
   else {
   }
   VhallShowType oldKickType = info.kickType;
   info.kickType = VhallShowType_Hide;
   if(m_gagList.pushBack(info)) {
      m_audienceWidget.AppendGagList(info);
      info.kickType = VhallShowType_Allow;

      int count = m_audienceWidget.Count();
      m_audienceWidget.AppendOnlineList(info);
      if (onlineRemove) {
         //m_sumOnlineNumbers += (m_audienceWidget.Count() - count);
      }

      info.kickType = oldkickType;
      m_audienceWidget.SetGagNumber(m_gagList.count());
   }
   info.kickType = oldKickType;
   SyncOnlineListNumber();
}
// 单个用户状态改变
void VhallAudienceLogic::ChangeModeGagUser(VhallAudienceUserInfo &info ) {
   m_audienceWidget.ChangeGagList(info);
}
// 移出单个用户
void VhallAudienceLogic::RemoveKickUser(VhallAudienceUserInfo &info){
   m_audienceWidget.RemovekickList(info);   
   m_kickList.Remove(info);
   m_audienceWidget.SetKickNumber(m_kickList.count());
}
// 添加单个用户
void VhallAudienceLogic::AppendKickUser(VhallAudienceUserInfo &info){
   if(!m_kickList.pushBack(info)) {
      return ;
   }
   m_audienceWidget.AppendkickList(info);
   m_audienceWidget.SetKickNumber(m_kickList.count());
}
// 单个用户状态改变
void VhallAudienceLogic::ChangeModeKickUser(VhallAudienceUserInfo &info ){
   m_audienceWidget.ChangekickList(info);
}
// 接收消息
void VhallAudienceLogic::RecvMsg(Event &e) {
   //在线列表
   if(e.m_eMsgType == e_RQ_UserOnlineList) {
      //总页数
      m_sumPageCount= e.m_iPageCount;
      if (m_sumPageCount<0) {
         m_sumPageCount=0;
      }
      //当前页
      m_currentPage = e.m_currentPage;
      if(m_currentPage < 0) {
         m_currentPage = 0;
      }
      
      //总人数
      if(m_bActiveMessage == false) {
         m_sumOnlineNumbers= e.m_sumNumber;
         if (m_sumOnlineNumbers<0) {
            m_sumOnlineNumbers=0;
         }
      }
      else if (m_sumOnlineNumbers<e.m_sumNumber) {
         m_sumOnlineNumbers=e.m_sumNumber;
      }

      //需要考虑分页机制
      m_onlineList.Init(e.m_oUserList,"user",VhallShowType_Allow,VhallShowType_Allow);
      SyncOnlineList();


      if(wcscmp(e.m_oUserInfo.m_szRole,L"user") != 0) {
         VH::CComPtr<IMainUILogic> pMainUILogic = NULL;
         DEF_GET_INTERFACE_PTR(SingletonVhallRightExtraWidgetIns, PID_IMainUI, IID_IMainUILogic, pMainUILogic, return);

         for(auto itor = e.m_oUserList.begin(); itor != e.m_oUserList.end(); itor ++) {
            if(wcscmp(e.m_oUserInfo.m_szRole,L"user") != 0) {
               pMainUILogic->OnlineEvent(true, itor->m_szUserID, itor->m_szRole, itor->m_szUserName, e_RQ_UserOnlineList);
            }
         }
      }
      
      TRACE6("#e_RQ_UserOnlineList# m_currentPage=%d,m_sumPageCount=%d,m_sumOnlineNumbers=%d\n",
         m_currentPage,m_sumPageCount,m_sumOnlineNumbers);
   }
   //禁言列表
   else if(e.m_eMsgType == e_RQ_UserProhibitSpeakList){
      m_gagList.Init(e.m_oUserList, "", VhallShowType_Prohibit, VhallShowType_Allow);
      SyncGagList();
      SyncOnlineList();
     // qDebug()<<"======================DEBUG e_RQ_UserProhibitSpeakList";
      m_specialList.Debug();
      //qDebug()<<"======================";
   }
   //踢出列表
   else if(e.m_eMsgType == e_RQ_UserKickOutList){
      qDebug()<<"VhallAudienceLogic::RecvMsg e.m_eMsgType == e_RQ_UserKickOutList";
      m_kickList.Init(e.m_oUserList, "", VhallShowType_Hide, VhallShowType_Prohibit);
      SyncKickList();
      //qDebug()<<"======================DEBUG e_RQ_UserKickOutList";
      m_specialList.Debug();
      // qDebug()<<"======================";

      VH::CComPtr<IMainUILogic> pMainUILogic = NULL;
      DEF_GET_INTERFACE_PTR(SingletonVhallRightExtraWidgetIns, PID_IMainUI, IID_IMainUILogic, pMainUILogic, return);
      if(pMainUILogic != NULL) {
         for(auto itor = e.m_oUserList.begin(); itor != e.m_oUserList.end(); itor ++) {
            if(wcscmp(e.m_oUserInfo.m_szRole,L"user") != 0) {
               pMainUILogic->KitoutEvent(true,itor->m_szUserID,itor->m_szRole);
            }
         }
      }
   }
   //特殊用户列表
   else if(e.m_eMsgType == e_RQ_UserSpecialList){
      m_specialList.Init(e.m_oUserList,"",VhallShowType_Allow,VhallShowType_Allow);
      SyncOnlineList();
    //  qDebug()<<"======================DEBUG e_RQ_UserSpecialList";
      m_specialList.Debug();
      //qDebug()<<"======================";
   }
   //单个用户禁言
   else if(e.m_eMsgType == e_RQ_UserProhibitSpeakOneUser) {      
      qDebug()<<"e_RQ_UserProhibitSpeakOneUser";
      VhallAudienceUserInfo info;
      if(m_onlineList.GetUserInfo(e.m_oUserInfo,info)){
         info.gagType=VhallShowType_Prohibit;
         AppendGagUser(info);
         ChangeModeOnlineUser(info);
         ChangeModeKickUser(info);
      }
      else if(m_gagList.GetUserInfo(e.m_oUserInfo,info)) {
         info.gagType=VhallShowType_Prohibit;
         AppendGagUser(info);
         info.kickType = VhallShowType_Allow;
         ChangeModeOnlineUser(info);
         ChangeModeKickUser(info);
      }
      else if (m_specialList.GetUserInfo(e.m_oUserInfo, info)) {
         
         info.gagType = VhallShowType_Prohibit;
         AppendGagUser(info);
         info.kickType = VhallShowType_Allow;
         ChangeModeOnlineUser(info);
         ChangeModeKickUser(info);
      }
      else {
         return ;
         info = VhallAudienceUserInfo(e.m_oUserInfo);
         info.gagType = VhallShowType_Prohibit;
         AppendGagUser(info);
         ChangeModeOnlineUser(info);
      }
   }
   //单个用户取消禁言
   else if(e.m_eMsgType == e_RQ_UserAllowSpeakOneUser) {
      qDebug()<<"e_RQ_UserAllowSpeakOneUser";
      VhallAudienceUserInfo info;
      if (m_gagList.GetUserInfo(e.m_oUserInfo,info)) {
         info.gagType = VhallShowType_Allow;
         info.kickType=VhallShowType_Allow;
         RemoveGagUser(info);
         ChangeModeOnlineUser(info );
         ChangeModeKickUser(info);
      }
      else {
         return ;
         info = VhallAudienceUserInfo(e.m_oUserInfo);
         info.gagType = VhallShowType_Allow;
         info.kickType = VhallShowType_Allow;
         RemoveGagUser(info);
         ChangeModeOnlineUser(info);
         ChangeModeKickUser(info);
      }


   }
   //踢出单个用户
   else if(e.m_eMsgType == e_RQ_UserKickOutOneUser) {
      qDebug()<<"e_RQ_UserKickOutOneUser";
      VhallAudienceUserInfo info;
      if (m_onlineList.GetUserInfo(e.m_oUserInfo,info)) {
         info.kickType=VhallShowType_Prohibit;
         RemoveOnlineUser(info);
         AppendKickUser(info);
      }     
      else if (m_specialList.GetUserInfo(e.m_oUserInfo, info)) {
         info.kickType = VhallShowType_Prohibit;
         RemoveOnlineUser(info);
         AppendKickUser(info);
      }
      else if (m_gagList.GetUserInfo(e.m_oUserInfo, info)) {
         info.kickType = VhallShowType_Prohibit;
         RemoveGagUser(info);
         RemoveOnlineUser(info);
         AppendKickUser(info);
         
      }

      if(wcscmp(e.m_oUserInfo.m_szRole,L"user") != 0) {
         qDebug()<<"VhallAudienceLogic::RecvMsg e.m_eMsgType == e_RQ_UserKickOutOneUser";
         VH::CComPtr<IMainUILogic> pMainUILogic = NULL;
         DEF_GET_INTERFACE_PTR(SingletonVhallRightExtraWidgetIns, PID_IMainUI, IID_IMainUILogic, pMainUILogic, return);
         if(pMainUILogic != NULL) {
            pMainUILogic->KitoutEvent(true,e.m_oUserInfo.m_szUserID,e.m_oUserInfo.m_szRole);
         }
      }      

      
      SyncOnlineListNumber();
   }
   //取消踢出单个用户
   else if(e.m_eMsgType == e_RQ_UserAllowJoinOneUser) {
      qDebug()<<"e_RQ_UserAllowJoinOneUser";
      VhallAudienceUserInfo info;
      if (m_kickList.GetUserInfo(e.m_oUserInfo,info)) {         
         info.kickType=VhallShowType_Allow;
         RemoveKickUser(info);
      }   

      if(wcscmp(e.m_oUserInfo.m_szRole,L"user") != 0) {
         VH::CComPtr<IMainUILogic> pMainUILogic = NULL;
         DEF_GET_INTERFACE_PTR(SingletonVhallRightExtraWidgetIns, PID_IMainUI, IID_IMainUILogic, pMainUILogic, return);
         if(pMainUILogic != NULL) {
            pMainUILogic->KitoutEvent(false,e.m_oUserInfo.m_szUserID,e.m_oUserInfo.m_szRole);
         }
      }
   }
   //用户上线
   else if(e.m_eMsgType == e_Ntf_UserOnline) {   
      TRACE6("%s e.m_eMsgType == e_Ntf_UserOnline\n",__FUNCTION__);
      if(wcscmp(e.m_oUserInfo.m_szRole,L"user") != 0) {
         VH::CComPtr<IMainUILogic> pMainUILogic = NULL;
         DEF_GET_INTERFACE_PTR(SingletonVhallRightExtraWidgetIns, PID_IMainUI, IID_IMainUILogic, pMainUILogic);
         if(pMainUILogic != NULL) {
            TRACE6("%s OnlineEvent id:%ws role:%ws name:%ws\n", __FUNCTION__,
				e.m_oUserInfo.m_szUserID, e.m_oUserInfo.m_szRole, e.m_oUserInfo.m_szUserName);
            pMainUILogic->OnlineEvent(true, e.m_oUserInfo.m_szUserID, e.m_oUserInfo.m_szRole, e.m_oUserInfo.m_szUserName, e_Ntf_UserOnline);
         }
      }

      m_bActiveMessage = true;
      m_sumOnlineNumbers= e.m_sumNumber;
      SyncOnlineListNumber();

      static int index=0;
      if(index%30==0) {
         TRACE6("#e_Ntf_UserOnline# m_sumOnlineNumbers=%d\n",m_sumOnlineNumbers);
         index=0;
      }
      index++;

      VhallAudienceUserInfo info=VhallAudienceUserInfo(e.m_oUserInfo);
      if (info.role != USER_HOST) {
         info.gagType= e.m_isGag?VhallShowType_Prohibit:VhallShowType_Allow;
         info.kickType = VhallShowType_Allow;
      }
      else {
         info.kickType = VhallShowType_Hide;
         info.gagType= VhallShowType_Hide;
      }
      
      VhallAudienceUserInfo tinfo;
      m_specialList.Debug();
      if(m_specialList.GetUserInfo(e.m_oUserInfo,tinfo)) {
         return ;
      }
      
      m_onlineList.Debug();
      
      if(m_onlineList.GetUserInfo(e.m_oUserInfo,tinfo)) {         
         return ;
      }
      
      if(m_onlineList.count() >= 50) {    
         qDebug()<<"m_onlineList.count()>50 "<<m_onlineList.count();
         //m_sumOnlineNumbers++;
         return ;
      }
      
      if(e.m_isGag) {
         AppendGagUser(info);
      }
      else {
         AppendOnlineUser(info);
      }

      RemoveKickUser(info);
   }
   //用户下线
   else if(e.m_eMsgType == e_Ntf_UserOffline) {
      if(wcscmp(e.m_oUserInfo.m_szRole,L"user") != 0) {
         VH::CComPtr<IMainUILogic> pMainUILogic = NULL;
         DEF_GET_INTERFACE_PTR(SingletonVhallRightExtraWidgetIns, PID_IMainUI, IID_IMainUILogic, pMainUILogic);
         if(pMainUILogic != NULL) {
            pMainUILogic->OnlineEvent(false, e.m_oUserInfo.m_szUserID, e.m_oUserInfo.m_szRole, e.m_oUserInfo.m_szUserName, e_Ntf_UserOffline);
         }
      }

      m_bActiveMessage = true;

      m_sumOnlineNumbers= e.m_sumNumber;
      SyncOnlineListNumber();
      static int index=0;
      if(index%30==0) {
         TRACE6("#e_Ntf_UserOffline# m_sumOnlineNumbers=%d\n",m_sumOnlineNumbers);
         index=0;
      }
      index++;
      
      VhallAudienceUserInfo info=VhallAudienceUserInfo(e.m_oUserInfo);
      RemoveGagUser(info);
      RemoveOnlineUser(info);
   }
   //全体禁言
   else if(e.m_eMsgType == e_RQ_UserProhibitSpeakAllUser) {
      VH::CComPtr<IMainUILogic> pMainUILogic;
      DEF_GET_INTERFACE_PTR(SingletonVhallRightExtraWidgetIns, PID_IMainUI, IID_IMainUILogic, pMainUILogic, return);
      pMainUILogic->FadoutTip(L"已开启全体禁言");      
   } 
   //取消全体禁言
   else if(e.m_eMsgType == e_RQ_UserAllowSpeakAllUser) {
      VH::CComPtr<IMainUILogic> pMainUILogic;
      DEF_GET_INTERFACE_PTR(SingletonVhallRightExtraWidgetIns, PID_IMainUI, IID_IMainUILogic, pMainUILogic, return);
      pMainUILogic->FadoutTip(L"已关闭全体禁言");  
   }
}
bool VhallAudienceLogic::IsUserGag(wchar_t *userId) {
   return m_gagList.Has(userId);
}

bool VhallAudienceLogic::IsUserKick(wchar_t *userId){
   return m_kickList.Has(userId);
}

void VhallAudienceLogic::Refresh() {
   
   qDebug()<<"VhallAudienceLogic::Refresh()    1";
   m_sumPageCount = 0;
   m_currentPage = 0;
   m_sumOnlineNumbers = 0;
   m_timeClicked = 0;
   m_bActiveMessage = false;
   
   qDebug()<<"VhallAudienceLogic::Refresh()    2";
   m_audienceWidget.Clear();
   m_specialList.clear();
   m_onlineList.clear();
   m_gagList.clear();
   m_kickList.clear();
   qDebug()<<"VhallAudienceLogic::Refresh()    3";

   SyncOnlineList();
   SyncGagList();
   SyncKickList();
   
   qDebug()<<"VhallAudienceLogic::Refresh()    4";
}
void VhallAudienceLogic::SetIsHost(bool ok) {
   m_audienceWidget.SetIsHost(ok);
}

void VhallAudienceLogic::ReleaseSelf(){
	m_audienceWidget.ReleaseSelf();
}
