
//CTabWidget.h
#ifndef CTABWIDGET_H
#define CTABWIDGET_H
#include <QTabWidget>
#include <QtGui>
#include "CTabBar.h"
#include "CTabItemDlg.h"
#include "priConst.h"
#include "CWidget.h"
#include "PicturePreviewWdg.h"
#include "LibCefViewWdg.h"
#include "MemberListWdg.h"
#include <thread>
#include <map>
#include <mutex>
#include <list>

class CPaintLabel;
class NoticeSendWdg;
class CTabBar;
class CSuspensionDlg;


class CTabWidget :public QTabWidget, public MsgCallbackDelegate
{
   Q_OBJECT
public:
   CTabWidget(QWidget* = 0);
   virtual ~CTabWidget();

   virtual void OnRecvMsg(std::string fun_name, std::string);
   virtual void OnWebViewDestoryed();
   virtual void OnWebViewLoadEnd();
   virtual void OnTitleChanged(const int id, std::string title_name) {};
   void Destroy();

   void SetMemberListCAliveDlg(CAliveDlg* obj);

   void setTabButton(int index, QTabBar::ButtonPosition position, CPaintLabel *widget);
   void SetShowTabBar(const bool& bShow = true);
   void createTabLabel();
   void creatTab(const bool& showMember, const bool& showNotic);
   QString strTabText(int index);
   void ChangeNoticeShow(bool bCurTool /*= true*/);
   void FreshWebView();
   void InitChatWebView(const QString& strUrl);
   void ReleaseChatWebView();
   void reLoad();
   void RunJavaScrip(const QString& msg);
   void ShowMemberFromDesktop(bool is_desktop_share, QPoint pos);
   CTabBar * GetBar() { return mTabBar; }
   void AppendAnnouncement(const QString& msg, const QString& pushTime);
   void SetHandsUpState(eHandsUpState  eState);
   void creatWebView();
   void HideImagePrivew();
   QString AnalysisFlashMsgOnOffLine(const QString& param, VhallAudienceUserInfo* Info);

   static DWORD WINAPI UserLineThread(LPVOID);
   void ProcessTask();
signals:
   void sig_changed(bool bShowMem);
   void sig_JsCallOnOffLine(QString param);
   void sig_FlashJsCallUserOnLine(QString param);
   void sig_FlashJsCallUserOffLine(QString param);
   void sig_DragMember(bool);

public slots:
   void slotChatClicked();
   void slotCloseShareDlg();
   void JsCallQtMsg(QString name, QString param);
   void JsCallQtOpenUrl(QString url);
   void JSCallQtUserOnline(QString param);
   void JSCallQtUserOffline(QString param);

protected:
   void paintEvent(QPaintEvent *event);
   void customEvent(QEvent* event);
   void showEvent(QShowEvent* e);

   void RecvLineMsg(VhallAudienceUserInfo* lineParam);
   void HandleProcessMember();
private slots:
   void loadFinished(bool);
   void slot_ShowHandsUpPage();
   void slot_changed(int);
   void slot_DragChat(const QPoint& posShow);
   void slot_DragMember(const QPoint& posShow);
   void ShowDragDlg(QWidget *draged, QPoint posShow);
   void slot_tabDrag(int index, QPoint point, bool drag);
   void slot_clickedTabDrag(int index, QPoint point);

   void clostEjectTabItem(CTabItemDlg *widget);
   void slot_tabBarDoubleClicked();
   void slot_closeMember();
   void slot_removeImagePreview();
   void slot_tabClickMaxBtn();
   void slot_OnFresh();
public:
   CTabBar *mTabBar;
   CAliveDlg* mCAliveDlg = nullptr;
   NoticeSendWdg* mNoticeSendWdg = nullptr;
   MemberListWdg* mpMemberListDlg = nullptr;
   QString m_strChatUrl;

   CWidget* mpMemberReplaceWdg = nullptr;
   CSuspensionDlg *mpSuspChat = NULL;
   CPaintLabel* mpPaintLable = nullptr;
   CPaintLabel* mpPaintLableL = nullptr;
   CTabItemDlg* mpEjectTabItem = nullptr;
   CTabItemDlg* mpMemberEjectTabItem = nullptr;
   bool mbEjectNoticeSend = false;
   ImagePreviewWdg* mImagePreviewWdgPtr = nullptr;
   QString mJsCallQtMsg;

   LibCefViewWdg* mDocCefWidget = nullptr;
   bool mIsCreatedTab = false;
   static bool mbIsThreadRuning;
   static HANDLE mThreadHandle;
   std::thread* mUserLinePostThread = nullptr;
   std::list<VhallAudienceUserInfo*> mUserOnLineMsg;
   int mProcessCount = 0;
   std::mutex mUserLineMutex;
};
#endif // CTABWIDGET_H
