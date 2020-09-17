#include "stdafx.h"
#include "CFloatRenderWdg.h"

#include <QApplication>
#include <QDesktopWidget>

CFloatRenderWdg::CFloatRenderWdg(QWidget *parent)
: QWidget(parent) {
   ui.setupUi(this);
   SetStayTop();
   ui.widget->installEventFilter(this);
}

CFloatRenderWdg::~CFloatRenderWdg() {

}

void CFloatRenderWdg::AppendRender(QString& id, QWidget* render) {
   QMap<QString, QWidget *>::iterator iter = m_pRenderWidgets.find(id);
   if (iter == m_pRenderWidgets.end()) {
      m_pRenderWidgets[id] = render;
      ui.verticalLayout_2->addWidget(render);
   }
}

void CFloatRenderWdg::RemoveRender(QString& id) {
   QMap<QString, QWidget *>::iterator iter = m_pRenderWidgets.find(id);
   if (iter != m_pRenderWidgets.end()) {
      ui.verticalLayout_2->removeWidget(iter.value());
      m_pRenderWidgets.remove(id);
   }
}

int CFloatRenderWdg::ResetWndSize(bool bShowAllUser/* = false*/) {
   int nRenderCount = m_pRenderWidgets.count();
   if (!bShowAllUser) {
      QMap<QString, QWidget*>::iterator iter = m_pRenderWidgets.begin();
      while (iter != m_pRenderWidgets.end()) {
         if (iter.value()->isHidden()) {
            nRenderCount--;
         }
         iter++;
      }
   }
   int nWndHeight = RENDER_MOVETITLE_HEIGHT + RENDER_WND_HEIGHT *  nRenderCount;
   QRect ret;
   ret.setWidth(RENDER_WND_WIDTH);
   ret.setHeight(nWndHeight);
   setMaximumWidth(RENDER_WND_WIDTH);
   setMinimumWidth(RENDER_WND_WIDTH);
   setMaximumHeight(nWndHeight);
   setMinimumHeight(nWndHeight);
   this->setGeometry(ret);
   QRect r = QApplication::desktop()->availableGeometry();
   int nInitX = r.width() - 280;
   int nInitY = 40;
   this->move(nInitX, nInitY);
   return nRenderCount;
}

const QMap<QString, QWidget*> CFloatRenderWdg::GetRenderWidgetsMap() const {
   return m_pRenderWidgets;
}

void CFloatRenderWdg::ClearRenderWidgets() {
   QMap<QString, QWidget *>::iterator iter = m_pRenderWidgets.begin();
   while (iter != m_pRenderWidgets.end()) {
      ui.verticalLayout_2->removeWidget(iter.value());
      iter++;
   }
   m_pRenderWidgets.clear();
   return;
}

bool CFloatRenderWdg::eventFilter(QObject *obj, QEvent *event) {
   if (obj && event && obj == ui.widget) {
      if (event->type() == QEvent::MouseButtonPress) {
         QMouseEvent *mouseEvent = (QMouseEvent*)event;
         if (mouseEvent && mouseEvent->button() == Qt::LeftButton) {
            m_bMousePressed = true;
            m_mousePos = mouseEvent->globalPos();
            m_wndPos = this->pos();
         }
      } else if (event->type() == QEvent::MouseButtonRelease){
         m_bMousePressed = false;
      } else if (event->type() == QEvent::MouseMove) {
         QMouseEvent *mouseEvent = (QMouseEvent*)event;
         if (mouseEvent) {
            QPoint currentMousePos = mouseEvent->globalPos();
            QPoint offPos = currentMousePos - m_mousePos;
            QPoint moveToPos = m_wndPos + offPos;
            this->move(moveToPos);
         }
      }
   }
   return QWidget::eventFilter(obj, event);
}

void CFloatRenderWdg::SetRenderWidgetHidden(QString& id, bool bHide) {
   QMap<QString, QWidget *>::iterator iter = m_pRenderWidgets.find(id);
   if (iter != m_pRenderWidgets.end()) {
      if (bHide) {
         iter.value()->hide();
      } else {
         iter.value()->show();
      }
   }
   return;
}

void CFloatRenderWdg::SetStayTop() {
   this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint | Qt::WindowStaysOnTopHint | Qt::Tool);
}
