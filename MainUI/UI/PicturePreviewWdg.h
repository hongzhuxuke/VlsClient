#pragma once

#include <QWidget>
#include "ui_PicturePreviewWdg.h"
#include "VHBaseWnd.h"
#include <QPushButton>
#include <list>
#include <map>
#include <QMovie>

class ImagePreviewWdg : public QWidget
{
   Q_OBJECT

public:
   ImagePreviewWdg(QWidget *parent = Q_NULLPTR);
   ~ImagePreviewWdg();

   void InsertPicFiles(const std::list<QString>& pic_list,int index);
   void ShowImage(QEvent* event);

signals:
   void sig_wndRemove();

private slots:
   void slot_OnClose();
   void slot_OnLeft();
   void slot_OnRight();

protected:
   virtual void customEvent(QEvent *);
   virtual void enterEvent(QEvent *event);
   virtual void leaveEvent(QEvent *event);

protected:
   void mousePressEvent(QMouseEvent *);
   void mouseMoveEvent(QMouseEvent *);
   void mouseReleaseEvent(QMouseEvent *);

private:
   void ShowBtn();
   void HideBtn();
   void LoadImageToLabel(const QString& filePath);
private:
   QPoint mPressPoint;
   bool mIsMoved = false;

private:
   Ui::PicturePreviewWdg ui;

   std::map<int, QString> mDownLoadFileMap;
   std::list<QString> mPicFileList;
   QPushButton* mLeftBtn;
   QPushButton* mRightBtn;
   int mFileIndex;
   QMovie *mGifMovie;
};
