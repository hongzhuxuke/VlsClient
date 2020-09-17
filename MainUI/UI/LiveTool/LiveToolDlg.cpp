#include "stdafx.h"
#include "LiveToolDlg.h"
#include "ICommonData.h"
#include "VH_Macro.h"
#include "ButtonWdg.h"

#define BORDERCOLOR "border-color:rgba(67,67,67,1);"

#define LuckNormal "border-image:url(:/button/images/luck.png);"
#define LuckClicked "border-image:url(:/button/images/luck_hover.png);"

#define RedPacket "border-image:url(:/button/images/red_packet.png);"
#define RedPacketClicked "border-image:url(:/button/images/red_packet_hover.png);"

#define SignNormal "border-image:url(:/button/images/sign.png);"
#define SignClicked "border-image:url(:/button/images/sign_hover.png);"

#define QuestionnaireNormal "border-image:url(:/button/images/questionnaire.png);"
#define QuestionnaireClicked "border-image:url(:/button/images/questionnaire_hover.png);"

#define QuestionNormal "border-image:url(:/button/images/questions.png);"
#define QuestionClicked "border-image:url(:/button/images/questions_hover.png);"

LiveToolDlg::LiveToolDlg(QDialog *parent /*= 0*/)
   : VHDialog(parent) {
   ui.setupUi(this);
   this->setFocusPolicy(Qt::StrongFocus);
   this->setWindowFlags(Qt::FramelessWindowHint | Qt::SplashScreen);
   setAttribute(Qt::WA_TranslucentBackground);
}

LiveToolDlg::~LiveToolDlg() {

}

bool LiveToolDlg::Create() {
   VH::CComPtr<ICommonData> pCommonData;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return false, ASSERT(FALSE));
   ClientApiInitResp initApiResp;
   pCommonData->GetInitApiResp(initApiResp);

   mOpIdList.clear();
   std::list<QString>::iterator iter = initApiResp.permission_data.begin();
   QString strTemp = "";
   int iOpType = 0;
   for (; iter != initApiResp.permission_data.end(); iter++)
   {
      strTemp = *iter;
      iOpType = strTemp.trimmed().toInt();
      switch (iOpType)
      {
      case eWebOpType_questionnaire:
         mOpIdList.append(eOpType_questionnaire);
         break;
      case eWebOpType_Questions:
         mOpIdList.append(eOpType_Questions);
         break;
      case eWebOpType_Luck:
         mOpIdList.append(eOpType_Luck);
         break;
      case eWebOpType_envelopes:
         mOpIdList.append(eOpType_envelopes);
         break;
      case eWebOpType_Signin:
         mOpIdList.append(eOpType_Signin);
         break;
      default:
         break;
      }
      strTemp = "";
   }

   BtnMenuItem* pItem = nullptr;
   for (size_t i = 0; i < mOpIdList.size(); i++){
      iOpType = mOpIdList.at(i);
      pItem = nullptr;
      if (iOpType <= eOpType_envelopes && eOpType_Questions <= iOpType){
         pItem = new BtnMenuItem(this);
         switch (iOpType)
         {
         case eOpType_questionnaire:// 问卷
            pItem->Create(ButtonWdg::eFontDirection_Underside, 0, 12, 18, QuestionnaireNormal, QuestionnaireClicked, QuestionnaireNormal);
            pItem->setIconAlignment(Qt::AlignHCenter | Qt::AlignBottom);
            pItem->setFontText(tr("问卷"), Qt::AlignHCenter | Qt::AlignTop);
            break;
         case eOpType_Questions://  问答
            pItem->Create(ButtonWdg::eFontDirection_Underside, 0, 12, 18, QuestionNormal, QuestionClicked, QuestionNormal);
            pItem->setIconAlignment(Qt::AlignHCenter | Qt::AlignBottom);
            pItem->setFontText(tr("问答"), Qt::AlignHCenter | Qt::AlignTop);
            break;
         case eOpType_Luck:
            pItem->Create(ButtonWdg::eFontDirection_Underside, 0, 12, 18, LuckNormal, LuckClicked, LuckNormal);
            pItem->setIconAlignment(Qt::AlignHCenter | Qt::AlignBottom);
            pItem->setFontText(tr("抽奖"), Qt::AlignHCenter | Qt::AlignTop);
            break;
         case eOpType_envelopes:
            pItem->Create(ButtonWdg::eFontDirection_Underside, 0, 12, 18, RedPacket, RedPacketClicked, RedPacket);
            pItem->setIconAlignment(Qt::AlignHCenter | Qt::AlignBottom);
            pItem->setFontText(tr("红包"), Qt::AlignHCenter | Qt::AlignTop);
            break;
         case eOpType_Signin:
            pItem->Create(ButtonWdg::eFontDirection_Underside, 0, 12, 18, SignNormal, SignClicked, SignNormal);
            pItem->setIconAlignment(Qt::AlignHCenter | Qt::AlignBottom);
            pItem->setFontText(tr("签到"), Qt::AlignHCenter | Qt::AlignTop);
            break;
         default:
            break;
         }
         pItem->setIconTextSize(20, 20);
         pItem->setBorderColor(BORDERCOLOR, BORDERCOLOR);
         pItem->SetOpType(iOpType);
         connect(pItem, &BtnMenuItem::Clicked, this, &LiveToolDlg::sigClicked);
         connect(pItem, &BtnMenuItem::btnClicked, this, &LiveToolDlg::close);
         mListItemBtn.append(pItem);
         addLayOut(pItem, i);
      }
   }

   int iNum = mOpIdList.size();
   if (mOpIdList.size() == 1){
      this->resize(104, 77);
   }
   else if (mOpIdList.size() == 2){
      this->resize(172, 77);
   }
   else if (mOpIdList.size() == 3){
      this->resize(238, 77);
   }
   else if (mOpIdList.size() == 4){
      this->resize(304, 132);
   }
   else{
      iNum = 6;
      this->resize(370, 132);
   }
   QString strQss = QString("#ModelFrame{border-image: url(:/toolTip/img/toolTip/LiveToolDlg%1.png);}").arg(iNum);
   this->setStyleSheet(strQss);
   return true;
}

