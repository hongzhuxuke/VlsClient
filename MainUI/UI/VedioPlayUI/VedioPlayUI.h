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

   //初始化播放列表
   void InitPlayList();

   //反初始化播放列表
   void UnInitPlayList();

   //初始化循环列表
   void InitCycleControl();

   //反初始化循环列表
   void UnInitCycleControl();

   //返回值false代表空，true代表有可播放的文件
   bool GetCurFile(QString& selectFile);

   void AddPlayList(char* fileName);

   void AppendPlayList(QStringList &playList,int);

   //切换下一项
   void SwitchToNext();

   //返回当前声音大小
   int GetCurVolume();

   //返回当前播放模式
   CycleType GetCurPlayMode();
   //设置当前播放模式
   void SetCurPlayMode(CycleType type);
   
   //更新播放时间
   void UpdatePlayTime(QString qPlayTime);

   //更新播放状态
   void UpdatePlayStates(bool bPlay);

   //是否达到列表尾部
   bool IsArriveTheEnd();

   //设置播放进度值
   void SetProgressValue(int nCurValue, int nMaxValue);

   //播放列表数量
   int PlayListNum();

   //获得播放列表
   QStringList GetPlayList(int &currentIndex);   
   //停止动画
   void StopAdmin();
   void StopAdminIMMediately(bool);
   //刷新按钮状态
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
   //播放进度改变
   void OnProgressChanged(int);

   //音量改变大小
   void OnVolumnChanged(int);
   void Slot_HideTimer();
   //播放点击
   void OnPlayClicked();

   //添加文件
   void OnAddFiles();

   //停止插播
   void OnStopPlay();

   //显示播放列表
   void OnShowPlayList();

   //隐藏播放列表
   void OnHidePlayList();

   //播放列表点击
   void OnPlayListClick();

   //显示播放模式菜单
   void OnShowPlayModeMenu();

   //隐藏播放模式菜单
   void OnHidePlayModeMenu();

   //播放模式菜单点击
   void OnPlayModeMenuClick();

   //点击循环控制
   void OnCycleControl(VHallMenuWidgetItem *);

   //播放列表选中
   void OnPlayListChecked(VHallMenuWidgetItem *);

   //静音
   void OnMuteClicked();

   //进入插播列表
   void OnEnterFileList();

   //进入文件循环列表
   void OnEnterCircleList();

   //插播进度条精简模式
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
   //播放进度条
   VHallProcessSlider *m_pProgressSlider = NULL;
   //开始插播
   PushButton* m_pPlayBtn = NULL;
   //时间按钮
   QLabel *m_pTimeLbl = NULL;
   //音量按钮
   PushButton *m_pVolumeBtn = NULL;
   //添加文件按钮
   ToolButton *m_pAddFileBtn = NULL;
   //音量进度条
   QSlider *m_pVolumeSlider = NULL;
   //文件列表按钮
   PushButton *m_pFileListBtn = NULL;
   //循环按钮
   ToolButton *m_pLoopSettingBtn = NULL;
   //结束插播
   ToolButton *m_pExitBtn = NULL;
   //隐藏
   QPushButton *mHideBtn = NULL;
    
   //循环控制窗体
   VHallMenuWidget *m_pMenuCircleControl = NULL;
   //文件列表窗体
   VHallMenuWidget *m_pMenuPlayList = NULL;

   QBoxLayout* m_pControlLayout;
   QBoxLayout* m_pVolumeLayout;
   QBoxLayout* m_pMainLayout;

   QWidget* m_pControlWidget = NULL;


   //循环类型
   CycleType m_CycleType;

   unsigned int m_iVolume;
   //bool m_bIsPlay;               //是否正在播放
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
