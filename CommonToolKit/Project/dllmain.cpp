// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
   	case DLL_PROCESS_ATTACH:
         OutputDebugString(L"~~~~~~CommonToolKit DLL_PROCESS_ATTACH\n");
         break;
   	case DLL_THREAD_ATTACH:
         break;
      case DLL_THREAD_DETACH:
         break;
      case DLL_PROCESS_DETACH:
         OutputDebugString(L"~~~~~~CommonToolKit DLL_PROCESS_DETACH\n");
   		break;
	}
	return TRUE;
}

//??鵼??????
extern "C" HRESULT WINAPI GetClassObject(REFIID riid, void ** appvObj) {
   return SingletonCommonToolKitIns::Instance().QueryInterface(riid, appvObj);
}

