#include "LeftToolWdg.h"
#include "Msg_MainUI.h"
#include "MainUIIns.h"
#include "ToolButton.h"
#include "pathmanager.h"
#include "ConfigSetting.h"
#include "Msg_VhallRightExtraWidget.h"
#include "CRPluginDef.h"
#include "MainUIIns.h"
#include "Msg_MainUI.h"
#include "ICommonData.h"
#include "VH_Macro.h"
#include "DebugTrace.h"
#include "priConst.h"

#define BORDERCOLOR  "rgba(67,67,67,1);"

#define MEMBERQSS "QPushButton{\
border-image: url(:/button/images/member.png) 0 60 0 60;\
}\
QPushButton:hover{\
border-image : url(:/button/images/member.png) 0 120 0 0;\
}\
QPushButton:pressed{\
border-image: url(:/button/images/member.png) 0 60 0 60;\
}"

#define MEMBERUPPERQSS "QPushButton{\
border-image : url(:/button/images/memberUpper.png) 0 60 0 60;\
}\
QPushButton:hover{\
border-image : url(:/button/images/memberUpper.png) 0 120 0 0;\
}\
QPushButton:pressed{\
border-image : url(:/button/images/memberUpper.png) 0 60 0 60;\
}"




#define DocNormal "border-image:url(:/button/images/doc.png);"
#define DocClicked "border-image:url(:/button/images/doc_hover.png);"
#define DocDisable "border-image:url(:/button/images/doc_disable.png);"

#define DesktopNormal "border-image:url(:/button/images/desktop_tool.png);"
#define DesktopClicked "border-image:url(:/button/images/desktop_tool_hover.png);"
#define DesktopDisable "border-image:url(:/button/images/desktop_tool_disable.png);"

#define PlayFileNormal "border-image:url(:/button/images/play_file.png);"
#define PlayFileClicked "border-image:url(:/button/images/play_file_hover.png);"
#define PlayFileDisable "border-image:url(:/button/images/play_file_disable.png);"

#define PluginNormal "border-image:url(:/button/images/plugin_tool.png);"
#define PluginClicked "border-image:url(:/button/images/plugin_tool_hover.png);"
#define PluginDisable "border-image:url(:/button/images/plugin_tool_disable.png);"

#define MemberNormal "border-image:url(:/button/images/member_normal.png);"
#define MemberClicked "border-image:url(:/button/images/member_hover.png);"
#define MemberDisable "border-image:url(:/button/images/member_disable.png);"

#define ShareNormal "border-image:url(:/button/images/share.png);"
#define ShareClicked "border-image:url(:/button/images/share_hover.png);"
#define ShareDisable "border-image:url(:/button/images/share_disable.png);"

#define WbBtnNormal "border-image:url(:/button/images/borad.png);"
#define WbBtnClicked "border-image:url(:/button/images/borad_hover.png);"
#define WbBtnDisable "border-image:url(:/button/images/borad-disable.png);"


