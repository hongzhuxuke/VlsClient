#include "StdAfx.h"
#include "PluginManager.h"

CPluginManager::STRU_PLUGIN_INFO::STRU_PLUGIN_INFO()
{
	m_hModule = NULL;
	m_pCRPluginObjectPtr = NULL;
}

CPluginManager::STRU_PLUGIN_INFO::STRU_PLUGIN_INFO(const CPluginManager::STRU_PLUGIN_INFO& aoSrc)
{
	m_hModule = aoSrc.m_hModule;
	m_pCRPluginObjectPtr = aoSrc.m_pCRPluginObjectPtr;
	m_oDepenList.assign(aoSrc.m_oDepenList.begin(), aoSrc.m_oDepenList.end());
}

CPluginManager::STRU_PLUGIN_INFO::~STRU_PLUGIN_INFO()
{
	ASSERT(NULL == m_hModule);
	m_pCRPluginObjectPtr = NULL;
	m_oDepenList.clear();
}

CPluginManager::STRU_PLUGIN_INFO& CPluginManager::STRU_PLUGIN_INFO::operator = (const STRU_PLUGIN_INFO& aoSrc)
{
	if(this != &aoSrc)
	{
		m_hModule = aoSrc.m_hModule;
		m_pCRPluginObjectPtr = aoSrc.m_pCRPluginObjectPtr;
		m_oDepenList.assign(aoSrc.m_oDepenList.begin(), aoSrc.m_oDepenList.end());
	}
	return (*this);
}


CPluginManager::CPluginManager(void)
{
	m_lRefCount = 0;
}

CPluginManager::~CPluginManager(void)
{
	m_oCriticalPluginEvent.Lock();
	if(m_oCRPluginMgrEventList.size() > 0)
	{
		LIST_CRPluginMgrEvent::iterator iter = m_oCRPluginMgrEventList.begin();
		for(; m_oCRPluginMgrEventList.end() != iter; iter++)
		{
         IPluginManagerEvent * pPlugInEvent = (*iter);
			if(pPlugInEvent)
				pPlugInEvent->Release();
		}
		m_oCRPluginMgrEventList.clear();
	}
	m_oCriticalPluginEvent.UnLock();

	m_oCriticalPlugin.Lock();
	if(m_oGUID2PluginObjectMap.size() > 0)
	{
		MAP_GUID2PluginObject::iterator iter = m_oGUID2PluginObjectMap.begin();
		if(iter != m_oGUID2PluginObjectMap.end())
		{
			STRU_PLUGIN_INFO * pInfo = iter->second;
			if(pInfo)
				delete pInfo;
		}
		m_oGUID2PluginObjectMap.clear();
	}
	m_oCriticalPlugin.UnLock();
}

HRESULT STDMETHODCALLTYPE CPluginManager::QueryInterface(REFIID riid, void ** appvObject)
{
	if(NULL == appvObject)
		return CRE_INVALIDARG;
	
	if(riid == IID_VHIUnknown)
	{
		*appvObject = (VH_IUnknown*)this;
		AddRef();
		return CRE_OK;
	}
	else if(riid == IID_IPluginManager)
	{
		*appvObject = (IPluginManager*)this;
		AddRef();
		return CRE_OK;
	}

	return CRE_NOINTERFACE;
}

ULONG STDMETHODCALLTYPE CPluginManager::AddRef(void)
{
	::InterlockedIncrement(&m_lRefCount);
	return m_lRefCount;
}

ULONG CPluginManager::Release(void)
{
	::InterlockedDecrement(&m_lRefCount);
	return m_lRefCount;
}

