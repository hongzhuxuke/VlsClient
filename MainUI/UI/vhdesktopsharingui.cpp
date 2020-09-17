#include "vhdesktopsharingui.h"
#include "ui_vhdesktopsharingui.h"
#include <QDesktopWidget>
#include <QDebug>
#include <QPainter>
#include <QApplication>
#include <QScreen>
#include "DebugTrace.h"
#include "CRPluginDef.h"
#include "MainUIIns.h"
#include "Msg_MainUI.h"
#include "BtnMenuWdg.h"
#include "webinarIdWdg.h"
#include "CRPluginDef.h"
#include "MainUIIns.h"
#include "Msg_MainUI.h"
#include "ICommonData.h"
#include "VH_Macro.h"
#include "DebugTrace.h"
#include "pub.Const.h"
#include "pub.Struct.h"
#include "priConst.h"
#include "pathmanager.h"
#include "ConfigSetting.h"
#include "pathmanager.h"

#define CHECK_SCREEN_SIZE_TIME   5000

#define FIX_WIDTH_ONLY_ONE_SCREEN   356 - 43
#define FIX_WIDTH_MULTI_SCREEN   356

#define MENUiTEMHOVER   "QToolButton{ \
	border:none;\
	font-size: 12px; \
	color: #52CC90;  /*字体颜色*/\
}"          

#define MENUiTEMLEAVE "QToolButton{ border:none;\
	font-size: 12px; \
	color: #666666;  /*字体颜色*/\
}\
QToolButton:hover{	color: #52CC90;  /*字体颜色*/\
}"

#define BTNLIVEQSS "QPushButton#tBtnLive{\
font-family: \"iconfont\";\
font:%1px;\
color: rgb(255, 255, 255);\
background - color: rgba(0, 0, 0, 2);\
}\
QPushButton#tBtnLive:hover\
{\
	color: #FC5659;\
	background - color: rgba(0, 0, 0, 153);\
}"

#define TIMETICK 50
#define VIDEO_LIVE_WIDTH   294
#define INTERACTIVE_LIVE_WIDTH  339 

#define ToStartLive       "QPushButton{border-image:url(:/button/images/share_start_live.png);}QPushButton:hover{border-image:url(:/button/images/share_start_live_hover.png);}"
#define ToStopLive        "QPushButton{border-image:url(:/button/images/stop_living.png);}QPushButton:hover{border-image:url(:/button/images/stop_living_hover.png);}"
#define Camera            "QPushButton{border-image:url(:/button/images/share_camera.png);}QPushButton:hover{border-image:url(:/button/images/share_camera_hover.png);}"
#define CameraClose        "QPushButton{border-image:url(:/button/images/camera_close.png);}QPushButton:hover{border-image:url(:/button/images/camera_close_hover.png);}"
#define MicOpen           "QPushButton{border-image:url(:/button/images/share_mic.png);}QPushButton:hover{border-image:url(:/button/images/share_mic_hover.png);}"
#define MicMute          "QPushButton{border-image:url(:/button/images/mic_mute.png);}QPushButton:hover{border-image:url(:/button/images/mic_mute_hover.png);}"
#define Setting           "QPushButton{border-image:url(:/button/images/share_setting.png);}QPushButton:hover{border-image:url(:/button/images/share_setting_hover.png);}"
#define ToolBtn            "QPushButton{border-image:url(:/button/images/share_tool.png);}QPushButton:hover{border-image:url(:/button/images/share_tool_hover.png);}"
#define EndShare         "QPushButton{border-image:url(:/button/images/share_stop_live.png);}QPushButton:hover{border-image:url(:/button/images/share_stop_live_hover.png);}"
#define DownArrow         "QPushButton{border-image:url(:/button/images/share_down_arrow.png);}QPushButton:hover{border-image:url(:/button/images/share_down_arrow_hover.png);}"
#define UpArrow         "QPushButton{border-image:url(:/button/images/share_up_arrow.png);}QPushButton:hover{border-image:url(:/button/images/share_up_arrow_hover.png);}"
 
#define NetState_H    QString("border-image: url(:/check/images/device_check/icon_net_h.png);")
#define NetState_M    QString("border-image: url(:/check/images/device_check/icon_net_m.png);")
#define NetState_L    QString("border-image: url(:/check/images/device_check/icon_net_low.png);")

