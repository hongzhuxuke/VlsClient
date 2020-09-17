#include "stdafx.h"
#include "RuntimeUIThreadManager.h"


CRuntimeUIThreadManager::CRuntimeUIThreadManager(void)
: m_lRefCount(0)
, m_dwUIThreadID(0) {

}

CRuntimeUIThreadManager::~CRuntimeUIThreadManager(void) {

}

//��ȡĳ�������UI�̶߳���ӿ�
BOOL CRuntimeUIThreadManager::GetPluginUIThread(IPluginObject * apPluginObject, IUIThreadObject ** appPluginUIThread) {
   if (NULL == appPluginUIThread) {
      ASSERT(FALSE);
      return FALSE;
   }

   if (CRE_OK != apPluginObject->QueryInterface(IID_IUIThreadObject, (void**)appPluginUIThread)) {
      ASSERT(FALSE);
      return FALSE;
   }

   if (NULL == *appPluginUIThread) {
      ASSERT(FALSE);
      return FALSE;
   }

   return TRUE;
}

HRESULT STDMETHODCALLTYPE CRuntimeUIThreadManager::QueryInterface(REFIID riid, void **ppvObject) {
   if (NULL == ppvObject) {
      ASSERT(FALSE);
      return CRE_INVALIDARG;
   }

   if (IsEqualGUID(IID_VHIUnknown, riid)) {
      AddRef();
      *ppvObject = reinterpret_cast<VH_IUnknown*>(this);

      return CRE_OK;
   } else if (IsEqualGUID(IID_IUIThreadManager, riid)) {
      AddRef();
      *ppvObject = reinterpret_cast<IUIThreadManager*>(this);

      return CRE_OK;
   } else if (IsEqualGUID(IID_IPluginManagerEvent, riid)) {
      AddRef();
      *ppvObject = reinterpret_cast<IPluginManagerEvent*>(&m_xPluginManagerEvent);

      return CRE_OK;
   }

   return CRE_NOINTERFACE;
}

ULONG STDMETHODCALLTYPE CRuntimeUIThreadManager::AddRef(void) {
   return ::InterlockedIncrement(&m_lRefCount);
}

ULONG STDMETHODCALLTYPE CRuntimeUIThreadManager::Release(void) {
   return ::InterlockedDecrement(&m_lRefCount);
}

void CRuntimeUIThreadManager::Init(DWORD adwUIThreadID) {
   m_dwUIThreadID = adwUIThreadID;

   do {
      VH::CComPtr<IPluginManager> ptrMPluginManager;

      // ȡ�ò��������
      if (CRE_OK != GetPluginManager(ptrMPluginManager)) {
         TRACE6("%s GetPluginManager err\n", __FUNCTION__);
         ASSERT(FALSE);
         break;
      }
      TRACE6("%s GetPluginManager ok\n",__FUNCTION__);
      // ����ȡ�ò���������ӿ�
      if (NULL == ptrMPluginManager) {
         ASSERT(FALSE);
         break;
      }

      VH::CComPtr<IPluginManagerEvent> ptrPluginManagerEvent;

      // ȡ���̹߳�����ʵ�ֵĲ���������¼�
      if (CRE_OK != QueryInterface(IID_IPluginManagerEvent, ptrPluginManagerEvent)) {
         TRACE6("%s QueryInterface ok\n", __FUNCTION__);
         ASSERT(FALSE);
         break;
      }
      TRACE6("%s QueryInterface ok\n", __FUNCTION__);
      // ����ȡ���¼��ӿ�
      if (NULL == ptrPluginManagerEvent) {
         TRACE6("%s NULL == ptrPluginManagerEven err\n", __FUNCTION__);
         ASSERT(FALSE);
         break;
      }
      
      // ע�����������¼�
      if (CRE_OK != ptrMPluginManager->RegisterEvent(ptrPluginManagerEvent)) {
         TRACE6("%s RegisterEvent err\n", __FUNCTION__);
         ASSERT(FALSE);
         break;
      }
      TRACE6("%s RegisterEvent\n", __FUNCTION__);
   } while (0);
}

