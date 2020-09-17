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
	static unsigned int CheckMessageThread(void * apParam);
	unsigned int CheckMessageWork();

private:
   OBSControlLogic         m_oOBSControlLogic;
   DeviceManager           m_oDeviceManager;
	BOOL						   m_bIsAlive;					//是否存活的
	long						   m_lThreadCount;				//线程数量
	CCriticalSection			m_oThreadCountCS;			//线程数量临界
};

typedef CSingleton<OBSControlIns> SingletonOBSControlIns;

#endif //__COMMONTOOLKIT_INSTANCE__H_INCLUDE__