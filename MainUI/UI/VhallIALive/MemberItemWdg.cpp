#include "MemberItemWdg.h"
#include <QPainter>
#include <QListWidgetItem> 
#include <QTimer>
#include "VhallNetWorkInterface.h"
#include "priConst.h"
#include "cbasedlg.h"
#include "DebugTrace.h"
#include "pathManager.h"
#include "pub.Const.h"
#include "MenuWdg.h"
#include "Msg_CommonToolKit.h"
#include "ICommonData.h"
#include "VH_Macro.h"
#include "CRPluginDef.h"
#include "MainUIIns.h"
#include "ConfigSetting.h"
#include "pathManage.h"
#include <QFileInfo>
#include <QBitmap>

#define IMAGE_SAVE_FLODER  QString("image")
#define FORBIDDEN   QStringLiteral("聊天禁言")
#define CANCELFORBIDDEN QStringLiteral("取消禁言")
#define KICKOUT_ROOM  QStringLiteral("踢出房间")
#define CANCELKICKOUT_ROOM QStringLiteral("取消踢出")
#define SETMAIN QStringLiteral("设为主讲人")

QString HostTextQss = QString::fromStdWString(L"background-color:rgb(252, 86, 89, 15);color:rgb(252, 86, 89);border-radius:5px;");
QString GuestTextQss = QString::fromStdWString(L"background-color:rgb(77, 161, 255,15);color: rgb(77, 161, 255);border-radius:5px;");
QString AssistantTextQss = QString::fromStdWString(L"background-color: rgb(166, 166, 166,15);color:rgb(166, 166, 166);border-radius:5px;");

#define MENUiTEMHOVER   "QToolButton{ \
	border:none;\
	font-size:12px; \
   background-color:#FC5659;\
	color:#FFFFFF;\
}"          

#define MENUiTEMLEAVE "QToolButton{ \
   border:none;\
	font-size:12px; \
   background-color:#FFFFFF;\
	color:#666666;\
}"

#define FORBIDDENHOVER QStringLiteral("QToolButton{ \
	border-image: url(:/states/images/forbidden_hover.png); \
}")

#define FORBIDDENLEAVE QStringLiteral("QToolButton{ \
	border:none;\
	border-image: url(:/states/images/forbidden.png); \
} \
QToolButton:hover{ \
	border-image: url(:/states/images/forbidden.png); \
}")

#define CANCLEFORBIDDENHOVER QStringLiteral("QToolButton{ \
	border-image: url(:/states/images/forbidden_cancel_hover.png) \
}")

#define CANCLEFORBIDDENLEAVE QStringLiteral("QToolButton{ \
	border:none;\
	border-image: url(:/states/images/forbidden_cancel.png); \
} \
QToolButton:hover{ \
	border-image: url(:/states/images/forbidden_cancel.png); \
}")

#define KICKOUTHOVER QStringLiteral("QToolButton{ \
	border-image: url(:/states/images/kick_out_member_hover.png); \
}")

#define KICKOUTLEAVE QStringLiteral("QToolButton{ \
	border:none;\
	border-image: url(:/states/images/kick_out_member.png); \
} \
QToolButton:hover{ \
	border-image: url(:/states/images/kick_out_member_hover.png); \
}")

#define CANCLEKICKOUTHOVER QStringLiteral("QToolButton{ \
	border-image: url(:/states/images/kick_out_member_hover.png); \
}")

#define CANCLEKICKOUTLEAVE QStringLiteral("QToolButton{ \
	border:none;\
	border-image: url(:/states/images/kick_out_member.png); \
} \
QToolButton:hover{ \
	border-image: url(:/states/images/kick_out_member.png); \
}")

#define ROLE_USER  10
#define ROLE_ASSISTANT  20
#define ROLE_GUEST  30
#define ROLE_HOST  40

#define UPPERMIC_DOWN  0
#define UPPERMIC_ON 1

CMemberItem::CMemberItem(QWidget *parent /*= Q_NULLPTR*/)
   :CWidget(parent)
{

   mCheckSpeakTimer = new QTimer(this);
   connect(mCheckSpeakTimer,SIGNAL(timeout()),this,SLOT(slot_CheckHandsUpTimeout()));
}

CMemberItem::~CMemberItem()
{

}

void CMemberItem::slot_CheckHandsUpTimeout() {
   if (mpLabUpper) {
      mCheckSpeakTimer->stop();
      mCheckSpeakTimer->setSingleShot(true);
      mpLabUpper->setPixmap(QPixmap(""));
      mbIsHandsUp = false;
   }
}

