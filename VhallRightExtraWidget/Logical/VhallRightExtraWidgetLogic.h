#ifndef _VHALLRIGHTEXTRAWIDGETLOGIC_H_
#define _VHALLRIGHTEXTRAWIDGETLOGIC_H_
#include <QObject>
#include "IVhallRightExtraWidgetLogic.h"

#include "vhinteraction.h"
#include <QTimer>
#include "pub.Struct.h"

//#define WEBINAR_NOTICE  QStringLiteral("����")
//typedef enum REBarShowMode_e {
//   REBarShowMode_Shrink,     //����״̬
//   REBarShowMode_Display,    //��ʾ״̬
//   REBarShowMode_Other
//}REBarShowMode;

#define CREATE_RECORD_TIME  30000//30���ڷ�������������¼���ļ�

class VhallRightExtraWidgetLogic :public QObject, public IVhallRightExtraWidgetLogic {
   Q_OBJECT
public:
   VhallRightExtraWidgetLogic(void);
   ~VhallRightExtraWidgetLogic(void);

public:
   BOOL Create();
   void Destroy();

   /////////////////////////////IUnknown�ӿ�/////////////////////////////////////////
   virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void ** appvObject);
   virtual ULONG STDMETHODCALLTYPE   AddRef(void);
   virtual ULONG STDMETHODCALLTYPE   Release(void);
   //virtual void STDMETHODCALLTYPE MouseMoved();
   virtual void STDMETHODCALLTYPE GetStartStreamUrl(wchar_t *);   
   virtual void STDMETHODCALLTYPE Refresh();
   //virtual void STDMETHODCALLTYPE ShowExtraWidget(bool bShow = true);
   virtual bool STDMETHODCALLTYPE GetCreateRecordState();
   virtual int STDMETHODCALLTYPE AskGenerateRecord(const char* stopId = NULL);
   virtual bool STDMETHODCALLTYPE GenRecord();
   virtual bool STDMETHODCALLTYPE StartWebnair();
   virtual int STDMETHODCALLTYPE StopWebnair(bool bIsCheckShortTime = true);
   virtual void STDMETHODCALLTYPE GetLastError(wchar_t *);
   virtual int STDMETHODCALLTYPE GetLastCode(); 
   virtual void STDMETHODCALLTYPE StartTimer();
   virtual char* STDMETHODCALLTYPE GetMsgToken();
   virtual int STDMETHODCALLTYPE CreateRecord();
   // ������Ϣ
   void DealMessage(DWORD adwMessageID, void* apData, DWORD adwLen);
   bool Get(QString method,QString param,bool);
   

signals:
   void SigAskServerPlayback(bool);
   void SigCanNotCreateRecord(bool);
   void SigStopDesktopSharing();
private:
   bool IsStopRecordState();
   bool commitRecord();
   //void DealStartPublish(void* apData, DWORD adwLen);
   void DealInitCommonInfo(void* apData, DWORD adwLen);
   void StopCreateRecordTimer();
   void StartCreateRecordTimer();
public slots:
   void SlotAskServerPlayback(bool);
   //void ExBarClicked(REBarShowMode);
   //void SlotClickedTest();
   void SlotUnInitFinished();
private slots:
   void SlotCreateRecordTimeOut();
   void SlotCanNotCreateRecord(bool);
private:
   long  m_lRefCount;
   QString m_msgToken;
   QString m_msgUrl;
   Key_Data oKeyData;
   QTimer *m_pCreateRecordTimer;
   bool m_bCreateRecord;
   QString m_stopId ;
   QString m_lastGetError;
   int m_lastGetCode;
};

#endif //_VHALLRIGHTEXTRAWIDGETLOGIC_H_