void CRuntimeUIThreadManager::UnInit() {
   do {
      VH::CComPtr<IPluginManager> ptrMPluginManager;

      // ȡ�ò��������
      if (CRE_OK != GetPluginManager(ptrMPluginManager)) {
         ASSERT(FALSE);
         break;
      }

      // ����ȡ�ò���������ӿ�
      if (NULL == ptrMPluginManager) {
         ASSERT(FALSE);
         break;
      }

      VH::CComPtr<IPluginManagerEvent> ptrPluginManagerEvent;

      // ȡ���̹߳�����ʵ�ֵĲ���������¼�
      if (CRE_OK != QueryInterface(IID_IPluginManagerEvent, ptrPluginManagerEvent)) {
         ASSERT(FALSE);
         break;
      }

      // ����ȡ���¼��ӿ�
      if (NULL == ptrPluginManagerEvent) {
         ASSERT(FALSE);
         break;
      }

      // ��ע�����������¼�
      if (CRE_OK != ptrMPluginManager->UnRegisterEvent(ptrPluginManagerEvent)) {
         ASSERT(FALSE);
         break;
      }
   } while (0);

   m_dwUIThreadID = 0;
}

// ����ָ����UI�߳���������������Ŀ���߳�ID,0��ʾʧ��
DWORD STDMETHODCALLTYPE CRuntimeUIThreadManager::Begin(DWORD dwThreadIndex, IUIThreadObject * apMUIThreadObject) {
   if (NULL == apMUIThreadObject) {
      ASSERT(FALSE);
      return -1;
   }

   PluginUIThreadDict::iterator iter = m_oPluginUIThreadDict.find(apMUIThreadObject);

   if (iter == m_oPluginUIThreadDict.end()) {
      // ���ö����ʼ��
      apMUIThreadObject->Init(m_dwUIThreadID);

      // �������ü���
      apMUIThreadObject->AddRef();

      // ���浽�б�
      m_oPluginUIThreadDict.insert(apMUIThreadObject);
   } else {
      return -1;
   }

   return m_dwUIThreadID;
}

// ����ָ����UI�߳̽�������
HRESULT STDMETHODCALLTYPE CRuntimeUIThreadManager::End(DWORD dwThreadIndex, IUIThreadObject * apMUIThreadObject) {
   if (NULL == apMUIThreadObject) {
      ASSERT(FALSE);
      return CRE_FALSE;
   }

   PluginUIThreadDict::iterator iter = m_oPluginUIThreadDict.find(apMUIThreadObject);

   if (iter == m_oPluginUIThreadDict.end()) {
      return FALSE;
   } else {
      // ���ö����ս�
      apMUIThreadObject->UnInit();

      // ��С���ü���
      apMUIThreadObject->Release();

      // ���б����
      m_oPluginUIThreadDict.erase(iter);
   }

   return CRE_OK;
}

// ��ָ����UI�߳�ע��UI��Ϣ�ص�
HRESULT STDMETHODCALLTYPE CRuntimeUIThreadManager::RegisterUIMessageEvent(DWORD dwThreadIndex, IUIThreadMessageEvent * apEvent) {
   if (NULL != apEvent) {
      PluginUIThreadMsgEventDict::iterator iter = m_oPluginUIThreadMsgEventDict.find(apEvent);
      if (iter == m_oPluginUIThreadMsgEventDict.end()) {
         apEvent->AddRef();
         m_oPluginUIThreadMsgEventDict.insert(apEvent);
      }
   }

   return CRE_OK;
}

// ��ָ����UI�߳�ȡ��ע��UI��Ϣ�ص�
HRESULT STDMETHODCALLTYPE CRuntimeUIThreadManager::UnRegisterUIMessageEvent(DWORD dwThreadIndex, IUIThreadMessageEvent * pEvent) {
   if (NULL != pEvent) {
      m_oPluginUIThreadMsgEventDict.erase(pEvent);
      pEvent->Release();
   }

   return CRE_OK;
}

// ��ָ����UI�߳�ע��UIԤ������Ϣ�ص�
HRESULT STDMETHODCALLTYPE CRuntimeUIThreadManager::RegisterUIPreMessageEvent(IUIPreMessageEvent * apEvent) {
   if (NULL != apEvent) {
      PluginUIPreMsgEventDict::iterator iter = m_oPluginUIPreMsgEventDict.find(apEvent);
      if (iter == m_oPluginUIPreMsgEventDict.end()) {
         apEvent->AddRef();
         m_oPluginUIPreMsgEventDict.insert(apEvent);
      }
   }

   return CRE_OK;
}

