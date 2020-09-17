#pragma once
#include <list>
#include <map>
#include <set>
#include "IMessageDispatcher.h"
#include "MemPoolBase.h"
#include "Singleton.h"

//消息项
struct STRU_DATA_MESSAGE_QUEUE_ITEM : public CMemPoolBase
{
	DWORD	m_dwSenderID;	//发送者的ID
	DWORD	m_dwMessageID;	//消息ID
	void *	m_pData;		//数据
	ULONG	m_uDataLength;	//数据长
public:
	BOOL SetData(const void * apData, DWORD adwLen);
public:
	STRU_DATA_MESSAGE_QUEUE_ITEM();
	~STRU_DATA_MESSAGE_QUEUE_ITEM();
};

//定义类型
typedef std::set< IMessageEvent* >	EventDict;

//消息队列项,用于PostMessage
struct STRU_DATA_MESSAGE_ITEM : public CMemPoolBase
{
	DWORD		m_dwMessageID;	//消息ID
	EventDict	m_oEventDict;	//事件列表
public:
	STRU_DATA_MESSAGE_ITEM();
};

class CMessageDispatcher : public IMessageDispatcher
{
public:
	//构造函数/析构函数
	CMessageDispatcher();
	~CMessageDispatcher();

public:
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void ** ppvObject);
	virtual ULONG STDMETHODCALLTYPE AddRef(void);
	virtual ULONG STDMETHODCALLTYPE Release(void);

public:
	////设置分发器事件
	//virtual HRESULT STDMETHODCALLTYPE SetMessageDispatcherEvent(IMessageDispatcherEvent * apMessageDispatcherEvent);
	////取得分发器事件
	//virtual HRESULT STDMETHODCALLTYPE GetMessageDispatcherEvent(IMessageDispatcherEvent ** appMessageDispatcherEvent);
	//向消息派发器注册消息
	virtual HRESULT STDMETHODCALLTYPE RegisterMessage(DWORD adwMessageID, DWORD adwFlag, IMessageEvent * apMessageEvent);
	//向消息派发器解除注册
	virtual HRESULT STDMETHODCALLTYPE UnRegisterMessage(DWORD adwMessageID, IMessageEvent * apMessageEvent);
	//向消息派发器投递消息，消息进入队列，函数立即返回
	virtual HRESULT STDMETHODCALLTYPE PostCRMessage(DWORD adwSenderID, DWORD adwMessageID, void * apData, DWORD adwLen);

public:
	//创建并运行PostMessageThread线程 
	BOOL Run();
	//终止PostMessageThread线程
	BOOL Stop();

	//消息管理器进入只出不入状态，等待队列中所有消完处理完成
	void WaitMsgComplete();

private:
	//类型定义
	typedef std::map< DWORD, STRU_DATA_MESSAGE_ITEM* > MessageItemList;
	typedef std::map< DWORD, STRU_DATA_MESSAGE_ITEM* >::iterator MessageItemListIter;
	typedef std::map< DWORD, STRU_DATA_MESSAGE_ITEM* >::const_iterator MessageItemListConstIter;
	//
	typedef std::list< STRU_DATA_MESSAGE_QUEUE_ITEM* > DMQIList;
	typedef std::list< STRU_DATA_MESSAGE_QUEUE_ITEM* >::iterator DMQIListIter;

private:
	//处理投递消息的线程
	static DWORD WINAPI PostMessageThread(LPVOID * apParam);
	//处理投递消息的线程
	DWORD PostMessageWork();
	//定时器回调函数,真正处理发送消息的函数
	void DealPostMessage();
	//由析构函数调用,释放内存,清理资源
	void Clean();

private:
	//IMessageDispatcherEvent *	m_pMessageDispatcherEvent;

	MessageItemList				m_oMessageItemList;			//消息项列表
	DMQIList					m_oMessageQueueList;		//消息队列表,用于投递方式的消息
	
	HANDLE						m_hEvent;					//事件句柄

	CCriticalSection			m_oCriticalSectionMessage;	//临界区 用于串行化m_oMessageItemList
	CCriticalSection			m_oCriticalSectionQueue;	//临界区 用于串行化m_oMessageQueueList

	BOOL						m_bRunning;					//PostMessage线程运行状态
	HANDLE						m_hThread;					//线程句柄
	long						m_lRefCount;
	BOOL						m_bProhibitPost;			//禁止投递
};

typedef CSingleton<CMessageDispatcher> CSingletonMessageDispatcher;