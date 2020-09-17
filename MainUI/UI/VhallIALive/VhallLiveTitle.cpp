#include "VhallLiveTitle.h"
#include "Pathmanager.h"
#include "ConfigSetting.h"
#include "pub.Const.h"
#include "MainUIIns.h"
#include "ButtonWdg.h"
#include <QTimer>
#include "CWidget.h"
#include "RecordDlg.h"
#include "DebugTrace.h"
#include <QDesktopWidget>
#define STRNORMAL  "color: #979797;"
#define STRHOVER  "color: #FC5659;"
#define STRBORAD_COLOR  "border-color:#979797;"

#define COUNTDOWNTIME 30
#define RecordNormal "border-image:url(:/button/images/record.png);"
#define RecordNormalHover "border-image:url(:/button/images/record_hover.png);"
#define RecordPauseNormal "border-image:url(:/button/images/record_pause.png);"
#define RecordPauseHover "border-image:url(:/button/images/record_pause_hover.png);"
#define RecordStop "border-image:url(:/button/images/record_stop.png);"
#define RecordStopHover "border-image:url(:/button/images/record_stop_hover.png);"

VhallLiveTitle::VhallLiveTitle(QWidget *parent)
   : CWidget(parent)
{
   ui.setupUi(this);
   ui.pushButton_close->hide();
   ui.pushButton_fresh->hide();
   ui.pushButton_fullSize->hide();
   ui.pushButton_minSize->hide();
   //ui.pushButton_setting->hide();
   ui.label_logo->hide();
   ui.label_userName->hide();
  // ui.pushButton_close->loadPixmap(":/sysButton/close_button");
   //ui.pushButton_fresh->loadPixmap(":/sysButton/refresh_button");
   ui.pushButton_fullSize->loadPixmap(":/sysButton/max_button");
   ui.pushButton_minSize->loadPixmap(":/sysButton/min_button");
   ui.pushButton_setting->loadPixmap(":/sysButton/img/sysButton/setting.png");
   ui.pushButton_fresh->loadPixmap(":/sysButton/img/sysButton/fresh.png");

   connect(ui.pushButton_setting, SIGNAL(clicked()), this, SLOT(slot_OnSetting()));
   connect(ui.pushButton_close, SIGNAL(clicked()), this, SLOT(slot_OnClose()));
   connect(ui.pushButton_fresh, SIGNAL(clicked()), this, SLOT(slot_OnFresh()));
   connect(ui.pushButton_fullSize, SIGNAL(sigClicked()), this, SLOT(slot_OnFullSize()));
   connect(ui.pushButton_minSize, SIGNAL(sigClicked()), this, SLOT(slot_OnMinSize()));
   connect(ui.widget_room_id, SIGNAL(sig_copy()), this, SLOT(slot_OnCopy()));
   connect(ui.pushButton_startLive, &CTimeButton::clicked, this, &VhallLiveTitle::Slot_OnClickedStartLiveCtrlBtn);
   connect(ui.pushButton_toSpeak, &QPushButton::clicked, this, &VhallLiveTitle::Slot_HandsUp);
   //connect(ui.pushButton_toSpeak, &QPushButton::clicked, this, &VhallLiveTitle::sigToSpeakClicked);

   ui.pushButton_startLive->updateLiveStatus(false);
   //录制
   ui.RecordBtnWdg->Create(ButtonWdg::eFontDirection_Right, 7, 14, 6, RecordNormal, RecordNormalHover, RecordNormal);
   ui.RecordBtnWdg->SetBorderWidth(1);
   ui.RecordBtnWdg->setRaduis(true);
   ui.RecordBtnWdg->setIconTextSize(5, 6);
   ui.RecordBtnWdg->setIconSheetStyle(STRNORMAL, STRHOVER, STRBORAD_COLOR);
   ui.RecordBtnWdg->setFixedSize(90, 26);
   SetRecordState(eRecordState_Stop);
   connect(ui.RecordBtnWdg, &ButtonWdg::btnClicked, this, &VhallLiveTitle::slotRecordClicked);

   ui.pushButton_startLive->setText(QStringLiteral("开始直播"));
   ui.pushButton_close->setToolTip(QStringLiteral("关闭"));
   ui.pushButton_fresh->setToolTip(QStringLiteral("刷新"));
   ui.pushButton_fullSize->setToolTip(QStringLiteral("最大化"));
   ui.pushButton_minSize->setToolTip(QStringLiteral("最小化"));
   ui.pushButton_setting->setToolTip(QStringLiteral("设置"));

   //ui.pushButton_close->setStyleSheet(TOOLTIPQSS);
   ui.pushButton_fresh->setStyleSheet(TOOLTIPQSS);
   ui.pushButton_fullSize->setStyleSheet(TOOLTIPQSS);
   ui.pushButton_minSize->setStyleSheet(TOOLTIPQSS);
   ui.pushButton_setting->setStyleSheet(TOOLTIPQSS);
   QString toolConfPath = CPathManager::GetAppDataPath() + QString::fromStdWString(VHALL_TOOL_CONFIG);
   int hideLogo = ConfigSetting::ReadInt(toolConfPath, GROUP_DEFAULT, KEY_VHALL_LOGO_HIDE, 0);
   if (hideLogo != 0) {
      ui.label_logo->hide();
   }
   else {
      QString imagePath = CPathManager::GetTitleLogoImagePath();
      mPixmap.load(imagePath);
      ui.label_logo->setPixmap(mPixmap);
   }
}

