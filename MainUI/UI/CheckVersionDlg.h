#ifndef CHECKVERSIONDLG_H
#define CHECKVERSIONDLG_H

#include <QDialog>
#include <QFile>
#include <QVBoxLayout>
#include <QIODevice>
#include <QFileInfo>
#include <QProgressBar>
#include <QLabel>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QAuthenticator>
#include <QtNetwork/QNetworkProxy>
#include <QMutex>
#include "vhprocessbar.h"
#include "cbasedlg.h"

using namespace std;
#define CHECKVER_JSON_ERROR -1
#define CHECKVER_DATA_ERROR -2
#define CHECKVER_JSON_SUCCESS 2
#define CHECKVER_ALL_SUCCESS 1

#ifndef	IA64
#define	IA64 __int64
#endif//IA64

class TitleWidget;
class CCheckVersionDlg : public CBaseDlg {
   Q_OBJECT

public:
   explicit CCheckVersionDlg(QDialog *parent = 0, bool bUpdata = true);
   ~CCheckVersionDlg();

   void Release();

   void SetSplashHide(bool bHide) {
      m_bSplashHide = bHide;
   }
   QString GetQVersion();
   wstring GetVersion();
   int CheckVersion();
   QString GetDownloadFilePath();
   static IA64	GetCurVersion(const wchar_t* exeFile, std::wstring &);
   bool GetSuspensionLoad()
   {
	   return mbSuspensionLoad;
   }
public Q_SLOTS:
	virtual int exec();

public slots:
	void StartDownload();
	void GetNewVersion();
	void slotSuspensionLoad();

protected:
   void paintEvent(QPaintEvent *);
   bool eventFilter(QObject *, QEvent *);
   void customEvent(QEvent*);
private slots:
   void DownloadReplyFinished(QNetworkReply *reply);
   void onDownloadProgress(qint64 bytesSent, qint64 bytesTotal);
   void onReadyRead();
   int VersionReplayFinished(QNetworkReply*reply);
   void slot_authenticationRequired(QNetworkReply *, QAuthenticator *);
   void slot_proxyAuthenticationRequired(const QNetworkProxy &, QAuthenticator *);
   void VersionCheckTimeout();
   void slot_startCheck();

private:
   int parseJson(QByteArray *bytes);
   IA64 GetLongVersion(string sVersion);
   void SetProxyUserInfo(QAuthenticator *auth);
   void SetNetworkProxy(QNetworkAccessManager* manager);

public:
   string  mNewVersion;
   QString mDownloadLink;
   QString mDownloadTips;
   bool mIsForceUpdata = false; //是否强制更新
   IA64 curVersion;
   wstring mCurrVersionW;

   bool m_bMengZhu;
   QPixmap pixmap;

private:
   QFile *mFile = NULL;
   QString mDownloadFilePath;
   QFileInfo fileInfo;
   QNetworkAccessManager *mChkVersionAccessManager;
   QNetworkAccessManager *mDownloadAccessManager;
   QNetworkRequest mRequest;
   QNetworkReply *mChkVersionReply;
   QNetworkReply *mDownloadReply = NULL;

   QTimer      *mTimeoutTimer;
   QTimer *mStartCheckTimer;
   QMutex      mCheckMutex;

   bool        mIsChecked;

   bool httpRequestAbort;
   VHProcessBar *mProcessBar = NULL;
   bool m_bSplashHide = false;

   QPoint pressPoint;
   QPoint startPoint;
   bool mbSuspensionLoad = false;
};

#endif // CHECKVERSIONDLG_H
