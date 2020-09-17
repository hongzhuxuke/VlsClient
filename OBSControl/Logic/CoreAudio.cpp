#include "CoreAudio.h"
#include <QDebug>
#include <qiodevice.h>
#include <windows.h>


VHCoreAudio::VHCoreAudio() :QIODevice(NULL){
   mOutput = NULL;
   connect(this, SIGNAL(SigReset(char *, unsigned , unsigned )), this, SLOT(SlotReset(char *, unsigned , unsigned )));
   connect(this,SIGNAL(readChannelFinished()),this,SLOT(SlotFinished()));
   connect(this,SIGNAL(aboutToClose()),this,SLOT(SlotClose()));            
}
VHCoreAudio::~VHCoreAudio() {
   if (mOutput) {
      delete mOutput;
      mOutput=NULL;
   }
   if(m_pdata) {
      delete m_pdata;
      m_pdata = NULL;
      m_dalaLength = 0;
   }
}

void VHCoreAudio::SlotFinished(){
   OutputDebugString(L"VHCoreAudio::SlotFinished\n");
}
void VHCoreAudio::SlotClose(){
   OutputDebugString(L"VHCoreAudio::SlotClose\n");
}
void VHCoreAudio::stateChanged(QAudio::State stat){
   qDebug() << "VHCoreAudio::stateChanged" << stat;

   if(stat == QAudio::IdleState) {
      mOutput->stop();
   }
   if (stat == QAudio::StoppedState) {
      mutex.lock();
	  mbuffer.clear();
      mutex.unlock();
      mOutput->start(this);
   }

}

void VHCoreAudio::SlotReset(char *format, unsigned rate, unsigned channels) {
   qDebug()<<"VHCoreAudio::SlotReset";
   mFmt.setSampleRate(44100);
   mFmt.setChannelCount(2);
   mFmt.setSampleSize(16);
   mFmt.setCodec("audio/pcm");
   mFmt.setByteOrder(QAudioFormat::LittleEndian);
   mFmt.setSampleType(QAudioFormat::SignedInt);

   QAudioDeviceInfo info(QAudioDeviceInfo::defaultOutputDevice());
   if (info.isFormatSupported(mFmt)) {
      qDebug() << "YES";
   }
   else {
      qDebug() << "NO";
   }

   mutex.lock(); 
   mbuffer.clear();
   mChannels = channels;
   mBitPerSample = rate;
   mutex.unlock();

   if (mOutput) {
      delete mOutput;
      mOutput = NULL;
   }

   mOutput = new QAudioOutput(mFmt, this);
   this->open(QIODevice::ReadOnly);
   connect(mOutput,SIGNAL(stateChanged(QAudio::State)),this,SLOT(stateChanged(QAudio::State)));
   mOutput->start(this);
}

bool VHCoreAudio::Reset(char *format, unsigned rate, unsigned channels) {
   emit SigReset(format,rate,channels);

   return true;
}

bool VHCoreAudio::PlayBack(const void *samples, unsigned count) {
   if(m_dalaLength<count) {
      if(m_pdata) {
         delete m_pdata;
         m_pdata = NULL;
      }
      m_dalaLength = count *mChannels * 16/8;
      m_pdata = new unsigned short [m_dalaLength];
   }

   unsigned tmpCount = m_dalaLength ;
   float *tmpSample = (float *)samples ; 
   unsigned short *tmpData = m_pdata;

   //PCM float ? signed 16
   while(tmpCount--) {
      float value = (*(tmpSample++))*32767.0f;      
      *(tmpData++) = unsigned short (value);
   }

   mutex.lock();
   if (mChannels&&mBitPerSample&&count) {
      mbuffer.append((const char *)m_pdata, m_dalaLength);
   }
   mutex.unlock();
   
   return true;
}

qint64 VHCoreAudio::readData(char *data, qint64 maxlen) {
   if (maxlen==0) {
      return 0;
   }
   qint64 ret=0;
   mutex.lock();
   if (mbuffer.length()>maxlen) {
      ret = maxlen;
      memcpy(data, mbuffer.data(), maxlen);
	  mbuffer.remove(0,maxlen);
   }
   else {
      ret = mbuffer.length();
      memcpy(data, mbuffer.data(), ret);
	  mbuffer.remove(0, ret);
   }
   mutex.unlock();
   return ret;

}

qint64 VHCoreAudio::writeData(const char *, qint64) {
   qDebug()<<__FUNCTION__;
   return 0;
}

//void VHCoreAudio::lookedUp(const QHostInfo &host)
//{
//	if (host.error() != QHostInfo::NoError) {
//		return;
//	}
//
//	foreach(QHostAddress address, host.addresses())
//	{
//		m_strLogSdIp = address.toString();
//		break;
//	}
//}

//QString VHCoreAudio::GetLogSdIp()
//{
//	return m_strLogSdIp;
//}
