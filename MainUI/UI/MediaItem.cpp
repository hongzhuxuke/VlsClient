#include <QLabel>
#include <QCheckBox>
#include <QLayout>
#include <QPainter>
#include <QColor>
#include <QDebug>
#include "MediaItem.h"
#include "push_button.h"
#include "pub.Const.h"
#if _MSC_VER >= 1600  
#pragma execution_character_set("utf-8")  
#endif  
MediaItem::MediaItem(MediaInfo *mediaInfo, QWidget *parent)
: QWidget(parent)
{
   setFixedHeight(36);
   setFixedWidth(360);
   mMediaCbx = new QCheckBox(this);
   qDebug()<<"mMediaCbx size:"<<mMediaCbx->size();
   mMediaCbx->setChecked(mediaInfo->isSelected);

   mMediaCbx->setFixedHeight(16);
   mMediaCbx->setFixedWidth(16);
   
   mCheckedState = mediaInfo->isSelected;
   mMediaName = new QLabel(this);
   mMediaName->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);

   mMediaDeleteBtn = new PushButton(this);
   mMediaDeleteBtn->loadPixmap(":/sysButton/deleteItem");
   mMediaDeleteBtn->setToolTip(tr("ÒÆ³ý"));
   mMediaDeleteBtn->setStyleSheet(TOOLTIPQSS);
   mMediaFileName=mediaInfo->strFileName;

   QHBoxLayout *Layout = new QHBoxLayout(this);
   Layout->setSpacing(3);
   Layout->addWidget(mMediaCbx, Qt::AlignLeft);
   Layout->addWidget(mMediaName, Qt::AlignLeft);
   Layout->addWidget(mMediaDeleteBtn, Qt::AlignRight);
   Layout->setContentsMargins(0, 0, 0, 0);
   setLayout(Layout);
   mMediaName->setToolTip(mediaInfo->strFileName);
   mMediaName->setStyleSheet(TOOLTIPQSS);
   QString showText=mMediaName->fontMetrics().elidedText
   	(mediaInfo->strFileName,Qt::ElideMiddle,320);
   mMediaName->setText(showText);
   mMediaName->setAlignment(Qt::AlignVCenter|Qt::AlignLeft);
   //debug
   //mMediaName->setStyleSheet("background-color:red;color:white;");
   
   setToolTip(mediaInfo->strFileName);
   setStyleSheet(TOOLTIPQSS);
   connect(mMediaCbx, SIGNAL(stateChanged(int)), this, SLOT(emitCheckBoxState(int)));
   connect(mMediaCbx, SIGNAL(clicked()), this, SIGNAL(CheckBoxClicked()));
   connect(mMediaDeleteBtn, SIGNAL(clicked()), this, SLOT(emitDeleteItems()));
   //setStyleSheet("border-color: rgb(255, 25, 113);");
}

MediaItem::~MediaItem()
{

}
bool MediaItem::isChecked()
{
	return mMediaCbx->isChecked();
}
QString MediaItem::getFilename()
{
	return mMediaFileName;
}
void MediaItem::emitDeleteItems() {
   emit deleteItems(this);
}

void MediaItem::emitCheckBoxState(int state){
   mCheckedState = state;
   emit CheckBoxState(state);
}
