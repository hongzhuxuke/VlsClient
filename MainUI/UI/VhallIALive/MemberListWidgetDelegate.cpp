#include "MemberListWidgetDelegate.h"
#include <QMouseEvent>
#include <QPainter>
#include "Msg_CommonToolKit.h"
#include "ICommonData.h"
#include "IMainUILogic.h"
#include "VH_Macro.h"
#include "CRPluginDef.h"
#include "MainUIIns.h"
#include <QLabel>
#include "pub.Const.h"
#include "ICommonData.h"
#include "VH_Macro.h"
#include "CRPluginDef.h"
#include "DebugTrace.h"
#include "MenuWdg.h"
#include <QPoint>
#include <QWidget>
#include "ConfigSetting.h"
#include "cbasedlg.h"
#include "VhallIALive.h"

#define HeadImage_Width 24
#define HeadImage_Height 24
#define HeadImage_Radius 12

#define HostRoleQColor  QColor(252, 86, 89, 15)

#define IMAGE_SAVE_FLODER  QString("image")
#define FORBIDDEN   QStringLiteral("聊天禁言")
#define CANCELFORBIDDEN QStringLiteral("取消禁言")
#define KICKOUT_ROOM  QStringLiteral("踢出房间")
#define CANCELKICKOUT_ROOM QStringLiteral("取消踢出")
#define SETMAIN QStringLiteral("设为主讲人")


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

MemberListWidgetDelegate::MemberListWidgetDelegate(QObject *parent)
   :QStyledItemDelegate(parent){
   if (nullptr == mPMoreOpMenu){
      mPMoreOpMenu = new MenuWdg(/*this*/);
      mPMoreOpMenu->addItem(FORBIDDEN, MENUiTEMHOVER, MENUiTEMLEAVE, eOptype_Forbidden, FORBIDDENHOVER, FORBIDDENLEAVE);
      mPMoreOpMenu->addItem(CANCELFORBIDDEN, MENUiTEMHOVER, MENUiTEMLEAVE, eOptype_CancleForbidden, CANCLEFORBIDDENHOVER, CANCLEFORBIDDENLEAVE);
      mPMoreOpMenu->addItem(KICKOUT_ROOM, MENUiTEMHOVER, MENUiTEMLEAVE, eOptype_KickOut, KICKOUTHOVER, KICKOUTLEAVE);
      mPMoreOpMenu->addItem(CANCELKICKOUT_ROOM, MENUiTEMHOVER, MENUiTEMLEAVE, eOptype_CancleKickOut, CANCLEKICKOUTHOVER, CANCLEKICKOUTLEAVE);
      mPMoreOpMenu->addItem(SETMAIN, MENUiTEMHOVER, MENUiTEMLEAVE, eOptype_setMain, CANCLEKICKOUTHOVER, CANCLEKICKOUTLEAVE);
      connect(mPMoreOpMenu, &MenuWdg::sigClicked, this, &MemberListWidgetDelegate::slotCliecked);
   }

}

MemberListWidgetDelegate::~MemberListWidgetDelegate(){
}

void MemberListWidgetDelegate::Slot_HandsUpTimeout() {
   VH::CComPtr<IMainUILogic> pMainUILogic;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IMainUI, IID_IMainUILogic, pMainUILogic, return, ASSERT(FALSE));
   VhallIALive *rtclive = (VhallIALive*)pMainUILogic->GetCAliveDlg();
   if (rtclive) {
      rtclive->HandsUpTimeout(mStrUserId);
   }
}

void MemberListWidgetDelegate::slotCliecked(const int& iOpType) {
   switch (iOpType){
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
      SetMainView();
      break;
   default:
      break;
   }
}

void MemberListWidgetDelegate::btnbtnKickOutClicked(const bool& checked) {
   if (checked){
      sendOperatRQ(e_RQ_UserKickOutOneUser);
   }
   else{
      sendOperatRQ(e_RQ_UserAllowJoinOneUser);
   }
}

void MemberListWidgetDelegate::SetMainView() {
   TRACE6("%s user:%s name:%s \n", __FUNCTION__,mStrUserId.toStdString().c_str(), mNickName.toStdString().c_str());
   VH::CComPtr<IMainUILogic> pMainUILogic;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IMainUI, IID_IMainUILogic, pMainUILogic, return, ASSERT(FALSE));
   VhallIALive *rtclive = (VhallIALive*)pMainUILogic->GetCAliveDlg();
   if (rtclive) {
      rtclive->slot_setToSpeaker(mStrUserId, mRoleName);
   }
}

