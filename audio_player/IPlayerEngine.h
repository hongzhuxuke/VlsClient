/*****
*  PCM原始文件播放*
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
   *  创建引擎播放设备，指定播放设备。如果传入为空，则表示使用默认设备。
   **/
   virtual int CreatePlayEngine(const char* devGuid,HWND hwnd) = 0;
   /**
   *  pcm文件播放
   */
   virtual int PlayAudio(int sampleRate, int channel, int bitPerSample, const char* file) = 0;
   /**
   *  停止播放
   */
   virtual int StopPlay() = 0;
   /**
   *  设置播放音量
   **/
   virtual int SetPlayVolume(int vol) = 0;
   /**
   *  是否播放完成
   **/
   virtual bool IsPlaying(int vol) = 0;
};

VHPLAYER_ENGINE_EXPORT IPlayerEngine* CreateEngineInstance();
VHPLAYER_ENGINE_EXPORT void DestoryEngineInstance();

}
