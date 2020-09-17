#ifndef TOOLBARWIDGET_H
#define TOOLBARWIDGET_H

#include <QStringList>
#include <QRubberBand>
#include <map>
#include "CWidget.h"

using namespace std;

class QLabel;
class VolumeDisplayWiget;

class TimerClock;
class HoverLabel;
class ToolButton;
class QHBoxLayout;
class AudioSliderUI;
class VhallWaiting;
class QSpacerItem;
class StartButton;
class VhallLiveMainDlg;
class ButtonModTextWdg;
class ButtonWdg;
class QToolButton;

class ToolBarWidget : public CWidget {
   Q_OBJECT
public:
   explicit ToolBarWidget(QWidget *parent = 0);
   ~ToolBarWidget();
   void setLiveMainDlg(VhallLiveMainDlg* pLiveMainDlg)
   {
	   m_pLiveMainDlg = pLiveMainDlg;
   }

   bool Create();

   void Destroy();

   void AddVoiceTranslate();
   void SetVoiceTranslateBtnState(bool open);
   void RemoveVoiceTranslate();
	void SetStreamButtonStatus(bool isActive);
	void SetMicMute(bool bMute);
	void UpdateVolumn(int/*, StartButton* pbtn*/);
    //void UpdateStreamSpeedUI(QString qStrStreamSpeed, int stageValue);
    void UpdateStreamStateUI(QString qStrStreamState, int iStreamCnt);
    //void StartTimeClock(bool bStart);
    //void ClearTimeClock();
    int GetPlayTime();
    void SetSpeakerMute(bool bMute);
    //void SetDesktopSharing(bool bShow) ;
    //void ShowMemberListBtn(bool show);
    //void ShowLiveTool(bool);
    void logCPUAndMemory(int);
    
    void SetStartLiveState(bool start);
    //void SetMemberListShow(bool bShow = true);
signals:
   void sigMemberListClicked();
   void sigClicked();
   void sigMicStateChange(const bool& bState);
   void sigClickedCamera();
public slots:
	void micBtnClicked();
private slots:
   void updateVolumeDisplay();

   void showStreamStatus();
   void hideStreamStatus();
   void speakerBtnClicked();
   void speakerBtnEnter();
   //void cameraBtnEnter();
   
   void multiMediaEnter();
   void SetSpeakerVolumn(int v);
   
   void SlotCameraClicked();
   void liveToolClicked();
   void Slot_RecordMicValue();
   void SetMicVolumn(int);
   void micBtnEnter();
	void OnVedioPlayClick();
	void OnAddImageClick();
	void OnAddTextClick();
	void OnVoiceTranslateClick();

private:
	VhallLiveMainDlg* m_pLiveMainDlg = NULL;
   QLabel *m_pStreamStatus;
   QHBoxLayout* m_pMainLayout = NULL;
   int m_iRtmpStreamCount;
   QTimer *mVoiceTimer;
   AudioSliderUI* m_pSpeakerSliderUI = NULL; 
   bool m_bSpeakerShow = false;
   bool m_bMultiMediaShow = false;

   ButtonModTextWdg* m_pCameraBtnMTW;		//�����豸
   /*�˴��Ѷ�ý���*/
   ButtonWdg* mpInsertVedio = NULL;	//������Ƶ
   ButtonWdg* mpInsertImage = NULL; //����ͼƬ
   ButtonWdg* mpInsertText = NULL;		//�����ı�

   QLabel* mpVoiceTranslateLabel = NULL;
   QToolButton*mpVoiceTranslate = NULL; //ʵʱ��Ļ
   QLabel* mpMicLabel = NULL;
   QToolButton *m_pMicButton = NULL;						//���

   QList<QSpacerItem*> mListSpacerItem;
   QTimer *m_pMicValueTimer = NULL;
   bool m_bRecordMicValue = true;
   bool m_bMicShow = false;
   bool m_bMicMute = false;
   int iOldMicVal = 100;
};

#endif // TOOLBARWIDGET_H
