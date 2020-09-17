#include "stdafx.h"
#include "PluginManager.h"

//BOOL APIENTRY DllMain(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
//{
//	switch (ul_reason_for_call)
//	{
//	case DLL_PROCESS_ATTACH:
//	case DLL_THREAD_ATTACH:
//	case DLL_THREAD_DETACH:
//	case DLL_PROCESS_DETACH:
//		break;
//	}
//    return TRUE;
//}

HRESULT GetPluginManager(IPluginManager ** appPluginManager)
{
	CPluginManager& loCRPluginManager = CSingletonCRPluginManager::Instance();
	return loCRPluginManager.QueryInterface(IID_IPluginManager, (void**)appPluginManager);
}