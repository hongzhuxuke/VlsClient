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

//֡�ʷ�Χ
struct CameraSettingUIFrameInternal

{
   long long minInternal;
   long long maxInternal;
};
//�ֱ���
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
#define PAGE_COMMON_RESOLUTION            PAGE_STATMENT(PAGE_COMMON_RESOLUTION)     //�ֱ���
#define PAGE_COMMON_LINE                  PAGE_STATMENT(PAGE_COMMON_LINE)           //��·
#define PAGE_COMMON_SERVER_PLAYBACK       PAGE_STATMENT(PAGE_COMMON_SERVER_PLAYBACK)           //�ط�
#define PAGE_COMMON_DESKTOP_CAPTURE_TYPE  PAGE_STATMENT(PAGE_COMMON_DESKTOP_CAPTURE_TYPE)           //����ɼ�ģʽ

#define PAGE_VIDEO                        PAGE_STATMENT(PAGE_VIDEO)
#define PAGE_VIDEO_DEVICE                 PAGE_STATMENT(PAGE_VIDEO_DEVICE)          //�豸
#define PAGE_VIDEO_RESOLUTION             PAGE_STATMENT(PAGE_VIDEO_RESOLUTION)      //�ֱ���
#define PAGE_VIDEO_FPS                    PAGE_STATMENT(PAGE_VIDEO_FPS)             //֡��
#define PAGE_VIDEO_DEINTERLACE            PAGE_STATMENT(PAGE_VIDEO_DEINTERLACE)     //ȥ����
#define PAGE_VIDEO_HIGHQUALITYCOD         PAGE_STATMENT(PAGE_VIDEO_HIGHQUALITYCOD)     //����������
#define PAGE_VIDEO_QUALITY                PAGE_STATMENT(PAGE_VIDEO_QUALITY)         //��Ƶ��������
 

#define PAGE_AUDIO
#define PAGE_AUDIO_MIC                    PAGE_STATMENT(PAGE_AUDIO_MIC)             //��˷�
#define PAGE_AUDIO_ISNOISE                PAGE_STATMENT(PAGE_AUDIO_ISNOISE)         //�Ƿ���
#define PAGE_AUDIO_FORCEMONO              PAGE_STATMENT(PAGE_AUDIO_FORCEMONO)         //ǿ�Ƶ�����
#define PAGE_AUDIO_SPEAKER                PAGE_STATMENT(PAGE_AUDIO_SPEAKER)         //������

#define PAGE_AUDIO_KBPS              PAGE_STATMENT(PAGE_AUDIO_KBPS)													//��Ƶ����
#define PAGE_AUDIO_SAMPLERATE               PAGE_STATMENT(PAGE_AUDIO_SAMPLERATE)						//��Ƶ������
#define PAGE_AUDIO_NOISEREDUCTION              PAGE_STATMENT(PAGE_AUDIO_NOISEREDUCTION)		//��˷罵��
#define PAGE_AUDIO_MICGAIN               PAGE_STATMENT(PAGE_AUDIO_MICGAIN)									//��˷�����

#define PAGE_DESKTOP_ENHANCE                PAGE_STATMENT(PAGE_DESKTOP_ENHANCE)         //���湲����

#define PAGE_VT_FONT    PAGE_STATMENT(PAGE_VT_FONT)									//�����С
#define PAGE_VT_LAN     PAGE_STATMENT(PAGE_VT_LAN)									//����

#define PAGE_RECORD
#define PAGE_RECORD_ISRECORD              PAGE_STATMENT(PAGE_RECORD_ISRECORD)       //�Ƿ�¼���ļ�
#define PAGE_RECORD_FILENAME              PAGE_STATMENT(PAGE_RECORD_FILENAME)       //¼�Ƶ��ļ���
#define PAGE_RECORD_FILEPATH              PAGE_STATMENT(PAGE_RECORD_FILEPATH)       //�����·��
//#define PAGE_RECORD_FILESNUM              PAGE_STATMENT(PAGE_RECORD_FILESNUM)       //