//注册管理器事件
HRESULT STDMETHODCALLTYPE CPluginManager::RegisterEvent(IPluginManagerEvent * apPluginManagerEvent)
{
	if(NULL == apPluginManagerEvent)
		return CRE_INVALIDARG; 

	CCriticalAutoLock loGuard(m_oCriticalPluginEvent);

	LIST_CRPluginMgrEvent::iterator iter = m_oCRPluginMgrEventList.begin();
	for(; m_oCRPluginMgrEventList.end() != iter; ++ iter)
	{
      IPluginManagerEvent * pPluginManagerEvent = (*iter);

		ASSERT(pPluginManagerEvent);

		// 目标已存在
		if(pPluginManagerEvent == apPluginManagerEvent)
		{
			return CRE_OBJECT_ALREADY_EXISTS;
		}
	}

	//添加保存引用
	apPluginManagerEvent->AddRef();
	//放入存储队列
	m_oCRPluginMgrEventList.push_back(apPluginManagerEvent);

	return CRE_OK;
}

//取消注册管理器事件
HRESULT STDMETHODCALLTYPE CPluginManager::UnRegisterEvent(IPluginManagerEvent * apPluginManagerEvent)
{
	if(NULL == apPluginManagerEvent)
		return CRE_INVALIDARG;

	CCriticalAutoLock loGuard(m_oCriticalPluginEvent);

	LIST_CRPluginMgrEvent::iterator iter = m_oCRPluginMgrEventList.begin();
	for(; m_oCRPluginMgrEventList.end() != iter; ++ iter)
	{
      IPluginManagerEvent * pPluginManagerEvent = (*iter);

		ASSERT(pPluginManagerEvent);

		// 继续下一个
		if(pPluginManagerEvent == apPluginManagerEvent)
		{
			m_oCRPluginMgrEventList.erase(iter);
			apPluginManagerEvent->Release();

			return CRE_OK;
		}
	}

	// 目标没有找到
	return CRE_OBJECT_NOTFOUND;
}

//加载一个插件到管理器，如果 appPluginObject 不为 NULL，则返回插件对象接口
HRESULT STDMETHODCALLTYPE CPluginManager::LoadPlugin(const WCHAR * apwzFileName, IPluginObject ** appPluginObject)
{
	if(NULL == apwzFileName || 0 == apwzFileName[0])
	{
		return CRE_INVALIDARG;
	}

	//载入dll
	HMODULE hModule = NULL;	
	hModule = LoadLibraryW(apwzFileName);
	int err = GetLastError();
	//加载DLL失败
	if (NULL == hModule)
	{
		ASSERT(FALSE);
		return CRE_FALSE;
	}

	//定义函数类型
	typedef HRESULT (WINAPI * TYPE_GetClassObject) (REFIID, void**);

	do
	{
		TYPE_GetClassObject pGetObject = NULL;

		//取得函数地址
		pGetObject = (TYPE_GetClassObject)(GetProcAddress(hModule, "GetClassObject"));

		//无法取得函数地址
		if(NULL == pGetObject)
		{
			break;
		}

		CCRPluginObjectPtr ptrCRPluginObjectPtr;

		//取得对象
		if(CRE_OK != pGetObject(IID_IPluginObject, ptrCRPluginObjectPtr))
		{
			break;
		}

		//无法取得对象
		if(NULL == ptrCRPluginObjectPtr)
		{
			break;
		}

		STRU_CR_PLUGIN_INFO loPluginInfo;

		//取得插件信息
		if(CRE_OK != ptrCRPluginObjectPtr->GetPluginInfo(loPluginInfo))
		{
			break;
		}

		CCriticalAutoLock loGuard(m_oCriticalPlugin);

		//查找是否存在
		MAP_GUID2PluginObject::iterator itor = m_oGUID2PluginObjectMap.find(loPluginInfo.m_oCRPID);

		//已经存在了
		if(itor != m_oGUID2PluginObjectMap.end())
		{
			break;
		}

		STRU_PLUGIN_INFO * pItem = NULL;

		//分配新项
		pItem = new STRU_PLUGIN_INFO();

		//分配失败
		if(NULL == pItem)
		{
			break;
		}

		//保存数据
		pItem->m_hModule = hModule;
		pItem->m_pCRPluginObjectPtr = ptrCRPluginObjectPtr;

		//保存依赖
		for(WORD i = 0; i < loPluginInfo.m_wCRPDependCount; i ++)
		{
			pItem->m_oDepenList.push_back(loPluginInfo.m_oCRPDepends[i]);
		}

		//保存映射
		m_oGUID2PluginObjectMap[loPluginInfo.m_oCRPID] = pItem;

		//连接到插件管理器
		ptrCRPluginObjectPtr->ConnectPluginManager(this);

		//发送加载通知
		SendLoadPluginNotfiy(ptrCRPluginObjectPtr);

		//需要传出
		if(appPluginObject)
		{
			ptrCRPluginObjectPtr->AddRef();
			*appPluginObject = ptrCRPluginObjectPtr;
		}

		return CRE_OK;
	}
	while(0);

	//失败释放库
	if(hModule)
	{
		FreeLibrary(hModule);
		hModule = NULL;
	}

	return CRE_FALSE;
}

