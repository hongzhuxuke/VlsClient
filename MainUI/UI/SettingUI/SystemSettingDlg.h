#ifndef SYSTEMSETTINGDLG_H
#define SYSTEMSETTINGDLG_H

#include <QDialog>
#include <QMap>
#include <QJsonObject>
#include "ui_SystemSettingUI.h"
#include "VHDialog.h"
#include "VH_ConstDeff.h"
#include "NoiseTooltip.h"

#include "NoiseSettingSlider.h"
#include "IOBSControlLogic.h"

#include "Msg_MainUI.h"
#include "Msg_OBSControl.h"
#include "Msg_CommonToolKit.h"

//帧率范围
struct CameraSettingUIFrameInternal

{
   long long minInternal;
   long long maxInternal;
};
//分辨率
struct CameraSettingUIResolution {
   int w;
   int h;
   VideoFormat format;
   QList<CameraSettingUIFrameInternal> frameInternalList;
};

class CameraSettingUIResolutionList
{
public:
   void Clear();
   void Append(FrameInfo);
   int Count();
   bool GetFrameInfo(int, CameraSettingUIResolution *);
   QString DisplayString(int);
   int SetCurrentFrameInfo(FrameInfo);
   UINT64 GetCurrentFrameInternal(QString);
   VideoFormat GetCurrentVideoFormat();
   bool GetCurrentResolution(int &w, int &h);
   void sort();
   void SetCurrentIndex(int);
private:
   QList<CameraSettingUIResolution> resList;
   QList<UINT64> m_frameInternalList;
   int m_currentIndex = 0;
};

enum e_page_type {
   e_page_common  = 0x1<<0,
   e_page_vedio   = 0x1<<1,
   e_page_audio   = 0x1<<2,
   e_page_ai      = 0x1<<3,
   e_page_record  = 0x1<<4,
   e_page_voicetranslate = 0x1 << 5,
   e_page_proxy   = 0x1<<6,
   e_page_debug   = 0x1<<7,
   e_page_question= 0x1<<8,
   e_page_aboutUs = 0x1<<9,
};

#define PAGE_STATMENT(X) #X
#define PAGE_COMMON  
#define PAGE_COMMON_RESOLUTION            PAGE_STATMENT(PAGE_COMMON_RESOLUTION)     //分辨率
#define PAGE_COMMON_LINE                  PAGE_STATMENT(PAGE_COMMON_LINE)           //线路
#define PAGE_COMMON_SERVER_PLAYBACK       PAGE_STATMENT(PAGE_COMMON_SERVER_PLAYBACK)           //回放
#define PAGE_COMMON_DESKTOP_CAPTURE_TYPE  PAGE_STATMENT(PAGE_COMMON_DESKTOP_CAPTURE_TYPE)           //桌面采集模式

#define PAGE_VIDEO                        PAGE_STATMENT(PAGE_VIDEO)
#define PAGE_VIDEO_DEVICE                 PAGE_STATMENT(PAGE_VIDEO_DEVICE)          //设备
#define PAGE_VIDEO_RESOLUTION             PAGE_STATMENT(PAGE_VIDEO_RESOLUTION)      //分辨率
#define PAGE_VIDEO_FPS                    PAGE_STATMENT(PAGE_VIDEO_FPS)             //帧率
#define PAGE_VIDEO_DEINTERLACE            PAGE_STATMENT(PAGE_VIDEO_DEINTERLACE)     //去交错
#define PAGE_VIDEO_HIGHQUALITYCOD         PAGE_STATMENT(PAGE_VIDEO_HIGHQUALITYCOD)     //高质量编码
#define PAGE_VIDEO_QUALITY                PAGE_STATMENT(PAGE_VIDEO_QUALITY)         //视频质量评估
 

#define PAGE_AUDIO
#define PAGE_AUDIO_MIC                    PAGE_STATMENT(PAGE_AUDIO_MIC)             //麦克风
#define PAGE_AUDIO_ISNOISE                PAGE_STATMENT(PAGE_AUDIO_ISNOISE)         //是否降噪
#define PAGE_AUDIO_FORCEMONO              PAGE_STATMENT(PAGE_AUDIO_FORCEMONO)         //强制单声道
#define PAGE_AUDIO_SPEAKER                PAGE_STATMENT(PAGE_AUDIO_SPEAKER)         //扬声器

