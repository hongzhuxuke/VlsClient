//CTabWidget.cpp
#include "CTabWidget.h"
#include <QStyleOption> 
#include <QPainter> 
#include "CPaintLabel.h"
#include "MemberListWdg.h"
#include "Notice/NoticeSendWdg.h"
#include "TabWidget/CPaintLabel.h"
#include "TabWidget/CTabItemDlg.h"
#include "priConst.h"
#include "suspensionDlg.h"
#include "ICommonData.h"
#include "MainUIIns.h"
#include "VH_Macro.h"
#include "CRPluginDef.h"
#include "DebugTrace.h"
#include <QApplication>
#include <QDesktopWidget>
#include "ConfigSetting.h"
#include "AlertTipsDlg.h"
#include "pathmanager.h"

#define CHAT_WEBVIEW_NAME   "WebEngineViewWdg"
#define PROCESS_COUNT   50

bool CTabWidget::mbIsThreadRuning = false;
HANDLE CTabWidget::mThreadHandle = nullptr;

CTabWidget::CTabWidget(QWidget *parent) :QTabWidget(parent) {
   mTabBar = new CTabBar;
   setTabBar(mTabBar);//这里是关键，这样用我们自定义的CTabBar替换原来的QTabBar
   creatTab(true, true);
   mImagePreviewWdgPtr = new ImagePreviewWdg();
   if (mImagePreviewWdgPtr) {
      mImagePreviewWdgPtr->hide();
      connect(mImagePreviewWdgPtr, SIGNAL(sig_wndRemove()), this, SLOT(slot_removeImagePreview()));
   }
   mpEjectTabItem = new CTabItemDlg(this);
   if (mpEjectTabItem) {
      connect(mpEjectTabItem, &CTabItemDlg::sig_doubleClickedTitleBar, this, &CTabWidget::slot_tabBarDoubleClicked);
      connect(mpEjectTabItem, &CTabItemDlg::sig_clickMaxBtn, this, &CTabWidget::slot_tabClickMaxBtn);
      connect(mpEjectTabItem, &CTabItemDlg::sig_OnFresh, this, &CTabWidget::slot_OnFresh);
   }
   mpMemberEjectTabItem = new CTabItemDlg(this);
   if (mpMemberEjectTabItem) {
      connect(mpMemberEjectTabItem, &CTabItemDlg::sig_doubleClickedTitleBar, this, &CTabWidget::slot_closeMember);
      mpMemberEjectTabItem->SetBtnShow(false,false);
   }
   setMinimumSize(QSize(381, 580));
   setMaximumSize(QSize(381, 580));

   mbIsThreadRuning = true;
   mThreadHandle = ::CreateEvent(NULL, FALSE, FALSE, NULL);
   mUserLinePostThread = new std::thread(CTabWidget::UserLineThread, this);
}

CTabWidget::~CTabWidget() {

   mbIsThreadRuning = false;
   if (mThreadHandle) {
      ::SetEvent(mThreadHandle);
   }
   if (mUserLinePostThread) {
      mUserLinePostThread->join();
      delete mUserLinePostThread;
      mUserLinePostThread = NULL;
   }
   if (mThreadHandle) {
      ::CloseHandle(mThreadHandle);
   }
   

   if (mDocCefWidget) {
      removeTab(0);
      delete mDocCefWidget;
      mDocCefWidget = nullptr;
   }

   if (mTabBar) {
      delete mTabBar;
      mTabBar = nullptr;
   }

   if (mImagePreviewWdgPtr) {
      delete mImagePreviewWdgPtr;
      mImagePreviewWdgPtr = nullptr;
   }

   std::unique_lock<std::mutex> lock(mUserLineMutex);
   std::list<VhallAudienceUserInfo*>::iterator iterOnLine = mUserOnLineMsg.begin();
   while (iterOnLine != mUserOnLineMsg.end()) { //上线消息不在队列里面则保存
      delete *iterOnLine;
      iterOnLine = mUserOnLineMsg.erase(iterOnLine);
   }
}

void CTabWidget::setTabButton(int index, QTabBar::ButtonPosition position, CPaintLabel *widget) {
   mTabBar->setTabButton(index, position, widget);
   connect(widget, &CPaintLabel::sig_DoubleClicked, mTabBar, &CTabBar::slot_DoubleClicked);
}

