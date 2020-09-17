#include "stdafx.h"
#include "MessageDispatcher.h"

//BOOL APIENTRY DllMain(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
//{
//    return TRUE;
//}

//获取 消息派发器 接口指针
BOOL GetMessageDispatcher(IMessageDispatcher ** appMessageDispatcher)
{
	CMessageDispatcher& loMessageDispatcher = CSingletonMessageDispatcher::Instance();

	if(FALSE == loMessageDispatcher.Run())
	{
		return FALSE;
	}

	if(CRE_OK == loMessageDispatcher.QueryInterface(IID_IMessageDispatcher, (void**)appMessageDispatcher))
	{
		return TRUE;
	}

	return FALSE;
}

//关闭 消息派发器
void CloseMessageDispatcher()
{
	CMessageDispatcher& loMessageDispatcher = CSingletonMessageDispatcher::Instance();
	loMessageDispatcher.Stop();
}