#include "MemberListWdg.h"
#include "cbasedlg.h"
#include <QtAlgorithms>
#include <QPainter> 
#include <QTimer>
#include <QApplication>
#include <QDesktopWidget> 
#include "VhallIALive.h"
#include "priConst.h"
#include "DebugTrace.h"   
#include <QScrollBar> 
#include "Msg_CommonToolKit.h"
#include "ICommonData.h"
#include "pathManager.h"
#include "pathManage.h"
#include "VSSAPIManager.h"
#include "MemberParam.h"
#include "ConfigSetting.h"
#include "VH_Macro.h"
#include "MainUIIns.h"
#include "CRPluginDef.h"
#include "MemberListWidgetDelegate.h"

#define MAX_DOWNLOAD_PAGE 4
#define MAX_SHOW_USER_COUNT   200
#define MAX_CHECK_MEMBERLIST  30000
#define IMAGE_SAVE_FLODER  QString("image")
#define CanNotSend  QStringLiteral("QPushButton{color:rgb(255, 255, 255);\
border:0px;border-top-right-radius:4px;border-bottom-right-radius:4px;background-color:rgb(166, 166, 168);font: 12px \"微软雅黑\";}")

#define CanSend  QStringLiteral("QPushButton{color:rgb(255, 255, 255);\
border:0px;border-top-right-radius:4px;border-bottom-right-radius:4px;background-color:rgb(252, 86, 89);font: 12px \"微软雅黑\";}")

QString StrEnableType = QString::fromStdString("border-image: url(:/button/images/handsUp_Open.png);");
QString StrDisableType = QString::fromStdString("border-image: url(:/button/images/handsUp_close.png);");

QString OnLineMemberPageBtnSelect = QString::fromWCharArray(L"QPushButton{font:12px \"微软雅黑\";\
                                                                  color:rgb(255, 255, 255);\
                                                                  border:0px;color:rgb(202, 202, 202);\
                                                                  background-color:rgb(253, 86, 90);\
                                                                  border-top-left-radius:3px;\
                                                                  border-bottom-left-radius:3px;}");

QString OnLineMemberPageBtnNoSelect = QString::fromWCharArray(L"QPushButton{font:12px \"微软雅黑\";\
                                                                  color:rgb(255, 255, 255);\
                                                                  border:0px;\
                                                                  color:rgb(202, 202, 202);\
                                                                  background-color:rgb(102, 102, 102);\
                                                                  border-top-left-radius:3px;\
                                                                  border-bottom-left-radius:3px;}\
                                                               QPushButton:hover{font:12px \"微软雅黑\";\
                                                                  color:rgb(255, 255, 255); \
                                                                  border:0px;\
                                                                  color:rgb(202, 202, 202);\
                                                                  background-color:rgb(150, 150, 150);\
                                                                  border-top-left-radius:3px; \
                                                                  border-bottom-left-radius:3px;}");

QString HandsUpMemberPageBtnSelect = QString::fromWCharArray(L"QPushButton{font:12px \"微软雅黑\";\
                                                                  color: rgb(255, 255, 255);\
                                                                  border:0px;\
                                                                  background-color: rgb(253, 86, 90);}");

QString HandsUpMemberPageBtnNoSelect = QString::fromWCharArray(L"QPushButton{font:12px \"微软雅黑\";\
                                                                  color:rgb(255, 255, 255);\
                                                                  border:0px;\
                                                                  color:rgb(202, 202, 202);\
                                                                  background-color:rgb(102, 102, 102);}\
                                                               QPushButton:hover{font:12px \"微软雅黑\";\
                                                                  color:rgb(255, 255, 255); \
                                                                  border:0px;\
                                                                  color:rgb(202, 202, 202);\
                                                                  background-color:rgb(150, 150, 150);}");

QString LimitMemberPageBtnSelect = QString::fromWCharArray(L"QPushButton{font:12px \"微软雅黑\";\
                                                                  color: rgb(255, 255, 255);\
                                                                  border:0px;\
                                                                  background-color: rgb(253, 86, 90);\
                                                                  border-top-right-radius:3px;\
                                                                  border-bottom-right-radius:3px;}");

QString LimitMemberPageBtnNoSelect = QString::fromWCharArray(L"QPushButton{font:12px \"微软雅黑\";\
                                                                  color: rgb(255, 255, 255);\
                                                                  border:0px;color: rgb(202, 202, 202);\
                                                                  background-color: rgb(102, 102, 102);\
                                                                  border-top-right-radius:3px;\
                                                                  border-bottom-right-radius:3px;}\
                                                               QPushButton:hover{font:12px \"微软雅黑\";\
                                                                  color:rgb(255, 255, 255); \
                                                                  border:0px;\
                                                                  color:rgb(202, 202, 202);\
                                                                  background-color:rgb(150, 150, 150);\
                                                                  border-top-right-radius:3px; \
                                                                  border-bottom-right-radius:3px;}");


QString LimitListHasMemberSheet = QString::fromStdString("QListWidget#LimitMemberPage{ background-color:rgb(50, 50, 50); border:0px; }");
QString LimitListNoMemberSheet = QString::fromStdString("QListWidget#LimitMemberPage{border-image:url(:/widget/images/prohibitChatWdg.png);}");


MemberListWdg::MemberListWdg(QWidget *parent)
   : QWidget(nullptr)
{
   ui.setupUi(this);
   QString sheet = StrDisableType;
   ui.tbtnAllowRaiseHands->setStyleSheet(sheet);

   //在线列表页去除鼠标点击item出现虚线
   ui.onLineListWdt->setSortingEnabled(false);  //设置为可排序状态
   ui.onLineListWdt->setEditTriggers(QAbstractItemView::NoEditTriggers);
   ui.onLineListWdt->setFocusPolicy(Qt::NoFocus);
   ui.onLineListWdt->installEventFilter(this);
   //使用自定义绘图
   ui.onLineListWdt->setItemDelegate(new MemberListWidgetDelegate(ui.onLineListWdt));
   //举手列表页
   ui.raiseHandsListWdt->setEditTriggers(QAbstractItemView::NoEditTriggers);
   ui.raiseHandsListWdt->setFocusPolicy(Qt::NoFocus);
   ui.raiseHandsListWdt->setItemDelegate(new MemberListWidgetDelegate(ui.raiseHandsListWdt));
   //受限列表
   ui.LimitMemberPage->setEditTriggers(QAbstractItemView::NoEditTriggers);
   ui.LimitMemberPage->setFocusPolicy(Qt::NoFocus);
   ui.LimitMemberPage->setItemDelegate(new MemberListWidgetDelegate(ui.LimitMemberPage));
   //搜索列表
   ui.serarchListWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
   ui.serarchListWidget->setFocusPolicy(Qt::NoFocus);
   ui.serarchListWidget->setItemDelegate(new MemberListWidgetDelegate(ui.serarchListWidget));
   mNormalRect = this->geometry();

   connect(ui.onLineListWdt->verticalScrollBar(), &QScrollBar::valueChanged, this, &MemberListWdg::slot_OnLineMemberSliderChanged);
   connect(ui.pushButton_search_ok, SIGNAL(clicked()), this, SLOT(slot_SearchSignalMember()));

   mCheckHandsUpTimer = new QTimer(this);
   connect(mCheckHandsUpTimer, SIGNAL(timeout()), this, SLOT(slot_HostCheckHandsUpTimeout()));
   mCheckHandsUpTimer->start(10000);

   mCheckMemberListTimer = new QTimer(this);
   if (mCheckHandsUpTimer) {
      connect(mCheckMemberListTimer, SIGNAL(timeout()), this, SLOT(slot_OnCheckMemberListTimeout()));
      mCheckMemberListTimer->setSingleShot(true);
   }

   ui.tbtnAllowRaiseHands->hide();
   ui.labAllowRaiseHands->hide();
   ui.btnRaiseHands->hide();
   ui.widget_search->hide();
   ui.raiseHandsNoMember->hide();
   ui.limitNoMember->hide();
   ui.raiseHandsNoMember->hide();
   ui.searchNoMember->hide();

   ui.limitNoMember->SetBackGroud("border-image: url(:/widget/images/no_online_member.png);", QString::fromWCharArray(L"没有禁言或踢出的成员"));
   ui.raiseHandsNoMember->SetBackGroud("border-image:url(:/widget/images/no_hands_up_member.png);", QString::fromWCharArray(L"暂时无人举手"));
   ui.searchNoMember->SetBackGroud("border-image: url(:/widget/images/no_search_member.png);", QString::fromWCharArray(L"很抱歉，没有搜索到您要找的人"));

   mFindMemberNotice = new QTimer(this);
   connect(mFindMemberNotice,SIGNAL(timeout()),this,SLOT(slot_HideFindMemberNotice()));
   slot_HideFindMemberNotice();

   connect(ui.ledtLookUp,SIGNAL(textChanged(const QString &)),this,SLOT(slot_OnTextChanged(const QString &)));
   mMouseMoveTimer = new QTimer(this);
   connect(mMouseMoveTimer, SIGNAL(timeout()), this, SLOT(slot_mouseMove()));

}

MemberListWdg::~MemberListWdg() {
   if (mCheckMemberListTimer) {
      mCheckMemberListTimer->stop();
   }
   if (NULL != mpRefreshTime) {
      slotEnableRefresh();
      delete mpRefreshTime;
      mpRefreshTime = NULL;
   }
   clearData();
}

void MemberListWdg::InitLiveTypeInfo(bool is_host, bool is_rtc_live) {
   mbIsHost = is_host;
   mbIsRtcLive = is_rtc_live;
   if (mbIsRtcLive) {
      if (mbIsHost) {
         ui.labAllowRaiseHands->show();
         ui.tbtnAllowRaiseHands->show();
      }
   }
}