#define PAGE_AUDIO_KBPS              PAGE_STATMENT(PAGE_AUDIO_KBPS)													//音频码率
#define PAGE_AUDIO_SAMPLERATE               PAGE_STATMENT(PAGE_AUDIO_SAMPLERATE)						//音频采样率
#define PAGE_AUDIO_NOISEREDUCTION              PAGE_STATMENT(PAGE_AUDIO_NOISEREDUCTION)		//麦克风降噪
#define PAGE_AUDIO_MICGAIN               PAGE_STATMENT(PAGE_AUDIO_MICGAIN)									//麦克风增益

#define PAGE_DESKTOP_ENHANCE                PAGE_STATMENT(PAGE_DESKTOP_ENHANCE)         //桌面共享锐化

#define PAGE_VT_FONT    PAGE_STATMENT(PAGE_VT_FONT)									//字体大小
#define PAGE_VT_LAN     PAGE_STATMENT(PAGE_VT_LAN)									//语言

#define PAGE_RECORD
#define PAGE_RECORD_ISRECORD              PAGE_STATMENT(PAGE_RECORD_ISRECORD)       //是否录制文件
#define PAGE_RECORD_FILENAME              PAGE_STATMENT(PAGE_RECORD_FILENAME)       //录制的文件名
#define PAGE_RECORD_FILEPATH              PAGE_STATMENT(PAGE_RECORD_FILEPATH)       //保存的路径
//#define PAGE_RECORD_FILESNUM              PAGE_STATMENT(PAGE_RECORD_FILESNUM)       //

#define PAGE_PROXY
#define PAGE_PROXY_OPEN                   PAGE_STATMENT(PAGE_PROXY_OPEN)            //是否开启代理
#define PAGE_PROXY_IP                     PAGE_STATMENT(PAGE_PROXY_IP)              //代理IP
#define PAGE_PROXY_PORT                   PAGE_STATMENT(PAGE_PROXY_PORT)            //代理端口
#define PAGE_PROXY_USERNAME               PAGE_STATMENT(PAGE_PROXY_USERNAME)        //代理用户名
#define PAGE_PROXY_PASSWORD               PAGE_STATMENT(PAGE_PROXY_PASSWORD)        //代理密码

class TitleWidget;
class QHBoxLayout;
class ToolButton;
class ToolButtonEx;
class SystemSettingDlg : public VHDialog {
   Q_OBJECT

public:
   SystemSettingDlg(QDialog *parent = 0);
   ~SystemSettingDlg();

   bool Create();
   void Destroy();

	QString GetSavedDir();
	void SetSavedDir(const QString& strSavedDir);

   void Init();
   //获取清晰度
   int GetQuality();
	QString	GetQualityText();
   //获取推流线路
   int GetPublishLine();

   //按类型切换页面
   void ChangePage(e_page_type eType);
   //显示
   void Show(const QRect& rect, e_page_type eType, bool mode = true);
   //清空
   void Clear();
   //设置活动频道号
   void SetActivityID(char* szActivityID);
   //设置分辨率
   void SetResolution(int nIndex);
   //添加分辨率
   void AddResolution(QString sResolution); 
   //添加推流线路
   void AddPubLines(QString qsPubLine);
   //设置当前线路
   void SetCurLine(int iLine);
   //获取线路索引
   int GetCurPublishIndex();
   void DealLiveStatus(int liveStatus);
   //更新调试信息
   void UpdateDebugInfo(QString sServerIP, QString sBandWidth, QString sCurLostFrame, QString sAllLostFrame);
 
   //设置保存路径
   void SetSavePath(QString qsSavePath);

   //获取保存文件名
   QString GetSaveFileName();

   //设置录制
   void SetRecord(bool bSaveMode);
   //void SetRecordFilesNum(const int& iFilesNum);
   //是否开启录制
   bool IsRecord();
   ////本次直播录制内容本地生成一个文件
   //bool IsFilesNum();
   //获取问题反馈类型和内容
   void GetQuestionInfo(int& nTpye, QString& sContent);