// ��ָ����UI�߳�ȡ��UIԤ������Ϣ�ص�
HRESULT STDMETHODCALLTYPE CRuntimeUIThreadManager::UnRegisterUIPreMessageEvent(IUIPreMessageEvent * apEvent) {
   if (NULL != apEvent) {
      m_oPluginUIPreMsgEventDict.erase(apEvent);
      apEvent->Release();
   }

   return CRE_OK;
}

// �����߳���Ϣ
BOOL CRuntimeUIThreadManager::OnPreTranslateMessage(MSG * apMsg) {
   PluginUIThreadMsgEventDict::iterator iter = m_oPluginUIThreadMsgEventDict.begin();
   for (; iter != m_oPluginUIThreadMsgEventDict.end(); iter++) {
      IUIThreadMessageEvent* pEvent = *iter;
      pEvent->PreTranslateMessage(m_dwUIThreadID, apMsg->hwnd, apMsg->message, apMsg->wParam, apMsg->lParam);
   }

   // Ԥ������Ϣ����
   PluginUIPreMsgEventDict::iterator itor = m_oPluginUIPreMsgEventDict.begin();
   for (; itor != m_oPluginUIPreMsgEventDict.end(); itor++) {
      IUIPreMessageEvent* pEvent = *itor;
      pEvent->PreTranslateMessage(apMsg);
   }
   return TRUE;
}

// ʱ�ӻص��㲥������Ϣ
BOOL CRuntimeUIThreadManager::OnTimerBroadcastDealMessage() {
   PluginUIThreadDict::iterator iter = m_oPluginUIThreadDict.begin();

   for (; iter != m_oPluginUIThreadDict.end(); iter++) {
      VH::CComPtr<IUIThreadObject> ptrPluginUIThread(*iter);

      if (NULL == ptrPluginUIThread) {
         continue;
      }

      if (CRE_OK != ptrPluginUIThread->DealCachedMessage(m_dwUIThreadID)) {
         ASSERT(FALSE);
      }
   }

   return TRUE;
}

// ��鿪ʼ����
BOOL CRuntimeUIThreadManager::OnMessageCheckBeginTask(WPARAM wparam, LPARAM lparam) {
   IUIThreadObject* apMUIThreadObject = (IUIThreadObject*)lparam;

   PluginUIThreadDict::iterator iter = m_oPluginUIThreadDict.find(apMUIThreadObject);

   if (iter == m_oPluginUIThreadDict.end()) {
      // ���ö����ʼ��
      apMUIThreadObject->Init(m_dwUIThreadID);

      // �������ü���
      apMUIThreadObject->AddRef();

      // ���浽�б�
      m_oPluginUIThreadDict.insert(apMUIThreadObject);
   } else {
      return FALSE;
   }

   return TRUE;
}

// ����������
BOOL CRuntimeUIThreadManager::OnMessageCheckEndTask(WPARAM wparam, LPARAM lparam) {
   IUIThreadObject* apMUIThreadObject = (IUIThreadObject*)lparam;

   PluginUIThreadDict::iterator iter = m_oPluginUIThreadDict.find(apMUIThreadObject);

   if (iter == m_oPluginUIThreadDict.end()) {
      return FALSE;
   } else {
      // ���ö����ս�
      apMUIThreadObject->UnInit();

      // ��С���ü���
      apMUIThreadObject->Release();

      // ���б����
      m_oPluginUIThreadDict.erase(iter);
   }

   return TRUE;
}

// ע����Ϣ�¼�
BOOL CRuntimeUIThreadManager::OnMessageRegisterMsgEvent(WPARAM wparam, LPARAM lparam) {
   IUIThreadMessageEvent *pEvent = (IUIThreadMessageEvent*)lparam;

   if (NULL != pEvent) {
      PluginUIThreadMsgEventDict::iterator iter = m_oPluginUIThreadMsgEventDict.find(pEvent);
      if (iter == m_oPluginUIThreadMsgEventDict.end()) {
         pEvent->AddRef();
         m_oPluginUIThreadMsgEventDict.insert(pEvent);
      }
   }

   return TRUE;
}

