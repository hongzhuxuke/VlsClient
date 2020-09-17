#include "StdAfx.h"
#include "MainUIIns.h"

#include "Msg_MainUI.h"
#include "Msg_OBSControl.h"
#include "Msg_CommonToolKit.h"
#include "Msg_VhallRightExtraWidget.h"

#include "pub.Struct.h"
#include "pathManage.h"
#include "pathManager.h"
#include "Msg_MainUI.h"

MainUIIns::MainUIIns(void)
: CPluginBase(PID_IMainUI, ENUM_PLUGIN_MAINUI, 10)
, m_dwUIThreadID(0) {
   wcsncpy(m_oPluginInfo.m_wzCRPName, L"MAINUIģ��", DEF_MAX_CRP_NAME_LEN);
   wcsncpy(m_oPluginInfo.m_wzCRPDescrip, L"��������ʾ/������", DEF_MAX_CRP_DESCRIP_LEN);

   m_oPluginInfo.m_dwMajorVer = 1;
   m_oPluginInfo.m_dwMinorVer = 0;
   m_oPluginInfo.m_dwPatchVer = 0;
   m_oPluginInfo.m_dwBuildVer = 1;
   wcsncpy(m_oPluginInfo.m_wzCRPVerDescrip, L"��ʼ���汾", DEF_MAX_CRP_VER_DESCRIP_LEN);
}