void CMemberItem::setUpper(QPushButton *btn, QLabel* lab)
{
   btn->setCheckable(true);
   mpBtnUpper = btn;
   mpLabUpper = lab;
   SetUpperStates(false);
   VH::CComPtr<ICommonData> pCommonData;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
   ClientApiInitResp respInitData;
   pCommonData->GetInitApiResp(respInitData);
   if (respInitData.selectLiveType == eLiveType_VhallActive && respInitData.role_name == USER_GUEST) {
      mpBtnUpper->hide();
   }
}

void CMemberItem::setProhibitSpeaking(QPushButton *btn, QLabel* lab)
{
	//if (btn) {
	//	btn->setCheckable(true);
 //  }

   //btn->setChecked(false);
   //mpBtnProhibitSpeaking = btn;
   mpLabProhibitSpeaking = lab;
   SetProhibitChatStates(false);

}

void CMemberItem::setKickOut(QPushButton *btn, QLabel* lab)
{
   //btn->setCheckable(true);
   //mpBtnKickOut = btn;
   SetKickOutStates(false);

   mpLabKickOut = lab;
}

void CMemberItem::RecvUserApplySpeak(bool apply) {
   if (apply) {
      if (mpLabUpper) {
         mCheckSpeakTimer->start(30000);
         mCheckSpeakTimer->setSingleShot(true);
         mpLabUpper->setPixmap(QPixmap(":/role/images/handsUp_speak.png"));
         mbIsHandsUp = true;
      }
   }
   else {
      if (mpLabUpper) {
         mCheckSpeakTimer->stop();
         mpLabUpper->setPixmap(QPixmap(""));
         mbIsHandsUp = false;
      }
   }
}

void CMemberItem::SetSpeakerState(bool mainSpeaker) {
   if (mbIsSpeaking) {
      if (mainSpeaker) {
         mpLabUpper->setPixmap(QPixmap(":/role/images/host_icon.png"));
      }
      else {
         mpLabUpper->setPixmap(QPixmap(":/role/images/speaker.png"));
      }
   }
   mbIsHandsUp = false;
   if (mCheckSpeakTimer) {
      mCheckSpeakTimer->stop();
   }

}

void CMemberItem::SetUpperStates(const bool& bUpper /*= true*/)
{
   mbIsHandsUp = false;
   if (bUpper) {
      if (mRole == USER_HOST) {
         if (mpLabUpper) {
            mpLabUpper->setPixmap(QPixmap(":/role/images/host_icon.png"));
         }
      }
      else {
         if (mpLabUpper) {
            mpLabUpper->setPixmap(QPixmap(":/role/images/speaker.png"));
         }
      }
      if (mpBtnUpper)
      {
         mpBtnUpper->setChecked(false);
         mpBtnUpper->setText(DOWNMIC);
      }
   }
   else
   {
      if (mRole == USER_HOST) {
         if (mpLabUpper) {
            mpLabUpper->setPixmap(QPixmap(""));
         }
      }
      else {
         if (mpLabUpper) {
            mpLabUpper->setPixmap(QPixmap(""));
         }
      }
      if (mpBtnUpper) {
         mpBtnUpper->setChecked(true);
         mpBtnUpper->setText(UPPERWHEAT);
      }
   }

   mbIsSpeaking = bUpper;
   mpBtnUpper->setEnabled(true);
   if (mpBtnUpper && !mpBtnUpper->isHidden()) {
      enterEvent(nullptr);
   }
   if (mCheckSpeakTimer) {
      mCheckSpeakTimer->stop();
   }
}

bool CMemberItem::UpperStates()
{
   bool bState = true;
   if (mpBtnUpper)
   {
      bState = !mpBtnUpper->isChecked();
   }
   return bState;
}

bool CMemberItem::IsHandsUp() {
   return mbIsHandsUp;
}

bool CMemberItem::IsSpeaking() {
   return mbIsSpeaking;
}

void CMemberItem::SetProhibitChatStates(const bool& bProhibit /*= true*/)
{
   if (bProhibit){  //禁言
	   if (mpLabProhibitSpeaking)
		   mpLabProhibitSpeaking->show();
   }
   else{
      if (mpLabProhibitSpeaking)
		  mpLabProhibitSpeaking->hide();
   }
}

bool CMemberItem::ProhibitChatStates()
{
   if (mpLabProhibitSpeaking) {
      return mpLabProhibitSpeaking->isHidden() ? false : true;
   }
   return false;
}

void CMemberItem::SetKickOutStates(const bool& bKickOut /*= true*/)
{
   if (bKickOut){//踢出
	   if (mpLabKickOut) {
		   mpLabKickOut->show();
	   }
   }
   else{
	   if (mpLabKickOut) {
		   mpLabKickOut->hide();
	   }
   }
}

bool CMemberItem::KickOutStates(){
   if (mpLabKickOut) {
      return mpLabKickOut->isHidden() ? false : true;
   }
   return false;
}

