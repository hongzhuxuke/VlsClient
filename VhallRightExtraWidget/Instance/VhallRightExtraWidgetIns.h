#ifndef __VHALLRIGHTEXTRAWIDGET_INSTANCE__H_INCLUDE__
#define __VHALLRIGHTEXTRAWIDGET_INSTANCE__H_INCLUDE__

#pragma once
#include "PluginBase.h"
#include "Singleton.h"
#include "VhallRightExtraWidgetLogic.h"
#include "CRPluginDef.h"

class VhallRightExtraWidgetIns : public CPluginBase {
public:
   VhallRightExtraWidgetIns(void);
   ~VhallRightExtraWidgetIns(void);
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
   BOOL						m_bIsAlive;					//是否存活的
   long						m_lThreadCount;				//线程数量
   DWORD                m_dwUIThreadID;				//期望关联的UI线程ID
   CCriticalSection			m_oThreadCountCS;			//线程数量临界
   VhallRightExtraWidgetLogic m_pRightExtraWidgetLogic;

};

typedef CSingleton<VhallRightExtraWidgetIns> SingletonVhallRightExtraWidgetIns;

#endif //__VHALLRIGHTEXTRAWIDGET_INSTANCE__H_INCLUDE__