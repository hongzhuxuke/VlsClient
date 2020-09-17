#include "stdafx.h"
#include "AudioSliderUI.h"
#include "ToolButton.h"

#include "Msg_MainUI.h"

#include <QPainter>
#include <QPushButton>
#include <QSlider>
#include <QLabel>
#include <QToolTip>
#include <QFrame>
#include <QApplication>
#include <QDebug>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMouseEvent>
AudioSliderUI::AudioSliderUI(QDialog *parent)
: VHDialog(parent) {
   this->setFocusPolicy(Qt::StrongFocus);
   this->setWindowFlags(Qt::FramelessWindowHint | Qt::SplashScreen | Qt::WindowStaysOnTopHint);
}

AudioSliderUI::~AudioSliderUI() {

}
void AudioSliderUI::LoadPixmap(QString pixmapPath) {
   this->mBackgroundPixmap.load(pixmapPath);
   if (!this->mBackgroundPixmap.isNull()) {
      setFixedSize(this->mBackgroundPixmap.size());
   }
}
bool AudioSliderUI::Create() {
   this->setStyleSheet("background-color:transparent;");
   this->LoadPixmap(":/sysButton/volumnBackground");
   QVBoxLayout *mainLayout=new QVBoxLayout();
   mainLayout->setContentsMargins(0,2,0,2);
   mainLayout->setSpacing(0);
   //音量调节   
   m_pVolumeSlider = new QSlider(Qt::Vertical, this);
   m_pVolumeSlider->setMaximum(100);
   m_pVolumeSlider->setValue(100);
   m_pVolumeSlider->setStyleSheet(
      "QSlider{"
      "margin:0px 0px;"
      "background: transparent;"
      "}"
      "QSlider::groove:vertical {"
      "border: 1px solid gray;"
      "width: 2px;"
      "margin: 0px 0px;"
      "top: 5px;"
      "bottom: 5px;"
      "}"
      
      "QSlider::handle:vertical {"
      "background: rgb(209,209,209);"
      "border-radius: 5px;"
      "border: 1px solid rgba(150,150,150,0);"
      "width: 10px;"
      "height: 10px;"
      "margin: -5px -5px;"
      "}"
      "QSlider::sub-page:vertical{"
      "background: rgb(94,94,94);"
      "}"
      "QSlider::add-page:vertical{"
      "background: rgb(164,164,164);"
      "}"
      );
   connect(m_pVolumeSlider, SIGNAL(valueChanged(int)), this, SLOT(OnVolumnChangedSlot(int)));

   //静音按钮
   m_Mute = new ToolButton(":/toolWidget/mute", this);
   connect(m_Mute, SIGNAL(sigClicked()), this, SLOT(OnMuteClick()));
   //设置按钮
   m_VoSettingBtn = new ToolButton(":/toolWidget/audioSet", this);
   m_VoSettingBtn->setCursor(QCursor(Qt::PointingHandCursor));
   connect(m_VoSettingBtn, SIGNAL(sigClicked()), this, SLOT(OnSettingClick()));

   QWidget *topWidget=new QWidget();
   QHBoxLayout *topLayout=new QHBoxLayout();
   topLayout->setContentsMargins(0,0,0,0);
   topLayout->setSpacing(0);
   topLayout->addWidget(new QWidget());
   topLayout->addWidget(m_pVolumeSlider);
   topLayout->addWidget(new QWidget());
   topWidget->setLayout(topLayout);

   QWidget *bottomWidget = new QWidget();
   QHBoxLayout *bottomLayout = new QHBoxLayout();
   bottomWidget->setMinimumHeight(16);
   bottomLayout->setContentsMargins(0,0,0,0);
   bottomLayout->setSpacing(0);
   bottomLayout->addWidget(new QWidget());
   bottomLayout->addWidget(m_Mute);
   bottomLayout->addWidget(new QWidget());
   bottomWidget->setLayout(bottomLayout);
   m_bottomWidget = bottomWidget;

   QHBoxLayout *setLayout = new QHBoxLayout();
   setLayout->setContentsMargins(0, 0, 0, 5);
   setLayout->addStretch();
   setLayout->addWidget(m_VoSettingBtn);
   setLayout->addStretch();

   //主布局
   mainLayout->addWidget(topWidget);
   mainLayout->addWidget(bottomWidget);
   mainLayout->addSpacing(6);
   mainLayout->addLayout(setLayout);

   this->setLayout(mainLayout);
   m_pVolumeSlider->installEventFilter(this);
   m_pVolumeSlider->setPageStep(1);

   connect(&mCloseTimer,SIGNAL(timeout()),this,SLOT(Timeout()));

   return true;
}

