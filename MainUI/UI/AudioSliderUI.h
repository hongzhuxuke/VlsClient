#ifndef AUDIOSLIERUI_H
#define AUDIOSLIERUI_H
#include <QTimer>

#include "VHDialog.h"

class QGridLayout;
class QSlider;
class QLabel;
class QFrame;
class ToolButton;
class AudioSliderUI : public VHDialog {
   Q_OBJECT
public:
   explicit AudioSliderUI(QDialog *parent = 0);
   ~AudioSliderUI();
   void LoadPixmap(QString pixmapPath);
   void HideMuteBtn();
   void HideSettingBtn();
   
   void WillClose(int );
   void StopClose();
   void SetVolumn(int);
protected:
   virtual void paintEvent(QPaintEvent *);   
   virtual void focusOutEvent(QFocusEvent *);
   virtual void enterEvent(QEvent *);
   virtual void leaveEvent(QEvent *);
   bool eventFilter(QObject *obj, QEvent *ev);
public:
   bool Create();
   void Destroy();
   void Show(int volumn,QPoint btnGlobalPos);
public slots:
   void Timeout();
signals:
   void sigVolumnChanged(int);
private slots:
   void OnVolumnChangedSlot(int);
   void OnMuteClick();
   void OnSettingClick();
private:
   QSlider *m_pVolumeSlider = NULL;
   ToolButton *m_Mute = NULL;
   ToolButton *m_VoSettingBtn = NULL;
   QFrame *m_HLine = NULL;
   bool m_bEnter = false;
   QWidget *m_bottomWidget = NULL;
   QPixmap mBackgroundPixmap;
   QTimer mCloseTimer;
};

#endif // AUDIOSLIERUI_H
