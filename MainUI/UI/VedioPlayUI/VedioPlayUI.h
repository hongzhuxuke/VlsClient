#ifndef VEDIOPLAYUI_H
#define VEDIOPLAYUI_H

#include <QWidget>
#include <QPushButton>
#include <QTimer>
#include <QAbstractAnimation>
enum CycleType {
   CycleTyle_None,
   CycleTyle_List,
   CycleTyle_Single,
   CycleTyle_END
};

class VHallProcessSlider;
class PushButton;
class QLabel;
class QToolButton;
class ToolButton;
class VHallMenuWidget;
class QSlider;
class QBoxLayout;
class VHallMenuWidgetItem;
class QPropertyAnimation;

class VedioPlayUI : public QWidget {
   Q_OBJECT
public:
   VedioPlayUI(QWidget *parent);
   ~VedioPlayUI();

public:
   bool Create();
   void Destroy();

   //��ʼ�������б�
   void InitPlayList();

   //����ʼ�������б�
   void UnInitPlayList();

   //��ʼ��ѭ���б�
   void InitCycleControl();

   //����ʼ��ѭ���б�
   void UnInitCycleControl();

   //����ֵfalse����գ�true�����пɲ��ŵ��ļ�
   bool GetCurFile(QString& selectFile);

   void AddPlayList(char* fileName);

   void AppendPlayList(QStringList &playList,int);

   //�л���һ��
   void SwitchToNext();

   //���ص�ǰ������С
   int GetCurVolume();

   //���ص�ǰ����ģʽ
   CycleType GetCurPlayMode();
   //���õ�ǰ����ģʽ
   void SetCurPlayMode(CycleType type);
   
   //���²���ʱ��
   void UpdatePlayTime(QString qPlayTime);

   //���²���״̬
   void UpdatePlayStates(bool bPlay);

   //�Ƿ�ﵽ�б�β��
   bool IsArriveTheEnd();

   //���ò��Ž���ֵ
   void SetProgressValue(int nCurValue, int nMaxValue);

   //�����б�����
   int PlayListNum();

   //��ò����б�
   QStringList GetPlayList(int &currentIndex);   
   //ֹͣ����
   void StopAdmin();
   void StopAdminIMMediately(bool);
   //ˢ�°�ť״̬
   void FlushStatus();
   void SetCaptureType(int type);

   void TryShowToolBar();
   void TryHideToolBar(int pauseMsec=300);
   bool IsShow();
   void Enabled(bool);
public slots:
	void show();
	void hide();
private slots:
    void slot_HideUI();
   //���Ž��ȸı�
   void OnProgressChanged(int);

   //�����ı��С
   void OnVolumnChanged(int);
   void Slot_HideTimer();
   //���ŵ��
   void OnPlayClicked();

   //����ļ�
   void OnAddFiles();

   //ֹͣ�岥
   void OnStopPlay();

   //��ʾ�����б�
   void OnShowPlayList();

   //���ز����б�
   void OnHidePlayList();

   //�����б���
   void OnPlayListClick();

   //��ʾ����ģʽ�˵�
   void OnShowPlayModeMenu();

   //���ز���ģʽ�˵�
   void OnHidePlayModeMenu();

   //����ģʽ�˵����
   void OnPlayModeMenuClick();

   //���ѭ������
   void OnCycleControl(VHallMenuWidgetItem *);

   //�����б�ѡ��
   void OnPlayListChecked(VHallMenuWidgetItem *);

   //����
   void OnMuteClicked();

   //����岥�б�
   void OnEnterFileList();

   //�����ļ�ѭ���б�
   void OnEnterCircleList();

   //�岥����������ģʽ
   void SetMiniMode(bool bMini,int dulation=400);

   //
   void SlotAdminFinished();

   //
   void LeaveTimerTimeout();

   //
   void FlushCircleButton(bool bShow=true);

protected:
   virtual void paintEvent(QPaintEvent *);
   virtual void hideEvent(QHideEvent *);
   virtual void resizeEvent(QResizeEvent *);
   virtual void enterEvent(QEvent *);
   virtual void leaveEvent(QEvent *);
   bool eventFilter(QObject *obj, QEvent *ev);
	void mousePressEvent(QMouseEvent * event);
   void SetDeviceSystemVolume(int volume);

signals:
   void SigRepos(bool);
   void SigForceHide(bool);
public slots:
   void SlotSetForceActice(bool);
   void SlotPlayListClose();
   void SlotFlushPlayList(QStringList,int current);
   //void stateChanged(QAbstractAnimation::State,QAbstractAnimation::State);
private:
   //���Ž�����
   VHallProcessSlider *m_pProgressSlider = NULL;
   //��ʼ�岥
   PushButton* m_pPlayBtn = NULL;
   //ʱ�䰴ť
   QLabel *m_pTimeLbl = NULL;
   //������ť
   PushButton *m_pVolumeBtn = NULL;
   //����ļ���ť
   ToolButton *m_pAddFileBtn = NULL;
   //����������
   QSlider *m_pVolumeSlider = NULL;
   //�ļ��б�ť
   PushButton *m_pFileListBtn = NULL;
   //ѭ����ť
   ToolButton *m_pLoopSettingBtn = NULL;
   //�����岥
   ToolButton *m_pExitBtn = NULL;
   //����
   QPushButton *mHideBtn = NULL;
    
   //ѭ�����ƴ���
   VHallMenuWidget *m_pMenuCircleControl = NULL;
   //�ļ��б���
   VHallMenuWidget *m_pMenuPlayList = NULL;

   QBoxLayout* m_pControlLayout;
   QBoxLayout* m_pVolumeLayout;
   QBoxLayout* m_pMainLayout;

   QWidget* m_pControlWidget = NULL;


   //ѭ������
   CycleType m_CycleType;

   unsigned int m_iVolume;
   //bool m_bIsPlay;               //�Ƿ����ڲ���
   bool m_bIsMute;
   bool m_bIsFileListShow = false;
   bool m_bIsMenuCircleControlShow = false;
   bool m_bMiniMode = false;
   bool m_bEnterEventActive = true;
   bool m_bClickedHide = false;
   bool m_bForceEventActive = true;
   bool m_bForceHideAdmin = false;
   
   QPropertyAnimation *m_propertyAdmin = NULL;
   QTimer m_LeaveTimer;
   QWidget *m_pPadding = NULL;
   QString m_currentFile = "";
   int mLiveType = 0;
   QTimer* mShowHideTimer = nullptr;
};

#endif // VEDIOPLAYUI_H