void CTabWidget::Destroy() {
   if (mDocCefWidget) {
      delete mDocCefWidget;
      mDocCefWidget = nullptr;
   }
}

void CTabWidget::paintEvent(QPaintEvent *event) {
   QStyleOption opt; // 需要头文件#include <QStyleOption>  
   opt.init(this);
   QPainter p(this); // 需要头文件#include <QPainter>  
   style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void CTabWidget::customEvent(QEvent* event) {
   if (CustomEvent_JsCallQtKickOut == event->type()) {
      QJsonParseError json_error;
      QJsonDocument parse_doucment = QJsonDocument::fromJson(QByteArray(mJsCallQtMsg.toStdString().c_str()), &json_error);
      if (json_error.error == QJsonParseError::NoError) {
         if (parse_doucment.isObject()) {
            QJsonObject obj = parse_doucment.object();
            if (obj.contains("type") && obj["type"].isString()) {
               QString type = obj["type"].toString();
               if (type == "kickout") {
                  if (obj.contains("msg") && obj["msg"].isObject()) {
                     QJsonObject msgObj = obj["msg"].toObject();
                     if (msgObj.contains("confirmText")) {
                        QString confirmText = msgObj["confirmText"].toString();
                        AlertTipsDlg tip(QString::fromWCharArray(L"%1").arg(confirmText), true, nullptr);
                        int x = 0, y = 0;
                        int screen_index = ConfigSetting::ContainsInDesktopIndex(mpEjectTabItem->isHidden() ? this->parentWidget() : (QWidget*)mpEjectTabItem);
                        ConfigSetting::GetInScreenPos(screen_index, &tip, x, y);
                        tip.move(x, y);
                        TRACE6("%s screen_index %d x %d y %d\n", __FUNCTION__, screen_index, x, y);
                        bool is_kick = false;
                        if (tip.exec() == QDialog::Accepted) {
                           is_kick = true;
                        }
                        if (mDocCefWidget) {
                           QString funPage = QString("QtCallJsKickOut(%1,%2)").arg(mJsCallQtMsg).arg(is_kick ? 1 : 0);
                           mDocCefWidget->AppCallJsFunction(funPage.toStdString().c_str());
                        }
                     }
                  }
               }
            }
         }
      }
   }
   else if(CustomEvent_LibCefMsg == event->type()) {
      CefMsgEvent* cus_event = dynamic_cast<CefMsgEvent*>(event);
      if (cus_event) {
         JsCallQtMsg(cus_event->mFunName, cus_event->mCefMsgData);
      }
   }
   else if (CustomEvent_ProcessMember == event->type()) {
      HandleProcessMember();
   }
}

void CTabWidget::createTabLabel() {
  
}

void CTabWidget::slot_changed(int index) {
   QString strTilte = mTabBar->tabText(index).trimmed();
   VH::CComPtr<ICommonData> pCommonData = NULL;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData);
   ClientApiInitResp respInitData;
   pCommonData->GetInitApiResp(respInitData);
   if (respInitData.player == FLASH_LIVE_CREATE_TYPE) {
      if (strTilte == STRCHAT) {
         if (mDocCefWidget) {
            mDocCefWidget->AppCallJsFunction("CallJsChangeTab(1)");
         }
      }
   }
}

void CTabWidget::creatTab(const bool& showMember, const bool& showNotic) {
   setFocusPolicy(Qt::NoFocus);
   setMovable(true);
   createTabLabel();
   connect(mTabBar, &QTabBar::currentChanged, this, &CTabWidget::slot_changed);
   connect(mTabBar, &CTabBar::sig_tabDrag, this, &CTabWidget::slot_tabDrag);
   connect(mTabBar, &CTabBar::sig_clickedTabDrag, this, &CTabWidget::slot_clickedTabDrag);

   VH::CComPtr<ICommonData> pCommonData = NULL;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData);
   ClientApiInitResp respInitData;
   pCommonData->GetInitApiResp(respInitData);

   mpMemberListDlg = new MemberListWdg(this);
   if (mpMemberListDlg) {
      connect(mpMemberListDlg,SIGNAL(sig_ClickedHandsUpBtn()),this,SLOT(slot_ShowHandsUpPage()));
      mpMemberListDlg->InitLiveTypeInfo(respInitData.role_name.compare(USER_HOST) == 0 ? true : false, respInitData.selectLiveType == eLiveType_VhallActive ? true : false);
      mpMemberListDlg->setWindowTitle(STRMEMBER);
   }

   if (showNotic) {    //公告
      mNoticeSendWdg = new NoticeSendWdg(this);
      if (mNoticeSendWdg) {
         mNoticeSendWdg->setObjectName("mNoticeSendWdg");
         mNoticeSendWdg->setWindowTitle(STRNOTICE);
      }
   }
}

