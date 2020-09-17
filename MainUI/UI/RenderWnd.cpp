#include "stdafx.h"
#include <QApplication>
#include <QMouseEvent>
#include <QSignalMapper>
#include <QMenu>
#include <QAction>
#include "RenderWnd.h"
#include "DebugTrace.h"
#include "MainUIIns.h"
#include "Msg_OBSControl.h"
#include "ConfigSetting.h"
#include "pathManager.h"

RenderWnd::RenderWnd(QWidget *parent) : QWidget(parent) {
   /*¹Ø±ÕäÖÈ¾ÇøÓòµÄÖØ»æÊÂ¼þ£¬·ÀÖ¹ÉÁË¸*/
   setUpdatesEnabled(false);
   setAttribute(Qt::WA_TranslucentBackground);
   setAttribute(Qt::WA_PaintOnScreen);
   QPalette palette;
   palette.setColor(QPalette::Background, QColor(255, 0, 0));
   setPalette(palette);
   setAutoFillBackground(true);
   this->setFocusPolicy(Qt::ClickFocus);
}


RenderWnd::~RenderWnd() {

}

bool RenderWnd::Create() {
   m_pContextMenu = new QMenu(this);
   m_pContextMenu->setObjectName("graphicMenu");
   if (NULL == m_pContextMenu) {
      ASSERT(FALSE);
      return true;
   }
   m_pActionFullScreen = new QAction(tr("È«ÆÁ"), NULL);
   m_pActionMoveTop = new QAction(tr("ÖÃÓÚ¶¥²ã"), NULL);
   m_pActionMoveBottom = new QAction(tr("ÖÃÓÚµ×²ã"), NULL);
   m_pActionModify = new QAction(tr("ÐÞ¸Ä"), NULL);
   m_pActionDelete = new QAction(tr("É¾³ý"), NULL);
   if (NULL == m_pActionFullScreen || NULL == m_pActionMoveTop || NULL == m_pActionMoveBottom || NULL == m_pActionModify || NULL == m_pActionDelete) {
      ASSERT(FALSE);
      return true;
   }

   QSignalMapper *signal_mapper = new QSignalMapper(this);
   if (NULL == signal_mapper) {
      ASSERT(FALSE);
      return true;
   }

   signal_mapper->setMapping(m_pActionFullScreen, 0);
   signal_mapper->setMapping(m_pActionMoveTop, 1);
   signal_mapper->setMapping(m_pActionMoveBottom, 2);
   signal_mapper->setMapping(m_pActionModify, 3);
   signal_mapper->setMapping(m_pActionDelete, 4);

   m_pContextMenu->addAction(m_pActionFullScreen);
   m_pContextMenu->addAction(m_pActionMoveTop);
   m_pContextMenu->addAction(m_pActionMoveBottom);
   m_pContextMenu->addSeparator();
   m_pContextMenu->addAction(m_pActionModify);
   m_pContextMenu->addAction(m_pActionDelete);

   connect(m_pActionDelete, SIGNAL(triggered()), signal_mapper, SLOT(map()));
   connect(m_pActionModify, SIGNAL(triggered()), signal_mapper, SLOT(map()));
   connect(m_pActionMoveTop, SIGNAL(triggered()), signal_mapper, SLOT(map()));
   connect(m_pActionMoveBottom, SIGNAL(triggered()), signal_mapper, SLOT(map()));
   connect(m_pActionFullScreen, SIGNAL(triggered()), signal_mapper, SLOT(map()));
   connect(signal_mapper, SIGNAL(mapped(int)), this, SLOT(processSource(int)));

   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IOBSControl, IID_IOBSControlLogic, m_pOBSControlLogic, return false, ASSERT(FALSE));
   return true;
}

void RenderWnd::Destroy() {
   if (NULL != m_pContextMenu) {
      delete m_pContextMenu;
      m_pContextMenu = NULL;
   }

   if (NULL != m_pActionFullScreen) {
      delete m_pActionFullScreen;
      m_pActionFullScreen = NULL;
   }

   if (NULL != m_pActionMoveTop) {
      delete m_pActionMoveTop;
      m_pActionMoveTop = NULL;
   }

   if (NULL != m_pActionMoveBottom) {
      delete m_pActionMoveBottom;
      m_pActionMoveBottom = NULL;
   }

   if (NULL != m_pActionModify) {
      delete m_pActionModify;
      m_pActionModify = NULL;
   }

   if (NULL != m_pActionDelete) {
      delete m_pActionDelete;
      m_pActionDelete = NULL;
   }

   if (NULL != m_pActionFullScreen) {
      delete m_pActionFullScreen;
      m_pActionFullScreen = NULL;
   }
}

void RenderWnd::SetRenderPos(bool isCenterPos) {
    mbIsInCenterPos = isCenterPos;
}

void RenderWnd::mousePressEvent(QMouseEvent *mouseEvent) {
    if (!mbIsInCenterPos) {
        return;
    }
    POINTS pos = { mouseEvent->x(), mouseEvent->y() };
    if (Qt::LeftButton& mouseEvent->button()) {
        if (NULL != m_pOBSControlLogic) {
            //m_pOBSControlLogic->OnMouseEvent(WM_LBUTTONDOWN, pos);
        }
    } else if (Qt::RightButton& mouseEvent->button()) {
        //mGraphics->OnMouseEvent(WM_RBUTTONDOWN, pos);
    }
}

