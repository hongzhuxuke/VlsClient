#ifndef CFLOATRENDERWDG_H
#define CFLOATRENDERWDG_H

#include <QWidget>
#include <QPoint>
#include "ui_CFloatRenderWdg.h"

#define RENDER_WND_WIDTH   230
#define RENDER_WND_HEIGHT  135

#define RENDER_MOVETITLE_HEIGHT 18

class CFloatRenderWdg : public QWidget {
   Q_OBJECT

public:
   CFloatRenderWdg(QWidget *parent = 0);
   ~CFloatRenderWdg();
   void AppendRender(QString& id, QWidget* render);
   void RemoveRender(QString& id);
   void ClearRenderWidgets();
   int ResetWndSize(bool bShowAllUser = false);
   void SetStayTop();
   const QMap<QString, QWidget*> GetRenderWidgetsMap() const;
   void SetRenderWidgetHidden(QString& id,bool hide);
   virtual bool eventFilter(QObject *obj, QEvent *event);

private:
   Ui::CFloatRenderWdg ui;
   QMap<QString, QWidget *> m_pRenderWidgets;
   bool m_bMousePressed = false;
   QPoint m_wndPos;
   QPoint m_mousePos;
};

#endif // CFLOATRENDERWDG_H