void MemberListWdg::RecvUserListMsg(Event& e) {
   switch (e.m_eMsgType) {
      case e_RQ_UserProhibitSpeakAllUser: {//全体禁言
         ProhibitChat(true);
         break;
      }
      case e_RQ_UserAllowSpeakAllUser: {  //取消全体禁言
         ProhibitChat(false);
         break;
      }
      case e_RQ_UserProhibitSpeakOneUser: { //单个用户禁言
         MemberListWdg::addForbidChatMember(e.m_oUserInfo, true, false);
         break;
      }
      case e_RQ_UserAllowSpeakOneUser: {//单个用户取消禁言
         MemberListWdg::addForbidChatMember(e.m_oUserInfo, false, false);
         break;
      }
      case e_RQ_UserKickOutOneUser: { //踢出单个用户
         MemberListWdg::addKickOutMember(e.m_oUserInfo, true, false);
         break;
      }
      case e_RQ_UserAllowJoinOneUser: { //允许单个用户加入	（取消踢出）
         MemberListWdg::addKickOutMember(e.m_oUserInfo, false, false);
         break;
      }
      case e_RQ_SearchOnLineMember: { //搜索到的成员
         HandleSearchMember(e);
         break;
      }
      case e_RQ_UserOnlineList: {//在线列表
         HandleGetUserOnlineList(e);
         break;
      }
      case e_RQ_UserProhibitSpeakList: { //禁言列表
         AddUserList(e.m_oUserList, true, true, this, &MemberListWdg::addForbidChatMember);
         break;
      }
      case e_RQ_UserKickOutList: {//踢出列表	
         AddUserList(e.m_oUserList, true, true, this, &MemberListWdg::addKickOutMember);
         break;
      }
      case e_RQ_UserSpecialList: { //特殊用户列表 主持人、嘉宾、助理
         HandleGetUserSpecialList(e);
         break;
      }
      case e_RQ_switchHandsup_open://举手开关-开
      case e_RQ_switchHandsup_close: { //举手开关-关
         DealSwitchHandsUp(e);
         break;
      }
      case e_RQ_handsUp: {//举手
         break;
      }
      case e_RQ_CancelHandsUp: {//取消举手
         addRaiseHandsMember(e.m_oUserInfo, false, false);
         break;
      }
      case e_RQ_notSpeak: {//下麦
         dealNotSpeak(QString::fromStdWString(e.m_oUserInfo.userId));
         break;
      }
      case e_RQ_addSpeaker: {//上麦
         addSpeaker(e);
         break;
      }
      case e_RQ_replyInvite_Fail:
      case e_RQ_replyInvite_Error: {
         updateReplyInvite(e.m_oUserInfo, false);
         break;
      }
      case e_RQ_replyInvite_Suc: {
         updateReplyInvite(e.m_oUserInfo, true);
         break;
      }
      case e_RQ_setMainShow: //设置主讲人。主画面
      case e_RQ_setMainSpeaker: {
         HandleSetMain(e);
         break;
      }
      case e_RQ_setPublishStart: {
         addSpeaker(e);
         break;
      }
      case e_RQ_setOver: {
         break;
      }
      default:break;
   }
   calcListNum();
}

void MemberListWdg::updateReplyInvite(const VhallAudienceUserInfo &info, const bool& bUpperSuc)
{
   QListWidgetItem *itemTemp = getOnlineItemWdt(QString::fromStdWString(info.userId));
   if (NULL != itemTemp) {
      //itemTemp->SetUpperStates(bUpperSuc);
   }
}

void MemberListWdg::HandleGetUserSpecialList(Event& e) {
   if (miOnlineUserCont <= 0) {
      setUserCount(e.m_oUserList.size());
   }
   VH::CComPtr<ICommonData> pCommonData;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
   ClientApiInitResp mRespInitData;
   StartLiveUrl mStartLiveData;
   pCommonData->GetInitApiResp(mRespInitData);
   for (int i = 0; i < e.m_oUserList.size(); i++) {
      QString id = QString::fromStdWString(e.m_oUserList.at(i).userId);
      if (mpAlive) {
         bool isExit = mpAlive->IsExistRenderWnd(id);
         e.m_oUserList.at(i).mIsRtcPublisher = isExit == true ? 1 : 0;
         if (mRespInitData.role_name == USER_HOST && (id == mRespInitData.join_uid || id == mRespInitData.user_id)) {
            if (mpAlive->GetIsLiving()) {
               e.m_oUserList.at(i).mIsRtcPublisher = isExit == true ? 1 : 0;
            }
            else {
               e.m_oUserList.at(i).mIsRtcPublisher = 1;
            }
         }
      }
   }

   AddUserList(e.m_oUserList, true, false, this, &MemberListWdg::addOnlineMember);
   slotEnableRefresh();
}

void MemberListWdg::RemoveFirstUserMembmer() {
   map<QString, QListWidgetItem*>::iterator iter = mOnlineItemMap.begin();
   while (iter != mOnlineItemMap.end()) {
      VhallAudienceUserInfo user_info = iter->second->data(Qt::DisplayRole).value<VhallAudienceUserInfo>();
      QString speak_status = iter->second->data(SHOW_SPEAK_STATE).isNull() ? QString("") : iter->second->data(SHOW_SPEAK_STATE).toString();
      if (iter->second != nullptr && QString::fromStdWString(user_info.role) == USER_USER && speak_status != DOWNMIC) {
         TRACE6("%s remove Id %s\n", __FUNCTION__, iter->first.toStdString().c_str());
         RemoveInfo(ui.onLineListWdt, iter->first);
         delete iter->second;
         mOnlineItemMap.erase(iter); 
         mUserMemberCount--;
         break;
      }
      iter++;
   }
}

void MemberListWdg::HandleGetUserOnlineList(Event& e) {
   setUserCount();
   if (e.m_sumNumber > 0) {
      setUserCount(e.m_sumNumber);
   }
   //设置 当前页面/总页数
   if (e.m_iPageCount > 0 && e.m_currentPage > 0 && e.m_currentPage <= e.m_iPageCount) {
      mPageCont = e.m_iPageCount;//总页数
      mCurPage = e.m_currentPage; //当前页数
      if (e.m_oUserList.size() > 0) {
         int cur_count = ui.onLineListWdt->count();
         for (size_t i = 0; i < e.m_oUserList.size(); i++) {
            if (cur_count > MAX_SHOW_USER_COUNT) {
               RemoveFirstUserMembmer();
            }
            VhallAudienceUserInfo uInfo = e.m_oUserList[i];
            uInfo.mIsRtcPublisher = mpAlive->IsExistRenderWnd(QString::fromStdWString(uInfo.userId));
            if (QString::fromStdWString(uInfo.role) != USER_USER) {
               addOnlineMember(uInfo, true, false);
            }
            else {
               addOnlineMember(uInfo, false, false);
            }
         }
      }
      slotEnableRefresh();
   }
   else {
      if (1 != mReqPage) {
         on_btnRefresh_clicked();
      }
      else {
         mPageCont = 1;//总页数
         mCurPage = 1; //当前页数
      }
   }
}

void MemberListWdg::HandleSearchMember(Event& e) {
   if (e.m_oUserList.size() == 0) {
      ui.stackedWidget->setCurrentIndex(eTabMember_SearchPage);
      ui.searchNoMember->show();
      ui.serarchListWidget->hide();
   }
   else {
      ui.stackedWidget->setCurrentIndex(eTabMember_SearchPage);
      ui.searchNoMember->hide();
      ui.serarchListWidget->show();

      for (size_t i = 0; i < e.m_oUserList.size(); i++) {
         VhallAudienceUserInfo info = e.m_oUserList[i];
         if (IsExistUserInList(ui.serarchListWidget, QString::fromStdWString(info.userId))) {
            continue;
         }
         QListWidgetItem *itemWidget = new QListWidgetItem(ui.serarchListWidget);
         if (NULL != itemWidget) {
            CheckShowMenu(QString::fromStdWString(info.role), itemWidget);
            itemWidget->setSelected(false);
            QVariant var = QVariant::fromValue(info);
            itemWidget->setData(Qt::DisplayRole, var);
            CheckShowSpeakState(QString::fromStdWString(info.userId), QString::fromStdWString(info.role), info.mIsRtcPublisher, itemWidget);
            map<QString, QListWidgetItem*>::iterator iter = mForbidItemMap.find(QString::fromStdWString(info.userId));
            if (iter != mForbidItemMap.end() && iter->second != nullptr) {
               //pWdg->SetProhibitChatStates(iter->second->ProhibitChatStates());
               if (iter->second->data(SHOW_CHAT_FORBID).isNull()) {
                  int state = iter->second->data(SHOW_CHAT_FORBID).toInt();
                  itemWidget->setData(SHOW_CHAT_FORBID, 1);
               }
            }
            else {
               itemWidget->setData(SHOW_CHAT_FORBID, 0);
            }
            LoadMemberHeadImage(info, itemWidget);
            ui.serarchListWidget->addItem(itemWidget);
            if (!info.mIsRtcPublisher) {
               //如果没有上麦，但是在举手列表里面，显示举手。
               map<QString, QListWidgetItem*>::iterator iter = mRaiseHandsItemMap.find(QString::fromStdWString(info.userId));
               if (iter != mRaiseHandsItemMap.end()) {
                  itemWidget->setData(SHOW_MAIN_VIEW, STATE_HANDS_UP);
                  itemWidget->setData(SHOW_HANDSUP_TIME, QDateTime::currentDateTime().toMSecsSinceEpoch());
               }
            }
         }
      }
   }
}

void  MemberListWdg::CheckShowMenu(QString insertUser, QListWidgetItem* insertItem) {
   VH::CComPtr<ICommonData> pCommonData;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
   ClientApiInitResp mRespInitData;
   pCommonData->GetInitApiResp(mRespInitData);
   if (insertUser == USER_USER) {
      insertItem->setData(SHOW_MORE_MENU, 1);
   }
   else if (mRespInitData.role_name == USER_HOST && insertUser != USER_HOST) {
      insertItem->setData(SHOW_MORE_MENU, 1);
   }
}

bool MemberListWdg::IsUserHandsUp(QString id) {
   map<QString, QListWidgetItem*>::iterator iterHandsUp = mRaiseHandsItemMap.find(id);
   if (iterHandsUp != mRaiseHandsItemMap.end()) {
      return true;
   }
   map<QString, QListWidgetItem*>::iterator iter = mOnlineItemMap.find(id);
   if (iter != mOnlineItemMap.end() && iter->second != nullptr) {
      int showHandsUp = iter->second->data(SHOW_MAIN_VIEW).isNull() ? 0 : iter->second->data(SHOW_MAIN_VIEW).toInt();
      return showHandsUp == STATE_HANDS_UP;
   }
   return false;
}