LeftToolWdg::LeftToolWdg(QWidget *parent)
   : CWidget(parent)
{
   ui.setupUi(this);
   //文档
   ui.pushButton_doc->Create(ButtonWdg::eFontDirection_Underside, 8, 12, 17, DocNormal, DocClicked, DocDisable);
   ui.pushButton_doc->setIconTextSize(22,22);
   ui.pushButton_doc->setBorderColor(BORDERCOLOR, BORDERCOLOR);
   ui.pushButton_doc->setIconAlignment(Qt::AlignHCenter | Qt::AlignBottom);
   ui.pushButton_doc->setFontText(DOC, Qt::AlignHCenter | Qt::AlignTop);
   connect(ui.pushButton_doc, &ButtonWdg::btnClicked, this, &LeftToolWdg::slot_OnClickedDoc);

   ui.pushButton_Whiteboard->Create(ButtonWdg::eFontDirection_Underside, 8, 12, 17, WbBtnNormal, WbBtnClicked, WbBtnDisable);
   ui.pushButton_Whiteboard->setIconTextSize(22, 22);
   ui.pushButton_Whiteboard->setBorderColor(BORDERCOLOR, BORDERCOLOR);
   ui.pushButton_Whiteboard->setIconAlignment(Qt::AlignHCenter | Qt::AlignBottom);
   ui.pushButton_Whiteboard->setFontText(WHITE_BOARD, Qt::AlignHCenter | Qt::AlignTop);
   connect(ui.pushButton_Whiteboard, &ButtonWdg::btnClicked, this, &LeftToolWdg::slot_OnClickedWhiteBorad);


   ui.DesktopSharingBtn->Create(ButtonWdg::eFontDirection_Underside, 8, 12, 17, DesktopNormal, DesktopClicked, DesktopDisable);
   ui.DesktopSharingBtn->setIconTextSize(22, 22);
   ui.DesktopSharingBtn->setBorderColor(BORDERCOLOR, BORDERCOLOR);
   ui.DesktopSharingBtn->setIconAlignment(Qt::AlignHCenter | Qt::AlignBottom);
   ui.DesktopSharingBtn->setFontText(DESKTOP_SHARE, Qt::AlignHCenter | Qt::AlignTop);
   connect(ui.DesktopSharingBtn, &ButtonWdg::btnClicked, this, &LeftToolWdg::SlotDesktopClicked);

   //插播文件
   ui.btnInsertVedio->hide();
   ui.btnInsertVedio->Create(ButtonWdg::eFontDirection_Underside, 8, 12, 17, PlayFileNormal, PlayFileClicked, PlayFileDisable);
   ui.btnInsertVedio->setIconTextSize(22, 22);
   ui.btnInsertVedio->setBorderColor(BORDERCOLOR, BORDERCOLOR);
   ui.btnInsertVedio->setIconAlignment(Qt::AlignHCenter | Qt::AlignBottom);
   ui.btnInsertVedio->setFontText(INSERT_PLAY_FILE, Qt::AlignHCenter | Qt::AlignTop);
   connect(ui.btnInsertVedio, &ButtonWdg::btnClicked, this, &LeftToolWdg::sig_InsertVedio);//sig_OnPlayFileClicked

   //互动工具
   ui.pushButton_pluginUrl->Create(ButtonWdg::eFontDirection_Underside, 8, 12, 17, PluginNormal, PluginClicked, PluginDisable);
   ui.pushButton_pluginUrl->setIconTextSize(22, 22);
   ui.pushButton_pluginUrl->setBorderColor(BORDERCOLOR, BORDERCOLOR);
   ui.pushButton_pluginUrl->setIconAlignment( Qt::AlignHCenter | Qt::AlignBottom);
   ui.pushButton_pluginUrl->setFontText(LIVE_TOOL_NAME, Qt::AlignHCenter | Qt::AlignTop);
   connect(ui.pushButton_pluginUrl, &ButtonWdg::btnClicked, this, &LeftToolWdg::slot_OnClickedPluginUrl);
   SetEixtUnseeHandsUpList(false);
   //成员

   //分享
   ui.SharedBtn->Create(ButtonWdg::eFontDirection_Underside, 8, 12, 17, ShareNormal, ShareClicked, ShareDisable);
   ui.SharedBtn->setIconTextSize(22, 22);
   ui.SharedBtn->setBorderColor(BORDERCOLOR, BORDERCOLOR);
   ui.SharedBtn->setIconAlignment(Qt::AlignHCenter | Qt::AlignBottom);
   ui.SharedBtn->setFontText(SHARE_URL, Qt::AlignHCenter | Qt::AlignTop);
   connect(ui.SharedBtn, &ButtonWdg::btnClicked, this, &LeftToolWdg::sigSharedBtnClicked);
   connect(this, &LeftToolWdg::SigSetDesktopSharing, this, &LeftToolWdg::SlotSetDesktopSharing);

   QString toolConfPath = CPathManager::GetAppDataPath() + QString::fromStdWString(VHALL_TOOL_CONFIG);
   int hideLogo = ConfigSetting::ReadInt(toolConfPath, GROUP_DEFAULT, KEY_VHALL_LOGO_HIDE, 0);
   if (hideLogo == 1) {//隐藏logo
      ui.SharedBtn->hide();
   }
   else{
      ui.SharedBtn->show();
   }
}

void LeftToolWdg::SetEixtUnseeHandsUpList(bool bExit){
   //if (bExit){  //有人举手
   //   //ui.btnMember->setStyleSheet(MEMBERUPPERQSS);
   //   ui.btnMember->SetPaintPoint(true);
   //}
   //else{ //正常态
   //   //ui.btnMember->setStyleSheet(MEMBERQSS);
   //   ui.btnMember->SetPaintPoint(false);
   //}
}

void LeftToolWdg::ShowInsertVedio(const bool& bShow/* = true*/){
   bShow ? ui.btnInsertVedio->show() : ui.btnInsertVedio->hide();
}

LeftToolWdg::~LeftToolWdg(){
}

void LeftToolWdg::SetDocBtnClicked(bool click) {
   ui.pushButton_doc->SetClicked(true);
   ui.pushButton_doc->SetHoverState(true);
}

void LeftToolWdg::SetDocBtnSelect() {   
   if (ui.pushButton_doc->isEnabled() && ui.pushButton_Whiteboard->isEnabled()) {
      ui.pushButton_doc->SetClicked(true);
      ui.pushButton_doc->SetHoverState(true);
      ui.pushButton_Whiteboard->SetClicked(false);
      ui.pushButton_Whiteboard->SetHoverState(false);
      mbClickedWhiteBoard = false;
   }
}

