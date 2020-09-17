#ifndef CINTERACTIVELOGINWDG_H
#define CINTERACTIVELOGINWDG_H

#include <QWidget>
#include <QDialog>
#include "ui_cinteractiveloginwdg.h"
#include "vhallwaiting.h"
#include "cbasedlg.h"
class TitleButton;

class CInteractiveLoginWdg : public CBaseDlg
{
    Q_OBJECT

public:
    CInteractiveLoginWdg(QDialog *parent = 0);
    ~CInteractiveLoginWdg();

    QString GetRoomID();
    QString GetRoomPwd();
    QString GetUserNickName();
    QString GetToken();
    QString GetHeadImageUrl();
    QString GetPluginUrl();

protected:
   virtual bool eventFilter(QObject *o, QEvent *e);
   virtual void paintEvent(QPaintEvent *);
   virtual void closeEvent(QCloseEvent *);

   bool IsEnableJoinActive();

public slots:
   void Slot_JoinInteractive();
   void Slot_Close();
   void Slot_NickNameEdited(const QString&);
   QString GetUserSig(const QString&msgToken);
private:
    Ui::CInteractiveLoginWdg ui;
    TitleButton *m_pBtnClose;
    QPoint pressPoint;
    QPoint startPoint;
    QPixmap pixmap;
    VhallWaiting *m_pWaiting = NULL;

    QString mPluginsUrl;
    QString mlistUrl;
    QString mtoken;
    QString mUserName;
    QString mUserImage;
    QString mDoMain;
    QString mAuthToken;

    QString mRoomID;
    QString mNickName;
    QString mRoomPwd;
};

#endif // CINTERACTIVELOGINWDG_H