#define PAGE_PROXY
#define PAGE_PROXY_OPEN                   PAGE_STATMENT(PAGE_PROXY_OPEN)            //�Ƿ�������
#define PAGE_PROXY_IP                     PAGE_STATMENT(PAGE_PROXY_IP)              //����IP
#define PAGE_PROXY_PORT                   PAGE_STATMENT(PAGE_PROXY_PORT)            //����˿�
#define PAGE_PROXY_USERNAME               PAGE_STATMENT(PAGE_PROXY_USERNAME)        //�����û���
#define PAGE_PROXY_PASSWORD               PAGE_STATMENT(PAGE_PROXY_PASSWORD)        //��������

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
   //��ȡ������
   int GetQuality();
	QString	GetQualityText();
   //��ȡ������·
   int GetPublishLine();

   //�������л�ҳ��
   void ChangePage(e_page_type eType);
   //��ʾ
   void Show(const QRect& rect, e_page_type eType, bool mode = true);
   //���
   void Clear();
   //���ûƵ����
   void SetActivityID(char* szActivityID);
   //���÷ֱ���
   void SetResolution(int nIndex);
   //��ӷֱ���
   void AddResolution(QString sResolution); 
   //���������·
   void AddPubLines(QString qsPubLine);
   //���õ�ǰ��·
   void SetCurLine(int iLine);
   //��ȡ��·����
   int GetCurPublishIndex();
   void DealLiveStatus(int liveStatus);
   //���µ�����Ϣ
   void UpdateDebugInfo(QString sServerIP, QString sBandWidth, QString sCurLostFrame, QString sAllLostFrame);
 
   //���ñ���·��
   void SetSavePath(QString qsSavePath);

   //��ȡ�����ļ���
   QString GetSaveFileName();

   //����¼��
   void SetRecord(bool bSaveMode);
   //void SetRecordFilesNum(const int& iFilesNum);
   //�Ƿ���¼��
   bool IsRecord();
   ////����ֱ��¼�����ݱ�������һ���ļ�
   //bool IsFilesNum();
   //��ȡ���ⷴ�����ͺ�����
   void GetQuestionInfo(int& nTpye, QString& sContent);

   //--------------------------------------------------����CameraSettingUI(��϶Ⱥܸ��޷����)---------------------------------------------------//
   //���豸
   void BindDevice(DeviceInfo &);
   //����豸��
   void AddCameraItem(DeviceInfo);
   //�����豸��
   void ClearItem();
   //����豸��
   void AddFrame(QString );
   //��ӷֱ���
   void SetResolutionList(FrameInfoList &);
   //���õ�ǰ�ֱ���
   void SetCurResolution(FrameInfo &);
   //���õ�ǰȥ��������
   void SetCurDeinterLacing(DeinterlacingType);
   //--------------------------------------------------����CameraSettingUI---------------------------------------------------//

   //��Ƶ����
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
   //ˢ�������б�
   void RefreshAskList();

   //����ͷ����
   void CameraSetting(DeviceInfo);
	int GetHighQualityCod();
   //��õ�ǰ�ı���·��
   QString GetCurrentSavePath();
   //�Ƿ���
   bool GetIsNoise();
   //��ȡ���뷧ֵ
   int GetThresHoldValue();
   //��ǰ��˷�
   DeviceInfo GetMicDeviceInfo();

	/*************��Ƶ����****************/
	int GetKbps();								//��Ƶ���� 
	int GetAudioSampleRate();			//��Ƶ������
	bool GetNoiseReduction();			//��         ��
	float GetMicGain();						//��˷�����
	/*************��Ƶ����****************/
	QString GetVedioResolution();
	QString GetFrame();
	QString GetDeinterLace();

   //����Ӧ����
   void SetCopyrightInfo(QString qsAppName, QString qsCopyright);

   //�����Ƿ�����1080P
   void EnabledExtraResolution(bool bEnable1080p);

   //ʹ�����ɻط�
   void EnabledServerPlayBack(bool);

   //void EnableParent(bool);

   //������Ϣ��ȡ
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
   //����
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
    //�л��豸
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

    //��������
    void OnMaxLength();
    //¼��ѡ��
    void OnRecordCheck(int);
    //��Ƶ��������
    void OnVideoQuality(int);
	//¼�������ļ�����ѡ��
	void On_recordFilesNum_stateChanged(int state);
    //�Ƿ���
    void OnIsNoiseChecked(int);
   void OnDesktopCaptureTypeChagned(int state);
    //�Ƿ��¼
    void OnServerPlaybackStatusChanged(int);
    //��������ַ����������
    void on_lineEdit_proxy_ip_textChanged(const QString &);
    
public slots:
    //¼���ļ���������
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


   //--------------------------------------------------����CameraSettingUI(��϶Ⱥܸ��޷����)---------------------------------------------------//
   CameraSettingUIResolutionList m_resolutionList;
   QList<UINT64> m_frameInternalList;
   int mStepValue;
   //DeviceInfo m_DeviceInfo;
   //--------------------------------------------------����CameraSettingUI(��϶Ⱥܸ��޷����)---------------------------------------------------//

   //-------------------Apply Button Status-----------------------
   int m_ApplyIdentify = 0 ;   
   ToolButtonEx *m_pApplyBtn = NULL;
   QMap<e_page_type,QJsonObject> m_ApplyInitialStatus;
	QString m_qsSavedDir;

    QString mPlayerDevId;
   std::map<QString,int> mDevBeautyLeavel;
};

#endif // SYSTEMSETTINGDLG_H
