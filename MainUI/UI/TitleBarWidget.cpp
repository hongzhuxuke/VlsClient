#include "stdafx.h"
#include <QLabel>
#include <QBoxLayout>
#include <QPainter>
#include <QMouseEvent>
#include <QDebug>
#include <QSpacerItem>
#include "title_button.h"
#include "ToolButton.h"
#include "TitleBarWidget.h"
#include <QDesktopWidget>
#include "MainUIIns.h"
#include "Msg_MainUI.h"  
#include "ICommonData.h"
#include <QDesktopWidget>
#include "Msg_MainUI.h"
#include "Msg_OBSControl.h"
#include "ICommonData.h"
#include "pathmanager.h"
#include "ConfigSetting.h"
#include "start_button.h"
#include "VhallLiveMainDlg.h"
#include "CTimeButton.h"
#include "webinarIdWdg.h"


bool TitleBarWidget::m_bIsPressed = false;
#define STRNORMAL  "color: #979797;"
#define STRHOVER  "color: #FC5659;"
#define STRBORAD_COLOR  "border-color:#979797;"

//#define BOERDER_NOMAL "border-color:rgba(67,67,67,1);background-color:rgba(67,67,67,255); " //;
//#define BOERDER_HOVER "border-color:rgba(44,44,44,1);background-color:rgba(44,00,44,255); " //


#define RecordNormal "border-image:url(:/button/images/record.png);"
#define RecordNormalHover "border-image:url(:/button/images/record_hover.png);"
#define RecordPauseNormal "border-image:url(:/button/images/record_pause.png);"
#define RecordPauseHover "border-image:url(:/button/images/record_pause_hover.png);"
#define RecordStop "border-image:url(:/button/images/record_stop.png);"
#define RecordStopHover "border-image:url(:/button/images/record_stop_hover.png);"


TitleBarWidget::TitleBarWidget(QWidget *parent)
   : CWidget(parent)
   , m_bIsMoveEnabled(true)
   , m_pBtnSetting(NULL)
   , m_pBtnMin(NULL)
   , mpBtnMax(NULL)
   , m_pBtnClose(NULL)
   , m_pTitleLayout(NULL)
   , mRecordState(eRecordState_Stop)
{
}

TitleBarWidget::~TitleBarWidget() {

}
bool TitleBarWidget::MouseIsPressed() {
   return TitleBarWidget::m_bIsPressed;
}

