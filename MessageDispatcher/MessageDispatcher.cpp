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

////设置分发器事件
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

////取得分发器事件
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

//向消息派发器注册消息
HRESULT STDMETHODCALLTYPE CMessageDispatcher::RegisterMessage(DWORD adwMessageID, DWORD adwFlag, IMessageEvent * apMessageEvent)
{
	if (NULL == apMessageEvent)
	{
		return CRE_FALSE;
	}

	//STRU_DATA_MESSAGE_ITEM * loDMI = NULL;

	////解发事件
	//if (NULL != m_pMessageDispatcherEvent)
	//{
	//	m_pMessageDispatcherEvent->OnRegisterMessage(adwMessageID, adwFlag, apMessageEvent);
	//}

	CCriticalAutoLock loGuard(m_oCriticalSectionMessage);

	//在m_oMessageItemList查找id为adwMessageID的项
	MessageItemListIter loIter = m_oMessageItemList.find(adwMessageID);

	if (loIter == m_oMessageItemList.end())
	{
		//构造新项并添加到m_oMessageItemList中
		STRU_DATA_MESSAGE_ITEM * loDMI = NULL;

		loDMI = new STRU_DATA_MESSAGE_ITEM;

		//分配失败
		if (NULL == loDMI)
		{
			return CRE_FALSE;
		}

		apMessageEvent->AddRef();

		//保存事件对象
		loDMI->m_oEventDict.insert(apMessageEvent);
		loDMI->m_dwMessageID = adwMessageID;

		//保存新项
		m_oMessageItemList[adwMessageID] = loDMI;
	}
	//Id已存在
	else
	{
		STRU_DATA_MESSAGE_ITEM * loDMI = loIter->second;

		ASSERT(loDMI);

		//查找apMessageEvent是否已存在
		EventDict::iterator itor = loDMI->m_oEventDict.find(apMessageEvent);

		//没有
		if (itor == loDMI->m_oEventDict.end())
		{
			apMessageEvent->AddRef();

			//不存在,添加到m_oEventDict中
			loDMI->m_oEventDict.insert(apMessageEvent);
		}
	}

	return CRE_OK;
}

//向消息派发器解除注册
HRESULT STDMETHODCALLTYPE CMessageDispatcher::UnRegisterMessage(DWORD adwMessageID, IMessageEvent * apMessageEvent)
{
	if (NULL == apMessageEvent)
	{
		return CRE_FALSE;
	}

	////解发事件
	//if (NULL != m_pMessageDispatcherEvent)
	//{
	//	m_pMessageDispatcherEvent->OnUnRegisterMessage(adwMessageID, apMessageEvent);
	//}

	CCriticalAutoLock loGuard(m_oCriticalSectionMessage);

	//在m_oMessageItemList查找id为adwMessageID的项
	MessageItemListIter loIter = m_oMessageItemList.find(adwMessageID);

	//不存在
	if (loIter == m_oMessageItemList.end())
	{
		return CRE_FALSE;
	}

	STRU_DATA_MESSAGE_ITEM * loDMI = loIter->second;

	ASSERT(loDMI);

	//查找apMessageEvent是否已存在
	EventDict::iterator iter = loDMI->m_oEventDict.find(apMessageEvent);

	//不存在
	if (iter == loDMI->m_oEventDict.end())
	{
		return CRE_FALSE;
	}

	//已存在,从m_oEventDict中删除
	loDMI->m_oEventDict.erase(iter);

	//释放引用
	apMessageEvent->Release();

	//当前消息已经没有人注册了
	if (loDMI->m_oEventDict.size() < 1)
	{
		//从队列中擦除
		m_oMessageItemList.erase(loIter);

		//删除对象
		delete loDMI;
		loDMI = NULL;
	}

	return CRE_OK;
}

//向消息派发器投递消息，消息进入队列，函数立即返回
HRESULT STDMETHODCALLTYPE CMessageDispatcher::PostCRMessage(DWORD adwSenderID, DWORD adwMessageID, void * apData, DWORD adwLen)
{
	//是否禁止投递的
	if (m_bProhibitPost)
	{
		return CRE_FALSE;
	}

	////解发事件
	//if (NULL != m_pMessageDispatcherEvent)
	//{
	//	if (CRE_OK != m_pMessageDispatcherEvent->OnMessagePreEnqueue(adwSenderID, adwMessageID, apData, adwLen))
	//	{
	//		return CRE_FALSE;
	//	}
	//}

	//创建消息项
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

	//加放队列
	m_oCriticalSectionQueue.Lock();
	m_oMessageQueueList.push_back(pDMQI);
	m_oCriticalSectionQueue.UnLock();

	//通知有消息来到
	if (m_hEvent)
	{
		::SetEvent(m_hEvent);
	}

	return CRE_OK;
}

