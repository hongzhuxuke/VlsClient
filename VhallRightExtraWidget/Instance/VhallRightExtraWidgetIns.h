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
   BOOL						m_bIsAlive;					//�Ƿ����
   long						m_lThreadCount;				//�߳�����
   DWORD                m_dwUIThreadID;				//����������UI�߳�ID
   CCriticalSection			m_oThreadCountCS;			//�߳������ٽ�
   VhallRightExtraWidgetLogic m_pRightExtraWidgetLogic;

};

typedef CSingleton<VhallRightExtraWidgetIns> SingletonVhallRightExtraWidgetIns;

#endif //__VHALLRIGHTEXTRAWIDGET_INSTANCE__H_INCLUDE__