void MemberListWdg::LoadMemberHeadImage(const VhallAudienceUserInfo &info, QListWidgetItem* item) {
   if (info.headImage.length() < 10) {
      return;
   }

   QString fileUrl = QString::fromStdWString(info.headImage);
   QString configPath = CPathManager::GetConfigPath();
   QString proxyHost;
   QString proxyUsr;
   QString proxyPwd;
   int proxyPort;
   int is_http_proxy = ConfigSetting::ReadInt(configPath, GROUP_DEFAULT, PROXY_OPEN, 0);
   if (is_http_proxy) {
      TRACE6("%s is_http_proxy:%d\n", __FUNCTION__, is_http_proxy);
      proxyHost = ConfigSetting::ReadString(configPath, GROUP_DEFAULT, PROXY_HOST, "");
      proxyPort = ConfigSetting::ReadInt(configPath, GROUP_DEFAULT, PROXY_PORT, 80);
      proxyUsr = ConfigSetting::ReadString(configPath, GROUP_DEFAULT, PROXY_USERNAME, "");
      proxyPwd = ConfigSetting::ReadString(configPath, GROUP_DEFAULT, PROXY_PASSWORD, "");
   }
   VH::CComPtr<ICommonData> pCommonData;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
   QObject* obj = pCommonData->MainUiLoginObj();
   if (!fileUrl.contains("http")) {
      fileUrl = "http:" + fileUrl;
   }
   if (fileUrl.contains("?")) {
      int index = fileUrl.lastIndexOf("?");
      fileUrl = fileUrl.left(index);
   }

   int lastIndex = fileUrl.lastIndexOf("/");
   QString fileName = fileUrl.mid(lastIndex, fileUrl.length());
   fileName = fileName.replace("/", "\\");
   QString filePath = QString::fromStdWString(GetAppDataPath()) + IMAGE_SAVE_FLODER + fileName;
   TRACE6("%s fileUrl:%s\n", __FUNCTION__, fileUrl.toStdString().c_str());
   QFileInfo fileInfo(filePath);
   if (fileInfo.exists()) {
      item->setData(SHOW_HEAD_IMAGE, filePath);
   }
   else {
      HTTP_GET_REQUEST request(fileUrl.toStdString());
      request.SetEnableDownLoadFile(true, filePath.toStdString());
      request.mbIsNeedSyncWork = true;
      QString id = QString::fromStdWString(info.userId);
      GetHttpManagerInstance()->HttpGetRequest(request, [&, obj, fileUrl, filePath, id](const std::string& msg, int code, const std::string userData) {
         TRACE6("%s code:%d msg:%s\n", __FUNCTION__, code, msg.c_str());
         if (code == 0) {
            QApplication::postEvent(obj, new CustomDownLoadEvent(CustomEvent_DownLoadFile, code, fileUrl, filePath, id, 0));
         }
      });
   }
}

void MemberListWdg::SortMemberItem(QListWidget* memberListWidget , const VhallAudienceUserInfo &info, QListWidgetItem* itemWidget) {
   int inserIndex = memberListWidget->count();
   if (QString::fromStdWString(info.role) == USER_HOST) {
      memberListWidget->insertItem(0, itemWidget);
      TRACE6("%s insert index %d\n", __FUNCTION__, 0);
   }
   else if (QString::fromStdWString(info.role) == USER_ASSISTANT || QString::fromStdWString(info.role) == USER_GUEST) {
      int index = 0;
      for (int i = 0; i < memberListWidget->count(); i++) {
         QListWidgetItem *line_item = memberListWidget->item(i);
         VhallAudienceUserInfo cur_user = line_item->data(Qt::DisplayRole).value<VhallAudienceUserInfo>();
         if (QString::fromStdWString(cur_user.role) == USER_HOST || QString::fromStdWString(cur_user.role) == USER_GUEST) {
            index++;
         }
         else {
            break;
         }
      }
      memberListWidget->insertItem(index, itemWidget);
      TRACE6("%s insert index %d\n", __FUNCTION__, index);
   }
   else if (QString::fromStdWString(info.role) == USER_USER) {
      int index = 0;
      if (info.mIsRtcPublisher) {
         for (int i = 0; i < memberListWidget->count(); i++) {
            QListWidgetItem *line_item = memberListWidget->item(i);
            VhallAudienceUserInfo cur_user = line_item->data(Qt::DisplayRole).value<VhallAudienceUserInfo>();
            if (QString::fromStdWString(cur_user.role) == USER_HOST || QString::fromStdWString(cur_user.role) == USER_GUEST || QString::fromStdWString(cur_user.role) == USER_ASSISTANT) {
               index++;
            }
            else {
               break;
            }
         }
      }
      else {
         index = memberListWidget->count();
      }
      memberListWidget->insertItem(index, itemWidget);
      TRACE6("%s insert index %d\n", __FUNCTION__, index);
   }
}

long long MemberListWdg::GetShowUserMember() {
   return mUserMemberCount;
}

bool MemberListWdg::addOnlineMember(const VhallAudienceUserInfo &info, const bool& bextend /*= true*/, const bool& bForAll/*= true*/) {
   bool bAdd = false;
   QListWidgetItem* temp = getOnlineItemWdt(QString::fromStdWString(info.userId));
   TRACE6("%s info.userId %s\n", __FUNCTION__, QString::fromStdWString(info.userId).toStdString().c_str());
   if (NULL == temp) { //没有当前用户
      QListWidgetItem *itemWidget = new QListWidgetItem();
      if (NULL != itemWidget) {
         itemWidget->setSelected(false);
         QVariant var = QVariant::fromValue(info);
         itemWidget->setData(Qt::DisplayRole, var);
         mOnlineItemMap.insert(std::make_pair(QString::fromStdWString(info.userId), itemWidget));
         if (QString::fromStdWString(info.role) == USER_USER) {
            mUserMemberCount++;
         }
         SortMemberItem(ui.onLineListWdt, info, itemWidget);
         LoadMemberHeadImage(info, itemWidget);
         itemWidget->setData(SHOW_CHAT_FORBID, info.gagType == VhallShowType_Allow ? 1 : 0);
         itemWidget->setData(SHOW_KICK_OUT, info.kickType == VhallShowType_Allow ? 1 : 0);

         map<QString, QListWidgetItem*>::iterator iter = mForbidItemMap.find(QString::fromStdWString(info.userId));
         if (iter != mForbidItemMap.end() && iter->second != nullptr) {
            int forbid = iter->second->data(SHOW_CHAT_FORBID).toInt();
            int kickOut = iter->second->data(SHOW_KICK_OUT).toInt();
            itemWidget->setData(SHOW_CHAT_FORBID, forbid);
            itemWidget->setData(SHOW_KICK_OUT, kickOut);
         }
         
         VH::CComPtr<ICommonData> pCommonData;
         DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return false, ASSERT(FALSE));
         ClientApiInitResp mRespInitData;
         pCommonData->GetInitApiResp(mRespInitData);
         if (mRespInitData.selectLiveType == eLiveType_Live && QString::fromStdWString(info.role) == USER_HOST) {
            itemWidget->setData(SHOW_MAIN_VIEW, STATE_STAR);
         }
         else if(mRespInitData.selectLiveType == eLiveType_VhallActive){
            CheckShowSpeakState(QString::fromStdWString(info.userId), QString::fromStdWString(info.role), info.mIsRtcPublisher, itemWidget);
         }
         CheckShowMenu(QString::fromStdWString(info.role) ,itemWidget);
         if (!info.mIsRtcPublisher) {
            //如果没有上麦，但是在举手列表里面，显示举手。
            map<QString, QListWidgetItem*>::iterator iter = mRaiseHandsItemMap.find(QString::fromStdWString(info.userId));
            if (iter != mRaiseHandsItemMap.end()) {
               itemWidget->setData(SHOW_MAIN_VIEW, STATE_HANDS_UP);
               itemWidget->setData(SHOW_HANDSUP_TIME, QDateTime::currentDateTime().toMSecsSinceEpoch());
            }
         }
         bAdd = true;
      }
   }
   else//存在的话仅仅更新
   {
      //temp->SetUserInfo(info);
   }
   TRACE6("%s onLineListWdt item count %d\n", __FUNCTION__, ui.onLineListWdt->count());
   return bAdd;
}

