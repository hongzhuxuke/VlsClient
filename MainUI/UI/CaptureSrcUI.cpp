#include "stdafx.h"
#include "CaptureSrcUI.h"
#include "ToolButton.h"

#include "Msg_MainUI.h"
#include "ICommonData.h"
#include "ButtonWdg.h"

#include <QPainter>
#include <QPushButton>

#define BORDERCOLOR "border-color:rgba(67,67,67,1);"

#define DesktopNormal "border-image:url(:/button/images/desktop_select.png);"
#define DesktopClicked "border-image:url(:/button/images/desktop_select_hover.png);"

#define SoftNormal "border-image:url(:/button/images/soft_share.png);"
#define SoftClicked "border-image:url(:/button/images/soft_share_hover.png);"

#define AreaNormal "border-image:url(:/button/images/area_share.png);"
#define AreaClicked "border-image:url(:/button/images/area_share_hover.png);"

CaptureSrcUI::CaptureSrcUI(QDialog *parent)
: VHDialog(parent) {
	ui.setupUi(this);
   this->setFocusPolicy(Qt::StrongFocus);
   this->setWindowFlags(Qt::FramelessWindowHint | Qt::SplashScreen);
   setAttribute(Qt::WA_TranslucentBackground);
   this->setFixedSize(238, 77);
}

CaptureSrcUI::~CaptureSrcUI() {

}
void CaptureSrcUI::SetDesktopShareStatus(bool bShared){
   emit this->SigSetDesktopShareStatus(bShared);
}

bool CaptureSrcUI::Create() {
   ui.DesktopBtnWdg->Create(ButtonWdg::eFontDirection_Underside, 8, 12,18, DesktopNormal, DesktopClicked, DesktopNormal);
   ui.SoftWareBtnWdg->Create(ButtonWdg::eFontDirection_Underside, 8, 12, 18, SoftNormal, SoftClicked, SoftNormal);
   ui.RegionBtnWdg->Create(ButtonWdg::eFontDirection_Underside, 8, 12, 20, AreaNormal, AreaClicked, AreaNormal);

   ui.DesktopBtnWdg->setIconTextSize(22, 22);
   ui.SoftWareBtnWdg->setIconTextSize(22, 22);
   ui.RegionBtnWdg->setIconTextSize(22, 22);

   ui.DesktopBtnWdg->setBorderColor(BORDERCOLOR, BORDERCOLOR);
   ui.SoftWareBtnWdg->setBorderColor(BORDERCOLOR, BORDERCOLOR);
   ui.RegionBtnWdg->setBorderColor(BORDERCOLOR, BORDERCOLOR);

   ui.DesktopBtnWdg->setIconAlignment(Qt::AlignHCenter | Qt::AlignBottom);
   ui.SoftWareBtnWdg->setIconAlignment(Qt::AlignHCenter | Qt::AlignBottom);
   ui.RegionBtnWdg->setIconAlignment(Qt::AlignHCenter | Qt::AlignBottom);

   ui.DesktopBtnWdg->setFontText(tr("桌面共享"), Qt::AlignHCenter | Qt::AlignTop);
   ui.SoftWareBtnWdg->setFontText(tr("软件演示"), Qt::AlignHCenter | Qt::AlignTop);
   ui.RegionBtnWdg->setFontText(tr("区域演示"), Qt::AlignHCenter | Qt::AlignTop);

   connect(ui.DesktopBtnWdg, &ButtonWdg::btnClicked, this, &CaptureSrcUI::OnDesktopClick);
   connect(ui.SoftWareBtnWdg, &ButtonWdg::btnClicked, this, &CaptureSrcUI::OnSoftwareClick);
   connect(ui.RegionBtnWdg, &ButtonWdg::btnClicked, this, &CaptureSrcUI::OnRegionClick);

   return true;
}

void CaptureSrcUI::Destroy() {

}

void CaptureSrcUI::OnDesktopClick() {
    STRU_MAINUI_CLICK_CONTROL loControl;
    loControl.m_eType = (enum_control_type)(control_MonitorSrc);
    emit sig_hideCaptureSrcUi();
    hide();
    SingletonMainUIIns::Instance().PostCRMessage(MSG_MAINUI_CLICK_CONTROL, &loControl, sizeof(STRU_MAINUI_CLICK_CONTROL));
}

void CaptureSrcUI::enterEvent(QEvent *e) {
   QDialog::enterEvent(e);
   m_bEnter = true;
}
void CaptureSrcUI::leaveEvent(QEvent *e) {
   QDialog::leaveEvent(e);
   m_bEnter = false;
   close();
}

void CaptureSrcUI::focusOutEvent(QFocusEvent *e) {
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

void CaptureSrcUI::showEvent(QShowEvent *e) {
   setFocus();
   VHDialog::showEvent(e);
}

void CaptureSrcUI::OnSoftwareClick() {
   STRU_MAINUI_CLICK_CONTROL loControl;
   loControl.m_eType = (enum_control_type)(control_WindowSrc);
   emit sig_hideCaptureSrcUi();
	QJsonObject body;
	SingletonMainUIIns::Instance().reportLog(STR_VH_LIVE_OPERATE, eLogRePortK_SoftwareShare, L"SoftwareShare", body);

   SingletonMainUIIns::Instance().PostCRMessage(MSG_MAINUI_CLICK_CONTROL, &loControl, sizeof(STRU_MAINUI_CLICK_CONTROL));
}

void CaptureSrcUI::OnRegionClick() {

    QJsonObject body;
    SingletonMainUIIns::Instance().reportLog(STR_VH_LIVE_OPERATE, eLogRePortK_RegionShare, L"RegionShare", body);
    emit sig_hideCaptureSrcUi();
    hide();
    STRU_MAINUI_CLICK_CONTROL loControl;
    loControl.m_eType = (enum_control_type)(control_RegionShare);

    SingletonMainUIIns::Instance().PostCRMessage(MSG_MAINUI_CLICK_CONTROL, &loControl, sizeof(STRU_MAINUI_CLICK_CONTROL));

    VH::CComPtr<ICommonData> pCommonData;
    DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
    pCommonData->ReportEvent("qyys00");
}