//卸载一个插件从管理器
HRESULT STDMETHODCALLTYPE CPluginManager::UnLoadPlugin(REFGUID aoCRPGuid)
{
	CCriticalAutoLock loAutoLock(m_oCriticalPlugin);

	STRU_PLUGIN_INFO * pItem = NULL;
	//查找插件对象
	MAP_GUID2PluginObject::iterator iter = m_oGUID2PluginObjectMap.find(aoCRPGuid);
	//没有找到插件对象
	if(iter == m_oGUID2PluginObjectMap.end())
	{
		return CRE_FALSE;
	}

	//找到，取得数据对象
	pItem = iter->second;

	//数据对象无效
	if(NULL == pItem)
	{
		return CRE_FALSE;
	}

	//断开与插件管理器的连接
	pItem->m_pCRPluginObjectPtr->DisconnectPluginManager();

	//TODO: 依赖

	//发送将被卸载事件
	SendUnLoadPluginBeforeNotfiy(pItem->m_pCRPluginObjectPtr);

	//释放对象引用
	pItem->m_pCRPluginObjectPtr = NULL;

	//卸载库对象
   if (pItem->m_hModule) {
      //FreeLibrary(pItem->m_hModule);
      pItem->m_hModule = NULL;
   }


	//删除内存
	if(pItem)
		delete pItem;
	pItem = NULL;

	//从管理器中删除
	m_oGUID2PluginObjectMap.erase(iter);

	//发送卸载完成事件
	SendUnLoadedPluginNotfiy(aoCRPGuid);

	return CRE_OK;
}

//是否可以加载某一个插件
HRESULT STDMETHODCALLTYPE CPluginManager::IsCanLoadPlugin(REFGUID aoCRPGuid)
{
	CCriticalAutoLock loAutoLock(m_oCriticalPluginEvent);

	if(m_oCRPluginMgrEventList.size() < 1)
		return CRE_OK;
	
	LIST_CRPluginMgrEvent::iterator iter = m_oCRPluginMgrEventList.begin();
	for(; m_oCRPluginMgrEventList.end() != iter; iter++)
	{
      IPluginManagerEvent * pPlugInEvent = (*iter);
		if(NULL == pPlugInEvent)
		{
			continue;
		}
		if(CRE_OK == pPlugInEvent->IsCanLoadPlugin(aoCRPGuid))
		{
			continue;
		}
		return CRE_FALSE;
	}

	return CRE_OK;
}