void RenderWnd::mouseMoveEvent(QMouseEvent *mouseEvent) {
    if (!mbIsInCenterPos) {
        return;
    }
    POINTS pos = { mouseEvent->x(), mouseEvent->y() };
    if (NULL != m_pOBSControlLogic) {
        //m_pOBSControlLogic->OnMouseEvent(WM_MOUSEMOVE, pos);
    }
    QWidget::mouseMoveEvent(mouseEvent);
}

void RenderWnd::mouseReleaseEvent(QMouseEvent *mouseEvent) {
    if (!mbIsInCenterPos) {
        return;
    }
    if (NULL == m_pOBSControlLogic) {
        ASSERT(FALSE);
        return;
    }
    POINTS pos = { mouseEvent->x(), mouseEvent->y() };
    if (Qt::LeftButton& mouseEvent->button()) {                  
        m_pOBSControlLogic->OnMouseEvent(WM_LBUTTONUP, pos);
    } else if (Qt::RightButton& mouseEvent->button()) {
        m_pOBSControlLogic->OnMouseEvent(WM_RBUTTONUP, pos);
    }
}

void RenderWnd::enterEvent(QEvent *event) {
    emit sig_enterEvent(mbIsInCenterPos);
}

void RenderWnd::leaveEvent(QEvent *) {
    emit sig_LeaveEvent();
}

void RenderWnd::mouseDoubleClickEvent(QMouseEvent *mouseEvent)
{
    if (!mbIsInCenterPos) {
        return;
    }
    processSource(0);
}

void RenderWnd::resizeEvent(QResizeEvent * re) {
   if (NULL != m_pOBSControlLogic) {
      RECT crc = { 0, 0, re->size().width(), re->size().height() };
      m_pOBSControlLogic->Resize(crc, false);
   }
}
bool RenderWnd::nativeEvent(const QByteArray &eventType, void *message, long *) {
    if (!mbIsInCenterPos) {
        return false;
    }
    Q_UNUSED(eventType);
    MSG* msg = reinterpret_cast<MSG*>(message);
    if (NULL == m_pOBSControlLogic) {
        ASSERT(FALSE);
        return false;
    }
    if (WM_LBUTTONDOWN == msg->message
        || WM_RBUTTONDOWN == msg->message
        || WM_RBUTTONUP == msg->message
        || WM_MOUSEMOVE == msg->message
        || WM_LBUTTONUP == msg->message) {
        POINTS pos;
        pos.x = (short)LOWORD(msg->lParam);
        pos.y = (short)HIWORD(msg->lParam);

        QString dpi = ConfigSetting::ReadString(CPathManager::GetConfigPath(), GROUP_DEFAULT, DESKTOP_DIP, "1");
        if (!dpi.isEmpty() && dpi == "1") {
           double screen_dipSize = ConfigSetting::GetCurrentScreenDPISize(CPathManager::GetConfigPath(), this);
           if (screen_dipSize > 0) {
              pos.x = (double)pos.x / screen_dipSize;
              pos.y = (double)pos.y / screen_dipSize;
           }
        }

        mLastButtnEvent = msg->message;
        m_pOBSControlLogic->OnMouseEvent(msg->message, pos);
        //if Right-Mouse-Click, call delete menu and delete function
        if ((WM_RBUTTONDOWN == msg->message || WM_RBUTTONUP == msg->message) && m_pOBSControlLogic->IsHasNoPersistentSource()) {
            QCursor cur = this->cursor();
            if(m_pOBSControlLogic->GetCurrentItemType()==SRC_MONITOR) {
                m_pActionFullScreen->setDisabled(true);
                m_pActionMoveTop->setDisabled(true);
                m_pActionMoveBottom->setDisabled(true);
            }
            else {
                m_pActionFullScreen->setDisabled(false);
                m_pActionMoveTop->setDisabled(false);
                m_pActionMoveBottom->setDisabled(false);
            }

            //ÊÇ·ñ¿ÉÐÞ¸Ä
            if (m_pOBSControlLogic->IsCanModify()) {
                m_pActionModify->setDisabled(true);
            } else {
                m_pActionModify->setDisabled(false);
            }

            m_pContextMenu->exec(cur.pos());
            return true;
        }
        return false;
    }

    if (WM_SIZE == msg->message) {
        RECT crc = { 0, 0, 0, 0 };
        m_pOBSControlLogic->Resize(crc, true);
    }  
    return false;//QT will processit . such as return QWidget::nativeEvent(eventType, message, result);
}

void RenderWnd::processSource(int opType) {
   //call graphic's delete function
   //mGraphics->ClearSource();
   STRU_OBSCONTROL_PROCESSSRC loProcessSrc;
   loProcessSrc.m_dwType = opType;
   SingletonMainUIIns::Instance().PostCRMessage(
   MSG_OBSCONTROL_PROCESSSRC, &loProcessSrc, sizeof(STRU_OBSCONTROL_PROCESSSRC));
}


void RenderWnd::slot_OnChangeView() {
    emit sig_ChangeView(mbIsInCenterPos);
}

void RenderWnd::showEvent(QShowEvent *e) {
   setAttribute(Qt::WA_Mapped);
   QWidget::showEvent(e);
}
