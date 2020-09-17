#include "DesktopCameraRenderWdg.h"
#include "WebRtcSDKInterface.h"
#include "AreaCloseWdg.h"
#include <QDesktopWidget>
#define MIN_WIDTH    240
#define MIN_HEIGHT   142

DesktopCameraRenderWdg::DesktopCameraRenderWdg(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    ui.widget_top->installEventFilter(this);
    ui.widget_bottom->installEventFilter(this);
    ui.widget_top_left->installEventFilter(this);
    ui.widget_bottom_left->installEventFilter(this);
    ui.widget_top_right->installEventFilter(this);
    ui.widget_bottom_right->installEventFilter(this);
    ui.widget_left->installEventFilter(this);
    ui.widget_right->installEventFilter(this);
    SetCloseCamera(false);
}

DesktopCameraRenderWdg::~DesktopCameraRenderWdg()
{
}

void DesktopCameraRenderWdg::slot_closeRender() {
   emit sig_closeDesktopCameraRender();
   hide();
   ReSize();
}

void DesktopCameraRenderWdg::ShowCloseBtn() {

}

void DesktopCameraRenderWdg::SetCurrentIndex(int index) {
   ui.stackedWidget->setCurrentIndex(index);
}

void DesktopCameraRenderWdg::mousePressEvent(QMouseEvent *event) {
   if (event) {
      mPressPoint = this->pos() - event->globalPos();
      mIsMoved = true;
   }
   m_rtPreGeometry = geometry();
   m_ptViewMousePos = event->globalPos();
}

void DesktopCameraRenderWdg::showEvent(QShowEvent *e) {
   QWidget::showEvent(e);
   ShowCloseBtn();
}

void DesktopCameraRenderWdg::hideEvent(QHideEvent *e) {
   //if (mCloseBtn) {
   //   mCloseBtn->hide();
   //}
   QWidget::hideEvent(e);
}