void AudioSliderUI::Show(int volumn ,QPoint btnGlobalPos) {      
   disconnect(m_pVolumeSlider, SIGNAL(valueChanged(int)), this, SLOT(OnVolumnChangedSlot(int)));
   m_pVolumeSlider->setValue(volumn);
   connect(m_pVolumeSlider, SIGNAL(valueChanged(int)), this, SLOT(OnVolumnChangedSlot(int)));
   show();
   move(btnGlobalPos.x()-this->width()/2,btnGlobalPos.y()-this->height() - 7);
   this->setFocus();
}

void AudioSliderUI::Destroy() {

}

void AudioSliderUI::paintEvent(QPaintEvent *e) {
   if(!this->mBackgroundPixmap.isNull()) {
      QPainter painter(this);
      painter.drawPixmap(rect(),this->mBackgroundPixmap);
   }
   else {
      QWidget::paintEvent(e);
   }

}
void AudioSliderUI::enterEvent(QEvent *e) {
   QDialog::enterEvent(e);
   m_bEnter=true;
}
void AudioSliderUI::leaveEvent(QEvent *e) {
   QDialog::leaveEvent(e);
   m_bEnter=false;
}

void AudioSliderUI::focusOutEvent(QFocusEvent *e) {
   QDialog::focusOutEvent(e);
   if(!m_bEnter){
      this->close();
      return ;
   }
   QWidget *focusWidget = this->focusWidget();
   bool focusThis = false;

   while (focusWidget) {
      if (focusWidget == this) {
         focusThis = true;
         break;
      }

      focusWidget = focusWidget->parentWidget();
   }

   if (!focusThis) {
      this->close();
      m_bEnter=false;
   } else {
      this->setFocus();
   }
} 

void AudioSliderUI::OnMuteClick() {
   
   STRU_MAINUI_MUTE loMute;
   loMute.m_eType = mute_Mic;
   loMute.m_bMute = m_pVolumeSlider->value() != 0;
   SingletonMainUIIns::Instance().PostCRMessage(MSG_MAINUI_MUTE, &loMute, sizeof(loMute));

	QJsonObject body;
	SingletonMainUIIns::Instance().reportLog(STR_VH_LIVE_OPERATE,eLogRePortK_MicMute, L"MicMute", body);
   this->close();
}

void AudioSliderUI::OnSettingClick(){
   SingletonMainUIIns::Instance().PostCRMessage(MSG_MAINUI_SHOW_AUDIO_SETTING_CARD, NULL, NULL);
   this->hide();
}

//点击slider即达
bool AudioSliderUI::eventFilter(QObject *obj, QEvent *ev) {
   if (ev) {
      if (ev->type() == QEvent::MouseButtonPress) {
         //麦克风音量slider, 点击即到
         if (obj == m_pVolumeSlider){
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(ev);
            if (mouseEvent->button() == Qt::LeftButton)
            {
               int dur = m_pVolumeSlider->maximum() - m_pVolumeSlider->minimum();
               int pos = m_pVolumeSlider->minimum() + dur * (1.0 - ((double)(mouseEvent->y() - 5) / (m_pVolumeSlider->height() - 10)));
               if (pos != m_pVolumeSlider->sliderPosition())
               {
                  m_pVolumeSlider->setValue(pos);
               }
            }
         }
      }
   }
   return QObject::eventFilter(obj, ev);
}

void AudioSliderUI::OnVolumnChangedSlot(int nValue){
   QToolTip::showText(QCursor::pos(), QString("%1").arg(nValue));
   TRACE6("%s nValue:%d\n",__FUNCTION__,nValue);
   emit sigVolumnChanged(nValue);
}

void AudioSliderUI::HideMuteBtn() {
   m_bottomWidget->setVisible(false);
}

void AudioSliderUI::HideSettingBtn() {
   m_VoSettingBtn->setVisible(false);
}

void AudioSliderUI::WillClose(int t) {
   mCloseTimer.start(t);
}

void AudioSliderUI::StopClose() {
   mCloseTimer.stop();
}

void AudioSliderUI::SetVolumn(int v) {
   disconnect(m_pVolumeSlider, SIGNAL(valueChanged(int)), this, SLOT(OnVolumnChangedSlot(int)));
   m_pVolumeSlider->setValue(v);
   connect(m_pVolumeSlider, SIGNAL(valueChanged(int)), this, SLOT(OnVolumnChangedSlot(int)));
}

void AudioSliderUI::Timeout() {
   if(!m_bEnter) {
      this->close();
   }
   mCloseTimer.stop();
}
