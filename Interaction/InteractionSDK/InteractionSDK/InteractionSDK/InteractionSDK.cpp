// InteractionSDK.cpp : ���� DLL Ӧ�ó���ĵ���������
//

#include "stdafx.h"
#include "InteractionClient.h"
#include "InteractionAliveClient.h"
#include <windows.h>
#include <QtWidgets/QApplication>


VHALL_EXPORT IInteractionClient* CreateAliveInteractionClient() {
	return new InteractionAliveClient();
}
VHALL_EXPORT void DestroyAliveInteractionClient(IInteractionClient** interactionClient) {
	delete *interactionClient;
	*interactionClient = NULL;
}