VhallLiveTitle::~VhallLiveTitle(){
   disconnect(ui.pushButton_setting, SIGNAL(clicked()), this, SLOT(slot_OnSetting()));
   disconnect(ui.pushButton_close, SIGNAL(clicked()), this, SLOT(slot_OnClose()));
   disconnect(ui.pushButton_fresh, SIGNAL(clicked()), this, SLOT(slot_OnFresh()));
   disconnect(ui.pushButton_fullSize, SIGNAL(sigClicked()), this, SLOT(slot_OnFullSize()));
   disconnect(ui.pushButton_minSize, SIGNAL(sigClicked()), this, SLOT(slot_OnMinSize()));
   disconnect(ui.widget_room_id, SIGNAL(sig_copy()), this, SLOT(slot_OnCopy()));
   disconnect(ui.pushButton_startLive, &CTimeButton::clicked, this, &VhallLiveTitle::Slot_OnClickedStartLiveCtrlBtn);
   disconnect(ui.pushButton_toSpeak, &QPushButton::clicked, this, &VhallLiveTitle::Slot_HandsUp);
   if (nullptr != mpCountDownTimer) {
      if(mpCountDownTimer->isActive())
         mpCountDownTimer->stop();
      delete mpCountDownTimer; 
      mpCountDownTimer = nullptr;
   }
}


void VhallLiveTitle::SetEnableCloseBtn(bool enable) {
   ui.pushButton_close->setEnabled(enable);
}

void VhallLiveTitle::SetToSpeakEnble(bool bEnble /*= true*/)
{
   ui.pushButton_toSpeak->setEnabled(bEnble);
}

void VhallLiveTitle::SetToSpeakBtnState(bool toSpeak) {
   TRACE6("%s pushButton_toSpeak toSpeak %d",__FUNCTION__, toSpeak);
   if (toSpeak) {
      ui.pushButton_toSpeak->setText(APPLY_SPEAK);
      mbIsSpeaking = false;
   }
   else {
      if (nullptr != mpCountDownTimer && mpCountDownTimer->isActive()) {
         mpCountDownTimer->stop();
      }
      ui.pushButton_toSpeak->setText(DOWNMIC);
      mbIsSpeaking = true;
   }
   this->update();
}

bool VhallLiveTitle::GetToSpeakBtnState() {
   return mbIsSpeaking;
}

void VhallLiveTitle::SetLiveInfo(const QString& streamID, const QString& roomName, const QString& userName) {
   ui.label_RoomTitle->setText(CPathManager::GetString(roomName, 24));
   ui.label_RoomTitle->setToolTip(roomName);
   ui.label_RoomTitle->setStyleSheet(TOOLTIPQSS);
   ui.widget_room_id->SetId(streamID);
}

void VhallLiveTitle::SetHideMaxBtn(bool hide) {
   ui.pushButton_fullSize->setHidden(hide);
}