void LeftToolWdg::SetWBBtnSelect() {
   if (ui.pushButton_doc->isEnabled() && ui.pushButton_Whiteboard->isEnabled()) {
      ui.pushButton_doc->SetClicked(false);
      ui.pushButton_doc->SetHoverState(false);
      ui.pushButton_Whiteboard->SetClicked(true);
      ui.pushButton_Whiteboard->SetHoverState(true);
      mbClickedWhiteBoard = true;
   }
}

void LeftToolWdg::ReInitDocAndWhiteBoradSheet() {
   ui.pushButton_doc->SetClicked(false);
   ui.pushButton_Whiteboard->SetClicked(false);
   mbClickedWhiteBoard = false;
}

void LeftToolWdg::ReInitDocSelectStatus() {
   ui.pushButton_doc->SetClicked(true);
   ui.pushButton_doc->SetHoverState(true);
   ui.pushButton_Whiteboard->SetClicked(false);
   ui.pushButton_Whiteboard->SetHoverState(false);
   mbClickedWhiteBoard = false;
}

void LeftToolWdg::CheckStartMode() {
   VH::CComPtr<ICommonData> pCommonData;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
   ClientApiInitResp initApiResp;
   pCommonData->GetInitApiResp(initApiResp);
   if (pCommonData) {
      if (pCommonData->GetStartMode() == eStartMode_flash) {
         ui.pushButton_doc->hide();
         ui.pushButton_Whiteboard->hide();
         ui.pushButton_pluginUrl->hide();
         //ui.btnMember->hide();
      }
      else if (initApiResp.selectLiveType == eLiveType_VhallActive) {
         bool enableTool = false;
         if (initApiResp.player == H5_LIVE_CREATE_TYPE) {
            std::list<QString>::iterator iter = initApiResp.permission_data.begin();
            for (; iter != initApiResp.permission_data.end(); iter++) {
               if ((*iter).toInt() == eWebOpType_questionnaire || (*iter).toInt() == eWebOpType_Questions || (*iter).toInt() == eWebOpType_Luck || (*iter).toInt() == eWebOpType_Signin) {
                  enableTool = true;
                  break;
               }
            }
            //ui.btnMember->hide();
         }
         else if (initApiResp.player == FLASH_LIVE_CREATE_TYPE) {
            if (!pCommonData->GetMembersManagerState()) {
               /*ui.btnMember->hide();*/
            }
            std::list<QString>::iterator iter = initApiResp.permission_data.begin();
            for (; iter != initApiResp.permission_data.end(); iter++) {
               if ((*iter) == "webinar_award" || (*iter) == "survey" || (*iter) == "personal_chat" || (*iter) == "sign_in") {
                  enableTool = true;
                  break;
               }
            }
         }

         if (!enableTool) {
            ui.pushButton_pluginUrl->hide();
         }
      }
   }
}

void LeftToolWdg::ShowSharedButton(bool show) {
   if (show) {
      if (ui.SharedBtn)
         ui.SharedBtn->show();
   }
   else {
      if (ui.SharedBtn)
         ui.SharedBtn->hide();
   }
}

void LeftToolWdg::GetPluginUrlToolDlgPos(int& x, int& y){
   QPoint globalPoint = ui.pushButton_pluginUrl->mapToGlobal(QPoint(0, 0));
   x = globalPoint.x() + ui.pushButton_pluginUrl->width();
   y = globalPoint.y();
}

void LeftToolWdg::setEnablePluginBtn(bool enable) {
   ui.pushButton_pluginUrl->SetBtnEnable(enable);
   mbEnablePlguin = enable;
}

void LeftToolWdg::setEnableDesktopBtn(bool enable) {
   ui.DesktopSharingBtn->SetBtnEnable(enable);
   mbEnableDesktop = enable;
}

void LeftToolWdg::setEnableDocAndWhiteBtn(bool enable) {
   ui.pushButton_Whiteboard->SetBtnEnable(enable);
   ui.pushButton_doc->SetBtnEnable(enable);
}

void LeftToolWdg::setEnableInsertVedio(bool enable){
   ui.btnInsertVedio->SetBtnEnable(enable);
}

void LeftToolWdg::ShowWhiteboardBtn(const bool& bShow/* = true*/){
   if (IsFlashLive()) {
      return;
   }
   bShow ? ui.pushButton_Whiteboard->show() : ui.pushButton_Whiteboard->hide();
}

void LeftToolWdg::ShowDesktopButton(bool show) {
   if (show) {
      ui.DesktopSharingBtn->show();
   }
   else {
      ui.DesktopSharingBtn->hide();
   }
}

void LeftToolWdg::SetDesktopShow(bool show) {
   m_bDesktopShow = show;
}

