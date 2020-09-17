#pragma once
#include <QWidget>
#include "IOBSControlLogic.h"
class QMenu;
class QAction;


class RenderWnd : public QWidget {
   Q_OBJECT
public:
   RenderWnd(QWidget *parent = 0);
   ~RenderWnd();

   bool Create();
   void Destroy();

   void SetRenderPos(bool isCenterPos);

private slots:
   void processSource(int opType);
   void slot_OnChangeView();
signals:
   void processGraphic(int opType);
   void sig_ChangeView(bool isCenterPos);
   void sig_enterEvent(bool isCenterPos);
   void sig_LeaveEvent();
protected:
   virtual void mousePressEvent(QMouseEvent *mouseEvent);
   virtual void mouseMoveEvent(QMouseEvent *mouseEvent);
   virtual void mouseReleaseEvent(QMouseEvent *mouseEvent);
   virtual void mouseDoubleClickEvent(QMouseEvent *mouseEvent);
   virtual void enterEvent(QEvent *event);
   virtual void leaveEvent(QEvent *);
   virtual void resizeEvent(QResizeEvent *);
   bool nativeEvent(const QByteArray &eventType, void *message, long *result);
   void showEvent(QShowEvent *e);

private:

   QMenu* m_pContextMenu = NULL;
   QAction* m_pActionFullScreen = NULL;
   QAction* m_pActionMoveTop = NULL;
   QAction* m_pActionMoveBottom = NULL;
   QAction* m_pActionModify = NULL;
   QAction* m_pActionDelete = NULL;

   VH::CComPtr<IOBSControlLogic> m_pOBSControlLogic;

   bool mbIsInCenterPos = true;
   bool mbButtonPressed = false;
   int mLastButtnEvent = 0;
};

