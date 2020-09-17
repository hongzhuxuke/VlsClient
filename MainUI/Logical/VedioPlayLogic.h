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

   /////////////////////////////IUnknown�ӿ�/////////////////////////////////////////
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
   // �ض�λ�岥����
   virtual void STDMETHODCALLTYPE ReposVedioPlay(/*bool bShow*/);

   virtual void STDMETHODCALLTYPE StopAdmin(bool);


   // ������Ϣ
   void DealMessage(DWORD adwMessageID, void* apData, DWORD adwLen);

   // ����Click����
   void DealClickControl(void* apData, DWORD adwLen);

   // �����ŵ��
   void DealVedioPlayClick(void* apData, DWORD adwLen);

   // ����ֹͣ�岥
   void DealStopPlay(void* apData, DWORD adwLen);

   // ��������ļ�
   void DealAddFiles(void* apData, DWORD adwLen);

   // �������б�ı�
   void DealPlayListChg(void* apData, DWORD adwLen);

   // ���������ı�
   void DealVolumeChange(void* apData, DWORD adwLen);

private:
   //��ʾ�岥����
   int ShowVedioPlayWidget(BOOL bShow);

   //��Ӳ����ļ�
   void AddPlayFiles();

   //����ѡ���ļ�
   void PlaySelectedFile();

   //��ʾʱ���ת��
   QString dulation2Str(long long currDulation, long long maxDulation);

   QString dulation2StrPri(long long dulation);

private slots:
   //ѭ������
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
   //ѭ�����Ŷ�ʱ��
   QTimer m_qCycleTimer;

   long	m_lRefCount;
   BOOL m_bShowState;      //�岥��ʾ״̬
   bool m_bForceHide;
   bool m_bPlayCurrentFile = false;
   int mLiveType = -1;
   QString mLastPlayFile;
   bool mbIsEnablePlayOutAudio = false;
   bool mbIsPlayEnd = true;
};

#endif //__VEDIO_PLAY_LOGIC__H_INCLUDE__S
