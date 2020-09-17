#include "vhdesktopvideo.h"
#include "ui_vhdesktopvideo.h"
#include "MainUIIns.h"
#include "Msg_OBSControl.h"
#include "IOBSControlLogic.h"
#include "Msg_MainUI.h"
#include "CRPluginDef.h"
#include <QScreen>
#include <QDebug>
#include <windows.h>

#define LINE_WIDTH   8

VHDesktopVideo::VHDesktopVideo(QWidget *parent) :
    QWidget(parent),
    m_mousePressed(false),
    m_insideType(MOUSE_INSIDE_UNKNOWN),
    ui(new Ui::VHDesktopVideo)
{
    ui->setupUi(this);
    videos[0]=(HWND)ui->video0->winId();
    videos[1]=(HWND)ui->video1->winId();
    
    ui->video0->setUpdatesEnabled(false);
    ui->video0->setAttribute(Qt::WA_TranslucentBackground);
    ui->video0->setAttribute(Qt::WA_PaintOnScreen);
    
    ui->video1->setUpdatesEnabled(false);
    ui->video1->setAttribute(Qt::WA_TranslucentBackground);
    ui->video1->setAttribute(Qt::WA_PaintOnScreen);

    
    videoType[0]=false;
    videoType[1]=false;

    nInitX = 0;
    nInitY = 0;

    this->setWindowFlags(
        Qt::FramelessWindowHint |
        Qt::WindowMinimizeButtonHint |
        Qt::Tool|
        Qt::WindowStaysOnTopHint);
    
    ui->video0->installEventFilter(this);
    ui->video1->installEventFilter(this);
    ui->video0->setFocusPolicy(Qt::NoFocus);
    ui->video1->setFocusPolicy(Qt::NoFocus);


    ui->left->installEventFilter(this);
    ui->left_top->installEventFilter(this);
    ui->top->installEventFilter(this);
    ui->right_top->installEventFilter(this);
    ui->right->installEventFilter(this);
    ui->right_down->installEventFilter(this);
    ui->down->installEventFilter(this);
    ui->left_down->installEventFilter(this);
    
    this->installEventFilter(this);
    connect(this,SIGNAL(SigResizeVideo()),this,SLOT(SlotResizeVideo()));
}
VHDesktopVideo::~VHDesktopVideo()
{
    delete ui;
}

