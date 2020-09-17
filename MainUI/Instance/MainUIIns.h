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
	//实例接口查询
	virtual HRESULT InstanceQueryInterface(REFIID riid, void ** appvObject);
	//创建插件
	virtual HRESULT InstanceCreatePlugin();
	//销毁插件
	virtual HRESULT InstanceDestroyPlugin();
	//初始化需要注册的消息
	virtual HRESULT InstanceInitRegMessage(CRMessageIDQueue& aoCRMessageIDQueue);
	//处理消息
	virtual HRESULT InstanceDealCRMessage(DWORD adwSenderID, DWORD adwMessageID, void * apData, DWORD adwLen);

private:
   MainUILogic        m_oMainUILogic;
   SettingLogic       m_oSettingLogic;
   VedioPlayLogic     m_oVedioPlayLogic;
   DWORD                m_dwUIThreadID;				//期望关联的UI线程ID
	CCriticalSection			m_oThreadCountCS;			//线程数量临界
};

typedef CSingleton<MainUIIns> SingletonMainUIIns;

#endif //__MAINUI_INSTANCE__H_INCLUDE__