#define Mic_Level_1    QString("border-image: url(:/button/images/desk_top_mic_1.png);")
#define Mic_Level_2    QString("border-image: url(:/button/images/desk_top_mic_2.png);")
#define Mic_Level_3    QString("border-image: url(:/button/images/desk_top_mic_3.png);")
#define Mic_Level_4    QString("border-image: url(:/button/images/desk_top_mic_4.png);")
#define Mic_Level_5    QString("border-image: url(:/button/images/desk_top_mic_5.png);")


VHDesktopSharingUI::VHDesktopSharingUI(QDialog *parent) :
   QDialog(parent),
    ui(new Ui::VHDesktopSharingUI)
{
   ui->setupUi(this);
   this->setWindowOpacity(0.99);
   this->setWindowFlags(this->windowFlags() | Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint/* | Qt::Tool | Qt::WindowStaysOnTopHint*/);
   setAttribute(Qt::WA_TranslucentBackground);
   setAutoFillBackground(false);
   this->setPalette(Qt::transparent);

   mCheckScreenSizeChange = new QTimer(this);
   if (mCheckScreenSizeChange) {
      connect(mCheckScreenSizeChange, SIGNAL(timeout()), this, SLOT(slot_OnCheckSizeChanged()));
   }
   
   connect(this,SIGNAL(sigShow()),this,SLOT(slotShow()));
   connect(this,SIGNAL(sigClose()),this,SLOT(slotClose()));
   
   ui->tBtnLive->setStyleSheet(ToStartLive);
   ui->tBtnCamera->setStyleSheet(Camera);
   ui->tBtnMic->setStyleSheet(MicOpen);
   ui->tBtnStopSharing->setStyleSheet(EndShare);
   ui->tBtnQuikTool->setStyleSheet(ToolBtn);
   ui->tBtnSetting->setStyleSheet(Setting);
   //ui->tBtnPlayer->setStyleSheet(Setting);
   ui->btnEx->setStyleSheet(UpArrow);

   connect(ui->tBtnStopSharing,&QPushButton::clicked,this,  &VHDesktopSharingUI::slot_StopSharing);
   connect(ui->tBtnCamera, &QPushButton::clicked,this, &VHDesktopSharingUI::slot_OnClickedCamera);
   connect(ui->tBtnSetting, &QPushButton::clicked,this, &VHDesktopSharingUI::slot_OnSettingClicked);
   connect(ui->tBtnQuikTool, &QPushButton::clicked, this, &VHDesktopSharingUI::SlottBtnQuikTool);
   connect(ui->tBtnLive, &QPushButton::clicked,this, &VHDesktopSharingUI::sigLiveClicked);
   connect(ui->tBtnMic, &QPushButton::clicked,this, &VHDesktopSharingUI::slot_OnClickedMic);
   connect(ui->pushButton_shareSelect, &QPushButton::clicked, this, &VHDesktopSharingUI::sigShareSelect);
   
   ui->tBtnStopSharing->setToolTip(QStringLiteral("停止共享"));
   ui->tBtnCamera->setToolTip(QStringLiteral("摄像头"));
   ui->tBtnSetting->setToolTip(QStringLiteral("设置"));
   ui->tBtnQuikTool->setToolTip(QStringLiteral("快捷工具"));
   ui->tBtnLive->setToolTip(QStringLiteral("开始直播"));
   ui->tBtnMic->setToolTip(QStringLiteral("麦克风"));
   ui->btnEx->setToolTip(QStringLiteral("隐藏"));
   ui->pushButton_shareSelect->setToolTip(QStringLiteral("屏幕切换"));


   mPMoreOpMenu = new BtnMenuWdg(this);
   mPMoreOpMenu->addItem(BtnMenuWdg::eOptype_Member, BtnMenuWdg::ePlaceType_Top);
   mPMoreOpMenu->addItem(BtnMenuWdg::eOptype_Chat, BtnMenuWdg::ePlaceType_Mid);
   mPMoreOpMenu->addItem(BtnMenuWdg::eOptype_Interactive, BtnMenuWdg::ePlaceType_Bottom);
   connect(mPMoreOpMenu, &BtnMenuWdg::sigClicked, this, &VHDesktopSharingUI::slotMenuCliecked);
   this->setFixedSize(356, 56);

   mActiveLiveFloatCtrl = new DesktopCameraCtrl(this);
   if (mActiveLiveFloatCtrl) {
      mActiveLiveFloatCtrl->hide();
      connect(mActiveLiveFloatCtrl,SIGNAL(sig_ClickedSwitchBtn()), this,SLOT(slot_ClickedSwitchBtn()));
   }
}