bool TitleBarWidget::Create(const bool& bLivingWdg) {
   if (bLivingWdg)//是否显示 -- 是否直播wdg 
   {
      //录制
      mPRecordBtnWdg = new ButtonWdg(this);
      mPRecordBtnWdg->Create(ButtonWdg::eFontDirection_Right, 7, 14, 6/*, 1*/, RecordNormal, RecordNormalHover, RecordNormal);
      mPRecordBtnWdg->SetBorderWidth(1);
      mPRecordBtnWdg->setRaduis(true);
      mPRecordBtnWdg->setIconSheetStyle(STRNORMAL, STRHOVER, STRBORAD_COLOR);
      //mPRecordBtnWdg->setBorderColor(BOERDER_NOMAL, BOERDER_HOVER);
      mPRecordBtnWdg->setIconTextSize(5,6);
      mPRecordBtnWdg->setFixedSize(90, 26);
      SetRecordState(eRecordState_Stop);

      connect(mPRecordBtnWdg, &ButtonWdg::btnClicked, this, &TitleBarWidget::slotRecordClicked);
      mPRecordBtnWdg->hide();
      //推流按钮
      m_pStartLiveBtn = new CTimeButton(this);
      m_pStartLiveBtn->setFixedSize(90, 26);
      //m_pStartLiveBtn->setStyleSheet(TOOLTIPQSS);
      //m_pStartLiveBtn->setToolTip(tr("开始直播"));
      m_pStartLiveBtn->setText(QString::fromStdWString(L"开始直播"));
      connect(m_pStartLiveBtn, &CTimeButton::clicked, this, &TitleBarWidget::SlotLiveClicked);

      mpStartLiveBtnTimer = new QTimer(this);
      if (mpStartLiveBtnTimer) {
         mpStartLiveBtnTimer->setSingleShot(true);
         connect(mpStartLiveBtnTimer, SIGNAL(timeout()), this, SLOT(slot_OnStartLiveTimeOut()));
      }

      m_pWebinarName = new CLabel(this);
      if (m_pWebinarName) {
         m_pWebinarName->setObjectName("streamName");
         m_pWebinarName->setStyleSheet("#streamName{font-size : 16px;color: rgba(236, 236, 236, 255);}");
      }

      mpWebinarIdWdg = new webinarIdWdg(this);
      if (mpWebinarIdWdg) {
         connect(mpWebinarIdWdg, &webinarIdWdg::sig_copy, this, &TitleBarWidget::Slot_CopyID);
      }
      mpWebinarIdWdg->setFixedSize(117, 20);
      //mpWebinarIdWdg->setStyleSheet( "font:14px;");//background-color: rgb(170, 0, 0);
        //最大化按钮
      mpBtnMax = new TitleButton(this);
      if (NULL == mpBtnMax) {
         ASSERT(FALSE);
         return false;
      }
      mpBtnMax->loadPixmap(":/sysButton/max_button");
      mpBtnMax->setToolTip(tr("最大化"));
      mpBtnMax->setStyleSheet(TOOLTIPQSS);
   }

   m_pBtnRefresh = new TitleButton(this);
   if (m_pBtnRefresh) {
      m_pBtnRefresh->loadPixmap(":/sysButton/img/sysButton/fresh.png");
      m_pBtnRefresh->setToolTip(QString::fromStdWString(L"刷新"));
      m_pBtnRefresh->setStyleSheet(TOOLTIPQSS);
      connect(m_pBtnRefresh, SIGNAL(clicked()), this, SIGNAL(sigRefresh()));
      connect(m_pBtnRefresh, &TitleButton::clicked, this, &TitleBarWidget::sigClicked);
      m_pBtnRefresh->hide();
   }

   m_pBtnSetting = new TitleButton(this);
   if (NULL != m_pBtnSetting) {
      m_pBtnSetting->loadPixmap(":/sysButton/img/sysButton/setting.png");
      m_pBtnSetting->setToolTip(QString::fromStdWString(L"设置"));
      m_pBtnSetting->setStyleSheet(TOOLTIPQSS);
      connect(m_pBtnSetting, SIGNAL(clicked()), this, SLOT(onMenuClick()));
   }

   m_pBtnMin = new TitleButton(this);
   if (NULL == m_pBtnMin) {
      ASSERT(FALSE);
      return false;
   }
   m_pBtnMin->loadPixmap(":/sysButton/min_button");
   m_pBtnMin->setToolTip(QString::fromStdWString(L"最小化"));
   m_pBtnMin->setStyleSheet(TOOLTIPQSS);

   m_pBtnClose = new TitleButton(this);
   if (NULL == m_pBtnClose) {
      ASSERT(FALSE);
      return false;
   }
   m_pBtnClose->loadPixmap(":/sysButton/close_button");
   m_pBtnClose->setToolTip(QString::fromStdWString(L"关闭窗口"));
   m_pBtnClose->setStyleSheet(TOOLTIPQSS);

   connect(m_pBtnMin, SIGNAL(clicked()), this, SLOT(onMinClick()));
   if (NULL != mpBtnMax) {
      connect(mpBtnMax, &TitleButton::clicked, this, &TitleBarWidget::slotMaxClick);
   }
   connect(m_pBtnClose, SIGNAL(clicked()), this, SLOT(onCloseClick()));

   m_pTitleLayout = new QHBoxLayout();
   QString toolConfPath = CPathManager::GetAppDataPath() + QString::fromStdWString(VHALL_TOOL_CONFIG);
   int hideLogo = ConfigSetting::ReadInt(toolConfPath, GROUP_DEFAULT, KEY_VHALL_LOGO_HIDE, 0);
   if (hideLogo == 0 && !bLivingWdg) {//隐藏logo  bLivingWdg 是否显示（直播wdg） 
      m_pVersionTitle = new QLabel(this);
      QString imagePath = CPathManager::GetTitleLogoImagePath();
      QPixmap qImageTitleLogo(imagePath);
      if (NULL == m_pVersionTitle) {
         ASSERT(FALSE);
         return false;
      }
      m_pVersionTitle->setPixmap(qImageTitleLogo);
      m_pVersionTitle->setFixedSize(qImageTitleLogo.size());
      m_pTitleLayout->addWidget(m_pVersionTitle, 0, Qt::AlignVCenter);//向布局中添加logo
      m_pTitleLayout->addStretch();
      QSpacerItem *horizontalSpacer = new QSpacerItem(140, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);
      m_pTitleLayout->addItem(horizontalSpacer);//logo后的弹簧
   }

   if (NULL != m_pWebinarName) {//活动名称
      m_pTitleLayout->addWidget(m_pWebinarName, 0, Qt::AlignLeft | Qt::AlignHCenter);
   }

   if (NULL != mpWebinarIdWdg){
      m_pTitleLayout->addWidget(mpWebinarIdWdg);
   }
   m_pTitleLayout->addStretch();

   if (!bLivingWdg){ //是否显示 -- 不是直播wdg ，显示头像
      //头像图片
      m_pImageLayout = new QHBoxLayout();
      m_pTitleLayout->addLayout(m_pImageLayout);  //头像、昵称  布局区 占位
   }

   if (NULL != mPRecordBtnWdg)
      m_pTitleLayout->addWidget(mPRecordBtnWdg);			//录制
   if (NULL != m_pStartLiveBtn)
      m_pTitleLayout->addWidget(m_pStartLiveBtn);		//开始直播

   m_pTitleLayout->addWidget(m_pBtnSetting, 0, Qt::AlignVCenter);  //设置
   m_pTitleLayout->addWidget(m_pBtnRefresh, 0, Qt::AlignVCenter);//刷新
   ////////////////////分享/////////////////////

   m_pTitleLayout->addWidget(m_pBtnMin, 0, Qt::AlignVCenter);
   m_pTitleLayout->addWidget(mpBtnMax, 0, Qt::AlignVCenter);
   m_pTitleLayout->addWidget(m_pBtnClose, 0, Qt::AlignVCenter);

   m_pTitleLayout->setSpacing(10);
   m_pTitleLayout->setContentsMargins(20, 0, 10, 0);

   setLayout(m_pTitleLayout);
   setMaximumHeight(40);
   setMinimumHeight(40);

   QPalette palette;
   palette.setColor(QPalette::Background, QColor(43, 44, 46));
   setPalette(palette);
   setAutoFillBackground(true);
   return true;
}

