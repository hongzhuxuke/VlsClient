#ifndef VHALLWEBVIEW_H
#define VHALLWEBVIEW_H

#include <QWidget>
#include "ui_VhallLiveListViewDlg.h"
#include "vhallextrawidgetuserinfo.h"
#include "vhallexitwidget.h"
#include "vhallteaching.h"
//#include <QWebEngineView>
//#include <QWebChannel>
//#include <QWebEngineSettings>
#include <QAuthenticator>
#include <QNetworkProxy>
#include "cbasedlg.h"
#include "LibCefViewWdg.h"

class TitleBarWidget;
class QVBoxLayout;
class QSpacerItem;

class VhallLiveListViewDlg : public QWidget, public MsgCallbackDelegate
{
   Q_OBJECT

public:
   explicit VhallLiveListViewDlg(QWidget *parent = 0);
   ~VhallLiveListViewDlg();


   virtual void OnRecvMsg(std::string fun_name, std::string msg);
   virtual void OnWebViewDestoryed();
   virtual void OnWebViewLoadEnd();
   virtual void OnTitleChanged(const int id, std::string title_name) {};
   bool Create();
   void Destory();

   void Load(QString, QObject *);
   void CenterWindow(QWidget* parent);
   void SetEnabled(bool);
   void SetUserName(QString &);
   void SetUserImage(QString &);
   void executeJSCode(QString method);
   void ShowTeaching();
   void SetWaiting(QWidget *w) { m_pwaiting = w; }
   void SetHttpProxyInfo(const bool bSetHttpProxy = false, const QString ip = QString(), const QString port = QString(), const QString userName = QString(), const QString pwd = QString());
   QString GetNickName();
   void ToExitAppWithAnimation();
   void ReleaseWebEngineView();
signals:
   void SigJsCode(QString);
   void exitClicked(bool);
   void SigTest();
   void sig_ExitAppDirect();
protected:
   bool eventFilter(QObject *o, QEvent *e);
   virtual bool nativeEvent(const QByteArray &eventType, void *message, long *result);

public slots:
   void loadFinished(bool);
   void loadFinished(QNetworkReply *);
   void SlotJsCode(QString method);
   void ToExit();
   void SlotRefresh();
   void slot_reloadURL();
   void slot_CloseAndExitApp();
   void slot_OnShowMin();

private:
   void AddWebEngineView();

private:
   Ui::VhallLiveListViewDlg ui;
   QVBoxLayout *m_pCenterLayout;
   TitleBarWidget *m_pTitle = NULL;
   QObject *m_obj = NULL;
   VhallExtraWidgetUserInfo *m_userInfo = NULL;

   VhallExitWidget *m_exitWidget = NULL;
   bool m_bInit = false;
   bool m_bFinished = false;
   bool m_bShowTeaching = true;
   QWidget *m_pwaiting = NULL;
   QString m_liveUrl;

   QNetworkProxy proxy;
   //QWebEngineView *m_pWebEngineView = NULL;
   //QWebChannel* m_pWebChannel = NULL;
   LibCefViewWdg* mListCefView = nullptr;
   QSpacerItem *m_pVerticalSpacer = NULL;
   QString m_ip;
   QString m_port;
   QString m_userName;
   QString m_pwd;
   bool m_bEnableHttpProxy = false;
};

#endif // VHALLWEBVIEW_H

