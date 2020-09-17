#ifndef __OBS_CONTROL_LOGIC__H_INCLUDE__
#define __OBS_CONTROL_LOGIC__H_INCLUDE__

#pragma once

#include "IOBSControlLogic.h"
#include "vhavdatasourcestatus.h"
#include "Msg_OBSControl.h"
#include <QString>
#include <QDateTime>
#include <QMap>
#include <pub.Struct.h>
#include <thread>
#include <QMutex>

class IGraphics;
class IMediaCoreEvent;
class IAudioCapture;
class IMediaCore;
class IMediaReader;
class SocketClient;
class VHCoreAudio;

class OBSControlLogic : public IOBSControlLogic, public IMediaCoreEvent, public IDataReceiver{
public:
   OBSControlLogic(void);
   ~OBSControlLogic(void);

   virtual BOOL STDMETHODCALLTYPE Create();

   virtual void STDMETHODCALLTYPE Destroy();

/*public:
   BOOL Create();
   void Destroy();*/   
public:
    /////////////////////////////IUnknown接口/////////////////////////////////////////
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void ** appvObject);
    virtual ULONG STDMETHODCALLTYPE   AddRef(void);
    virtual ULONG STDMETHODCALLTYPE   Release(void);
    //////////////////////////////////////////////////////////////////////////////////

    ///IOBSControlLogic
    virtual void STDMETHODCALLTYPE GetStreamStatus(std::vector<StreamStatus> * streamStats);
    virtual bool STDMETHODCALLTYPE IsMediaPlaying();
    virtual bool STDMETHODCALLTYPE MediaPlay(char* fileName, bool audioFile);
    virtual bool STDMETHODCALLTYPE SetEnablePlayOutMediaAudio(bool enable);
    virtual void STDMETHODCALLTYPE MediaStop();
    virtual void STDMETHODCALLTYPE MediaPause();
    virtual void STDMETHODCALLTYPE MediaResume();
    virtual void STDMETHODCALLTYPE MediaSeek(const unsigned long long& seekTime);
    virtual int STDMETHODCALLTYPE GetPlayerState();
    virtual void STDMETHODCALLTYPE SetVolume(const unsigned int & volume);
    virtual const long STDMETHODCALLTYPE VhallGetMaxDulation();
    virtual const long STDMETHODCALLTYPE VhallGetCurrentDulation();
    virtual void STDMETHODCALLTYPE OnMouseEvent(const UINT& mouseEvent, const POINTS& mousePos);
    virtual void STDMETHODCALLTYPE Resize(const RECT& client, bool bRedrawRenderFrame);
    virtual bool STDMETHODCALLTYPE IsHasNoPersistentSource();
    virtual bool STDMETHODCALLTYPE IsCanModify();
    virtual SOURCE_TYPE STDMETHODCALLTYPE GetCurrentItemType();
    virtual bool STDMETHODCALLTYPE IsHasMonitorSource();
    virtual void STDMETHODCALLTYPE GetCurrentFramePicSize(int &,int &);   
    virtual void STDMETHODCALLTYPE GetBaseSize(int &,int &);
    virtual unsigned char** STDMETHODCALLTYPE LockCurrentFramePic(unsigned long long &t);
    virtual void STDMETHODCALLTYPE UnlockCurrentFramePic();
    virtual bool STDMETHODCALLTYPE SetSourceVisible(wchar_t *sourceName,bool,bool);   
    virtual bool STDMETHODCALLTYPE WaitSetSourceVisible();
    virtual unsigned char * STDMETHODCALLTYPE MemoryCreate(int);
    virtual void  STDMETHODCALLTYPE MemoryFree(void *);
    virtual void  STDMETHODCALLTYPE Reset(const bool bIsNewRecord);
    virtual bool STDMETHODCALLTYPE IsHasPlaybackAudioDevice();   
    virtual bool STDMETHODCALLTYPE SetRecordPath(bool,wchar_t *);
    virtual void STDMETHODCALLTYPE GetAudioMeter(float& audioMag, float& audioPeak, float& audioMax);
    virtual void STDMETHODCALLTYPE GetMicAudioMeter(float& audioMag, float& audioPeak, float& audioMax);
    virtual void STDMETHODCALLTYPE SetSaveMicAudio(bool bSave);
    virtual int STDMETHODCALLTYPE GetGraphicsDeviceInfoCount();
    virtual bool STDMETHODCALLTYPE GetGraphicsDeviceInfo(DeviceInfo &,DataSourcePosType &posType,int count);
    virtual bool STDMETHODCALLTYPE GetGraphicsDeviceInfoExist(DeviceInfo &,bool &);
    virtual bool STDMETHODCALLTYPE ModifyDeviceSource(DeviceInfo&srcDevice, DeviceInfo&desDevice,DataSourcePosType posType);
    virtual bool STDMETHODCALLTYPE ReloadDevice(DeviceInfo&srcDevice);
    virtual float STDMETHODCALLTYPE MuteMic(bool);
    virtual float STDMETHODCALLTYPE MuteSpeaker(bool);
    virtual void STDMETHODCALLTYPE SetMicVolunm(float);
    virtual void STDMETHODCALLTYPE SetSpekerVolumn(float);
    virtual float STDMETHODCALLTYPE GetMicVolunm();
    virtual float STDMETHODCALLTYPE GetSpekerVolumn();
    virtual bool STDMETHODCALLTYPE GetCurrentMic(DeviceInfo &);
    virtual bool STDMETHODCALLTYPE IsHasSource(SOURCE_TYPE type);
    virtual void STDMETHODCALLTYPE DeviceRecheck();
    virtual void STDMETHODCALLTYPE EnterSharedDesktop();
    virtual void STDMETHODCALLTYPE LeaveSharedDesktop();
    virtual void STDMETHODCALLTYPE SetForceMono(bool);
    virtual bool STDMETHODCALLTYPE GetForceMono();
    virtual bool STDMETHODCALLTYPE ResetPublishInfo(const char *currentUrl,const char *nextUrl);
    virtual bool STDMETHODCALLTYPE InitPlayDevice(const wchar_t* devId);
    virtual int STDMETHODCALLTYPE GetSumSpeed();
    virtual UINT64 STDMETHODCALLTYPE GetSendVideoFrameCount();
    virtual void STDMETHODCALLTYPE ModifyAreaShared(int,int,int,int);
    virtual void STDMETHODCALLTYPE ClearAllSource(bool all);
    virtual void STDMETHODCALLTYPE ClearSourceByType(SOURCE_TYPE);
    virtual void STDMETHODCALLTYPE DealAddCameraSync(void* apData, DWORD adwLen);
    virtual int STDMETHODCALLTYPE GetRendState(DeviceInfo) ;
    virtual void STDMETHODCALLTYPE DoHideLogo(bool);
    virtual void STDMETHODCALLTYPE SetAudioCapture(bool);
    virtual QDateTime STDMETHODCALLTYPE GetStartStreamTime();
    virtual void STDMETHODCALLTYPE LivePushAmf0Msg(const char* data, int length);
    virtual bool STDMETHODCALLTYPE IsStartStream();
    //virtual QString STDMETHODCALLTYPE CommitString();
    //结束录制 bCoercion :true 表示重新连接
    virtual void STDMETHODCALLTYPE StopRecord(bool bCoercion = false, long liveTime = -1);
    virtual void STDMETHODCALLTYPE InitCapture();
    /**控制码率自适应开关**/
    virtual void RateControlSwitch(bool on);
    ////IDataReceiver
    virtual void PushAudioSegment(float *buffer, unsigned int numFrames, unsigned long long timestamp);
    virtual void PushVideoSegment(unsigned char *buffer, unsigned int size, unsigned long long timestamp,bool bSame){};
    //桌面共享锐化
    virtual void DesktopEnhanceControlSwitch(bool on);
    ///IMediaCoreEvent
    virtual void OnNetworkEvent(const MediaCoreEvent& eventType, void *data = NULL);
    virtual void OnMediaCoreLog(const char *);   
    virtual void OnMediaReportLog(const char *);
    virtual void OnMeidaTransition(const wchar_t *,int length);
    virtual void GetMediaFileWidthAndHeight(const char* path, int &width, int& height);
    virtual void StopPushStream();

    static DWORD WINAPI ThreadCreateMediaReader(LPVOID p);
    void CreateMediaReaderInstance();
    bool IsExistMediaReader();
