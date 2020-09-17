#ifndef HTTPTASKWORKTHREAD_H
#define HTTPTASKWORKTHREAD_H

#include <QObject>
#include "CriticalSection.h"
#include <QThread>
#include <QEventLoop>
#include <QMap>
#include <QNetworkReply>
#include <QTimer>

class STRU_HTTPCENTER_HTTP_RQ;
class STRU_HTTPCENTER_HTTP_RS;
class QNetworkAccessManager;
class QNetworkProxy;
class QNetworkReply;
class QAuthenticator;


class HttpTaskWorkThread : public QThread
{
  Q_OBJECT
public:
   HttpTaskWorkThread(QObject *parent = NULL);
   ~HttpTaskWorkThread();

   void SetHttpManger(QNetworkAccessManager* manager);

   void SetTaskParam(STRU_HTTPCENTER_HTTP_RS& param);
   void QuitEventLoop();

protected:
   void run();

private slots:
   void slot_authenticationRequired(QNetworkReply *, QAuthenticator *);
   void slot_proxyAuthenticationRequired(const QNetworkProxy &, QAuthenticator *);
   void slot_networkReplyErr(QNetworkReply::NetworkError);
   void slot_eventLoopQuit(QNetworkReply*);

private:
   STRU_HTTPCENTER_HTTP_RS *mTaskParam = NULL;
   QNetworkAccessManager* mHttpManager = NULL;
   QEventLoop *mLoopEvent = NULL;
   QString mUserName;
   QString mUserPwd;
   QTimer mHttpTimeOut;
};

#endif // HTTPTASKWORKTHREAD_H
