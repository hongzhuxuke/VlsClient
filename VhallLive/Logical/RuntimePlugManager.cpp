#include "stdafx.h"
#include "RuntimePlugManager.h"
#include "ConfigSetting.h"
#include "IPluginManager.h"
#include "IMessageDispatcher.h"
#include "pathManage.h"

#include "CRPluginDef.h"


CRuntimePlugManager::CRuntimePlugManager(void) /*:
  m_bLoadRightExtraWidgetPlugin(false)*/{
   memset(m_szGGSysPath, 0, sizeof(m_szGGSysPath));
}


CRuntimePlugManager::~CRuntimePlugManager(void) {

}


// ��ʼ��ϵͳ
HRESULT CRuntimePlugManager::InitSystem(wchar_t* apMainPath,bool bLoadRightExtraWidgetPlugin) {
   //��������
   TRACE6("%s start \n", __FUNCTION__);
   wcsncpy_s(m_szGGSysPath, apMainPath, MAX_PATH);
   //m_bLoadRightExtraWidgetPlugin=bLoadRightExtraWidgetPlugin;
   
   // ���ػ������
   if (CRE_OK != LoadBasePulgins()) {
      TRACE6("%s LoadBasePulgins err\n",__FUNCTION__);
      ASSERT(FALSE);
      return CRE_FALSE;
   }
   TRACE6("%s LoadBasePulgins \n", __FUNCTION__);
   // �����������
   if (CRE_OK != CreateBasePlugins()) {
      TRACE6("%s CRE_OK != CreateBasePlugins() \n", __FUNCTION__);
      ASSERT(FALSE);
      return CRE_FALSE;
   }
   TRACE6("%s ok \n", __FUNCTION__);
   return CRE_OK;
}

// ����ʼ��ϵͳ
HRESULT CRuntimePlugManager::UnInitSystem() {
   TRACE6("CRuntimePlugManager::UnInitSystem() Begin \n");
   // ���ٻ������
   if (CRE_OK != DestroyBasePlugins()) {
      ASSERT(FALSE);
   }

   // ж�ػ������
   if (CRE_OK != UnLoadBasePulgins()) {
      ASSERT(FALSE);
   }

   memset(m_szGGSysPath, 0, sizeof(m_szGGSysPath));

   TRACE6("CRuntimePlugManager::UnInitSystem() End \n");
   return CRE_OK;
}

// ���ػ������
HRESULT CRuntimePlugManager::LoadBasePulgins() {
   // ����CommonToolKit���
   if (CRE_OK != LoadPlugin(m_szGGSysPath, L"CommonToolKit.dll")) {
      TRACE6("CRuntimePlugManager::LoadBasePulgins().LoadPlugin ��CommonToolKit.dll��err \n");
      ASSERT(FALSE);
      return CRE_FALSE;
   }

   // ����MainUI���
   if (CRE_OK != LoadPlugin(m_szGGSysPath, L"MainUI.dll")) {
      TRACE6("CRuntimePlugManager::LoadBasePulgins().LoadPlugin ��MainUI.dll��err \n");
      ASSERT(FALSE);
      return CRE_FALSE;
   }

   // ����OBSControl���
   if (CRE_OK != LoadPlugin(m_szGGSysPath, L"OBSControl.dll")) {
	   TRACE6("CRuntimePlugManager::LoadBasePulgins().LoadPlugin ��OBSControl.dll��err \n");
	   ASSERT(FALSE);
	   return CRE_FALSE;
   }

   /*wstring confPath = GetAppDataPath() + CONFIGPATH;
   QString qsConfPath = qsConfPath.fromStdWString(confPath);
   int hideWcap = ConfigSetting::ReadInt(qsConfPath, GROUP_DEFAULT, KEY_VHALL_HIDE_WCAP, 0);
   if (hideWcap == 0) {
      if (CRE_OK != LoadPlugin(m_szGGSysPath, L"WinPCapTool.dll")) {
         TRACE6("CRuntimePlugManager::LoadBasePulgins().LoadPlugin ��WinPCapTool.dll��err \n");
         ASSERT(FALSE);
         return CRE_FALSE;
      }
   }*/

   TRACE6("CRuntimePlugManager::LoadBasePulgins() ok\n");
   return CRE_OK;
}

