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
     *  �������沥���豸��ָ�������豸���������Ϊ�գ����ʾʹ��Ĭ���豸��
     **/
   virtual int CreatePlayEngine(const char* devGuid, HWND hwnd);
   /**
   *  pcm�ļ�����
   */
   virtual int PlayAudio(int sampleRate, int channel, int bitPerSample, const char* file) ;
   /**
   *  ֹͣ����
   */
   virtual int StopPlay();
   /**
   *  ���ò�������
   **/
   virtual int SetPlayVolume(int vol);
   /**
   *  �Ƿ񲥷����
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

