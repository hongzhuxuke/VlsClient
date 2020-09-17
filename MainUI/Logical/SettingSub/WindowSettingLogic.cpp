#include "StdAfx.h"
#include "WindowSettingLogic.h"
#include "WindowSettingDlg.h"

#include "IDeviceManager.h"
#include "ICommonData.h"
#include "ConfigSetting.h"

#include <Psapi.h>
#include <QBuffer>
#include <QFileDialog>
#include <QObject>
#include <QScreen>
#include "Msg_MainUI.h"
#include "Msg_OBSControl.h"
#include "IOBSControlLogic.h"
#include "VHMonitorCapture.h"

WindowSettingLogic::WindowSettingLogic(void)
: m_pWindowSettingDlg(NULL) {

}

WindowSettingLogic::~WindowSettingLogic(void) {

}

BOOL WindowSettingLogic::Create() {
   do {
      VH::CComPtr<IMainUILogic> pMainUILogic;
      DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IMainUI, IID_IMainUILogic, pMainUILogic, return FALSE, ASSERT(FALSE));

      QWidget* pMainUI = NULL;
      pMainUILogic->GetMainUIWidget((void**)&pMainUI);
      //ѡ�����Դ���ڴ���
      m_pWindowSettingDlg = new WindowSettingDlg((QDialog*)pMainUI);
      if (NULL == m_pWindowSettingDlg || !m_pWindowSettingDlg->Create()) {
         ASSERT(FALSE);
         break;
      } 
      m_pImageSettingDlg = new ImageSettingDlg((QDialog*)pMainUI);
      if (NULL == m_pImageSettingDlg || !m_pImageSettingDlg->Create()) {
         ASSERT(FALSE);
         break;
      }
      return TRUE;
   } while (FALSE);
   return FALSE;
}

void WindowSettingLogic::Destroy() {
   //����������ִ���
   //if (NULL != m_pWindowSettingDlg) {
   //   m_pWindowSettingDlg->Destroy();
   //   delete m_pWindowSettingDlg;
   //   m_pWindowSettingDlg = NULL;
   //}
}

void WindowSettingLogic::DealMessage(DWORD adwMessageID, void* apData, DWORD adwLen) {

   //desktop Exclude others
   VH::CComPtr<IOBSControlLogic> pObsControlLogic = NULL;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IOBSControl, IID_IOBSControlLogic, pObsControlLogic, return, ASSERT(FALSE));
   bool isHasMonitor=pObsControlLogic->IsHasSource(SRC_MONITOR);
   if(isHasMonitor) {
      return ;
   }

   switch (adwMessageID) {
      //CLICK����
   case MSG_MAINUI_CLICK_CONTROL:
      DealClickControl(apData, adwLen);
      break;

      //������Դ
   case MSG_MAINUI_CLICK_ADD_WNDSRC:
      DealAddWindowSrc(apData, adwLen);
      break;

      //�޸�ͼƬ
   case MSG_MAINUI_MODIFY_IMAGE:
      DealModifyImage(apData, adwLen);
      break;
   default:
      break;
   }
}

void WindowSettingLogic::DealClickControl(void* apData, DWORD adwLen) {
   DEF_CR_MESSAGE_DATA_DECLARE_CA(STRU_MAINUI_CLICK_CONTROL, loMessage, return);
   switch (loMessage.m_eType) {
      //���Դ
   case control_WindowSrc:
      AddWindowSrc();
      TRACE6("[BP] WindowSettingLogic::DealClickControl control_WindowSrc\n");
      break;

      //���ͼƬ
   case control_AddImage:
      ProcessImages(loMessage.m_dwExtraData);
      
      TRACE6("[BP] WindowSettingLogic::DealClickControl control_AddImage\n");
      break;

   default:
      break;
   }
}

