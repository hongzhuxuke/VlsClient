#pragma once
#include <list>
#include <map>
#include <set>
#include "IMessageDispatcher.h"
#include "MemPoolBase.h"
#include "Singleton.h"

//��Ϣ��
struct STRU_DATA_MESSAGE_QUEUE_ITEM : public CMemPoolBase
{
	DWORD	m_dwSenderID;	//�����ߵ�ID
	DWORD	m_dwMessageID;	//��ϢID
	void *	m_pData;		//����
	ULONG	m_uDataLength;	//���ݳ�
public:
	BOOL SetData(const void * apData, DWORD adwLen);
public:
	STRU_DATA_MESSAGE_QUEUE_ITEM();
	~STRU_DATA_MESSAGE_QUEUE_ITEM();
};

//��������
typedef std::set< IMessageEvent* >	EventDict;

//��Ϣ������,����PostMessage
struct STRU_DATA_MESSAGE_ITEM : public CMemPoolBase
{
	DWORD		m_dwMessageID;	//��ϢID
	EventDict	m_oEventDict;	//�¼��б�
public:
	STRU_DATA_MESSAGE_ITEM();
};

class CMessageDispatcher : public IMessageDispatcher
{
public:
	//���캯��/��������
	CMessageDispatcher();
	~CMessageDispatcher();

public:
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void ** ppvObject);
	virtual ULONG STDMETHODCALLTYPE AddRef(void);
	virtual ULONG STDMETHODCALLTYPE Release(void);

public:
	////���÷ַ����¼�
	//virtual HRESULT STDMETHODCALLTYPE SetMessageDispatcherEvent(IMessageDispatcherEvent * apMessageDispatcherEvent);
	////ȡ�÷ַ����¼�
	//virtual HRESULT STDMETHODCALLTYPE GetMessageDispatcherEvent(IMessageDispatcherEvent ** appMessageDispatcherEvent);
	//����Ϣ�ɷ���ע����Ϣ
	virtual HRESULT STDMETHODCALLTYPE RegisterMessage(DWORD adwMessageID, DWORD adwFlag, IMessageEvent * apMessageEvent);
	//����Ϣ�ɷ������ע��
	virtual HRESULT STDMETHODCALLTYPE UnRegisterMessage(DWORD adwMessageID, IMessageEvent * apMessageEvent);
	//����Ϣ�ɷ���Ͷ����Ϣ����Ϣ������У�������������
	virtual HRESULT STDMETHODCALLTYPE PostCRMessage(DWORD adwSenderID, DWORD adwMessageID, void * apData, DWORD adwLen);

public:
	//����������PostMessageThread�߳� 
	BOOL Run();
	//��ֹPostMessageThread�߳�
	BOOL Stop();

	//��Ϣ����������ֻ������״̬���ȴ��������������괦�����
	void WaitMsgComplete();

private:
	//���Ͷ���
	typedef std::map< DWORD, STRU_DATA_MESSAGE_ITEM* > MessageItemList;
	typedef std::map< DWORD, STRU_DATA_MESSAGE_ITEM* >::iterator MessageItemListIter;
	typedef std::map< DWORD, STRU_DATA_MESSAGE_ITEM* >::const_iterator MessageItemListConstIter;
	//
	typedef std::list< STRU_DATA_MESSAGE_QUEUE_ITEM* > DMQIList;
	typedef std::list< STRU_DATA_MESSAGE_QUEUE_ITEM* >::iterator DMQIListIter;

private:
	//����Ͷ����Ϣ���߳�
	static DWORD WINAPI PostMessageThread(LPVOID * apParam);
	//����Ͷ����Ϣ���߳�
	DWORD PostMessageWork();
	//��ʱ���ص�����,������������Ϣ�ĺ���
	void DealPostMessage();
	//��������������,�ͷ��ڴ�,������Դ
	void Clean();

private:
	//IMessageDispatcherEvent *	m_pMessageDispatcherEvent;

	MessageItemList				m_oMessageItemList;			//��Ϣ���б�
	DMQIList					m_oMessageQueueList;		//��Ϣ���б�,����Ͷ�ݷ�ʽ����Ϣ
	
	HANDLE						m_hEvent;					//�¼����

	CCriticalSection			m_oCriticalSectionMessage;	//�ٽ��� ���ڴ��л�m_oMessageItemList
	CCriticalSection			m_oCriticalSectionQueue;	//�ٽ��� ���ڴ��л�m_oMessageQueueList

	BOOL						m_bRunning;					//PostMessage�߳�����״̬
	HANDLE						m_hThread;					//�߳̾��
	long						m_lRefCount;
	BOOL						m_bProhibitPost;			//��ֹͶ��
};

typedef CSingleton<CMessageDispatcher> CSingletonMessageDispatcher;