void MemberListWdg::CheckShowSpeakState(QString user_id, QString role_name, bool isSpeak,QListWidgetItem* itemWidget) {
   VH::CComPtr<ICommonData> pCommonData;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return , ASSERT(FALSE));
   ClientApiInitResp mRespInitData;
   pCommonData->GetInitApiResp(mRespInitData);
   isSpeak = isSpeak ? true : mpAlive->IsExistRenderWnd(user_id);
   if (mRespInitData.selectLiveType == eLiveType_Live) {
      itemWidget->setData(SHOW_SPEAK_STATE, "");
   }
   else {
      if (mRespInitData.role_name == USER_HOST) {
         int phoneUser = itemWidget->data(Qt::DisplayRole).value<VhallAudienceUserInfo>().mDevType == 1 ? 1 : 0;
         int handsUpState = itemWidget->data(SHOW_MAIN_VIEW).toInt();
         if (mpAlive->GetIsLiving() && (mRespInitData.join_uid == user_id || mRespInitData.user_id == user_id)) {
            if (mpAlive->GetMainSpeakerID() != user_id) {
               itemWidget->setData(SHOW_SPEAK_STATE, isSpeak ? DOWNMIC : UPPERWHEAT);
               if (isSpeak) {
                  itemWidget->setData(SHOW_MAIN_VIEW, STATE_SPEAKING);
                  itemWidget->setData(SHOW_MORE_MENU, 1);
               }
               else {
                  itemWidget->setData(SHOW_MAIN_VIEW, 0);
                  itemWidget->setData(SHOW_MORE_MENU, 0);
               }
            }
            else{
               itemWidget->setData(SHOW_MAIN_VIEW, STATE_STAR);
               itemWidget->setData(SHOW_SPEAK_STATE, "");
               itemWidget->setData(SHOW_MORE_MENU, 0);
            }
         }
         else if(isSpeak && mpAlive->GetIsLiving()){   //设置嘉宾，学员
            if (mpAlive->GetMainSpeakerID() == user_id) {
               itemWidget->setData(SHOW_MAIN_VIEW, STATE_STAR);
               itemWidget->setData(SHOW_SPEAK_STATE, DOWNMIC);
            }
            else {
               itemWidget->setData(SHOW_MAIN_VIEW, 0);
               itemWidget->setData(SHOW_SPEAK_STATE, DOWNMIC);
            }  
         }
         else { //非直播
            if (mpAlive->GetIsLiving() && phoneUser == 0) {
               itemWidget->setData(SHOW_SPEAK_STATE, UPPERWHEAT);
               if (handsUpState == STATE_SPEAKING) {
                  itemWidget->setData(SHOW_MAIN_VIEW, 0);
               }
            }
            else if(!mpAlive->GetIsLiving() && phoneUser == 0){
               if (role_name == USER_HOST) {
                  itemWidget->setData(SHOW_MAIN_VIEW, STATE_STAR);
               }
               else if(role_name == USER_GUEST){
                  itemWidget->setData(SHOW_SPEAK_STATE, UPPERWHEAT);
               }
            }    
         }
      }
      else{
         int phoneUser = itemWidget->data(Qt::DisplayRole).value<VhallAudienceUserInfo>().mDevType == 1 ? 1 : 0;
         int handsUpState = itemWidget->data(SHOW_MAIN_VIEW).toInt();
         if (mpAlive->GetIsLiving() && (mRespInitData.join_uid == user_id || mRespInitData.user_id == user_id)) { //处理嘉宾自己
            itemWidget->setData(SHOW_MORE_MENU, 0);
            itemWidget->setData(SHOW_SPEAK_STATE, "");
            if (mpAlive->GetMainSpeakerID() != user_id) {
               if (isSpeak) {
                  itemWidget->setData(SHOW_MAIN_VIEW, STATE_SPEAKING);
               }
               else {
                  itemWidget->setData(SHOW_MAIN_VIEW, STATE_HIDE_HANDS_UP);
               }
            }
            else {
               itemWidget->setData(SHOW_MAIN_VIEW, STATE_STAR);
            }
         }
         else if(mpAlive->GetIsLiving()){ //直播中处理主持人和观众
            if (mpAlive->GetMainSpeakerID() == user_id) {
               itemWidget->setData(SHOW_SPEAK_STATE, "");
               itemWidget->setData(SHOW_MAIN_VIEW, STATE_STAR);
            }
            else {
               if (isSpeak) {
                  itemWidget->setData(SHOW_MAIN_VIEW, STATE_SPEAKING);
               }
               else {
                  itemWidget->setData(SHOW_MAIN_VIEW, handsUpState ? STATE_HANDS_UP : STATE_HIDE_HANDS_UP);
                  if (handsUpState) {
                     itemWidget->setData(SHOW_HANDSUP_TIME, QDateTime::currentDateTime().toMSecsSinceEpoch());
                  }
               }
            }
         }
         else {//非直播嘉宾处理
            itemWidget->setData(SHOW_SPEAK_STATE, "");
            if (role_name == USER_HOST) {
               itemWidget->setData(SHOW_MAIN_VIEW, STATE_STAR);
            }
            else if (role_name == USER_GUEST || role_name == USER_ASSISTANT) {
               itemWidget->setData(SHOW_MAIN_VIEW, STATE_HIDE_HANDS_UP);
            }
            else if (role_name == USER_GUEST) {
               itemWidget->setData(SHOW_MAIN_VIEW, STATE_HIDE_HANDS_UP);
               itemWidget->setData(SHOW_MORE_MENU, 1);
            }
         }
      }
   }
}

bool MemberListWdg::addForbidChatMember(const VhallAudienceUserInfo &info, const bool& bForbid /*= true*/, const bool& bForAll/*= true*/)
{
   if (QString::fromStdWString(info.userId).isEmpty()) {
      return false;
   }
   bool badd = false;
   QString imagePath;
   VhallAudienceUserInfo infoTemp;
   QListWidgetItem *itemTemp = getOnlineItemWdt(QString::fromStdWString(info.userId));
   if (NULL != itemTemp){
      itemTemp->setData(SHOW_CHAT_FORBID, bForbid ? 1 : 0);
      itemTemp->setData(SHOW_MAIN_VIEW, STATE_HIDE_HANDS_UP);
      imagePath = itemTemp->data(SHOW_HEAD_IMAGE).toString();
   }
   else{
      infoTemp.userName = info.userName;
      infoTemp.role = info.role;
   }
   infoTemp.userId = info.userId;

   //如果没有上麦，但是在举手列表里面，显示举手。
   map<QString, QListWidgetItem*>::iterator iter = mRaiseHandsItemMap.find(QString::fromStdWString(info.userId));
   if (iter != mRaiseHandsItemMap.end()) {
      if (iter->second) {
         iter->second->setData(SHOW_CHAT_FORBID, bForbid ? 1 : 0);
         iter->second->setData(SHOW_MAIN_VIEW, STATE_HIDE_HANDS_UP);
      }
   }
   itemTemp = nullptr;
   map<QString, QListWidgetItem*>::iterator iterForBid = mForbidItemMap.find(QString::fromStdWString(info.userId));
   if (iterForBid != mForbidItemMap.end()) {
      itemTemp = iterForBid->second;
   }
   if (NULL == itemTemp){   //不存在
      if (bForbid){//禁言
         QListWidgetItem *item = new QListWidgetItem(ui.LimitMemberPage);
         if (NULL != item){
            QVariant var = QVariant::fromValue(info);
            item->setData(Qt::DisplayRole, var);
            item->setData(SHOW_CHAT_FORBID, bForbid ? 1 : 0);
            item->setData(SHOW_MAIN_VIEW, STATE_HIDE_HANDS_UP);
            item->setData(SHOW_HEAD_IMAGE, imagePath);
            CheckShowSpeakState(QString::fromStdWString(info.userId), QString::fromStdWString(info.role), info.mIsRtcPublisher, item);
            ui.LimitMemberPage->addItem(item);
            CheckShowMenu(QString::fromStdWString(info.userId), item);
            mForbidItemMap.insert(std::make_pair(QString::fromStdWString(info.userId), item));
            LoadMemberHeadImage(info, item);
         }
      }
   }
   else{//存在
      itemTemp->setData(SHOW_CHAT_FORBID, bForbid ? 1 : 0);
      int isKicked = itemTemp->data(SHOW_KICK_OUT).toInt();
      if (!bForbid){//取消禁言，判断如果没有提出则移除
         if (isKicked == 0) {
            RemoveInfo(ui.LimitMemberPage, QString::fromStdWString(info.userId));
            map<QString, QListWidgetItem*>::iterator iter = mForbidItemMap.find(QString::fromStdWString(info.userId));
            if (iter != mForbidItemMap.end()) {
               delete iter->second;
               mForbidItemMap.erase(iter);
            }
         }
      }
   }
   return badd;
}

bool MemberListWdg::addKickOutMember(const VhallAudienceUserInfo &info, const bool& bKickOut /*= true*/, const bool& bForAll/*= true*/){
   if (QString::fromStdWString(info.userId).isEmpty()) {
      return false;
   }
   bool bAdd = false;
   if (bKickOut) {
      VhallAudienceUserInfo infoTemp;
      QString imagePath;
      //更新在线成员列表中状态
      QListWidgetItem *itemTemp = getOnlineItemWdt(QString::fromStdWString(info.userId));
      if (NULL != itemTemp) {
         infoTemp = itemTemp->data(Qt::DisplayRole).value<VhallAudienceUserInfo>();
         itemTemp->setData(SHOW_KICK_OUT, 1);
         imagePath = itemTemp->data(SHOW_HEAD_IMAGE).toString();
      }
      else {
         infoTemp.userName = info.userName;
         infoTemp.role = info.role;
      }
      infoTemp.userId = info.userId;
      //如果是互动更新举手列表中状态
      map<QString, QListWidgetItem*>::iterator iterHandsUp = mRaiseHandsItemMap.find(QString::fromStdWString(info.userId));
      if (iterHandsUp != mRaiseHandsItemMap.end()) {
         QListWidgetItem* handsUpItem = iterHandsUp->second;
         if (NULL != handsUpItem) {
            itemTemp->setData(SHOW_KICK_OUT, 1);
         }
      }
      //踢出
      if (IsExistInKickOutList(QString::fromStdWString(info.userId))) {
         return true;
      }
      QListWidgetItem *item = new QListWidgetItem(ui.LimitMemberPage);
      if (NULL != item) {
         QVariant var = QVariant::fromValue(info);
         item->setData(Qt::DisplayRole, var);
         item->setData(SHOW_KICK_OUT,1);
         item->setData(SHOW_HEAD_IMAGE, imagePath);
         ui.LimitMemberPage->addItem(item);
         CheckShowMenu(QString::fromStdWString(info.userId), item);
         mForbidItemMap.insert(std::make_pair(QString::fromStdWString(info.userId), item));
         LoadMemberHeadImage(info, item);
      }
   }
   else {  //取消踢出
      map<QString, QListWidgetItem*>::iterator iterForBid = mForbidItemMap.find(QString::fromStdWString(info.userId));
      if (iterForBid != mForbidItemMap.end()) {
         QListWidgetItem *itemTemp = iterForBid->second;
         if (itemTemp) {
            itemTemp->setData(SHOW_KICK_OUT, 0);
            if (itemTemp->data(SHOW_CHAT_FORBID).toInt() == 0) { //如果受限同时没有被禁言,则从受限列表移除
               RemoveInfo(ui.LimitMemberPage, QString::fromStdWString(info.userId));
               map<QString, QListWidgetItem*>::iterator iter = mForbidItemMap.find(QString::fromStdWString(info.userId));
               if (iter != mForbidItemMap.end()) {
                  delete iter->second;
                  mForbidItemMap.erase(iter);
               }
            }
         }
      }
      QListWidgetItem *online = getOnlineItemWdt(QString::fromStdWString(info.userId));
      if (online) {
         online->setData(SHOW_KICK_OUT, 0);
      }
   }
   return bAdd;
}

QString MemberListWdg::GetUserNickName(const QString& strId) {
   QString name;
   QListWidgetItem* item = getOnlineItemWdt(strId);
   //if (item) {
   //   name = item->GetName();
   //}
   return name;
}

void MemberListWdg::SetEnableWheat(const bool& bEnable)
{

}

void MemberListWdg::SetSelfUpperWheat(const bool& bUpper/* = true*/)
{

}


void MemberListWdg::FlushBtnState(const QString& join_uid) {
      //map<QString, QListWidgetItem*>::iterator iter = mOnlineItemMap.find(join_uid);
      //if (iter != mOnlineItemMap.end()) {
      //   if (iter->second) {
      //      iter->second->FlushBtnState();
      //   }
      //}
}