QPoint TitleBarWidget::RecordPos()
{
   QPoint refPos;
   if (NULL != mPRecordBtnWdg){
      QPoint pos = mPRecordBtnWdg->pos();
      refPos = this->mapToGlobal(pos);
   }
   return refPos;
}

void TitleBarWidget::AddImageWidget(QWidget *w) {
   if (!w || !m_pImageLayout) {
      return;
   }
   m_pImageLayout->addWidget(w);
}

void TitleBarWidget::RemoveImageWidget(QWidget *w) {
   if (!w || !m_pImageLayout) {
      return;
   }
   m_pImageLayout->removeWidget(w);
}

void TitleBarWidget::HideLogo(bool bHide) {
   if (m_pVersionTitle != NULL) {
      if (bHide) {
         m_pVersionTitle->hide();
      }
      else {
         m_pVersionTitle->show();
      }
   }
}

void TitleBarWidget::HideSetting() {
   if (NULL != m_pBtnSetting) {
      m_pBtnSetting->hide();
   }
}

void TitleBarWidget::ShowRefreshButton(const bool& bShow) {
   if (m_pBtnRefresh) {
      if (bShow) {
         m_pBtnRefresh->show();
      }
      else {
         m_pBtnRefresh->hide();
      }
   }
}

void TitleBarWidget::Destroy() {
   if (NULL != m_pBtnClose) {
      delete m_pBtnClose;
      m_pBtnClose = NULL;
   }

   if (NULL != m_pBtnMin) {
      delete m_pBtnMin;
      m_pBtnMin = NULL;
   }

   if (NULL != mpBtnMax) {
      delete mpBtnMax;
      mpBtnMax = NULL;
   }

   if (NULL != m_pBtnSetting) {
      delete m_pBtnSetting;
      m_pBtnSetting = NULL;
   }

   if (NULL != m_pVersionTitle) {
      delete m_pVersionTitle;
      m_pVersionTitle = NULL;
   }
   if (NULL != m_pTitleLayout) {
      delete m_pTitleLayout;
      m_pTitleLayout = NULL;
   }
}

void TitleBarWidget::setMoveEnabled(bool ok) {
   m_bIsMoveEnabled = ok;
}

