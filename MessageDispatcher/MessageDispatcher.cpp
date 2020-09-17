#include "stdafx.h"
#include "MessageDispatcher.h"

#define MAX_MSG_PROCESS_TIME_OUT    5000

STRU_DATA_MESSAGE_QUEUE_ITEM::STRU_DATA_MESSAGE_QUEUE_ITEM()
{
	m_dwSenderID = 0;
	m_dwMessageID = 0;
	m_pData = NULL;
	m_uDataLength = 0;
}

STRU_DATA_MESSAGE_QUEUE_ITEM::~STRU_DATA_MESSAGE_QUEUE_ITEM()
{
	if (m_pData)
		CGlobalMemPool::Free(m_pData);
	m_pData = NULL;
}

BOOL STRU_DATA_MESSAGE_QUEUE_ITEM::SetData(const void * apData, DWORD adwLen)
{
	if (m_pData)
		CGlobalMemPool::Free(m_pData);
	m_pData = NULL;

	if (adwLen < 1)
		return TRUE;

	m_pData = CGlobalMemPool::Malloc(adwLen);

	if (NULL == m_pData)
		return FALSE;

	memcpy(m_pData, apData, adwLen);
	m_uDataLength = adwLen;

	return TRUE;
}

STRU_DATA_MESSAGE_ITEM::STRU_DATA_MESSAGE_ITEM()
{
	m_dwMessageID = 0;
}

CMessageDispatcher::CMessageDispatcher()
{
	//m_pMessageDispatcherEvent = NULL;
	m_hThread = NULL;
	m_hEvent = NULL;
	m_lRefCount = 0;
	m_bRunning = FALSE;
	m_bProhibitPost = FALSE;
}

CMessageDispatcher::~CMessageDispatcher()
{
	Clean();
}

HRESULT STDMETHODCALLTYPE CMessageDispatcher::QueryInterface(REFIID riid, void ** ppvObject)
{
	if (NULL == ppvObject)
		return CRE_INVALIDARG;

	if (riid == IID_VHIUnknown)
	{
		*ppvObject = (VH_IUnknown*)this;
		AddRef();
		return CRE_OK;
	}
	else if (riid == IID_IMessageDispatcher)
	{
		*ppvObject = (IMessageDispatcher*)this;
		AddRef();
		return CRE_OK;
	}

	return CRE_NOINTERFACE;
}

ULONG STDMETHODCALLTYPE CMessageDispatcher::AddRef(void)
{
	::InterlockedIncrement(&m_lRefCount);
	return m_lRefCount;
}

ULONG STDMETHODCALLTYPE CMessageDispatcher::Release(void)
{
	::InterlockedDecrement(&m_lRefCount);
	if (m_lRefCount == 0)
	{
		int a = 0;
	}
	return m_lRefCount;
}

////���÷ַ����¼�
//HRESULT STDMETHODCALLTYPE CMessageDispatcher::SetMessageDispatcherEvent(IMessageDispatcherEvent * apMessageDispatcherEvent)
//{
//	if (NULL != m_pMessageDispatcherEvent)
//	{
//		m_pMessageDispatcherEvent->Release();
//	}
//
//	m_pMessageDispatcherEvent = apMessageDispatcherEvent;
//
//	if (NULL != m_pMessageDispatcherEvent)
//	{
//		m_pMessageDispatcherEvent->AddRef();
//	}
//	return CRE_OK;
//}

////ȡ�÷ַ����¼�
//HRESULT STDMETHODCALLTYPE CMessageDispatcher::GetMessageDispatcherEvent(IMessageDispatcherEvent ** appMessageDispatcherEvent)
//{
//	if (NULL == m_pMessageDispatcherEvent || NULL == appMessageDispatcherEvent)
//	{
//		return CRE_FALSE;
//	}
//
//	*appMessageDispatcherEvent = m_pMessageDispatcherEvent;
//	m_pMessageDispatcherEvent->AddRef();
//
//	return CRE_OK;
//}

//����Ϣ�ɷ���ע����Ϣ
HRESULT STDMETHODCALLTYPE CMessageDispatcher::RegisterMessage(DWORD adwMessageID, DWORD adwFlag, IMessageEvent * apMessageEvent)
{
	if (NULL == apMessageEvent)
	{
		return CRE_FALSE;
	}

	//STRU_DATA_MESSAGE_ITEM * loDMI = NULL;

	////�ⷢ�¼�
	//if (NULL != m_pMessageDispatcherEvent)
	//{
	//	m_pMessageDispatcherEvent->OnRegisterMessage(adwMessageID, adwFlag, apMessageEvent);
	//}

	CCriticalAutoLock loGuard(m_oCriticalSectionMessage);

	//��m_oMessageItemList����idΪadwMessageID����
	MessageItemListIter loIter = m_oMessageItemList.find(adwMessageID);

	if (loIter == m_oMessageItemList.end())
	{
		//���������ӵ�m_oMessageItemList��
		STRU_DATA_MESSAGE_ITEM * loDMI = NULL;

		loDMI = new STRU_DATA_MESSAGE_ITEM;

		//����ʧ��
		if (NULL == loDMI)
		{
			return CRE_FALSE;
		}

		apMessageEvent->AddRef();

		//�����¼�����
		loDMI->m_oEventDict.insert(apMessageEvent);
		loDMI->m_dwMessageID = adwMessageID;

		//��������
		m_oMessageItemList[adwMessageID] = loDMI;
	}
	//Id�Ѵ���
	else
	{
		STRU_DATA_MESSAGE_ITEM * loDMI = loIter->second;

		ASSERT(loDMI);

		//����apMessageEvent�Ƿ��Ѵ���
		EventDict::iterator itor = loDMI->m_oEventDict.find(apMessageEvent);

		//û��
		if (itor == loDMI->m_oEventDict.end())
		{
			apMessageEvent->AddRef();

			//������,��ӵ�m_oEventDict��
			loDMI->m_oEventDict.insert(apMessageEvent);
		}
	}

	return CRE_OK;
}