void LiveToolDlg::addLayOut(BtnMenuItem*  pItem, const int& index){
   int iR = index / 3;
   int iC = index % 3;
   ui.gridLayout->addWidget(pItem, iR, iC, 1, 1);
}

void LiveToolDlg::Destroy() {
}

void LiveToolDlg::showEvent(QShowEvent *e) {
   setFocus();
   VHDialog::showEvent(e);
}

void LiveToolDlg::enterEvent(QEvent *e) {
   QDialog::enterEvent(e);
   m_bEnter = true;
}

void LiveToolDlg::leaveEvent(QEvent *e) {
   QDialog::leaveEvent(e);
   m_bEnter = false;
   close();
}

void LiveToolDlg::focusOutEvent(QFocusEvent *e) {
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
   }
   else {
      this->setFocus();
   }
}


/////////////////////////////////////////////////

LiveToolWdg::LiveToolWdg(QWidget *parent /*= 0*/)
   : CWidget(parent) {
   ui.setupUi(this);
   //this->setFocusPolicy(Qt::StrongFocus);
   //this->setWindowFlags(Qt::FramelessWindowHint | Qt::SplashScreen);
   //setAttribute(Qt::WA_TranslucentBackground);
}

LiveToolWdg::~LiveToolWdg() {

}

bool LiveToolWdg::Create() {
   VH::CComPtr<ICommonData> pCommonData;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return false, ASSERT(FALSE));
   ClientApiInitResp initApiResp;
   pCommonData->GetInitApiResp(initApiResp);
   mOpIdList.clear();
   for (int i = 0; i < mListItemBtn.count(); i++) {
      BtnMenuItem* item = mListItemBtn.at(i);
      ui.verticalLayout_2->removeWidget(item);
      delete item;
   }
   mListItemBtn.clear();

   std::list<QString>::iterator iter = initApiResp.permission_data.begin();
   QString strTemp = "";
   int iOpType = 0;
   for (; iter != initApiResp.permission_data.end(); iter++)
   {
      strTemp = *iter;
      iOpType = strTemp.trimmed().toInt();
      switch (iOpType)
      {
      case eWebOpType_questionnaire:
         mOpIdList.append(eOpType_questionnaire);
         break;
      case eWebOpType_Questions:
         mOpIdList.append(eOpType_Questions);
         break;
      case eWebOpType_Luck:
         mOpIdList.append(eOpType_Luck);
         break;
      case eWebOpType_envelopes:
         mOpIdList.append(eOpType_envelopes);
         break;
      case eWebOpType_Signin:
         mOpIdList.append(eOpType_Signin);
         break;
      default:
         break;
      }
      strTemp = "";
   }

   BtnMenuItem* pItem = nullptr;
   for (size_t i = 0; i < mOpIdList.size(); i++) {
      iOpType = mOpIdList.at(i);
      pItem = nullptr;
      if (iOpType <= eOpType_envelopes && eOpType_Questions <= iOpType){
         pItem = new BtnMenuItem(this);
         //pItem->setFixedSize(QSize(60, 60));
         switch (iOpType)
         {
         case eOpType_questionnaire:// 问卷
            pItem->Create(ButtonWdg::eFontDirection_Underside, 8, 12, 18, QuestionnaireNormal, QuestionnaireClicked, QuestionnaireNormal);
            pItem->setIconAlignment(Qt::AlignHCenter | Qt::AlignBottom);
            pItem->setFontText(tr("问卷"), Qt::AlignHCenter | Qt::AlignTop);
            break;
         case eOpType_Questions://  问答
            pItem->Create(ButtonWdg::eFontDirection_Underside, 8, 12, 18, QuestionNormal, QuestionClicked, QuestionNormal);
            pItem->setIconAlignment(Qt::AlignHCenter | Qt::AlignBottom);
            pItem->setFontText(tr("问答"), Qt::AlignHCenter | Qt::AlignTop);
            break;
         case eOpType_Luck:
            pItem->Create(ButtonWdg::eFontDirection_Underside, 8, 12, 18, LuckNormal, LuckClicked, LuckNormal);
            pItem->setIconAlignment(Qt::AlignHCenter | Qt::AlignBottom);
            pItem->setFontText(tr("抽奖"), Qt::AlignHCenter | Qt::AlignTop);
            break;
         case eOpType_envelopes:
            pItem->Create(ButtonWdg::eFontDirection_Underside, 8, 12, 18, RedPacket, RedPacketClicked, RedPacket);
            pItem->setIconAlignment(Qt::AlignHCenter | Qt::AlignBottom);
            pItem->setFontText(tr("红包"), Qt::AlignHCenter | Qt::AlignTop);
            break;
         case eOpType_Signin:
            pItem->Create(ButtonWdg::eFontDirection_Underside, 8, 12, 18, SignNormal, SignClicked, SignNormal);
            pItem->setIconAlignment(Qt::AlignHCenter | Qt::AlignBottom);
            pItem->setFontText(tr("签到"), Qt::AlignHCenter | Qt::AlignTop);
            break;
         default:
            break;
         }
         //pItem->Create(ButtonWdg::eFontDirection_Underside, 8, 12, 18);
         pItem->setIconTextSize(20, 20);
         pItem->setBorderColor(BORDERCOLOR, BORDERCOLOR);
         pItem->SetOpType(iOpType);
         connect(pItem, &BtnMenuItem::Clicked, this, &LiveToolWdg::sigClicked);
         mListItemBtn.append(pItem);
         addLayOut(pItem);
      }
   }
   return true;
}

void LiveToolWdg::addLayOut(BtnMenuItem*  pItem){
   ui.verticalLayout_2->addWidget(pItem);
}

void LiveToolWdg::Destroy() {
}