void MemberListWdg::slot_HideFindMemberNotice() {
   ui.widget_findMemberNotice->hide();
}

void MemberListWdg::slot_OnTextChanged(const QString &text) {
   text.isEmpty() ? ui.pushButton_search_ok->setEnabled(false) : ui.pushButton_search_ok->setEnabled(true);
   text.isEmpty() ? ui.pushButton_search_ok->setStyleSheet(CanNotSend) : ui.pushButton_search_ok->setStyleSheet(CanSend);
}

void MemberListWdg::ShowSetmainSpeaker(const QString& join_uid) {
   map<QString, QListWidgetItem*>::iterator iter = mOnlineItemMap.begin();
   while (iter != mOnlineItemMap.end()) {
      if (iter->second) {
         VhallAudienceUserInfo user_info = iter->second->data(Qt::DisplayRole).value<VhallAudienceUserInfo>();
         QString item_user = QString::fromStdWString(user_info.userId);
         //只有上麦的主持人和嘉宾，可以显示成为主讲人
         CheckShowSpeakState(item_user, QString::fromStdWString(user_info.role), mpAlive->IsExistRenderWnd(item_user), iter->second);

      }
      iter++;
   }

}

void MemberListWdg::ResetSpeakState(QListWidget* listWidget) {
   for (int i = 0; i < listWidget->count(); i++) {
      QListWidgetItem* item = listWidget->item(i);
      if (item) {
         VhallAudienceUserInfo infoTemp = item->data(Qt::DisplayRole).value<VhallAudienceUserInfo>();
         QString speak_state = item->data(SHOW_SPEAK_STATE).toString();
         if (QString::fromStdWString(infoTemp.role) == USER_HOST) {
            item->setData(SHOW_MAIN_VIEW, STATE_STAR);
            item->setData(SHOW_MORE_MENU, 0);
         }
         else {
            if (speak_state == DOWNMIC) {
               item->setData(SHOW_SPEAK_STATE, UPPERWHEAT);
               item->setData(SHOW_MAIN_VIEW, STATE_HIDE_HANDS_UP);
            }
         }
      }
   }
}

void MemberListWdg::ResetSpeakState() {
   ResetSpeakState(ui.onLineListWdt);
   ResetSpeakState(ui.LimitMemberPage);
   ResetSpeakState(ui.serarchListWidget);
}

void MemberListWdg::UpdataOnLineUserHandsUpState(const QString& strId, bool handsUp) {
   QListWidgetItem* temp = getOnlineItemWdt(strId);
   if (temp) {
      temp->setData(SHOW_MAIN_VIEW, handsUp ? STATE_HANDS_UP : STATE_HIDE_HANDS_UP);
      if (handsUp) {
         temp->setData(SHOW_HANDSUP_TIME, QDateTime::currentDateTime().toMSecsSinceEpoch());
      }
   }
   if (!handsUp) {
      VhallAudienceUserInfo info;
      info.userId = strId.toStdWString();
      addRaiseHandsMember(info,false);
   }
}

void MemberListWdg::CancelHandsUpIcon() {
   //超过举手30秒时间，隐藏举手icon
   for (int i = 0; i < ui.onLineListWdt->count(); i++) {
      QListWidgetItem* item = ui.onLineListWdt->item(i);
      if (item) {
         VhallAudienceUserInfo infoTemp = item->data(Qt::DisplayRole).value<VhallAudienceUserInfo>();
         long long handsUpTime = item->data(SHOW_HANDSUP_TIME).toLongLong();
         long long currentTime = QDateTime::currentDateTime().toMSecsSinceEpoch();
         int handsUpState = item->data(SHOW_MAIN_VIEW).toInt();
         if (currentTime - handsUpTime >= 30000 && handsUpState == STATE_HANDS_UP) {
            item->setData(SHOW_MAIN_VIEW, STATE_HIDE_HANDS_UP);
            RemoveInfo(ui.raiseHandsListWdt, QString::fromStdWString(infoTemp.userId));
            map<QString, QListWidgetItem*>::iterator iter = mRaiseHandsItemMap.find(QString::fromStdWString(infoTemp.userId));
            if (iter != mRaiseHandsItemMap.end()) {
               delete iter->second;
               mRaiseHandsItemMap.erase(iter);
            }
         }
      }
   }
}

bool MemberListWdg::addRaiseHandsMember(const VhallAudienceUserInfo &info, const bool& userHandsUp /*= true*/, const bool& bForAll/*= true*/){
   bool badd = false;
   if (userHandsUp){
      if (IsExistUserInList(ui.raiseHandsListWdt, QString::fromStdWString(info.userId))) {
         return true;
      }
      VhallAudienceUserInfo infoTemp;
      QListWidgetItem *item = new QListWidgetItem(ui.raiseHandsListWdt);
      if (NULL != item){
         ui.raiseHandsListWdt->addItem(item);
         CheckShowMenu(QString::fromStdWString(info.userId), item);
         QVariant var = QVariant::fromValue(info);
         item->setData(Qt::DisplayRole, var);
         item->setData(SHOW_CHAT_FORBID, info.gagType == VhallShowType_Allow ? 1 : 0);
         item->setData(SHOW_KICK_OUT, info.kickType == VhallShowType_Allow ? 1 : 0);
         item->setData(SHOW_MAIN_VIEW, STATE_HANDS_UP);
         item->setData(SHOW_HANDSUP_TIME, QDateTime::currentDateTime().toMSecsSinceEpoch());
         item->setData(SHOW_SPEAK_STATE, UPPERWHEAT);
         mRaiseHandsItemMap.insert(std::make_pair(QString::fromStdWString(info.userId), item));
         LoadMemberHeadImage(info, item);
      }
      UpdataOnLineUserHandsUpState(QString::fromStdWString(info.userId), true);
   }
   else{
      RemoveInfo(ui.raiseHandsListWdt, QString::fromStdWString(info.userId));
      map<QString, QListWidgetItem*>::iterator iter = mRaiseHandsItemMap.find(QString::fromStdWString(info.userId));
      if (iter != mRaiseHandsItemMap.end()) {
         delete iter->second;
         mRaiseHandsItemMap.erase(iter);
      }
      QListWidgetItem* temp = getOnlineItemWdt(QString::fromStdWString(info.userId));
      if (temp) {
         temp->setData(SHOW_MAIN_VIEW, STATE_HIDE_HANDS_UP);
      }
   }

   if (eTabMember_raiseHands != ui.stackedWidget->currentIndex()){
      VH::CComPtr<ICommonData> pCommonData;
      DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return false, ASSERT(FALSE));
      ClientApiInitResp mRespInitData;
      pCommonData->GetInitApiResp(mRespInitData);
      if (mRespInitData.role_name == USER_HOST) {
         ui.btnRaiseHands->SetHandsUpState(eHandsUpState_existence);
      }
   }
   calcListNum();
   return badd;
}

void MemberListWdg::SetUpperStatesById(const QString& strId, const bool& bUpper){
   bool bRef = false;
   int iCount = ui.onLineListWdt->count();
   for (int i = iCount - 1; i >= 0; i--) {
      QListWidgetItem *item = ui.onLineListWdt->item(i);
      if (NULL != item) {
         QWidget *w = ui.onLineListWdt->itemWidget(item);
         if (NULL != w) {
            CMemberItem *itemWidget = dynamic_cast<CMemberItem *>(w);
            if (itemWidget->isCurrentUser(strId)) {
               itemWidget->SetUpperStates(bUpper);
            }
         }
      }
   }
}

void MemberListWdg::show(const QRect& rect){
   if (isMinimized()) {
      int iX = rect.x() + (rect.width() - mNormalRect.width()) / 2;
      int iy = rect.y() + (rect.height() - mNormalRect.height()) / 2;
      QWidget::showNormal();
      this->move(iX, iy);
   }
   else {
      int iX = rect.x() + (rect.width() - mNormalRect.width()) / 2;
      int iy = rect.y() + (rect.height() - mNormalRect.height()) / 2;
      QWidget::show();
      this->move(iX, iy);
   }
   if (!ui.btnRefresh->isEnabled()) {
      startRefreshUpdateTime();
   }
}

bool compareOnlineData(const COnlineItemWdg& pwdg1, const COnlineItemWdg& pwdg2){
   return pwdg1 < pwdg2;
}

QListWidgetItem* MemberListWdg::getOnlineItemWdt(const QString& strId) {
   map<QString, QListWidgetItem*>::iterator it = mOnlineItemMap.find(strId);
   if (it != mOnlineItemMap.end()) {
      return it->second;
   }
   return nullptr;
}


void MemberListWdg::RemoveOnLineItemWdt(const QString& strId){
   TRACE6("%s uid %s\n", __FUNCTION__, strId.toStdString().c_str());
   map<QString, QListWidgetItem*>::iterator iter = mOnlineItemMap.find(strId);
   if (iter != mOnlineItemMap.end()) {
      VhallAudienceUserInfo infoTemp = iter->second->data(Qt::DisplayRole).value<VhallAudienceUserInfo>();
      if (QString::fromStdWString(infoTemp.role) == USER_USER) {
         mUserMemberCount--;
      }
      ui.onLineListWdt->removeItemWidget(iter->second);       
      delete iter->second;
      mOnlineItemMap.erase(iter);
   }
   ui.labCount->setText(QString::fromWCharArray(L"%1人在线").arg(miOnlineUserCont < 0 ? 0 : miOnlineUserCont));
   TRACE6("%s onLineListWdt item count %d\n", __FUNCTION__, ui.onLineListWdt->count());
}

void MemberListWdg::slot_Upper(const QString & strId, const bool & bUpper){
   map<QString, QListWidgetItem*>::iterator it = mOnlineItemMap.find(strId);
   if (it != mOnlineItemMap.end()) {
      QListWidgetItem* temp = it->second;
      if (temp) {
         //temp->SetUpperStates(bUpper);
      }
   }

  /* it = mForbidItemMap.find(strId);
   if (it != mForbidItemMap.end()) {
      COnlineItemWdg* temp = it->second;
      if (temp) {
         temp->SetUpperStates(bUpper);
      }
   }

   it = mRaiseHandsItemMap.find(strId);
   if (it != mRaiseHandsItemMap.end()) {
      COnlineItemWdg* temp = it->second;
      if (temp) {
         temp->SetUpperStates(bUpper);
      }
   }*/
}

void MemberListWdg::on_btnClose_clicked(bool checked /*= false*/){
   emit sig_Hide();
   close();
}


