#ifndef __COMMONTOOLKIT_INSTANCE__H_INCLUDE__
#define __COMMONTOOLKIT_INSTANCE__H_INCLUDE__

#pragma once
#include "PluginBase.h"
#include "Singleton.h"
#include "CommonData.h"
/*
   ==============================
      ��ģ���д��ڱ�����Ҫ�Ų�
   ==============================
*/
class CommonToolKitIns : public CPluginBase
{
public:
	CommonToolKitIns(void);
	~CommonToolKitIns(void);

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
   CommonData           m_oCommonData;
   
	BOOL						m_bIsAlive;					//�Ƿ����
	long						m_lThreadCount;				//�߳�����
	CCriticalSection			m_oThreadCountCS;			//�߳������ٽ�
};

typedef CSingleton<CommonToolKitIns> SingletonCommonToolKitIns;

#endif //__COMMONTOOLKIT_INSTANCE__H_INCLUDE__
