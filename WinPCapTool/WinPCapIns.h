#ifndef __COMMONTOOLKIT_INSTANCE__H_INCLUDE__
#define __COMMONTOOLKIT_INSTANCE__H_INCLUDE__

#pragma once
#include "WinPCapIns.h"
#include "PluginBase.h"
#include "Singleton.h"
#include "winpcaptool_global.h"
#include "WinPCapLogic.h"
/*
   ==============================
      ��ģ���д��ڱ�����Ҫ�Ų�
   ==============================
*/
class WinPCapIns : public CPluginBase
{
public:
   WinPCapIns(void);
   ~WinPCapIns(void);

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
   CCriticalSection			m_oThreadCountCS;			//�߳������ٽ�

   WinPCapLogic* mWinPCapLogic = NULL;
};

typedef CSingleton<WinPCapIns> SingletonWinPCapToolKitIns;

#endif //__COMMONTOOLKIT_INSTANCE__H_INCLUDE__