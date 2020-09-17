#include "CTabItemDlg.h"
#include <QPropertyAnimation>
#include "CTabItemDlg.h"
#include <QDebug>
#include <QGraphicsDropShadowEffect> 
#include <QDesktopWidget>
#include "ICommonData.h"
#include "VH_Macro.h"
#include "MainUIIns.h"
#include "Msg_MainUI.h"
#include "CRPluginDef.h"
#include "DebugTrace.h"
#include "ConfigSetting.h"

#define MAX_WIDTH    780
#define MIN_WIDTH    322
#define MIN_HEIGHT   580


CTabItemDlg::CTabItemDlg(QWidget *parent)
   : QDialog(parent) {
   ui.setupUi(this);
   setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint | Qt::Dialog /*| Qt::WindowStaysOnTopHint*/);
   //setAttribute(Qt::WA_TranslucentBackground);
   //setAutoFillBackground(false);
   ui.btnClose->loadPixmap(":/sysButton/close_button");
   ui.pushButton_max->loadPixmap(":/sysButton/max_button");
   ui.pushButton_fresh->loadPixmap(":/sysButton/img/sysButton/fresh.png");

   ui.btnClose->setToolTip(CLOSE_TIPS);
   connect(ui.btnClose, SIGNAL(clicked()), this, SIGNAL(sig_doubleClickedTitleBar()));
   connect(ui.pushButton_max, SIGNAL(clicked()), this, SIGNAL(sig_clickMaxBtn()));
   connect(ui.pushButton_fresh, SIGNAL(clicked()), this, SLOT(slot_OnFresh()));
   ui.widget_top->installEventFilter(this);
   ui.widget_bottom->installEventFilter(this);
   setFixedHeight(MIN_HEIGHT);
}

CTabItemDlg::~CTabItemDlg() {
}

void CTabItemDlg::SetBtnShow(bool is_show_fresh, bool is_show_max) {
   is_show_fresh == true ? ui.pushButton_fresh->show() : ui.pushButton_fresh->hide();
   is_show_max == true ? ui.pushButton_max->show() : ui.pushButton_max->hide();
}

void CTabItemDlg::SetMaxSizeState() {
   if (mbIsMax) {
      ui.pushButton_max->loadPixmap(":/sysButton/max_button");
      setFixedWidth(MIN_WIDTH);
      setFixedHeight(MIN_HEIGHT);
      mbIsMax = false;
      ui.pushButton_max->setToolTip(MAX_TIPS);
   }
   else {
      ui.pushButton_max->loadPixmap(":/sysButton/normal_button");
      setFixedWidth(MAX_WIDTH);
      mbIsMax = true;
      ui.pushButton_max->setToolTip(MIN_TIPS);
   }
   ui.pushButton_max->SetEnabled(true);
   ui.pushButton_max->repaint();
}

void CTabItemDlg::showEvent(QShowEvent *e) {
   setAttribute(Qt::WA_Mapped);
   QWidget::showEvent(e);
}

bool CTabItemDlg::eventFilter(QObject *obj, QEvent *event) {
   if (obj && event) {
      if (event->type() == QEvent::Enter) {
         if (obj == ui.widget_top) {
            setCursorShape(TOP);
         } else if (ui.widget_bottom == obj){
            setCursorShape(BUTTOM);
         } else {
            setCursorShape(NORMAL);
         }
      }
      else if (event->type() == QEvent::Leave) {
         setCursorShape(NORMAL);
      }
   }
   return QWidget::eventFilter(obj, event);
}

void CTabItemDlg::leaveEvent(QEvent *event) {
   setCursorShape(NORMAL);
}

void CTabItemDlg::slot_OnFresh() {
   emit sig_OnFresh();
}

void CTabItemDlg::mousePressEvent(QMouseEvent *event) {
   if (event) {
      mPressPoint = this->pos() - event->globalPos();
      mIsMoved = true;
   }
   m_rtPreGeometry = geometry();
   m_ptViewMousePos = event->globalPos();
}

