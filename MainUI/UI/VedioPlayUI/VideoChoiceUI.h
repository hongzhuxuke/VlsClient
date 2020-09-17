#ifndef _VIDEOCHOICEUI_H_
#define _VIDEOCHOICEUI_H_
#include <QObject>
#include <QModelIndexList>
#include <QMouseEvent>
#include <QCheckBox>
#include "VHDialog.h"
#include "vhallmenuwidgetitem.h"
#include "NoiseTooltip.h"
class TitleWidget;
class PushButton;
class QListWidget;
class QLabel;
class QComboBox;
class QToolButton;
class QListWidgetItem;
class PushButton;

#define MAXFILECOUNT 10
class VideoChoiceItem : public QWidget {
   Q_OBJECT
      
public:
   explicit VideoChoiceItem (QWidget *parent=NULL);
   ~VideoChoiceItem();
   void setFileName(const QString &);
   void setWidth(int);
   QString getFilename();
   void setEndLine(bool);
   void setChecked(bool);
   void FlushLabelColor(bool pressed);
protected:
   //virtual void paintEvent(QPaintEvent *);
   virtual void mouseReleaseEvent(QMouseEvent *);
   virtual void mousePressEvent(QMouseEvent *);
   virtual void leaveEvent(QEvent *);
public slots:
   void closeBtnClicked();
signals:
   void closeClicked(VideoChoiceItem *);
private:
   QString m_qText;
   QLabel *m_pLabel = NULL;
   PushButton *m_pCloseButton = NULL;
   bool m_bDrawLine = false;
   bool m_bChecked = false;
};

class VideoChoiceUI : public VHDialog {
   Q_OBJECT   

public:
   explicit VideoChoiceUI(QDialog *parent = 0);
   ~VideoChoiceUI();
public:
   bool Create();
   void Destroy();
   void Clear();
   QStringList GetPlayList(int &currentIndex);   
   int getCircleMode();   
   void AppendFileList(const QStringList &,int);
   void SetCircleMode(int mode);
   void CenterWindow(QWidget* parent);
   void RightTip(const QString &);
   void FlashTips();
   void FlushItems();
   void FLushFileList(bool accept);
   void SetLiveType(int type);
   bool IsEnablePlayOutAudio();
   void SetCurrentPlayOutState(bool enable);
   void HidePlayOutCheck(bool hide);

signals:
   void SigSyncFileList(QStringList,int);
public slots:
   void ChoiceFiles();
   void PlayListDoubleClicked(const QModelIndex &index);
   void DeleteItem(VideoChoiceItem *);
   void currentItemChanged(QListWidgetItem*,QListWidgetItem*);
   void SureClicked();
   void OnCircleModeChanged(int);
protected:
   virtual void paintEvent(QPaintEvent *);
   virtual void showEvent(QShowEvent *);
   virtual bool eventFilter(QObject *obj, QEvent *ev);

private:
   TitleWidget *m_pTitleBar = NULL;
   QListWidget *m_pListWidget = NULL;
   QComboBox *m_pCircleCombox = NULL;
   QLabel *m_rightTipLabel = NULL;
   QLabel *mFileTypeTips = NULL;
   QPushButton *m_pSureButton = NULL;
   bool m_bPaintItem = true;
   int mLiveType = 0;
   QLabel *mPlayOutTipsLable;
   NoiseTooltip* mpPlayTipsWdg = NULL;
   QCheckBox* mpPlayOutCheckBox = NULL;
};
#endif