// $_FUNCTION_BEGIN ******************************
// 函数名称：Run
// 函数参数：
// 返 回 值：
// 函数说明：创建并运行PostMessageThread线程 
// $_FUNCTION_END ********************************
BOOL CMessageDispatcher::Run()
{
	if (m_bRunning)
	{
		return TRUE;
	}

	m_bRunning = TRUE;

	//创建PostMessageThread线程,以this作为参数,并记录线程Id
	m_hThread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)&CMessageDispatcher::PostMessageThread, this, 0, NULL);

	if (INVALID_HANDLE_VALUE == m_hThread)
	{
		m_bRunning = FALSE;
	}

	return m_bRunning;
}
// $_FUNCTION_BEGIN ******************************
// 函数名称：Stop
// 函数参数：
// 返 回 值：
// 函数说明：终止PostMessageThread线程
// $_FUNCTION_END ********************************
BOOL CMessageDispatcher::Stop()
{
	//禁止消息进入
	m_bProhibitPost = TRUE;

	//退出线程
	m_bRunning = FALSE;

	if (m_hEvent)
	{
		::SetEvent(m_hEvent);
	}

	//等待线程结束
	if (m_hThread)
	{
		::WaitForSingleObject(m_hThread, INFINITE);
		::CloseHandle(m_hThread);
		m_hThread = NULL;
	}

	return TRUE;
}

//由析构函数调用,释放内存,清理资源
void CMessageDispatcher::Clean()
{
	//清理m_oMessageItemList
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

	//清理m_oMessageQueueList
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
// 函数名称：PostMessageThread
// 函数参数：LPVOID* apParam CMessageDispatcher的this指针
// 返 回 值：保留
// 函数说明：处理投递消息的线程
// $_FUNCTION_END ********************************
DWORD WINAPI CMessageDispatcher::PostMessageThread(LPVOID* apParam)
{
	//获取this
	CMessageDispatcher * pThis = reinterpret_cast<CMessageDispatcher*>(apParam);

	if (pThis)
	{
		pThis->PostMessageWork();
	}

	return 0;
}

//处理投递消息的线程
DWORD CMessageDispatcher::PostMessageWork()
{
	//创建事件句柄
	m_hEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);

	while (m_bRunning)
	{
		::WaitForSingleObject(m_hEvent, MAX_MSG_PROCESS_TIME_OUT);
		DealPostMessage();
	}

	//关闭事件
	if (m_hEvent)
	{
		::CloseHandle(m_hEvent);
		m_hEvent = NULL;
	}

	//消息循环结束设置运行标志为FALSE
	m_bRunning = FALSE;

	return 0;
}

//定时器回调函数,真正处理发送消息的函数
void CMessageDispatcher::DealPostMessage()
{
	//处理消息,最大处理个数为
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
		//在m_oMessageItemList查找id为dwMessageID的项
		MessageItemListIter loMessageItemListIter = m_oMessageItemList.find(dwMessageID);
		if (loMessageItemListIter != m_oMessageItemList.end())
		{
			//找到了
			STRU_DATA_MESSAGE_ITEM * pMsgItem = loMessageItemListIter->second;

			//遍历m_oEventDict处理消息
			for (EventDict::iterator iter = pMsgItem->m_oEventDict.begin(); iter != pMsgItem->m_oEventDict.end(); ++iter)
			{
				IMessageEvent * pMessageEvent = *iter;
				if (pMessageEvent)
				{
					//通知接收事件
					pMessageEvent->OnRecvMessage(pMsgQueueItem->m_dwSenderID, dwMessageID, pMsgQueueItem->m_pData, pMsgQueueItem->m_uDataLength);
				}
			}
		}
		m_oCriticalSectionMessage.UnLock();

		//删除消息项
		delete pMsgQueueItem;

		static int iMessageCount = 0;
		//使用计数来控制内存回收,假设每条消息要50K内存,当分配内存累计超过50M时,进行清理
		if (++iMessageCount > 1000)
		{
			iMessageCount = 0;
			CGlobalMemPool::Recovery();
		}
	}
}