void VHDesktopSharingUI::SetLiveState(bool isVideoLive) {
   //if (isVideoLive) {
   //   this->setFixedSize(VIDEO_LIVE_WIDTH, 56);
   //} else {
   //   this->setFixedSize(INTERACTIVE_LIVE_WIDTH, 56);
   //}
}

void VHDesktopSharingUI::slot_ClickedSwitchBtn() {
   emit sigFloatSwitchBtn();
}

void VHDesktopSharingUI::SetFloatWndBtnState(bool show) {
   if (mActiveLiveFloatCtrl) {
      mActiveLiveFloatCtrl->SetDesktopCameraWndState(show);
   }
}

void VHDesktopSharingUI::SetMicVol(int level){
   if (!mbMicMute) {
      if (level >= 0 && level < 4) {
         ui->tBtnMic->setStyleSheet(Mic_Level_1);
      }
      else if (level >= 4 && level < 16) {
         ui->tBtnMic->setStyleSheet(Mic_Level_2);
      }
      else if (level >= 16 && level < 36) {
         ui->tBtnMic->setStyleSheet(Mic_Level_3);
      }
      else if (level >=36 && level < 64) {
         ui->tBtnMic->setStyleSheet(Mic_Level_4);
      }
      else if (level >= 64 && level <= 100) {
         ui->tBtnMic->setStyleSheet(Mic_Level_5);
      }
      ui->tBtnMic->repaint();
      ui->tBtnMic->update();
      update();      repaint();
      setFocus();
   }
}

void VHDesktopSharingUI::SetPushStreamLost(float lostRate) {
   //优0-3%    良 4%-9%   差：>10%
   if (lostRate <= 0.03) {
      ui->pushButton_netState->setStyleSheet(NetState_H);
   }
   else if (lostRate > 0.03&& lostRate <= 0.09) {
      ui->pushButton_netState->setStyleSheet(NetState_M);
   }
   else {
      ui->pushButton_netState->setStyleSheet(NetState_L);
   }
   ui->pushButton_netState->repaint();
   this->repaint();
}

void VHDesktopSharingUI::SetCurrentShareScreenInfo(const VHD_WindowInfo& info) {
   mCurrentWindowInfo = info;
   if (mCheckScreenSizeChange) {
      mCheckScreenSizeChange->stop();
   }
   QJsonObject body;
   STRU_MAINUI_LOG log;
   swprintf_s(log.m_wzRequestUrl, DEF_MAX_HTTP_URL_LEN, L"key=%s&k=%d&op=%s&w=%d&h=%d", STR_VH_LIVE_OPERATE, eLogRePortK_VHSDK_DesktopShare, L"DesktopShare", info.rect.right- info.rect.left, info.rect.bottom - info.rect.top);
   QString json = CPathManager::GetStringFromJsonObject(body);
   strcpy_s(log.m_wzRequestJson, json.mid(0, DEF_MAX_HTTP_URL_LEN).toLatin1().data());
   SingletonMainUIIns::Instance().PostCRMessage(MSG_MAINUI_LOG, &log, sizeof(STRU_MAINUI_LOG));
}

VHD_WindowInfo VHDesktopSharingUI::GetCurrentShareWindowInfo() {
   return mCurrentWindowInfo;
}

void VHDesktopSharingUI::slot_OnSettingClicked() {
   emit sigSettingClicked();
   if (mActiveLiveFloatCtrl) {
      mActiveLiveFloatCtrl->hide();
   }
   setFocus();
   update();   repaint();
}

void VHDesktopSharingUI::slot_OnShowTimer() {
   show();
   raise();
}

