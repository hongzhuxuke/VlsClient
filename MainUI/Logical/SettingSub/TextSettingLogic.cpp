#include "StdAfx.h"
#include "TextSettingLogic.h"
#include "TextSettingDlg.h"

#include "Msg_MainUI.h"
#include "Msg_OBSControl.h"
#include "IOBSControlLogic.h"
#include <QImage>
#include <QDebug>
TextSettingLogic::TextSettingLogic(void)
: m_pTextSettingDlg(NULL) {

}

TextSettingLogic::~TextSettingLogic(void) {

}

BOOL TextSettingLogic::Create() {
   do {
      return TRUE;
   } while (FALSE);
   return FALSE;
}

void TextSettingLogic::Destroy() {
   //销毁添加文字窗口
   //if (NULL != m_pTextSettingDlg) {
   //   m_pTextSettingDlg->Destroy();
   //   delete m_pTextSettingDlg;
   //   m_pTextSettingDlg = NULL;
   //}
}

void TextSettingLogic::CreateTextUI() {
    VH::CComPtr<IMainUILogic> pMainUILogic;
    DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IMainUI, IID_IMainUILogic, pMainUILogic, return, ASSERT(FALSE));

    QWidget* pMainUI = NULL;
    pMainUILogic->GetMainUIWidget((void**)&pMainUI);
    //添加文字窗口创建
    if (m_pTextSettingDlg == nullptr) {
       m_pTextSettingDlg = new TextSettingDlg((QDialog*)pMainUI);
       if (NULL == m_pTextSettingDlg) {
          ASSERT(FALSE);
          return;
       }
       if (!m_pTextSettingDlg->Create()) {
          return;
       }
    }
}

void TextSettingLogic::DealMessage(DWORD adwMessageID, void* apData, DWORD adwLen) {
   //desktop Exclude others
   VH::CComPtr<IOBSControlLogic> pObsControlLogic = NULL;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IOBSControl, IID_IOBSControlLogic, pObsControlLogic, return, ASSERT(FALSE));
   bool isHasMonitor=pObsControlLogic->IsHasSource(SRC_MONITOR);
   if(isHasMonitor) {
      return ;
   }

   switch (adwMessageID) {
      //点击控制
   case MSG_MAINUI_CLICK_CONTROL:
      DealClickControl(apData, adwLen);
      break;

      //修改文本
   case MSG_MAINUI_MODIFY_TEXT:
      DealModifyText(apData, adwLen);
      break;

   default:
      break;
   }
}

void TextSettingLogic::DealClickControl(void* apData, DWORD adwLen) {
   DEF_CR_MESSAGE_DATA_DECLARE_CA(STRU_MAINUI_CLICK_CONTROL, loMessage, return);
   switch (loMessage.m_eType) {
      //添加文字
   case control_AddText:
      AddText();
      break;

   default:
      break;
   }
}

//修改文字
void TextSettingLogic::DealModifyText(void* apData, DWORD adwLen) {
   DEF_CR_MESSAGE_DATA_DECLARE_CA(STRU_OBSCONTROL_TEXT, loMessage, return);
   CreateTextUI();
   if (NULL != m_pTextSettingDlg) {
      VH::CComPtr<IMainUILogic> pMainUILogic;
      DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IMainUI, IID_IMainUILogic, pMainUILogic, return , ASSERT(FALSE));
      
      QWidget* pMainUI = NULL;
      pMainUILogic->GetMainUIWidget((void**)&pMainUI);
      QPixmap basePixmap=QPixmap::grabWidget(pMainUI);
      m_pTextSettingDlg->SetBasePixmap(basePixmap);
      
      QWidget *pContentWidget=NULL;
      pMainUILogic->GetContentWidget((void **)&pContentWidget);      
      VH::CComPtr<IOBSControlLogic> pObsControlLogic=NULL;
      DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IOBSControl, IID_IOBSControlLogic, pObsControlLogic, return, ASSERT(FALSE));
      int contentWidth;
      int contentHeight;
      pObsControlLogic->GetCurrentFramePicSize(contentWidth,contentHeight);
      int baseWidth;
      int baseHeight;
      
      pObsControlLogic->GetBaseSize(baseWidth,baseHeight);
      
      bool setVisibleRet=pObsControlLogic->SetSourceVisible(loMessage.m_strSourceName,false,true);
      if(setVisibleRet)
      {
         pObsControlLogic->WaitSetSourceVisible();
      }
      unsigned long long t = 0;
      unsigned char **centerImgBuf=pObsControlLogic->LockCurrentFramePic(t);
      m_pTextSettingDlg->SetCerterImg(61, 102,pContentWidget->width(),pContentWidget->height(),contentWidth,contentHeight,centerImgBuf,baseWidth,baseHeight);
      pObsControlLogic->UnlockCurrentFramePic();
      pObsControlLogic->SetSourceVisible(loMessage.m_strSourceName,true,false);


      m_pTextSettingDlg->SetTextRect(loMessage.m_ix,
         loMessage.m_iy,
         loMessage.m_iw,
         loMessage.m_ih);


      m_pTextSettingDlg->SetCurType(loMessage.m_iControlType);
      m_pTextSettingDlg->SetTextFormat(&loMessage);


      m_pTextSettingDlg->show();
   }
}

void TextSettingLogic::AddText() {
    CreateTextUI();
    if (NULL != m_pTextSettingDlg) {
        m_pTextSettingDlg->SetCurType(1);
        m_pTextSettingDlg->setModal(true);
      
        VH::CComPtr<IMainUILogic> pMainUILogic;
        DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IMainUI, IID_IMainUILogic, pMainUILogic, return , ASSERT(FALSE));
      
        QWidget* pMainUI = NULL;
        pMainUILogic->GetMainUIWidget((void**)&pMainUI);
        QPixmap basePixmap=QPixmap::grabWidget(pMainUI);
        m_pTextSettingDlg->SetBasePixmap(basePixmap);
      
        QWidget *pContentWidget=NULL;
        pMainUILogic->GetContentWidget((void **)&pContentWidget);      
        VH::CComPtr<IOBSControlLogic> pObsControlLogic=NULL;
        DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IOBSControl, IID_IOBSControlLogic, pObsControlLogic, return, ASSERT(FALSE));
        int contentWidth;
        int contentHeight;
        pObsControlLogic->GetCurrentFramePicSize(contentWidth,contentHeight);

        int baseWidth;
        int baseHeight;
        pObsControlLogic->GetBaseSize(baseWidth,baseHeight);
        unsigned long long t;
        unsigned char **centerImgBuf=pObsControlLogic->LockCurrentFramePic(t);
        m_pTextSettingDlg->SetCerterImg(61,102,pContentWidget->width(),pContentWidget->height(),contentWidth,contentHeight,centerImgBuf,baseWidth,baseHeight);
        pObsControlLogic->UnlockCurrentFramePic();
      
        m_pTextSettingDlg->SetTextRect(0,0,0,0);
        m_pTextSettingDlg->CenterWindow((QWidget*)m_pTextSettingDlg->parent());

        m_pTextSettingDlg->show();
    }
}
