#ifndef VHALLLOGINWIDGET_H
#define VHALLLOGINWIDGET_H

#include <QWidget>
#include <QDialog>
#include <QObject>
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

class VhallLoginWidget : public QWidget
{
    Q_OBJECT

public:
   explicit VhallLoginWidget(QWidget *parent = 0);
    ~VhallLoginWidget();    
    void CheckAutoLogin();
    QString GetRoomID() {return mRoomID;}
    QString GetRoomPwd() {return mRoomPWD;}

    void EnableClickedExitApp();
    bool GetShowTeaching() {return mShowTeaching;};
    bool IsPwdLogin();
    
    QString GetAuthToken();
    void SetAuthToken(QString);
    bool IsPhoneNumberEffective(QString);
	 void SetVersion(const QString& strVersion);
protected:
    bool eventFilter(QObject *, QEvent *);
    void paintEvent(QPaintEvent *);
    void closeEvent(QCloseEvent *event);
    void customEvent(QEvent* event);

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
    void Slot_ExitApp();
    void Slot_MinApp();
    void slot_CloseAndExitApp();

private:
    void Login();
    void LoginFailed();
    void LoginSuccessed(QString);
    void GetVerifyCode();
    void GetVeryfyCodeEnd();

    void HandleLogin(QEvent* event);
    void HandleGetVerifyCode(QEvent* event);

    void Tip(QString title,QString msg);
	 void loginLogReport(bool bResult = false);
    void ShowJoinActiveWnd();

signals:
    void sig_ExitApp();
    void sig_Login();
    void sig_PwdLogin();
private:
    Ui::VhallLoginWidget *ui;
    QPoint pressPoint;
    QPoint startPoint;
    QPixmap pixmap;
    QString mConfigPath;
    QString mLoginFailedReason;
    QTimer mTimer;
   
    QString mRoomID;  //登录界面加入互动房间ID
    QString mRoomPWD;

    QString mAuthToken;
    bool mShowTeaching = false;
	 QDateTime m_startLogInTime;
    IPluginWebServer* m_pPluginHttpServer = NULL;
    bool mbIsPwdLogin = false;
    bool bFirstLogin = true;
};

#endif // VHALLLOGINWIDGET_H
