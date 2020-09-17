#pragma once
#include <list>
#include <map>
#include <vector>
#include "IPluginManager.h"

#include "Singleton.h"

inline bool operator < (const GUID & aoGuid1, const GUID & aoGuid2)
{
	return static_cast<bool>(memcmp(&aoGuid1, &aoGuid2, sizeof(GUID)) < 0);
}

class CPluginManager : public IPluginManager
{
	typedef VH::CComPtr<IPluginObject>	CCRPluginObjectPtr;
	typedef std::vector<GUID>				GUID_List;

	//���������Ϣ
	struct STRU_PLUGIN_INFO
	{
		HMODULE					m_hModule;
		CCRPluginObjectPtr		m_pCRPluginObjectPtr;
		GUID_List				m_oDepenList;
	public:
		STRU_PLUGIN_INFO();
		STRU_PLUGIN_INFO(const STRU_PLUGIN_INFO& aoSrc);
		~STRU_PLUGIN_INFO();
	public:
		STRU_PLUGIN_INFO& operator = (const STRU_PLUGIN_INFO& aoSrc);
	};

	typedef std::list<IPluginManagerEvent*>		LIST_CRPluginMgrEvent;
	typedef std::map<GUID, STRU_PLUGIN_INFO*>		MAP_GUID2PluginObject;
	typedef std::pair<GUID, STRU_PLUGIN_INFO*>		PAIR_GUID2PluginObject;

	typedef std::map<GUID, BOOL>					MAP_GUID2BOOL;
	typedef std::pair<GUID, BOOL>					PAIR_GUID2BOOL;

public:
	CPluginManager(void);
	~CPluginManager(void);

public:
	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void ** appvObject);
	ULONG STDMETHODCALLTYPE AddRef(void);
	ULONG STDMETHODCALLTYPE Release(void);

public:
	//ע��������¼�
   virtual HRESULT STDMETHODCALLTYPE RegisterEvent(IPluginManagerEvent * apPluginManagerEvent);
	//ȡ��ע��������¼�
   virtual HRESULT STDMETHODCALLTYPE UnRegisterEvent(IPluginManagerEvent * apPluginManagerEvent);
	//����һ������������������ appPluginObject ��Ϊ NULL���򷵻ز������ӿ�
	virtual HRESULT STDMETHODCALLTYPE LoadPlugin(const WCHAR * apwzFileName, IPluginObject ** appPluginObject);
	//ж��һ������ӹ�����
	virtual HRESULT STDMETHODCALLTYPE UnLoadPlugin(REFGUID aoCRPGuid);
	//�Ƿ���Լ���ĳһ�����
	virtual HRESULT STDMETHODCALLTYPE IsCanLoadPlugin(REFGUID aoCRPGuid);
	//ѯ���Ƿ����ж�ز��
	virtual HRESULT STDMETHODCALLTYPE IsCanUnLoadPlugin(REFGUID aoCRPGuid);
	//��ȡ�������ӿ�
	virtual HRESULT STDMETHODCALLTYPE GetPlugin(REFGUID pluginGuid, IPluginObject ** appPluginObject);

private:
	//���ͼ��¼�
	BOOL SendLoadPluginNotfiy(IPluginObject * apPluginObject);
	//����ж��ǰ�¼�
	BOOL SendUnLoadPluginBeforeNotfiy(IPluginObject * apPluginObject);
	//����ж������¼�
	BOOL SendUnLoadedPluginNotfiy(REFGUID aoCRPGuid);

private:
	long						m_lRefCount;				//���ü�����
	LIST_CRPluginMgrEvent		m_oCRPluginMgrEventList;	//����¼�����������
	MAP_GUID2PluginObject		m_oGUID2PluginObjectMap;	// GUID - �������ӳ���

	CCriticalSection			m_oCriticalPlugin;
	CCriticalSection			m_oCriticalPluginEvent;
};

typedef CSingleton<CPluginManager> CSingletonCRPluginManager;