void MemberListWidgetDelegate::sendOperatRQ(const MsgRQType& type){
   TRACE6("%s type %d user:%s name:%s \n",__FUNCTION__,type, mStrUserId.toStdString().c_str(), mNickName.toStdString().c_str());
   RQData oData;
   oData.m_eMsgType = type;
   oData.m_oUserInfo.userId = mStrUserId.toStdWString();
   oData.m_oUserInfo.userName = mNickName.toStdWString();

   VH::CComPtr<IMainUILogic> pMainUILogic = NULL;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IMainUI, IID_IMainUILogic, pMainUILogic);
   if (pMainUILogic) {
      pMainUILogic->SlotControlUser(&oData);
   }  
}

void MemberListWidgetDelegate::btnProhibitSpeakingClicked(const bool& checked) {
   if (checked){
      sendOperatRQ(e_RQ_UserProhibitSpeakOneUser);
      VH::CComPtr<ICommonData> pCommonData;
      DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
      ClientApiInitResp respInitData;
      pCommonData->GetInitApiResp(respInitData);
      if (respInitData.selectLiveType == eLiveType_VhallActive) {
         VH::CComPtr<IMainUILogic> pMainUILogic;
         DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IMainUI, IID_IMainUILogic, pMainUILogic, return, ASSERT(FALSE));
         VhallIALive *rtclive = (VhallIALive*)pMainUILogic->GetCAliveDlg();
         if (rtclive) {
            rtclive->ProhibiteLedPutDownWheat(mStrUserId);
         }
      }
   }
   else{
      sendOperatRQ(e_RQ_UserAllowSpeakOneUser);
   }
}

/**
 * 压缩图片为指定宽高
 */