// ж�ػ������
HRESULT CRuntimePlugManager::UnLoadBasePulgins() {
   TRACE6("CRuntimePlugManager::UnLoadBasePulgins() Begin \n");
   //if(m_bLoadRightExtraWidgetPlugin) {
   //   //ж��VhallRightExtraWidget
   //   if (CRE_OK != UnLoadPlugin(PID_IVhallRightExtraWidget)) {
   //      ASSERT(FALSE);
   //   }
   //}

   // ж��OBS���
   if (CRE_OK != UnLoadPlugin(PID_IOBSControl)) {
      ASSERT(FALSE);
   }

   // ж��MainUI���
   if (CRE_OK != UnLoadPlugin(PID_IMainUI)) {
      ASSERT(FALSE);
   }

   // ж��HTTP���
   if (CRE_OK != UnLoadPlugin(PID_ICommonToolKit)) {
      ASSERT(FALSE);
   }
   TRACE6("CRuntimePlugManager::UnLoadBasePulgins() End \n");
   return CRE_OK;
}

// �����������
HRESULT CRuntimePlugManager::CreateBasePlugins() {
   // ����CommonToolKit���
   TRACE6("%s start run\n", __FUNCTION__);
   if (CRE_OK != CreatePlugin(PID_ICommonToolKit)) {
      TRACE6("CRuntimePlugManager::CreateBasePlugins().CreatePlugin ��PID_ICommonToolKit��err \n");
      ASSERT(FALSE);
      return CRE_FALSE;
   }
   TRACE6(" %s CreatePlugin(PID_ICommonToolKit) ok\n",__FUNCTION__);
   // ����MainUI���
   if (CRE_OK != CreatePlugin(PID_IMainUI)) {
      TRACE6("CRuntimePlugManager::CreateBasePlugins().CreatePlugin ��PID_ICommonToolKit��err \n");
      ASSERT(FALSE);
      return CRE_FALSE;
   }


   TRACE6(" %s CreatePlugin(PID_IMainUI) ok\n", __FUNCTION__);
   // ����OBSControl���
   if (CRE_OK != CreatePlugin(PID_IOBSControl)) {
      TRACE6("CRuntimePlugManager::CreateBasePlugins().CreatePlugin ��PID_IOBSControl��err \n");
      ASSERT(FALSE);
      return CRE_FALSE;
   }
   TRACE6(" %s CreatePlugin(PID_IOBSControl) ok\n", __FUNCTION__);
   TRACE6("%s run ok\n", __FUNCTION__);

   return CRE_OK;
}


//���ٻ������
HRESULT CRuntimePlugManager::DestroyBasePlugins() {
   TRACE6("CRuntimePlugManager::DestroyBasePlugins() Begin \n");
   //if(m_bLoadRightExtraWidgetPlugin) {
   //   // ����VhallRightExtraWidget���
   //   if (CRE_OK != DestroyPlugin(PID_IVhallRightExtraWidget)) {
   //      ASSERT(FALSE);
   //   }
   //}

   // ����OBSControl���
   if (CRE_OK != DestroyPlugin(PID_IOBSControl)) {
      ASSERT(FALSE);
   }

   // ����MainUI���
   if (CRE_OK != DestroyPlugin(PID_IMainUI)) {
      ASSERT(FALSE);
   }

   // ����CommonToolKit���
   if (CRE_OK != DestroyPlugin(PID_ICommonToolKit)) {
      ASSERT(FALSE);
   }
   TRACE6("CRuntimePlugManager::DestroyBasePlugins() End \n");

   return CRE_OK;
}

