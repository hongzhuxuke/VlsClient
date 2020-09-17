#ifndef H_RUNNINGSTATEWDG_H
#define H_RUNNINGSTATEWDG_H

#include "CWidget.h"
#include "ui_RunningStateWdg.h"
#include <QMouseEvent>
#include <QPoint>
#include <QDateTime>

class RunningStateWdg : public CWidget
{
    Q_OBJECT

public:
    RunningStateWdg(QWidget *parent = Q_NULLPTR);
    ~RunningStateWdg();

    void SetCPU(int value);
    void SetNetUpload(int value);
    void SetNetRecv(int value);
    void SetVolume(int value);

    void SetPushStreamFps(int fps);
    void SetPushStreamBitRate(int bitrate);
    void SetPushStreamLostPacket(int lost, int count);

	 void ShowPushStreamFps(const bool& bShow = true);
	 void ShowPushStreamBitRate(const bool& bShow = true);
	 void ShowPushStreamLostPacket(const bool& bShow = true);

    void HideNetState(bool hide);
    float GetPushStreamLostRate();
protected:
    void mousePressEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);

signals:
    void sigMousePressed();
    void sigClicked();

private:
    Ui::RunningStateWdg ui;
    unsigned long mLastLost = 0;
    unsigned long mLastTotal = 0;

    QPoint m_qPressPoint;
    bool m_bIsMoveEnabled;
    static bool m_bIsPressed;
    qint64 mDataTime = 0;
    float mLostRate = 0.0;
};

#endif //H_RUNNINGSTATEWDG_H