MainUIIns::~MainUIIns(void) {
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------------------------------------------------------
//��ѯ��չ�ӿ�
HRESULT MainUIIns::InstanceQueryInterface(REFIID riid, void ** appvObject) {
   if (riid == IID_IMainUILogic) {
      return m_oMainUILogic.QueryInterface(riid, appvObject);
   } else if (riid == IID_ISettingLogic) {
      return m_oSettingLogic.QueryInterface(riid, appvObject);
   } else {
      return m_oVedioPlayLogic.QueryInterface(riid, appvObject);
   }
}

//�������
HRESULT MainUIIns::InstanceCreatePlugin() {
   //��HTTP���ع�����
   do {
      // ����log
      InitDebugTrace(L"MainUI", 6);
      TRACE6("%s============================================================================================================= \n",__FUNCTION__);

      //�߼�����
      if (!m_oMainUILogic.Create()) {
         break;
      }

      //Setting�߼�����
      //if (!m_oSettingLogic.Create()) {
      //   break;
      //}

      //VedioPlay�߼�����
      if (!m_oVedioPlayLogic.Create()) {
         break;
      }

      //����UI�߳�
      m_dwUIThreadID = BeginUIThread(ENUM_PLUGIN_THREAD_UI);

      return CRE_OK;
   } while (0);

   //ʧ������
   InstanceDestroyPlugin();

   return CRE_FALSE;
}

//���ٲ��
HRESULT MainUIIns::InstanceDestroyPlugin() {
   TRACE6("MainUIIns::InstanceDestroyPlugin() Begin \n");

   //�ر�UI�߳�
   if (0 != m_dwUIThreadID) {
      EndUIThread(ENUM_PLUGIN_THREAD_UI);
      m_dwUIThreadID = 0;
   }

   //VedioPlay�߼�����
   m_oVedioPlayLogic.Destroy();

   //Setting�߼�����
   m_oSettingLogic.Destroy();

   //�߼�����
   m_oMainUILogic.Destroy();

   TRACE6("MainUIIns::InstanceDestroyPlugin() End \n");

   return CRE_OK;
}

//��ʼ����Ҫע�����Ϣ
HRESULT MainUIIns::InstanceInitRegMessage(CRMessageIDQueue& aoCRMessageIDQueue) {
   //����ڲ�
   aoCRMessageIDQueue.push_back(MSG_MAINUI_WIDGET_SHOW);
   aoCRMessageIDQueue.push_back(MSG_MAINUI_START_LIVE_PRIVATE);
   aoCRMessageIDQueue.push_back(MSG_MAINUI_CLICK_CONTROL);
   aoCRMessageIDQueue.push_back(MSG_MAINUI_NOISE_VALUE_CHANGE);
   aoCRMessageIDQueue.push_back(MSG_MAINUI_CLICK_ADD_WNDSRC);
   aoCRMessageIDQueue.push_back(MSG_MAINUI_VOLUME_CHANGE);
   aoCRMessageIDQueue.push_back(MSG_MAINUI_LIST_CHANGE);
   aoCRMessageIDQueue.push_back(MSG_MAINUI_SHOW_AUDIOSETTING);
   aoCRMessageIDQueue.push_back(MSG_MAINUI_MUTE);
   aoCRMessageIDQueue.push_back(MSG_MAINUI_VEDIOPLAY_PLAY);
   aoCRMessageIDQueue.push_back(MSG_MAINUI_VEDIOPLAY_STOPPLAY);
   aoCRMessageIDQueue.push_back(MSG_MAINUI_VEDIOPLAY_ADDFILE);
   aoCRMessageIDQueue.push_back(MSG_MAINUI_CLOSE_SYS_SETTINGWND);
   aoCRMessageIDQueue.push_back(MSG_MAINUI_MAINUI_MOVE);
   aoCRMessageIDQueue.push_back(MSG_MAINUI_MAINUI_PLAYLISTCHG);
   aoCRMessageIDQueue.push_back(MSG_MAINUI_SAVE_SETTING);
   aoCRMessageIDQueue.push_back(MSG_MAINUI_MODIFY_TEXT);
   aoCRMessageIDQueue.push_back(MSG_MAINUI_MGR_CAMERA);
   aoCRMessageIDQueue.push_back(MSG_MAINUI_MGR_CLOSEWND);
   aoCRMessageIDQueue.push_back(MSG_MAINUI_MODIFY_CAMERA);
   aoCRMessageIDQueue.push_back(MSG_MAINUI_MODIFY_SAVE);
   aoCRMessageIDQueue.push_back(MSG_MAINUI_MODIFY_IMAGE);
   aoCRMessageIDQueue.push_back(MSG_MAINUI_HTTP_TASK);
   aoCRMessageIDQueue.push_back(MSG_MAINUI_SHOW_LOGIN);
   aoCRMessageIDQueue.push_back(MSG_VHALLRIGHTEXTRAWIDGET_INITCOMMONINFO);
   aoCRMessageIDQueue.push_back(MSG_MAINUI_CAMERA_SELECT);
   aoCRMessageIDQueue.push_back(MSG_MAINUI_CAMERA_FULL);
   aoCRMessageIDQueue.push_back(MSG_MAINUI_CAMERA_DEVICECHG);
   aoCRMessageIDQueue.push_back(MSG_MAINUI_CAMERA_UPDATE_CACHE);
   aoCRMessageIDQueue.push_back(MSG_MAINUI_CAMERA_SETTING);
   aoCRMessageIDQueue.push_back(MSG_MAINUI_CAMERA_DELETE);
   aoCRMessageIDQueue.push_back(MSG_MAINUI_DEVICE_CHANGE);
   aoCRMessageIDQueue.push_back(MSG_MAINUI_DELETE_MONITOR);
   aoCRMessageIDQueue.push_back(MSG_MAINUI_SETTING_CLOSE);
   aoCRMessageIDQueue.push_back(MSG_MAINUI_SETTING_CONFIRM);
   aoCRMessageIDQueue.push_back(MSG_MAINUI_SETTING_FOCUSIN);
   aoCRMessageIDQueue.push_back(MSG_OBSCONTROL_PUSH_STREAM_BITRATE_DOWN);
   //����ⲿ
   aoCRMessageIDQueue.push_back(MSG_COMMONDATA_DATA_INIT);  
   aoCRMessageIDQueue.push_back(MSG_OBSCONTROL_AUDIO_NOTIFY);
   aoCRMessageIDQueue.push_back(MSG_OBSCONTROL_STREAMSTATUS_CHANGE);
   //��չ���
   aoCRMessageIDQueue.push_back(MSG_VHALLRIGHTEXTRAWIDGET_CREATE);
   aoCRMessageIDQueue.push_back(MSG_VHALLRIGHTEXTRAWIDGET_ACTIVE);
   //�ر�������
   aoCRMessageIDQueue.push_back(MSG_MAINUI_DO_CLOSE_MAINWINDOW);
   aoCRMessageIDQueue.push_back(MSG_MAINUI_OBSCONTROL_PROCESSSRC);
   //������Ƶ���ý���
   aoCRMessageIDQueue.push_back(MSG_MAINUI_SHOW_AUDIO_SETTING_CARD);
   //���湲��
   aoCRMessageIDQueue.push_back(MSG_OBSCONTROL_SHAREREGION);
   //����������
   aoCRMessageIDQueue.push_back(MSG_MAINUI_HIDE_AREA);
   //�ײ���Ϣ֪ͨ
   aoCRMessageIDQueue.push_back(MSG_OBSCONTROL_STREAM_NOTIFY);
   //��ʾ����ͷ�б�
   aoCRMessageIDQueue.push_back(MSG_MAINUI_DESKTOP_SHOW_CAMERALIST);
   //���ɾ������ͷ�¼�
   aoCRMessageIDQueue.push_back(MSG_OBSCONTROL_ADDCAMERA);
   //��ʾ����
   aoCRMessageIDQueue.push_back(MSG_MAINUI_DESKTOP_SHOW_SETTING);
   //���湲��
   aoCRMessageIDQueue.push_back(MSG_OBSCONTROL_DESKTOPSHARE);
   //������־
   aoCRMessageIDQueue.push_back(MSG_MAINUI_LOG);
   //ˢ�����
   aoCRMessageIDQueue.push_back(MSG_VHALLRIGHTEXTRAWIDGET_END_REFRESH);
   //��������
   aoCRMessageIDQueue.push_back(MSG_OBSCONTROL_STREAM_RESET);
   //˵�������Ҽ�ɾ������ͷ
   //����״̬�б�
   aoCRMessageIDQueue.push_back(MSG_VHALLRIGHTEXTRAWIDGET_RIGHT_SYNC_USERLIST);
   aoCRMessageIDQueue.push_back(MSG_OBSCONTROL_STREAM_PUSH_SUCCESS); 
   aoCRMessageIDQueue.push_back(MSG_OBSCONTROL_VOICE_TRANSITION);  
   aoCRMessageIDQueue.push_back(MSG_MAINUI_RECORD_CHANGE);
   aoCRMessageIDQueue.push_back(MSG_HTTPCENTER_HTTP_RS);
   aoCRMessageIDQueue.push_back(MSG_MAINUI_CLOSE_VHALL_ACTIVE);
   aoCRMessageIDQueue.push_back(MSG_MAINUI_VHALL_ACTIVE_EVENT);
   aoCRMessageIDQueue.push_back(MSG_MAINUI_VHALL_ACTIVE_DEVICE_SELECT);  
   aoCRMessageIDQueue.push_back(MSG_MAINUI_VHALL_ACTIVE_SocketIO_MSG);   
   aoCRMessageIDQueue.push_back(MSG_MAINUI_PUSH_STREAM_STATUS);
   aoCRMessageIDQueue.push_back(MSG_MAINUI_START_LIVE_STATUS);
   return CRE_OK;
}

//������Ϣ
HRESULT MainUIIns::InstanceDealCRMessage(DWORD adwSenderID, DWORD adwMessageID, void * apData, DWORD adwLen) {
   m_oMainUILogic.DealMessage(adwMessageID, apData, adwLen);
   m_oSettingLogic.DealMessage(adwMessageID, apData, adwLen);
   m_oVedioPlayLogic.DealMessage(adwMessageID, apData, adwLen);
   return CRE_OK;
}

void MainUIIns::reportLog(const wchar_t* lKey, const eLogRePortK ekey, const wchar_t *operate, const QJsonObject body){
	STRU_MAINUI_LOG log;
	swprintf_s(log.m_wzRequestUrl, DEF_MAX_HTTP_URL_LEN,L"key=%s&k=%d&op=%s",lKey, ekey, operate);	
   //swprintf_s(log.m_wzRequestUrl, DEF_MAX_HTTP_URL_LEN,L"key=%s&k=%d&",lKey, ekey);
	QString json = CPathManager::GetStringFromJsonObject(body);
	strcpy_s(log.m_wzRequestJson, json.mid(0, DEF_MAX_HTTP_URL_LEN).toLatin1().data());
	SingletonMainUIIns::Instance().PostCRMessage(MSG_MAINUI_LOG, &log, sizeof(STRU_MAINUI_LOG));
}
