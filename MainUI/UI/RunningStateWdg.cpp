#include "RunningStateWdg.h"
#include "ConfigSetting.h"
#include <QDebug>
#include "pathManage.h"
#include "DebugTrace.h"
#include "pathmanager.h"
#include "webinarIdWdg.h"

#define CPUSTATE		QStringLiteral("CPU使用率：")
#define NETUPLOADSTATE		QStringLiteral("上行网速：")
#define NETRECVSTATEWDG		QStringLiteral("下行网速：")
#define BITRATESTATEWDG		QStringLiteral("码率：")
#define FPSSTATEWDG		QStringLiteral("帧率：")
#define LOSTPACKETSTATEWDG	QStringLiteral("当前丢包率：")
bool RunningStateWdg::m_bIsPressed = false;

#define RedArrow  "border-image: url(:/button/images/red_arrow_up.png);"
#define GrayArrow  "border-image: url(:/button/images/gray_arrow_down.png);"


#define VOL_LEVEL_1  QString("border-image: url(:/check/images/device_check/live_mic_1.png);")
#define VOL_LEVEL_2  QString("border-image: url(:/check/images/device_check/live_mic_2.png);")
#define VOL_LEVEL_3  QString("border-image: url(:/check/images/device_check/live_mic_3.png);")
#define VOL_LEVEL_4  QString("border-image: url(:/check/images/device_check/live_mic_4.png);")
#define VOL_LEVEL_5  QString("border-image: url(:/check/images/device_check/live_mic_5.png);")
#define VOL_LEVEL_6  QString("border-image: url(:/check/images/device_check/live_mic_6.png);")

RunningStateWdg::RunningStateWdg(QWidget *parent)
    : CWidget(parent)
    , m_bIsMoveEnabled(true)
{
    ui.setupUi(this);

	SetCPU(0);
	SetNetUpload(0);
	SetNetRecv(0);
	SetPushStreamFps(0);
	SetPushStreamBitRate(0);
	SetPushStreamLostPacket(0, 1);

	ui.labIcoCpu->setObjectName(QStringLiteral("mpLabIcon"));
	ui.labIcoLostPacket->setObjectName(QStringLiteral("mpLabIcon"));

   QSize fixSize(5,12);
   ui.labIcoCpu->setFixedSize(fixSize);
   ui.labIcoLostPacket->setFixedSize(fixSize);
   

	ui.labIcoCpu->setStyleSheet(RedArrow);
	ui.labIcoLostPacket->setStyleSheet(RedArrow);
}

RunningStateWdg::~RunningStateWdg()
{
}

void RunningStateWdg::SetCPU(int value) {
    QString tips = QStringLiteral("%1%").arg(value);
	value > 80 ? ui.labIcoCpu->show()/*ui.cpuStateWdg->setExceedHigh(true)*/ : ui.labIcoCpu->hide() /*ui.cpuStateWdg->setExceedHigh()*/;
	ui.labCpu->setText(tips);
}

void RunningStateWdg::SetNetUpload(int bytesSpeed) {

    QString tips;
    if (bytesSpeed < 1024 * 1024) {
        double doubleSpeed = (double)(bytesSpeed) / (double)(1024);
        QString speedStr = QString::number(doubleSpeed, 10, 2);
        tips = QStringLiteral("%1K/s").arg(speedStr);
    }
    else if (bytesSpeed >= 1024 * 1024) {
        double doubleSpeed = (double)(bytesSpeed) / (double)(1024*1024);
        QString speedStr = QString::number(doubleSpeed, 10, 2);
        tips = QStringLiteral("%1M/s").arg(speedStr);
    }

}

void RunningStateWdg::SetNetRecv(int bytesSpeed) {
    QString tips;
    if ( bytesSpeed < 1024 * 1024) {
        double doubleSpeed = (double)(bytesSpeed) / (1024);
        QString speedStr = QString::number(doubleSpeed, 10, 2);
        tips = QStringLiteral("%1K/s").arg(speedStr);
    }
    else if (bytesSpeed >= 1024 * 1024) {
        double doubleSpeed = (double)(bytesSpeed) / (double)(1024 * 1024);
        QString speedStr = QString::number(doubleSpeed, 10, 2);
        tips = QStringLiteral("%1M/s").arg(speedStr);
    }
}

void RunningStateWdg::SetPushStreamFps(int fps) {
    if (fps > 0) {
        std::wstring confPath = GetAppDataPath() + CONFIGPATH;
        QString qsConfPath = qsConfPath.fromStdWString(confPath);
        int vcodec_fps = DEFAULT_VCODEC_FPS;
        vcodec_fps = ConfigSetting::ReadInt(qsConfPath, GROUP_DEFAULT, KEY_VCODEC_FPS, DEFAULT_VCODEC_FPS);
        fps = vcodec_fps <= 0 ? DEFAULT_VCODEC_FPS : vcodec_fps;
    }
    QString tips = QStringLiteral("%1fps").arg(fps);
    ui.labFpsState->setText(tips);
}

