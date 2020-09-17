#include "stdafx.h"
#include "MessageDispatcher.h"

//BOOL APIENTRY DllMain(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
//{
//    return TRUE;
//}

//��ȡ ��Ϣ�ɷ��� �ӿ�ָ��
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

//�ر� ��Ϣ�ɷ���
void CloseMessageDispatcher()
{
	CMessageDispatcher& loMessageDispatcher = CSingletonMessageDispatcher::Instance();
	loMessageDispatcher.Stop();
}