#include "Msg_VhallRightExtraWidget.h"
#include "VhallRightExtraWidgetIns.h"
#include "VhallNoticeInfoLogic.h"
#include "VhallRightExtraWidgetIns.h"
#include "DebugTrace.h"
#include <windows.h>
#include <QDebug>
VhallNoticeInfoLogic::VhallNoticeInfoLogic(QObject *parent ):
   QObject(parent){
}

VhallNoticeInfoLogic::~VhallNoticeInfoLogic(void) {
   TRACE6("%s delete end\n", __FUNCTION__);
}


BOOL VhallNoticeInfoLogic::Create() {
   VH::CComPtr<IVhallRightExtraWidgetLogic> pVhallRightExtraWidget;
   m_noticeInfoWidget.showMinimized();
   DEF_GET_INTERFACE_PTR(SingletonVhallRightExtraWidgetIns, PID_IVhallRightExtraWidget, IID_IVhallRightExtraWidgetLogic, pVhallRightExtraWidget, return FALSE);
   pVhallRightExtraWidget->AppendModuleWidget(&m_noticeInfoWidget,L"公告");
   return TRUE;
}
void VhallNoticeInfoLogic::Destroy() {
   VH::CComPtr<IVhallRightExtraWidgetLogic> pVhallRightExtraWidget;
   DEF_GET_INTERFACE_PTR(SingletonVhallRightExtraWidgetIns, PID_IVhallRightExtraWidget, IID_IVhallRightExtraWidgetLogic, pVhallRightExtraWidget, return);
   pVhallRightExtraWidget->RemoveModuleWidget(&m_noticeInfoWidget);
   TRACE6("%s delete end\n", __FUNCTION__);
}
// 处理消息
void VhallNoticeInfoLogic::DealMessage(DWORD adwMessageID, void* apData, DWORD adwLen) {

}
// 接收消息
void VhallNoticeInfoLogic::RecvMsg(Event &e) {
   /*QString text = e.m_wzText;
   if (e.m_eMsgType != e_RQ_ReleaseAnnouncement){
      return;
   }else{
      m_noticeInfoWidget.appendNoticeMsg(text);
   }*/
   return;
}
//发送消息
void VhallNoticeInfoLogic::SendMsg(MsgRQType eMsgType, RQData vData) {
   RQData oData;
   oData.m_eMsgType=e_RQ_UserOnlineList;
   SingletonVhallRightExtraWidgetIns::Instance().PostCRMessage(MSG_VHALLRIGHTEXTRAWIDGET_SENDMSG,&oData,sizeof(RQData));   
}
void VhallNoticeInfoLogic::Refresh(bool bInit){
   m_noticeInfoWidget.Refresh(bInit);
}

void VhallNoticeInfoLogic::RecvAnnouncement(QString content,QString time) {
   m_noticeInfoWidget.appendNoticeMsg(content);
}

void VhallNoticeInfoLogic::ShowNoticeWnd() {
   m_noticeInfoWidget.ShowNoticeWnd();
}

bool VhallNoticeInfoLogic::IsLoadUrlFinished() {
   return m_noticeInfoWidget.IsLoadUrlFinished();
}