QPixmap scaledPixmap(const QPixmap &src, int width, int height) {
   return src.scaled(width, (height == 0 ? width : height), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
}

/**
 * 圆角图片
 * src 原图片
 * radius 图片半径
 */
QPixmap generatePixmap(const QPixmap &src,int width, int height, const int &radius) {
   // 无效图片不处理
   if (src.isNull()) {
      return src;
   }

   // 压缩图片
   QPixmap pixmap = scaledPixmap(src, width, height);

   QPixmap dest(2 * radius, 2 * radius);
   dest.fill(Qt::transparent);
   QPainter painter(&dest);
   // 抗锯齿 + 平滑边缘处理
   painter.setRenderHints(QPainter::Antialiasing, true);
   painter.setRenderHints(QPainter::SmoothPixmapTransform, true);
   // 裁剪为圆角
   QPainterPath path;
   path.addEllipse(0, 0, 2 * radius, 2 * radius);
   painter.setClipPath(path);
   painter.drawPixmap(0, 0, 2 * radius, 2 * radius, pixmap);
   return dest;
}

void MemberListWidgetDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const{
   QRect rect = option.rect; // 目标矩形
   painter->setRenderHint(QPainter::Antialiasing);  // 反锯齿;
   // 取得该项对应的数据
   VhallAudienceUserInfo user_info = index.data(Qt::DisplayRole).value<VhallAudienceUserInfo>();
   int showMoreMenu = index.data(SHOW_MORE_MENU).isNull() ? 0 : index.data(SHOW_MORE_MENU).toInt();
   int showChatForbid = index.data(SHOW_CHAT_FORBID).isNull() ? 0 : index.data(SHOW_CHAT_FORBID).toInt();
   int showKickOut = index.data(SHOW_KICK_OUT).isNull() ? 0 : index.data(SHOW_KICK_OUT).toInt();
   int showPhone = user_info.mDevType == 1 ? 1 : 0;
   int mainView = index.data(SHOW_MAIN_VIEW).isNull() ? 0 : index.data(SHOW_MAIN_VIEW).toInt();

   QString speak_status = index.data(SHOW_SPEAK_STATE).isNull() ? QString("") : index.data(SHOW_SPEAK_STATE).toString();
   QString headImage = index.data(SHOW_HEAD_IMAGE).isNull() ? QString(":/role/images/head.png") : index.data(SHOW_HEAD_IMAGE).toString();

   QColor curColor = painter->pen().color();
   // 状态显示: 若该项被选中
   if (option.state & QStyle::State_Selected) {
      painter->setPen(QColor(0x2C, 0x2C, 0x2C));
      painter->setBrush(QColor(0x2C, 0x2C, 0x2C));
      painter->drawRoundedRect(rect, 0, 0);
      painter->setBrush(Qt::NoBrush);
   }
   else {
      painter->setPen(QColor(0x43, 0x43, 0x43));
      painter->setBrush(QColor(0x43, 0x43, 0x43));
      painter->drawRoundedRect(rect, 0, 0);
      painter->setBrush(Qt::NoBrush);
   }
   painter->setPen(curColor);

   int leftStartPaintWidth = 0;
   // 头像图片
   {
      QRect dst = rect;
      dst.setX(rect.left() + 15);
      leftStartPaintWidth += 15;
      dst.setWidth(HeadImage_Width);
      QRect area(0, 0, HeadImage_Width, HeadImage_Width);
      area.moveCenter(dst.center());
      QPixmap pixmapPic(headImage);
      pixmapPic = generatePixmap(pixmapPic,24,24, 12);
      painter->drawPixmap(area, pixmapPic);
      leftStartPaintWidth += 24;
   }

   // 昵称
   {
      QLabel tmp;
      tmp.hide();
      QFont wordfont;
      wordfont.setFamily("Arial");
      wordfont.setPointSize(9);
      QFontMetrics fm(wordfont);
      QString nickName = QString::fromStdWString(user_info.userName);
      if (nickName.length() > 10) {
         nickName = nickName.mid(0,10);
         nickName += QString("..");
      }
      tmp.setText(nickName);
      QRect rec = fm.boundingRect(tmp.text());
      int name_len = rec.width();//这个就获得了字符串所占的像素宽度
      painter->setPen(QColor(0x99, 0x99, 0x99));
      QRect dst = rect;
      leftStartPaintWidth += 9;
      dst.setLeft(rect.left() + leftStartPaintWidth);
      painter->drawText(dst, Qt::AlignLeft | Qt::AlignVCenter, nickName);
      leftStartPaintWidth += name_len;
   }
   //角色提示
   {
      leftStartPaintWidth += 15;
      QRect dst = rect;
      QRect area(0, 0, 44, 15);
      area.setX(rect.left() + leftStartPaintWidth);
      area.setY(rect.y() + (option.rect.height() - 15) / 2);
      area.setHeight(15);
      if (QString::fromStdWString(user_info.role) == USER_HOST) {
         area.setWidth(44);
         leftStartPaintWidth += 44;
         QPixmap pixmapPic(":/role/images/member_host_icon.png");
         pixmapPic = scaledPixmap(pixmapPic, 44, 15);//generatePixmap(pixmapPic, 44, 15, 0);
         painter->drawPixmap(area, pixmapPic);
      }
      else if (QString::fromStdWString(user_info.role) == USER_GUEST) {
         area.setWidth(34);
         leftStartPaintWidth += 34;
         QPixmap pixmapPic(":/role/images/member_guest_icon.png");
         pixmapPic = scaledPixmap(pixmapPic, 34, 15);
         painter->drawPixmap(area, pixmapPic);
      }
      else if (QString::fromStdWString(user_info.role) == USER_ASSISTANT) {
         area.setWidth(34);
         leftStartPaintWidth += 34;
         QPixmap pixmapPic(":/role/images/member_assistant_icon.png");
         pixmapPic = scaledPixmap(pixmapPic, 34, 15);
         painter->drawPixmap(area, pixmapPic);
      }
   }

   int rightBtnMoveLen = 0;
   // 点击menu
   if(showMoreMenu == 1){
      rightBtnMoveLen += 30;
      QRect area(rect.width() - rightBtnMoveLen, rect.y() + (rect.height() - 15) / 2, 15, 15);
      QString icon = QString(":/states/images/more_operate.png");
      QPixmap pixmapPic(icon);
      painter->drawPixmap(area, pixmapPic);
      rightBtnMoveLen += 5;
   }
   rightBtnMoveLen += 15;
   // 显示手机图标
   if(showPhone == 1 || user_info.mDevType == 3){
      QRect area(rect.width() - rightBtnMoveLen, rect.y() + (rect.height() - 15) / 2, 11, 15);
      QString icon = QString(":/states/images/phone_login.png");
      QPixmap pixmapPic(icon);
      painter->drawPixmap(area, pixmapPic);
      rightBtnMoveLen += 11;
      rightBtnMoveLen += 8;
   }
   //上麦\下麦文字
   if (!speak_status.isEmpty() && showChatForbid == 0 && QString::fromStdWString(user_info.role) != USER_ASSISTANT) {
      QLabel tmp;
      tmp.hide();
      QFont wordfont;
      wordfont.setFamily("Arial");
      wordfont.setPointSize(9);
      QFontMetrics fm(wordfont);
      tmp.setText(speak_status);
      QRect rec = fm.boundingRect(tmp.text());
      int name_len = rec.width();//这个就获得了字符串所占的像素宽度
      int name_height = rec.height();//这个就获得了字符串所占的像素宽度
      painter->setPen(QColor(0x99, 0x99, 0x99));
      QColor curColor = painter->pen().color();

      QRect area(rect.width() - rightBtnMoveLen - name_len, rect.y() + (rect.height() - 18) / 2, 34, 18);
      if (mIsEnterSpeak) {
         painter->setPen(QColor(0x66, 0x66, 0x66));
         painter->setBrush(QColor(0x66, 0x66, 0x66));
      }
      else {
         painter->setPen(QColor(0x40, 0x40, 0x40));
         painter->setBrush(QColor(0x40, 0x40, 0x40));
      }
      painter->drawRoundedRect(area, 2, 2);

      QRect area_text(rect.width() - rightBtnMoveLen - name_len + 5, rect.y() + (rect.height() - 18) / 2, name_len, 18);
      painter->setPen(QColor(0xff, 0xff, 0xff));
      painter->setBrush(QColor(0xff, 0xff, 0xff));
      painter->drawText(area_text, Qt::AlignLeft | Qt::AlignVCenter, speak_status);

      painter->setBrush(Qt::NoBrush);
      painter->setPen(curColor);
      rightBtnMoveLen += 25;
      rightBtnMoveLen += 10;
   }

   // 踢出
   if (showKickOut == 1) {
      rightBtnMoveLen += 10;
      QRect area(rect.width() - rightBtnMoveLen, rect.y() + (rect.height() - 15) / 2, 15, 15);
      QString icon = QString(":/states/images/kick_out_member.png");
      QPixmap pixmapPic(icon);
      painter->drawPixmap(area, pixmapPic);
      rightBtnMoveLen += 10;
   }
   // 禁言
   if (showChatForbid == 1) {
      rightBtnMoveLen += 10;
      QRect area(rect.width() - rightBtnMoveLen, rect.y() + (rect.height() - 15) / 2, 15, 15);
      QString icon = QString(":/states/images/forbidden_cancel.png");
      QPixmap pixmapPic(icon);
      painter->drawPixmap(area, pixmapPic);
      rightBtnMoveLen += 10;
   }
   // 主讲人 星标
   if (mainView == STATE_STAR) {
      rightBtnMoveLen += 10;
      QRect area(rect.width() - rightBtnMoveLen, rect.y() + (rect.height() - 15) / 2, 15, 15);
      QString icon = QString(":/role/images/host_icon.png");
      QPixmap pixmapPic(icon);
      painter->drawPixmap(area, pixmapPic);
      rightBtnMoveLen += 10;
   }
   //显示举手 星标
   else if (mainView > 0) {
      rightBtnMoveLen += 10;
      if (mainView == STATE_HANDS_UP) {
         QRect area(rect.width() - rightBtnMoveLen, rect.y() + (rect.height() - 15) / 2, 12, 15);
         QString icon = QString(":/role/images/handsUp_speak.png"); //举手
         QPixmap pixmapPic(icon);
         painter->drawPixmap(area, pixmapPic);
      }
      else {
         QRect area(rect.width() - rightBtnMoveLen, rect.y() + (rect.height() - 14) / 2, 12, 14);
         QString icon = QString(":/role/images/speaker.png");  //发言中
         QPixmap pixmapPic(icon);
         painter->drawPixmap(area, pixmapPic);
      }
      rightBtnMoveLen += 10;
   }
   else if (!speak_status.isEmpty() && speak_status == DOWNMIC) {
      rightBtnMoveLen += 10;
      QRect area(rect.width() - rightBtnMoveLen, rect.y() + (rect.height() - 14) / 2, 12, 14);
      QString icon = QString(":/role/images/speaker.png"); //发言中
      QPixmap pixmapPic(icon);
      painter->drawPixmap(area, pixmapPic);
      rightBtnMoveLen += 10;
   }
}

QSize MemberListWidgetDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const{
   QSize size = QStyledItemDelegate::sizeHint(option, index);
   size.setHeight(40);
   return size;
}

bool MemberListWidgetDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) {
   VhallAudienceUserInfo user_info = index.data(Qt::DisplayRole).value<VhallAudienceUserInfo>();
   int showMoreMenu = index.data(SHOW_MORE_MENU).isNull() ? 0 : index.data(SHOW_MORE_MENU).toInt();
   int showChatForbid = index.data(SHOW_CHAT_FORBID).isNull() ? 0 : index.data(SHOW_CHAT_FORBID).toInt();
   int showKickOut = index.data(SHOW_KICK_OUT).isNull() ? 0 : index.data(SHOW_KICK_OUT).toInt();
   int showPhone = (user_info.mDevType == 1 || user_info.mDevType == 3) ? 1 : 0;
   int mainView = index.data(SHOW_MAIN_VIEW).isNull() ? 0 : index.data(SHOW_MAIN_VIEW).toInt();
   QString speak_status = index.data(SHOW_SPEAK_STATE).isNull() ? QString("") : index.data(SHOW_SPEAK_STATE).toString();

   mStrUserId = QString::fromStdWString(user_info.userId);
   mRoleName = QString::fromStdWString(user_info.role);
   mNickName = QString::fromStdWString(user_info.userName);
   QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
   if (mouseEvent) {
      int ty = mouseEvent->type();
      int rightBtnMoveLen = 0;
      // 点击menu
      QPoint pos = mouseEvent->pos();
      if (showMoreMenu == 1) {
         rightBtnMoveLen += 30;
         QRect area(option.rect.width() - rightBtnMoveLen, option.rect.y() + (option.rect.height() - 15) / 2, 15, 15);
         if (ty == QEvent::MouseButtonPress && area.contains(pos)) {
            ClickedMenu(mStrUserId, mRoleName, QCursor::pos(), showChatForbid, showKickOut);
         }
         rightBtnMoveLen += 5;
      }
      rightBtnMoveLen += 15;
      if (showPhone == 1) {
         QRect area(option.rect.width() - rightBtnMoveLen, option.rect.y() + (option.rect.height() - 15) / 2, 11, 15);
         rightBtnMoveLen += 19;
      }

      //上麦图标
      if (!speak_status.isEmpty()) {
         QLabel tmp;
         tmp.hide();
         QFont wordfont;
         wordfont.setFamily("Arial");
         wordfont.setPointSize(9);
         QFontMetrics fm(wordfont);
         tmp.setText(speak_status);
         QRect rec = fm.boundingRect(tmp.text());
         int name_len = rec.width();//这个就获得了字符串所占的像素宽度
         int name_height = rec.height();//这个就获得了字符串所占的像素宽度
         QRect area(option.rect.width() - rightBtnMoveLen - name_len, option.rect.y() + (option.rect.height() - 18) / 2, 34, 18);
         TRACE6("%s ty %d QEvent::MouseButtonPress %d area.x %d area.y %d  width %d height %d pos.x %d pos.y %d\n",__FUNCTION__, ty , QEvent::MouseButtonPress ,area.x(), area.y(), area.width(), area.height(), pos.x(), pos.y());
         if (ty == QEvent::MouseButtonPress && (area.contains(pos) || (pos.x() >= area.x() && pos.x() <= area.x() + 34 && pos.y() >= area.y() && pos.y() <= area.y() + 18))) {
            TRACE6("SpeakState\n");
            SpeakState(speak_status);
         }
         else  if (ty == QEvent::Enter && area.contains(pos)) {
            mIsEnterSpeak = true;
         }
         else if (ty == QEvent::Leave && area.contains(pos)) {
            mIsEnterSpeak = false;
         }
      }
   }
   return QStyledItemDelegate::editorEvent(event, model, option, index);
}

