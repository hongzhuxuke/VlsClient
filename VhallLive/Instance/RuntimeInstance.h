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

//运行实例
class CRuntimeInstance : public QObject, public IMessageEvent
{
   Q_OBJECT
public:
   CRuntimeInstance(void);
   ~CRuntimeInstance(void);

public:
   void InitCurVersion();
   // 创建
   HRESULT Create();

   // 销毁
   HRESULT Destory();
   //参数检测
   HRESULT CheckParam(int, char *argv[]);

   // 启动
   HRESULT StartSystem(bool *);
   // 设置
   HRESULT SetSocketClient(FuncNotifyService);

   // 关闭
   HRESULT ShutdownSystem();

   // 获取StreamName
   string GetStreamName();

   // 预处理
   BOOL PreTranslateMessage(MSG* pMsg);

   // 投递消息
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
   //与 IMessageDispatcher 的连接
   virtual HRESULT STDMETHODCALLTYPE Connect(IMessageDispatcher * apMessageDispatcher);

   //断开与 IMessageDispatcher 的连接
   virtual HRESULT STDMETHODCALLTYPE Disconnect();

   //接收数据
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
   //版本检查更新
   bool CheckVersionAndUpdate();
   void UpdateLogIn(int mode, QString streamId = QString("0"));
   //static void GetAppPath(wchar_t* szAppPath);
   void GetAuthConfig();
   // 注册消息
   //HRESULT RegisterMessage(BOOL bIsReg);

   //派发缓存消息
   HRESULT DispatchCachedMessage();

   //处理消息
   HRESULT InstanceDealMessage(DWORD adwSenderID, DWORD adwMessageID, void * apData, DWORD adwLen);

   //检查内存池
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
   wchar_t m_szGGSysPath[MAX_PATH + 1];	// 系统路径
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
   //CR消息项
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
   //定义消息队列
   typedef std::list<STRU_MESSAGE_ITEM*>	MessageQueue;

   DWORD m_dwCycleMsgDealNum;				//周期处理消息数量
   long m_lCurrentDealMessage;				//当前处理消息数量

   MessageQueue m_oMessageQueue;		//CR消息队列
   CCriticalSection m_oCriticalSectionMQ;			//临界区 CR消息队列
   string m_sStreamName;
   CCheckVersionDlg* m_pCheckVerDlg = nullptr;
   QString mConfigPath;
   //char szLogIdBase[256];//日志基础标号
   int mIStartMode;			//启动方式
   int mIDisplayCutRecord;	//是否显示 打点录制

   QString mPrivateStartInfo;
};
//单例模式的运行实例
typedef CSingleton<CRuntimeInstance> CSingletonRuntimeInstance;
#endif // !H_CRUNTIMEINSTANCE_H