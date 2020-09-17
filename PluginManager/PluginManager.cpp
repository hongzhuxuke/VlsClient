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

//ע��������¼�
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

		// Ŀ���Ѵ���
		if(pPluginManagerEvent == apPluginManagerEvent)
		{
			return CRE_OBJECT_ALREADY_EXISTS;
		}
	}

	//��ӱ�������
	apPluginManagerEvent->AddRef();
	//����洢����
	m_oCRPluginMgrEventList.push_back(apPluginManagerEvent);

	return CRE_OK;
}

//ȡ��ע��������¼�
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

		// ������һ��
		if(pPluginManagerEvent == apPluginManagerEvent)
		{
			m_oCRPluginMgrEventList.erase(iter);
			apPluginManagerEvent->Release();

			return CRE_OK;
		}
	}

	// Ŀ��û���ҵ�
	return CRE_OBJECT_NOTFOUND;
}

//����һ������������������ appPluginObject ��Ϊ NULL���򷵻ز������ӿ�
HRESULT STDMETHODCALLTYPE CPluginManager::LoadPlugin(const WCHAR * apwzFileName, IPluginObject ** appPluginObject)
{
	if(NULL == apwzFileName || 0 == apwzFileName[0])
	{
		return CRE_INVALIDARG;
	}

	//����dll
	HMODULE hModule = NULL;	
	hModule = LoadLibraryW(apwzFileName);
	int err = GetLastError();
	//����DLLʧ��
	if (NULL == hModule)
	{
		ASSERT(FALSE);
		return CRE_FALSE;
	}

	//���庯������
	typedef HRESULT (WINAPI * TYPE_GetClassObject) (REFIID, void**);

	do
	{
		TYPE_GetClassObject pGetObject = NULL;

		//ȡ�ú�����ַ
		pGetObject = (TYPE_GetClassObject)(GetProcAddress(hModule, "GetClassObject"));

		//�޷�ȡ�ú�����ַ
		if(NULL == pGetObject)
		{
			break;
		}

		CCRPluginObjectPtr ptrCRPluginObjectPtr;

		//ȡ�ö���
		if(CRE_OK != pGetObject(IID_IPluginObject, ptrCRPluginObjectPtr))
		{
			break;
		}

		//�޷�ȡ�ö���
		if(NULL == ptrCRPluginObjectPtr)
		{
			break;
		}

		STRU_CR_PLUGIN_INFO loPluginInfo;

		//ȡ�ò����Ϣ
		if(CRE_OK != ptrCRPluginObjectPtr->GetPluginInfo(loPluginInfo))
		{
			break;
		}

		CCriticalAutoLock loGuard(m_oCriticalPlugin);

		//�����Ƿ����
		MAP_GUID2PluginObject::iterator itor = m_oGUID2PluginObjectMap.find(loPluginInfo.m_oCRPID);

		//�Ѿ�������
		if(itor != m_oGUID2PluginObjectMap.end())
		{
			break;
		}

		STRU_PLUGIN_INFO * pItem = NULL;

		//��������
		pItem = new STRU_PLUGIN_INFO();

		//����ʧ��
		if(NULL == pItem)
		{
			break;
		}

		//��������
		pItem->m_hModule = hModule;
		pItem->m_pCRPluginObjectPtr = ptrCRPluginObjectPtr;

		//��������
		for(WORD i = 0; i < loPluginInfo.m_wCRPDependCount; i ++)
		{
			pItem->m_oDepenList.push_back(loPluginInfo.m_oCRPDepends[i]);
		}

		//����ӳ��
		m_oGUID2PluginObjectMap[loPluginInfo.m_oCRPID] = pItem;

		//���ӵ����������
		ptrCRPluginObjectPtr->ConnectPluginManager(this);

		//���ͼ���֪ͨ
		SendLoadPluginNotfiy(ptrCRPluginObjectPtr);

		//��Ҫ����
		if(appPluginObject)
		{
			ptrCRPluginObjectPtr->AddRef();
			*appPluginObject = ptrCRPluginObjectPtr;
		}

		return CRE_OK;
	}
	while(0);

	//ʧ���ͷſ�
	if(hModule)
	{
		FreeLibrary(hModule);
		hModule = NULL;
	}

	return CRE_FALSE;
}

//ж��һ������ӹ�����
HRESULT STDMETHODCALLTYPE CPluginManager::UnLoadPlugin(REFGUID aoCRPGuid)
{
	CCriticalAutoLock loAutoLock(m_oCriticalPlugin);

	STRU_PLUGIN_INFO * pItem = NULL;
	//���Ҳ������
	MAP_GUID2PluginObject::iterator iter = m_oGUID2PluginObjectMap.find(aoCRPGuid);
	//û���ҵ��������
	if(iter == m_oGUID2PluginObjectMap.end())
	{
		return CRE_FALSE;
	}

	//�ҵ���ȡ�����ݶ���
	pItem = iter->second;

	//���ݶ�����Ч
	if(NULL == pItem)
	{
		return CRE_FALSE;
	}

	//�Ͽ�����������������
	pItem->m_pCRPluginObjectPtr->DisconnectPluginManager();

	//TODO: ����

	//���ͽ���ж���¼�
	SendUnLoadPluginBeforeNotfiy(pItem->m_pCRPluginObjectPtr);

	//�ͷŶ�������
	pItem->m_pCRPluginObjectPtr = NULL;

	//ж�ؿ����
   if (pItem->m_hModule) {
      //FreeLibrary(pItem->m_hModule);
      pItem->m_hModule = NULL;
   }


	//ɾ���ڴ�
	if(pItem)
		delete pItem;
	pItem = NULL;

	//�ӹ�������ɾ��
	m_oGUID2PluginObjectMap.erase(iter);

	//����ж������¼�
	SendUnLoadedPluginNotfiy(aoCRPGuid);

	return CRE_OK;
}

//�Ƿ���Լ���ĳһ�����
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

//ѯ���Ƿ����ж�ز��
HRESULT STDMETHODCALLTYPE CPluginManager::IsCanUnLoadPlugin(REFGUID aoCRPGuid)
{
	VH::CComPtr<IPluginObject> ptrCRPluginObject;
	//ȡ�ò������
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

//��ȡ�������ӿ�
HRESULT STDMETHODCALLTYPE CPluginManager::GetPlugin(REFGUID pluginGuid, IPluginObject ** appPluginObject)
{
	if(appPluginObject == NULL)
		return  CRE_INVALIDARG;

	CCriticalAutoLock loGuard(m_oCriticalPlugin);
	
	MAP_GUID2PluginObject::iterator iter = m_oGUID2PluginObjectMap.find(pluginGuid);
	//������
	if(iter == m_oGUID2PluginObjectMap.end())
		return CRE_FALSE;
	
	STRU_PLUGIN_INFO * pInfo = iter->second;

	ASSERT(pInfo);

	if(NULL == pInfo->m_pCRPluginObjectPtr)
		return CRE_FALSE;

	//���ؽӿڲ�ѯ
	return pInfo->m_pCRPluginObjectPtr->QueryInterface(IID_IPluginObject, (void**)appPluginObject);
}

//���ز��֪ͨ
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

//����ж��ǰ�¼�
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

//����ж������¼�
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