void TitleBarWidget::onMenuClick() {
   STRU_MAINUI_CLICK_CONTROL loControl;
   loControl.m_eType = control_ShowSetting;
   SingletonMainUIIns::Instance().PostCRMessage(MSG_MAINUI_CLICK_CONTROL, &loControl, sizeof(STRU_MAINUI_CLICK_CONTROL));

   VH::CComPtr<ICommonData> pCommonData;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
   pCommonData->ReportEvent("sz00");
   sigClicked();
}

void TitleBarWidget::onMinClick() {
   qDebug() << "onMinClick";
   STRU_MAINUI_CLICK_CONTROL loControl;
   loControl.m_eType = control_Minimize;
   loControl.m_dwExtraData = m_parentWidget != NULL ? (DWORD)m_parentWidget : (DWORD)this->parent();

   SingletonMainUIIns::Instance().PostCRMessage(MSG_MAINUI_CLICK_CONTROL, &loControl, sizeof(STRU_MAINUI_CLICK_CONTROL));
   sigClicked();
   emit sigMin();
}

void TitleBarWidget::slotMaxClick()
{
   emit sigMaxClicked();
}

void TitleBarWidget::SetMaxIcon(bool bMax /*= true*/)
{
   if (bMax) {
      mpBtnMax->loadPixmap(":/sysButton/normal_button");
   }
   else {
      mpBtnMax->loadPixmap(":/sysButton/max_button");
   }
   mpBtnMax->setEnabled(true);
   setFocus();
   this->repaint();
}

void TitleBarWidget::slot_OnStartLiveTimeOut() {
   if (m_pStartLiveBtn) {
      m_pStartLiveBtn->setEnabled(true);
   }
}

void TitleBarWidget::onCloseClick() {
   qDebug() << "onCloseClick";
   TRACE6("%s\n", __FUNCTION__);
   STRU_MAINUI_CLICK_CONTROL loControl;
   loControl.m_eType = control_CloseApp;
   loControl.m_dwExtraData = m_parentWidget != NULL ? (DWORD_PTR)m_parentWidget : (DWORD_PTR)this->parent();
   SingletonMainUIIns::Instance().PostCRMessage(MSG_MAINUI_CLICK_CONTROL, &loControl, sizeof(STRU_MAINUI_CLICK_CONTROL));
   emit sigToClose();
   sigClicked();
}

void TitleBarWidget::Slot_CopyID() {
   emit sigCopy();
}

void TitleBarWidget::SlotLiveClicked() {
   STRU_MAINUI_CLICK_CONTROL loControl;
   loControl.m_eType = control_StartLive;
   loControl.m_dwExtraData = m_pStartLiveBtn->GetLiveStatus() ? 0 : 1;//开播状态
   //QJsonObject body;
   //body["ls"] = (loControl.m_dwExtraData == 0 ? "Stop" : "Start");
   //SingletonMainUIIns::Instance().reportLog(STR_VH_LIVE_OPERATE, eLogRePortK_StartLive, L"StartLive", body);
   SingletonMainUIIns::Instance().PostCRMessage(MSG_MAINUI_CLICK_CONTROL, &loControl, sizeof(STRU_MAINUI_CLICK_CONTROL));
   if (m_pStartLiveBtn && mpStartLiveBtnTimer) {
      m_pStartLiveBtn->setEnabled(false);
      mpStartLiveBtnTimer->start(2000);
   }
   emit sigClicked();
}


void TitleBarWidget::mousePressEvent(QMouseEvent *e) {
   m_qPressPoint = e->pos();
   m_bIsPressed = true;
   TRACE6("%s\n", __FUNCTION__);
   emit sigMousePressed();
}

void TitleBarWidget::SetPressedPoint(QPoint pos) {
   m_qPressPoint = pos;
}

void TitleBarWidget::mouseMoveEvent(QMouseEvent *e) {
   if (!m_bIsMoveEnabled) {
      TRACE6("%s m_bIsMoveEnabled is false\n", __FUNCTION__);
      return;
   }
   if ((e->buttons() == Qt::LeftButton) && m_bIsPressed) {
      QWidget* parent_widget = m_parentWidget != NULL ? m_parentWidget : this->parentWidget();
      QPoint parent_point = parent_widget->pos();
      parent_point.setX(parent_point.x() + e->x() - m_qPressPoint.x());
      parent_point.setY(parent_point.y() + e->y() - m_qPressPoint.y());
      parent_widget->move(parent_point);
      sigMouseMoved();
   }
}

