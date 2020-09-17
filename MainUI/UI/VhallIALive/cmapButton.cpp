#include "cmapButton.h"

CMapButton::CMapButton(QWidget *parent)
	: QToolButton(parent)
{
	
}

CMapButton::~CMapButton()
{
}

void CMapButton::setMapButton(CMapButton* pbtn)
{
   mpBtn = pbtn;
	connect(this, &CMapButton::sig_hover, pbtn, &CMapButton::slot_hover);
	connect(this, &CMapButton::sig_leave, pbtn, &CMapButton::slot_leave);
	connect(pbtn, &CMapButton::sig_hover, this, &CMapButton::slot_hover);
	connect(pbtn, &CMapButton::sig_leave, this, &CMapButton::slot_leave);
}

void CMapButton::setEnabled(bool enable)
{
	slot_leave();
	QToolButton::setEnabled(enable);
}

void CMapButton::EnterButton() {
   emit sig_hover();
   setFocus();
   m_bEnter = true;
   if (this->isEnabled()) {
      setStyleSheet(mStrHoverStyle);
      emit sig_hover();
      if (mpBtn) {
         mpBtn->slot_hover();
      }
   }
   QRect rect = this->geometry();
}

void CMapButton::LeaveButton() {
   emit sig_leave();
   if (this->isEnabled())
   	emit sig_leave();
   QRect rect = this->geometry();
   m_bEnter = false;
   setStyleSheet(mStrLeaveStyle);
   if (mpBtn) {
      mpBtn->slot_leave();
   }
}

void CMapButton::setHoverStyle(const QString & str)
{
	mStrHoverStyle = str;
	mStrHoverStyle.trimmed().isEmpty()&& mStrLeaveStyle.trimmed().isEmpty() ? hide() : show();
}
void CMapButton::setLeaveStyle(const QString & str)
{
	mStrLeaveStyle = str;
	mStrHoverStyle.trimmed().isEmpty() && mStrLeaveStyle.trimmed().isEmpty() ? hide() : show();
	setStyleSheet(str);
	//QString strsd = str;
	//globalToolManager->GetDataManager()->WriteLog("CMapButton::setLeaveStyle objcName :%s QSS %s: enabled: %d",
	//	 this->objectName().toLatin1().data(), strsd.toLatin1().data(), this->isEnabled());
}

void CMapButton::slot_hover() {
	setStyleSheet(mStrHoverStyle);
	//QString strsd = mStrHoverStyle;
	//globalToolManager->GetDataManager()->WriteLog("CMapButton::slot_hover objcName :%s QSS:%s enabled:%d",
	//	this->objectName().toLatin1().data(), strsd.toLatin1().data(), this->isEnabled());
}

void CMapButton::slot_leave() {
	setStyleSheet(mStrLeaveStyle);
	//QString strsd = mStrLeaveStyle;
	//globalToolManager->GetDataManager()->WriteLog("CMapButton::slot_leave objcName :%s QSS:%s enabled:%d",
	//	this->objectName().toLatin1().data(), strsd.toLatin1().data(), this->isEnabled());
}

void CMapButton::enterEvent(QEvent *event)
{
 //  setFocus();
 //  m_bEnter = true;
 //  if (this->isEnabled()) {
 //     setStyleSheet(mStrHoverStyle);
 //     emit sig_hover();
 //     if (mpBtn) {
 //        mpBtn->slot_hover();
 //     }
 //  }
	//QRect rect = this->geometry();
}

void CMapButton::leaveEvent(QEvent *event)
{
	//if (this->isEnabled())
	//	emit sig_leave();
	//QRect rect = this->geometry();
 //  m_bEnter = false;
 //  setStyleSheet(mStrLeaveStyle);
 //  if (mpBtn) {
 //     mpBtn->slot_leave();
 //  }
}

void CMapButton::focusOutEvent(QFocusEvent * e)
{
   QToolButton::focusOutEvent( e);
   if (!m_bEnter) {
      //hide();
      emit sigfocusOut();
      //this->close();
      return;
   }
}