void MemberListWidgetDelegate::SpeakState(QString speak_state) {
   if (mRoleName.compare(USER_HOST) == 0) { //主持人对自己
      VH::CComPtr<IMainUILogic> pMainUILogic;
      DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IMainUI, IID_IMainUILogic, pMainUILogic, return, ASSERT(FALSE));
      VhallIALive *rtclive = (VhallIALive*)pMainUILogic->GetCAliveDlg();
      if (rtclive) {
         rtclive->slot_HostPublish(speak_state == UPPERWHEAT ? true : false);
      }
   }
   else{
      if (mIsEnableInvite == false) {
         return;
      }

      VH::CComPtr<IMainUILogic> pMainUILogic;
      DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IMainUI, IID_IMainUILogic, pMainUILogic, return, ASSERT(FALSE));
      VhallIALive *rtclive = (VhallIALive*)pMainUILogic->GetCAliveDlg();
      if (rtclive) {
         rtclive->slot_Upper(mStrUserId, speak_state == UPPERWHEAT ? true : false);
      }
      if (NULL == mpUpperTime) {
         mpUpperTime = new QTimer(this);
         connect(mpUpperTime, &QTimer::timeout, this, &MemberListWidgetDelegate::slot_upperTimeOut);
      }
      mpUpperTime->start(2000);
      mIsEnableInvite = false;
   }
}