   //--------------------------------------------------来自CameraSettingUI(耦合度很高无法拆分)---------------------------------------------------//
   //绑定设备
   void BindDevice(DeviceInfo &);
   //添加设备项
   void AddCameraItem(DeviceInfo);
   //清理设备名
   void ClearItem();
   //添加设备项
   void AddFrame(QString );
   //添加分辨率
   void SetResolutionList(FrameInfoList &);
   //设置当前分辨率
   void SetCurResolution(FrameInfo &);
   //设置当前去交错设置
   void SetCurDeinterLacing(DeinterlacingType);
   //--------------------------------------------------来自CameraSettingUI---------------------------------------------------//

   //音频设置
   void SetMicVolume(float fVolume);
   void SetSpeakerVolume(float fVolume);
   //void SetEnhanceLeve(float fLevel);

   void AddMicItem(wchar_t* wzDeviceName, DeviceInfo itemData);
   bool GetCurMicItem(DeviceInfo& itemData);
   void SetCurMicItem(DeviceInfo& itemData);
   void ClearMicItem();
   bool GetMicItem(DeviceInfo& itemData,int index);
 
   void AddSpeakerItem(wchar_t* wzDeviceName, wchar_t* wzDeviceID);
   bool GetCurSpeakerItem(wchar_t* wzDeviceID);
   void ClearSpeakerItem();
   void InitEnhanceParam(int nPageStep, int nSingleStep, int nMin, int nMax, int nValue, int);

   bool GetForceMono();

   void GetRecordFilePath(wchar_t *);
   void SetVersion(QString qsVersion);

   void ClearReportContent();

   bool GetCurrentCameraDeviceInfo(DeviceInfo &info);
   void SetSaveFileName(QString);
   //刷新问题列表
   void RefreshAskList();

   //摄像头设置
   void CameraSetting(DeviceInfo);
	int GetHighQualityCod();
   //获得当前的保存路径
   QString GetCurrentSavePath();
   //是否降噪
   bool GetIsNoise();
   //获取降噪阀值
   int GetThresHoldValue();
   //当前麦克风
   DeviceInfo GetMicDeviceInfo();

	/*************音频设置****************/
	int GetKbps();								//音频码率 
	int GetAudioSampleRate();			//音频采样率
	bool GetNoiseReduction();			//降         噪
	float GetMicGain();						//麦克风增益
	/*************视频设置****************/
	QString GetVedioResolution();
	QString GetFrame();
	QString GetDeinterLace();

   //设置应用名
   void SetCopyrightInfo(QString qsAppName, QString qsCopyright);

   //设置是否允许1080P
   void EnabledExtraResolution(bool bEnable1080p);

   //使能生成回放
   void EnabledServerPlayBack(bool);

   //void EnableParent(bool);

   //代理信息获取
   bool GetIsEnableProxy();
   QString GetProxyIP();
   QString GetProxyPort();
   QString GetProxyUserName();
   QString GetProxyPwd();
   void SetCutRecordDisplay(const int iCutRecord);
   void SetDesktopShare(bool isCapture);
   void SetLiveState(bool isLiveing);

   void SelectCamera(DeviceInfo &info);
   void RemoveCamera(DeviceInfo &info);

protected:
   //void paintEvent(QPaintEvent *);
   bool eventFilter(QObject *obj, QEvent *ev);
private slots:   
   void CheckApplyStatus();

private:
   void RefreshDeinterLace();
   int PageTypeToId(e_page_type);
   e_page_type PageIdToType(int);
   void show();   
   void CenterWindow(QWidget* parent);
   QJsonObject InitPageStatus(e_page_type);
   QJsonObject ApplyPageStatus(e_page_type);
   void ApplySettings();
   void SaveNoiseSet();
   QJsonObject GetPageData(e_page_type);
   void ReloadCameraAtttibute(DeviceInfo deviceInfo);
   void ReloadCameraFps(int);
   void ReloadCameraResolution(FrameInfoList &frameInfoList,FrameInfo currentFrameInfo);
   void ReloadCameraDeinterLace(DeinterlacingType);
   void RecheckFps();
   bool GetCameraDeviceInfo(int,DeviceInfo&);
   void UIEnableProxy();
   void UIDisableProxy();
   //降噪
   void EnableNoiceUI(bool enable);
public slots:
	 void on_cmbKbps_currentTextChanged(const QString & text);
	 void on_cmbAudioSampleRate_currentTextChanged(const QString & text);
	 void on_ckbNoiseReduction_stateChanged(int state);
	 void on_micGainSld_valueChanged(int value);
	 void on_btnReStore_clicked(bool checked = false);
	 //void slot_widget_noiseSetting_SignalNoiseValueChange(int);
	 void on_cmbHighQualityCod_currentTextChanged(const QString & text);
	 

