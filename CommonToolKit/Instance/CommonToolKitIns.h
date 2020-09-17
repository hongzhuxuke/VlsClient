#ifndef __COMMONTOOLKIT_INSTANCE__H_INCLUDE__
#define __COMMONTOOLKIT_INSTANCE__H_INCLUDE__

#pragma once
#include "PluginBase.h"
#include "Singleton.h"
#include "CommonData.h"
/*
   ==============================
      本模块中存在崩溃需要排查
   ==============================
*/
class CommonToolKitIns : public CPluginBase
{
public:
	CommonToolKitIns(void);
	~CommonToolKitIns(void);

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
   CommonData           m_oCommonData;
   
	BOOL						m_bIsAlive;					//是否存活的
	long						m_lThreadCount;				//线程数量
	CCriticalSection			m_oThreadCountCS;			//线程数量临界
};

typedef CSingleton<CommonToolKitIns> SingletonCommonToolKitIns;

#endif //__COMMONTOOLKIT_INSTANCE__H_INCLUDE__
