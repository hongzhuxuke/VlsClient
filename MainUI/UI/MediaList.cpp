#include <QLayout>
#include <QFile>
#include <QTextStream>
#include <iostream>
#include "MediaList.h"

#define CONFIGPATH L"flielist.dat"
using namespace std;

#if _MSC_VER >= 1600  
#pragma execution_character_set("utf-8")  
#endif  
MediaList::MediaList(QWidget *parent)
   : QWidget(parent)
{  
   mChosenFileNum = 0;
   mListedFileNum = 0;
   Layout = new QVBoxLayout(this);
   initFileList();
   Layout->setContentsMargins(16, 4, 16, 0);
   Layout->setSizeConstraint(QLayout::SetMinAndMaxSize);
   setLayout(Layout);

}

MediaList::~MediaList()
{

}

void MediaList::initFileList() {
  /* MediaInfo *MD1 = new MediaInfo();
   MD1->isSelected = true;
   MD1->strFileName = tr("≤‚ ‘111111111111111111111111");
   MediaInfo *MD2 = new MediaInfo();
   MD2->isSelected = false;
   MD2->strFileName = tr("≤‚ ‘2");
   mMediaInfoList.append(*MD1);
   mMediaInfoList.append(*MD2);
   mMediaInfoList.append(*MD2);
   mMediaInfoList.append(*MD2);
   mMediaInfoList.append(*MD2);*/

   for (int i = 0; i < mMediaInfoList.length(); i++){
      addSingleItem(&mMediaInfoList[i]);
   }

   //wstring confPath = GetAppPath() + CONFIGPATH;
   //QString qsConfPath = qsConfPath.fromStdWString(confPath);
   //QFile f(qsConfPath);
   //if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
   //{
   //   cout << "Open failed." << endl;
   //   return -1;
   //}
   //QTextStream txtInput(&f);
   //QString lineStr;
   //while (!txtInput.atEnd())
   //{
   //   lineStr = txtInput.readLine();
   //   cout << lineStr << endl;
   //}
   //f.close();

   for (int i = 0; i < mMediaInfoList.length(); i++){
      if (mMediaInfoList[i].isSelected)
         mChosenFileNum++;
   }
   mListedFileNum = mMediaInfoList.length();
   emit updateNum();
}

void MediaList::addSingleItem(MediaInfo* mediaInfo){
   MediaItem *newItem = new MediaItem(mediaInfo, this);
   QLine line;
   Layout->addWidget(newItem, Qt::AlignTop);
   mMediaItemList.append(newItem);
   connect(newItem, SIGNAL(deleteItems(MediaItem*)), this, SLOT(on_deleteItem_slot(MediaItem*)));
   connect(newItem, SIGNAL(CheckBoxState(int)), this, SLOT(on_CheckBoxState_slot(int)));
}

void MediaList::addSelectFiles(QStringList selectFiles){
   int addFileNum = selectFiles.length();
   for (int i = 0; i < selectFiles.length(); i++){
      MediaInfo *newItem = new MediaInfo();
      newItem->isSelected = true;
      newItem->strFileName = selectFiles[i];
      addSingleItem(newItem);
   }

   mChosenFileNum = mChosenFileNum + addFileNum;
   mListedFileNum = mListedFileNum + addFileNum;
   emit updateNum();
}

void MediaList::on_deleteItem_slot(MediaItem* willDeleteDevice) {
   if (willDeleteDevice->mCheckedState)
      mChosenFileNum--;
   mListedFileNum--;
   emit updateNum();

   Layout->removeWidget(willDeleteDevice);
   delete willDeleteDevice;
   mMediaItemList.removeOne(willDeleteDevice);
}

void MediaList::on_clearAll_slot(){
   for (int i = 0; i < mMediaItemList.length(); i++){
      Layout->removeWidget(mMediaItemList[i]);
      delete mMediaItemList[i];
   }
   mMediaItemList.clear();
   update();

   mChosenFileNum = 0;
   mListedFileNum = 0;
   emit updateNum();
}

void MediaList::on_CheckBoxState_slot(int state){
   if (state)
      mChosenFileNum++;
   else
      mChosenFileNum--;
   emit updateNum();
}