void CTabWidget::ChangeNoticeShow(bool bCurTool /*= true*/){

}

void CTabWidget::slot_DragChat(const QPoint& posShow) {
   if (nullptr != mDocCefWidget && mpEjectTabItem) {
      ShowDragDlg(mDocCefWidget, posShow + QPoint(0.5 * mDocCefWidget->width(), 0));      
   }
}

void CTabWidget::slot_DragMember(const QPoint& posShow) {

}

void CTabWidget::HideImagePrivew() {
   if (mImagePreviewWdgPtr) {
      mImagePreviewWdgPtr->hide();
   }
}

void CTabWidget::ShowDragDlg(QWidget *draged, QPoint posShow) {
   if (nullptr == draged) {
      return;
   }
   TRACE6("%s ShowDragDlg\n", __FUNCTION__);
   if (nullptr != mpMemberEjectTabItem) {
      QString windowName = draged->windowTitle();
      TRACE6("%s windowName %s\n", __FUNCTION__, windowName.toStdString().c_str());
      if (windowName == STRMEMBER && mpMemberEjectTabItem) {
         TRACE6("%s STRMEMBER\n", __FUNCTION__);
         mpMemberEjectTabItem->SetTitle(windowName);
         mpMemberEjectTabItem->AddCenterWdg(draged);
         mpMemberEjectTabItem->move(posShow);
         mpMemberEjectTabItem->show();
         draged->show();
         setCurrentIndex(0);
         mTabBar->setCurrentIndex(0);
         return;
      }
   }

   if (nullptr != mpEjectTabItem) {
      QString windowName = draged->windowTitle();
      TRACE6("%s windowName %s\n", __FUNCTION__, windowName.toStdString().c_str());
      if (windowName == STRCHAT) {
         TRACE6("%s STRCHAT\n", __FUNCTION__);
         clostEjectTabItem(mpEjectTabItem);
         mpEjectTabItem->SetTitle(windowName);
         mpEjectTabItem->AddCenterWdg(draged);
         mpEjectTabItem->move(posShow);
         mpEjectTabItem->show();
         draged->show();
         setCurrentIndex(0);
         mTabBar->setCurrentIndex(0);
         TRACE6("%s show desktop STRCHAT\n", __FUNCTION__);
      }
   }
   TRACE6("%s ShowDragDlg end\n", __FUNCTION__);
}

void CTabWidget::slot_tabDrag(int index, QPoint point, bool drag) {
   if (index < mTabBar->count()) {
      setCurrentIndex(index);
      mTabBar->setCurrentIndex(index);
   }
   qDebug() << mTabBar->count() << "slot_tabDrag  " << index << endl;
   QWidget *draged = widget(index);
   TRACE6("%s index %d\n", __FUNCTION__, index);
   if (draged) {
      QPoint posShow = QCursor::pos();
      if (drag) {
         ConfigSetting::MoveToVisiblePos(QCursor::pos(), posShow);
         TRACE6("%s posShow x %d y %d\n", __FUNCTION__, index, posShow.x(), posShow.y());
      }
      else {
         VH::CComPtr<IMainUILogic> pMainUILogic;
         DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IMainUI, IID_IMainUILogic, pMainUILogic, return, ASSERT(FALSE));
         QWidget* pMainUI = NULL;
         pMainUILogic->GetShareUIWidget((void**)&pMainUI);

         int x, y;
         int screen_index = ConfigSetting::ContainsInDesktopIndex(pMainUI);
         ConfigSetting::GetInScreenPos(screen_index, mImagePreviewWdgPtr, x, y);
         TRACE6("%s screen_index %d x %d y %d\n", __FUNCTION__, screen_index, x, y);
         posShow.setX(x);
         posShow.setY(y);
      }
      ShowDragDlg(draged, posShow);
   }
   TRACE6("%s index %d end\n", __FUNCTION__, index);
}

