#ifndef __COMMONTOOLKIT_INSTANCE__H_INCLUDE__
#define __COMMONTOOLKIT_INSTANCE__H_INCLUDE__

#pragma once
#include "PluginBase.h"
#include "Singleton.h"
#include "OBSControlLogic.h"
#include "DeviceManager.h"
#include "pub.Const.h"
#include <QJsonObject>

class OBSControlIns : public CPluginBase
{
public:
	OBSControlIns(void);
	~OBSControlIns(void);
	static void reportLog(const wchar_t* lKey, const eLogRePortK ekey, const QJsonObject body);
protected:
	//----------------------------------------------------------------------------------------------------------------
	//ʵ���ӿڲ�ѯ
	virtual HRESULT InstanceQueryInterface(REFIID riid, void ** appvObject);
	//�������
	virtual HRESULT InstanceCreatePlugin();
	//���ٲ��
	virtual HRESULT InstanceDestroyPlugin();
	//��ʼ����Ҫע�����Ϣ
	virtual HRESULT InstanceInitRegMessage(CRMessageIDQueue& aoCRMessageIDQueue);
	//������Ϣ
	virtual HRESULT InstanceDealCRMessage(DWORD adwSenderID, DWORD adwMessageID, void * apData, DWORD adwLen);

private:
	static unsigned int CheckMessageThread(void * apParam);
	unsigned int CheckMessageWork();

private:
   OBSControlLogic         m_oOBSControlLogic;
   DeviceManager           m_oDeviceManager;
	BOOL						   m_bIsAlive;					//�Ƿ����
	long						   m_lThreadCount;				//�߳�����
	CCriticalSection			m_oThreadCountCS;			//�߳������ٽ�
};

typedef CSingleton<OBSControlIns> SingletonOBSControlIns;

#endif //__COMMONTOOLKIT_INSTANCE__H_INCLUDE__