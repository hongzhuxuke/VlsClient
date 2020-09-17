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


// 初始化系统
HRESULT CRuntimePlugManager::InitSystem(wchar_t* apMainPath,bool bLoadRightExtraWidgetPlugin) {
   //保存数据
   TRACE6("%s start \n", __FUNCTION__);
   wcsncpy_s(m_szGGSysPath, apMainPath, MAX_PATH);
   //m_bLoadRightExtraWidgetPlugin=bLoadRightExtraWidgetPlugin;
   
   // 加载基础插件
   if (CRE_OK != LoadBasePulgins()) {
      TRACE6("%s LoadBasePulgins err\n",__FUNCTION__);
      ASSERT(FALSE);
      return CRE_FALSE;
   }
   TRACE6("%s LoadBasePulgins \n", __FUNCTION__);
   // 创建基础插件
   if (CRE_OK != CreateBasePlugins()) {
      TRACE6("%s CRE_OK != CreateBasePlugins() \n", __FUNCTION__);
      ASSERT(FALSE);
      return CRE_FALSE;
   }
   TRACE6("%s ok \n", __FUNCTION__);
   return CRE_OK;
}

// 反初始化系统
HRESULT CRuntimePlugManager::UnInitSystem() {
   TRACE6("CRuntimePlugManager::UnInitSystem() Begin \n");
   // 销毁基础插件
   if (CRE_OK != DestroyBasePlugins()) {
      ASSERT(FALSE);
   }

   // 卸载基础插件
   if (CRE_OK != UnLoadBasePulgins()) {
      ASSERT(FALSE);
   }

   memset(m_szGGSysPath, 0, sizeof(m_szGGSysPath));

   TRACE6("CRuntimePlugManager::UnInitSystem() End \n");
   return CRE_OK;
}

// 加载基础插件
HRESULT CRuntimePlugManager::LoadBasePulgins() {
   // 加载CommonToolKit插件
   if (CRE_OK != LoadPlugin(m_szGGSysPath, L"CommonToolKit.dll")) {
      TRACE6("CRuntimePlugManager::LoadBasePulgins().LoadPlugin （CommonToolKit.dll）err \n");
      ASSERT(FALSE);
      return CRE_FALSE;
   }

   // 加载MainUI插件
   if (CRE_OK != LoadPlugin(m_szGGSysPath, L"MainUI.dll")) {
      TRACE6("CRuntimePlugManager::LoadBasePulgins().LoadPlugin （MainUI.dll）err \n");
      ASSERT(FALSE);
      return CRE_FALSE;
   }

   // 加载OBSControl插件
   if (CRE_OK != LoadPlugin(m_szGGSysPath, L"OBSControl.dll")) {
	   TRACE6("CRuntimePlugManager::LoadBasePulgins().LoadPlugin （OBSControl.dll）err \n");
	   ASSERT(FALSE);
	   return CRE_FALSE;
   }

   /*wstring confPath = GetAppDataPath() + CONFIGPATH;
   QString qsConfPath = qsConfPath.fromStdWString(confPath);
   int hideWcap = ConfigSetting::ReadInt(qsConfPath, GROUP_DEFAULT, KEY_VHALL_HIDE_WCAP, 0);
   if (hideWcap == 0) {
      if (CRE_OK != LoadPlugin(m_szGGSysPath, L"WinPCapTool.dll")) {
         TRACE6("CRuntimePlugManager::LoadBasePulgins().LoadPlugin （WinPCapTool.dll）err \n");
         ASSERT(FALSE);
         return CRE_FALSE;
      }
   }*/

   TRACE6("CRuntimePlugManager::LoadBasePulgins() ok\n");
   return CRE_OK;
}

// 卸载基础插件
HRESULT CRuntimePlugManager::UnLoadBasePulgins() {
   TRACE6("CRuntimePlugManager::UnLoadBasePulgins() Begin \n");
   //if(m_bLoadRightExtraWidgetPlugin) {
   //   //卸载VhallRightExtraWidget
   //   if (CRE_OK != UnLoadPlugin(PID_IVhallRightExtraWidget)) {
   //      ASSERT(FALSE);
   //   }
   //}

   // 卸载OBS插件
   if (CRE_OK != UnLoadPlugin(PID_IOBSControl)) {
      ASSERT(FALSE);
   }

   // 卸载MainUI插件
   if (CRE_OK != UnLoadPlugin(PID_IMainUI)) {
      ASSERT(FALSE);
   }

   // 卸载HTTP插件
   if (CRE_OK != UnLoadPlugin(PID_ICommonToolKit)) {
      ASSERT(FALSE);
   }
   TRACE6("CRuntimePlugManager::UnLoadBasePulgins() End \n");
   return CRE_OK;
}

