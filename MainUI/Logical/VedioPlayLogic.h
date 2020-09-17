#ifndef __VEDIO_PLAY_LOGIC__H_INCLUDE__
#define __VEDIO_PLAY_LOGIC__H_INCLUDE__

#pragma once

#include "IVedioPlayLogic.h"

#include <QObject>
#include <QTimer>
#include <atomic>

class VedioPlayUI;
class VideoChoiceUI;
class VedioPlayLogic : public QObject, public IVedioPlayLogic {
   Q_OBJECT
public:
   VedioPlayLogic(void);
   ~VedioPlayLogic(void);

public:
   BOOL Create();
   void Destroy();

   /////////////////////////////IUnknown接口/////////////////////////////////////////
   virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void ** appvObject);
   virtual ULONG STDMETHODCALLTYPE   AddRef(void);
   virtual ULONG STDMETHODCALLTYPE   Release(void);
   virtual void STDMETHODCALLTYPE StopPlayFile();
   virtual void STDMETHODCALLTYPE ResetPlayUiSize(int width);
   virtual int  STDMETHODCALLTYPE GetPlayUILiveType();
   virtual bool STDMETHODCALLTYPE IsPlayMediaFileUIShown();
   virtual bool STDMETHODCALLTYPE ShowPlayUI(bool);
   virtual void STDMETHODCALLTYPE ForceHide(bool bShow) ;
   virtual void STDMETHODCALLTYPE CloseFileChoiceUI();
   // 重定位插播窗口
   virtual void STDMETHODCALLTYPE ReposVedioPlay(/*bool bShow*/);

   virtual void STDMETHODCALLTYPE StopAdmin(bool);


   // 处理消息
   void DealMessage(DWORD adwMessageID, void* apData, DWORD adwLen);

   // 处理Click控制
   void DealClickControl(void* apData, DWORD adwLen);

   // 处理播放点击
   void DealVedioPlayClick(void* apData, DWORD adwLen);

   // 处理停止插播
   void DealStopPlay(void* apData, DWORD adwLen);

   // 处理添加文件
   void DealAddFiles(void* apData, DWORD adwLen);

   // 处理播放列表改变
   void DealPlayListChg(void* apData, DWORD adwLen);

   // 处理音量改变
   void DealVolumeChange(void* apData, DWORD adwLen);

private:
   //显示插播窗口
   int ShowVedioPlayWidget(BOOL bShow);

   //添加播放文件
   void AddPlayFiles();

   //播放选中文件
   void PlaySelectedFile();

   //显示时间戳转换
   QString dulation2Str(long long currDulation, long long maxDulation);

   QString dulation2StrPri(long long dulation);

private slots:
   //循环播放
   void OnCyclePlay();
   //
   void SlotRepos(bool);

   //
   void SlotForceHide(bool);
private:
   void CreatePlayMediaFileUI();
   void HandlePlayLiveMediaFile();
	void HandlePlayVhallActiveMediaFile();

   void HandleLiveCyclePlay();
	void HandleVhallActiveCyclePlay();
   void IsEnableShowPlayOutChoise();

private:
   VedioPlayUI* m_pVedioPlayUI;
   VideoChoiceUI *m_pVideoChoiceUI = NULL;
   //循环播放定时器
   QTimer m_qCycleTimer;

   long	m_lRefCount;
   BOOL m_bShowState;      //插播显示状态
   bool m_bForceHide;
   bool m_bPlayCurrentFile = false;
   int mLiveType = -1;
   QString mLastPlayFile;
   bool mbIsEnablePlayOutAudio = false;
   bool mbIsPlayEnd = true;
};

#endif //__VEDIO_PLAY_LOGIC__H_INCLUDE__S
