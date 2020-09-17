#ifndef MEDIALIST_H
#define MEDIALIST_H

#include <QWidget>
#include "MediaItem.h"

class MediaItem;
class QVBoxLayout;
class MediaList : public QWidget
{
   Q_OBJECT

public:
    MediaList(QWidget *parent);
    ~MediaList();
public:
   int mChosenFileNum;
   int mListedFileNum;

private:
   QVBoxLayout *Layout = NULL;
signals:
   void updateNum();

private slots:
   //void updateNum();
   void on_deleteItem_slot(MediaItem*);
   void on_CheckBoxState_slot(int);
   void on_clearAll_slot();

private:
   void initFileList();
   void addSingleItem(MediaInfo* mediaInfo);
public:
   void addSelectFiles(QStringList selectFiles);
   void clearAll();
public:
   QList<MediaItem *> mMediaItemList;
   QList<MediaInfo> mMediaInfoList;
};

#endif // MEDIALIST_H