void DesktopCameraRenderWdg::mouseMoveEvent(QMouseEvent *event) {
   //窗体不是最大的话就改变鼠标的形状
   if (Qt::WindowMaximized != windowState()) {
      if (cursor().shape() == Qt::ArrowCursor) {
         if (event && (event->buttons() == Qt::LeftButton) && mIsMoved) {
            this->move(event->globalPos() + mPressPoint);
            this->raise();
            ShowCloseBtn();
         }
         return;
      }
      else if ((cursor().shape() == Qt::SizeVerCursor || cursor().shape() == Qt::SizeHorCursor) && !mIsMoved) {
         setCursorShape(NORMAL);
      }
      else {
         //获取当前的点，这个点是全局的
         QPoint ptCurrentPos = QCursor::pos();
         //计算出移动的位置，当前点 - 鼠标左键按下的点
         QPoint ptMoveSize = ptCurrentPos - m_ptViewMousePos;
         QRect rtTempGeometry = m_rtPreGeometry;
         if (cursor().shape() == Qt::SizeVerCursor || cursor().shape() == Qt::SizeHorCursor || cursor().shape() == Qt::SizeFDiagCursor || cursor().shape() == Qt::SizeBDiagCursor) {
            QPoint diff = this->cursor().pos() - this->m_ptViewMousePos;
            int left = rtTempGeometry.x();
            int top = rtTempGeometry.y();
            int right = rtTempGeometry.x() + rtTempGeometry.width();
            int bottom = rtTempGeometry.y() + rtTempGeometry.height();
            int new_left = left;
            int new_top = top;
            int new_right = right;
            int new_bottom = bottom;
            int x_diff = diff.x();
            int y_diff = diff.y();
            int min_width = this->minimumWidth();
            int min_height = this->minimumHeight();

            switch (m_iCalCursorPos)
            {
            case BUTTOM:
               new_left = left + diff.x();
               if (right - new_left < min_width) {
                  new_left = right - min_width;
               }
               new_bottom = bottom + diff.y();
               if (new_bottom - top < min_height) {
                  new_bottom = top + min_height;
               }
               rtTempGeometry.setX(new_left);
               rtTempGeometry.setY(new_top);
               rtTempGeometry.setWidth(double(new_bottom - new_top) * (double)240 / (double)142);
               rtTempGeometry.setHeight(new_bottom - new_top);
               break;
            case RIGHT: {
               new_right = right + diff.x();
               if (new_right - left < min_width) {
                  new_right = left + min_width;
               }
               new_bottom = bottom + diff.y();
               if (new_bottom - top < min_height) {
                  new_bottom = top + min_height;
               }
               rtTempGeometry.setX(new_left);
               rtTempGeometry.setY(new_top);
               rtTempGeometry.setWidth(new_right - new_left);
               rtTempGeometry.setHeight((double)(new_right - new_left) * (double)142 / (double)240);
               break;
            }
            case LEFT:
               new_left = left + diff.x();
               if (right - new_left < min_width) {
                  new_left = right - min_width;
               }
               new_top = top + diff.y();
               if (bottom - new_top < min_height) {
                  new_top = bottom - min_height;
               }
               rtTempGeometry.setX(new_left);
               rtTempGeometry.setY(new_top);
               rtTempGeometry.setWidth(new_right - new_left);
               rtTempGeometry.setHeight((double)(new_right - new_left) * (double)142 / (double)240);
               break;
            case TOPRIGHT:
            case TOPLEFT: 
            case TOP:{
               new_left = left + diff.x();
               if (right - new_left < min_width) {
                  new_left = right - min_width;
               }
               new_top = top + diff.y();
               if (bottom - new_top < min_height) {
                  new_top = bottom - min_height;
               }
               rtTempGeometry.setX(new_left);
               rtTempGeometry.setY(new_top);
               rtTempGeometry.setWidth((double)(new_bottom - new_top) * (double)240 / (double)142);
               rtTempGeometry.setHeight(new_bottom - new_top);
               break;
            }
            case BUTTOMLEFT: {
               new_left = left + diff.x();
               if (right - new_left < min_width) {
                  new_left = right - min_width;
               }
               new_bottom = bottom + diff.y();
               if (new_bottom - top < min_height) {
                  new_bottom = top + min_height;
               }
               rtTempGeometry.setX(new_left);
               rtTempGeometry.setY(new_top);
               rtTempGeometry.setWidth(new_right - new_left);
               rtTempGeometry.setHeight((double)(new_right - new_left) * (double)142 / (double)240);
               break;
            }
            case BUTTOMRIGHT: {
               new_right = right + diff.x();
               if (new_right - left < min_width) {
                  new_right = left + min_width;
               }
               new_bottom = bottom + diff.y();
               if (new_bottom - top < min_height) {
                  new_bottom = top + min_height;
               }
               rtTempGeometry.setX(new_left);
               rtTempGeometry.setY(new_top);
               rtTempGeometry.setWidth(new_right - new_left);
               rtTempGeometry.setHeight((double)(new_right - new_left) * (double)142 / (double)240);
               break;
            }         
            default:
               break;
            }
            //移动窗体，如果比最小窗体大，就移动
            if (rtTempGeometry.height() >= MIN_HEIGHT && rtTempGeometry.width() >= MIN_WIDTH) {
               setGeometry(rtTempGeometry);
               ShowCloseBtn();
            }
         }
      }
   }
}