//����Ϣ�ɷ������ע��
HRESULT STDMETHODCALLTYPE CMessageDispatcher::UnRegisterMessage(DWORD adwMessageID, IMessageEvent * apMessageEvent)
{
	if (NULL == apMessageEvent)
	{
		return CRE_FALSE;
	}

	////�ⷢ�¼�
	//if (NULL != m_pMessageDispatcherEvent)
	//{
	//	m_pMessageDispatcherEvent->OnUnRegisterMessage(adwMessageID, apMessageEvent);
	//}

	CCriticalAutoLock loGuard(m_oCriticalSectionMessage);

	//��m_oMessageItemList����idΪadwMessageID����
	MessageItemListIter loIter = m_oMessageItemList.find(adwMessageID);

	//������
	if (loIter == m_oMessageItemList.end())
	{
		return CRE_FALSE;
	}

	STRU_DATA_MESSAGE_ITEM * loDMI = loIter->second;

	ASSERT(loDMI);

	//����apMessageEvent�Ƿ��Ѵ���
	EventDict::iterator iter = loDMI->m_oEventDict.find(apMessageEvent);

	//������
	if (iter == loDMI->m_oEventDict.end())
	{
		return CRE_FALSE;
	}

	//�Ѵ���,��m_oEventDict��ɾ��
	loDMI->m_oEventDict.erase(iter);

	//�ͷ�����
	apMessageEvent->Release();

	//��ǰ��Ϣ�Ѿ�û����ע����
	if (loDMI->m_oEventDict.size() < 1)
	{
		//�Ӷ����в���
		m_oMessageItemList.erase(loIter);

		//ɾ������
		delete loDMI;
		loDMI = NULL;
	}

	return CRE_OK;
}

//����Ϣ�ɷ���Ͷ����Ϣ����Ϣ������У�������������
HRESULT STDMETHODCALLTYPE CMessageDispatcher::PostCRMessage(DWORD adwSenderID, DWORD adwMessageID, void * apData, DWORD adwLen)
{
	//�Ƿ��ֹͶ�ݵ�
	if (m_bProhibitPost)
	{
		return CRE_FALSE;
	}

	////�ⷢ�¼�
	//if (NULL != m_pMessageDispatcherEvent)
	//{
	//	if (CRE_OK != m_pMessageDispatcherEvent->OnMessagePreEnqueue(adwSenderID, adwMessageID, apData, adwLen))
	//	{
	//		return CRE_FALSE;
	//	}
	//}

	//������Ϣ��
	STRU_DATA_MESSAGE_QUEUE_ITEM * pDMQI = NULL;
	pDMQI = new STRU_DATA_MESSAGE_QUEUE_ITEM();
	if (NULL == pDMQI)
	{
		return CRE_FALSE;
	}

	pDMQI->m_dwSenderID = adwSenderID;
	pDMQI->m_dwMessageID = adwMessageID;

	if (FALSE == pDMQI->SetData(apData, adwLen))
	{
		delete pDMQI;
		return CRE_FALSE;
	}

	//�ӷŶ���
	m_oCriticalSectionQueue.Lock();
	m_oMessageQueueList.push_back(pDMQI);
	m_oCriticalSectionQueue.UnLock();

	//֪ͨ����Ϣ����
	if (m_hEvent)
	{
		::SetEvent(m_hEvent);
	}

	return CRE_OK;
}

// $_FUNCTION_BEGIN ******************************
// �������ƣ�Run
// ����������
// �� �� ֵ��
// ����˵��������������PostMessageThread�߳� 
// $_FUNCTION_END ********************************
BOOL CMessageDispatcher::Run()
{
	if (m_bRunning)
	{
		return TRUE;
	}

	m_bRunning = TRUE;

	//����PostMessageThread�߳�,��this��Ϊ����,����¼�߳�Id
	m_hThread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)&CMessageDispatcher::PostMessageThread, this, 0, NULL);

	if (INVALID_HANDLE_VALUE == m_hThread)
	{
		m_bRunning = FALSE;
	}

	return m_bRunning;
}
// $_FUNCTION_BEGIN ******************************
// �������ƣ�Stop
// ����������
// �� �� ֵ��
// ����˵������ֹPostMessageThread�߳�
// $_FUNCTION_END ********************************
BOOL CMessageDispatcher::Stop()
{
	//��ֹ��Ϣ����
	m_bProhibitPost = TRUE;

	//�˳��߳�
	m_bRunning = FALSE;

	if (m_hEvent)
	{
		::SetEvent(m_hEvent);
	}

	//�ȴ��߳̽���
	if (m_hThread)
	{
		::WaitForSingleObject(m_hThread, INFINITE);
		::CloseHandle(m_hThread);
		m_hThread = NULL;
	}

	return TRUE;
}