void LeftToolWdg::ShowMemberButoon(bool show){
   if (IsFlashLive()) {
      return;
   }
   //show ? ui.btnMember->show() : ui.btnMember->hide();
}

void LeftToolWdg::SetDesktopSharing(bool bShow) {
   emit SigSetDesktopSharing(bShow);
}

void LeftToolWdg::SlotSetDesktopSharing(bool bShow) {
   m_bDesktopSharing = bShow;
   //if (!m_pDesktopSharingButton) {
   //	return;
   //}

   //if (m_bDesktopSharing) {
   //	ui.DesktopSharingBtn->changeImage(":/CtrlBtn/img/CtrlButton/desktop_share.png");
   //}
   //else {
   //	ui.DesktopSharingBtn->changeImage(":/CtrlBtn/img/CtrlButton/desktop_share.png");
   //}
}

void LeftToolWdg::SlotDesktopClicked() {
   VH::CComPtr<ICommonData> pCommonData;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
   if (pCommonData) {
      if (pCommonData->GetLiveType() == eLiveType_Live) {
         //STRU_MAINUI_CLICK_CONTROL loControlCamera;
         //loControlCamera.m_eType = control_AddCamera;
         //loControlCamera.m_globalX = 0;
         //loControlCamera.m_globalY = 0;
         //loControlCamera.m_dwExtraData = 0;
         //SingletonMainUIIns::Instance().PostCRMessage(MSG_MAINUI_CLICK_CONTROL, &loControlCamera, sizeof(STRU_MAINUI_CLICK_CONTROL));
         STRU_MAINUI_CLICK_CONTROL loControl;
         loControl.m_eType = control_CaptureSrc;
         QPoint globalPoint = ui.DesktopSharingBtn->mapToGlobal(QPoint(0, 0));
         loControl.m_globalX = globalPoint.x() + ui.DesktopSharingBtn->width();
         loControl.m_globalY = globalPoint.y();
         //m_bDesktopShow = !m_bDesktopShow;
         //if (!m_bDesktopShow) {
         //    return;
         //}
         loControl.m_dwExtraData = 1;
         SingletonMainUIIns::Instance().PostCRMessage(MSG_MAINUI_CLICK_CONTROL, &loControl, sizeof(STRU_MAINUI_CLICK_CONTROL));
      }
   }
   emit sigClicked();
}


bool LeftToolWdg::IsClickedWB() {
   return mbClickedWhiteBoard;
}

void LeftToolWdg::slot_OnClickedWhiteBorad() {
   emit sig_OnClickedWhiteboard();
   ui.pushButton_doc->SetClicked(false);   
   ui.pushButton_doc->SetHoverState(false);

   ui.pushButton_Whiteboard->SetClicked(true);
   ui.pushButton_Whiteboard->SetHoverState(true);
   mbClickedWhiteBoard = true;
   //if (ui.pushButton_wicon->isEnabled()) {
   //   ui.pushButton_wicon->setStyleSheet(WhiteBoardHoverICon);
   //   ui.pushButton_wText->setStyleSheet(WhiteBoardHoverText);
   //}
   //else {
   //   ui.pushButton_wicon->setStyleSheet(WhiteBoardDisableICon);
   //   ui.pushButton_wText->setStyleSheet(WhiteBoardTextDisable);
   //}

}

void LeftToolWdg::slot_OnClickedDoc() {
   emit sig_OnClickedDoc();
   ui.pushButton_doc->SetClicked(true);
   ui.pushButton_doc->SetHoverState(true);

   ui.pushButton_Whiteboard->SetClicked(false);
   ui.pushButton_Whiteboard->SetHoverState(false);

   //if (ui.pushButton_wicon->isEnabled()) {
   //   ui.pushButton_wicon->setStyleSheet(WhiteBoardICon);
   //   ui.pushButton_wText->setStyleSheet(WhiteBoardText);
   //}
   //else {
   //   ui.pushButton_wicon->setStyleSheet(WhiteBoardDisableICon);
   //   ui.pushButton_wText->setStyleSheet(WhiteBoardTextDisable);
   //}
   
   mbClickedWhiteBoard = false;
}

void LeftToolWdg::slot_OnClickedDesktop(bool) {
   emit sig_OnClickedDesktop();
}

void LeftToolWdg::slot_OnClickedPluginUrl() {
   QPoint globalPoint = ui.pushButton_pluginUrl->mapToGlobal(QPoint(0, 0));
   int x = globalPoint.x() + ui.pushButton_pluginUrl->width();
   int y = globalPoint.y();
   emit sig_OnClickedPluginUrl(x, y);
}


bool LeftToolWdg::IsFlashLive() {
   VH::CComPtr<ICommonData> pCommonData;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return false, ASSERT(FALSE));
   if (pCommonData) {
      if (pCommonData->GetStartMode() == eStartMode_flash) {
         return true;
      }
   }
   return false;
}
