#pragma once

#include <QWidget>
#include "ui_VolumePaintWdg.h"
#include <QTimer>
enum VolumeType {
    VolumeType_PreviewMic = 0,
    VolumeType_Play_File = 1,
};

class VolumePaintWdg : public QWidget
{
    Q_OBJECT

public:
    VolumePaintWdg(QWidget *parent = Q_NULLPTR);
    ~VolumePaintWdg();
    
    void SetLiveType(int type);
    void SetVolumeType(VolumeType type);
    void SetMicVolume(int value);
signals:
   void sig_havVolume();
protected:
    virtual void paintEvent(QPaintEvent*);

private slots:
    void slot_PaintVolume();

private:
    Ui::VolumePaintWdg ui;
    int mVolumeType = 0;
    int micVolume = 0;
    QTimer mPaintTimer;
   int mLiveType = 0;
};
