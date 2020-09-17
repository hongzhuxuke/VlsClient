#ifndef NOISETOOLTIP_H
#define NOISETOOLTIP_H

#include <QWidget>
#include "ui_NoiseTooltip.h"

class NoiseTooltip : public QWidget
{
    Q_OBJECT

public:
    NoiseTooltip(QWidget *parent = 0);
    ~NoiseTooltip();
	 void setText(const QString& str);
private:
    Ui::NoiseTooltip ui;
};

#endif // NOISETOOLTIP_H