void RunningStateWdg::SetPushStreamBitRate(int bytesSpeed) {
    QString tips;
    if (bytesSpeed < 1024 ) {
        tips = QStringLiteral("%1kbps").arg(bytesSpeed);
    }
    else if (bytesSpeed >= 1024  && bytesSpeed < 1024 * 1024) {
        double doubleSpeed = (double)(bytesSpeed) / (double)(1024);
        QString speedStr = QString::number(doubleSpeed, 10, 2);
        tips = QStringLiteral("%1mbps").arg(speedStr);
    }
    ui.labbitRate->setText(tips);
}

void RunningStateWdg::SetPushStreamLostPacket(int lost,int count) {
    double lostRate = (double)(lost - mLastLost) / (double)(count - mLastTotal);
    if ((lost - mLastLost >= count - mLastTotal) && lost > 0 && count > 0) {
        lostRate = 100.00;
    }
    QString lostStr = (count - mLastTotal) <= 0 ? "0.00" : QString::number(lostRate, 10, 2);
    if (lostStr == "100.00") {
#ifdef DEBUG
       TRACE6("%s count %d mLastTotal %d mLastLost %d",__FUNCTION__, count, mLastTotal, mLastLost);
#endif
    }
    QString tips = QStringLiteral("%1%").arg(lostStr);
	 ui.labLostPacket->setText(tips);
	 lostRate > 0.05 ? ui.labIcoLostPacket->show() : ui.labIcoLostPacket->hide();
    mLostRate = lostRate;
    mLastTotal = count;
}

float RunningStateWdg::GetPushStreamLostRate() {
   return mLostRate;
}

void RunningStateWdg::SetVolume(int value) {
   if (value <= 0) {
      ui.label_mic_vol->setStyleSheet(VOL_LEVEL_1);
   }
   else if (value > 0 && value <= 16) {
      ui.label_mic_vol->setStyleSheet(VOL_LEVEL_2);
   }
   else if (value > 16 && value <= 36) {
      ui.label_mic_vol->setStyleSheet(VOL_LEVEL_3);
   }
   else if (value > 36 && value <= 64) {
      ui.label_mic_vol->setStyleSheet(VOL_LEVEL_4);
   }
   else if (value > 64 && value <= 75) {
      ui.label_mic_vol->setStyleSheet(VOL_LEVEL_5);
   }
   else {
      ui.label_mic_vol->setStyleSheet(VOL_LEVEL_6);
   }
}


void RunningStateWdg::ShowPushStreamFps(const bool& bShow /*= true*/) {
	if (bShow)
	{
		ui.labFpsState->show();
		ui.labFpsStateTitle->show();
	}
	else
	{
		ui.labFpsState->hide();
		ui.labFpsStateTitle->hide();
	}

}

void RunningStateWdg::ShowPushStreamBitRate(const bool& bShow /*= true*/) {
	if (bShow)
	{
		ui.labbitRate->show();
		ui.labBitRateTitle->show();
      ui.label_mic_name->show();
      ui.widget->show();
	}
	else {
		ui.labbitRate->hide();
		ui.labBitRateTitle->hide();
      ui.label_mic_name->hide();
      ui.widget->hide();
	}

}

void RunningStateWdg::HideNetState(bool hide) {
    ui.horizontalSpacer_4->changeSize(0, 0);
}

void RunningStateWdg::ShowPushStreamLostPacket(const bool& bShow /*= true*/)
{
	if (bShow)
	{
		ui.labLostPacket->show();
		ui.labLostPacketTitle->show();
	}
	else
	{
		ui.labLostPacket->hide();
		ui.labLostPacketTitle->hide();
	}

}

void RunningStateWdg::mousePressEvent(QMouseEvent *e) {
    m_qPressPoint = e->pos();
    m_bIsPressed = true;
    TRACE6("%s\n", __FUNCTION__);
    emit sigMousePressed();
}

void RunningStateWdg::mouseMoveEvent(QMouseEvent *e) {
    if (!m_bIsMoveEnabled) {
        TRACE6("%s m_bIsMoveEnabled is false\n", __FUNCTION__);
        return;
    }
    if ((e->buttons() == Qt::LeftButton) && m_bIsPressed) {
        QWidget* parent_widget = this->parentWidget();
        QPoint parent_point = parent_widget->pos();
        parent_point.setX(parent_point.x() + e->x() - m_qPressPoint.x());
        parent_point.setY(parent_point.y() + e->y() - m_qPressPoint.y());
        parent_widget->move(parent_point);
        TRACE6("%s\n", __FUNCTION__);
    }
    TRACE6("%s end\n", __FUNCTION__);
}

void RunningStateWdg::mouseReleaseEvent(QMouseEvent *) {
    if (m_bIsPressed) {
        m_bIsPressed = false;
        TRACE6("%s\n", __FUNCTION__);
    }
}