bool CMemberItem::isCurrentUser(const QString& strRole)
{
   //TRACE6("%s  find %s but Current %s \n", __FUNCTION__, strRole.toLatin1().data(), mStrId.toLatin1().data());
   return mStrId.compare(strRole.trimmed()) == 0;
}


QString CMemberItem::GetId()
{
   return mStrId;
}

QString CMemberItem::GetRole()
{
   return mRole;
}

void CMemberItem::btnUpperClicked(const bool& checked)
{
	bool bProhibit = (NULL != mpLabProhibitSpeaking && mpLabProhibitSpeaking->isVisible());
   if (!bProhibit)
   {
      mpBtnUpper->setChecked(checked);
      emit sig_btnUpper_clicked(mStrId, checked);
   }
}

void CMemberItem::btnProhibitSpeakingClicked(const bool& checked)
{

   //if (NULL != mpBtnProhibitSpeaking)
   //{
      //mpBtnProhibitSpeaking->setChecked(checked);
      if (checked)
      {
         sendOperatRQ(e_RQ_UserProhibitSpeakOneUser);
      }
      else
      {
         sendOperatRQ(e_RQ_UserAllowSpeakOneUser);
      }
   //}
}

void CMemberItem::btnbtnKickOutClicked(const bool& checked)
{
   //if (NULL != mpBtnKickOut)
   //{
      //mpBtnKickOut->setChecked(checked);
      if (checked)
      {
         sendOperatRQ(e_RQ_UserKickOutOneUser);
      }
      else
      {
         sendOperatRQ(e_RQ_UserAllowJoinOneUser);
      }
   //}
}

void CMemberItem::sendOperatRQ(const MsgRQType& type)
{
   RQData oData;
   oData.m_eMsgType = type;

   oData.m_oUserInfo.userId = mStrId.toStdWString();
   oData.m_oUserInfo.userName = GetName().toStdWString();

   emit sig_OperationUser(oData);
}

COnlineItemWdg::COnlineItemWdg(QObject* event_reciver,QWidget *parent)
   : CMemberItem(parent)
{
   ui.setupUi(this);
   mEventReciver = event_reciver;
   static int iStatiId = 0;
   iStatiId++;
   m_iStaticId = iStatiId;
   setUpper(ui.btnUpper, ui.labUpper);
   setProhibitSpeaking(nullptr, ui.labChat);
   setKickOut(nullptr, ui.labKickOut);
   ShowSetmainSpeaker(false);
   ui.widget->hide();
   ui.labChat->hide();
   ui.labKickOut->hide();
   ui.btnUpper->hide();
   ui.label_phone->hide();
}

COnlineItemWdg::COnlineItemWdg(QObject* event_reciver, COnlineItemWdg* pitem, QWidget *parent /*= Q_NULLPTR*/)
   : CMemberItem(parent)
{
   ui.setupUi(this);
   mEventReciver = event_reciver;
   static int iStatiId = 990;
   iStatiId++;
   m_iStaticId = iStatiId;
   setUpper(ui.btnUpper, ui.labUpper);
   setProhibitSpeaking(nullptr, ui.labChat);
   setKickOut(nullptr, ui.labKickOut);
   ui.labName->setText(CPathManager::GetString(pitem->GetName(), 14));
   ui.labName->setToolTip(pitem->GetName());
   ui.labName->setStyleSheet(TOOLTIPQSS);
   mStrId = pitem->mStrId;
   mRole = pitem->mRole;
   if (mRole.compare(USER_HOST) == 0) {
      ui.labIdentityTile->setText(QString::fromStdWString(L"主持人"));
      ui.labIdentityTile->setStyleSheet(HostTextQss);
   }
   else if (mRole.compare(USER_GUEST) == 0) {
      ui.labIdentityTile->setText(QString::fromStdWString(L"嘉宾"));
      ui.labIdentityTile->setStyleSheet(GuestTextQss);
   }
   else if (mRole.compare(USER_ASSISTANT) == 0) {
      ui.labIdentityTile->setText(QString::fromStdWString(L"助理"));
      ui.labIdentityTile->setStyleSheet(AssistantTextQss);
   }
   else if (mRole.compare(USER_USER) == 0) {
      ui.labIdentityTile->setText(QString::fromStdWString(L""));
   }
   ui.label_phone->hide();
   ShowSetmainSpeaker(false);
   ui.widget->hide();
   SetUpperStates(pitem->UpperStates());
   SetProhibitChatStates(pitem->ProhibitChatStates());
   SetKickOutStates(pitem->KickOutStates());
   ui.labChat->hide();
   ui.labKickOut->hide();
   ui.btnUpper->hide();
   mpAlive = pitem->mpAlive;
   VH::CComPtr<ICommonData> pCommonData;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
   ClientApiInitResp respInitData;
   pCommonData->GetInitApiResp(respInitData);
   if (USER_HOST == respInitData.role_name && USER_HOST != mRole && respInitData.selectLiveType == eLiveType_VhallActive) {
	   ui.btnUpper->show();
   }
   if (mRole == USER_ASSISTANT) {
      ui.btnUpper->hide();
   }
   CalcWeight();
}

