#include "stdafx.h"
#include "MultiMediaUI.h"
#include "ToolButton.h"

#include "Msg_MainUI.h"

#include <QPainter>
#include <QPushButton>

MultiMediaUI::MultiMediaUI(QDialog *parent)
: VHDialog(parent) {
   this->setFocusPolicy(Qt::StrongFocus);
   this->setWindowFlags(Qt::FramelessWindowHint | Qt::SplashScreen);
}

MultiMediaUI::~MultiMediaUI() {

}

bool MultiMediaUI::Create() {
   ToolButton*pInsertVedio = new ToolButton(":/CtrlBtn/img/CtrlButton/insertMediaFile.png", this);
   ToolButton*pInsertImage = new ToolButton(":/CtrlBtn/img/CtrlButton/insertImage.png", this);
   ToolButton*pInsertText = new ToolButton(":/CtrlBtn/img/CtrlButton/insertText.png", this);
   m_pVoiceTranslate = new ToolButton(":/CtrlBtn/img/CtrlButton/openVoiceTranslate.png", this);

   connect(pInsertVedio, SIGNAL(sigClicked()), this, SLOT(OnVedioPlayClick()));
   connect(pInsertImage, SIGNAL(sigClicked()), this, SLOT(OnAddImageClick()));
   connect(pInsertText, SIGNAL(sigClicked()), this, SLOT(OnAddTextClick()));
   connect(m_pVoiceTranslate, SIGNAL(sigClicked()), this, SLOT(OnVoiceTranslateClick()));

   m_pMainLayout = new QVBoxLayout();
   m_pMainLayout->addWidget(pInsertVedio);
   m_pMainLayout->addWidget(pInsertImage);
   m_pMainLayout->addWidget(pInsertText);
   m_pMainLayout->addWidget(m_pVoiceTranslate);
   m_pVoiceTranslate->hide();

   setLayout(m_pMainLayout);
   mPixmap.load(":/toolWidget/Popup_bg");
   setFixedSize(mPixmap.width(), mPixmap.height());
   return true;
}

void MultiMediaUI::Destroy() {
   if (NULL != m_pMainLayout) {
      delete m_pMainLayout;
      m_pMainLayout = NULL;
   }
}

void MultiMediaUI::paintEvent(QPaintEvent *) {
   QPainter painter(this);
   painter.drawPixmap(rect(), mPixmap.copy(0, 0, mPixmap.width(), mPixmap.height()));
}

void MultiMediaUI::enterEvent(QEvent *e) {
   QDialog::enterEvent(e);
   m_bEnter = true;
}
void MultiMediaUI::leaveEvent(QEvent *e) {
   QDialog::leaveEvent(e);
   m_bEnter = false;
}

void MultiMediaUI::focusOutEvent(QFocusEvent *e) {
   QDialog::focusOutEvent(e);
   if (!m_bEnter) {
      this->close();
      return;
   }
   QWidget *focusWidget = this->focusWidget();
   bool focusThis = false;

   while (focusWidget) {
      if (focusWidget == this) {
         focusThis = true;
         break;
      }

      focusWidget = focusWidget->parentWidget();
   }

   if (!focusThis) {
      this->close();
      m_bEnter = false;
   } else {
      this->setFocus();
   }
}

void MultiMediaUI::showEvent(QShowEvent *e) {
   setFocus();
   VHDialog::showEvent(e);
}

void MultiMediaUI::OnVedioPlayClick() {
    QJsonObject body;
    SingletonMainUIIns::Instance().reportLog(STR_VH_LIVE_OPERATE, eLogRePortK_MultiMedia_InsertVedio, L"InsertVedio", body);

    STRU_MAINUI_CLICK_CONTROL loControl;
    loControl.m_eType = (enum_control_type)(control_VideoSrc);
    loControl.m_dwExtraData = eLiveType_Live;
    SingletonMainUIIns::Instance().PostCRMessage(MSG_MAINUI_CLICK_CONTROL, &loControl, sizeof(STRU_MAINUI_CLICK_CONTROL));
    TRACE6("%s \n",__FUNCTION__);
}

void MultiMediaUI::OnAddImageClick() {
	QJsonObject body;
	SingletonMainUIIns::Instance().reportLog(STR_VH_LIVE_OPERATE, eLogRePortK_MultiMedia_InsertImage, L"InsertImage", body);

   STRU_MAINUI_CLICK_CONTROL loControl;
   loControl.m_eType = (enum_control_type)(control_AddImage);
   loControl.m_dwExtraData = 1;            //1Ìí¼Ó 2ÐÞ¸Ä
   SingletonMainUIIns::Instance().PostCRMessage(
      MSG_MAINUI_CLICK_CONTROL, &loControl, sizeof(STRU_MAINUI_CLICK_CONTROL));
   TRACE6("%s \n", __FUNCTION__);
}

void MultiMediaUI::OnAddTextClick() {
	QJsonObject body;
	SingletonMainUIIns::Instance().reportLog(STR_VH_LIVE_OPERATE, eLogRePortK_MultiMedia_InsertText, L"InsertText", body);

   STRU_MAINUI_CLICK_CONTROL loControl;
   loControl.m_eType = (enum_control_type)(control_AddText);
   SingletonMainUIIns::Instance().PostCRMessage(
      MSG_MAINUI_CLICK_CONTROL, &loControl, sizeof(STRU_MAINUI_CLICK_CONTROL));
   TRACE6("%s \n", __FUNCTION__);
}

void MultiMediaUI::OnVoiceTranslateClick() {
   VH::CComPtr<IMainUILogic> pMainUILogic;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IMainUI, IID_IMainUILogic, pMainUILogic, return, ASSERT(FALSE));
   if (pMainUILogic && pMainUILogic->GetVoiceTranslate() && pMainUILogic->IsOpenVoiceTranslateFunc()) {
      pMainUILogic->OpenVoiceTranslateFun(false);
      m_pVoiceTranslate->changeImage(":/CtrlBtn/img/CtrlButton/openVoiceTranslate.png");
   } else if (pMainUILogic && pMainUILogic->GetVoiceTranslate() && !pMainUILogic->IsOpenVoiceTranslateFunc()) {
      m_pVoiceTranslate->changeImage(":/CtrlBtn/img/CtrlButton/closeVoiceTranslate.png");
      pMainUILogic->OpenVoiceTranslateFun(true);
   }

   STRU_MAINUI_CLICK_CONTROL loControl;
   loControl.m_eType = (enum_control_type)(control_VoiceTranslate);
   SingletonMainUIIns::Instance().PostCRMessage(MSG_MAINUI_CLICK_CONTROL, &loControl, sizeof(STRU_MAINUI_CLICK_CONTROL));
}

void MultiMediaUI::AddVoiceTranslate() {
   if (m_pVoiceTranslate != NULL) {
      m_pVoiceTranslate->changeImage(":/CtrlBtn/img/CtrlButton/openVoiceTranslate.png");
      m_pVoiceTranslate->show();
   }
}

void MultiMediaUI::RemoveVoiceTranslate() {
   if (m_pVoiceTranslate != NULL) {
      m_pVoiceTranslate->changeImage(":/CtrlBtn/img/CtrlButton/openVoiceTranslate.png");
      m_pVoiceTranslate->hide();
   }
}