    void on_savedDirBtn_clicked();
    void on_openCurDir_clicked();
    void on_confirm_clicked();
    void on_apply_clicked();
    void on_comboBox_proxyEnable_currentIndexChanged(int index);
    void on_btn_testing_clicked();

    void OnButtonJudge(int);
    void OnClose();   
    void OnDeviceOption();
    void OnMicVolumeChange(int);
    void OnSpeakerVolumeChange(int);
    //void OnMicEnhanceChanged(int);

    void OnMicMuteClick();
    void OnSpeakerMuteClick();
    void OnMicListChanged(int);
    void OnSpeakerListChanged(int);
    void OnSoundChanelListChanged(int);
    void OnDesktopEnhanceChanged(int);
    void OnBeautyValueChange(int value);
    //切换设备
    void OnChgDevice(int nIndex);
    void OnBeautyDeviceSelect(int index);
    void OnResolutionIndexChg(int nIndex);
    void OnFrameChg(int nIndex);
    void OnDetainterChg(int nIndex);
    void OnCommonResolutionChg(int nIndex);
    void OnLinesChg(int nIndex);

    void OnVTLanguagenChg(int);
    void OnVTFontSizeChg(int);
    void OnVTReset();

    //问题内容
    void OnMaxLength();
    //录制选中
    void OnRecordCheck(int);
    //视频画面质量
    void OnVideoQuality(int);
	//录制生成文件个数选择
	void On_recordFilesNum_stateChanged(int state);
    //是否降噪
    void OnIsNoiseChecked(int);
   void OnDesktopCaptureTypeChagned(int state);
    //是否记录
    void OnServerPlaybackStatusChanged(int);
    //网络代理地址栏输入内容
    void on_lineEdit_proxy_ip_textChanged(const QString &);
    
public slots:
    //录制文件名输入检测
void CheckInputText(const QString &text);

private:
   void ModifyLimitText(QString text);

private:
   Ui::SystemSettingDlg ui;
   TitleWidget *m_pTitleBar = NULL;
   QButtonGroup *m_pButtonGroup =NULL;
   //ToolButton *m_pDeviceOption;
   ToolButtonEx *m_pChangeDir = NULL;
   ToolButtonEx *m_pOpenCurDir = NULL;
   QObject *currentParent = NULL;
   QObject *bakParent = NULL;
   NoiseTooltip* m_pNoiseTips = NULL;
   DeviceInfo mCurrentBeautyDev;
   //bool m_bMouseCtrl;


   //--------------------------------------------------来自CameraSettingUI(耦合度很高无法拆分)---------------------------------------------------//
   CameraSettingUIResolutionList m_resolutionList;
   QList<UINT64> m_frameInternalList;
   int mStepValue;
   //DeviceInfo m_DeviceInfo;
   //--------------------------------------------------来自CameraSettingUI(耦合度很高无法拆分)---------------------------------------------------//

   //-------------------Apply Button Status-----------------------
   int m_ApplyIdentify = 0 ;   
   ToolButtonEx *m_pApplyBtn = NULL;
   QMap<e_page_type,QJsonObject> m_ApplyInitialStatus;
	QString m_qsSavedDir;

    QString mPlayerDevId;
   std::map<QString,int> mDevBeautyLeavel;
};

#endif // SYSTEMSETTINGDLG_H