void CTabWidget::slot_clickedTabDrag(int index, QPoint point) {
   if (index < mTabBar->count()) {
      setCurrentIndex(index);
      mTabBar->setCurrentIndex(index);
   }
   qDebug() << mTabBar->count() << "slot_tabDrag  " << index << endl;
   QWidget *draged = widget(index);
   if (draged) {
      QString windowName = draged->windowTitle();
      if (!windowName.trimmed().isEmpty()) {
         if (windowName == STRMEMBER) {
            //emit sig_DragMember(true);
            //SetHandsUpState(eHandsUpState_No);
         }
      }
   }
}

void CTabWidget::clostEjectTabItem(CTabItemDlg *widget) {
   TRACE6("%s \n", __FUNCTION__);
   if (nullptr == widget) {
      return;
   }
   if (mDocCefWidget) {         
      TRACE6("%s insertTab %s\n", __FUNCTION__, widget->windowTitle().toStdString().c_str());
      insertTab(0, mDocCefWidget, widget->windowTitle());
      widget->RemoveCenterWdg(mDocCefWidget);
      widget->hide();
      setCurrentIndex(0);
      mTabBar->setCurrentIndex(0);
   }
}

void CTabWidget::slot_tabBarDoubleClicked() {
   CTabItemDlg *widget = qobject_cast<CTabItemDlg*>(sender());
   clostEjectTabItem(widget);
   TRACE6("%s slot_tabBarDoubleClicked\n", __FUNCTION__);
}

void CTabWidget::slot_closeMember() {
   ShowMemberFromDesktop(false,QPoint());
}

void CTabWidget::slot_removeImagePreview() {
   if (mImagePreviewWdgPtr) {
      int x = 0, y = 0;
      ConfigSetting::GetInScreenPos(this, mImagePreviewWdgPtr);
      mImagePreviewWdgPtr->show();
   }
}

void CTabWidget::slot_tabClickMaxBtn() {
   if (mpEjectTabItem) {
      mpEjectTabItem->SetMaxSizeState();
   }
}

void CTabWidget::slot_OnFresh() {
   reLoad();
}

void CTabWidget::ShowMemberFromDesktop(bool is_desktop_share,QPoint pos) {
   if (is_desktop_share) {
      mpMemberEjectTabItem->SetTitle(STRMEMBER);
      mpMemberEjectTabItem->AddCenterWdg(mpMemberListDlg);
      mpMemberEjectTabItem->move(pos);
      mpMemberEjectTabItem->show();
      setCurrentIndex(0);
      mTabBar->setCurrentIndex(0);
   }
   else {
      if (mpMemberListDlg) {
         insertTab(0, mpMemberListDlg, mpMemberListDlg->windowTitle());
         mpMemberEjectTabItem->RemoveCenterWdg(mpMemberListDlg);
         mpMemberEjectTabItem->hide();
         //mpPaintLable = nullptr;
         //mpPaintLableL = nullptr;
         createTabLabel();
         //setTabButton(1, QTabBar::RightSide, mpPaintLable);
         //setTabButton(1, QTabBar::LeftSide, mpPaintLableL);
         setCurrentIndex(0);
         mTabBar->setCurrentIndex(0);
      }
   }
}

void CTabWidget::ReleaseChatWebView() {
   Destroy();
}

void CTabWidget::reLoad() {
   if (mDocCefWidget) {
      mDocCefWidget->LoadUrl(m_strChatUrl);
   }
}

void CTabWidget::slotChatClicked() {
   TRACE6("%s \n", __FUNCTION__);
   for (int index = 0; index < count(); ++index) {
      QString tabName = mTabBar->tabText(index);
      if (tabName == STRCHAT) {
         TRACE6("%s STRCHAT\n", __FUNCTION__);
         slot_tabDrag(index, QPoint(20, 20),false);
         break;
      }
   }
}