COnlineItemWdg::~COnlineItemWdg()
{
   //add_log("ixk", "%s  %d", __FUNCTION__, m_iStaticId);
   if (NULL != mpUpperTime)
   {
      if (mpUpperTime->isActive())
      {
         mpUpperTime->stop();
      }
      mpUpperTime->deleteLater();
      mpUpperTime = NULL;
   }

   if (nullptr != mPMoreOpMenu)
   {
	   delete mPMoreOpMenu;
	   mPMoreOpMenu = NULL;
   }
}

void COnlineItemWdg::SetAlive(CAliveDlg* pAlive) {
   mpAlive = pAlive;
}

void COnlineItemWdg::StopCheckUpperTimeout() {
   if (mpUpperTime) {
      mpUpperTime->stop();
   }
}

QPixmap PixmapToRound(const QPixmap &src, int radius)
{
   if (src.isNull()) {
      return QPixmap();
   }
   QSize size(2 * radius, 2 * radius);
   QBitmap mask(size);
   QPainter painter(&mask);
   painter.setRenderHint(QPainter::Antialiasing);
   painter.setRenderHint(QPainter::SmoothPixmapTransform);
   painter.fillRect(0, 0, size.width(), size.height(), Qt::white);
   painter.setBrush(QColor(0, 0, 0));
   painter.drawRoundedRect(0, 0, size.width(), size.height(), 99, 99);
   QPixmap image = src.scaled(size);
   image.setMask(mask);
   return image;
}