void MemberListWdg::on_btnMin_clicked(bool checked /*= false*/)
{
   this->showMinimized();
}

void MemberListWdg::SetCurrentListTab(const eTabMember& tabM)
{
   ui.stackedWidget->setCurrentIndex(tabM);
}

void MemberListWdg::SetAlive(CAliveDlg* pAlive)
{
   mpAlive = pAlive;

}

void MemberListWdg::setHansUpStates(const bool& bChecked)
{
   if (bChecked) {
      QString sheet = StrEnableType;
      ui.tbtnAllowRaiseHands->setStyleSheet(sheet);
   }
   else {
      QString sheet = StrDisableType;
      ui.tbtnAllowRaiseHands->setStyleSheet(sheet);
   }
}

void MemberListWdg::SetHansUpShow(const bool& bshow)
{
   if (bshow) {
      ui.btnRaiseHands->show();
      ui.tbtnAllowRaiseHands->show();
      ui.labAllowRaiseHands->show();
   }
   else {
      ui.btnRaiseHands->hide();
      ui.tbtnAllowRaiseHands->hide();
      ui.labAllowRaiseHands->hide();
   }
}

bool MemberListWdg::GetHansUpShow(){
   return ui.btnRaiseHands->isVisible();
}

void MemberListWdg::NtfUserOnline(const VhallAudienceUserInfo &Info) { //上线
   if (VhallShowType_Allow == Info.gagType) {
      addForbidChatMember(Info, true, false);
   }
   StartCheckMemberList();
   bool bAdd = true;
   if (USER_USER == QString::fromStdWString(Info.role)) { //普通用户
      QListWidgetItem* temp = getOnlineItemWdt(QString::fromStdWString(Info.userId));
      if (Info.miUserCount == -1) {
         if (temp == nullptr) {
            miOnlineUserCont++;
         }
      }
      else {
         setUserCount(Info.miUserCount);
      }
      if (ui.onLineListWdt->count() < MAX_SHOW_USER_COUNT) {
         bAdd = addOnlineMember(Info, false, false);
      }
   }
   else {
      if (1 == mCurPage || 1 == mPageCont) {
         QListWidgetItem* temp = getOnlineItemWdt(QString::fromStdWString(Info.userId));
         if (!temp) {
            if (Info.miUserCount == -1) {
               miOnlineUserCont++;
            }
            else {
               setUserCount(Info.miUserCount);
            }
         }
         bAdd = addOnlineMember(Info, false, false);
      }
   }
   calcListNum();
}

void MemberListWdg::StartCheckMemberList() {
   if (mCheckMemberListTimer) {
      mCheckMemberListTimer->stop();
      mCheckMemberListTimer->start(MAX_CHECK_MEMBERLIST);
      mCheckMemberListTimer->setSingleShot(true);
   }
}

void MemberListWdg::NtfUserOffLine(const QString& strId, int iUserCount){
   if (iUserCount != -1) {
      setUserCount(iUserCount);
   }
   else {
      if (miOnlineUserCont > 0) {
         miOnlineUserCont--;
      }
   }
   StartCheckMemberList();
   ui.labCount->setText(QString::fromWCharArray(L"%1人在线").arg(miOnlineUserCont < 0 ? 0 : miOnlineUserCont));
   RemoveOnLineItemWdt(strId);
   RemoveInfo(ui.raiseHandsListWdt, strId);
   map<QString, QListWidgetItem*>::iterator iter = mRaiseHandsItemMap.find(strId);
   if (iter != mRaiseHandsItemMap.end()) {
      delete iter->second;
      mRaiseHandsItemMap.erase(iter);
   }
   calcListNum();
   int iCont = mOnlineItemMap.size();
   if (0 == iCont) {
      on_btnRefresh_clicked();
   }
   TRACE6("%s onLineListWdt item count %d\n", __FUNCTION__, ui.onLineListWdt->count());
}

int MemberListWdg::calcWdgHeight(const int&  iRowCount) const
{
   int iheight = 0;
   iheight = 53 * iRowCount;
   if (iheight > 417){
      iheight = 417;
   }
   return iheight; //上下两个边框 距离是2
}

void MemberListWdg::calcListNum() {
   //成员列表
   ui.labCount->setText(QString::fromWCharArray(L"%1人在线").arg(miOnlineUserCont < 0 ? 0 : miOnlineUserCont));
   //举手列表
   int iCont = ui.raiseHandsListWdt->count();
   if (iCont <= 0) {
      ui.raiseHandsNoMember->show();
      ui.raiseHandsListWdt->hide();
   }
   else {
      ui.raiseHandsNoMember->hide();
      ui.raiseHandsListWdt->show();
   }

   //受限列表
   iCont = ui.LimitMemberPage->count();
   if (iCont <= 0) {
      ui.limitNoMember->show();
      ui.LimitMemberPage->hide();
   }
   else {
      ui.limitNoMember->hide();
      ui.LimitMemberPage->show();
   }
   TRACE6("%s onLineListWdt item count %d\n", __FUNCTION__, ui.onLineListWdt->count());
}

void MemberListWdg::dealNotSpeak(const QString& strId) {
   QListWidgetItem *itemTemp = getOnlineItemWdt(strId);
   if (NULL != itemTemp) {
      itemTemp->setData(SHOW_MAIN_VIEW, STATE_HIDE_HANDS_UP);
   }
}

void MemberListWdg::addSpeaker(Event& e) {
   QString id = QString::fromStdWString(e.m_oUserInfo.userId);
   if (id == "0") {
      return;
   }
   QListWidgetItem *itemTemp = getOnlineItemWdt(id);
   if (NULL != itemTemp) {
      CheckShowSpeakState(id, QString::fromStdWString(e.m_oUserInfo.role),true, itemTemp);
      VhallAudienceUserInfo uInfo = e.m_oUserInfo;
      if (QString::fromStdWString(e.m_oUserInfo.role) == USER_USER) {
         VhallAudienceUserInfo user_info;
         user_info.userId = e.m_oUserInfo.userId;
         user_info.role = e.m_oUserInfo.role;
         user_info.mIsRtcPublisher = true;
         SortMemberItem(ui.onLineListWdt, user_info, itemTemp);
      }
   }
   else {
      VhallAudienceUserInfo uInfo = e.m_oUserInfo;
      uInfo.mIsRtcPublisher = true;
      if (QString::fromStdWString(e.m_oUserInfo.role) != USER_USER) {
         addOnlineMember(uInfo, true, false);
      }
      else {
         addOnlineMember(uInfo, false, false);
      }
   }
   RemoveInfo(ui.raiseHandsListWdt, id);//上麦
   map<QString, QListWidgetItem*>::iterator iter = mRaiseHandsItemMap.find(id);
   if (iter != mRaiseHandsItemMap.end()) {
      delete iter->second;
      mRaiseHandsItemMap.erase(iter);
   }
}

void MemberListWdg::DealSwitchHandsUp(const Event& e)
{

}

void MemberListWdg::HandleDownLoadHead(QEvent *event) {
   CustomDownLoadEvent* custom_event = dynamic_cast<CustomDownLoadEvent*>(event);
   if (custom_event) {
      map< QString, QListWidgetItem* >::iterator iter = mOnlineItemMap.find(custom_event->mMsg);
      if (iter != mOnlineItemMap.end()) {
         iter->second->setData(SHOW_HEAD_IMAGE,custom_event->mSavePath);
      }
       iter = mForbidItemMap.find(custom_event->mMsg);
      if (iter != mForbidItemMap.end()) {
         iter->second->setData(SHOW_HEAD_IMAGE, custom_event->mSavePath);
      }
       iter = mRaiseHandsItemMap.find(custom_event->mMsg);
      if (iter != mRaiseHandsItemMap.end()) {
         iter->second->setData(SHOW_HEAD_IMAGE, custom_event->mSavePath);
      }
   }
}

void MemberListWdg::DealSwitchHansUp(bool open) {
   if (open) {
      QString sheet = StrEnableType;
      ui.tbtnAllowRaiseHands->setStyleSheet(sheet);
   }
   else {
      QString sheet = StrDisableType;
      ui.tbtnAllowRaiseHands->setStyleSheet(sheet);
      clearInfoList(ui.raiseHandsListWdt);
      mRaiseHandsItemMap.clear();
      if (eTabMember_raiseHands == ui.stackedWidget->currentIndex()) {
         ui.stackedWidget->setCurrentIndex(eTabMember_onLine);
      }
   }
}

void MemberListWdg::on_btnOnLine_clicked(bool checked /*= false*/)
{
   ui.stackedWidget->setCurrentIndex(eTabMember_onLine);
   UpdatePageBtn(eTabMember_onLine);
   if (mbOpenSearch) {
      ui.widget_search->show();
      ui.widget_memberInfo->hide();
   }
   else {
      ui.widget_search->hide();
      ui.widget_memberInfo->show();
   }
   ui.btnLookUp->show();
}

void MemberListWdg::on_btnRaiseHands_clicked(bool checked /*= false*/){
   ui.btnRaiseHands->SetHandsUpState(eHandsUpState_No);
   ui.stackedWidget->setCurrentIndex(eTabMember_raiseHands);
   UpdatePageBtn(eTabMember_raiseHands);
   ui.widget_search->hide();
   ui.btnLookUp->hide();
   ui.widget_memberInfo->show();
   emit sig_ClickedHandsUpBtn();
}

void MemberListWdg::on_btnProhibitSpeaking_clicked(bool checked /*= false*/){
   ui.stackedWidget->setCurrentIndex(eTabMember_limit);
   UpdatePageBtn(eTabMember_limit);
   ui.widget_search->hide();
   ui.btnLookUp->hide();
   ui.widget_memberInfo->show();
}

void MemberListWdg::on_btnKickOut_clicked(bool checked /*= false*/){
   ui.stackedWidget->setCurrentIndex(eTabMember_onLine);
   UpdatePageBtn(eTabMember_onLine);
}

