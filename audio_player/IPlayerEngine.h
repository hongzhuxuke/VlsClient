/*****
*  PCMԭʼ�ļ�����*
****/

#pragma once
#ifdef  VHPLAYER_ENGINE_EXPORT
#define VHPLAYER_ENGINE_EXPORT     __declspec(dllimport)
#else
#define VHPLAYER_ENGINE_EXPORT     __declspec(dllexport)
#endif

#include <windows.h>  

namespace vlive {

class IPlayerEngine {
   public:
   /**
   *  �������沥���豸��ָ�������豸���������Ϊ�գ����ʾʹ��Ĭ���豸��
   **/
   virtual int CreatePlayEngine(const char* devGuid,HWND hwnd) = 0;
   /**
   *  pcm�ļ�����
   */
   virtual int PlayAudio(int sampleRate, int channel, int bitPerSample, const char* file) = 0;
   /**
   *  ֹͣ����
   */
   virtual int StopPlay() = 0;
   /**
   *  ���ò�������
   **/
   virtual int SetPlayVolume(int vol) = 0;
   /**
   *  �Ƿ񲥷����
   **/
   virtual bool IsPlaying(int vol) = 0;
};

VHPLAYER_ENGINE_EXPORT IPlayerEngine* CreateEngineInstance();
VHPLAYER_ENGINE_EXPORT void DestoryEngineInstance();

}