public:
    // 处理消息
    void DealMessage(DWORD adwMessageID, void* apData, DWORD adwLen);
    // 处理推流
    void DealStartPublish(void* apData, DWORD adwLen);
    //实时字幕显示
    void DealEnableVt(void* apData, DWORD adwLen);
    // 处理文本
    void DealTextControl(void* apData, DWORD adwLen);
    // 处理图片
    void DealImageControl(void* apData, DWORD adwLen);
    // 处理桌面共享
    void DealDesktopShare(void* apData, DWORD adwLen);
    // 处理添加软件源
    void DealAddWndSrc(void* apData, DWORD adwLen);
    // 处理区域共享
    void DealRegionShare(void* apData, DWORD adwLen);
    // 处理源相关
    void DealProcessSrc(void* apData, DWORD adwLen);
    // 处理添加摄像头
    void DealAddCamera(void* apData, DWORD adwLen);
    // 处理音频捕获
    void DealAudioCapture(void* apData, DWORD adwLen);
    //处理视频设置
    void DealVidioSet(void* apData, DWORD adwLen);
    // 处理白板、文档的数据
    void DealPushAMF0(void* apData, DWORD adwLen);
    void HandleCloseAudioDev();
    //处理点击事件
    void DealClickControl(void* apData, DWORD adwLen);
    //处理打点录制请求回应
    void DealPointRecordRs(void* apData, DWORD adwLen);

