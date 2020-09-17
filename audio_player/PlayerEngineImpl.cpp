#include "PlayerEngineImpl.h"
#include <mutex>

static PlayerEngineImpl* gPlayerEngineImpl = nullptr; 
static std::mutex gMutex;

#define DSVOLUME_TO_DB(volume) ((DWORD)-30*100-volume)

PlayerEngineImpl::PlayerEngineImpl() {

}

PlayerEngineImpl::~PlayerEngineImpl() {
   StopPlay();
   if (m_pDS) {
      IDirectSound8_Release(m_pDS);
   }
}

int PlayerEngineImpl::CreatePlayEngine(const char* devGuid, HWND hwnd) {
   //Init DirectSound  
   if (FAILED(DirectSoundCreate8(NULL, &m_pDS, NULL))) {
      return -1;
   }

   if (FAILED(m_pDS->SetCooperativeLevel(hwnd, DSSCL_NORMAL))) {
      return -1;
   }

   return 0;
}
//48000 2 16
int PlayerEngineImpl::InitPlayFormat(int sample_rate, int channels, int bits_per_sample) {
   DSBUFFERDESC dsbd;
   memset(&dsbd, 0, sizeof(dsbd));
   dsbd.dwSize = sizeof(dsbd);
   dsbd.dwFlags = DSBCAPS_GLOBALFOCUS | DSBCAPS_CTRLPOSITIONNOTIFY | DSBCAPS_GETCURRENTPOSITION2;
   dsbd.dwBufferBytes = MAX_AUDIO_BUF * BUFFERNOTIFYSIZE;
   //WAVE Header  
   dsbd.lpwfxFormat = (WAVEFORMATEX*)malloc(sizeof(WAVEFORMATEX));
   dsbd.lpwfxFormat->wFormatTag = WAVE_FORMAT_PCM;
   /* format type */
   (dsbd.lpwfxFormat)->nChannels = channels;
   /* number of channels (i.e. mono, stereo...) */
   (dsbd.lpwfxFormat)->nSamplesPerSec = sample_rate;
   /* sample rate */
   (dsbd.lpwfxFormat)->nAvgBytesPerSec = sample_rate * (bits_per_sample / 8)*channels;
   /* for buffer estimation */
   (dsbd.lpwfxFormat)->nBlockAlign = (bits_per_sample / 8)*channels;
   /* block size of data */
   (dsbd.lpwfxFormat)->wBitsPerSample = bits_per_sample;
   /* number of bits per sample of mono data */
   (dsbd.lpwfxFormat)->cbSize = 0;

   //Creates a sound buffer object to manage audio samples.   
   HRESULT hr1;
   if (FAILED(m_pDS->CreateSoundBuffer(&dsbd, &m_pDSBuffer, NULL))) {
      return -1;
   }
   if (FAILED(m_pDSBuffer->QueryInterface(IID_IDirectSoundBuffer8, (LPVOID*)&m_pDSBuffer8))) {
      return -1;
   }
   //Get IDirectSoundNotify8  
   if (FAILED(m_pDSBuffer8->QueryInterface(IID_IDirectSoundNotify, (LPVOID*)&m_pDSNotify))) {
      return -1;
   }
   for (int i = 0; i < MAX_AUDIO_BUF; i++) {
      m_pDSPosNotify[i].dwOffset = i * BUFFERNOTIFYSIZE;
      m_event[i] = ::CreateEvent(NULL, false, false, NULL);
      m_pDSPosNotify[i].hEventNotify = m_event[i];
   }
   m_pDSNotify->SetNotificationPositions(MAX_AUDIO_BUF, m_pDSPosNotify);
   m_pDSNotify->Release();

   //Start Playing  
   BOOL isPlaying = TRUE;
   LPVOID buf = NULL;
   DWORD  buf_len = 0;
   DWORD res = WAIT_OBJECT_0;
   DWORD offset = BUFFERNOTIFYSIZE;
}


int PlayerEngineImpl::PlayAudio(int sampleRate, int channel, int bitPerSample, const char* file) {
   mPlayFile = file;
   StopPlay();
   InitPlayFormat(sampleRate, channel, bitPerSample);
   mThreadRuning = true;
   mPlayThread = new std::thread(ThreadProFun, this);
   return 0;
}


int PlayerEngineImpl::StopPlay() {
   if (m_pDSBuffer8) {
      m_pDSBuffer8->Stop();
   }

   mIsPlaying = false;
   mThreadRuning = false;
   for (int i = 0; i < MAX_AUDIO_BUF; i++) {
      if (m_event[i]) {
         SetEvent(m_event[i]);
      }
   }

   if (mPlayThread) {
      mPlayThread->join();
      delete mPlayThread;
      mPlayThread = nullptr;
   }
   for (int i = 0; i < MAX_AUDIO_BUF; i++) {
      if (m_event[i]) {
         CloseHandle(m_event[i]);
         m_event[i] = nullptr;
      }
   }
   if (m_pDSBuffer8) {
      IDirectSound8_Release(m_pDSBuffer8);
      m_pDSBuffer8 = nullptr;
   }
   if (m_pDSBuffer) {
      IDirectSound8_Release(m_pDSBuffer);
      m_pDSBuffer = nullptr;
   }
   return 0;
}