bool VHDesktopVideo::eventFilter(QObject *obj, QEvent *event) {

   int type = GetMouseInsideType();
   QEvent::Type eventType = event->type();
   if (eventType == QEvent::MouseMove)
   {
      QMouseEvent* mouseEvent = (QMouseEvent*)event;
      QPoint curPosLocal = mouseEvent->pos();
      MOUSE_INSIDE insideType = GetMouseInsideType();

      QPoint curPosGlobal = this->mapToGlobal(curPosLocal);
      if (!m_mousePressed)	// 鼠标未按下
      {
         if (insideType == MOUSE_INSIDE_VIDEO0 || insideType == MOUSE_INSIDE_VIDEO1){
            SetCursor(LoadCursor(NULL, IDC_ARROW));
         }
      } 
      else	// 鼠标已按下
      {
         if (m_insideType == MOUSE_INSIDE_VIDEO0 || m_insideType == MOUSE_INSIDE_VIDEO1) {
            QMouseEvent *me = dynamic_cast<QMouseEvent *>(event);
            this->move(m_StartPoint + me->globalPos() - m_PressPoint);
         } 
         else {
            UpdateWindowsSize(m_insideType,event);
            if (m_insideType == MOUSE_INSIDE_RIGHT || m_insideType == MOUSE_INSIDE_DOWN || m_insideType == MOUSE_INSIDE_RIGHT_DOWN) {
               //nothing to do;
            }
            else if (m_insideType == MOUSE_INSIDE_TOP || m_insideType == MOUSE_INSIDE_LEFT || m_insideType == MOUSE_INSIDE_LEFT_TOP) {
               QRect ret = this->rect();
               this->move(m_rightBottomPos - QPoint(rect().width(),rect().height()));
            }
            else if (m_insideType == MOUSE_INSIDE_RIGHT_TOP) {
               QRect ret = this->rect();
               this->move(m_leftBottomPos - QPoint(0, rect().height()));
            }
            else if (m_insideType == MOUSE_INSIDE_LEFT_DOWN) {
               QRect ret = this->rect();
               this->move(m_rightTopPos - QPoint(rect().width(), 0));
            }
         }
      }
   }
   else if (eventType == QEvent::MouseButtonPress)
   {
      QMouseEvent* mouseEvent = (QMouseEvent*)event;
      if (mouseEvent->button() == Qt::LeftButton) {
         m_mousePressed = true;
         m_insideType = GetMouseInsideType();
         QMouseEvent *me = dynamic_cast<QMouseEvent *>(event);
         if (me) {
            m_PressPoint = me->globalPos();
            m_StartPoint = this->pos();
            m_startRect = this->geometry();
            m_lastWndRect = this->geometry();


            m_leftTopPos = this->pos();
            m_leftBottomPos = this->pos() + QPoint(0,this->rect().height());
            m_rightTopPos = this->pos() + QPoint(this->rect().width(), 0);
            m_rightBottomPos = this->pos() + QPoint(this->rect().width(), this->rect().height());
         }
      }
   } 
   else if (eventType == QEvent::MouseButtonRelease) 
   {
      m_insideType = MOUSE_INSIDE_UNKNOWN;
      m_mousePressed = false;
      this->setCursor(Qt::ArrowCursor);
      m_LastPos = this->pos();
      m_lastWndRect = this->geometry();


      m_leftTopPos = this->pos();
      m_leftBottomPos = this->pos() + QPoint(0, this->rect().height());
      m_rightTopPos = this->pos() + QPoint(this->rect().width(), 0);
      m_rightBottomPos = this->pos() + QPoint(this->rect().width(), this->rect().height());
   }
       

   return QWidget::eventFilter(obj, event);
}
HWND VHDesktopVideo::LockVideo(QString deviceID,int index) {
    HWND ret= NULL;
    if (videoType[0] == false) {
       videoType[0] = true;
       ret = videos[0];
       videoDeviceID[0] = deviceID;

       if (bSwap == false && videoType[1] == true) {
          ui->gridLayout->removeWidget(ui->video0);
          ui->gridLayout->removeWidget(ui->video1);
          ui->gridLayout->addWidget(ui->video1);
          ui->gridLayout->addWidget(ui->video0);
          bSwap = true;
       }
    } else if (videoType[1] == false) {
       videoType[1] = true;
       ret = videos[1];
       videoDeviceID[1] = deviceID;
       if (bSwap == true && videoType[0] == true) {
          ui->gridLayout->removeWidget(ui->video0);
          ui->gridLayout->removeWidget(ui->video1);
          ui->gridLayout->addWidget(ui->video0);
          ui->gridLayout->addWidget(ui->video1);

          bSwap = false;
       }
    }

    ResizeVideo();
    return ret;
}
void VHDesktopVideo::UnlockVideo(HWND hwnd){
    if(videos[0]==hwnd) {
        videoType[0]=false;
        videoDeviceID[0] = QString();
    }
    else if(videos[1]==hwnd) {
        videoType[1]=false;
        videoDeviceID[1] = QString();
    }
    emit SigResizeVideo();
}

void VHDesktopVideo::ResetLockVideo() {
   for (int i = 0; i < 2; i++) {
      videoType[i] = false;
      videoDeviceID[i] = QString();
   }
}

void VHDesktopVideo::SlotResizeVideo(){
   ResizeVideo();
}