void TitleBarWidget::mouseReleaseEvent(QMouseEvent *) {
   if (m_bIsPressed) {
      m_bIsPressed = false;
      TRACE6("%s\n", __FUNCTION__);
   }
   QWidget* parent_widget = m_parentWidget != NULL ? m_parentWidget : this->parentWidget();
   QPoint pos = parent_widget->pos();
   QRect rect = QApplication::desktop()->availableGeometry(pos);
   QRect wnd_rect = parent_widget->frameGeometry();
   if (pos.y() > rect.height() - 150) {
      pos.setY(rect.height() - 150);
      parent_widget->move(pos);
   }
   else if (pos.y() < rect.y()) {
      parent_widget->move(pos + QPoint(0, +100));
   }
   else if (wnd_rect.x() >= rect.x() + rect.width() - 100) {
      parent_widget->move(QPoint(wnd_rect.x(), wnd_rect.y()) + QPoint(-100, 0));
   }
   else if (wnd_rect.x() + this->width() < rect.x() + 100) {
      parent_widget->move(QPoint(wnd_rect.x(), wnd_rect.y()) + QPoint(100, 0));
   }
}


int TitleBarWidget::iStartLiveX()
{
   int iRef = 0;
   if (NULL != m_pStartLiveBtn) {
      iRef = m_pStartLiveBtn->x();
   }
   return iRef;
}

int TitleBarWidget::iStartLiveWidth()
{
   int iRef = 0;
   if (NULL != m_pStartLiveBtn) {
      iRef = m_pStartLiveBtn->width();
   }
   return iRef;
}

int TitleBarWidget::iStartLiveHeight(){
   int iRef = 0;
   if (NULL != m_pStartLiveBtn) {
      iRef = m_pStartLiveBtn->height();
   }
   return iRef;
}

void TitleBarWidget::SetStreamButtonStatus(bool isActive) {
   if (NULL != m_pStartLiveBtn) {
      m_pStartLiveBtn->updateLiveStatus(isActive);
   }
}

bool TitleBarWidget::isStreamStatusActive()
{
   bool bRef = false;
   if (NULL != m_pStartLiveBtn){
      bRef = m_pStartLiveBtn->GetLiveStatus();
   }
   return bRef;
}

void TitleBarWidget::SetControlRightWidget(bool ok) {
   this->m_bControlRightWidget = ok;
}

void TitleBarWidget::ShowWebinarName(bool show) {
   if (show) {
      if (m_pWebinarName) {
         m_pWebinarName->show();
      }
      if (mpWebinarIdWdg) {
         mpWebinarIdWdg->show();
      }
   }
   else {
      if (m_pWebinarName) {
         m_pWebinarName->hide();
      }
      if (mpWebinarIdWdg) {
         mpWebinarIdWdg->hide();
      }
   }
}

void TitleBarWidget::SetWebinarName(QString name, QString id) {
   QString title = QString("%1").arg(CPathManager::GetString(name, 24), id);
   if (m_pWebinarName) {
      m_pWebinarName->setText(title);
      m_pWebinarName->setToolTip(name);
      m_pWebinarName->setStyleSheet(TOOLTIPQSS);
   }

   if (mpWebinarIdWdg) {
      mpWebinarIdWdg->SetId(id);
   }
}

void TitleBarWidget::ClearWebinarName() {
   m_pWebinarName->clear();
   mpWebinarIdWdg->SetId("");
}

void TitleBarWidget::StartWithActiveLive(int liveTime) {
   if (NULL != m_pStartLiveBtn) {
      m_pStartLiveBtn->InitStartTime(liveTime);
      if (liveTime > 0) {
         m_pStartLiveBtn->updateLiveStatus(true);
      }
   }
}

void TitleBarWidget::ReInit() {
   if (m_pStartLiveBtn) {
      m_pStartLiveBtn->Clear();
   }
}

void TitleBarWidget::ClearTimeClock() {
   if (NULL != m_pStartLiveBtn) {
      m_pStartLiveBtn->Clear();
   }
}

void TitleBarWidget::StartTimeClock(bool bStart) {
   if (NULL != m_pStartLiveBtn) {
      if (bStart) {
         m_pStartLiveBtn->updateLiveStatus(true);
      }
      else {
         m_pStartLiveBtn->updateLiveStatus(false);
      }
   }
}

long TitleBarWidget::GetLiveStartTime() {
   if (m_pStartLiveBtn) {
      return m_pStartLiveBtn->GetClassTime();
   }
   return 0;
}