void COnlineItemWdg::LoadImage(QString file) {
   QImage privew_image;
   bool bRet = privew_image.load(file);
   if (bRet) {
      QPixmap image = QPixmap::fromImage(privew_image);
      image = image.scaled(24, 24, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
      image = PixmapToRound(image,12);
      ui.label_head->setPixmap(image);
   }
}

void COnlineItemWdg::DownLoadHeadImage(QString image_path) {
   if (image_path.isEmpty() || image_path.length() < 10) {
      return;
   }

   QString fileUrl = image_path;
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
   if (fileInfo.exists()){
      LoadImage(filePath);
   }
   else {
      HTTP_GET_REQUEST request(fileUrl.toStdString());
      request.SetEnableDownLoadFile(true, filePath.toStdString());
      request.mbIsNeedSyncWork = true;
      QString id = mStrId;
      GetHttpManagerInstance()->HttpGetRequest(request, [&, obj, fileUrl, filePath, id](const std::string& msg, int code, const std::string userData) {
         TRACE6("%s code:%d msg:%s\n", __FUNCTION__, code, msg.c_str());
         if (code == 0) {
            QApplication::postEvent(obj, new CustomDownLoadEvent(CustomEvent_DownLoadFile, code, fileUrl, filePath, id, 0));
         }
      });
   }
}

void COnlineItemWdg::customEvent(QEvent* event) {
   if (event->type() == CustomEvent_DownLoadFile) {
      CustomDownLoadEvent* custom_event = dynamic_cast<CustomDownLoadEvent*>(event);
      if (custom_event) {
         LoadImage(custom_event->mSavePath);
      }
   }
}

bool COnlineItemWdg::IsUserHandsUp() {
   if (mRole == USER_USER) {
      return IsHandsUp();
   }
   return false;
}

void COnlineItemWdg::SetUserInfo(const VhallAudienceUserInfo& info){
   QString str = CPathManager::GetString(QString::fromStdWString(info.userName), 14);
   if (info.userName.length() > 14) {
      str += "..";
   }
   ui.labName->setText(str);
   ui.labName->setToolTip(QString::fromStdWString(info.userName));
   ui.labName->setStyleSheet(TOOLTIPQSS);
   mStrId = QString::fromStdWString(info.userId);
   mRole = QString::fromStdWString(info.role);
   mDevType = info.mDevType;
   if (mRole == USER_HOST) {
      ui.labIdentityTile->setText(QString::fromStdWString(L"主持人"));
      ui.labIdentityTile->setStyleSheet(HostTextQss);
   }
   else if (mRole == USER_GUEST) {
      ui.labIdentityTile->setText(QString::fromStdWString(L"嘉宾"));
      ui.labIdentityTile->setStyleSheet(GuestTextQss);
   }
   else if (mRole == USER_ASSISTANT) {
      ui.labIdentityTile->setText(QString::fromStdWString(L"助理"));
      ui.labIdentityTile->setStyleSheet(AssistantTextQss);
   }
   else if (mRole == USER_USER) {
      ui.labIdentityTile->setText(QString::fromStdWString(L""));
   }
   DownLoadHeadImage(QString::fromStdWString(info.headImage));
   VH::CComPtr<ICommonData> pCommonData;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
   ClientApiInitResp respInitData;
   pCommonData->GetInitApiResp(respInitData);
   if (respInitData.role_name == USER_HOST) {
      UpdataHostMoreBtn();
      if (respInitData.selectLiveType == eLiveType_VhallActive) {
         if (USER_HOST != mRole) {
            ui.btnUpper->show();
            if (mRole == USER_ASSISTANT) {
               ui.btnUpper->hide();
            }
            else if (info.mDevType != 1) {
               info.gagType == VhallShowType_Allow ? ui.btnUpper->hide() : ui.btnUpper->show();
            }
         }
         else if (respInitData.join_uid == mStrId && !info.mIsRtcPublisher) {
            ui.btnUpper->show();
            if (mRole == USER_ASSISTANT) {
               ui.btnUpper->hide();
            }
         }
      }
   }
   else {
      if (mRole == USER_USER) {
         ui.btnMoreOp->show();
      }
      else {
         ui.btnMoreOp->hide();
      }
   }

   if (info.mDevType == 1) {
      ui.btnUpper->hide();
      ui.label_phone->show();
   }
   else if(info.mDevType == 3) {
      ui.label_phone->show();
   }

   //if(info.kickType)
   CalcWeight();
}

void COnlineItemWdg::ShowMoreBtn(bool show) {
   if (show) {
      ui.btnMoreOp->show();
   }
   else {
      ui.btnMoreOp->hide();
   }
}

void COnlineItemWdg::ShowToSpeakBtn(bool show) {
   VH::CComPtr<ICommonData> pCommonData;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
   ClientApiInitResp respInitData;
   pCommonData->GetInitApiResp(respInitData);
   if (respInitData.selectLiveType == eLiveType_VhallActive) {
      if (respInitData.role_name == USER_GUEST) {
         ui.btnUpper->hide();
      }
      else if (mDevType != 1) {
         show == true ? ui.btnUpper->show() : ui.btnUpper->hide();
      }
   }
}

void COnlineItemWdg::SetMainView(QString id, bool main) {
   if (main) {
      ui.btnUpper->hide();
      ui.btnMoreOp->hide();
   }
   else {
      ui.btnUpper->show();
      ui.btnMoreOp->show();
      if (mpAlive && mpAlive->IsExistRenderWnd(id)) {
         ui.btnUpper->setText(DOWNMIC);
         mbSelfUpper = false;
         SetSelfUpperWheat(false);
      }
   }
}

void COnlineItemWdg::enterEvent(QEvent *event) {
   CMemberItem::enterEvent(event);
}

void COnlineItemWdg::SetEnableWheat(const bool& bEnable)
{
   if (USER_HOST == mRole)
      mbHostEnableWheat = bEnable;
}

void COnlineItemWdg::ShowSetmainSpeaker(bool show) {
   mbEnableSetMainSpeaker = show;
   if (!ui.widget->isHidden()) {
      enterEvent(nullptr);
   }
   if (!ui.btnUpper->isHidden() && ui.btnUpper->text() == DOWNMIC) {
      enterEvent(nullptr);
   }
}

void COnlineItemWdg::FlushBtnState() {
   enterEvent(nullptr);
}

void COnlineItemWdg::SetSelfUpperWheat(const bool& bUpper /*= true*/)
{
   mbSelfUpper = bUpper;
   if (mbSelfUpper)
      ui.btnUpper->setText(UPPERWHEAT);//self
   else
      ui.btnUpper->setText(DOWNMIC);//self

   //VH::CComPtr<ICommonData> pCommonData;
   //DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
   //ClientApiInitResp respInitData;
   //pCommonData->GetInitApiResp(respInitData);
   //if (((respInitData.player == H5_LIVE_CREATE_TYPE && respInitData.user_id == mStrId) ||
   //   (respInitData.player == FLASH_LIVE_CREATE_TYPE && respInitData.join_uid == mStrId)) && respInitData.role_name == USER_HOST){
   //   if (mbSelfUpper) { //当前已经下麦，
   //      ui.btnUpper->show();
   //   }
   //   else {
   //      ui.btnUpper->hide();
   //   }
   //}
}

void COnlineItemWdg::leaveEvent(QEvent *event)
{
//   if (globalToolManager->GetDataManager()->GetJoinUid() != mStrId){
//      if (USER_USER == mRole)
//      {
//         //ui.btnProhibitSpeaking->hide();
//         //ui.btnKickOut->hide();
////         ui.line->hide();
//         ui.widget->hide();
//         //ui.btnUpper->hide();
////         ui.lineUpper->hide();
//      }
//      else if (vh_vss::JOIN_ROLE_HOST == globalToolManager->GetDataManager()->GetRoomInfo().role_name && USER_GUEST == mRole)
//      {
//         //ui.btnUpper->hide();
////         ui.lineUpper->hide();
//         //ui.btnProhibitSpeaking->hide();
//         //ui.btnKickOut->hide();
////         ui.line->hide();
//         ui.widget->hide();
//      }
//      else if (vh_vss::JOIN_ROLE_Guest == globalToolManager->GetDataManager()->GetRoomInfo().role_name && USER_USER == mRole)
//      {
//         //ui.btnProhibitSpeaking->hide();
//         //ui.btnKickOut->hide();
////         ui.line->hide();
//         ui.widget->hide();
//      }
//   }
//   else if (USER_HOST == mRole)//主持人对自己
//   {
//      //显示上麦 、下麦
//      //ui.btnSelfUpper->hide();
//      ui.widget->hide();
//   }
   CMemberItem::leaveEvent(event);
}

void COnlineItemWdg::mousePressEvent(QMouseEvent *event)
{

}
void COnlineItemWdg::mouseMoveEvent(QMouseEvent *event)
{

}
QString COnlineItemWdg::GetName()
{
   return ui.labName->text().trimmed();
}

void COnlineItemWdg::SetUpperStates(const bool& bUpper /*= true*/)
{

   VH::CComPtr<ICommonData> pCommonData;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
   ClientApiInitResp respInitData;
   pCommonData->GetInitApiResp(respInitData);

   CMemberItem::SetUpperStates(bUpper);
   if (mpAlive && respInitData.selectLiveType == eLiveType_VhallActive) {
      if (mpAlive->GetMainSpeakerID() == mStrId) {
         CMemberItem::SetSpeakerState(true);
      }
      else {
         CMemberItem::SetSpeakerState(false);
      }
   }


   if (respInitData.role_name == USER_HOST) {
      if ((respInitData.player == H5_LIVE_CREATE_TYPE && respInitData.user_id == mStrId) ||
         (respInitData.player == FLASH_LIVE_CREATE_TYPE && respInitData.join_uid == mStrId)) {
         if (respInitData.selectLiveType == eLiveType_VhallActive) {
            if (mpAlive->GetIsLiving()) {
               if (bUpper) {
                  if (mpAlive->GetMainSpeakerID() == mStrId || mpAlive->GetMainViewRole() == USER_USER) {
                     ui.btnMoreOp->hide();
                  }
                  else {
                     ui.btnMoreOp->show();
                     ui.btnUpper->setText(DOWNMIC);
                     ui.btnUpper->show();
                     SetSelfUpperWheat(false);
                  }
               }
               else {
                  ui.btnMoreOp->hide();
                  ui.btnUpper->setText(UPPERWHEAT);
                  ui.btnUpper->show();
                  SetSelfUpperWheat(true);
               }
            }
            else {
               ui.btnMoreOp->hide();
            }
         }
         else {
            ui.btnMoreOp->hide();
         }
      }
      else {
         ui.btnMoreOp->show();
      }
   }
   if (mRole == USER_ASSISTANT) {
      ui.btnUpper->hide();
   }
   CalcWeight();
}

int COnlineItemWdg::GetWeight()
{
   return mIWeight;
}

void COnlineItemWdg::UpdataHostMoreBtn() {
   VH::CComPtr<ICommonData> pCommonData;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
   ClientApiInitResp respInitData;
   pCommonData->GetInitApiResp(respInitData);
   if ((respInitData.player == H5_LIVE_CREATE_TYPE && respInitData.user_id == mStrId) ||
      (respInitData.player == FLASH_LIVE_CREATE_TYPE && respInitData.join_uid == mStrId)) {
      if (respInitData.selectLiveType == eLiveType_VhallActive) {
         if (mpAlive->GetIsLiving()) {
            if (mpAlive->IsExistRenderWnd(mStrId)) {
               if (mpAlive->GetMainSpeakerID() == mStrId) {
                  ui.btnMoreOp->hide();
               }
               else {
                  ui.btnMoreOp->show();
                  ui.btnUpper->setText(DOWNMIC);
                  ui.btnUpper->show();
                  SetSelfUpperWheat(false);
               }
            }
            else {
               ui.btnMoreOp->hide();
               ui.btnUpper->setText(UPPERWHEAT);
               ui.btnUpper->show();
               SetSelfUpperWheat(true);
            }
         }
         else {
            ui.btnMoreOp->hide();
         }
      }
      else {
         ui.btnMoreOp->hide();
      }
   }
   else {
      ui.btnMoreOp->show();
   }
}

bool COnlineItemWdg::operator==(const COnlineItemWdg &obj)
{
   return this->mIWeight == obj.mIWeight;
}

bool COnlineItemWdg::operator>=(const COnlineItemWdg& obj)
{
   return this->mIWeight >= obj.mIWeight;
}

bool COnlineItemWdg::operator>(const COnlineItemWdg& obj)
{
   return this->mIWeight > obj.mIWeight;
}

bool COnlineItemWdg::operator<=(const COnlineItemWdg& obj)
{
   return this->mIWeight <= obj.mIWeight;
}

bool COnlineItemWdg::operator<(const COnlineItemWdg& obj)
{
   return this->mIWeight < obj.mIWeight;
}

int COnlineItemWdg::CalcWeight()
{
   mIWeight = 100;
   if (USER_HOST == mRole)
   {
      mIWeight -= ROLE_HOST;
   }
   else if (USER_GUEST == mRole)
   {
      mIWeight -= ROLE_GUEST;
   }
   else if (USER_ASSISTANT == mRole)
   {
      mIWeight -= ROLE_ASSISTANT;
   }
   else
   {
      mIWeight -= ROLE_USER;
   }

   if (ui.btnUpper->text().trimmed().compare(DOWNMIC) == 0)
   {
      mIWeight -= UPPERMIC_ON;
   }
   else
   {
      mIWeight -= UPPERMIC_DOWN;
   }

   if (NULL != mpItem)
   {
      mpItem->setText(QString::number(mIWeight));
   }
   return mIWeight;
}

void COnlineItemWdg::on_btnSelfUpper_clicked(bool checked/* = false*/)
{
   emit sig_selfUpper(mbSelfUpper);
}

void COnlineItemWdg::on_btnUpper_clicked(bool checked /*= false*/)
{
	 if (mRole.compare(USER_HOST) == 0) { //主持人对自己
		 emit sig_selfUpper(mbSelfUpper);
	 }
	 else
	 {
       QString speak_state = ui.btnUpper->text();
       if (speak_state == UPPERWHEAT) {
          emit sig_btnUpper_clicked(mStrId, true);
       }
       else {
          emit sig_btnUpper_clicked(mStrId, false);
       }
		 ui.btnUpper->setEnabled(false);
		 if (NULL == mpUpperTime){
			 mpUpperTime = new QTimer(this);
			 connect(mpUpperTime, &QTimer::timeout, this, &COnlineItemWdg::slot_upperTimeOut);
		 }
		 mpUpperTime->start(10000);
	}
   
}

//void COnlineItemWdg::on_btnProhibitSpeaking_clicked(bool checked /*= false*/)
//{
//
//   btnProhibitSpeakingClicked(!checked);
//}

//void COnlineItemWdg::on_btnKickOut_clicked(bool checked /*= false*/)
//{
//   btnbtnKickOutClicked(!checked);//踢出 false
//}

void COnlineItemWdg::on_pushButton_setMain_clicked(bool checked/* = false*/) {
   emit sig_setMainSpeaker(mStrId, mRole);
}

void COnlineItemWdg::slot_upperTimeOut()
{
   if (NULL != mpUpperTime && mpUpperTime->isActive()) {
      mpUpperTime->stop();
   }
   SetUpperStates(false);
}
void COnlineItemWdg::createAction()
{
	//ui.labMoreOp->hide();
   VH::CComPtr<ICommonData> pCommonData;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
   ClientApiInitResp respInitData;
   pCommonData->GetInitApiResp(respInitData);
	if (nullptr == mPMoreOpMenu /*&& respInitData.user_id != mStrId*/)
	{
		mPMoreOpMenu = new MenuWdg(/*this*/);
		mPMoreOpMenu->addItem(FORBIDDEN, MENUiTEMHOVER, MENUiTEMLEAVE, eOptype_Forbidden, FORBIDDENHOVER, FORBIDDENLEAVE);
		mPMoreOpMenu->addItem(CANCELFORBIDDEN, MENUiTEMHOVER, MENUiTEMLEAVE, eOptype_CancleForbidden, CANCLEFORBIDDENHOVER, CANCLEFORBIDDENLEAVE);
		mPMoreOpMenu->addItem(KICKOUT_ROOM, MENUiTEMHOVER, MENUiTEMLEAVE, eOptype_KickOut, KICKOUTHOVER, KICKOUTLEAVE);
		mPMoreOpMenu->addItem(CANCELKICKOUT_ROOM, MENUiTEMHOVER, MENUiTEMLEAVE, eOptype_CancleKickOut, CANCLEKICKOUTHOVER, CANCLEKICKOUTLEAVE);
		mPMoreOpMenu->addItem(SETMAIN, MENUiTEMHOVER, MENUiTEMLEAVE, eOptype_setMain, CANCLEKICKOUTHOVER, CANCLEKICKOUTLEAVE);
		connect(mPMoreOpMenu, &MenuWdg::sigClicked, this, &COnlineItemWdg::slotCliecked);
	}
}

void COnlineItemWdg::slotCliecked(const int& iOpType)
{
	switch (iOpType)
	{
	case eOptype_Forbidden://禁言
		btnProhibitSpeakingClicked(true);
		break;
	case	eOptype_CancleForbidden:
		btnProhibitSpeakingClicked(false);
		break;
	case eOptype_KickOut:
		btnbtnKickOutClicked(true);//踢出 false
		break;
	case eOptype_CancleKickOut:
		btnbtnKickOutClicked(false);//踢出 false
		break;
	case eOptype_setMain:
		emit sig_setMainSpeaker(mStrId, mRole);
		break;
	default:
		break;
	}
}

void COnlineItemWdg::on_btnMoreOp_clicked(bool checked /*= false*/)
{
   VH::CComPtr<ICommonData> pCommonData;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
   ClientApiInitResp respInitData;
   pCommonData->GetInitApiResp(respInitData);
	createAction();
   if (nullptr == mPMoreOpMenu) {
      return;
   }
	if (ui.labKickOut->isVisible()){  //踢出
		mPMoreOpMenu->SetItemShow(eOptype_CancleKickOut, true);
		mPMoreOpMenu->SetItemShow(eOptype_Forbidden, false);
		mPMoreOpMenu->SetItemShow(eOptype_CancleForbidden, false);
		mPMoreOpMenu->SetItemShow(eOptype_KickOut, false);
      mPMoreOpMenu->SetItemShow(eOptype_setMain, false);
		mPMoreOpMenu->SetShowItemCount(1);
	}
	else//在房间
	{
		mPMoreOpMenu->SetItemShow(eOptype_CancleKickOut, false);
		mPMoreOpMenu->SetItemShow(eOptype_KickOut, true);
		if (ui.labChat->isVisible()){  //被禁言中
			mPMoreOpMenu->SetItemShow(eOptype_Forbidden, false);
			mPMoreOpMenu->SetItemShow(eOptype_CancleForbidden, true);
		}
		else{
			mPMoreOpMenu->SetItemShow(eOptype_Forbidden, true);
			mPMoreOpMenu->SetItemShow(eOptype_CancleForbidden, false);
		}
      mPMoreOpMenu->SetItemShow(eOptype_setMain, false);
      if (respInitData.selectLiveType == eLiveType_Live) {
         mPMoreOpMenu->SetShowItemCount(2);
      }
      else {
         if (mpAlive) {
            if (respInitData.role_name == USER_HOST) {
               if (mpAlive->IsExistRenderWnd(mStrId) && (mRole == USER_HOST || mRole == USER_GUEST)) {
                  if (((respInitData.player == H5_LIVE_CREATE_TYPE && respInitData.user_id == mStrId) ||
                     (respInitData.player == FLASH_LIVE_CREATE_TYPE && respInitData.join_uid == mStrId)) && mpAlive->GetMainSpeakerID() != mStrId) {
                     mPMoreOpMenu->SetItemShow(eOptype_Forbidden, false);
                     mPMoreOpMenu->SetItemShow(eOptype_CancleForbidden, false);
                     mPMoreOpMenu->SetItemShow(eOptype_CancleKickOut, false);
                     mPMoreOpMenu->SetItemShow(eOptype_KickOut, false);
                     mPMoreOpMenu->SetItemShow(eOptype_setMain, true);
                     mPMoreOpMenu->SetShowItemCount(1);
                  }
                  else if (mpAlive->GetMainSpeakerID() == mStrId) {
                     //已经上麦的用户，并且是主讲人的
                     mPMoreOpMenu->SetItemShow(eOptype_setMain, false);
                     mPMoreOpMenu->SetShowItemCount(2);
                  }
                  else {
                     mPMoreOpMenu->SetItemShow(eOptype_setMain, true);
                     mPMoreOpMenu->SetShowItemCount(3);
                  }
               }
               else {
                  mPMoreOpMenu->SetShowItemCount(2);
               }
            }
            else {
               mPMoreOpMenu->SetShowItemCount(2);
            }
         }
      }
	}
	QPoint pos = mapToGlobal(/*temppos +*/ ui.btnMoreOp->pos());
	int iX = pos.x();
	int iY = pos.y();
	mPMoreOpMenu->Show(iX - mPMoreOpMenu->width() + 10, iY - 8);
   //mPMoreOpMenu->show();
}