void CTabItemDlg::mouseMoveEvent(QMouseEvent *event) {
   //窗体不是最大的话就改变鼠标的形状
   if (cursor().shape() == Qt::ArrowCursor) {
      if (event && (event->buttons() == Qt::LeftButton) && mIsMoved && m_bIsEnableMove) {
         this->move(event->globalPos() + mPressPoint);
         this->raise();
      }
      return;
   }
   else if (cursor().shape() == Qt::SizeVerCursor && !mIsMoved) {
      setCursorShape(NORMAL);
   }
   else {
      //获取当前的点，这个点是全局的
      QPoint ptCurrentPos = QCursor::pos();
      //计算出移动的位置，当前点 - 鼠标左键按下的点
      QPoint ptMoveSize = ptCurrentPos - m_ptViewMousePos;
      QRect rtTempGeometry = m_rtPreGeometry;
      if (cursor().shape() == Qt::SizeVerCursor) {
         switch (m_iCalCursorPos)
         {
         case TOP:
            rtTempGeometry.setTop(m_rtPreGeometry.top() + ptMoveSize.y());
            break;
         case BUTTOM:
            rtTempGeometry.setBottom(m_rtPreGeometry.bottom() + ptMoveSize.y());
            break;
         default:
            break;
         }
         //移动窗体，如果比最小窗体大，就移动
         if (rtTempGeometry.height() >= MIN_HEIGHT) {
            setGeometry(rtTempGeometry);
            move(rtTempGeometry.x(), rtTempGeometry.y());
            setFixedHeight(rtTempGeometry.height());
         }
      }
   }
}

void CTabItemDlg::mouseReleaseEvent(QMouseEvent *) {
   mIsMoved = false;
   setCursorShape(NORMAL);
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

void CTabItemDlg::setCursorShape(int CalPos)
{
   Qt::CursorShape cursor;
   m_iCalCursorPos = CalPos;
   switch (CalPos)
   {
   case TOP:
      cursor = Qt::SizeVerCursor;
      break;
   case BUTTOM:
      cursor = Qt::SizeVerCursor;
      break;
   default:
      cursor = Qt::ArrowCursor;
      break;
   }
   setCursor(cursor);
}


void CTabItemDlg::keyPressEvent(QKeyEvent *event) {
   switch (event->key()) {
   case Qt::Key_Escape:
      break;
   default:
      QDialog::keyPressEvent(event);
   }
}

QObjectList CTabItemDlg::GetChildren()
{
   return ui.frame->children();
}

void CTabItemDlg::SetTitle(const QString& strTitle) {
   mTitleStr = strTitle;
   setWindowTitle(strTitle);
   ui.labTitle->setText(strTitle);
   TRACE6("%s strTitle %s\n", __FUNCTION__, strTitle.toStdString().c_str());
}


void CTabItemDlg::RemoveCenterWdg(QWidget* pWdg) {
   if (nullptr != pWdg) {
      ui.verticalLayout_center->removeWidget(pWdg);
      mbIsMax = true;
      SetMaxSizeState();
   }
}

void CTabItemDlg::AddCenterWdg(QWidget* pWdg) {
   VH::CComPtr<ICommonData> pCommonData;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
   ClientApiInitResp respInitData;
   pCommonData->GetInitApiResp(respInitData);
   //if (respInitData.player == FLASH_LIVE_CREATE_TYPE) {
   //   ui.pushButton_max->hide();
   //}
   //else {
   //   ui.pushButton_max->show();
   //}

   if (nullptr != pWdg) {
      ui.verticalLayout_center->addWidget(pWdg);
      pWdg->show();
      //QString str = QString("#%1{border-bottom-left-radius : 10px; border-bottom-right-radius: 10px; }").arg(pWdg->objectName());
      //pWdg->setStyleSheet(str);
   }
}

bool CTabItemDlg::event(QEvent * event) {  

   if (event->type() == QEvent::MouseButtonDblClick){
      emit sig_doubleClickedTitleBar();
      TRACE6("%s sig_doubleClickedTitleBar strTitle %s\n", __FUNCTION__, mTitleStr.toStdString().c_str());
      return true;
   }
   return QWidget::event(event);
}