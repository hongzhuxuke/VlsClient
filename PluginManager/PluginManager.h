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

	//插件对象信息
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
	//注册管理器事件
   virtual HRESULT STDMETHODCALLTYPE RegisterEvent(IPluginManagerEvent * apPluginManagerEvent);
	//取消注册管理器事件
   virtual HRESULT STDMETHODCALLTYPE UnRegisterEvent(IPluginManagerEvent * apPluginManagerEvent);
	//加载一个插件到管理器，如果 appPluginObject 不为 NULL，则返回插件对象接口
	virtual HRESULT STDMETHODCALLTYPE LoadPlugin(const WCHAR * apwzFileName, IPluginObject ** appPluginObject);
	//卸载一个插件从管理器
	virtual HRESULT STDMETHODCALLTYPE UnLoadPlugin(REFGUID aoCRPGuid);
	//是否可以加载某一个插件
	virtual HRESULT STDMETHODCALLTYPE IsCanLoadPlugin(REFGUID aoCRPGuid);
	//询问是否可以卸载插件
	virtual HRESULT STDMETHODCALLTYPE IsCanUnLoadPlugin(REFGUID aoCRPGuid);
	//获取插件对象接口
	virtual HRESULT STDMETHODCALLTYPE GetPlugin(REFGUID pluginGuid, IPluginObject ** appPluginObject);

private:
	//发送加事件
	BOOL SendLoadPluginNotfiy(IPluginObject * apPluginObject);
	//发送卸载前事件
	BOOL SendUnLoadPluginBeforeNotfiy(IPluginObject * apPluginObject);
	//发送卸载完成事件
	BOOL SendUnLoadedPluginNotfiy(REFGUID aoCRPGuid);

private:
	long						m_lRefCount;				//引用计数器
	LIST_CRPluginMgrEvent		m_oCRPluginMgrEventList;	//插件事件侦听器队列
	MAP_GUID2PluginObject		m_oGUID2PluginObjectMap;	// GUID - 插件对象映射表

	CCriticalSection			m_oCriticalPlugin;
	CCriticalSection			m_oCriticalPluginEvent;
};

typedef CSingleton<CPluginManager> CSingletonCRPluginManager;