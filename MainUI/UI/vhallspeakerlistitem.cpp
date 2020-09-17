#include "vhallspeakerlistitem.h"
#include "ui_vhallspeakerlistitem.h"
#include "MainUIIns.h"

VhallSpeakerListItem::VhallSpeakerListItem(QWidget *parent) :
QWidget(parent),
mCamera(false),
mMic(false),
mbVisable(false),
ui(new Ui::VhallSpeakerListItem) {
   ui->setupUi(this);
   RepaintIcon();
   ui->label_speaker->installEventFilter(this);
   ui->label_speaker->hide();
}

VhallSpeakerListItem::~VhallSpeakerListItem() {
   delete ui;
}

void VhallSpeakerListItem::setId(QString id) {
   mId = id;
}

void VhallSpeakerListItem::setName(QString name) {
   QString name_elide = geteElidedText(ui->label_name->font(), name, 134);
   ui->label_name->setText(name_elide);
   ui->label_name->setToolTip(name);
   ui->label_name->setStyleSheet("QToolTip {border: 0px solid;background: rgb(0,0,0);}");
}

void VhallSpeakerListItem::setMaster(bool master) {
   mMaster = master;
}

void VhallSpeakerListItem::setCameraIcon(bool ok) {
   mCamera = ok;
   RepaintIcon();
}

bool VhallSpeakerListItem::GetMicState() {
   return mMic;
}

void VhallSpeakerListItem::setMicIcon(bool ok) {
   mMic = ok;
   RepaintIcon();
}

void VhallSpeakerListItem::RepaintIcon() {
   if (mCamera) {
      ui->label_icon->setStyleSheet("QLabel{image: url(\":/interactivity/iconCamera\");}");
   } else if (mMic) {
      ui->label_icon->setStyleSheet("QLabel{image: url(\":/interactivity/iconSpeaker\");}");
   } else {
      ui->label_icon->setStyleSheet("border-image:none;)");
   }
   ui->label_icon->repaint();
}

bool VhallSpeakerListItem::eventFilter(QObject *o, QEvent *e) {
   if (ui->label_speaker == o) {
      if (e->type() == QEvent::Enter) {
         ui->label_speaker->setStyleSheet("color:#ff3530;");
      } else if (e->type() == QEvent::Leave) {
         ui->label_speaker->setStyleSheet("color:#c7c7c8;");
      } else if (e->type() == QEvent::MouseButtonPress) {
         ui->label_speaker->setStyleSheet("color:#c7c7c8;");
      } else if (e->type() == QEvent::MouseButtonRelease) {
         QJsonObject body;
         body["id"] = mId;
         SingletonMainUIIns::Instance().reportLog(L"interaction_underwheat", eLogRePortK_Interaction_UnderWheat, body);
         ui->label_speaker->setStyleSheet("color:#ff3530;");
         emit this->SigItemClicked(this->mId);
      }
   }
   return QWidget::eventFilter(o, e);
}

void VhallSpeakerListItem::SetOffVisiable(bool visible) {
   mbVisable = visible;
}

void VhallSpeakerListItem::enterEvent(QEvent *) {
   if (mbVisable) {
      ui->label_speaker->show();
   }
}

void VhallSpeakerListItem::leaveEvent(QEvent *) {
   if (mbVisable) {
      ui->label_speaker->hide();
   }
}

QString VhallSpeakerListItem::geteElidedText(QFont font, QString str, int MaxWidth) {
   QFontMetrics fontWidth(font);
   int width = fontWidth.width(str);
   if (width >= MaxWidth) {
      str = fontWidth.elidedText(str, Qt::ElideRight, MaxWidth);
   }
   return str;
}

