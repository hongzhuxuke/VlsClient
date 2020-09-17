#ifndef __MAINUI_INSTANCE__H_INCLUDE__
#define __MAINUI_INSTANCE__H_INCLUDE__

#pragma once
#include "PluginBase.h"
#include "Singleton.h"
#include "MainUILogic.h"
#include "SettingLogic.h"
#include "VedioPlayLogic.h"
#include "pub.Const.h"

class MainUIIns : public CPluginBase
{
public:
	MainUIIns(void);
	~MainUIIns(void);
	static void reportLog(const wchar_t* lKey, const eLogRePortK ekey, const wchar_t * operate, const QJsonObject body);
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
   MainUILogic        m_oMainUILogic;
   SettingLogic       m_oSettingLogic;
   VedioPlayLogic     m_oVedioPlayLogic;
   DWORD                m_dwUIThreadID;				//����������UI�߳�ID
	CCriticalSection			m_oThreadCountCS;			//�߳������ٽ�
};

typedef CSingleton<MainUIIns> SingletonMainUIIns;

#endif //__MAINUI_INSTANCE__H_INCLUDE__