//��������������,�ͷ��ڴ�,������Դ
void CMessageDispatcher::Clean()
{
	//����m_oMessageItemList
	m_oCriticalSectionMessage.Lock();
	for (MessageItemListIter itorMEI = m_oMessageItemList.begin(); itorMEI != m_oMessageItemList.end(); ++itorMEI)
	{
		if (itorMEI->second)
		{
			itorMEI->second->m_oEventDict.clear();
			delete itorMEI->second;
		}
	}
	m_oMessageItemList.clear();
	m_oCriticalSectionMessage.UnLock();

	//����m_oMessageQueueList
	m_oCriticalSectionQueue.Lock();
	for (DMQIListIter itorDMQI = m_oMessageQueueList.begin(); itorDMQI != m_oMessageQueueList.end(); ++itorDMQI)
	{
		STRU_DATA_MESSAGE_QUEUE_ITEM * pMsgQueueItem = *itorDMQI;
		if (pMsgQueueItem)
		{
			delete pMsgQueueItem;
		}
	}
	m_oMessageQueueList.clear();
	m_oCriticalSectionQueue.UnLock();
}

// $_FUNCTION_BEGIN ******************************
// �������ƣ�PostMessageThread
// ����������LPVOID* apParam CMessageDispatcher��thisָ��
// �� �� ֵ������
// ����˵��������Ͷ����Ϣ���߳�
// $_FUNCTION_END ********************************
DWORD WINAPI CMessageDispatcher::PostMessageThread(LPVOID* apParam)
{
	//��ȡthis
	CMessageDispatcher * pThis = reinterpret_cast<CMessageDispatcher*>(apParam);

	if (pThis)
	{
		pThis->PostMessageWork();
	}

	return 0;
}

//����Ͷ����Ϣ���߳�
DWORD CMessageDispatcher::PostMessageWork()
{
	//�����¼����
	m_hEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);

	while (m_bRunning)
	{
		::WaitForSingleObject(m_hEvent, MAX_MSG_PROCESS_TIME_OUT);
		DealPostMessage();
	}

	//�ر��¼�
	if (m_hEvent)
	{
		::CloseHandle(m_hEvent);
		m_hEvent = NULL;
	}

	//��Ϣѭ�������������б�־ΪFALSE
	m_bRunning = FALSE;

	return 0;
}

//��ʱ���ص�����,������������Ϣ�ĺ���
void CMessageDispatcher::DealPostMessage()
{
	//������Ϣ,��������Ϊ
 	while (1)
	{
		STRU_DATA_MESSAGE_QUEUE_ITEM * pMsgQueueItem = NULL;

		//do
		//{
		m_oCriticalSectionQueue.Lock();
		if (m_oMessageQueueList.size() > 0)
		{
			pMsgQueueItem = m_oMessageQueueList.front();
			m_oMessageQueueList.pop_front();
		}
		m_oCriticalSectionQueue.UnLock();
		//}
		//while(0);

		if (NULL == pMsgQueueItem)
		{
			break;
		}

		DWORD dwMessageID = pMsgQueueItem->m_dwMessageID;

		m_oCriticalSectionMessage.Lock();
		//��m_oMessageItemList����idΪdwMessageID����
		MessageItemListIter loMessageItemListIter = m_oMessageItemList.find(dwMessageID);
		if (loMessageItemListIter != m_oMessageItemList.end())
		{
			//�ҵ���
			STRU_DATA_MESSAGE_ITEM * pMsgItem = loMessageItemListIter->second;

			//����m_oEventDict������Ϣ
			for (EventDict::iterator iter = pMsgItem->m_oEventDict.begin(); iter != pMsgItem->m_oEventDict.end(); ++iter)
			{
				IMessageEvent * pMessageEvent = *iter;
				if (pMessageEvent)
				{
					//֪ͨ�����¼�
					pMessageEvent->OnRecvMessage(pMsgQueueItem->m_dwSenderID, dwMessageID, pMsgQueueItem->m_pData, pMsgQueueItem->m_uDataLength);
				}
			}
		}
		m_oCriticalSectionMessage.UnLock();

		//ɾ����Ϣ��
		delete pMsgQueueItem;

		static int iMessageCount = 0;
		//ʹ�ü����������ڴ����,����ÿ����ϢҪ50K�ڴ�,�������ڴ��ۼƳ���50Mʱ,��������
		if (++iMessageCount > 1000)
		{
			iMessageCount = 0;
			CGlobalMemPool::Recovery();
		}
	}
}