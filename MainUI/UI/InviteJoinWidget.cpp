#include "InviteJoinWidget.h"
#include <QPainter>
#include <QApplication>
#include <QDesktopWidget>
#include <QJsonObject>
#include "MainUIIns.h"

InviteJoinWidget::InviteJoinWidget(QWidget *parent)
: CBaseDlg(parent)
{
   ui.setupUi(this);
	mnMaxTimeOut = 10;
   setWindowFlags(Qt::FramelessWindowHint | Qt::Window | Qt::Tool);
   setAttribute(Qt::WA_TranslucentBackground);
   setAutoFillBackground(false);

   m_pTimer = new QTimer(this);
   connect(m_pTimer,SIGNAL(timeout()),this,SLOT(Slot_ShowTimeOut()));
   connect(ui.toolButtonAgree, SIGNAL(clicked()), this, SLOT(Slot_GuestAgreeHostInvite()));
   connect(ui.toolButtonRefuse, SIGNAL(clicked()), this, SLOT(Slot_GuestRefuseHostInvite()));   
   //ui.toolButtonAgree->setStyleSheet(
   //   "QToolButton{border-image: url(\":/interactivity/replyYes\");}"
   //   "QToolButton:hover{border-image: url(\":/interactivity/replyYesHover\");}"
   //   "QToolButton:pressed {border-image: url(\":/interactivity/replyYes\");}"
   //   "QToolButton:disabled {border-image: url(\":/interactivity/replyYes\");}"
   //);

   //ui.toolButtonRefuse->setStyleSheet(
   //   "QToolButton{border-image: url(\":/interactivity/replyNo\");}"
   //   "QToolButton:hover{border-image: url(\":/interactivity/replyNoHover\");}"
   //   "QToolButton:pressed {border-image: url(\":/interactivity/replyNo\");}"
   //   "QToolButton:disabled {border-image: url(\":/interactivity/replyNo\");}"
   //);
   
   mpixmap = QPixmap(":/interactivity/askBackground");

   m_pCloseBtn = new TitleButton();
   m_pCloseBtn->loadPixmap(":/sysButton/close_button");
   connect(m_pCloseBtn,SIGNAL(clicked()),this,SLOT(Slot_GuestRefuseHostInvite()));
   ui.layout_close->addWidget(m_pCloseBtn);
   ui.widget_title->installEventFilter(this);
}

InviteJoinWidget::~InviteJoinWidget() {
}

void InviteJoinWidget::InitTimeOut(int timeout) {
	mnMaxTimeOut = timeout;
}

void InviteJoinWidget::StartTimer() {
   if (m_pTimer) {
      m_pTimer->stop();
      m_pTimer->start(1000);
   }
   nTimeCount = mnMaxTimeOut;
   QString notice = QStringLiteral("%1s后自动拒绝").arg(nTimeCount);
   //ui.toolButtonRefuse->setText(notice);
   ui.label_TimeNotice->setText(notice);
   mbIsStartTimer = true;
}


int InviteJoinWidget::GetLeftTime() {
   return mbIsStartTimer ? nTimeCount : -1;
}

void InviteJoinWidget::CenterWindow(QWidget* parent) {
   int x = 0;
   int y = 0;
   if (NULL == parent) {
      const QRect rect = QApplication::desktop()->availableGeometry();
      x = rect.left() + (rect.width() - width()) / 2;
      y = rect.top() + (rect.height() - height()) / 2;
   } else {
      QPoint point(0, 0);
      point = parent->mapToGlobal(point);
      x = point.x() + (parent->width() - width()) / 2;
      y = point.y() + (parent->height() - height()) / 2;
   }
   move(x, y);
}

void InviteJoinWidget::Slot_GuestAgreeHostInvite() {
   if (m_pTimer) {
      m_pTimer->stop();
   }
	QJsonObject body;
	body["ac"] = "Agree";
	SingletonMainUIIns::Instance().reportLog(STR_VH_LIVE_OPERATE, eLogRePortK_Interaction_HorJonined, L"HorJonined", body);
   emit Sig_GuestAgreeHostInvite();
}

void InviteJoinWidget::Slot_GuestRefuseHostInvite() {
   if (m_pTimer) {
      m_pTimer->stop();
   }
	QJsonObject body;
	body["ac"] = "Refuse";
	SingletonMainUIIns::Instance().reportLog(STR_VH_LIVE_OPERATE, eLogRePortK_Interaction_HorJonined, L"HorJonined", body);
   emit Sig_GuestRefuseHostInvite();
}
void InviteJoinWidget::paintEvent(QPaintEvent *){
   QPainter painter(this);
   painter.drawPixmap(rect(),this->mpixmap);
}
bool InviteJoinWidget::eventFilter(QObject *o, QEvent *e) {
    if(o==ui.widget_title)
    {
       if(e->type()==QEvent::MouseButtonPress) {
           this->pressPoint=this->cursor().pos();
           this->startPoint=this->pos();
       }
       else if(e->type()==QEvent::MouseMove) {
           int dx=this->cursor().pos().x()-this->pressPoint.x();
           int dy=this->cursor().pos().y()-this->pressPoint.y();
           this->move(this->startPoint.x()+dx,this->startPoint.y()+dy);
       }
       else if (e->type() == QEvent::MouseButtonRelease) {
          QPoint pos = this->pos();
          QRect rect = QApplication::desktop()->availableGeometry(pos);
          QRect wnd_rect = this->frameGeometry();

          if (pos.y() > rect.height() - 150) {
             pos.setY(rect.height() - 150);
             this->move(pos);
          }
          else if (pos.y() < rect.y()) {
             this->move(pos + QPoint(0, +100));
          }
          else if (wnd_rect.x() >= rect.x() + rect.width() - 100) {
             this->move(QPoint(wnd_rect.x(), wnd_rect.y()) + QPoint(-100, 0));
          }
          else if (wnd_rect.x() + this->width() < rect.x() + 100) {
             this->move(QPoint(wnd_rect.x(), wnd_rect.y()) + QPoint(100, 0));
          }
       }
    }
    return QWidget::eventFilter(o,e);
}

void InviteJoinWidget::Slot_ShowTimeOut() {
   if (nTimeCount == 0) {
      emit Sig_GuestRefuseHostInvite();
      m_pTimer->stop();
		QJsonObject body;
		body["ac"] = "TimeOut";
      mbIsStartTimer = false;
		SingletonMainUIIns::Instance().reportLog(STR_VH_LIVE_OPERATE, eLogRePortK_Interaction_HorJonined, L"HorJonined", body);
   } else {
	   QString notice = QStringLiteral("%1s后自动拒绝").arg(nTimeCount);
      //ui.toolButtonRefuse->setText(notice);
	   ui.label_TimeNotice->setText(notice);
      nTimeCount--;
   }
}

void InviteJoinWidget::showEvent(QShowEvent *) {
   if (m_pTimer) {
		m_pTimer->stop();
      m_pTimer->start(1000);
   }
   nTimeCount = mnMaxTimeOut;
   QString notice = QStringLiteral("%1s后自动拒绝").arg(nTimeCount);
   //ui.toolButtonRefuse->setText(notice);
   ui.label_TimeNotice->setText(notice);
   mbIsStartTimer = false;
   
}


