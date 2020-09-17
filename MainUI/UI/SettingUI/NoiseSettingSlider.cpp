#include "stdafx.h"
#include "NoiseSettingSlider.h"
#include "IOBSControlLogic.h"

#include "Msg_MainUI.h"
#include "Msg_OBSControl.h"
#include "Msg_CommonToolKit.h"
#include "ToolBarWidget.h"  
#include "ConfigSetting.h"

#include <QPainter>
#include <QToolTip>

float NoiseDBtoLog(float db) {
    /* logarithmic scale for audio meter */
    return -log10(0.0f - (db - 6.0f));
}

#define VOL_Noise_MIN -96
#define VOL_Noise_MAX -0
#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif
#ifndef min
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif
float minNoiseLinear = NoiseDBtoLog(VOL_Noise_MIN);
float maxNoiseLinear = NoiseDBtoLog(VOL_Noise_MAX);

NoiseSettingSlider::NoiseSettingSlider(QWidget *parent)
    : QWidget(parent)
    , m_pMicVolumeGetTimer(NULL)
    , m_bEnableNoise(false)
    , m_NoiseValue(0)
{
   ui.setupUi(this);

   //降噪设置
   ui.horizontalSlider_Noise->setRange(0, 96);
   connect(ui.horizontalSlider_Noise, SIGNAL(valueChanged(int)), SLOT(OnSetNoiseValue(int)));
   if (m_pMicVolumeGetTimer == NULL) {
      m_pMicVolumeGetTimer = new QTimer(this);
      if (m_pMicVolumeGetTimer != NULL) {
         connect(m_pMicVolumeGetTimer, SIGNAL(timeout()), this, SLOT(OnGetMicVolumeTimeOut()));
      }
   }
   ui.horizontalSlider_Noise->installEventFilter(this);
}

NoiseSettingSlider::~NoiseSettingSlider()
{

}

void NoiseSettingSlider::InitNoiseSetting(bool enable, int noiseValue) {
   m_bEnableNoise = enable;
   if (enable) {      
      ui.horizontalSlider_Noise->setValue(noiseValue);
   }
   EnableNoise(enable);
}

void NoiseSettingSlider::OnGetMicVolumeTimeOut() {
    update();
}

void NoiseSettingSlider::OnSetNoiseValue(int value) {
   if (m_bEnableNoise){
      m_NoiseValue = value;
		if (ui.horizontalSlider_Noise->hasFocus()){
			QToolTip::showText(QCursor::pos(), QString("%1").arg(value));
		}
      TRACE6("%s set value:%d\n", __FUNCTION__, value);
      SingletonMainUIIns::Instance().PostCRMessage(MSG_MAINUI_NOISE_VALUE_CHANGE, &value, sizeof(value));
   }
}

void NoiseSettingSlider::showEvent(QShowEvent *event) {
    this->setAttribute(Qt::WA_Mapped);
    QWidget::showEvent(event);
}

void NoiseSettingSlider::paintEvent(QPaintEvent *) {
   QPainter painter(this);
   if (m_bEnableNoise) {
        //实时更新音量图标
        float audioMag = 0.0f;
        float audioPeak = 0.0f;
        float audioMax = 0.0f;

        float workVol, workMax, rmsScale, maxScale, peakScale;
        float curVolume;

        VH::CComPtr<IOBSControlLogic> pOBSControlLogic;
        DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IOBSControl, IID_IOBSControlLogic, pOBSControlLogic, return);

        if (pOBSControlLogic) {
           pOBSControlLogic->GetMicAudioMeter(audioMag, audioPeak, audioMax);
           curVolume = audioMag;
           workVol = min(VOL_Noise_MAX, max(VOL_Noise_MIN, curVolume)); // Bound volume levels
           rmsScale = (NoiseDBtoLog(workVol) - minNoiseLinear) / (maxNoiseLinear - minNoiseLinear); // Convert dB to logarithmic then to linear scale [0, 1]
           qDebug() << "workVol" << workVol;
           workVol = 96 + workVol;
        }

        int width = ui.widget->width();
        int h = (ui.widget->height() - 5) / 2 + 2;
        //float paintWidth = (((float)(width) * (float)(96)) / (float)(90)) * (rmsScale);
        float paintWidth = ((float)(width) / (float)(96))*(workVol);
        int noValueWidth = width - paintWidth;
        QPoint pos = ui.widget->pos();
       
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setPen(QPen(QColor(0xFC5659), 1));
        painter.setBrush(QColor(0xFC5659));
        painter.drawRoundedRect(pos.x(), h, paintWidth, 3, 1, 1);

        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setPen(QPen(QColor(0x5A5959), 1));
        painter.setBrush(QColor(0x5A5959));
        painter.drawRoundedRect(pos.x() + paintWidth, h, noValueWidth, 3, 1, 1);
   } else {
       int width = ui.widget->width();
       int h = (ui.widget->height() - 5) / 2;
       float paintWidth = 0;
       int noValueWidth = width - paintWidth;
       QPoint pos = ui.widget->pos();

       painter.setRenderHint(QPainter::Antialiasing, true);
       painter.setPen(QPen(QColor(0x5A5959), 1));
       painter.setBrush(QColor(0x5A5959));
       painter.drawRoundedRect(pos.x() + paintWidth, h, noValueWidth, 3, 1, 1);
   }
}

void NoiseSettingSlider::EnableNoise(bool enable) {
    m_bEnableNoise = enable;
    if (enable){
        if (m_pMicVolumeGetTimer != NULL){
            m_pMicVolumeGetTimer->start(100);
        }
        ui.horizontalSlider_Noise->setDisabled(false);
        ui.horizontalSlider_Noise->setValue(m_NoiseValue);
    } else {
       if (m_pMicVolumeGetTimer != NULL) {
          m_pMicVolumeGetTimer->stop();
       }
       ui.horizontalSlider_Noise->setValue(0);
       ui.horizontalSlider_Noise->setDisabled(true);
       update();
    }
}


int NoiseSettingSlider::GetNoiseValue() {
   return m_NoiseValue;
}

bool NoiseSettingSlider::eventFilter(QObject *obj, QEvent *event) {
   //麦克风音量slider, 点击即到
	if (event->type() == QEvent::MouseButtonPress && obj == ui.horizontalSlider_Noise  && ui.horizontalSlider_Noise->isEnabled()) {
      QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
      if (mouseEvent->button() == Qt::LeftButton) {
         int dur = ui.horizontalSlider_Noise->maximum() - ui.horizontalSlider_Noise->minimum();
         int pos = ui.horizontalSlider_Noise->minimum() + dur * ((double)(mouseEvent->x() - 5) / (ui.horizontalSlider_Noise->width() - 10));
         if (pos != ui.horizontalSlider_Noise->sliderPosition()) {
            ui.horizontalSlider_Noise->setValue(pos);
         }
      }
   }
   return QWidget::eventFilter(obj,event);
}

