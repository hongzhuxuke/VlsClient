#ifndef VHALLCHATWIDGET_H
#define VHALLCHATWIDGET_H

#include <QWidget>
#include <QWebEngineView>
#include <QWebChannel>
#include <QWebEngineSettings>
namespace Ui {
   class VhallChatWidget;
}

class VhallChatWidget : public QWidget {
   Q_OBJECT

public:
   explicit VhallChatWidget(QWidget *parent = 0);
   ~VhallChatWidget();

   void appendChatMsg(QString msg);
   void recv_forbidAll();
   void recv_cancelForbidAll();
   void InitUserInfo(QString userName, QString userImageUrl, QString userId);
   void setRoomId(QString roomId, char *filterurl, bool forbidchat);
   void Refresh();
   void SetIsHost(bool);
   void SetReloadChatHtml(bool bReload);

   bool IsLoadUrlFinished();
protected:
   bool eventFilter(QObject *obj, QEvent *e);
   void showEvent(QShowEvent *event);
private:
   void RegisterWebEngineView();
   void LoadUserInfo(QString userName, QString userImageUrl, QString userId);
   
private slots:
   void slotsTextChanged();

public slots:
   void slot_loadFinished(bool);
   int sendChatMsg(QString text);
   int post_forbidAll(bool aim);
   int forbid_someone(QString user_id, bool aim);
   int kickout_someone(QString user_id, bool aim);
   void open_filter_browser();
   QString get_init();
   QString get_initForbStatus();
   void addObjectToJs();
   int getForbidStatus(QString userID);
   int getKickoutStatus(QString userID);
   void insertArtist(QString artist);
   void doSendMsg(QString msg);
   void insertAtSomeone(QString uname);
   void hideRightMouseMenuFunc();

private:
   QWebEngineView *m_pWebEngineView = NULL;
   QWebChannel* m_pWebChannel = NULL;
   bool mIsLoadUrlFinished = true;
   bool m_bReloading = true;
   Ui::VhallChatWidget *ui;
   QString m_userName;
   QString m_userImageUrl;
   QString m_userId;
   QString m_roomId;
   //
   QString m_filterurl;
   //初始化时的聊天状态
   int m_initForbidchat;
   bool m_bHost;
};

#endif // VHALLCHATWIDGET_H
