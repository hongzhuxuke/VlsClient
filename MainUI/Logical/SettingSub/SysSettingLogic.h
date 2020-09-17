#ifndef __SYS_SETTING_LOGIC__H_INCLUDE__
#define __SYS_SETTING_LOGIC__H_INCLUDE__

#pragma once

#include <windows.h>
#include <QObject>
#include "TypeDeff.h"
#include "MediaDefs.h"
#include "VH_ConstDeff.h"
#include "pub.Const.h"
#include <thread>

struct FrameStatus {
   //��֡����
   unsigned long dropFrameCount;
   //������֡��
   unsigned long totalFrameCount;
   //������֡��
   unsigned long encodeFrameCount;
   //ʱ���
   unsigned long long os_times;
};

class SystemSettingDlg;
class QTimer;
class SysSettingLogic : public QObject {
   Q_OBJECT
public:
   SysSettingLogic(void);
   ~SysSettingLogic(void);

public:
   BOOL Create();
   void Destroy();


   static DWORD WINAPI ThreadProcess(LPVOID p);
   void ProcessTask();

   // ������Ϣ
   void DealMessage(DWORD adwMessageID, void* apData, DWORD adwLen);

   void InitDev();

   void GetDevice();

   // ����Click����
   void DealClickControl(void* apData, DWORD adwLen);

   //
   void DealLiveStatus(void* apData, DWORD adwLen);

   // �������ݳ�ʼ��֪ͨ
   void DealDataInitNotify(void* apData, DWORD adwLen);

   // ������״̬�ı�
   void DealStreamStatusChanged(void* apData, DWORD adwLen);

   // ���������ı�
   void DealVolumeChange(void* apData, DWORD adwLen);

   // ��������������
   void DealMuteSetting(void* apData, DWORD adwLen);

   // ��������ͷ����
   void DealCameraSetting(void* apData, DWORD adwLen);
   // �����豸���
   void DealDeviceChange(void *apData,DWORD adwLen);
   // �����Ҳര�ڴ����¼�
   void DealRightExternWidgetCreate(void *apData,DWORD adwLen);
   // ��������µ�����
   void DealDesktopSetting(void *apData,DWORD adwLen);

   //������Ƶ���ö�Ӧѡ�
   void DealAudioSettingCard(void *apData, DWORD adwLen);
   //
   bool GetIsServerPlayback();
   // ����
   void ApplySettings(int);
   //Ԥ����˷�
   void SetPriviewMic(DeviceInfo info);
   void SetUsedDevice(bool add, DeviceInfo info);
   // ��ʼ����������
   void InitCameraSetting();

   // ��ʼ����Ƶ����
   void InitAudioSetting();

   void InitAuidoUiInfo();

   void ActiveSettingUI();

   // ��ʼ��ϵͳ����
   void InitSysSetting();

   // ��ʾ���ô���
   void ShowSettingUI();

   //��״̬����
   void StreamStatusAnalysis();

   //����������
   void ProcessStreamConnectted(WCHAR *msg, int);

   //�������Ͽ�
   void ProcessStreamDisConnectted();

   //�ϱ�����
   void ReportQuestion();

   // ��ȡ�豸�б�
   void ObtainDeviceList(DeviceList& deviceList);

   //������˷�����
   void AdjustMicVolume(float fVolume);

   //��������������
   void AdjustSpeakerVolume(float fVolume);

   //�޸Ľ��뷧ֵ�ı�
   void DealNoiseValueChange(void *apData, DWORD adwLen);

   void SetCutRecordDisplay(const int iCutRecord);

   //�����쳣��������
   void ReSelectPushStreamLine();

   void PushStreamLineSuccess();

   void SetDesktopShare(bool isCapture);
   void SetLiveState(bool isCapture);

   bool IsHasAudioDevice();
public slots:
   // ���µ�����Ϣ״̬
   void UpdateDebugState();

private:
    void SelectFirstProtocol(QString protocol);

private:
   SystemSettingDlg* m_pSystemSettingDlg = nullptr;
   QTimer* m_pStreamStateTimer = nullptr;
   int m_iLastQuality;
   int m_iLastLine;
   int m_bIsSaveFile;
   //������Ϣ����
   QList<unsigned long long> m_LastBytesSentList;
   QList<unsigned long long> m_LastDropFramesList;
   int   m_iNumSecondsWaited4Byte;
   int   m_iNumSecondsWaited4Frame;
   int m_iVcodecFPS = 0;      //��ǰ֡��
   QList<FrameStatus> m_qFrameStatusList;
   QList<QList<struct LogFrameStatus>> mLogFrameStatusList;
   std::vector<StreamStatus> m_oStreamStatusList;
   //�����豸�б�
   DeviceList m_CameraItemMap;
   DeviceList m_oMicList;
   DeviceList m_oSpeakerList;
   DeviceInfo m_currentMic;
   DeviceInfo m_currentSpeaker;
   bool m_deviceReset = true;   
   bool m_bServerPlaybackEffect = false;
   static std::atomic_bool bManagerThreadRun;
   static HANDLE mThreadEvent;
   int mPushErrorCount = 0;
   int mPublishLine = 0;
   std::thread* mPushStreamStatusThread = nullptr;

};

#endif //__SYS_SETTING_LOGIC__H_INCLUDE__
