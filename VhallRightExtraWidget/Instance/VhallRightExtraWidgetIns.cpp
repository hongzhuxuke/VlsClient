#include <CRPluginDef.h>
#include "Msg_VhallRightExtraWidget.h"
#include "Msg_OBSControl.h"
#include "Msg_MainUI.h"
#include "IVhallRightExtraWidgetLogic.h"
#include "VhallRightExtraWidgetIns.h"     
#include "DebugTrace.h"
#include "BaseThread.h" 
#include "pub.Struct.h"
#include "pathmanager.h"
#include "Logging.h"
Logger *gLogger = NULL;

VhallRightExtraWidgetIns::VhallRightExtraWidgetIns() 
   : CPluginBase(PID_IVhallRightExtraWidget, ENUM_PLUGIN_VHALLRIGHETEXTRAWIDGET, 10)
   , m_lThreadCount(0)
   , m_dwUIThreadID(0)
   , m_bIsAlive(FALSE) {
   wcsncpy(m_oPluginInfo.m_wzCRPName, L"互动模块", DEF_MAX_CRP_NAME_LEN);
   wcsncpy(m_oPluginInfo.m_wzCRPDescrip, L"功能能包括聊天、用户列表", DEF_MAX_CRP_DESCRIP_LEN);

   m_oPluginInfo.m_dwMajorVer = 1;
   m_oPluginInfo.m_dwMinorVer = 0;
   m_oPluginInfo.m_dwPatchVer = 0;
   m_oPluginInfo.m_dwBuildVer = 1;
   wcsncpy(m_oPluginInfo.m_wzCRPVerDescrip, L"初始化版本", DEF_MAX_CRP_VER_DESCRIP_LEN);

   SYSTEMTIME loSystemTime;
   GetLocalTime(&loSystemTime);
   wchar_t lwzLogFileName[255] = { 0 };
   wsprintf(lwzLogFileName, L"%s%s_%4d_%02d_%02d_%02d_%02d%s", 
	   VH_LOG_DIR, L"VhallRightExtraWidgetIns", loSystemTime.wYear, loSystemTime.wMonth, loSystemTime.wDay, loSystemTime.wHour, loSystemTime.wMinute, L".log");
   gLogger = new Logger(lwzLogFileName, USER);

}
VhallRightExtraWidgetIns::~VhallRightExtraWidgetIns() {
   ASSERT(FALSE == m_bIsAlive);
   ASSERT(0 == m_lThreadCount);
   if (gLogger) {
      delete gLogger;
      gLogger = NULL;
   }
}
//实例接口查询
HRESULT VhallRightExtraWidgetIns::InstanceQueryInterface(REFIID riid, void ** appvObject) {
   return m_pRightExtraWidgetLogic.QueryInterface(riid,appvObject);
}
//创建插件
HRESULT VhallRightExtraWidgetIns::InstanceCreatePlugin() {
   do {
      InitDebugTrace(L"VhallRightExtraWidget", 6);

      if (!m_pRightExtraWidgetLogic.Create()) {
         break;
      }
      
      //打开工作标记
      m_bIsAlive = TRUE;

      //启动UI线程
      m_dwUIThreadID = BeginUIThread(ENUM_PLUGIN_THREAD_UI);

      return CRE_OK;

   } while (false);
   
   //失败销毁
   InstanceDestroyPlugin();

   return CRE_FALSE;
}
//销毁插件
HRESULT VhallRightExtraWidgetIns::InstanceDestroyPlugin() {
   //关闭运行标记
   m_bIsAlive = FALSE;

   //等待线程结束
   while (m_lThreadCount > 0) {
      CBaseThread::Sleep(10);
   }

   m_pRightExtraWidgetLogic.Destroy();
   return CRE_OK;
}
//初始化需要注册的消息
HRESULT VhallRightExtraWidgetIns::InstanceInitRegMessage(CRMessageIDQueue& aoCRMessageIDQueue) {
   //初始化通用信息
   aoCRMessageIDQueue.push_back(MSG_VHALLRIGHTEXTRAWIDGET_INITCOMMONINFO);
   //推流
   //aoCRMessageIDQueue.push_back(MSG_OBSCONTROL_PUBLISH);
   //获取下一页用户列表
   aoCRMessageIDQueue.push_back(MSG_VHALLRIGHTEXTRAWIDGET_GETNEWPAGEONLINELIST);
   //执行刷新
   aoCRMessageIDQueue.push_back(MSG_VHALLRIGHTEXTRAWIDGET_DO_REFRESH);
   //停止计时消息
   aoCRMessageIDQueue.push_back(MSG_VHALLRIGHTEXTRAWIDGET_STOP_CREATE_RECORD_TIMER);
   //右键用户
   aoCRMessageIDQueue.push_back(MSG_VHALLRIGHTEXTRAWIDGET_RIGHT_MOUSE_BUTTON_USER);   
   return CRE_OK;
}
//处理消息
HRESULT VhallRightExtraWidgetIns::InstanceDealCRMessage(DWORD adwSenderID, DWORD adwMessageID, void * apData, DWORD adwLen) {
   m_pRightExtraWidgetLogic.DealMessage(adwMessageID, apData, adwLen);
   return CRE_OK;
}

unsigned int VhallRightExtraWidgetIns::CheckMessageThread(void * apParam) {
   TRACE4("CommonToolKitIns::CheckMessageThread %lu\n", GetCurrentThreadId());
   VhallRightExtraWidgetIns* pThis = static_cast<VhallRightExtraWidgetIns*>(apParam);

   if (pThis) {
      pThis->m_oThreadCountCS.Lock();
      ++(pThis->m_lThreadCount);
      pThis->m_oThreadCountCS.UnLock();
      pThis->CheckMessageWork();
      pThis->m_oThreadCountCS.Lock();
      --(pThis->m_lThreadCount);
      pThis->m_oThreadCountCS.UnLock();

   }
   return 0;
}
unsigned int VhallRightExtraWidgetIns::CheckMessageWork() {
   while (m_bIsAlive) {
      //派发缓存消息
      DispatchCachedMessage();
      //睡眠一段时间
      CBaseThread::Sleep(30);
   }
   return 0;
}