void VhallLiveTitle::SetMaxIcon(bool bMax/* = true*/)
{
   ui.pushButton_fullSize->hide();
   if (bMax) {
      ui.pushButton_fullSize->setToolTip(QStringLiteral("向下还原"));
      ui.pushButton_fullSize->loadPixmap(":/sysButton/normal_button");
   }
   else {
      ui.pushButton_fullSize->setToolTip(QStringLiteral("最大化"));
      ui.pushButton_fullSize->loadPixmap(":/sysButton/max_button");
   }
   ui.pushButton_fullSize->show();
   ui.pushButton_fullSize->clearFocus();
   ui.pushButton_fullSize->SetEnabled(true);
   ui.pushButton_fullSize->repaint();
}

QPoint VhallLiveTitle::RecordPos()
{
   QPoint refPos;
   if (NULL != ui.RecordBtnWdg)
   {
      QPoint pos = ui.RecordBtnWdg->pos();
      refPos = this->mapToGlobal(pos);
   }
   return refPos;
}

void VhallLiveTitle::RecordStateSucce()
{
   switch (miNewRecordRs)
   {
   case eCurOp_Stop:
      SetRecordState(eRecordState_Stop);
      break;
   case eCurOp_Suspend:
      SetRecordState(eRecordState_Suspend);
      break;
   case eCurOp_Recovery:
      SetRecordState(eRecordState_Recording);
      break;
   default:
      break;
   }
}

void VhallLiveTitle::SetRecordState(const int& eState) {

   mRecordState = eState;
   switch (mRecordState) {
      case eRecordState_Stop: {//当前录制状态：停止 ---  显示“录制” 
         if (ui.RecordBtnWdg->GetRecordState() == QStringLiteral("录制中") || ui.RecordBtnWdg->GetRecordState() == QStringLiteral("暂停")) {
            createRecordDlg();
            mPRecordDlg->SetRecordState(eRecordState_Stop);
         }
         ui.RecordBtnWdg->setFontText(QStringLiteral("录制"), Qt::AlignVCenter | Qt::AlignLeft);//6pt
         ui.RecordBtnWdg->setIconAlignment(Qt::AlignVCenter | Qt::AlignRight); 
         ui.RecordBtnWdg->setIconSheet(RecordNormal, RecordNormalHover, RecordNormal);
         break;
      }                           
      case eRecordState_Recording: {//当前录制状态： 录制中 ---  显示“录制中”
         ui.RecordBtnWdg->setFontText(QStringLiteral("录制中"), Qt::AlignVCenter | Qt::AlignLeft);
         ui.RecordBtnWdg->setIconAlignment(Qt::AlignVCenter | Qt::AlignRight);
         ui.RecordBtnWdg->setIconSheet(RecordStop, RecordStopHover, RecordStop);
         createRecordDlg();
         mPRecordDlg->SetRecordState(eRecordState_Recording);
      }
      default: {//当前录制状态： 录制暂停 ---  显示“录制暂停”
         ui.RecordBtnWdg->setFontText(QStringLiteral("暂停"), Qt::AlignVCenter | Qt::AlignLeft);
         ui.RecordBtnWdg->setIconAlignment(Qt::AlignVCenter | Qt::AlignRight);
         ui.RecordBtnWdg->setIconSheet(RecordPauseNormal, RecordPauseHover, RecordPauseNormal);
         createRecordDlg();
         mPRecordDlg->SetRecordState(eRecordState_Suspend);
         break;
      } 
   }
   slotEnableRecordBtn(true);
}

void VhallLiveTitle::setRecordBtnShow(const int iDisplay) {
   if (eDispalyCutRecord_Show == iDisplay) {
      ui.RecordBtnWdg->show();
   }
   else {
      ui.RecordBtnWdg->hide();
   }
}

bool VhallLiveTitle::IsRecordBtnhide() {
   bool bHide = true;
   if (NULL != ui.RecordBtnWdg) {
      bHide = (!ui.RecordBtnWdg->isVisible());
   }
   return bHide;

}

