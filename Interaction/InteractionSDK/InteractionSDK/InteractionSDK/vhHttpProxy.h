#ifndef _VH_INTERACTION_PROXY_TEST_H_
#define _VH_INTERACTION_PROXY_TEST_H_
#include <QString>
#include <QObject>
#include <QTimer>  
#include <QMap>
#include <atomic>
#include <QMutex>
#include "CriticalSection.h"

class QNetworkReply;
class QAuthenticator;
class QNetworkProxy;
class QEventLoop;
class QNetworkAccessManager;

#define HTTP_LONG_POLLING_HEART_TIME   10000

class InternalHttpRequest :public QObject{
   Q_OBJECT
public:
    InternalHttpRequest(QObject* obj = NULL);
   ~InternalHttpRequest();

   void Release();
   bool VHProxyTest(QString ip, unsigned short port, QString userName, QString password, QString url);
   bool VHGetRet2(const QString& url, int time_out, QByteArray &ba, int &errCode, bool noTimeOut = false, QString io = QString());
   bool VHPostRet(const QString& url,/* int, QByteArray &ba,*/ QString &json, QString cookis = QString());
   void ExitLoop();
   void SetHttpProxyInfo(bool enable, QString ip = QString(), unsigned short port = 0, QString userName = QString(), QString password = QString());
   void StopSendPing();

private slots:
   void slot_authenticationRequired(QNetworkReply *, QAuthenticator *);
   void slot_proxyAuthenticationRequired(const QNetworkProxy &, QAuthenticator *);
private:
   QString m_ip;
   int m_port;
   QString m_user;
   QString m_pwd;
   bool m_bEnableProxy = false;
   QEventLoop *m_pGetEventLoop = NULL;
   QTimer* mpTimerLoopTimeOut;
   CCriticalSection		mNetWorkManagerCS;
   QNetworkAccessManager* m_pVHGetManager = NULL;
   QNetworkAccessManager* m_pVHPostManager = NULL;
   QEventLoop* m_pVHPostEventLoop = NULL;
   QTimer* mpPostTimer = NULL;
};


#endif