private:
    //开始录制
    void startRecord();
    //暂停录制
    void suspendRecord();
    //恢复录制
    void recoveryRecord();

    void ResetNoiseCancellingbyMicState(bool mic_open);
    void commitRecord(const int eRequestType);
    wstring GetRecoderFileName();
    static bool OBSSourceDeleteHook(void *sourceItem, void *param,SOURCE_TYPE );
    void UploadPushStreamErrorToUI();
    bool OBSSourceDelete(void* sourceItem,SOURCE_TYPE type);
    //初始化麦克风捕获
    void InitMicVolumeCapture();
    void UnitMicVolumeCapture();
    //   bIsNewRecord 表示是否需要改变本地录制状态 true 标识本地录值需要变换，false标识本地录制状态不需要修改， conn  是否重连
    void StartStream(bool, bool bIsNewRecord/*=true*/, bool conn /*= false*/);
    void StopStream(bool, bool bIsNewRecord/*=true*/, bool conn /*= false*/);

    //创建图形接口
    void CreateGraphic();
    int OSGetVersion();
public:
   void RefitItem();

private:
    IGraphics* m_pGraphics;
    IAudioCapture* m_pAudioCapture;
    QMutex mMediaCoreMutex;
    IMediaCore*    m_pMediaCore;
    QMutex mutex;
    IMediaReader* m_pMediaReader;
    //插播视频项
    void* m_pMediaItem;
    //插播音频项
    void* m_pAudioItemVedioPlay;
    bool m_bStartVedioPlay;
    bool m_bStartStream;
    long m_lRefCount;
    bool m_bIsAudioExist;
    bool m_bIsSaveFile;
    wchar_t m_wSaveFilePath[MAX_BUFF];
    bool m_bIsAero = true;
    VHCoreAudio *m_vhPlayback = NULL;
    bool m_bDispatch = false;
    struct Dispatch_Param m_dispatchParam;
    bool m_bIsAddLogoSource = false;
    QString m_strLogSdUrl;
    QDateTime m_StartStreamTime;
    bool mRecordEOF;//当前录制文件是否结束  true结束需要重新添加文件   false未结束
    std::thread* mInitVLCThread = nullptr;
    bool mbIsDestoryed = false;
};

#endif //__OBS_CONTROL_LOGIC__H_INCLUDE__
