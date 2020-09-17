#include "StdAfx.h"
#include "OBSControlIns.h"

#include "Msg_OBSControl.h"
#include "Msg_CommonToolKit.h"

#include "pub.Struct.h"
#include "pathmanager.h"
#include "Msg_MainUI.h"

OBSControlIns::OBSControlIns(void)
: CPluginBase(PID_IOBSControl, ENUM_PLUGIN_OBSCONTROL, 10)
, m_lThreadCount(0)
, m_bIsAlive(FALSE) {
   wcsncpy(m_oPluginInfo.m_wzCRPName, L"OBS控制模块", DEF_MAX_CRP_NAME_LEN);
   wcsncpy(m_oPluginInfo.m_wzCRPDescrip, L"负责统一处理对OBS渲染/编解码/推流", DEF_MAX_CRP_DESCRIP_LEN);

   m_oPluginInfo.m_dwMajorVer = 1;
   m_oPluginInfo.m_dwMinorVer = 0;
   m_oPluginInfo.m_dwPatchVer = 0;
   m_oPluginInfo.m_dwBuildVer = 1;
   wcsncpy(m_oPluginInfo.m_wzCRPVerDescrip, L"初始化版本", DEF_MAX_CRP_VER_DESCRIP_LEN);
}

OBSControlIns::~OBSControlIns(void) {
   ASSERT(FALSE == m_bIsAlive);
   ASSERT(0 == m_lThreadCount);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------------------------------------------------------
//查询扩展接口
HRESULT OBSControlIns::InstanceQueryInterface(REFIID riid, void ** appvObject) {
   if (riid == IID_IOBSControlLogic) {
      return m_oOBSControlLogic.QueryInterface(riid, appvObject);
   }
   else {
      return m_oDeviceManager.QueryInterface(riid, appvObject);
   }
}

//创建插件
HRESULT OBSControlIns::InstanceCreatePlugin() {
   //打开HTTP下载管理器
   do {
      // 设置log
      InitDebugTrace(L"OBSControl", 6);
      TRACE6("%s============================================================================================================= \n",__FUNCTION__);
      TRACE6("%s start  create m_oOBSControlLogic\n",__FUNCTION__);
      //逻辑创建
      //if (!m_oOBSControlLogic.Create()) {
      //   break;
      //}
      //TRACE6("create m_oOBSControlLogic ok\n");

      ////逻辑创建
      //if (!m_oDeviceManager.Create()) {
      //   break;
      //}
      //TRACE6("create m_oDeviceManager ok\n");

      //打开工作标记
      m_bIsAlive = TRUE;

      CBaseThread loBaseThread;
      //创建工作线程
      loBaseThread.BeginThread(OBSControlIns::CheckMessageThread, this);
      return CRE_OK;
   } while (0);

   //失败销毁
   //InstanceDestroyPlugin();

   return CRE_FALSE;
}

//销毁插件
HRESULT OBSControlIns::InstanceDestroyPlugin() {
   TRACE6("OBSControlIns::InstanceDestroyPlugin() Begin \n");

   //关闭运行标记
   m_bIsAlive = FALSE;

   //等待线程结束
   while (m_lThreadCount > 0) {
      CBaseThread::Sleep(10);
   }

   //逻辑销毁
   m_oDeviceManager.Destroy();

   //逻辑销毁
   m_oOBSControlLogic.Destroy();

   TRACE6("OBSControlIns::InstanceDestroyPlugin() End \n");
   return CRE_OK;
}

//初始化需要注册的消息
HRESULT OBSControlIns::InstanceInitRegMessage(CRMessageIDQueue& aoCRMessageIDQueue) {
   aoCRMessageIDQueue.push_back(MSG_OBSCONTROL_PUBLISH);
   aoCRMessageIDQueue.push_back(MSG_OBSCONTROL_TEXT);
   aoCRMessageIDQueue.push_back(MSG_OBSCONTROL_IMAGE);
   aoCRMessageIDQueue.push_back(MSG_OBSCONTROL_DESKTOPSHARE);
   aoCRMessageIDQueue.push_back(MSG_OBSCONTROL_WINDOWSRC);
   aoCRMessageIDQueue.push_back(MSG_OBSCONTROL_SHAREREGION);
   aoCRMessageIDQueue.push_back(MSG_OBSCONTROL_PROCESSSRC);
   aoCRMessageIDQueue.push_back(MSG_OBSCONTROL_ADDCAMERA);
   aoCRMessageIDQueue.push_back(MSG_OBSCONTROL_AUDIO_CAPTURE);
	aoCRMessageIDQueue.push_back(MSG_OBSCONTROL_VIDIO_HIGHQUALITYCOD);
   //外部消息
   aoCRMessageIDQueue.push_back(MSG_COMMONDATA_DATA_INIT);
   aoCRMessageIDQueue.push_back(MSG_OBSCONTROL_PUSH_AMF0);   
   aoCRMessageIDQueue.push_back(MSG_OBSCONTROL_ENABLE_VT);
   aoCRMessageIDQueue.push_back(MSG_OBSCONTROL_CLOSE_AUDIO_DEV);
   aoCRMessageIDQueue.push_back(MSG_MAINUI_CLICK_CONTROL);
   aoCRMessageIDQueue.push_back(MSG_HTTPCENTER_HTTP_RS);
  
   return CRE_OK;
}

//处理消息
HRESULT OBSControlIns::InstanceDealCRMessage(DWORD adwSenderID, DWORD adwMessageID, void * apData, DWORD adwLen) {
   m_oOBSControlLogic.DealMessage(adwMessageID, apData, adwLen);
   return CRE_OK;
}

//------------------------------------------------------------------------------------------------------------------------------
unsigned int OBSControlIns::CheckMessageThread(void * apParam) {
   TRACE4("OBSControlIns::CheckMessageThread %lu\n",GetCurrentThreadId());
   OBSControlIns* pThis = static_cast<OBSControlIns*>(apParam);

   if (pThis) {
      pThis->m_oThreadCountCS.Lock();
      ++(pThis->m_lThreadCount);
      pThis->m_oThreadCountCS.UnLock();
      //初始化COM
      CoInitialize(NULL);

      pThis->CheckMessageWork();

      //反初始化COM
      CoUninitialize();
      pThis->m_oThreadCountCS.Lock();
      --(pThis->m_lThreadCount);
      pThis->m_oThreadCountCS.UnLock();
   }
   return 0;
}

unsigned int OBSControlIns::CheckMessageWork() {
   while (m_bIsAlive) {
      //派发缓存消息
      DispatchCachedMessage();
      //睡眠一段时间
      CBaseThread::Sleep(30);
   }
   return 0;
}


void OBSControlIns::reportLog(const wchar_t* lKey, const eLogRePortK ekey, const QJsonObject body)
{
	STRU_MAINUI_LOG log;
	swprintf_s(log.m_wzRequestUrl, DEF_MAX_HTTP_URL_LEN,L"key=%s&k=%d",lKey,ekey);
	QString json = CPathManager::GetStringFromJsonObject(body);
	strcpy_s(log.m_wzRequestJson, json.mid(0, DEF_MAX_HTTP_URL_LEN).toLatin1().data());
	SingletonOBSControlIns::Instance().PostCRMessage(MSG_MAINUI_LOG, &log, sizeof(STRU_MAINUI_LOG));
}