void VHDesktopSharingUI::slot_OnClickedCamera() {
   VH::CComPtr<ICommonData> pCommonData;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
   ClientApiInitResp initApiResp;
   pCommonData->GetInitApiResp(initApiResp);
   if (initApiResp.selectLiveType == eLiveType_Live) {
      emit sigCameraClicked();
   }
   else {
      if (mActiveLiveFloatCtrl) {
         mActiveLiveFloatCtrl->move(mapToGlobal(ui->tBtnCamera->pos()) + QPoint(0,this->height() + 5));
         mActiveLiveFloatCtrl->show();
      }
   }
}

void VHDesktopSharingUI::slotMenuCliecked(const int& iOpType)
{
   TRACE6("%s iOpType %d\n", __FUNCTION__, iOpType);
	switch (iOpType)
	{
	case BtnMenuWdg::eOptype_Member:
		emit sigMemberClicked();
		break;
	case	BtnMenuWdg::eOptype_Chat:
		emit sigChatClicked();
		break;
	case BtnMenuWdg::eOptype_Interactive:
		emit sigInteractiveClicked(true, 0,0);
		break;
	default:
		break;
	}
}

void VHDesktopSharingUI::slot_StopSharing(){
   TRACE6("%s\n",__FUNCTION__);
   if (mActiveLiveFloatCtrl) {
      mActiveLiveFloatCtrl->hide();
   }
	if (nullptr != mPMoreOpMenu && mPMoreOpMenu->isVisible()){
		mPMoreOpMenu->hide();
		mPMoreOpMenu->close();
	}
   if (mCheckScreenSizeChange) {
      mCheckScreenSizeChange->stop();
   }
   if (mActiveLiveFloatCtrl) {
      mActiveLiveFloatCtrl->hide();
   }
   //if (mShowUITimer) {
   //   mShowUITimer->stop();
   //}
   mIsShow = false;
	emit sigToStopDesktopSharing();
}

void VHDesktopSharingUI::slot_OnClickedMic() {
   emit sigMicClicked();
   if (mActiveLiveFloatCtrl) {
      mActiveLiveFloatCtrl->hide();
   }
}

void VHDesktopSharingUI::slot_OnClickedPlayer() {
   emit sigPlayer();
   if (mActiveLiveFloatCtrl) {
      mActiveLiveFloatCtrl->hide();
   }
}

void VHDesktopSharingUI::SlottBtnQuikTool(){
   if (mActiveLiveFloatCtrl) {
      mActiveLiveFloatCtrl->hide();
   }
   VH::CComPtr<ICommonData> pCommonData;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
   if (pCommonData && pCommonData->GetStartMode() == eStartMode_flash) {
      return;
   }

   ClientApiInitResp initApiResp;
   pCommonData->GetInitApiResp(initApiResp);
   if (pCommonData) {
      if (initApiResp.selectLiveType == eLiveType_VhallActive ) {
         int showCount = 1;
         bool enableTool = false;
         bool enableMember = false;
         if (initApiResp.player == H5_LIVE_CREATE_TYPE) {
            std::list<QString>::iterator iter = initApiResp.permission_data.begin();
            for (; iter != initApiResp.permission_data.end(); iter++) {
               if ((*iter).toInt() == eWebOpType_questionnaire || (*iter).toInt() == eWebOpType_Questions || (*iter).toInt() == eWebOpType_Luck || (*iter).toInt() == eWebOpType_Signin) {
                  enableTool = true;
               }
               else if ((*iter).toInt() == eWebOpType_MemberManager) {
                  enableMember = true;
               }
            }
         }
         else {
            if (pCommonData->GetMembersManagerState()) {
               enableMember = true;
            }
            std::list<QString>::iterator iter = initApiResp.permission_data.begin();
            for (; iter != initApiResp.permission_data.end(); iter++) {
               if ((*iter) == "webinar_award" || (*iter) == "survey" || (*iter) == "personal_chat" || (*iter) == "sign_in") {
                  enableTool = true;
                  break;
               }
            }
         }

         if (enableTool) {
            showCount++;
         }
         if (enableMember) {
            showCount++;
         }
         mPMoreOpMenu->SetItemShow(BtnMenuWdg::eOptype_Member, enableMember);
         mPMoreOpMenu->SetItemShow(BtnMenuWdg::eOptype_Chat, true);
         mPMoreOpMenu->SetItemShow(BtnMenuWdg::eOptype_Interactive, enableTool);
         mPMoreOpMenu->SetShowItemCount(showCount);
      }
      else {
         if (mbShowChat) {
            mPMoreOpMenu->SetItemShow(BtnMenuWdg::eOptype_Member, true);
            mPMoreOpMenu->SetItemShow(BtnMenuWdg::eOptype_Chat, true);
            mPMoreOpMenu->SetItemShow(BtnMenuWdg::eOptype_Interactive, true);
            mPMoreOpMenu->SetShowItemCount(3);
         }
         else {
            mPMoreOpMenu->SetItemShow(BtnMenuWdg::eOptype_Member, false);
            mPMoreOpMenu->SetItemShow(BtnMenuWdg::eOptype_Chat, false);
            mPMoreOpMenu->SetItemShow(BtnMenuWdg::eOptype_Interactive, true);
            mPMoreOpMenu->SetShowItemCount(1);
         }
      }
   }



   QPoint pos = mapToGlobal(ui->tBtnQuikTool->pos());
   int iX = pos.x();
   int iY = pos.y();
   mPMoreOpMenu->show(iX, iY + ui->tBtnQuikTool->height());
}