void DesktopCameraRenderWdg::mouseReleaseEvent(QMouseEvent *) {
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

void DesktopCameraRenderWdg::leaveEvent(QEvent *event) {
   mIsMoved = false;
   setCursorShape(NORMAL);
}

bool DesktopCameraRenderWdg::eventFilter(QObject *obj, QEvent *event) {
   if (obj && event) {
      if (event->type() == QEvent::Enter) {
         if (obj == ui.widget_top) {
            setCursorShape(TOP);
         }
         else if (ui.widget_bottom == obj) {
            setCursorShape(BUTTOM);
         }
         else if (ui.widget_left == obj) {
            setCursorShape(LEFT);
         }
         else if (ui.widget_right == obj) {
            setCursorShape(RIGHT);
         }
         else if (ui.widget_top_left == obj) {
            setCursorShape(TOPLEFT);
         }
         else if (ui.widget_top_right == obj) {
            setCursorShape(TOPRIGHT);
         }
         else if (ui.widget_bottom_left == obj) {
            setCursorShape(BUTTOMLEFT);
         }
         else if (ui.widget_bottom_right == obj) {
            setCursorShape(BUTTOMRIGHT);
         }
         else {
            setCursorShape(NORMAL);
         }
      }
      else if (event->type() == QEvent::Leave) {
         setCursorShape(NORMAL);
      }
   }
   return QWidget::eventFilter(obj, event);
}

void DesktopCameraRenderWdg::setCursorShape(int CalPos) {
   Qt::CursorShape cursor;
   switch (CalPos)
   {
   case TOP:
   case BUTTOM:
      cursor = Qt::SizeVerCursor;
      if (ui.stackedWidget->currentIndex() == 1) {
         this->setUpdatesEnabled(false);
      }
      break;
   case LEFT:
   case RIGHT:
      cursor = Qt::SizeHorCursor;
      if (ui.stackedWidget->currentIndex() == 1) {
         this->setUpdatesEnabled(false);
      }
      break;
   case BUTTOMRIGHT:
   case TOPLEFT:{
      cursor = Qt::SizeFDiagCursor;
      if (ui.stackedWidget->currentIndex() == 1) {
         this->setUpdatesEnabled(false);
      }
      break;      
   }
   case TOPRIGHT:
   case BUTTOMLEFT: {
      cursor = Qt::SizeBDiagCursor;
      if (ui.stackedWidget->currentIndex() == 1) {
         this->setUpdatesEnabled(false);
      }
      break;
   }
   default:
      cursor = Qt::ArrowCursor;
      this->setUpdatesEnabled(true);
      repaint();
      break;
   }
   m_iCalCursorPos = CalPos;
   setCursor(cursor);
}

void DesktopCameraRenderWdg::StartRender() {
   vlive::GetWebRtcSDKInstance()->StartRenderLocalStream(vlive::VHStreamType_AVCapture, (void*)ui.widget->winId());
}

void DesktopCameraRenderWdg::CheckShow() {
   if (mDeviceID.isEmpty() || mIndex == -1) {
      return;
   }
   show();
   ShowCloseBtn();
}

QString DesktopCameraRenderWdg::GetDevId() {
   return mDeviceID;
}

HWND DesktopCameraRenderWdg::LockVideo(QString deviceID, int index,bool isDesktop) {
   mDeviceID = deviceID;
   mIndex = index;
   if (isDesktop) {
      show();
   } else{
      hide();
      ReSize();
   }
   return (HWND)ui.widget->winId();
}

void DesktopCameraRenderWdg::UnlockVideo(HWND wnd) {
   if((HWND)ui.widget->winId() == wnd){
      ClearInfo();
   }
}

void DesktopCameraRenderWdg::ClearInfo() {
   mDeviceID.clear();
   mIndex = -1;
   hide();
   ReSize();
}

void DesktopCameraRenderWdg::SetCloseCamera(bool close) {
   if (close) {
      ui.stackedWidget->setCurrentIndex(0);
   } else {
      ui.stackedWidget->setCurrentIndex(1);
      if (!isHidden()) {
         StartRender();
      }
   }
   repaint();
}

void DesktopCameraRenderWdg::ReSize() {
   ui.label_closeCamera->hide();
   QSize size(MIN_WIDTH, MIN_HEIGHT);
   resize(size);
}