void CTabWidget::slotCloseShareDlg() {
   //if (NULL != mpChatWebEngineView) {
   //   clostEjectTabItem(mpEjectTabItem);
   //}

   if (NULL != mDocCefWidget) {
      clostEjectTabItem(mpEjectTabItem);
   }
}

void CTabWidget::JsCallQtMsg(QString name, QString param) {
   VH::CComPtr<ICommonData> pCommonData;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
   ClientApiInitResp respInitData;
   pCommonData->GetInitApiResp(respInitData);
   mJsCallQtMsg = param;
   TRACE6("%s url:%s\n", __FUNCTION__, param.toStdString().c_str());

   if (name == "JsCallQtOpenUrl") {
      TRACE6("%s start\n", __FUNCTION__);
      QDesktopServices::openUrl(param);
   }
   else {
      QJsonParseError json_error;
      QString paramMsg = param;
      QJsonDocument parse_doucment = QJsonDocument::fromJson(QByteArray(param.toStdString().c_str()), &json_error);
      if (json_error.error == QJsonParseError::NoError) {
         if (parse_doucment.isObject()) {
            QJsonObject obj = parse_doucment.object();
            if (obj.contains("type") && obj["type"].isString()) {
               QString type = obj["type"].toString();
               if (type == "filterUrl") {
                  if (obj.contains("msg") && obj["msg"].isString()) {
                     QString msg = obj["msg"].toString();
                     QUrl filterUrl = msg + "?assistant_token=" + respInitData.assistant_token;
                     QDesktopServices::openUrl(filterUrl);
                  }
               }
               else if (type == "imgPreview") {
                  if (obj.contains("msg") && obj["msg"].isObject()) {
                     QJsonObject msgObj = obj["msg"].toObject();
                     QJsonArray list_image_data = msgObj["list"].toArray();
                     std::list<QString> pic_list;
                     for (int i = 0; i < list_image_data.count(); i++) {
                        if (list_image_data.at(i).isString()) {
                           QString image = list_image_data.at(i).toString();
                           pic_list.push_back(image);
                        }
                     }
                     int index = msgObj["index"].toInt();
                     if (mImagePreviewWdgPtr) {
                        mImagePreviewWdgPtr->InsertPicFiles(pic_list, index);
                        int x = 0, y = 0;
                        int screen_index = ConfigSetting::ContainsInDesktopIndex(mpEjectTabItem->isHidden() ? this->parentWidget() : (QWidget*)mpEjectTabItem);
                        ConfigSetting::GetInScreenPos(screen_index, mImagePreviewWdgPtr, x, y);
                        TRACE6("%s screen_index %d x %d y %d\n", __FUNCTION__, screen_index, x, y);
                        mImagePreviewWdgPtr->show();
                        mImagePreviewWdgPtr->move(x, y);
                     }
                  }
               }
               else if (type == "kickout") {
                  QApplication::postEvent(this, new QEvent(CustomEvent_JsCallQtKickOut));
               }
               else {
                  emit sig_JsCallOnOffLine(param);
               }
            }
         }
      }
   }
}

DWORD WINAPI CTabWidget::UserLineThread(LPVOID param) {
   TRACE6("CTabWidget::ThreadProcess\n");
   while (mbIsThreadRuning) {
      if (param) {
         DWORD ret = WaitForSingleObject(mThreadHandle, 1000);
         CTabWidget* sdk = (CTabWidget*)(param);
         if (sdk) {
            sdk->ProcessTask();
         }
      }
   }
   TRACE6("CTabWidget::ThreadProcess end\n");
   return 0;
}

void CTabWidget::ProcessTask() {
   std::unique_lock<std::mutex> lock(mUserLineMutex);
   if (mUserOnLineMsg.size() > 0) {
      QApplication::postEvent(this, new QEvent(CustomEvent_ProcessMember));
   }
}

