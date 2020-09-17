#ifndef CVOICETRANSLATEWDG_H
#define CVOICETRANSLATEWDG_H

#include <QWidget>
#include <QTimer>
#include "ui_cvoicetranslatewdg.h"

class CVoiceTranslateWdg : public QWidget {
   Q_OBJECT

public:
   CVoiceTranslateWdg(QWidget *parent = 0);
   ~CVoiceTranslateWdg();

   void ResetFontSize(const int size);
   void SetVoiceTranslateText(QString text);
   void SetWndToTop(bool top = false);

   void RegisterMainUI(QWidget *mainUi);

   bool IsTopPos();

protected:
   void mousePressEvent(QMouseEvent *event);

private slots:
   void slot_ClearText();

private:
   Ui::CVoiceTranslateWdg ui;
   bool m_bIsTop = false;
   QWidget *m_pMainUi = NULL;
   QTimer m_cleatTextTimer;
};

#endif // CVOICETRANSLATEWDG_H