// 创建基础插件
HRESULT CRuntimePlugManager::CreateBasePlugins() {
   // 创建CommonToolKit插件
   TRACE6("%s start run\n", __FUNCTION__);
   if (CRE_OK != CreatePlugin(PID_ICommonToolKit)) {
      TRACE6("CRuntimePlugManager::CreateBasePlugins().CreatePlugin （PID_ICommonToolKit）err \n");
      ASSERT(FALSE);
      return CRE_FALSE;
   }
   TRACE6(" %s CreatePlugin(PID_ICommonToolKit) ok\n",__FUNCTION__);
   // 创建MainUI插件
   if (CRE_OK != CreatePlugin(PID_IMainUI)) {
      TRACE6("CRuntimePlugManager::CreateBasePlugins().CreatePlugin （PID_ICommonToolKit）err \n");
      ASSERT(FALSE);
      return CRE_FALSE;
   }


   TRACE6(" %s CreatePlugin(PID_IMainUI) ok\n", __FUNCTION__);
   // 创建OBSControl插件
   if (CRE_OK != CreatePlugin(PID_IOBSControl)) {
      TRACE6("CRuntimePlugManager::CreateBasePlugins().CreatePlugin （PID_IOBSControl）err \n");
      ASSERT(FALSE);
      return CRE_FALSE;
   }
   TRACE6(" %s CreatePlugin(PID_IOBSControl) ok\n", __FUNCTION__);
   TRACE6("%s run ok\n", __FUNCTION__);

   return CRE_OK;
}


//销毁基础插件
HRESULT CRuntimePlugManager::DestroyBasePlugins() {
   TRACE6("CRuntimePlugManager::DestroyBasePlugins() Begin \n");
   //if(m_bLoadRightExtraWidgetPlugin) {
   //   // 销毁VhallRightExtraWidget插件
   //   if (CRE_OK != DestroyPlugin(PID_IVhallRightExtraWidget)) {
   //      ASSERT(FALSE);
   //   }
   //}

   // 销毁OBSControl插件
   if (CRE_OK != DestroyPlugin(PID_IOBSControl)) {
      ASSERT(FALSE);
   }

   // 销毁MainUI插件
   if (CRE_OK != DestroyPlugin(PID_IMainUI)) {
      ASSERT(FALSE);
   }

   // 销毁CommonToolKit插件
   if (CRE_OK != DestroyPlugin(PID_ICommonToolKit)) {
      ASSERT(FALSE);
   }
   TRACE6("CRuntimePlugManager::DestroyBasePlugins() End \n");

   return CRE_OK;
}

//创建插件
HRESULT CRuntimePlugManager::CreatePlugin(REFGUID pluginGuid) {
   VH::CComPtr<IPluginManager> ptrMPluginManager;
   TRACE6("%s GetPluginManager start\n", __FUNCTION__);
   //取得插件管理器
   if (CRE_OK != GetPluginManager(ptrMPluginManager)) {
      TRACE6("%s GetPluginManager err\n",__FUNCTION__);
      return CRE_FALSE;
   }
   TRACE6("%s GetPluginManager end\n",__FUNCTION__);
   //取得管理器失败
   if (NULL == ptrMPluginManager) {
      TRACE6("%s NULL == ptrMPluginManager err\n", __FUNCTION__);
      return CRE_FALSE;
   }

   VH::CComPtr<IPluginObject> ptrMPluginObject;
   //取得插件对象
   if (CRE_OK != ptrMPluginManager->GetPlugin(pluginGuid, ptrMPluginObject)) {
      TRACE6("%s GetPlugin err\n", __FUNCTION__);
      return CRE_FALSE;
   }
   TRACE6("%s GetPlugin\n", __FUNCTION__);

   //没有插件对象
   if (NULL == ptrMPluginObject) {
       TRACE6("%s ptrMPluginObject is null \n", __FUNCTION__);
      return CRE_FALSE;
   }

   VH::CComPtr<IMessageEvent> ptrMMessageEvent;
   TRACE6("%s QueryInterface\n", __FUNCTION__);
   //取得插件对象的消息派发器接口
   if (CRE_OK != ptrMPluginObject->QueryInterface(IID_IMessageEvent, ptrMMessageEvent)) {
       TRACE6("%s QueryInterface err\n", __FUNCTION__);
      return CRE_FALSE;
   }
   TRACE6("%s QueryInterface end\n", __FUNCTION__);
   //没有消息事件接口
   if (NULL == ptrMMessageEvent) {
      return CRE_FALSE;
   }

   VH::CComPtr<IMessageDispatcher> ptrMMessageDispatcher;

   //获取消息派发器接口
   if (!GetMessageDispatcher(ptrMMessageDispatcher)) {
      return CRE_FALSE;
   }

   //不能取得消息派发器接口
   if (NULL == ptrMMessageDispatcher) {
      return CRE_FALSE;
   }

   //连接到消息派发器
   if (CRE_OK != ptrMMessageEvent->Connect(ptrMMessageDispatcher)) {
      return CRE_FALSE;
   }

   TRACE6("%s ptrMPluginObject->Create\n", __FUNCTION__);
   //if (pluginGuid == PID_IOBSControl) {
   //   //OBS实例化，留在创建直播窗口时使用
   //   return CRE_OK;
   //}
   //创建插件
   return ptrMPluginObject->Create();
}