void CTabWidget::HandleProcessMember() {
   TRACE6("%s 1\n", __FUNCTION__);
   int mUserMember = 0;
   std::vector<VhallAudienceUserInfo*> tmpList;
   {
      std::unique_lock<std::mutex> lock(mUserLineMutex);
      {
         int count = mUserOnLineMsg.size();
         if (count > 0) {
            int process_count = count > PROCESS_COUNT ? PROCESS_COUNT : count;
            for (int i = 0; i < process_count; i++) {
               VhallAudienceUserInfo *item = mUserOnLineMsg.front();
               mUserOnLineMsg.pop_front();
               tmpList.push_back(item);
            }
         }
      }
   }
   for (int i = 0; i < tmpList.size(); i++) {
      VhallAudienceUserInfo *Info = tmpList.at(i);
      if (Info) {
         if (Info->fun_name == L"JSCallQtUserOnline") {
            if (mCAliveDlg) {
               mCAliveDlg->HandleJoinMember(Info);
            }
         }
         else if (Info->fun_name == L"JSCallQtUserOffline") {
            if (mCAliveDlg) {
               mCAliveDlg->HandleLeaveMember(Info);
            }
         }
         delete Info;
      }
   }

   TRACE6("%s 2\n", __FUNCTION__);
}

void CTabWidget::RecvLineMsg(VhallAudienceUserInfo* lineParam) {
   std::unique_lock<std::mutex> lock(mUserLineMutex);
   if (lineParam) {
      mUserOnLineMsg.push_back(lineParam);
   }
}

QString CTabWidget::AnalysisFlashMsgOnOffLine(const QString& param, VhallAudienceUserInfo* Info){
   QString strEvent = "";
   QString trValue = QByteArray::fromBase64(param.toUtf8());
   QJsonDocument doc = QJsonDocument::fromJson(trValue.toUtf8());
   QJsonObject obj = doc.object();

   QString strRoom = CPathManager::GetObjStrValue(obj, "room");
   Info->userName = CPathManager::GetObjStrValue(obj, "user_name").toStdWString();
   Info->userId = CPathManager::GetObjStrValue(obj, "user_id").toStdWString();
   Info->headImage = CPathManager::GetObjStrValue(obj, "avatar").toStdWString();
   if (obj.contains("data") && obj["data"].isObject()) {
      QJsonObject dataObj = obj["data"].toObject();
      Info->role = CPathManager::GetObjStrValue(dataObj, "role").toStdWString();
      if (dataObj.contains("device_type")) {
         Info->mDevType = CPathManager::GetObjStrValue(dataObj, "device_type").toInt();
      }
      if (dataObj.contains("is_gag")) {
         Info->gagType = CPathManager::GetObjStrValue(dataObj, "is_gag").toInt() == 0 ? VhallShowType_Hide : VhallShowType_Allow;
      }
      if (dataObj.contains("is_kicked")) {
         Info->kickType = CPathManager::GetObjStrValue(dataObj, "is_kicked").toInt() == 0 ? VhallShowType_Hide : VhallShowType_Allow;
      }
      if (dataObj.contains("role")) {
         Info->role = CPathManager::GetObjStrValue(dataObj, "role").toStdWString();
      }
      if (dataObj.contains("concurrent_user")) {
         Info->miUserCount = CPathManager::GetObjStrValue(dataObj, "concurrent_user").toInt();
      }
   }
   strEvent = CPathManager::GetObjStrValue(obj, "event");
   return strEvent;
}

void CTabWidget::SetMemberListCAliveDlg(CAliveDlg* obj) {
   mCAliveDlg = obj;
}

