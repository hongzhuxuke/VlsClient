#include "StdAfx.h"
#include "CommonToolKitIns.h"

#include "Msg_CommonToolKit.h"

CommonToolKitIns::CommonToolKitIns(void)
: CPluginBase(PID_ICommonToolKit, ENUM_PLUGIN_COMMONTOOLKIT, 10)
, m_lThreadCount(0)
, m_bIsAlive(FALSE)
{
	wcsncpy( m_oPluginInfo.m_wzCRPName, L"HTTP模块", DEF_MAX_CRP_NAME_LEN );
	wcsncpy( m_oPluginInfo.m_wzCRPDescrip, L"对HTTP请求/下载的封装，可扩展其他下载", DEF_MAX_CRP_DESCRIP_LEN );

	m_oPluginInfo.m_dwMajorVer = 1;
	m_oPluginInfo.m_dwMinorVer = 0;
	m_oPluginInfo.m_dwPatchVer = 0;
	m_oPluginInfo.m_dwBuildVer = 1;
	wcsncpy( m_oPluginInfo.m_wzCRPVerDescrip, L"初始化版本", DEF_MAX_CRP_VER_DESCRIP_LEN );
}

CommonToolKitIns::~CommonToolKitIns(void)
{
	ASSERT(FALSE == m_bIsAlive);
	ASSERT(0 == m_lThreadCount);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------------------------------------------------------
//查询扩展接口
HRESULT CommonToolKitIns::InstanceQueryInterface(REFIID riid, void ** appvObject)
{
   return m_oCommonData.QueryInterface(riid, appvObject);
}

//创建插件
HRESULT CommonToolKitIns::InstanceCreatePlugin()
{
	//打开HTTP下载管理器
	do
	{  
		// 设置log
		InitDebugTrace(L"HttpCenter", 6);
      TRACE6("%s============================================================================================================= \n",__FUNCTION__);

		//打开工作标记
		m_bIsAlive = TRUE;

		CBaseThread loBaseThread;
		//创建工作线程
		loBaseThread.BeginThread(CommonToolKitIns::CheckMessageThread, this);
		return CRE_OK;
	}
	while(0);

	//失败销毁
	InstanceDestroyPlugin();

	return CRE_FALSE;
}

//销毁插件
HRESULT CommonToolKitIns::InstanceDestroyPlugin()
{
	TRACE6("CommonToolKitIns::InstanceDestroyPlugin() Begin \n");

	//关闭运行标记
	m_bIsAlive = FALSE;

	//等待线程结束
	while(m_lThreadCount > 0)
	{
		CBaseThread::Sleep(10);
	}

	TRACE6("CommonToolKitIns::InstanceDestroyPlugin() End \n");
	return CRE_OK;
}

//初始化需要注册的消息
HRESULT CommonToolKitIns::InstanceInitRegMessage(CRMessageIDQueue& aoCRMessageIDQueue)
{
	aoCRMessageIDQueue.push_back(MSG_HTTPCENTER_HTTP_RQ);
	aoCRMessageIDQueue.push_back(MSG_HTTPCEMYER_HTTP_LOG_RQ);

	return CRE_OK;
}

//处理消息
HRESULT CommonToolKitIns::InstanceDealCRMessage(DWORD adwSenderID, DWORD adwMessageID, void * apData, DWORD adwLen)
{
	switch(adwMessageID)
	{
		//HTTP请求
	case MSG_HTTPCENTER_HTTP_RQ: {
			DEF_CR_MESSAGE_DATA_DECLARE(STRU_HTTPCENTER_HTTP_RQ, loMessage, CRE_FALSE);
			//添加任务到队列
		}
		break;	
   case MSG_HTTPCEMYER_HTTP_LOG_RQ: {
      DEF_CR_MESSAGE_DATA_DECLARE(STRU_HTTPCENTER_HTTP_RQ, loMessage, CRE_FALSE);
      //添加任务到队列
      break;
   }
	default:
		break;
	}
	return CRE_OK;
}

//------------------------------------------------------------------------------------------------------------------------------
unsigned int CommonToolKitIns::CheckMessageThread(void * apParam)
{
   TRACE4("CommonToolKitIns::CheckMessageThread %lu\n",GetCurrentThreadId());
	CommonToolKitIns* pThis = static_cast<CommonToolKitIns*>(apParam);

	if(pThis) {
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

unsigned int CommonToolKitIns::CheckMessageWork()
{
	while(m_bIsAlive) {
		//派发缓存消息
		DispatchCachedMessage();
		//睡眠一段时间
		CBaseThread::Sleep(30);
	}
	return 0;
}
