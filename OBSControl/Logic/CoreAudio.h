#ifndef  VHCORE_AUDIO_H_
#define  VHCORE_AUDIO_H_
#include <QAudioFormat>  
#include <QAudioOutput> 
#include <QFile>
#include <mutex>
//#include <QHostInfo>

class VHCoreAudio:public QIODevice {
   
   Q_OBJECT

public :
   VHCoreAudio();
   ~VHCoreAudio();
   bool Reset(char *format, unsigned rate, unsigned channels);
   bool PlayBack(const void *samples, unsigned count);
	//QString GetLogSdIp();
signals:
   void SigReset(char *format, unsigned rate, unsigned channels);
public slots:
   void SlotReset(char *format, unsigned rate, unsigned channels);
   void SlotFinished();
   void SlotClose();
   void stateChanged(QAudio::State);
	//void lookedUp(const QHostInfo &host);
protected:
   qint64 readData(char *data, qint64 maxlen);
   qint64 writeData(const char *, qint64);
private:
   QAudioFormat mFmt;
   QAudioOutput *mOutput;
   QByteArray mbuffer;
   std::mutex mutex;
   int mChannels=0;
   int mBitPerSample=0;
   unsigned short *m_pdata = NULL;
   int m_dalaLength = 0;
	//QString m_strLogSdIp;
};

#endif
