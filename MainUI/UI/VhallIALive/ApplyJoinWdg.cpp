#include "ApplyJoinWdg.h"
#include <QPainter>
#include <QApplication>
#include <QDesktopWidget>
#include <QJsonObject>
#include "pathmanager.h"

#define MAX_TIME_OUT 15

//CREATE_WND_FUNCTION(ApplyJoinWdg);
ApplyJoinWdg::ApplyJoinWdg(QWidget *parent)
: CBaseDlg(parent)
{
    ui.setupUi(this);
    mnMaxTimeOut = MAX_TIME_OUT;
    setWindowFlags(Qt::FramelessWindowHint | Qt::Window | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground);
    setAutoFillBackground(false);

    m_pTimer = new QTimer(this);
    connect(m_pTimer,SIGNAL(timeout()),this,SLOT(Slot_ShowTimeOut()));
    connect(ui.pushButtonAgree, SIGNAL(clicked()), this, SLOT(Slot_HostAgreeApply()));
    connect(ui.toolButtonRefuse, SIGNAL(clicked()), this, SLOT(Slot_HostRefuseApply()));
    mpixmap = QPixmap(":/interactivity/askBackground");

    m_pCloseBtn = new TitleButton();
    m_pCloseBtn->loadPixmap(":/sysButton/close_button");
    connect(m_pCloseBtn,SIGNAL(clicked()),this,SLOT(Slot_HostRefuseApply()));
    ui.layout_close->addWidget(m_pCloseBtn);
    ui.widget_title->installEventFilter(this);
}

ApplyJoinWdg::~ApplyJoinWdg() {
}

void ApplyJoinWdg::InitTimeOut(int timeout) {
	mnMaxTimeOut = timeout;
}

QString ApplyJoinWdg::GetUid()
{
	return mStrUid;
}

void ApplyJoinWdg::SetUid(const QString& strUid, const QString nickName){
	mStrUid = strUid;
    QString showName = CPathManager::GetString( nickName.isEmpty() ? strUid : nickName, 14);
	ui.labApplyInfo->setText(showName + QStringLiteral("申请上麦"));
}

void ApplyJoinWdg::CenterWindow(QWidget* parent) {
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

void ApplyJoinWdg::Slot_HostAgreeApply() {
    if (m_pTimer) {
        m_pTimer->stop();
    }
	emit sig_AgreeUpper(mStrUid);
	emit sig_Destroy(mStrUid);
	hide();
}

void ApplyJoinWdg::Slot_HostRefuseApply() {
   hide();
   if (m_pTimer) {
      m_pTimer->stop();
   }
   emit sig_RejectUpper(mStrUid);
   emit sig_Destroy(mStrUid);
}

void ApplyJoinWdg::paintEvent(QPaintEvent *){
   QPainter painter(this);
   painter.drawPixmap(rect(),this->mpixmap);
}

bool ApplyJoinWdg::eventFilter(QObject *o, QEvent *e) {
    if(o==ui.widget_title)
    {
       if(e->type()==QEvent::MouseButtonPress) {
           this->pressPoint=this->cursor().pos();
           this->startPoint=this->pos();
           mbPressed = true;
       }
       else if(e->type()==QEvent::MouseMove) {
          if (mbPressed) {
             int dx = this->cursor().pos().x() - this->pressPoint.x();
             int dy = this->cursor().pos().y() - this->pressPoint.y();
             this->move(this->startPoint.x() + dx, this->startPoint.y() + dy);
          }
       }
       else if (e->type() == QEvent::MouseButtonRelease) {
          mbPressed = false;
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

void ApplyJoinWdg::Slot_ShowTimeOut() {
   if (nTimeCount == 0) {
      emit Slot_HostRefuseApply();
      m_pTimer->stop();
   } else {
      QString notice = QStringLiteral("%1s后自动拒绝").arg(nTimeCount);
      ui.label_TimeNotice->setText(notice);
      nTimeCount--;
   }
}

void ApplyJoinWdg::hide()
{
   if (m_pTimer && m_pTimer->isActive()) {
      m_pTimer->stop();
   }
   nTimeCount = 0;
   CBaseDlg::hide();
}

void ApplyJoinWdg::showEvent(QShowEvent *) {
   if (m_pTimer) {
        m_pTimer->stop();
        m_pTimer->start(1000);
   }
   ui.label_TimeNotice->setText(QStringLiteral("%1s后自动拒绝").arg(mnMaxTimeOut));
   nTimeCount = mnMaxTimeOut;
}