void MemberListWdg::UpdatePageBtn(int page) {
   if (page == eTabMember_raiseHands) {
      ui.btnOnLine->setStyleSheet(OnLineMemberPageBtnNoSelect);
      ui.btnRaiseHands->setStyleSheet(HandsUpMemberPageBtnSelect);
      ui.btnProhibitSpeaking->setStyleSheet(LimitMemberPageBtnNoSelect);
   }
   else if (page == eTabMember_limit) {
      ui.btnOnLine->setStyleSheet(OnLineMemberPageBtnNoSelect);
      ui.btnRaiseHands->setStyleSheet(HandsUpMemberPageBtnNoSelect);
      ui.btnProhibitSpeaking->setStyleSheet(LimitMemberPageBtnSelect);
   }
   else if (page == eTabMember_onLine) {
      ui.btnOnLine->setStyleSheet(OnLineMemberPageBtnSelect);
      ui.btnRaiseHands->setStyleSheet(HandsUpMemberPageBtnNoSelect);
      ui.btnProhibitSpeaking->setStyleSheet(LimitMemberPageBtnNoSelect);
   }
}

void MemberListWdg::clearData(){
   mUserMemberCount = 0;
   mOnlineItemMap.clear();
   mForbidItemMap.clear();
   mRaiseHandsItemMap.clear();
   setUserCount();
   //先清空列表中数据
   clearInfoList(ui.onLineListWdt);
   clearInfoList(ui.raiseHandsListWdt);
   clearInfoList(ui.LimitMemberPage);
   clearInfoList(ui.serarchListWidget);
}

void MemberListWdg::initUpper(COnlineItemWdg * Itepwd, QString id){

}

void MemberListWdg::startRefreshUpdateTime(){
   if (mpRefreshTime == NULL){
      mpRefreshTime = new QTimer(this);
      connect(mpRefreshTime, &QTimer::timeout, this, &MemberListWdg::slotEnableRefresh);
   }
   if (NULL != mpRefreshTime){
      if (mpRefreshTime->isActive()){
         mpRefreshTime->stop();
      }
      ui.btnRefresh->setEnabled(false);
      mpRefreshTime->start(5000);
   }
}

void MemberListWdg::on_btnRefresh_clicked(bool checked /*= false*/) {
   startRefreshUpdateTime();
   mReqPage = 1;
   clearData();
   emit signal_Refresh();
}

void MemberListWdg::setUserCount(int iCount/* = -1*/){
   miOnlineUserCont = iCount;
}

void MemberListWdg::on_tbtnAllowRaiseHands_clicked(bool checked /*= false*/){
   QString sheet = ui.tbtnAllowRaiseHands->styleSheet();
   //当前处于开状态，那么进关。
   if (sheet == StrEnableType) {
      emit sigAllowRaiseHands(false);
   }
   else {
      emit sigAllowRaiseHands(true);
   }
}

void MemberListWdg::slotEnableRefresh(){
   if (NULL != mpRefreshTime && mpRefreshTime->isActive()){
      mpRefreshTime->stop();
   }
   ui.btnRefresh->setEnabled(true);
}

bool MemberListWdg::IsExistInKickOutList(const QString &strId) {
   bool bRef = false;
   int iCount = ui.LimitMemberPage->count();
   for (int i = iCount - 1; i >= 0; i--) {
      QListWidgetItem *item = ui.LimitMemberPage->item(i);
      if (NULL != item) {
         VhallAudienceUserInfo infoTemp = item->data(Qt::DisplayRole).value<VhallAudienceUserInfo>(); 
         if (infoTemp.userId == strId) {
            bRef = true;
            item->setData(SHOW_KICK_OUT, 1);
            break;
         }
      }
   }
   return bRef;
}

bool MemberListWdg::IsExistUserInList(QListWidget* listWidget ,const QString &strId) {
   bool bRef = false;
   int iCount = listWidget->count();
   for (int i = iCount - 1; i >= 0; i--) {
      QListWidgetItem *item = listWidget->item(i);
      if (NULL != item) {
         VhallAudienceUserInfo user_info = item->data(Qt::DisplayRole).value<VhallAudienceUserInfo>();
         if (user_info.userId == strId.toStdWString()) {
            return true;
         }
      }
   }
   return bRef;
}

bool MemberListWdg::RemoveInfo(QListWidget *pListWdt, const QString &strId)
{
   bool bRef = false;
   int iCount = pListWdt->count();
   for (int i = iCount - 1; i >= 0; i--) {
      QListWidgetItem *item = pListWdt->item(i);
      if (NULL != item) {
         VhallAudienceUserInfo user_info = item->data(Qt::DisplayRole).value<VhallAudienceUserInfo>();
         if (user_info.userId == strId) {
            pListWdt->takeItem(i);
         }
      }
   }
   return bRef;
}

void MemberListWdg::clearInfoList(QListWidget *pListWdt){
   while (pListWdt->count() > 0) {
      QListWidgetItem *item = pListWdt->item(0);
      if (NULL != item) {
         item = pListWdt->takeItem(0);
         delete item;
         item = NULL;
      }
   }
   pListWdt->clear();
}

void MemberListWdg::ProhibitChat(const bool& bProhibit /*= true*/)
{
   ProhibitChatWdg(ui.onLineListWdt, bProhibit);
   ProhibitChatWdg(ui.LimitMemberPage, bProhibit);
   ProhibitChatWdg(ui.LimitMemberPage, bProhibit);
   ProhibitChatWdg(ui.raiseHandsListWdt, bProhibit);
}

void MemberListWdg::HandleSetMain(const Event& e) {
   QString id = QString::fromStdWString(e.m_oUserInfo.userId);
   VH::CComPtr<ICommonData> pCommonData;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
   ClientApiInitResp respInitData;
   pCommonData->GetInitApiResp(respInitData);
   if (respInitData.role_name == USER_HOST) {
      if (respInitData.user_id == id) {
         QListWidgetItem* item = getOnlineItemWdt(respInitData.user_id);
         if (item != nullptr) {
            item->setData(SHOW_MAIN_VIEW, STATE_STAR);
         }
      }
      if (respInitData.user_id != id && mpAlive) { //主持人不是当前主讲人。
         QListWidgetItem* item = getOnlineItemWdt(respInitData.user_id);
         if (item != nullptr) {
            item->setData(SHOW_MAIN_VIEW, mpAlive->IsExistRenderWnd(respInitData.user_id) ? STATE_SPEAKING : 0);
         }
      }
   }
}

void MemberListWdg::ProhibitChatWdg(QListWidget *pListWdt, const bool& bProhibit /*= true*/)
{
   for (int i = 0; i < pListWdt->count(); i++) {
      QListWidgetItem *item = pListWdt->item(i);
      if (item) {
         QWidget *w = pListWdt->itemWidget(item);
         if (w) {
            CMemberItem *itemWidget = dynamic_cast<CMemberItem *>(w);
            itemWidget->SetProhibitChatStates(bProhibit);
         }
      }
   }
}
//
//void MemberListWdg::on_tBtnPre_clicked(bool checked /*= false*/){
//   mUserMemberCount = 0;
//   mOnlineItemMap.clear();
//   //先清空列表中数据
//   clearInfoList(ui.onLineListWdt);
//   mReqPage = mCurPage - 1;
//   //发出请求某一个页面的用户列表
//   emit sig_ReqApointPageUserList(mReqPage);
//}
//
//void MemberListWdg::on_tBtnNext_clicked(bool checked /*= false*/){
//   mUserMemberCount = 0;
//   mOnlineItemMap.clear();
//   //先清空列表中数据
//   clearInfoList(ui.onLineListWdt);
//   mReqPage = mCurPage + 1;
//   //发出请求某一个页面的用户列表
//   emit sig_ReqApointPageUserList(mReqPage);
//}
//
//void MemberListWdg::on_tBtnHomePage_clicked(bool checked/* = false*/){
//   if (1 != mCurPage){
//      mUserMemberCount = 0;
//      mOnlineItemMap.clear();
//      //先清空列表中数据
//      clearInfoList(ui.onLineListWdt);
//      mReqPage = 1;
//      emit sig_ReqApointPageUserList(mReqPage);
//   }
//}

void MemberListWdg::on_btnClear_clicked(bool checked /*= false*/) {
   ui.ledtLookUp->setText("");
   ui.horizontalSpacer->changeSize(216, 20, QSizePolicy::Expanding);
   ui.widget_pageBtn->show();
   //ui.widget_memberInfo->show();
   //ui.widget_search->hide();
   clearInfoList(ui.serarchListWidget);
   on_btnOnLine_clicked();
}

void MemberListWdg::on_ledtLookUp_textChanged(const QString &text) {
   map< QString, QListWidgetItem* >::iterator ite = mOnlineItemMap.begin();
   QString strContain = text.trimmed().toLower();
   if (strContain.compare("null") == 0) {
      strContain = "";
   }

   //QListWidgetItem * item = NULL;
   //for (int i = ui.onLineListWdt->count() - 1; i >= 0; i--) {
   //   if (strContain.trimmed().isEmpty()) {
   //      //显示
   //      ui.onLineListWdt->setItemHidden(ui.onLineListWdt->item(i), false);
   //   }
   //   else {
   //      item = ui.onLineListWdt->item(i);
   //      QWidget *w = ui.onLineListWdt->itemWidget(item);
   //      if (NULL != w) {
   //         COnlineItemWdg *itemWidget = dynamic_cast<COnlineItemWdg *>(w);
   //         if (itemWidget) {
   //            if (itemWidget->GetId().toLower().contains(strContain))//id匹配
   //            {
   //               //显示
   //               ui.onLineListWdt->setItemHidden(ui.onLineListWdt->item(i), false);
   //            }
   //            else if (itemWidget->GetName().toLower().contains(strContain)) //昵称匹配
   //            {
   //               //显示
   //               ui.onLineListWdt->setItemHidden(ui.onLineListWdt->item(i), false);
   //            }
   //            else {
   //               //隐藏
   //               ui.onLineListWdt->setItemHidden(ui.onLineListWdt->item(i), true);
   //            }
   //         }
   //      }
   //   }
   //}
}

void MemberListWdg::slot_OnCheckMemberListTimeout() {
   int memberInListCount = ui.onLineListWdt->count();
   TRACE6("%s memberInListCount:%d miOnlineUserCont:%d\n", __FUNCTION__, memberInListCount, miOnlineUserCont);
   if (miOnlineUserCont < MAX_SHOW_USER_COUNT && memberInListCount != miOnlineUserCont) {
      on_btnRefresh_clicked();
      if (mCheckMemberListTimer) {
         mCheckMemberListTimer->stop();
      }
   }
}