void WindowSettingLogic::DealModifyImage(void* apData, DWORD adwLen) {

   DEF_CR_MESSAGE_DATA_DECLARE_CA(STRU_OBSCONTROL_IMAGE, loMessage, return);

   int dwType = loMessage.m_dwType;
   float fx = loMessage.x;
   float fy = loMessage.y;
   float fw = loMessage.w;
   float fh = loMessage.h;
   QString fileName = QString::fromWCharArray(loMessage.m_strPath);

   VH::CComPtr<IMainUILogic> pMainUILogic;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IMainUI, IID_IMainUILogic, pMainUILogic, return, ASSERT(FALSE));

   VH::CComPtr<IOBSControlLogic> pObsControlLogic = NULL;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IOBSControl, IID_IOBSControlLogic, pObsControlLogic, return, ASSERT(FALSE));

   QWidget* pMainUI = NULL;
   pMainUILogic->GetMainUIWidget((void**)&pMainUI);

   //1��� 2�޸�
   m_pImageSettingDlg->SetCurType(dwType);
   m_pImageSettingDlg->setModal(true);

   QPixmap basePixmap = QPixmap::grabWidget(pMainUI);
   m_pImageSettingDlg->SetBasePixmap(basePixmap);

   QWidget *pContentWidget = NULL;
   pMainUILogic->GetContentWidget((void **)&pContentWidget);
   int contentWidth;
   int contentHeight;
   pObsControlLogic->GetCurrentFramePicSize(contentWidth, contentHeight);
   int baseWidth;
   int baseHeight;
   pObsControlLogic->GetBaseSize(baseWidth, baseHeight);
   bool setVisibleRet = pObsControlLogic->SetSourceVisible(loMessage.m_strSourceName, false, true);
   if (setVisibleRet) {
      pObsControlLogic->WaitSetSourceVisible();
   }


   unsigned long long t ;
   unsigned char **centerImgBuf = pObsControlLogic->LockCurrentFramePic(t);
   m_pImageSettingDlg->SetCerterImg(pContentWidget->x() + 63, pContentWidget->y() + 40, pContentWidget->width(), pContentWidget->height(), contentWidth, contentHeight, centerImgBuf, baseWidth, baseHeight);
   pObsControlLogic->UnlockCurrentFramePic();

   pObsControlLogic->SetSourceVisible(loMessage.m_strSourceName, true, false);

   if (fileName.isNull()) {
      return;
   }

   m_pImageSettingDlg->SetImage(fx, fy, fw, fh, fileName);
   m_pImageSettingDlg->show();


}
void WindowSettingLogic::DealAddWindowSrc(void* apData, DWORD adwLen) {
   if(!m_pWindowSettingDlg) {
      return ;
   }
   
   STRU_OBSCONTROL_WINDOWSRC  WindowSrc;
   QVariant data;
   m_pWindowSettingDlg->GetCurItem(data);
   WindowSrc.m_windowInfo=*(VHD_WindowInfo *)data.toByteArray().data();
   WindowSrc.m_windowInfo.isCompatible=m_pWindowSettingDlg->GetComptible();
      
   VHD_ActiveWindow(WindowSrc.m_windowInfo);
   GetWindowRect(WindowSrc.m_windowInfo.hwnd,&WindowSrc.m_windowInfo.rect);
   if(WindowSrc.m_windowInfo.rect.right > 0 
      && WindowSrc.m_windowInfo.rect.bottom > 0
         && WindowSrc.m_windowInfo.rect.right - WindowSrc.m_windowInfo.rect.left > 0
            &&WindowSrc.m_windowInfo.rect.bottom - WindowSrc.m_windowInfo.rect.top > 0 ) {
      SingletonMainUIIns::Instance().PostCRMessage(MSG_OBSCONTROL_WINDOWSRC, &WindowSrc, sizeof(WindowSrc));
      VH::CComPtr<ICommonData> pCommonData;
      DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
      pCommonData->ReportEvent("rjys01");
   }
}

