#ifndef NOISESETTINGSLIDER_H
#define NOISESETTINGSLIDER_H

#include <QWidget>
#include "ui_NoiseSettingSlider.h"
#include <QTimer>
#include <QLabel>

class NoiseSettingSlider : public QWidget
{
    Q_OBJECT

public:
    NoiseSettingSlider(QWidget *parent = 0);
    ~NoiseSettingSlider();

    void InitNoiseSetting(bool enable,int noiseValue);
    void EnableNoise(bool enable);
    int GetNoiseValue();

    virtual bool eventFilter(QObject *obj, QEvent *event);
public slots:
	void OnSetNoiseValue(int value);
protected:
   virtual void paintEvent(QPaintEvent *);
   virtual void showEvent(QShowEvent *event);
private slots:
   void OnGetMicVolumeTimeOut();
   

signals:
   void SignalNoiseValueChange(int);

private:
    Ui::NoiseSettingSlider ui;
    bool m_bEnableNoise;
    int m_NoiseValue;
    QTimer *m_pMicVolumeGetTimer;
};

#endif // NOISESETTINGSLIDER_H