void TitleBarWidget::SetRecordState(const int& eState) {
   mRecordState = eState;
   switch (mRecordState) {
      case eRecordState_Stop: {//当前录制状态：停止 ---  显示“录制” 
         mPRecordBtnWdg->setFontText(tr("录制"), Qt::AlignVCenter | Qt::AlignLeft);//6pt
         mPRecordBtnWdg->setIconAlignment(Qt::AlignVCenter | Qt::AlignRight);
         mPRecordBtnWdg->setIconSheet(RecordNormal, RecordNormalHover, RecordNormal);
         break;
      }                     
      case eRecordState_Recording: {//当前录制状态： 录制中 ---  显示“录制中”
         mPRecordBtnWdg->setFontText(tr("录制中"), Qt::AlignVCenter | Qt::AlignLeft);
         mPRecordBtnWdg->setIconAlignment(Qt::AlignVCenter | Qt::AlignRight);
         mPRecordBtnWdg->setIconSheet(RecordStop, RecordStopHover, RecordStop);
         break;
      }                 
      default: {//当前录制状态： 录制暂停 ---  显示“录制暂停”
         mPRecordBtnWdg->setFontText(tr("暂停"), Qt::AlignVCenter | Qt::AlignLeft);
         mPRecordBtnWdg->setIconAlignment(Qt::AlignVCenter | Qt::AlignRight);
         mPRecordBtnWdg->setIconSheet(RecordPauseNormal, RecordPauseHover, RecordPauseNormal);
         break;
      }
   }
   slotEnableRecordBtn(true);
}

void TitleBarWidget::setRecordBtnShow(const int iDisplay) {
   if (eDispalyCutRecord_Show == iDisplay) {
      mPRecordBtnWdg->show();
   }
   else {
      mPRecordBtnWdg->hide();
   }
}

void TitleBarWidget::slotEnableRecordBtn(bool enable	/*= true*/) {
   if (NULL == mpBtnRecordEnableTime) {
      mpBtnRecordEnableTime = new QTimer(this);
      connect(mpBtnRecordEnableTime, &QTimer::timeout, this, &TitleBarWidget::slotEnableRecordBtnTimeOut);
   }

   mPRecordBtnWdg->setEnabled(enable);

   if (enable){ //启用
      if (mpBtnRecordEnableTime&&mpBtnRecordEnableTime->isActive()) {
         mpBtnRecordEnableTime->stop();
      }
   }
   else {//禁用  2s后启用
      mpBtnRecordEnableTime->start(2000);
   }
}

void TitleBarWidget::slotEnableRecordBtnTimeOut() {
   slotEnableRecordBtn(true);
}

int TitleBarWidget::GetRecordState() {
   return mRecordState;
}

bool TitleBarWidget::IsRecordBtnhide() {
   bool bHide = true;
   if (NULL != mPRecordBtnWdg) {
      bHide = (!mPRecordBtnWdg->isVisible());
   }
   return bHide;
}

void TitleBarWidget::slotRecordClicked() {
   STRU_MAINUI_CLICK_CONTROL loControl;
   switch (mRecordState) {
   case eRecordState_Stop:{ //当前录制状态：停止
      if (m_pStartLiveBtn->GetLiveStatus()) {
         slotEnableRecordBtn(false);
         //开始录制
         QJsonObject body;
         body["rs"] = "StartRecord";
         SingletonMainUIIns::Instance().reportLog(STR_VH_LIVE_OPERATE, eLogRePortK_StartRecord, L"StartRecord", body);

         loControl.m_eType = (enum_control_type)(control_StartRecord);
         loControl.m_dwExtraData = true;            //false 结束录制 true开始录制
      }
      else {
         loControl.m_eType = control_Record;
      }
   }
   break;
   case eRecordState_Recording://当前录制状态： 录制中
   case eRecordState_Suspend:{//当前录制状态： 暂停录制
      loControl.m_eType = control_Record;
      m_bRecordShow = !m_bRecordShow;
      if (!m_bRecordShow) {
         return;
      }
      loControl.m_dwExtraData = m_bRecordShow ? 1 : 0;
   }
   break;
   default:
      break;
   }
   SingletonMainUIIns::Instance().PostCRMessage(
      MSG_MAINUI_CLICK_CONTROL, &loControl, sizeof(STRU_MAINUI_CLICK_CONTROL));
   emit sigClicked();
}
