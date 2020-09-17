#pragma once

#include <set>

#include "IPluginManager.h"

#include "IUIThreadManager.h"

//#include "CRMessageWnd.h"

#include "Singleton.h"


#define DEF_WM_CHECK_BEGIN_TASK					(WM_USER + 0xFF01)
#define DEF_WM_CHECK_END_TASK					(WM_USER + 0xFF02)
#define DEF_WM_REG_MSG_EVENT						(WM_USER + 0xFF03)
#define DEF_WM_UNREG_MSG_EVENT					(WM_USER + 0xFF04)


typedef std::set<IUIThreadObject*>			PluginUIThreadDict;
typedef std::set<IUIThreadMessageEvent*>		PluginUIThreadMsgEventDict;
typedef std::set<IUIPreMessageEvent*>		PluginUIPreMsgEventDict;

class IQueryInterface;

class CRuntimeUIThreadManager : public IUIThreadManager
{
public:
	CRuntimeUIThreadManager(void);
	~CRuntimeUIThreadManager(void);

public:
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void ** ppvObject);

	virtual ULONG STDMETHODCALLTYPE AddRef(void);

	virtual ULONG STDMETHODCALLTYPE Release(void);

public:
	void Init(DWORD adwUIThreadID);

	void UnInit();

public:
	// ����ָ����UI�߳���������������Ŀ���߳�ID,0��ʾʧ��
	virtual DWORD STDMETHODCALLTYPE Begin(DWORD dwThreadIndex, IUIThreadObject* apMUIThreadObject);

	// ����ָ����UI�߳̽�������
	virtual HRESULT STDMETHODCALLTYPE End(DWORD dwThreadIndex, IUIThreadObject* apMUIThreadObject);

	// ��ָ����UI�߳�ע��UI��Ϣ��
	virtual HRESULT STDMETHODCALLTYPE RegisterUIMessageEvent(DWORD dwThreadIndex, IUIThreadMessageEvent * apEvent);

	// ��ָ����UI�߳�ȡ��ע��UI��Ϣ�ص�
	virtual HRESULT STDMETHODCALLTYPE UnRegisterUIMessageEvent(DWORD dwThreadIndex, IUIThreadMessageEvent * pEvent);

	// ��ָ����UI�߳�ע��UIԤ������Ϣ�ص�
	virtual HRESULT STDMETHODCALLTYPE RegisterUIPreMessageEvent(IUIPreMessageEvent * apEvent);

	// ��ָ����UI�߳�ȡ��UIԤ������Ϣ�ص�
	virtual HRESULT STDMETHODCALLTYPE UnRegisterUIPreMessageEvent(IUIPreMessageEvent * apEvent);

public:
	// �����߳���Ϣ
	BOOL OnPreTranslateMessage(MSG* apMsg);

	// ʱ�ӻص��㲥������Ϣ
	BOOL OnTimerBroadcastDealMessage();

	// ��鿪ʼ����
	BOOL OnMessageCheckBeginTask(WPARAM wparam, LPARAM lparam);

	// ����������
	BOOL OnMessageCheckEndTask(WPARAM wparam, LPARAM lparam);

	// ע����Ϣ�¼�
	BOOL OnMessageRegisterMsgEvent(WPARAM wparam, LPARAM lparam);

	// �Ƴ���Ϣ�¼�
	BOOL OnMessageUnRegisterMsgEvent(WPARAM wparam, LPARAM lparam);

	// ע��UIԤ�������
	BOOL OnMessageRegisterMsgObject(WPARAM wparam, LPARAM lparam);

	// �Ƴ�UIԤ�������
	BOOL OnMessageUnRegisterMsgObject(WPARAM wparam, LPARAM lparam);

public:
	// ��ȡĳ�������UI�̶߳���ӿ�
	BOOL GetPluginUIThread(IPluginObject* apPluginObject, IUIThreadObject** appPluginUIThread);

public:
	//----------------------------------------------------------------------------------------------------------------
	// IMPluginManagerEvent�ӿ�
	BEGIN_INTERFACE_PART(PluginManagerEvent, IPluginManagerEvent);
		INIT_INTERFACE_PART(CRuntimeUIThreadManager, PluginManagerEvent);

		// ֪ͨĳһ����������ص���������
		virtual HRESULT STDMETHODCALLTYPE OnPluginLoaded(IPluginObject* apPluginObject);

		// ֪ͨĳһ������ӹ�����ж��֮ǰ
		virtual HRESULT STDMETHODCALLTYPE OnPluginUnLoadBefore(IPluginObject* apPluginObject);

		// ֪ͨĳһ������Ѵӹ�����ж��
		virtual HRESULT STDMETHODCALLTYPE OnPluginUnLoaded(REFGUID aoCRPGuid);

		// �Ƿ���Լ���ĳһ�����
		virtual HRESULT STDMETHODCALLTYPE IsCanLoadPlugin(REFGUID aoCRPGuid);

		// �Ƿ����ж��
		virtual HRESULT STDMETHODCALLTYPE IsCanUnLoadPlugin(IPluginObject* apPluginObject);

	END_INTERFACE_PART(PluginManagerEvent);
	//----------------------------------------------------------------------------------------------------------------

private:
    //CCRMessageWnd* m_pMessageWnd;

	long m_lRefCount;

	DWORD m_dwUIThreadID;

	PluginUIThreadDict m_oPluginUIThreadDict;
	PluginUIThreadMsgEventDict m_oPluginUIThreadMsgEventDict;
	PluginUIPreMsgEventDict m_oPluginUIPreMsgEventDict;
};

typedef CSingleton<CRuntimeUIThreadManager> CSingletonMainUIThreadMgr;