VHDesktopSharingUI::~VHDesktopSharingUI(){
    delete ui;
}

void VHDesktopSharingUI::SetPlayerState(bool open){
	//if (open) {
	//	ui->tBtnPlayer->setText(QChar(0xe742));// 打开状态
	//}
	//else {
	//	ui->tBtnPlayer->setText(QChar(0xe743));//关闭状态
	//}
}

void VHDesktopSharingUI::SetStreamButtonStatus(bool status){
	QString strQss = TOOLTIPQSS;
   if(status) {
	   ui->tBtnLive->setText(QChar(0xe73d));
	   ui->tBtnLive->setToolTip(QStringLiteral("结束直播"));
	   ui->tBtnLive->setStyleSheet(ToStopLive);
   }
   else {
	   ui->tBtnLive->setText(QChar(0xe721));
	   ui->tBtnLive->setToolTip(QStringLiteral("开始直播"));
	   ui->tBtnLive->setStyleSheet(ToStartLive);
   }
   m_bStreamStatus = status;
   ui->tBtnLive->repaint();
   setFocus();
   update();   repaint();
}

void VHDesktopSharingUI::SetMicMute(bool bMute){
   mbMicMute = bMute;
   if(bMute) {
	  ui->tBtnMic->setStyleSheet(MicMute); //关闭状态
   }
   else{
	  ui->tBtnMic->setStyleSheet(MicOpen);//打开状态
   }
   ui->tBtnMic->repaint();
   ui->tBtnMic->update();
   update();   repaint();
   setFocus();
}

void VHDesktopSharingUI::SetCameraState(bool open) {
	//if (open) {
	//	ui->tBtnCamera->setStyleSheet(Camera); //changeImage(":/CtrlBtn/img/CtrlButton/camera_open.png");
	//}
	//else {
	//	ui->tBtnCamera->setStyleSheet(CameraClose); //(":/CtrlBtn/img/CtrlButton/camera_close.png");
	//}
   ui->tBtnCamera->setStyleSheet(Camera);
	//ui->tBtnPlayer->repaint();
   update();   repaint();
   setFocus();
}