void CTabWidget::OnRecvMsg(std::string fun_name, std::string cus_msg) {
   if (fun_name.length() > 0 && cus_msg.length() > 0) {
      TRACE6("%s msg:%s\n", __FUNCTION__, cus_msg.c_str());
   }
   if (fun_name == "JSCallQtUserOnline" || fun_name == "JSCallQtUserOffline") {
      //flash活动的上下线消息，需要解析base64数据
      VhallAudienceUserInfo *Info = new VhallAudienceUserInfo(QString::fromStdString(fun_name).toStdWString());
      AnalysisFlashMsgOnOffLine(QString::fromStdString(cus_msg), Info);
      TRACE6("%s fun_name:%s uid %s name %s\n", __FUNCTION__, fun_name.c_str(), QString::fromStdWString(Info->userId).toStdString().c_str(), QString::fromStdWString(Info->userName).toStdString().c_str());
      RecvLineMsg(Info);
   }
   else if (fun_name == "JsCallQtMsg") {
      VH::CComPtr<ICommonData> pCommonData;
      DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
      UserOnLineParam lineParam;
      pCommonData->ParamToLineParam(QString::fromStdString(cus_msg), lineParam);
      if (lineParam.type == "Join" || lineParam.type == "Leave") {
         VhallAudienceUserInfo *Info = new VhallAudienceUserInfo(lineParam.type == "Join" ? L"JSCallQtUserOnline" : L"JSCallQtUserOffline");
         Info->role = lineParam.userRole.toStdWString();
         Info->userId = lineParam.userId.toStdWString();
         Info->userName = lineParam.userName.toStdWString();
         Info->gagType = lineParam.is_banned == true ? VhallShowType_Allow : VhallShowType_Hide;
         Info->headImage = lineParam.headImage.replace("https", "http").toStdWString();
         Info->mDevType = lineParam.mDevType;
         Info->miUserCount = lineParam.mUV;
         Info->kickType = VhallShowType_Hide;
         RecvLineMsg(Info);
      }
      else {
         CefMsgEvent *cefMsg = new CefMsgEvent(CustomEvent_LibCefMsg, QString::fromStdString(fun_name), QString::fromStdString(cus_msg));
         QApplication::postEvent(this, cefMsg);
      }
   }
   else {
      CefMsgEvent *cefMsg = new CefMsgEvent(CustomEvent_LibCefMsg, QString::fromStdString(fun_name), QString::fromStdString(cus_msg));
      QApplication::postEvent(this, cefMsg);
   }
}

void CTabWidget::OnWebViewDestoryed() {

}

void CTabWidget::OnWebViewLoadEnd() {

}

void CTabWidget::JsCallQtOpenUrl(QString url) {
   TRACE6("%s start\n", __FUNCTION__);
   QDesktopServices::openUrl(url);
}

void CTabWidget::JSCallQtUserOnline(QString param) {
   emit sig_FlashJsCallUserOnLine(param);
}

void CTabWidget::JSCallQtUserOffline(QString param) {
   emit sig_FlashJsCallUserOffLine(param);
}

void CTabWidget::loadFinished(bool ok) {

}

void CTabWidget::FreshWebView() {
   int iTab = currentIndex();
   creatWebView();
   reLoad();
   setCurrentIndex(iTab);
   mTabBar->setCurrentIndex(iTab);
}


void CTabWidget::InitChatWebView(const QString& strUrl) {
   m_strChatUrl = strUrl;
   VH::CComPtr<ICommonData> pCommonData = NULL;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData);
   ClientApiInitResp respInitData;
   pCommonData->GetInitApiResp(respInitData);
   if (respInitData.player == FLASH_LIVE_CREATE_TYPE) {
      m_strChatUrl = m_strChatUrl + "&hide_tab=1";
   }
   FreshWebView();
}

void CTabWidget::RunJavaScrip(const QString& msg) {
   if (mDocCefWidget) {
      mDocCefWidget->AppCallJsFunction(msg);
   }
   if (msg.contains("qtCallJsSocketIoSendMsg")) {
      QString data = msg.mid(QString::fromStdString("qtCallJsSocketIoSendMsg").length() + 2, msg.length());
      if (!data.isEmpty()) {
         data = data.mid(0,data.length() - 2);
         if (!data.isEmpty()) {
            QString context;
            QString time;
            QByteArray normal_data = QByteArray::fromBase64(QByteArray(data.toStdString().c_str()), QByteArray::Base64Option::Base64UrlEncoding);
            normal_data = normal_data.replace("\\","");
            normal_data = normal_data.mid(1, normal_data.length());
            normal_data = normal_data.mid(0, normal_data.length()-1);
            QJsonDocument doc = QJsonDocument::fromJson(normal_data);
            if (!doc.isNull()) {
               QJsonObject obj = doc.object();
               if (obj.contains("content")) {
                  context = obj["content"].toString();
               }
               if (obj.contains("publish_release_time")) {
                  time = obj["publish_release_time"].toString();
               }
               if (mNoticeSendWdg) {
                  mNoticeSendWdg->AppendMsg(context, time);
               }
            }

         }
      }
   }
}

