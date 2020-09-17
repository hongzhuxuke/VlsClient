#pragma once
#include "IPlayerEngine.h" 
#include <atomic>
#include <stdio.h>  
#include <stdlib.h>  
#include <windows.h>  
#include <dsound.h> 
#include <thread> 


#define MAX_AUDIO_BUF 4   
#define BUFFERNOTIFYSIZE 1920


//int sample_rate = 44100;  //PCM sample rate  
//int channels = 2;         //PCM channel number  
//int bits_per_sample = 16; //bits per sample 

class PlayerEngineImpl : public vlive::IPlayerEngine
{
public:
   PlayerEngineImpl();
   virtual ~PlayerEngineImpl();
   /**
     *  创建引擎播放设备，指定播放设备。如果传入为空，则表示使用默认设备。
     **/
   virtual int CreatePlayEngine(const char* devGuid, HWND hwnd);
   /**
   *  pcm文件播放
   */
   virtual int PlayAudio(int sampleRate, int channel, int bitPerSample, const char* file) ;
   /**
   *  停止播放
   */
   virtual int StopPlay();
   /**
   *  设置播放音量
   **/
   virtual int SetPlayVolume(int vol);
   /**
   *  是否播放完成
   **/
   virtual bool IsPlaying(int vol);


   static void ThreadProFun(void *);
   void ProcessFun();

private:
   int InitPlayFormat(int sample_rate, int channels, int bits_per_sample);

private:
   std::atomic_bool mIsPlaying = false;

   IDirectSound8 *m_pDS = NULL;
   IDirectSoundBuffer8 *m_pDSBuffer8 = NULL; //used to manage sound buffers.  
   IDirectSoundBuffer *m_pDSBuffer = NULL;
   IDirectSoundNotify8 *m_pDSNotify = NULL;
   DSBPOSITIONNOTIFY m_pDSPosNotify[MAX_AUDIO_BUF];
   HANDLE m_event[MAX_AUDIO_BUF];
   std::string mPlayFile;

   std::atomic_bool mThreadRuning = false;
   std::thread *mPlayThread = nullptr;

   std::atomic<int> mCurrentVol = 99;
};