void WindowSettingLogic::AddWindowSrc() {
   if (NULL != m_pWindowSettingDlg) {
      //��ʾǰ�����֮ǰ���б���
      m_pWindowSettingDlg->Clear();
      
      VHD_Window_Enum_init(VHD_Window);
      VHD_WindowInfo windowInfo;

      while(VHD_Window_Enum_update(&windowInfo)){ 
         RECT rect;
         GetWindowRect(windowInfo.hwnd, &rect);

         qDebug()<<"WindowSettingLogic::AddWindowSrc()"
            <<QString::fromWCharArray(windowInfo.name)
            <<windowInfo.hwnd
            <<rect.left
            <<rect.top
            <<rect.right
            <<rect.bottom
         ;
         m_pWindowSettingDlg->AddDeviceItem(windowInfo.name,QVariant(QByteArray((const char *)&windowInfo,sizeof(VHD_WindowInfo))));
      }
      VHD_Window_Enum_final();      
      
      m_pWindowSettingDlg->CenterWindow((QWidget*)m_pWindowSettingDlg->parent());
      m_pWindowSettingDlg->setModal(true);
      m_pWindowSettingDlg->show();
   }
}
//���ͼƬ
void WindowSettingLogic::ProcessImages(DWORD dwType) {

   VH::CComPtr<IMainUILogic> pMainUILogic;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IMainUI, IID_IMainUILogic, pMainUILogic, return, ASSERT(FALSE));

   VH::CComPtr<IOBSControlLogic> pObsControlLogic = NULL;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IOBSControl, IID_IOBSControlLogic, pObsControlLogic, return, ASSERT(FALSE));

   QWidget* pMainUI = NULL;
   pMainUILogic->GetMainUIWidget((void**)&pMainUI);

   //1��� 2�޸�
   m_pImageSettingDlg->SetCurType(dwType);
   //m_pImageSettingDlg->setModal(true);

   QPixmap basePixmap = QPixmap::grabWidget(pMainUI);
   m_pImageSettingDlg->SetBasePixmap(basePixmap);

   int x = 0, y = 0;
   QWidget *pContentWidget = NULL;
   pMainUILogic->GetContentWidget((void **)&pContentWidget);
   pMainUILogic->GetRenderPos(x,y);
   int contentWidth;
   int contentHeight;
   pObsControlLogic->GetCurrentFramePicSize(contentWidth, contentHeight);
   int baseWidth;
   int baseHeight;
   pObsControlLogic->GetBaseSize(baseWidth, baseHeight);
   QPoint pos = pContentWidget->pos();
   unsigned long long t=0;
   unsigned char **centerImgBuf = pObsControlLogic->LockCurrentFramePic(t);
   m_pImageSettingDlg->SetCerterImg(x, y, pContentWidget->width(), pContentWidget->height(), contentWidth, contentHeight, centerImgBuf, baseWidth, baseHeight);
   pObsControlLogic->UnlockCurrentFramePic();

   if (dwType == 1) {
      VH::CComPtr<IVedioPlayLogic> pVedioPlayLogic;
      DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IMainUI, IID_IVedioPlayLogic, pVedioPlayLogic, return);
      pVedioPlayLogic->ForceHide(true);
      //QString fileName = QFileDialog::getOpenFileName(QWidget::find((WId)pMainUI->winId()), QObject::tr("���ļ�"), "", QObject::tr("ͼ���ļ�(*.jpg *.png *.bmp);;Allfile(*.*)"), 0, 0);
      
      QString selectFileName;// = QFileDialog::getOpenFileName(NULL, QStringLiteral("���ļ�"), "", QStringLiteral("�ļ�(*.jpg *.JPG *.png *.PNG);"), 0, 0);
      TCHAR szPathName[2048];
      // lStructSize
      // ָ������ṹ�Ĵ�С�����ֽ�Ϊ��λ��
      // Windows 95/98��Windows NT 4.0������ΪWindows 95/98��Windows NT 4.0��������WINVER��_WIN32_WINNT >= 0x0500����ʱ��
      //  Ϊ�����Աʹ��OPENFILENAME_SIZE_VERSION_400��
      // Windows 2000�����߰汾���������ʹ��sizeof (OPENFILENAME)��
      OPENFILENAME ofn = { sizeof(OPENFILENAME) };
      ofn.hwndOwner = NULL;// ��OR�����ļ��Ի���ĸ�����
      //������ ���Ϊ NULL ��ʹ�ù�����
      //�����÷�������  ע�� /0
      lstrcpy(szPathName, TEXT(""));
      ofn.lpstrFilter = TEXT("ͼ���ļ�(*.jpg *.png *.bmp)\0*.jpg;*.png;*.bmp\\0");
      ofn.lpstrFile = szPathName;
      ofn.nMaxFile = sizeof(szPathName);//����û�ѡ���ļ��� ·�����ļ��� ������
      ofn.lpstrTitle = TEXT("ѡ���ļ�");//ѡ���ļ��Ի������
      ofn.Flags = OFN_EXPLORER;//�����Ҫѡ�����ļ� ��������  OFN_ALLOWMULTISELECT��־
      BOOL bOk = GetOpenFileName(&ofn);//���öԻ�����ļ�
      TRACE6("%s GetOpenFileName\n", __FUNCTION__);
      if (bOk) {
         selectFileName = QString::fromStdWString(szPathName);
      }


      pVedioPlayLogic->ForceHide(false);
      if (selectFileName.isNull()) {
         return;
      }

#define MAXSIZE 2000
      QString desFile = QDir::tempPath() + "/vhallTempPic.png";
      QImageReader imageReader;
      imageReader.setFileName(selectFileName);
      QSize imageSize = imageReader.size();
      if (imageSize.width() > MAXSIZE || imageSize.height() > MAXSIZE) {
         if (imageSize.width() > 0 && imageSize.height() > 0) {
            int w = imageSize.width();
            int h = imageSize.height();
            if (w > h) {
               w = MAXSIZE;
               h = imageSize.height() * 2000 / imageSize.width();
            } else {
               h = MAXSIZE;
               w = imageSize.width() * 2000 / imageSize.height();
            }
            imageSize.setWidth(w);
            imageSize.setHeight(h);
            
            imageReader.setScaledSize(imageSize);
            QImage img = imageReader.read(); // ��ȡͼƬ

            if (img.save(desFile)) {
               selectFileName = desFile;
            } else {
               return;
            }
         }
      }

      m_pImageSettingDlg->SetImage(0, 0, 0, 0, selectFileName);
      m_pImageSettingDlg->CenterWindow((QWidget*)m_pImageSettingDlg->parent());
      m_pImageSettingDlg->show();
   }

}
void WindowSettingLogic::slotReFlush() {
   AddWindowSrc();
}