int PlayerEngineImpl::SetPlayVolume(int inputVolume) {
   if (inputVolume < 0 || inputVolume > 100) {
      return -1;
   }
   mCurrentVol = inputVolume;
   return 0;
}


bool PlayerEngineImpl::IsPlaying(int vol) {
   return mIsPlaying;
}

void PlayerEngineImpl::ThreadProFun(void *param){
   if (param) {
      PlayerEngineImpl *runObj = (PlayerEngineImpl*)(param);
      runObj->ProcessFun();
   }
}


int volume_adjust(short  * in_buf, short  * out_buf, float in_vol)
{
   int i, tmp;

   // in_vol[0, 100]
   float vol = in_vol - 98;

   if (-98 < vol && vol < 0)
      vol = 1 / (vol*(-1));
   else if (0 <= vol && vol <= 1)
      vol = 1;
   /*
   else if(1<=vol && vol<=2)
       vol = vol;
   */
   else if (vol <= -98)
      vol = 0;
   else if (vol >= 2)
      vol = 40;  //这个值可以根据你的实际情况去调整

   tmp = (*in_buf)*vol; // 上面所有关于vol的判断，其实都是为了此处*in_buf乘以一个倍数，你可以根据自己的需要去修改

   // 下面的code主要是为了溢出判断
   if (tmp > 32767)
      tmp = 32767;
   else if (tmp < -32768)
      tmp = -32768;
   *out_buf = tmp;

   return 0;
}

void PlayerEngineImpl::ProcessFun() {
   FILE * fp;
   if ((fp = fopen(mPlayFile.c_str(), "rb")) == NULL) {
      printf("cannot open this file\n");
   }

   LPVOID buf = NULL;
   DWORD  buf_len = 0;
   DWORD res = WAIT_OBJECT_0;
   DWORD offset = BUFFERNOTIFYSIZE;

   m_pDSBuffer8->SetCurrentPosition(0);
   m_pDSBuffer8->Play(0, 0, 1);
   //Loop  
   mIsPlaying = true;
   while (mIsPlaying) {
      if ((res >= WAIT_OBJECT_0) && (res <= WAIT_OBJECT_0 + 3)) {
         m_pDSBuffer8->Lock(offset, BUFFERNOTIFYSIZE, &buf, &buf_len, NULL, NULL, 0);
         if (fread(buf, 1, buf_len, fp) != buf_len) {
            //File End  
            //Loop:  
            //fseek(fp, 0, SEEK_SET);
            //fread(buf, 1, buf_len, fp);
            //Close:  
            mIsPlaying = false;
         }
         short *s16In = (short *)buf;
         for (int i = 0; i < buf_len / 2; i++) {
            volume_adjust(s16In, s16In, mCurrentVol);
            s16In++;
         }
         m_pDSBuffer8->Unlock(buf, buf_len, NULL, 0);
         if (!mIsPlaying) {
            break;
         }
         offset += buf_len;
         offset %= (BUFFERNOTIFYSIZE * MAX_AUDIO_BUF);
         printf("this is %7d of buffer\n", offset);
      }
      res = WaitForMultipleObjects(MAX_AUDIO_BUF, m_event, FALSE, 1000);
   }
   mIsPlaying = false;
   //如果正常播放完毕，需要睡眠一下，否则缓存区的数据可能会没有播放完毕。
   Sleep(1000);
   if (m_pDSBuffer8) {
      m_pDSBuffer8->Stop();
      IDirectSound8_Release(m_pDSBuffer8);
      m_pDSBuffer8 = nullptr;
   }
   if (m_pDSBuffer) {
      IDirectSound8_Release(m_pDSBuffer);
      m_pDSBuffer = nullptr;
   }
   if (fp) {
      fclose(fp);
   }
}


namespace vlive {
   VHPLAYER_ENGINE_EXPORT IPlayerEngine* CreateEngineInstance() {
      std::unique_lock<std::mutex> lock(gMutex);
      if (gPlayerEngineImpl == nullptr) {
         gPlayerEngineImpl = new PlayerEngineImpl();
      }

      return (IPlayerEngine*)gPlayerEngineImpl;
   }

   VHPLAYER_ENGINE_EXPORT void DestoryEngineInstance() {
      std::unique_lock<std::mutex> lock(gMutex);
      if (gPlayerEngineImpl) {
         delete gPlayerEngineImpl;
         gPlayerEngineImpl = nullptr;
      }
   }
}