void VHDesktopVideo::ResizeVideo(){
    bool isHidden = this->isHidden();
    int count = 0;
    if(videoType[0]==true) {
        ui->video0->show();
        count++;
    }
    else {
        ui->video0->hide();
    }

    if(videoType[1]==true) {
        ui->video1->show();
        count++;
    }
    else {
        ui->video1->hide();
    }

    //初始状态
    if (m_LastPos.rx() == 0 && m_LastPos.ry() == 0) {
       if (count == 1) {
          if (!ui->video0->isHidden()) {
             ui->video0->setFixedSize(RENDER_WND_MIN_WIDTH, RENDER_WND_MIN_HEIGHT);
          } else {
             ui->video1->setFixedSize(RENDER_WND_MIN_WIDTH, RENDER_WND_MIN_HEIGHT);
          }
          this->setFixedSize(MAIN_WND_MIN_WIDTH, MAIN_WND_MIN_HEIGHT_ONE_VIEW);
       } else if (count == 2) {
          ui->video0->setFixedSize(RENDER_WND_MIN_WIDTH, RENDER_WND_MIN_HEIGHT);
          ui->video1->setFixedSize(RENDER_WND_MIN_WIDTH, RENDER_WND_MIN_HEIGHT);
          this->setFixedSize(MAIN_WND_MIN_WIDTH, MAIN_WND_MIN_HEIGHT_DOUBLE_VIEW);
       } else {
          ui->video0->setFixedSize(RENDER_WND_MIN_WIDTH, RENDER_WND_MIN_HEIGHT);
          ui->video1->setFixedSize(RENDER_WND_MIN_WIDTH, RENDER_WND_MIN_HEIGHT);
          this->setFixedSize(MAIN_WND_MIN_WIDTH, MAIN_WND_MIN_HEIGHT_DOUBLE_VIEW);
       }
       VH::CComPtr<IOBSControlLogic> pObsControlLogic = NULL;
       DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IOBSControl, IID_IOBSControlLogic, pObsControlLogic, return);
       if (pObsControlLogic->IsHasMonitorSource() && count != 0) {
          this->show();
          this->move(nInitX, nInitY);
          m_LastPos.setX(nInitX);
          m_LastPos.setY(nInitY);
       } else {
          this->hide();
       }
       m_lastWndRect = this->rect();
    }
    //当前共享过程中，保持上次状态。
    else {
       //根据上次的高度，获取新高度。

       int videoHeight = m_lastWndRect.height() / 2 <= SCALE_NEW_HEIGHT(m_lastWndRect.width() - LINE_WIDTH) ? m_lastWndRect.height() - LINE_WIDTH : (m_lastWndRect.height() - LINE_WIDTH) / 2;
       qDebug() << "m_lastWndRect widht height " << m_lastWndRect << " " << videoHeight;
       if (count == 1) {
          if (!ui->video0->isHidden()) {
             ui->video0->setFixedSize(m_lastWndRect.width() - LINE_WIDTH, videoHeight);
          } else {
             ui->video1->setFixedSize(m_lastWndRect.width() - LINE_WIDTH, videoHeight);
          }
          this->setFixedSize(m_lastWndRect.width(), videoHeight  + LINE_WIDTH);
       } else if (count == 2) {
          ui->video0->setFixedSize(m_lastWndRect.width() - LINE_WIDTH, videoHeight);
          ui->video1->setFixedSize(m_lastWndRect.width() - LINE_WIDTH, videoHeight);
          this->setFixedSize(m_lastWndRect.width(), videoHeight * 2  + LINE_WIDTH);
       } else {
          ui->video0->setFixedSize(m_lastWndRect.width() - LINE_WIDTH, videoHeight);
          ui->video1->setFixedSize(m_lastWndRect.width() - LINE_WIDTH, videoHeight);
          this->setFixedSize(m_lastWndRect.width(), videoHeight * 2  + LINE_WIDTH);
       }

       VH::CComPtr<IOBSControlLogic> pObsControlLogic = NULL;
       DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IOBSControl, IID_IOBSControlLogic, pObsControlLogic, return);
       if (pObsControlLogic->IsHasMonitorSource() && count != 0) {
          this->show();
          this->move(m_LastPos);
       } else {
          this->hide();
       }
    }

    this->adjustSize();
    this->repaint();
}
void VHDesktopVideo::Show(){
   this->show();   
   ResizeVideo();
}
void VHDesktopVideo::Hide(){
   this->hide();
}
void VHDesktopVideo::InitPos()
{
   RECT rt;
   SystemParametersInfo(SPI_GETWORKAREA,   0,   &rt,   0);
   nInitX = rt.right - INIT_POS_OFF_WIDTH;
   nInitY = rt.top  + 40;
   move(nInitX, nInitY);
}