void VhallLiveTitle::slotEnableRecordBtn(bool enable	/*= true*/) {
   if (NULL == mpBtnRecordEnableTime) {
      mpBtnRecordEnableTime = new QTimer(this);
      connect(mpBtnRecordEnableTime, &QTimer::timeout, this, &VhallLiveTitle::slotEnableRecordBtnTimeOut);
   }

   ui.RecordBtnWdg->setEnabled(enable);

   if (enable)//启用
   {
      if (mpBtnRecordEnableTime&&mpBtnRecordEnableTime->isActive()) {
         mpBtnRecordEnableTime->stop();
      }
   }
   else {//禁用  2s后启用
      mpBtnRecordEnableTime->start(2000);
   }
}


void VhallLiveTitle::slotRecordClicked() {
   //STRU_MAINUI_CLICK_CONTROL loControl;
   switch (mRecordState) {
   case eRecordState_Stop://当前录制状态：停止
   {
      if (ui.pushButton_startLive->GetLiveStatus()) {//已经开始直播，直接开始录制
         slotEnableRecordBtn(false);
         //开始录制
         QJsonObject body;
         body["rs"] = "StartRecord";
         SingletonMainUIIns::Instance().reportLog(STR_VH_LIVE_OPERATE, eLogRePortK_StartRecord, L"StartRecord", body);
         miNewRecordRs = eCurOp_Recovery;
         emit sigRecordType(eRecordReTyp_Start);
         //loControl.m_eType = (enum_control_type)(control_StartRecord);
         //loControl.m_dwExtraData = true;            //false 结束录制 true开始录制
      }
      else {//未开始直播提示
         //loControl.m_eType = control_Record;  //L"请先开始直播，再开启录制")
         emit sigFadeOutTip(RECORED_AFTER_LIVING);
      }
   }
   break;
   case eRecordState_Recording://当前录制状态： 录制中    显示CRecordDlg 对象，将控制权限给该对象
   case eRecordState_Suspend://当前录制状态： 暂停录制
   {
      createRecordDlg();

      QPoint Pos = RecordPos();
      mPRecordDlg->move(Pos.x() - 11, Pos.y() + ui.RecordBtnWdg->height());
      mPRecordDlg->show();
      //loControl.m_eType = control_Record;
      //m_bRecordShow = !m_bRecordShow;
      //if (!m_bRecordShow) {
      //	return;
      //}
      //loControl.m_dwExtraData = m_bRecordShow ? 1 : 0;
   }
   break;
   default:
      break;
   }
   //SingletonMainUIIns::Instance().PostCRMessage(
   //	MSG_MAINUI_CLICK_CONTROL, &loControl, sizeof(STRU_MAINUI_CLICK_CONTROL));
   //emit sigClicked();
}

void VhallLiveTitle::createRecordDlg()
{
   if (NULL == mPRecordDlg)
   {
      mPRecordDlg = new CRecordDlg(this);
      if (NULL != mPRecordDlg)
      {
         connect(mPRecordDlg, &CRecordDlg::sigClicked, this, &VhallLiveTitle::slot_RecordStateChanged);
      }
      if (NULL == mPRecordDlg || !mPRecordDlg->Create(false)) {
         //ASSERT(FALSE);
         return;
      }
   }
}

void VhallLiveTitle::slot_RecordStateChanged(int iOp)
{
   miNewRecordRs = iOp;
   switch (iOp)
   {
   case eCurOp_Recovery:
      emit sigRecordType(eRecordReTyp_Start);
      break;
   case eCurOp_Stop:
      emit sigRecordType(eRecordReTyp_Suspend);
      break;
   case eCurOp_Suspend:
      emit sigRecordType(eRecordReTyp_Stop);
      break;
   default:
      break;
   }
}

void VhallLiveTitle::slotEnableRecordBtnTimeOut() {
   slotEnableRecordBtn(true);
}

