#pragma once

#include "Singleton.h"

class IPluginObject;

class CRuntimePlugManager
{
public:
   CRuntimePlugManager(void);
   ~CRuntimePlugManager(void);
public:
   HRESULT InitSystem(wchar_t* apMainPath,bool bLoadRightExtraWidgetPlugin);
   HRESULT UnInitSystem();
   HRESULT GetPluginInterface(REFGUID aoPluginID, REFGUID riid, void** appInterface);
   HRESULT GetPlugin(REFGUID aoPluginID, IPluginObject ** appMPluginObject);
protected:
   HRESULT LoadBasePulgins();
   HRESULT UnLoadBasePulgins();
   HRESULT CreateBasePlugins();
   HRESULT DestroyBasePlugins();
private:
   HRESULT CreatePlugin(REFGUID pluginGuid);
   HRESULT DestroyPlugin(REFGUID pluginGuid);
   HRESULT LoadPlugin(const wchar_t* apwzPathName, const wchar_t* apwzFileName);
   HRESULT UnLoadPlugin(REFGUID aoGuid);
private:
   wchar_t m_szGGSysPath[MAX_PATH + 1];
   //bool m_bLoadRightExtraWidgetPlugin;
};

typedef CSingleton<CRuntimePlugManager> CSingletonRuntimePlugManager;