//销毁插件
HRESULT CRuntimePlugManager::DestroyPlugin(REFGUID pluginGuid) {
   VH::CComPtr<IPluginManager> ptrMPluginManager;
   //取得插件管理器
   if (CRE_OK != GetPluginManager(ptrMPluginManager)) {
      return CRE_FALSE;
   }

   //取得管理器失败
   if (NULL == ptrMPluginManager) {
      return CRE_FALSE;
   }

   VH::CComPtr<IPluginObject> ptrMPluginObject;

   //取得插件对象
   if (CRE_OK != ptrMPluginManager->GetPlugin(pluginGuid, ptrMPluginObject)) {
      return CRE_FALSE;
   }

   //没有插件对象
   if (NULL == ptrMPluginObject) {
      return CRE_FALSE;
   }

   VH::CComPtr<IMessageEvent> ptrMMessageEvent;

   //取得插件对象的消息派发器接口
   if (CRE_OK != ptrMPluginObject->QueryInterface(IID_IMessageEvent, ptrMMessageEvent)) {
      return CRE_FALSE;
   }

   //没有消息事件接口
   if (NULL == ptrMMessageEvent) {
      return CRE_FALSE;
   }

   //销毁插件
   if (CRE_OK != ptrMPluginObject->Destroy()) {
      return CRE_FALSE;
   }

   //断开与消息派发器的连接
   if (CRE_OK != ptrMMessageEvent->Disconnect()) {
      return CRE_FALSE;
   }


   return CRE_OK;
}

//--------------------------------------------------------------------------------------------------------------------------------
//加载插件
HRESULT CRuntimePlugManager::LoadPlugin(const wchar_t* apwzPathName, const wchar_t* apwzFileName) {
   wchar_t wzPluginPath[1024] = { 0 };

   //组合全路径
   _snwprintf_s(wzPluginPath, 1023, L"%s%s", apwzPathName, apwzFileName);

   VH::CComPtr<IPluginManager> ptrMPluginManager;

   //取得插件管理器
   if (CRE_OK != GetPluginManager(ptrMPluginManager)) {
      TRACE6("%s GetPluginManager err\n",__FUNCTION__);
      return CRE_FALSE;
   }

   //取得管理器失败
   if (NULL == ptrMPluginManager) {
      TRACE6("%s NULL == ptrMPluginManager \n", __FUNCTION__);
      return CRE_FALSE;
   }

   //加载插件对象
   if (CRE_OK == ptrMPluginManager->LoadPlugin(wzPluginPath, NULL)) {
      return CRE_OK;
   }
   TRACE6("%s LoadPlugin err\n", __FUNCTION__);
   return CRE_FALSE;
}

//卸载插件
HRESULT CRuntimePlugManager::UnLoadPlugin(REFGUID aoGuid) {
   VH::CComPtr<IPluginManager> ptrCRPluginManager;

   //取得插件管理器
   if (CRE_OK != GetPluginManager(ptrCRPluginManager)) {
      return CRE_FALSE;
   }

   //取得管理器失败
   if (NULL == ptrCRPluginManager) {
      return CRE_FALSE;
   }

   //加载插件对象
   if (CRE_OK == ptrCRPluginManager->UnLoadPlugin(aoGuid)) {
      return CRE_OK;
   }

   return CRE_FALSE;
}

//取得插件导出接口
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

//取得插件接口
HRESULT CRuntimePlugManager::GetPlugin(REFGUID aoPluginID, IPluginObject ** appMPluginObject) {
   VH::CComPtr<IPluginManager> ptrMPluginManager;

   //取得插件管理器
   if (CRE_OK != GetPluginManager(ptrMPluginManager)) {
      return CRE_FALSE;
   }

   //取得管理器失败
   if (NULL == ptrMPluginManager) {
      return CRE_FALSE;
   }

   //取得插件对象
   if (CRE_OK == ptrMPluginManager->GetPlugin(aoPluginID, appMPluginObject)) {
      return CRE_OK;
   }
   return CRE_FALSE;
}
