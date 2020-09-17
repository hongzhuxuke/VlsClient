#ifndef MULTIMEDIAUI_H
#define MULTIMEDIAUI_H

#include "VHDialog.h"

class QVBoxLayout;
class ToolButton;
class MultiMediaUI : public VHDialog {
   Q_OBJECT
public:
   explicit MultiMediaUI(QDialog *parent = 0);
   ~MultiMediaUI();

   void AddVoiceTranslate();
   void RemoveVoiceTranslate();

protected:
   void paintEvent(QPaintEvent *);
   void showEvent(QShowEvent *);
   virtual void focusOutEvent(QFocusEvent *);
   virtual void enterEvent(QEvent *);
   virtual void leaveEvent(QEvent *);

public:
   bool Create();
   void Destroy();

private slots:
   void OnVedioPlayClick();
   void OnAddImageClick();
   void OnAddTextClick();
   void OnVoiceTranslateClick();

private:
   QVBoxLayout* m_pMainLayout = NULL;
   ToolButton *m_pVoiceTranslate = NULL;
   QPixmap mPixmap;
   bool m_bEnter = false;
};

#endif // MULTIMEDIAUI_H
