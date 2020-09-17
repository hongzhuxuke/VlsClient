#include "Msg_VhallRightExtraWidget.h"
#include "VhallRightExtraWidgetIns.h"
#include "VhallChatLogic.h"
#include "DebugTrace.h"
#include <windows.h>
#include <QDebug>
VhallChatLogic::VhallChatLogic(QObject *parent) : QObject(parent) { 
   
}
VhallChatLogic::~VhallChatLogic(void) {
   TRACE6("%s delete end\n", __FUNCTION__);
}
void VhallChatLogic::Refresh() {
   this->m_chatWidget.Refresh();
}
void VhallChatLogic::SetIsHost(bool ok) {
   this->m_chatWidget.SetIsHost(ok);
}

BOOL VhallChatLogic::Create() {
   VH::CComPtr<IVhallRightExtraWidgetLogic> pVhallRightExtraWidget;
   m_chatWidget.showMinimized();
   DEF_GET_INTERFACE_PTR(SingletonVhallRightExtraWidgetIns, PID_IVhallRightExtraWidget, IID_IVhallRightExtraWidgetLogic, pVhallRightExtraWidget, return FALSE);
   pVhallRightExtraWidget->AppendModuleWidget(&m_chatWidget,L"聊天");
   return TRUE;
}
void VhallChatLogic::Destroy() {
   VH::CComPtr<IVhallRightExtraWidgetLogic> pVhallRightExtraWidget;
   DEF_GET_INTERFACE_PTR(SingletonVhallRightExtraWidgetIns, PID_IVhallRightExtraWidget, IID_IVhallRightExtraWidgetLogic, pVhallRightExtraWidget, return);
   pVhallRightExtraWidget->RemoveModuleWidget(&m_chatWidget);
   TRACE6("%s delete Destroy end\n", __FUNCTION__);
}
// 处理消息
void VhallChatLogic::DealMessage(DWORD adwMessageID, void* apData, DWORD adwLen) {

}
// 初始化个人信息
void VhallChatLogic::InitUserInfo(QString userName,QString userImageUrl,QString userId){
   m_chatWidget.InitUserInfo(userName, userImageUrl, userId);
   m_userId = userId;
}

// 接收消息
void VhallChatLogic::RecvMsg(Event &e) {
   if (e.m_eMsgType == e_RQ_UserChat){                                //聊天
      QString text = e.m_wzText;
      if(QString::fromWCharArray(e.m_oUserInfo.m_szUserID) != m_userId){
         m_chatWidget.appendChatMsg(text);
      }       
   }else if (e.m_eMsgType == e_RQ_UserProhibitSpeakAllUser) {       //全体禁言
      m_chatWidget.recv_forbidAll();
   }else if (e.m_eMsgType == e_RQ_UserAllowSpeakAllUser) {       //取消全体禁言
      m_chatWidget.recv_cancelForbidAll();
   }else{
      return;
   }
}
////发送消息
//void VhallChatLogic::SendMsg(MsgRQType eMsgType, RQData vData) {
///*      SingletonVhallRightExtraWidgetIns::Instance().PostCRMessage(
//         MSG_VHALLRIGHTEXTRAWIDGET_SENDMSG,&vData,sizeof(RQData));  */ 
//}
void VhallChatLogic::setRoomId(QString roomId,char *filterurl,bool forbidchat) {
   m_chatWidget.setRoomId(roomId,filterurl,forbidchat);
}

//切换时，隐藏内部部分内容
void VhallChatLogic::hideRightMouseMenuFunc(){
   m_chatWidget.hideRightMouseMenuFunc();
}

void VhallChatLogic::SetReloadChatHtml(bool bReload) {
   m_chatWidget.SetReloadChatHtml(bReload);
}

bool VhallChatLogic::IsLoadUrlFinished() {
   return m_chatWidget.IsLoadUrlFinished();
}
