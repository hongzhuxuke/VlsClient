#pragma once

#include <QWidget>
#include <QKeyEvent>
#include "VHWebView.h"
#include <QTimer>

class MsgCallbackDelegate {
public:
   virtual void OnRecvMsg(std::string msg_name, std::string msg) = 0;
   virtual void OnWebViewLoadEnd() = 0;
   virtual void OnWebViewDestoryed() = 0;
   virtual void OnTitleChanged(const int id, std::string title_name) = 0;
};

class LibCefViewWdg : public QWidget, public SimpleHandleEvent
{
   Q_OBJECT

public:
    LibCefViewWdg(MsgCallbackDelegate* delegate,bool isLiveListView = false, QWidget *parent = nullptr);
    ~LibCefViewWdg();

    /*����WebView�ɹ�ʱ�Ļص���
    *������id ��Ӧ����webview��id, url ����webviewʱ��Ӧ���ص�url��ַ��
    *�û����ݷ��ص�id���ɲ���ָ����webview*/
    virtual void OnHandleCreateWebView(int id, const char* url);
    /*webview�����¼�֪ͨ*/
    virtual void OnHandleWebViewDestoryed(int id);

    virtual void OnRecvMsg(const int id, std::string fun_name, std::string);

    virtual void OnTitleChanged(const int id, std::string title_name);

    void InitLibCef();
    void Create(QString url);
    void ChangeViewSize(int width, int height);
    void AppCallJsFunction(QString function);
    void LoadUrl(QString url);
    void CloseWeb();

    void DestoryCefView();

protected:
   virtual void showEvent(QShowEvent *e);
   virtual void resizeEvent(QResizeEvent *event);
   virtual void keyPressEvent(QKeyEvent *event);
   virtual void keyReleaseEvent(QKeyEvent *event);
   virtual void customEvent(QEvent* event);
   virtual void enterEvent(QEvent *event);

private slots:
   void slot_OnChnageWebView();

   void slot_physicalSizeChanged(const QSizeF &);
   void slot_physicalDotsPerInchChanged(qreal);
   void slot_logicalDotsPerInchChanged(qreal);
signals:
   void sig_PressKeyEsc();

private:
   vhall_paas::VHWebViewInterface* mDocCefView = nullptr;
   QString mUrl;
   int mDocIndex = -1;
   bool mbShown = false;
   MsgCallbackDelegate* mDelegate = nullptr;
   QTimer* mShowTimer = nullptr;
   double mDpi= 1.0;
   bool mbIsListView = false;
};