void VHDesktopSharingUI::on_btnEx_clicked(bool checked /*= false*/){
	if (this->y()>=0){
		int x, y;
      QRect rect;
      QString name;
      QDesktopWidget* desktopWidget = QApplication::desktop();
      if (desktopWidget) {
         rect = desktopWidget->rect();
         int index = desktopWidget->screenNumber(this);
         QList<QScreen *> screenlist = QGuiApplication::screens();
         for (int i = 0; i < screenlist.size(); i++) {
            name = screenlist.at(i)->name();
            if (QString::fromStdWString(mCurrentWindowInfo.name).isEmpty() && index == i) {
               rect = screenlist.at(i)->availableGeometry();
               break;
            }
            else if (QString::fromStdWString(mCurrentWindowInfo.name) == name) {
               rect = screenlist.at(i)->availableGeometry();
               break;
            }
         }
      }

      x = rect.x() + (rect.width() - this->width()) / 2;
		y = -43;
		ui->btnEx->setText(QChar(0xe72d));
		ui->btnEx->setToolTip(QStringLiteral("显示"));
      ui->btnEx->setStyleSheet(DownArrow);
		qDebug() << "move" << x << y;
		move(x, y);
	}
	else{
		ui->btnEx->setText(QChar(0xe72a));
		ui->btnEx->setToolTip(QStringLiteral("隐藏"));
      ui->btnEx->setStyleSheet(UpArrow);
		slotShow();
	}
}

void VHDesktopSharingUI::slotShow(){
   int open_enhance = ConfigSetting::ReadInt(CPathManager::GetConfigPath(), GROUP_DEFAULT, DESKTOP_ENHANCE, 0);
   QJsonObject body;
   body["eh"] = QString::number(open_enhance);
   SingletonMainUIIns::Instance().reportLog(STR_VH_LIVE_OPERATE, eLogRePortK_DesktopShare, open_enhance == 0 ? L"0" : L"1", body);

   show();
   Qt::WindowFlags flags = windowFlags();
   setWindowFlags(flags | Qt::WindowStaysOnTopHint);
   show();
   raise();
   QRect rect;
   QString name;
   QDesktopWidget* desktopWidget = QApplication::desktop();
   if (desktopWidget) {
      rect = desktopWidget->rect();
      int index = desktopWidget->screenNumber(this);
      QList<QScreen *> screenlist = QGuiApplication::screens();
      for (int i = 0; i < screenlist.size(); i++) {
         name = screenlist.at(i)->name();
         if (QString::fromStdWString(mCurrentWindowInfo.name).isEmpty() && index == i) {
            rect = screenlist.at(i)->availableGeometry();  
            break;
         }
         else if (QString::fromStdWString(mCurrentWindowInfo.name) == name) {
            rect = screenlist.at(i)->availableGeometry();
            break;
         }
      }
   }
   int x, y;
   x = rect.x() + (rect.width() - this->width()) / 2;
   y = 0;

   move(x,y);
   if (mCheckScreenSizeChange) {
      mCheckScreenSizeChange->start(CHECK_SCREEN_SIZE_TIME);
   }

   TRACE6("%s end\n", __FUNCTION__);
}


void VHDesktopSharingUI::slot_OnCheckSizeChanged() {
   VHD_Window_Enum_init(VHD_Desktop);
   vector<VHD_WindowInfo> allDesktop = VHD_DesktopWindowInfo();
   for (int i = 0; i < allDesktop.size(); i++) {
      VHD_WindowInfo info = allDesktop.at(i);
      if (mCurrentWindowInfo.screen_id == info.screen_id &&
         (mCurrentWindowInfo.rect.bottom != info.rect.bottom ||
         mCurrentWindowInfo.rect.left != info.rect.left ||
         mCurrentWindowInfo.rect.right != info.rect.right ||
         mCurrentWindowInfo.rect.top != info.rect.top)) {
         int width = info.rect.right - info.rect.left;
         QRect rect;
         QString name;
         QDesktopWidget* desktopWidget = QApplication::desktop();
         if (desktopWidget) {
            rect = desktopWidget->rect();
            int index = desktopWidget->screenNumber(this);
            QList<QScreen *> screenlist = QGuiApplication::screens();
            for (int i = 0; i < screenlist.size(); i++) {
               name = screenlist.at(i)->name();
               if (QString::fromStdWString(mCurrentWindowInfo.name).isEmpty() && index == i) {
                  rect = screenlist.at(i)->availableGeometry();
                  break;
               }
               else if (QString::fromStdWString(mCurrentWindowInfo.name) == name) {
                  rect = screenlist.at(i)->availableGeometry();
                  break;
               }
            }
         }
         int x, y;
         x = rect.x() + (rect.width() - this->width()) / 2;
         y = 0;
         move(x,y);
         //this->move(info.rect.left + (width - this->width()) / 2, info.rect.top);
         mCurrentWindowInfo = info;
      }
   }
}

