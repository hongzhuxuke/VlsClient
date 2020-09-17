#ifndef VHALLLOGINWIDGET_H
#define VHALLLOGINWIDGET_H

#include <QWidget>
#include <QDialog>
#include <QPixmap>
#include <QPaintEvent>
#include <QPainter>
#include <QTimer>
#include <QDateTime>
#include "cbasedlg.h"

namespace Ui {
class VhallLoginWidget;
}
class IPluginWebServer;
class CInteractiveLoginWdg;
class VhallLoginWidget : public CBaseDlg
{
    Q_OBJECT

public:
    explicit VhallLoginWidget(QDialog *parent = 0);
    ~VhallLoginWidget();    
    void CheckAutoLogin();
    QString GetToken(){return mtoken;}
    QString GetListUrl(){return mlistUrl;}
    QString GetPluginUrl(){return mPluginsUrl;}
    QString GetUserName(){return mUserName;}
    QString GetUserImageUrl(){return mUserImage;}
    QString GetRoomID() {return mRoomID;}
    QString GetRoomPwd() {return mRoomPWD;}
    bool GetShowTeaching() {return mShowTeaching;};
    bool IsPwdLogin();
    
    QString GetAuthToken();
    void SetAuthToken(QString);
    bool IsPhoneNumberEffective(QString);
	 void SetVersion(const QString& strVersion);
protected:
    bool eventFilter(QObject *, QEvent *);
    void paintEvent(QPaintEvent *);


private slots:
    void timeout();
    void on_checkBox_toggled(bool checked);
    void on_lineEdit_account_returnPressed();
    void on_lineEdit_password_returnPressed();
    void on_lineEdit_phone_returnPressed();
    void on_lineEdit_verify_code_returnPressed();
    void on_lineEidt_account_textChanged(const QString & text);
    void on_lineEdit_password_textChanged(const QString& text);
    void on_lineEdit_phone_textChanged(const QString& text);
    void slot_OnClickedProxySet();
private:
    void Login();
    void LoginFailed();
    void LoginSuccessed(QString);

    void GetVerifyCode();
    void GetVeryfyCodeEnd();

    void Tip(QString title,QString msg);
	 void loginLogReport(bool bResult = false);

    void ShowJoinActiveWnd();
private:
    Ui::VhallLoginWidget *ui;
    QPoint pressPoint;
    QPoint startPoint;
    QPixmap pixmap;
    QString mConfigPath;
    QString mLoginFailedReason;
    QTimer mTimer;
    
    QString mPluginsUrl;
    QString mlistUrl;
    QString mtoken;
    QString mUserName;
    QString mUserImage;

    QString mRoomID;
    QString mRoomPWD;

    QString mAuthToken;
    bool mShowTeaching = false;
	 QDateTime m_startLogInTime;
    IPluginWebServer* m_pPluginHttpServer = NULL;
    CInteractiveLoginWdg* mInterActiveLoginWdg = NULL;
    bool mbIsPwdLogin = false;
};

#endif // VHALLLOGINWIDGET_H