void VhallLiveTitle::SetTitleType(TitleType type) {
   //setObjectName("VhallLiveTitle");
   this->setStyleSheet("VhallLiveTitle{background-color: rgb(43, 44, 46);}");
   switch (type)
   {
   case eVhallIALiveNone: {

      ui.pushButton_close->hide();
      ui.pushButton_fresh->hide();
      ui.pushButton_fullSize->hide();
      ui.pushButton_minSize->hide();
      ui.pushButton_setting->hide();
      ui.label_logo->hide();
      //		ui.pushButton_share->hide();
      ui.label_RoomTitle->hide();
      //ui.label_userName->hide();
      ui.widget_room_id->hide();
      ui.widget_3->hide();
   }
                          break;
   case eVhallIALive: {

      ui.pushButton_close->show();
      ui.pushButton_fresh->show();
      ui.pushButton_fullSize->show();
      ui.pushButton_minSize->show();
      ui.pushButton_setting->show();
      ui.label_RoomTitle->show();
      //ui.label_userName->show();
      ui.widget_room_id->show();
      QString toolConfPath = CPathManager::GetAppDataPath() + QString::fromStdWString(VHALL_TOOL_CONFIG);
      int hideLogo = ConfigSetting::ReadInt(toolConfPath, GROUP_DEFAULT, KEY_VHALL_LOGO_HIDE, 0);
      if (hideLogo != 0) {
         //            ui.pushButton_share->hide();
         ui.label_logo->hide();
      }
      else {
         ui.label_logo->show();
         //            ui.pushButton_share->show();
      }
   }
                      break;
   case eVhallIALiveSetting: {

      ui.pushButton_close->show();
      ui.pushButton_fresh->hide();
      ui.pushButton_fullSize->hide();
      ui.pushButton_minSize->hide();
      ui.pushButton_setting->hide();
      ui.label_logo->hide();
      //		ui.pushButton_share->hide();	
      ui.label_RoomTitle->hide();
      ui.label_userName->hide();
      ui.widget_room_id->hide();
      ui.widget_3->hide();
   }
                             break;
   default:
      break;
   }
}

void VhallLiveTitle::slot_OnClose() {
   emit sig_ClickedClose();
}

void VhallLiveTitle::slot_OnFresh() {
   emit sig_ClickedFresh();
}

void VhallLiveTitle::slot_OnFullSize() {
   emit sig_ClickedFullSize();
   ui.pushButton_fullSize->repaint();
}

void VhallLiveTitle::slot_OnMinSize() {
   emit sig_ClickedMinSize();
}

void VhallLiveTitle::slot_OnSetting() {
   emit sig_ClickedSetting();
}

void VhallLiveTitle::slot_OnShare() {
   emit sig_ClickedShare();
}

void VhallLiveTitle::slot_OnCopy() {
   emit sig_OnCopy();
}


void VhallLiveTitle::mousePressEvent(QMouseEvent *event) {
   if (event) {
      mPressPoint = event->pos();
      mIsMoved = true;
   }
}

void VhallLiveTitle::mouseMoveEvent(QMouseEvent *event) {
   if (event && (event->buttons() == Qt::LeftButton) && mIsMoved  && m_bIsEnableMove) {
      QWidget* parent_widget = this->nativeParentWidget();
      QString objectName = parent_widget->objectName();
      QPoint parent_point = parent_widget->pos();
      parent_point.setX(parent_point.x() + event->x() - mPressPoint.x());
      parent_point.setY(parent_point.y() + event->y() - mPressPoint.y());
      parent_widget->move(parent_point);
      emit sigMouseMoved();
   }
}