//询问是否可以卸载插件
HRESULT STDMETHODCALLTYPE CPluginManager::IsCanUnLoadPlugin(REFGUID aoCRPGuid)
{
	VH::CComPtr<IPluginObject> ptrCRPluginObject;
	//取得插件对象
	if(CRE_OK != GetPlugin(aoCRPGuid, ptrCRPluginObject))
		return CRE_FALSE;

	CCriticalAutoLock loAutoLock(m_oCriticalPluginEvent);

	if(m_oCRPluginMgrEventList.size() < 1)
		return CRE_OK;
	
	LIST_CRPluginMgrEvent::iterator iter = m_oCRPluginMgrEventList.begin();
	for(; m_oCRPluginMgrEventList.end() != iter; iter++)
	{
      IPluginManagerEvent * pPlugInEvent = (*iter);
		if(NULL == pPlugInEvent)
		{
			continue;
		}
		if(CRE_OK == pPlugInEvent->IsCanUnLoadPlugin(ptrCRPluginObject))
		{
			continue;
		}
		return CRE_FALSE;
	}

	return CRE_OK;
}

//获取插件对象接口
HRESULT STDMETHODCALLTYPE CPluginManager::GetPlugin(REFGUID pluginGuid, IPluginObject ** appPluginObject)
{
	if(appPluginObject == NULL)
		return  CRE_INVALIDARG;

	CCriticalAutoLock loGuard(m_oCriticalPlugin);
	
	MAP_GUID2PluginObject::iterator iter = m_oGUID2PluginObjectMap.find(pluginGuid);
	//不存在
	if(iter == m_oGUID2PluginObjectMap.end())
		return CRE_FALSE;
	
	STRU_PLUGIN_INFO * pInfo = iter->second;

	ASSERT(pInfo);

	if(NULL == pInfo->m_pCRPluginObjectPtr)
		return CRE_FALSE;

	//返回接口查询
	return pInfo->m_pCRPluginObjectPtr->QueryInterface(IID_IPluginObject, (void**)appPluginObject);
}

//加载插件通知
BOOL CPluginManager::SendLoadPluginNotfiy(IPluginObject * apPluginObject)
{
	if(NULL == apPluginObject)
		return FALSE;

	CCriticalAutoLock loAutoLock(m_oCriticalPluginEvent);

	if(m_oCRPluginMgrEventList.size() < 1)
		return TRUE;
	
	LIST_CRPluginMgrEvent::iterator iter = m_oCRPluginMgrEventList.begin();
	for(; m_oCRPluginMgrEventList.end() != iter; iter++)
	{
      IPluginManagerEvent * pPlugInEvent = (*iter);
		if(pPlugInEvent)
			pPlugInEvent->OnPluginLoaded(apPluginObject);
	}

	return TRUE;
}

//发送卸载前事件
BOOL CPluginManager::SendUnLoadPluginBeforeNotfiy(IPluginObject * apPluginObject)
{
	if(NULL == apPluginObject)
		return FALSE;

	CCriticalAutoLock loAutoLock(m_oCriticalPluginEvent);

	if(m_oCRPluginMgrEventList.size() < 1)
		return TRUE;
	
	LIST_CRPluginMgrEvent::iterator iter = m_oCRPluginMgrEventList.begin();
	for(; m_oCRPluginMgrEventList.end() != iter; iter++)
	{
      IPluginManagerEvent * pPlugInEvent = (*iter);
		if(pPlugInEvent)
			pPlugInEvent->OnPluginUnLoadBefore(apPluginObject);
	}

	return TRUE;
}

//发送卸载完成事件
BOOL CPluginManager::SendUnLoadedPluginNotfiy(REFGUID aoCRPGuid)
{
	CCriticalAutoLock loAutoLock(m_oCriticalPluginEvent);

	if(m_oCRPluginMgrEventList.size() < 1)
		return TRUE;
	
	LIST_CRPluginMgrEvent::iterator iter = m_oCRPluginMgrEventList.begin();
	for(; m_oCRPluginMgrEventList.end() != iter; iter++)
	{
      IPluginManagerEvent * pPlugInEvent = (*iter);
		if(pPlugInEvent)
			pPlugInEvent->OnPluginUnLoaded(aoCRPGuid);
	}

	return TRUE;
}