VHDesktopVideo::MOUSE_INSIDE VHDesktopVideo::GetMouseInsideType() {
   if (ui->left->rect().contains(ui->left->mapFromGlobal(this->cursor().pos()))) {
      return MOUSE_INSIDE_LEFT;
   } 
   else if (ui->top->rect().contains(ui->top->mapFromGlobal(this->cursor().pos()))) {
      return MOUSE_INSIDE_TOP;
   } 
   else if (ui->right->rect().contains(ui->right->mapFromGlobal(this->cursor().pos()))) {
      return MOUSE_INSIDE_RIGHT;
   } 
   else if (ui->down->rect().contains(ui->down->mapFromGlobal(this->cursor().pos()))) {
      return MOUSE_INSIDE_DOWN;
   } 
   else if (ui->video0->rect().contains(ui->video0->mapFromGlobal(this->cursor().pos()))) {
      return MOUSE_INSIDE_VIDEO0;
   } 
   else if (ui->video1->rect().contains(ui->video1->mapFromGlobal(this->cursor().pos()))) {
      return MOUSE_INSIDE_VIDEO1;
   } 
   else if (ui->left_top->rect().contains(ui->left_top->mapFromGlobal(this->cursor().pos()))) {
      return MOUSE_INSIDE_LEFT_TOP;
   }
   else if (ui->left_down->rect().contains(ui->left_down->mapFromGlobal(this->cursor().pos()))) {
      return MOUSE_INSIDE_LEFT_DOWN;
   } 
   else if (ui->right_top->rect().contains(ui->right_top->mapFromGlobal(this->cursor().pos()))) {
      return MOUSE_INSIDE_RIGHT_TOP;
   }
   else if (ui->right_down->rect().contains(ui->right_down->mapFromGlobal(this->cursor().pos()))) {
      return MOUSE_INSIDE_RIGHT_DOWN;
   }
   else {
      return MOUSE_INSIDE_UNKNOWN;
   }
}

void VHDesktopVideo::UpdateWindowsSize(MOUSE_INSIDE type, QEvent *event) {

   if (type != MOUSE_INSIDE_LEFT && type != MOUSE_INSIDE_RIGHT &&
       type != MOUSE_INSIDE_TOP && type != MOUSE_INSIDE_DOWN &&
       type != MOUSE_INSIDE_LEFT_DOWN && type != MOUSE_INSIDE_LEFT_TOP &&
       type != MOUSE_INSIDE_RIGHT_DOWN && type != MOUSE_INSIDE_RIGHT_TOP) {
       return;
   }

   QMouseEvent *me = dynamic_cast<QMouseEvent *>(event);
   if (me == NULL) {
      return;
   }

   QRect geometry = m_startRect;
   QPoint diff = me->globalPos() - this->m_PressPoint;

   int oldX = geometry.x();
   int oldY = geometry.y();
   int oldWidth = geometry.width();
   int oldHeight = geometry.height();

   int newX;
   int newY;
   int newWidth;
   int newHeight;

   bool bDoubleView = false;
   if (!ui->video0->isHidden() && !ui->video1->isHidden()) {
      bDoubleView = true;
   }

   if (MOUSE_INSIDE_LEFT == type){
      newX = oldX + diff.x();
      newY = oldY;
      newWidth = oldWidth - diff.x();
      newHeight = SCALE_NEW_HEIGHT(newWidth);
      if (bDoubleView) {
         newHeight = newHeight * 2;
      }
   }
   else if(MOUSE_INSIDE_RIGHT == type){
      newX = oldX;
      newY = oldY;
      newWidth = oldWidth + diff.x();
      newHeight = SCALE_NEW_HEIGHT(newWidth);
      if (bDoubleView) {
         newHeight = newHeight * 2;
      }
   } 
   else if (MOUSE_INSIDE_TOP == type) {
      newX = oldX;
      newY = oldY + diff.y();
      newHeight = oldHeight - diff.y();
      newWidth = SCALE_NEW_WIDTH(newHeight);
      if (bDoubleView) {
         newWidth = newWidth / 2;
      }
   }
   else if (MOUSE_INSIDE_DOWN == type) {
      newX = oldX;
      newY = oldY;
      newHeight = oldHeight + diff.y();
      newWidth = SCALE_NEW_WIDTH(newHeight);
      if (bDoubleView) {
         newWidth = newWidth / 2;
      }
   }
   else if (MOUSE_INSIDE_LEFT_TOP == type)  {
      newX = oldX + diff.x();
      newY = oldY + diff.y();
      newWidth = oldWidth - diff.x();
      newHeight = SCALE_NEW_HEIGHT(newWidth);
      if (bDoubleView) {
         newHeight = newHeight * 2;
      }
   }
   else if (MOUSE_INSIDE_LEFT_DOWN == type) {
      newX = oldX + diff.x();
      newY = oldY;
      newHeight = oldHeight + diff.y();
      newWidth = SCALE_NEW_WIDTH(newHeight);
      if (bDoubleView) {
         newWidth = newWidth / 2;
      }
   }
   else if (MOUSE_INSIDE_RIGHT_TOP == type) {
      newX = oldX;
      newY = oldY + diff.y();
      newHeight = oldHeight - diff.y();
      newWidth = SCALE_NEW_WIDTH(newHeight);
      if (bDoubleView) {
         newWidth = newWidth / 2;
      }
   }
   else if (MOUSE_INSIDE_RIGHT_DOWN == type) {
      newX = oldX;
      newY = oldY;
      newHeight = oldHeight + diff.y();
      newWidth = SCALE_NEW_WIDTH(newHeight);
      if (bDoubleView) {
         newWidth = newWidth / 2;
      }
   }

   if (newWidth < MAIN_WND_MIN_WIDTH) {
      newWidth = MAIN_WND_MIN_WIDTH;
   } 
   
   if (!ui->video0->isHidden() && ui->video1->isHidden() && newHeight < MAIN_WND_MIN_HEIGHT_ONE_VIEW) {
      newHeight = MAIN_WND_MIN_HEIGHT_ONE_VIEW;
   } else if (ui->video0->isHidden() && !ui->video1->isHidden() && newHeight < MAIN_WND_MIN_HEIGHT_ONE_VIEW) {
      newHeight = MAIN_WND_MIN_HEIGHT_ONE_VIEW;
   } else if (!ui->video0->isHidden() && !ui->video1->isHidden() && newHeight < MAIN_WND_MIN_HEIGHT_DOUBLE_VIEW) {
      newHeight = MAIN_WND_MIN_HEIGHT_DOUBLE_VIEW;
   }

   qDebug() << "type  newWidth newHeight" << type  <<" "<< newWidth << " " << newHeight;
   geometry.setX(newX);
   geometry.setY(newY);
   geometry.setWidth(newWidth);
   geometry.setHeight(newHeight);

   m_lastWndRect = geometry;

   if (!ui->video0->isHidden() && ui->video1->isHidden()) {
      ui->video0->setFixedSize(geometry.width() - LINE_WIDTH, geometry.height() - LINE_WIDTH);
   } else if (!ui->video1->isHidden() && ui->video0->isHidden()) {
      ui->video1->setFixedSize(geometry.width() - LINE_WIDTH, geometry.height() - LINE_WIDTH);
   } else if (!ui->video1->isHidden() && !ui->video0->isHidden()) {
      ui->video0->setFixedSize(geometry.width() - LINE_WIDTH, geometry.height() / 2 - LINE_WIDTH);
      ui->video1->setFixedSize(geometry.width() - LINE_WIDTH, geometry.height() / 2 - LINE_WIDTH);
   }

   if (!this->isMaximized()) {
      this->setFixedSize(geometry.width(), geometry.height());
   }

   this->adjustSize();
}