void VhallLiveTitle::mouseReleaseEvent(QMouseEvent *) {
   if (mIsMoved) {
      mIsMoved = false;
   }
   QWidget* parent_widget = this->nativeParentWidget();
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

void VhallLiveTitle::ShowStartLiveBtn(bool show) {
   if (show) {
      ui.pushButton_startLive->show();
      ui.pushButton_toSpeak->hide();
   }
   else {
      ui.pushButton_toSpeak->show();
      ui.pushButton_startLive->hide();
   }
}

void VhallLiveTitle::slot_OnStartLiveTimeOut() {
   ui.pushButton_startLive->setEnabled(true);
}


void VhallLiveTitle::StartWithActiveLive(int liveTime) {
   if (NULL != ui.pushButton_startLive) {
      ui.pushButton_startLive->InitStartTime(liveTime);
      if (liveTime > 0) {
         ui.pushButton_startLive->updateLiveStatus(true);
      }
   }
}

void VhallLiveTitle::ReInit() {
   if (ui.pushButton_startLive) {
      ui.pushButton_startLive->Clear();
   }
}

void VhallLiveTitle::ClearTimeClock() {
   if (NULL != ui.pushButton_startLive) {
      ui.pushButton_startLive->Clear();
   }
}

void VhallLiveTitle::StartTimeClock(bool bStart) {
   if (NULL != ui.pushButton_startLive) {
      if (bStart) {
         ui.pushButton_startLive->updateLiveStatus(true);
      }
      else {
         ui.pushButton_startLive->updateLiveStatus(false);
      }
   }
}

long VhallLiveTitle::GetLiveStartTime() {
   if (ui.pushButton_startLive) {
      return ui.pushButton_startLive->GetClassTime();
   }
   return 0;
}

void VhallLiveTitle::SetStreamButtonStatus(bool isActive) {
   if (NULL != ui.pushButton_startLive) {
      ui.pushButton_startLive->updateLiveStatus(isActive);
   }
}

void VhallLiveTitle::SetLivingState(bool start) {
   mbIsLiving = start;
}

long VhallLiveTitle::GetLiveTime()
{
   return ui.pushButton_startLive->GetClassTime();
}

void VhallLiveTitle::startToSpeakCoutDown(){
   miCountDownTime = COUNTDOWNTIME;
   ui.pushButton_toSpeak->setText(QStringLiteral("等待应答(%1s)").arg(miCountDownTime));
   if (nullptr == mpCountDownTimer) {
      mpCountDownTimer = new QTimer(this);
      connect(mpCountDownTimer, &QTimer::timeout, this, &VhallLiveTitle::slot_CountDown);
   }

   if (nullptr != mpCountDownTimer) {
      mpCountDownTimer->start(1000);
   }
}

void VhallLiveTitle::SetPressedPoint(QPoint pos) {
   mPressPoint = pos;
}

void VhallLiveTitle::stopHandsUpCountDown()
{
   if (nullptr != mpCountDownTimer) {
      mpCountDownTimer->stop();
   }
   if (miCountDownTime == 0) {
      emit sigApplyTimeout();
   }
   miCountDownTime = 0;

   if (ui.pushButton_toSpeak->text().trimmed().compare(DOWNMIC) != 0 ) {
      ui.pushButton_toSpeak->setText(APPLY_SPEAK);
   }
   
}

void VhallLiveTitle::slot_CountDown(){
   if (miCountDownTime > 0) {
      
      ui.pushButton_toSpeak->setText(QStringLiteral("等待应答(%1s)").arg(--miCountDownTime));
   }
   else {
      stopHandsUpCountDown();
   }
}

void VhallLiveTitle::Slot_HandsUp()
{
   if (nullptr != mpCountDownTimer && mpCountDownTimer->isActive()) {
      emit sigCancelClicked();
   }
   else {
      if (mbIsLiving && ui.pushButton_toSpeak->text().trimmed().compare(APPLY_SPEAK) ==0 ) {
         startToSpeakCoutDown();
      }
      
      emit sigToSpeakClicked();
      
   }
}

void VhallLiveTitle::Slot_OnClickedStartLiveCtrlBtn() {
   //if (ui.RecordBtnWdg->GetRecordState() == QStringLiteral("录制中") || ui.RecordBtnWdg->GetRecordState() == QStringLiteral("暂停")) {
   //    emit sigRecordType(eRecordReTyp_Stop);
   //}
   if (NULL == mpStartLiveBtnTimer)
   {
      mpStartLiveBtnTimer = new QTimer(this);
      if (mpStartLiveBtnTimer) {
         mpStartLiveBtnTimer->setSingleShot(true);
         connect(mpStartLiveBtnTimer, &QTimer::timeout, this, &VhallLiveTitle::slot_OnStartLiveTimeOut);
      }
   }

   //ClearTimeClock();
   if (mpStartLiveBtnTimer) {
      ui.pushButton_startLive->setEnabled(false);
      mpStartLiveBtnTimer->start(2000);
   }
   emit Sig_OnClickedStartLiveCtrlBtn();
}