//�������
HRESULT CRuntimePlugManager::CreatePlugin(REFGUID pluginGuid) {
   VH::CComPtr<IPluginManager> ptrMPluginManager;
   TRACE6("%s GetPluginManager start\n", __FUNCTION__);
   //ȡ�ò��������
   if (CRE_OK != GetPluginManager(ptrMPluginManager)) {
      TRACE6("%s GetPluginManager err\n",__FUNCTION__);
      return CRE_FALSE;
   }
   TRACE6("%s GetPluginManager end\n",__FUNCTION__);
   //ȡ�ù�����ʧ��
   if (NULL == ptrMPluginManager) {
      TRACE6("%s NULL == ptrMPluginManager err\n", __FUNCTION__);
      return CRE_FALSE;
   }

   VH::CComPtr<IPluginObject> ptrMPluginObject;
   //ȡ�ò������
   if (CRE_OK != ptrMPluginManager->GetPlugin(pluginGuid, ptrMPluginObject)) {
      TRACE6("%s GetPlugin err\n", __FUNCTION__);
      return CRE_FALSE;
   }
   TRACE6("%s GetPlugin\n", __FUNCTION__);

   //û�в������
   if (NULL == ptrMPluginObject) {
       TRACE6("%s ptrMPluginObject is null \n", __FUNCTION__);
      return CRE_FALSE;
   }

   VH::CComPtr<IMessageEvent> ptrMMessageEvent;
   TRACE6("%s QueryInterface\n", __FUNCTION__);
   //ȡ�ò���������Ϣ�ɷ����ӿ�
   if (CRE_OK != ptrMPluginObject->QueryInterface(IID_IMessageEvent, ptrMMessageEvent)) {
       TRACE6("%s QueryInterface err\n", __FUNCTION__);
      return CRE_FALSE;
   }
   TRACE6("%s QueryInterface end\n", __FUNCTION__);
   //û����Ϣ�¼��ӿ�
   if (NULL == ptrMMessageEvent) {
      return CRE_FALSE;
   }

   VH::CComPtr<IMessageDispatcher> ptrMMessageDispatcher;

   //��ȡ��Ϣ�ɷ����ӿ�
   if (!GetMessageDispatcher(ptrMMessageDispatcher)) {
      return CRE_FALSE;
   }

   //����ȡ����Ϣ�ɷ����ӿ�
   if (NULL == ptrMMessageDispatcher) {
      return CRE_FALSE;
   }

   //���ӵ���Ϣ�ɷ���
   if (CRE_OK != ptrMMessageEvent->Connect(ptrMMessageDispatcher)) {
      return CRE_FALSE;
   }

   TRACE6("%s ptrMPluginObject->Create\n", __FUNCTION__);
   //if (pluginGuid == PID_IOBSControl) {
   //   //OBSʵ���������ڴ���ֱ������ʱʹ��
   //   return CRE_OK;
   //}
   //�������
   return ptrMPluginObject->Create();
}

//���ٲ��
HRESULT CRuntimePlugManager::DestroyPlugin(REFGUID pluginGuid) {
   VH::CComPtr<IPluginManager> ptrMPluginManager;
   //ȡ�ò��������
   if (CRE_OK != GetPluginManager(ptrMPluginManager)) {
      return CRE_FALSE;
   }

   //ȡ�ù�����ʧ��
   if (NULL == ptrMPluginManager) {
      return CRE_FALSE;
   }

   VH::CComPtr<IPluginObject> ptrMPluginObject;

   //ȡ�ò������
   if (CRE_OK != ptrMPluginManager->GetPlugin(pluginGuid, ptrMPluginObject)) {
      return CRE_FALSE;
   }

   //û�в������
   if (NULL == ptrMPluginObject) {
      return CRE_FALSE;
   }

   VH::CComPtr<IMessageEvent> ptrMMessageEvent;

   //ȡ�ò���������Ϣ�ɷ����ӿ�
   if (CRE_OK != ptrMPluginObject->QueryInterface(IID_IMessageEvent, ptrMMessageEvent)) {
      return CRE_FALSE;
   }

   //û����Ϣ�¼��ӿ�
   if (NULL == ptrMMessageEvent) {
      return CRE_FALSE;
   }

   //���ٲ��
   if (CRE_OK != ptrMPluginObject->Destroy()) {
      return CRE_FALSE;
   }

   //�Ͽ�����Ϣ�ɷ���������
   if (CRE_OK != ptrMMessageEvent->Disconnect()) {
      return CRE_FALSE;
   }


   return CRE_OK;
}