void MemberListWidgetDelegate::slot_upperTimeOut() {
   if (NULL != mpUpperTime && mpUpperTime->isActive()) {
      mpUpperTime->stop();
   }
   mIsEnableInvite = true;
}

void MemberListWidgetDelegate::ClickedMenu(QString id, QString name, QPoint pos,int showChatForbid,int showKickOut ) {
   VH::CComPtr<ICommonData> pCommonData;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
   ClientApiInitResp respInitData;
   pCommonData->GetInitApiResp(respInitData);

   if (nullptr == mPMoreOpMenu) {
      return;
   }
   if (showKickOut == 1) {  //踢出
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
      if (showChatForbid) {  //被禁言中
         mPMoreOpMenu->SetItemShow(eOptype_Forbidden, false);
         mPMoreOpMenu->SetItemShow(eOptype_CancleForbidden, true);
      }
      else {
         mPMoreOpMenu->SetItemShow(eOptype_Forbidden, true);
         mPMoreOpMenu->SetItemShow(eOptype_CancleForbidden, false);
      }
      mPMoreOpMenu->SetItemShow(eOptype_setMain, false);
      if (respInitData.selectLiveType == eLiveType_Live) {
         mPMoreOpMenu->SetShowItemCount(2);
      }
      else {
         VH::CComPtr<IMainUILogic> pMainUILogic;
         DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IMainUI, IID_IMainUILogic, pMainUILogic, return, ASSERT(FALSE));
         mpAlive = (CAliveDlg*)pMainUILogic->GetCAliveDlg();
         if (mpAlive) {
            if (respInitData.role_name == USER_HOST) {
               if (mpAlive->IsExistRenderWnd(mStrUserId) && (mRoleName == USER_HOST || mRoleName == USER_GUEST)) {
                  if (((respInitData.player == H5_LIVE_CREATE_TYPE && respInitData.user_id == mStrUserId) ||
                     (respInitData.player == FLASH_LIVE_CREATE_TYPE && respInitData.join_uid == mStrUserId)) && mpAlive->GetMainSpeakerID() != mStrUserId) {
                     mPMoreOpMenu->SetItemShow(eOptype_Forbidden, false);
                     mPMoreOpMenu->SetItemShow(eOptype_CancleForbidden, false);
                     mPMoreOpMenu->SetItemShow(eOptype_CancleKickOut, false);
                     mPMoreOpMenu->SetItemShow(eOptype_KickOut, false);
                     mPMoreOpMenu->SetItemShow(eOptype_setMain, true);
                     mPMoreOpMenu->SetShowItemCount(1);
                  }
                  else if (mpAlive->GetMainSpeakerID() == mStrUserId) {
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
   int iX = pos.x();
   int iY = pos.y();
   mPMoreOpMenu->Show(iX - mPMoreOpMenu->width() + 10, iY - 8);
}