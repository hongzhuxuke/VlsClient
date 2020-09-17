#include "vhalljoinactivity.h"
#include "ui_vhalljoinactivity.h"
#include "title_button.h"
#include <QDebug>
#include <QDesktopWidget>
#include "pub.Const.h"
#include "pub.Struct.h"
#include "Msg_MainUI.h"
#include "MainUIIns.h"
#include "pathmanager.h"
#include "ConfigSetting.h"
#include <QIntValidator>
#include "ConfigSetting.h"

#define LEDT_NICKNAME_LENTH 30   //用户昵称长度

VhallJoinActivity::VhallJoinActivity(QWidget *parent) :
CBaseDlg(parent),
   ui(new Ui::VhallJoinActivity) {
   ui->setupUi(this);
   QIntValidator validator(0, 999999999);
   ui->activityId->setValidator(&validator);
   ui->ledPassword->setValidator(&validator);
	ui->label->setOpenExternalLinks(true);
	setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint /*| Qt::WindowStaysOnTopHint*/);
   setAttribute(Qt::WA_TranslucentBackground);
   setAutoFillBackground(false);

   m_pBtnClose = new TitleButton(this);
   m_pBtnClose->loadPixmap(":/sysButton/close_button");
   m_pBtnClose->setToolTip(CLOSE_WND);
   m_pBtnClose->setStyleSheet(TOOLTIPQSS);
   ui->layout_close->addWidget(m_pBtnClose);
   
   connect(m_pBtnClose, SIGNAL(clicked()), this, SLOT(Slot_Close()));
   m_pBtnClose->setFocusPolicy(Qt::NoFocus);
   
   ui->widget_title->installEventFilter(this);
   ui->activityId->setPlaceholderText(INPUT_LIVE_ID);
   this->setWindowTitle(GUEST_RTC);
   pixmap = QPixmap(":/interactivity/joinActivity");
   QString styleSheet = "QToolButton{border-image: url(\":/interactivity/enterLive\");}"
                        "QToolButton:hover{border-image: url(\":/interactivity/enterLiveHover\");}"
                        "QToolButton:pressed{border-image: url(\":/interactivity/enterLive\");}\
						QToolButton:disabled{border-image: url(\":/interactivity/enter_live_unenable.png\")";
   
   ui->btn_enter->setStyleSheet(styleSheet);
   
   m_pWaiting = new VhallWaiting(this);
   m_pWaiting->SetPixmap(":/interactivity/cb");
   m_pWaiting->Append(":/interactivity/c1");
   m_pWaiting->Append(":/interactivity/c2");
   m_pWaiting->Append(":/interactivity/c3");
   m_pWaiting->Append(":/interactivity/c4");
   m_pWaiting->Append(":/interactivity/c5");
   m_pWaiting->Append(":/interactivity/c6");
   m_pWaiting->Append(":/interactivity/c7");
   m_pWaiting->Append(":/interactivity/c8");
   m_pWaiting->Append(":/interactivity/c9");
   m_pWaiting->Append(":/interactivity/c10");
   m_pWaiting->Append(":/interactivity/c11");
   m_pWaiting->Append(":/interactivity/c12");

   QString configPath = CPathManager::GetConfigPath();
   QString roomID = ConfigSetting::ReadString(configPath, GROUP_DEFAULT, KEY_ROOM_ID, "");
   QString userNickName = ConfigSetting::ReadString(configPath, GROUP_DEFAULT, KEY_NICK_NAME, "");
   QString roomPwd = ConfigSetting::ReadString(configPath, GROUP_DEFAULT, KEY_ROOM_PWD, "");
   if (!roomID.isEmpty()) {
       ui->activityId->setText(roomID);
   }
   if (!userNickName.isEmpty()) {
       ui->ledtNick->setText(userNickName);
   }
   if (!roomPwd.isEmpty()) {
       ui->ledPassword->setText(roomPwd);
   }
}
void VhallJoinActivity::CenterWindow(QWidget * parent){
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

VhallJoinActivity::~VhallJoinActivity() {
   delete ui;
}
QString VhallJoinActivity::GetActivityId(){
   return ui->activityId->text();
}
void VhallJoinActivity::ToJoin(){
	QString roomid = ui->activityId->text().trimmed();
   if (roomid.length() != 9) {
      ui->label->setText(INPUT_RTCLIVE_ID);
      return ;
   }

	QString strNick = ui->ledtNick->text().trimmed();
	if (ui->ledtNick->text().isEmpty()){
		ui->label->setText(INPUT_RTCLIVE_JOINNAME);
		return;
	}

   QString roomPwd = ui->ledPassword->text();
   if (roomPwd.isEmpty()) {
      ui->label->setText(INPUT_RTCLIVE_PWD);
      return;
   } else if (roomPwd.length() < CHECK_PWD_LEN) {
      ui->label->setText(INPUT_RTCLIVE_PWD_LIMIT);
      return;
   }


   m_pWaiting->Show();
   m_pBtnClose->setEnabled(false);
   ui->label->clear();
   ui->btn_enter->setEnabled(false);
   ui->activityId->setEnabled(false);
   emit this->SigJoinActivity(roomid, strNick, roomPwd, mbPwdLogin);
   setFocus();

   QString configPath = CPathManager::GetConfigPath();
   ConfigSetting::writeValue(configPath, GROUP_DEFAULT, KEY_ROOM_ID, roomid);
   ConfigSetting::writeValue(configPath, GROUP_DEFAULT, KEY_NICK_NAME, strNick);
   ConfigSetting::writeValue(configPath, GROUP_DEFAULT, KEY_ROOM_PWD, roomPwd);
}

void VhallJoinActivity::on_activityId_returnPressed(){
   ToJoin();
}

void VhallJoinActivity::on_btn_enter_clicked(){
   ToJoin();
}

void VhallJoinActivity::SlotJoinActivityEnd(bool bOk,QString str) {
   m_pWaiting->Close();
   m_pBtnClose->setEnabled(true);

   qDebug()<<"VhallJoinActivity::SlotJoinActivityEnd"<<bOk<<str;
   if(bOk) {
      this->accept();
   }
   else {
      ui->label->setText(str);
   }
   
   ui->btn_enter->setEnabled(true);
   ui->activityId->setEnabled(true);
   if (bOk) {
      emit SigClose();
      close();
   }
}
bool VhallJoinActivity::eventFilter(QObject *obj, QEvent *e) {
   if(ui->widget_title == obj) {
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
   else if(this == obj) {
      if(e->type()==QEvent::Move) {
         m_pWaiting->Repos();
      }
   }
   
   return QWidget::eventFilter(obj, e);
}
void VhallJoinActivity::paintEvent(QPaintEvent *event){
   QPainter painter(this);
   painter.drawPixmap(rect(),this->pixmap);
   QDialog::paintEvent(event);
}

void VhallJoinActivity::Slot_Close() {
   emit SigClose();
   close();
   ui->label->setText("");
}

void VhallJoinActivity::closeEvent(QCloseEvent *event) {
   Slot_Close();
}

void VhallJoinActivity::setDefultNick(QString& strNickName)
{
	if (!strNickName.trimmed().isEmpty())
	{
		ui->ledtNick->setText(strNickName);
	}
   setFocus();
}

void VhallJoinActivity::on_ledtNick_textEdited(const QString & text)
{
	if (text.trimmed().length()>LEDT_NICKNAME_LENTH)
	{
		ui->label->setText(INPUT_RTCLIVE_NICKLEN);
	}
	ui->ledtNick->setMaxLength(LEDT_NICKNAME_LENTH);
	ui->ledtNick->setText(text.trimmed());
	ui->ledtNick->setMaxLength(32767);
	if (ui->ledtNick->text().trimmed().length() < LEDT_NICKNAME_LENTH)
	{
		ui->label->setText("");
	}
	
}

void VhallJoinActivity::CloseWaiting() {
   if (m_pWaiting) {
      m_pWaiting->Close(false);
      m_pBtnClose->setEnabled(true);
   }
   ui->btn_enter->setEnabled(true);
}

void VhallJoinActivity::SetPwdLogin(bool pwdLogin) {
   mbPwdLogin = pwdLogin;
}

void VhallJoinActivity::showEvent(QShowEvent *) {
   ui->activityId->setFocus();
}


