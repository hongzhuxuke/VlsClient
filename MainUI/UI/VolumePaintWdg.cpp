#include "VolumePaintWdg.h"
#include <QPainter>
#include "WebRtcSDKInterface.h"
#include "VHPaasSDKCallback.h"
#include "pub.Const.h"
using namespace vlive;

VolumePaintWdg::VolumePaintWdg(QWidget *parent)
    : QWidget(parent){
    ui.setupUi(this);
    connect(&mPaintTimer, SIGNAL(timeout()), this, SLOT(slot_PaintVolume()));
}

VolumePaintWdg::~VolumePaintWdg()
{
    mPaintTimer.stop();
}

void VolumePaintWdg::SetLiveType(int type) {
   mLiveType = type;
}

void VolumePaintWdg::SetVolumeType(VolumeType type) {
    mVolumeType = type;
    mPaintTimer.start(100);
}

void VolumePaintWdg::SetMicVolume(int value) {
   micVolume = value;
}

void VolumePaintWdg::paintEvent(QPaintEvent*) {
    QPainter paint;
    paint.begin(this);
    QColor color(255, 192, 89);
    paint.setPen(QPen(color, 1, Qt::SolidLine));//设置画笔形式 
    paint.setBrush(QBrush(color, Qt::SolidPattern));//设置画刷形式 
    int value = 0;
    if (mVolumeType == VolumeType_PreviewMic) {
       if (mLiveType == eLiveType_VhallActive) {
          value = GetWebRtcSDKInstance()->GetPrevieMicVolumValue();
       }
       else {
          value = micVolume;
       }
    }
    else if (mVolumeType == VolumeType_Play_File) {
        value = GetWebRtcSDKInstance()->GetPlayAudioFileVolum();
    }
    if (value > 0) {
       emit sig_havVolume();
    }
    int paintWidth = (float)value / 100.0 * this->width();
    int stap = this->width() / 40;
    int i = 0;
    for (; i < paintWidth / stap; i++) {
        if (i % 2 == 0) {
            paint.drawRect(QRect(i * stap, 0, stap, 20));
        }
    }

    QColor rightColor(237, 237, 237);
    paint.setPen(QPen(rightColor, 1, Qt::SolidLine));//设置画笔形式 
    paint.setBrush(QBrush(rightColor, Qt::SolidPattern));//设置画刷形式 
    for (; i <= this->width() - paintWidth / stap; i++) {
        if (i % 2 == 0) {
            paint.drawRect(QRect(i * stap, 0, stap, 20));
        }
    }
    paint.end();
}

void VolumePaintWdg::slot_PaintVolume() {
    repaint();
}
