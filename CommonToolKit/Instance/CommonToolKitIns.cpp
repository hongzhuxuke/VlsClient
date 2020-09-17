#include "StdAfx.h"
#include "CommonToolKitIns.h"

#include "Msg_CommonToolKit.h"

CommonToolKitIns::CommonToolKitIns(void)
: CPluginBase(PID_ICommonToolKit, ENUM_PLUGIN_COMMONTOOLKIT, 10)
, m_lThreadCount(0)
, m_bIsAlive(FALSE)
{
	wcsncpy( m_oPluginInfo.m_wzCRPName, L"HTTPģ��", DEF_MAX_CRP_NAME_LEN );
	wcsncpy( m_oPluginInfo.m_wzCRPDescrip, L"��HTTP����/���صķ�װ������չ��������", DEF_MAX_CRP_DESCRIP_LEN );

	m_oPluginInfo.m_dwMajorVer = 1;
	m_oPluginInfo.m_dwMinorVer = 0;
	m_oPluginInfo.m_dwPatchVer = 0;
	m_oPluginInfo.m_dwBuildVer = 1;
	wcsncpy( m_oPluginInfo.m_wzCRPVerDescrip, L"��ʼ���汾", DEF_MAX_CRP_VER_DESCRIP_LEN );
}

CommonToolKitIns::~CommonToolKitIns(void)
{
	ASSERT(FALSE == m_bIsAlive);
	ASSERT(0 == m_lThreadCount);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------------------------------------------------------
//��ѯ��չ�ӿ�
HRESULT CommonToolKitIns::InstanceQueryInterface(REFIID riid, void ** appvObject)
{
   return m_oCommonData.QueryInterface(riid, appvObject);
}

//�������
HRESULT CommonToolKitIns::InstanceCreatePlugin()
{
	//��HTTP���ع�����
	do
	{  
		// ����log
		InitDebugTrace(L"HttpCenter", 6);
      TRACE6("%s============================================================================================================= \n",__FUNCTION__);

		//�򿪹������
		m_bIsAlive = TRUE;

		CBaseThread loBaseThread;
		//���������߳�
		loBaseThread.BeginThread(CommonToolKitIns::CheckMessageThread, this);
		return CRE_OK;
	}
	while(0);

	//ʧ������
	InstanceDestroyPlugin();

	return CRE_FALSE;
}

//���ٲ��
HRESULT CommonToolKitIns::InstanceDestroyPlugin()
{
	TRACE6("CommonToolKitIns::InstanceDestroyPlugin() Begin \n");

	//�ر����б��
	m_bIsAlive = FALSE;

	//�ȴ��߳̽���
	while(m_lThreadCount > 0)
	{
		CBaseThread::Sleep(10);
	}

	TRACE6("CommonToolKitIns::InstanceDestroyPlugin() End \n");
	return CRE_OK;
}

//��ʼ����Ҫע�����Ϣ
HRESULT CommonToolKitIns::InstanceInitRegMessage(CRMessageIDQueue& aoCRMessageIDQueue)
{
	aoCRMessageIDQueue.push_back(MSG_HTTPCENTER_HTTP_RQ);
	aoCRMessageIDQueue.push_back(MSG_HTTPCEMYER_HTTP_LOG_RQ);

	return CRE_OK;
}

//������Ϣ
HRESULT CommonToolKitIns::InstanceDealCRMessage(DWORD adwSenderID, DWORD adwMessageID, void * apData, DWORD adwLen)
{
	switch(adwMessageID)
	{
		//HTTP����
	case MSG_HTTPCENTER_HTTP_RQ: {
			DEF_CR_MESSAGE_DATA_DECLARE(STRU_HTTPCENTER_HTTP_RQ, loMessage, CRE_FALSE);
			//������񵽶���
		}
		break;	
   case MSG_HTTPCEMYER_HTTP_LOG_RQ: {
      DEF_CR_MESSAGE_DATA_DECLARE(STRU_HTTPCENTER_HTTP_RQ, loMessage, CRE_FALSE);
      //������񵽶���
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
		//�ɷ�������Ϣ
		DispatchCachedMessage();
		//˯��һ��ʱ��
		CBaseThread::Sleep(30);
	}
	return 0;
}