//--------------------------------------------------------------------------------------------------------------------------------
//���ز��
HRESULT CRuntimePlugManager::LoadPlugin(const wchar_t* apwzPathName, const wchar_t* apwzFileName) {
   wchar_t wzPluginPath[1024] = { 0 };

   //���ȫ·��
   _snwprintf_s(wzPluginPath, 1023, L"%s%s", apwzPathName, apwzFileName);

   VH::CComPtr<IPluginManager> ptrMPluginManager;

   //ȡ�ò��������
   if (CRE_OK != GetPluginManager(ptrMPluginManager)) {
      TRACE6("%s GetPluginManager err\n",__FUNCTION__);
      return CRE_FALSE;
   }

   //ȡ�ù�����ʧ��
   if (NULL == ptrMPluginManager) {
      TRACE6("%s NULL == ptrMPluginManager \n", __FUNCTION__);
      return CRE_FALSE;
   }

   //���ز������
   if (CRE_OK == ptrMPluginManager->LoadPlugin(wzPluginPath, NULL)) {
      return CRE_OK;
   }
   TRACE6("%s LoadPlugin err\n", __FUNCTION__);
   return CRE_FALSE;
}

//ж�ز��
HRESULT CRuntimePlugManager::UnLoadPlugin(REFGUID aoGuid) {
   VH::CComPtr<IPluginManager> ptrCRPluginManager;

   //ȡ�ò��������
   if (CRE_OK != GetPluginManager(ptrCRPluginManager)) {
      return CRE_FALSE;
   }

   //ȡ�ù�����ʧ��
   if (NULL == ptrCRPluginManager) {
      return CRE_FALSE;
   }

   //���ز������
   if (CRE_OK == ptrCRPluginManager->UnLoadPlugin(aoGuid)) {
      return CRE_OK;
   }

   return CRE_FALSE;
}

//ȡ�ò�������ӿ�
HRESULT CRuntimePlugManager::GetPluginInterface(REFGUID aoPluginID, REFGUID riid, void ** appInterface) {
   VH::CComPtr<IPluginObject> ptrMPluginObject;

   if (CRE_OK != GetPlugin(aoPluginID, ptrMPluginObject)) {
      return CRE_FALSE;
   }

   if (CRE_OK == ptrMPluginObject->QueryInterface(riid, appInterface)) {
      return CRE_OK;
   }
   return CRE_FALSE;
}

//ȡ�ò���ӿ�
HRESULT CRuntimePlugManager::GetPlugin(REFGUID aoPluginID, IPluginObject ** appMPluginObject) {
   VH::CComPtr<IPluginManager> ptrMPluginManager;

   //ȡ�ò��������
   if (CRE_OK != GetPluginManager(ptrMPluginManager)) {
      return CRE_FALSE;
   }

   //ȡ�ù�����ʧ��
   if (NULL == ptrMPluginManager) {
      return CRE_FALSE;
   }

   //ȡ�ò������
   if (CRE_OK == ptrMPluginManager->GetPlugin(aoPluginID, appMPluginObject)) {
      return CRE_OK;
   }
   return CRE_FALSE;
}