void MemberListWdg::slot_mouseMove() {
   if (mbIsEnter && mEnterMemberList) {
      QPoint pos = QCursor::pos();
      pos = ui.onLineListWdt->mapFromGlobal(pos);
      for (int i = 0; i < mEnterMemberList->count(); i++) {
         QListWidgetItem* item = mEnterMemberList->itemAt(pos);
         if (item) {
            item->setSelected(true);
         }
      }
   }
}

bool MemberListWdg::eventFilter(QObject *object, QEvent *event) {
   if (object == ui.onLineListWdt || object == ui.raiseHandsListWdt || object == ui.serarchListWidget || object == ui.LimitMemberPage) {
      mEnterMemberList = (QListWidget*)(object);
      QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
      if (mouseEvent) {
         if (mouseEvent->type() == QMouseEvent::Enter) {
            mbIsEnter = true;
            if (!mMouseMoveTimer->isActive()) {
               mMouseMoveTimer->start(50);
            }
         }
         else if (mouseEvent->type() == QMouseEvent::Leave) {
            mbIsEnter = false;
            mMouseMoveTimer->stop();
            for (int i = 0; i < mEnterMemberList->count(); i++) {
               QListWidgetItem* item = mEnterMemberList->item(i);
               if (item) {
                  item->setSelected(true);
               }
            }
         }
      }
   }
   return QWidget::eventFilter(object,event);
}

void MemberListWdg::customEvent(QEvent* event) {
   if (event) {
      if (event->type() == CustomEvent_CustomRecvSocketIOMsgEvent) {
         CustomSocketIOMsg* socketIOMsg = dynamic_cast<CustomSocketIOMsg*>(event);
         Event event;
         if (socketIOMsg) {
            if (AnalysisEventObject(socketIOMsg->recvObj, event)) {
               RecvUserListMsg(event);
            }
         }
      }
      else if (event->type() == CustomEvent_DownLoadFile) {
         CustomDownLoadEvent* socketIOMsg = dynamic_cast<CustomDownLoadEvent*>(event);
         if (socketIOMsg) {
            QListWidgetItem* wdg = getOnlineItemWdt(socketIOMsg->mMsg);
            if (wdg) {
               //wdg->LoadImage(socketIOMsg->mSavePath);
            }
         }
      }
   }
}

void MemberListWdg::slot_DownLoadPic(QString join_id, QString path) {
   map<QString, QListWidgetItem*>::iterator iter = mOnlineItemMap.find(join_id);
   if (iter != mOnlineItemMap.end()) {
      if (iter->second) {
         iter->second->setData(SHOW_HEAD_IMAGE, path);
      }
   }
}

void MemberListWdg::slot_HostCheckHandsUpTimeout() {
   CancelHandsUpIcon();
}

void MemberListWdg::slot_OnLineMemberSliderChanged(int v) {
   if (v == ui.onLineListWdt->verticalScrollBar()->maximum()) {
      if (ui.onLineListWdt->count() >= MAX_SHOW_USER_COUNT) {
         if (mFindMemberNotice) {
            ui.widget_findMemberNotice->show();
            mFindMemberNotice->setSingleShot(true);
            mFindMemberNotice->start(3000);
         }
      }

      QString nickName = ui.ledtLookUp->text();
      VH::CComPtr<ICommonData> pCommonData;
      ClientApiInitResp initResp;
      DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
      pCommonData->GetInitApiResp(initResp);
      QString vss_domain = ConfigSetting::ReadString(CPathManager::GetToolConfigPath(), GROUP_DEFAULT, KEY_VSS_URL_DOMAIN, vss_domainUrl);
      VSSAPIManager vssApi;

      QObject* recvObj = pCommonData->MainUiLoginObj();
      vssApi.InitAPIRequest(vss_domain.toStdString(), initResp.vss_token.toStdString(), initResp.vss_room_id.toStdString());
      if (initResp.player == H5_LIVE_CREATE_TYPE) {
         vssApi.VSSGetOnlineList(mCurPage + 1, 50, [&, recvObj](int libCurlCode, const std::string& msg, int page, int pagesize) {
            MemberParam param;
            int nCode;
            QString outMsg;
            Event memberDetail;
            memberDetail.Init();
            memberDetail.m_memberSlider = 1;
            param.ParamToMemberInfo(libCurlCode, QString::fromStdString(msg), e_RQ_UserOnlineList, nCode, outMsg, &memberDetail);
            if (nCode == 0 || nCode != 200) {
               return;
            }
            if (memberDetail.m_oUserList.size() == 0) {
               return;
            }
            QJsonObject obj = VHAliveInteraction::MakeEventObject(&memberDetail);
            if (recvObj) {
               QCoreApplication::postEvent(recvObj, new CustomSocketIOMsg(CustomEvent_CustomRecvSocketIOMsgEvent, obj), Qt::LowEventPriority);
            }
         });
      }
      else {
         WCHAR wzRQUrl[2048] = { 0 };
         StartLiveUrl liveUrl;
         QString e_domain = ConfigSetting::ReadString(CPathManager::GetToolConfigPath(), GROUP_DEFAULT, KEY_URL_DOMAIN, domainUrl);
         pCommonData->GetStartLiveUrl(liveUrl);
         QString httpUrl = QString("%1/api/client/v1/clientapi/onlineusers?token=%2&curr_page=%3").arg(e_domain).arg(liveUrl.msg_token).arg(mCurPage + 1);
         if (!httpUrl.isEmpty()) {
            HTTP_GET_REQUEST request(httpUrl.toStdString());
            request.SetHttpPost(true);
            GetHttpManagerInstance()->HttpGetRequest(request, [&, recvObj](const std::string& msg, int code, const std::string userData) {
               if (msg.size() > 0 && code == 0) {
                  Event oReq;
                  oReq.Init();
                  oReq.m_memberSlider = 1;
                  VHAliveInteraction::ParseJSon(e_RQ_UserOnlineList, QString::fromStdString(msg), oReq);
                  QJsonObject obj = VHAliveInteraction::MakeEventObject(&oReq);
                  if (recvObj) {
                     CustomSocketIOMsg *socketMsg = new CustomSocketIOMsg(CustomEvent_CustomRecvSocketIOMsgEvent, obj);
                     if (socketMsg) {
                        QCoreApplication::postEvent(recvObj, socketMsg, Qt::LowEventPriority);
                     }
                  }
               }
            });
         }
      }
   }
   else {
      if (mFindMemberNotice) {
         ui.widget_findMemberNotice->hide();
         mFindMemberNotice->stop();
      }
   }
}

void MemberListWdg::slot_SearchSignalMember() {
   QString nickName = ui.ledtLookUp->text();
   if (nickName.isEmpty()) {
      return;
   }
   VH::CComPtr<ICommonData> pCommonData;
   ClientApiInitResp initResp;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
   pCommonData->GetInitApiResp(initResp);
   QString domain = ConfigSetting::ReadString(CPathManager::GetToolConfigPath(), GROUP_DEFAULT, KEY_VSS_URL_DOMAIN, vss_domainUrl);
   VSSAPIManager vssApi;
   QObject* recvObj = pCommonData->MainUiLoginObj();
   vssApi.InitAPIRequest(domain.toStdString(), initResp.vss_token.toStdString(), initResp.vss_room_id.toStdString());
   if (initResp.player == H5_LIVE_CREATE_TYPE) {
      vssApi.VSSSearchOnLineMember(nickName.toStdString(), [&, recvObj](int libCurlCode, const std::string& msg, int page, int pagesize) {
         MemberParam param;
         int nCode;
         QString outMsg;
         Event memberDetail;
         memberDetail.Init();
         param.ParamToMemberInfo(libCurlCode, QString::fromStdString(msg), e_RQ_SearchOnLineMember, nCode, outMsg, &memberDetail);
         QJsonObject obj = VHAliveInteraction::MakeEventObject(&memberDetail);
         if (recvObj) {
            QCoreApplication::postEvent(recvObj, new CustomSocketIOMsg(CustomEvent_CustomRecvSocketIOMsgEvent, obj), Qt::LowEventPriority);
         }
      });
   }
   else {
      StartLiveUrl mStartLiveData;
      pCommonData->GetStartLiveUrl(mStartLiveData);
      QObject* recvObj = pCommonData->MainUiLoginObj();
      QString domain_Url = ConfigSetting::ReadString(CPathManager::GetToolConfigPath(), GROUP_DEFAULT, KEY_URL_DOMAIN, domainUrl);
      QString httpUrl = domain_Url + QString("/api/client/v1/clientapi/onlineusers?token=%1&curr_page=1&nick_name=%2").arg(mStartLiveData.msg_token).arg(nickName);
      TRACE6("%s url:%s\n", __FUNCTION__, httpUrl.toStdString().c_str());
      HTTP_GET_REQUEST request(httpUrl.toStdString());
      request.SetHttpPost(true);
      GetHttpManagerInstance()->HttpGetRequest(request, [&, recvObj](const std::string& msg, int libCurlCode, const std::string userData) {
         MemberParam param;
         int nCode;
         QString outMsg;
         Event memberDetail;
         memberDetail.Init();
         memberDetail.m_eMsgType = e_RQ_SearchOnLineMember;
         param.ParamToFlashMemberInfo(libCurlCode, QString::fromStdString(msg), e_RQ_SearchOnLineMember, nCode, outMsg, &memberDetail);
         QJsonObject obj = VHAliveInteraction::MakeEventObject(&memberDetail);
         if (recvObj) {
            QCoreApplication::postEvent(recvObj, new CustomSocketIOMsg(CustomEvent_CustomRecvSocketIOMsgEvent, obj), Qt::LowEventPriority);
         }
      });
   }
}


void MemberListWdg::on_btnLookUp_clicked(bool checked /*= false*/)
{
   if (ui.widget_search->isHidden()) {
      mbOpenSearch = true;
      ui.ledtLookUp->setText("");
      ui.widget_search->show();
      ui.widget_memberInfo->hide();
   }
   else {
      ui.ledtLookUp->setText("");
      mbOpenSearch = false;
      ui.widget_search->hide();
      ui.widget_memberInfo->show();
   }
   on_btnOnLine_clicked();
}


template<class T>
void AddUserList(const UserList& userList, const bool& bExtend, const bool& bForAll, T* obj, bool (T::*temF)(const VhallAudienceUserInfo &, const bool&, const bool&))
{
   if (userList.size() > 0){
      for (size_t i = 0; i < userList.size(); i++){
         VhallAudienceUserInfo uInfo = userList[i];
         (obj->*temF)(uInfo, bExtend, bForAll);
      }
   }

}



