#ifndef H_CRUNTIMEINSTANCE_H
#define H_CRUNTIMEINSTANCE_H

//#include <QWebEngineView>
#include<QObject>
#include "Singleton.h"
#include "MemPoolBase.h"
#include "IPluginManager.h"
#include "IMessageDispatcher.h"


class QTimer;
class CCheckVersionDlg;
class  SocketClient;

//����ʵ��
class CRuntimeInstance : public QObject, public IMessageEvent
{
   Q_OBJECT
public:
   CRuntimeInstance(void);
   ~CRuntimeInstance(void);

public:
   void InitCurVersion();
   // ����
   HRESULT Create();

   // ����
   HRESULT Destory();
   //�������
   HRESULT CheckParam(int, char *argv[]);

   // ����
   HRESULT StartSystem(bool *);
   // ����
   HRESULT SetSocketClient(FuncNotifyService);

   // �ر�
   HRESULT ShutdownSystem();

   // ��ȡStreamName
   string GetStreamName();

   // Ԥ����
   BOOL PreTranslateMessage(MSG* pMsg);

   // Ͷ����Ϣ
   HRESULT PostCRMessage(DWORD adwMessageID, void * apData, DWORD adwLen);

   const wchar_t * GetVersion();

   string GetEvents();

   bool AnalysisStartParam(int argc, char **argv, VHStartParam &);

   void SaveStartParam(const VHStartParam &);

   BOOL RunAdminCheck(wstring cmdLine);

   BOOL AppExistCheck(bool bSkipAppCheck);

   BOOL CreateServerConnect(string sStreamName, bool bMengZhu);

   void NotifyMsg(char *msg);

   SocketClient *GetSocketClient();
   HANDLE *GetNameCore();

   static DWORD __stdcall CheckStopThead(void* param);

   static void NotifyService(char *msg);

   QJsonObject Get(QString method, QString param);

   bool GetISRestart();
   void startDelOverdueLog();
protected:	// VH_IUnknown
   virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void ** appvObject);

   virtual ULONG STDMETHODCALLTYPE AddRef(void);

   virtual ULONG STDMETHODCALLTYPE Release(void);

protected:
   //�� IMessageDispatcher ������
   virtual HRESULT STDMETHODCALLTYPE Connect(IMessageDispatcher * apMessageDispatcher);

   //�Ͽ��� IMessageDispatcher ������
   virtual HRESULT STDMETHODCALLTYPE Disconnect();

   //��������
   virtual HRESULT STDMETHODCALLTYPE OnRecvMessage(DWORD adwSenderID, DWORD adwMessageID, void * apData, DWORD adwLen);

   void customEvent(QEvent* event);
   void HandleAppExit();
   void HandleBuParam(QEvent* event);

private slots:
   void updateClock();
   void slot_rmLogs();
private:
   void ReadAdress(string& strAddr);
   //char* UnicodeToUtf8(const wchar_t* unicode);
   //std::wstring CharToWchar(const char* c, size_t m_encode = CP_ACP);
   //char* G2U(const char* gb2312);
   //�汾������
   bool CheckVersionAndUpdate();
   void UpdateLogIn(int mode, QString streamId = QString("0"));
   //static void GetAppPath(wchar_t* szAppPath);
   void GetAuthConfig();
   // ע����Ϣ
   //HRESULT RegisterMessage(BOOL bIsReg);

   //�ɷ�������Ϣ
   HRESULT DispatchCachedMessage();

   //������Ϣ
   HRESULT InstanceDealMessage(DWORD adwSenderID, DWORD adwMessageID, void * apData, DWORD adwLen);

   //����ڴ��
   HRESULT TimerCheckMemPool();

   void DestorySplashTip();
private:
   void SaveCommand();
   bool GetCommand(wchar_t *);
   void NetworkError();
   void RemovOverLog(QString& strPath);
private:
   long m_lRefCount;
   DWORD m_dwCurThreadID;
   QTimer* m_pQTimer = NULL;
   wchar_t m_szGGSysPath[MAX_PATH + 1];	// ϵͳ·��
   std::wstring m_wsCurrentVersion;
   HANDLE gNameCore = NULL;
   SocketClient* gSocketClient = NULL;
   QString m_domain;
   QString m_msgToken;
   VHStartParam m_param;
   QByteArray m_userData;
   bool bRestart = false;

   QString mToken;
   QString mListUrl;
   QString mPluginUrl;

   QString mUserImageUrl;
   QString mUserName;
   bool mbIsPwdLogin = false;
   bool mShowTeaching;
   bool m_bSkipAppCheck = false;

   bool m_bHideSplash = false;
   QTimer* m_pDelLogTimer;
private:
   //CR��Ϣ��
   struct STRU_MESSAGE_ITEM : public CMemPoolBase
   {
      DWORD			m_dwSenderID;
      DWORD			m_dwMessageID;
      void *			m_pData;
      DWORD			m_dwLen;
   public:
      STRU_MESSAGE_ITEM();
      ~STRU_MESSAGE_ITEM();
   public:
      BOOL SetData(const void * apData, DWORD adwLen);
   };
   // 
private:
   //������Ϣ����
   typedef std::list<STRU_MESSAGE_ITEM*>	MessageQueue;

   DWORD m_dwCycleMsgDealNum;				//���ڴ�����Ϣ����
   long m_lCurrentDealMessage;				//��ǰ������Ϣ����

   MessageQueue m_oMessageQueue;		//CR��Ϣ����
   CCriticalSection m_oCriticalSectionMQ;			//�ٽ��� CR��Ϣ����
   string m_sStreamName;
   CCheckVersionDlg* m_pCheckVerDlg = nullptr;
   QString mConfigPath;
   //char szLogIdBase[256];//��־�������
   int mIStartMode;			//������ʽ
   int mIDisplayCutRecord;	//�Ƿ���ʾ ���¼��

   QString mPrivateStartInfo;
};
//����ģʽ������ʵ��
typedef CSingleton<CRuntimeInstance> CSingletonRuntimeInstance;
#endif // !H_CRUNTIMEINSTANCE_H