void VHDesktopSharingUI::slotClose(){
   mIsShow = false;
   QWidget::close();
}

void VHDesktopSharingUI::Show() {
    VH::CComPtr<ICommonData> pCommonData;
    DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
    if (pCommonData && pCommonData->GetStartMode() == eStartMode_flash) {
        ui->tBtnQuikTool->hide();
    }
    TRACE6("%s\n", __FUNCTION__);
    mIsShow = true;
    //if (mShowUITimer) {
    //   mShowUITimer->start(2000);
    //}
    emit this->sigShow();
}

void VHDesktopSharingUI::hideEvent(QHideEvent *e) {
   e->ignore();
   //int a = 0;
   //a = 0;
   if (mIsShow) {
      show();
   }
   else {
      e->ignore();
   }
   //QWidget::hideEvent(e);
   TRACE6("%s\n", __FUNCTION__);
}                      

void VHDesktopSharingUI::showEvent(QShowEvent *e) {
    this->activateWindow();
    QRect rect;
    QString name;
    QDesktopWidget* desktopWidget = QApplication::desktop();
    if (desktopWidget) {
       rect = desktopWidget->rect();
       int index = desktopWidget->screenNumber(this);
       QList<QScreen *> screenlist = QGuiApplication::screens();
       for (int i = 0; i < screenlist.size(); i++) {
          name = screenlist.at(i)->name();
          if (QString::fromStdWString(mCurrentWindowInfo.name).isEmpty() && index == i) {
             rect = screenlist.at(i)->availableGeometry();
             break;
          }
          else if (QString::fromStdWString(mCurrentWindowInfo.name) == name) {
             rect = screenlist.at(i)->availableGeometry();
             break;
          }
       }
    }




    int x, y;
    x = rect.x() + (rect.width() - this->width()) / 2;
    y = 0;
    this->move(x, y);
    TRACE6("%s x %d y %d\n", __FUNCTION__, rect.x(), rect.y());
    this->setAttribute(Qt::WA_Mapped);
    QWidget::showEvent(e);
}

void VHDesktopSharingUI::Close() {
   if (mPMoreOpMenu) {
      mPMoreOpMenu->hide();
   }
   if (mActiveLiveFloatCtrl) {
      mActiveLiveFloatCtrl->hide();
   }
   //if (mShowUITimer) {
   //   mShowUITimer->stop();
   //}
   mIsShow = false;
   emit this->sigClose();
}

void VHDesktopSharingUI::SetShowShareSelect(const bool show) {
   show ? ui->pushButton_shareSelect->show() : ui->pushButton_shareSelect->hide();
   show ? setFixedWidth(FIX_WIDTH_MULTI_SCREEN) : setFixedWidth(FIX_WIDTH_ONLY_ONE_SCREEN);      
}

void VHDesktopSharingUI::SetSetBtnShow(const bool& bShow/* = false*/)
{
	bShow ? ui->tBtnSetting->show() : ui->tBtnSetting->hide();
}

void VHDesktopSharingUI::SetPlayerShow(const bool& bShow /*= false*/)
{
	//bShow?ui->tBtnPlayer->show() :ui->tBtnPlayer->hide();
}

void VHDesktopSharingUI::SetLiveShow(const bool& bShow /*= true*/)
{
	bShow ? ui->tBtnLive->show() : ui->tBtnLive->hide();
}

QPoint VHDesktopSharingUI::GetCameraPos()
{
	return ui->tBtnCamera->mapToGlobal(QPoint(0, 0));
}

void VHDesktopSharingUI::setEnablePlguin(bool enable) {
    if (mPMoreOpMenu) {
        mPMoreOpMenu->SetEnable(BtnMenuWdg::eOptype_Interactive, enable);
    }
}

void VHDesktopSharingUI::SetStartLiveBtnEnable(bool enable) {
    ui->tBtnLive->setEnabled(enable);
    TRACE6("%s eanble:%d", __FUNCTION__, enable);
}

int VHDesktopSharingUI::GetCameraHeight()
{
	return ui->tBtnCamera->height();
}

void VHDesktopSharingUI::SetShowChat(bool bShow) {
	mbShowChat = bShow;
}