// �Ƴ���Ϣ�¼�
BOOL CRuntimeUIThreadManager::OnMessageUnRegisterMsgEvent(WPARAM wparam, LPARAM lparam) {
   IUIThreadMessageEvent *pEvent = (IUIThreadMessageEvent*)lparam;

   if (NULL != pEvent) {
      m_oPluginUIThreadMsgEventDict.erase(pEvent);
      pEvent->Release();
   }

   return TRUE;
}

HRESULT STDMETHODCALLTYPE CRuntimeUIThreadManager::XPluginManagerEvent::QueryInterface(REFIID riid, void __RPC_FAR *__RPC_FAR *ppvObject) {
   METHOD_PROLOGUE_(CRuntimeUIThreadManager, PluginManagerEvent);
   return pThis->QueryInterface(riid, ppvObject);
}

ULONG STDMETHODCALLTYPE CRuntimeUIThreadManager::XPluginManagerEvent::AddRef(void) {
   METHOD_PROLOGUE_(CRuntimeUIThreadManager, PluginManagerEvent);
   return pThis->AddRef();
}

ULONG STDMETHODCALLTYPE CRuntimeUIThreadManager::XPluginManagerEvent::Release(void) {
   METHOD_PROLOGUE_(CRuntimeUIThreadManager, PluginManagerEvent);
   return pThis->Release();
}

// ֪ͨĳһ����������ص���������
HRESULT STDMETHODCALLTYPE CRuntimeUIThreadManager::XPluginManagerEvent::OnPluginLoaded(IPluginObject * apPluginObject) {
   METHOD_PROLOGUE_(CRuntimeUIThreadManager, PluginManagerEvent);

   VH::CComPtr<IUIThreadObject> ptrMUIThreadObject;
   if (!pThis->GetPluginUIThread(apPluginObject, ptrMUIThreadObject)) {
      ASSERT(FALSE);
      return CRE_FALSE;
   }

   if (NULL == ptrMUIThreadObject) {
      ASSERT(FALSE);
      return CRE_FALSE;
   }

   VH::CComPtr<IUIThreadManager> ptrCRUIThreadManager;
   if (CRE_OK != pThis->QueryInterface(IID_IUIThreadManager, ptrCRUIThreadManager)) {
      ASSERT(FALSE);
      return CRE_FALSE;
   }

   if (NULL == ptrCRUIThreadManager) {
      ASSERT(FALSE);
      return CRE_FALSE;
   }

   // ���ӵ�UI�̹߳�����
   ptrMUIThreadObject->Connect(ptrCRUIThreadManager);

   return CRE_OK;
}

// ֪ͨĳһ������ӹ�����ж��֮ǰ
HRESULT STDMETHODCALLTYPE CRuntimeUIThreadManager::XPluginManagerEvent::OnPluginUnLoadBefore(IPluginObject * apPluginObject) {
   METHOD_PROLOGUE_(CRuntimeUIThreadManager, PluginManagerEvent);

   VH::CComPtr<IUIThreadObject> ptrMUIThreadObject;
   if (!pThis->GetPluginUIThread(apPluginObject, ptrMUIThreadObject)) {
      ASSERT(FALSE);
      return CRE_FALSE;
   }

   if (NULL == ptrMUIThreadObject) {
      ASSERT(FALSE);
      return CRE_FALSE;
   }

   // �Ͽ�����
   ptrMUIThreadObject->Disconnect();

   return CRE_OK;
}

// ֪ͨĳһ������Ѵӹ�����ж��
HRESULT STDMETHODCALLTYPE CRuntimeUIThreadManager::XPluginManagerEvent::OnPluginUnLoaded(REFGUID aoCRPGuid) {
   METHOD_PROLOGUE_(CRuntimeUIThreadManager, PluginManagerEvent);
   return CRE_OK;
}

// �Ƿ���Լ���ĳһ�����
HRESULT STDMETHODCALLTYPE CRuntimeUIThreadManager::XPluginManagerEvent::IsCanLoadPlugin(REFGUID aoCRPGuid) {
   METHOD_PROLOGUE_(CRuntimeUIThreadManager, PluginManagerEvent);
   return CRE_OK;
}

// �Ƿ����ж��
HRESULT STDMETHODCALLTYPE CRuntimeUIThreadManager::XPluginManagerEvent::IsCanUnLoadPlugin(IPluginObject * apPluginObject) {
   METHOD_PROLOGUE_(CRuntimeUIThreadManager, PluginManagerEvent);
   return CRE_OK;
}