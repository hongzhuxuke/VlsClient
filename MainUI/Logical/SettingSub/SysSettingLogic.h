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
   //丢帧总数
   unsigned long dropFrameCount;
   //发送总帧数
   unsigned long totalFrameCount;
   //编码总帧数
   unsigned long encodeFrameCount;
   //时间戳
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

   // 处理消息
   void DealMessage(DWORD adwMessageID, void* apData, DWORD adwLen);

   void InitDev();

   void GetDevice();

   // 处理Click控制
   void DealClickControl(void* apData, DWORD adwLen);

   //
   void DealLiveStatus(void* apData, DWORD adwLen);

   // 处理数据初始化通知
   void DealDataInitNotify(void* apData, DWORD adwLen);

   // 处理流状态改变
   void DealStreamStatusChanged(void* apData, DWORD adwLen);

   // 处理音量改变
   void DealVolumeChange(void* apData, DWORD adwLen);

   // 处理静音设置设置
   void DealMuteSetting(void* apData, DWORD adwLen);

   // 处理摄像头设置
   void DealCameraSetting(void* apData, DWORD adwLen);
   // 处理设备插拔
   void DealDeviceChange(void *apData,DWORD adwLen);
   // 处理右侧窗口创建事件
   void DealRightExternWidgetCreate(void *apData,DWORD adwLen);
   // 桌面分享下的设置
   void DealDesktopSetting(void *apData,DWORD adwLen);

   //呼出音频设置对应选项卡
   void DealAudioSettingCard(void *apData, DWORD adwLen);
   //
   bool GetIsServerPlayback();
   // 设置
   void ApplySettings(int);
   //预览麦克风
   void SetPriviewMic(DeviceInfo info);
   void SetUsedDevice(bool add, DeviceInfo info);
   // 初始化摄像设置
   void InitCameraSetting();

   // 初始化音频设置
   void InitAudioSetting();

   void InitAuidoUiInfo();

   void ActiveSettingUI();

   // 初始化系统设置
   void InitSysSetting();

   // 显示设置窗口
   void ShowSettingUI();

   //流状态分析
   void StreamStatusAnalysis();

   //处理流连接
   void ProcessStreamConnectted(WCHAR *msg, int);

   //处理流断开
   void ProcessStreamDisConnectted();

   //上报问题
   void ReportQuestion();

   // 获取设备列表
   void ObtainDeviceList(DeviceList& deviceList);

   //调节麦克风音量
   void AdjustMicVolume(float fVolume);

   //调节扬声器音量
   void AdjustSpeakerVolume(float fVolume);

   //修改降噪阀值改变
   void DealNoiseValueChange(void *apData, DWORD adwLen);

   void SetCutRecordDisplay(const int iCutRecord);

   //推流异常重新推流
   void ReSelectPushStreamLine();

   void PushStreamLineSuccess();

   void SetDesktopShare(bool isCapture);
   void SetLiveState(bool isCapture);

   bool IsHasAudioDevice();
public slots:
   // 更新调试信息状态
   void UpdateDebugState();

private:
    void SelectFirstProtocol(QString protocol);

private:
   SystemSettingDlg* m_pSystemSettingDlg = nullptr;
   QTimer* m_pStreamStateTimer = nullptr;
   int m_iLastQuality;
   int m_iLastLine;
   int m_bIsSaveFile;
   //调试信息参数
   QList<unsigned long long> m_LastBytesSentList;
   QList<unsigned long long> m_LastDropFramesList;
   int   m_iNumSecondsWaited4Byte;
   int   m_iNumSecondsWaited4Frame;
   int m_iVcodecFPS = 0;      //当前帧率
   QList<FrameStatus> m_qFrameStatusList;
   QList<QList<struct LogFrameStatus>> mLogFrameStatusList;
   std::vector<StreamStatus> m_oStreamStatusList;
   //摄像设备列表
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