void CTabWidget::AppendAnnouncement(const QString& msg, const QString& pushTime) {
   if (mNoticeSendWdg) {
      mNoticeSendWdg->AppendMsg(msg, pushTime);
   }
}

void CTabWidget::SetHandsUpState(eHandsUpState  eState) {
   bool bShow = false;
   QString strTilte = mTabBar->tabText(mTabBar->currentIndex()).trimmed();
   for (int i = 0; i < mTabBar->count(); i++) {
      QString txt = mTabBar->tabText(i);
      if (txt == STRMEMBER) {
         //if (strTilte.compare(STRMEMBER) != 0 && nullptr != mpPaintLable && mpMemberListDlg && !mpMemberListDlg->isVisible()) {
         //   mpPaintLable->SetHandsUpState(eHandsUpState_No);
         //}
      }
   }
}

void CTabWidget::slot_ShowHandsUpPage() {
   bool bShow = false;
   QString strTilte = mTabBar->tabText(mTabBar->currentIndex()).trimmed();
   for (int i = 0; i < mTabBar->count(); i++) {
      QString txt = mTabBar->tabText(i);
      //if (txt == STRMEMBER && mpPaintLable) {
      //   mpPaintLable->SetHandsUpState(eHandsUpState_No);
      //}
   }
}

void CTabWidget::SetShowTabBar(const bool& bShow /*= true*/) {
   if (NULL == mTabBar){
      return;
   }
   //bShow ? mTabBar->show() : mTabBar->hide();
   setCurrentIndex(0);
   mTabBar->setCurrentIndex(0);
}

QString CTabWidget::strTabText(int index) {
   return QString();
}

void CTabWidget::showEvent(QShowEvent* e) {
   if (!mIsCreatedTab) {
      mIsCreatedTab = true;
      VH::CComPtr<ICommonData> pCommonData = NULL;
      DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData);
      ClientApiInitResp respInitData;
      pCommonData->GetInitApiResp(respInitData);
      bool enableMember = false;
      bool enableNotice = false;
      if (respInitData.selectLiveType == eLiveType_Live) {
         enableMember = true;
         enableNotice = true;
      }
      else {
         if (respInitData.role_name != USER_HOST) {
            if (respInitData.player == FLASH_LIVE_CREATE_TYPE) {
               std::list<QString>::iterator iter = respInitData.permission_data.begin();
               for (; iter != respInitData.permission_data.end(); iter++) {
                  if ("members_manager" == (*iter)) {
                     enableMember = true;
                  }
                  else if ("webinar_notice" == (*iter)) {
                     enableNotice = true;
                  }
               }
            }
            else {
               std::list<QString>::iterator iter = respInitData.permission_data.begin();
               for (; iter != respInitData.permission_data.end(); iter++) {
                  if (eWebOpType_MemberManager == (*iter).toInt()) {
                     enableMember = true;
                  }
                  else if (eWebOpType_PublicNotice == (*iter).toInt()) {
                     enableNotice = true;
                  }
               }
            }
         }
         else {
            enableMember = true;
            enableNotice = true;
         }
      }

      creatWebView();
      if (mDocCefWidget) {
         mDocCefWidget->setObjectName(CHAT_WEBVIEW_NAME);
         addTab(mDocCefWidget, STRCHAT);
      }

      if (enableMember && mpMemberListDlg) {
         addTab(mpMemberListDlg, STRMEMBER);
         //setTabButton(1, QTabBar::RightSide, mpPaintLable);
         //setTabButton(1, QTabBar::LeftSide, mpPaintLableL);
      }

      if (enableNotice && mNoticeSendWdg) {    //公告
         mNoticeSendWdg->setObjectName("mNoticeSendWdg");
         mNoticeSendWdg->setWindowTitle(STRNOTICE);
         addTab(mNoticeSendWdg, STRNOTICE);
      }
   }
}

void CTabWidget::creatWebView(){
   if (mDocCefWidget == nullptr) {
      mDocCefWidget = new LibCefViewWdg(this);
      mDocCefWidget->InitLibCef();
      mDocCefWidget->setWindowTitle(STRCHAT);
      mDocCefWidget->setObjectName(STRCHAT);
   }
}