void VHDesktopVideo::MoveWindows() {
   int dx = this->startPoint.x() + this->cursor().pos().x() - this->pressPoint.x();
   int dy = this->startPoint.y() + this->cursor().pos().y() - this->pressPoint.y();
   int aScreenWidth = GetSystemMetrics(SM_CXVIRTUALSCREEN);
   int aScreenHeight = GetSystemMetrics(SM_CYVIRTUALSCREEN);

   if (dx<0) {
      dx = 0;
   }

   if (dx + this->width()>aScreenWidth) {
      dx = aScreenWidth - this->width();
   }

   if (dy<0) {
      dy = 0;
   }

   if (dy + this->height()>aScreenHeight) {
      dy = aScreenHeight - this->height();
   }

   this->move(dx, dy);
   m_LastPos.setX(dx);
   m_LastPos.setY(dy);
}

void VHDesktopVideo::SlotResetInitSize(bool) {
   m_LastPos = QPoint(0, 0);
   ResizeVideo();
}

void VHDesktopVideo::SlotCloseCamera(bool) {
   QString deviceID;
   if (!ui->video0->isHidden() && !ui->video1->isHidden()) {
      if (ui->video0->rect().contains(ui->video0->mapFromGlobal(this->cursor().pos()))) {
         videoType[0] = false;
         deviceID = this->videoDeviceID[0];
      } else if (ui->video1->rect().contains(ui->video1->mapFromGlobal(this->cursor().pos()))) {
         videoType[1] = false;
         deviceID = this->videoDeviceID[1];
      }
   }
   else if (ui->video1->isHidden() && !ui->video0->isHidden()) {
      videoType[0] = false;
      deviceID = this->videoDeviceID[0];
   }
   else if (!ui->video1->isHidden() && ui->video0->isHidden()) {
      videoType[1] = false;
      deviceID = this->videoDeviceID[1];
   }
}

void VHDesktopVideo::SetDesktopShare(bool start) {
   m_LastPos = QPoint(0,0);
   m_lastWndRect = QRect(0,0,0,0);
   m_bInitSize = start;
}

