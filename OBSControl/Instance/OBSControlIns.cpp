#include "StdAfx.h"
#include "OBSControlIns.h"

#include "Msg_OBSControl.h"
#include "Msg_CommonToolKit.h"

#include "pub.Struct.h"
#include "pathmanager.h"
#include "Msg_MainUI.h"

OBSControlIns::OBSControlIns(void)
: CPluginBase(PID_IOBSControl, ENUM_PLUGIN_OBSCONTROL, 10)
, m_lThreadCount(0)
, m_bIsAlive(FALSE) {
   wcsncpy(m_oPluginInfo.m_wzCRPName, L"OBS����ģ��", DEF_MAX_CRP_NAME_LEN);
   wcsncpy(m_oPluginInfo.m_wzCRPDescrip, L"����ͳһ�����OBS��Ⱦ/�����/����", DEF_MAX_CRP_DESCRIP_LEN);

   m_oPluginInfo.m_dwMajorVer = 1;
   m_oPluginInfo.m_dwMinorVer = 0;
   m_oPluginInfo.m_dwPatchVer = 0;
   m_oPluginInfo.m_dwBuildVer = 1;
   wcsncpy(m_oPluginInfo.m_wzCRPVerDescrip, L"��ʼ���汾", DEF_MAX_CRP_VER_DESCRIP_LEN);
}

OBSControlIns::~OBSControlIns(void) {
   ASSERT(FALSE == m_bIsAlive);
   ASSERT(0 == m_lThreadCount);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------------------------------------------------------
//��ѯ��չ�ӿ�
HRESULT OBSControlIns::InstanceQueryInterface(REFIID riid, void ** appvObject) {
   if (riid == IID_IOBSControlLogic) {
      return m_oOBSControlLogic.QueryInterface(riid, appvObject);
   }
   else {
      return m_oDeviceManager.QueryInterface(riid, appvObject);
   }
}

//�������
HRESULT OBSControlIns::InstanceCreatePlugin() {
   //��HTTP���ع�����
   do {
      // ����log
      InitDebugTrace(L"OBSControl", 6);
      TRACE6("%s============================================================================================================= \n",__FUNCTION__);
      TRACE6("%s start  create m_oOBSControlLogic\n",__FUNCTION__);
      //�߼�����
      //if (!m_oOBSControlLogic.Create()) {
      //   break;
      //}
      //TRACE6("create m_oOBSControlLogic ok\n");

      ////�߼�����
      //if (!m_oDeviceManager.Create()) {
      //   break;
      //}
      //TRACE6("create m_oDeviceManager ok\n");

      //�򿪹������
      m_bIsAlive = TRUE;

      CBaseThread loBaseThread;
      //���������߳�
      loBaseThread.BeginThread(OBSControlIns::CheckMessageThread, this);
      return CRE_OK;
   } while (0);

   //ʧ������
   //InstanceDestroyPlugin();

   return CRE_FALSE;
}

//���ٲ��
HRESULT OBSControlIns::InstanceDestroyPlugin() {
   TRACE6("OBSControlIns::InstanceDestroyPlugin() Begin \n");

   //�ر����б��
   m_bIsAlive = FALSE;

   //�ȴ��߳̽���
   while (m_lThreadCount > 0) {
      CBaseThread::Sleep(10);
   }

   //�߼�����
   m_oDeviceManager.Destroy();

   //�߼�����
   m_oOBSControlLogic.Destroy();

   TRACE6("OBSControlIns::InstanceDestroyPlugin() End \n");
   return CRE_OK;
}

//��ʼ����Ҫע�����Ϣ
HRESULT OBSControlIns::InstanceInitRegMessage(CRMessageIDQueue& aoCRMessageIDQueue) {
   aoCRMessageIDQueue.push_back(MSG_OBSCONTROL_PUBLISH);
   aoCRMessageIDQueue.push_back(MSG_OBSCONTROL_TEXT);
   aoCRMessageIDQueue.push_back(MSG_OBSCONTROL_IMAGE);
   aoCRMessageIDQueue.push_back(MSG_OBSCONTROL_DESKTOPSHARE);
   aoCRMessageIDQueue.push_back(MSG_OBSCONTROL_WINDOWSRC);
   aoCRMessageIDQueue.push_back(MSG_OBSCONTROL_SHAREREGION);
   aoCRMessageIDQueue.push_back(MSG_OBSCONTROL_PROCESSSRC);
   aoCRMessageIDQueue.push_back(MSG_OBSCONTROL_ADDCAMERA);
   aoCRMessageIDQueue.push_back(MSG_OBSCONTROL_AUDIO_CAPTURE);
	aoCRMessageIDQueue.push_back(MSG_OBSCONTROL_VIDIO_HIGHQUALITYCOD);
   //�ⲿ��Ϣ
   aoCRMessageIDQueue.push_back(MSG_COMMONDATA_DATA_INIT);
   aoCRMessageIDQueue.push_back(MSG_OBSCONTROL_PUSH_AMF0);   
   aoCRMessageIDQueue.push_back(MSG_OBSCONTROL_ENABLE_VT);
   aoCRMessageIDQueue.push_back(MSG_OBSCONTROL_CLOSE_AUDIO_DEV);
   aoCRMessageIDQueue.push_back(MSG_MAINUI_CLICK_CONTROL);
   aoCRMessageIDQueue.push_back(MSG_HTTPCENTER_HTTP_RS);
  
   return CRE_OK;
}

//������Ϣ
HRESULT OBSControlIns::InstanceDealCRMessage(DWORD adwSenderID, DWORD adwMessageID, void * apData, DWORD adwLen) {
   m_oOBSControlLogic.DealMessage(adwMessageID, apData, adwLen);
   return CRE_OK;
}

//------------------------------------------------------------------------------------------------------------------------------
unsigned int OBSControlIns::CheckMessageThread(void * apParam) {
   TRACE4("OBSControlIns::CheckMessageThread %lu\n",GetCurrentThreadId());
   OBSControlIns* pThis = static_cast<OBSControlIns*>(apParam);

   if (pThis) {
      pThis->m_oThreadCountCS.Lock();
      ++(pThis->m_lThreadCount);
      pThis->m_oThreadCountCS.UnLock();
      //��ʼ��COM
      CoInitialize(NULL);

      pThis->CheckMessageWork();

      //����ʼ��COM
      CoUninitialize();
      pThis->m_oThreadCountCS.Lock();
      --(pThis->m_lThreadCount);
      pThis->m_oThreadCountCS.UnLock();
   }
   return 0;
}

unsigned int OBSControlIns::CheckMessageWork() {
   while (m_bIsAlive) {
      //�ɷ�������Ϣ
      DispatchCachedMessage();
      //˯��һ��ʱ��
      CBaseThread::Sleep(30);
   }
   return 0;
}


void OBSControlIns::reportLog(const wchar_t* lKey, const eLogRePortK ekey, const QJsonObject body)
{
	STRU_MAINUI_LOG log;
	swprintf_s(log.m_wzRequestUrl, DEF_MAX_HTTP_URL_LEN,L"key=%s&k=%d",lKey,ekey);
	QString json = CPathManager::GetStringFromJsonObject(body);
	strcpy_s(log.m_wzRequestJson, json.mid(0, DEF_MAX_HTTP_URL_LEN).toLatin1().data());
	SingletonOBSControlIns::Instance().PostCRMessage(MSG_MAINUI_LOG, &log, sizeof(STRU_MAINUI_LOG));
}
