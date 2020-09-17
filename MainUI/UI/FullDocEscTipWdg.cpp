#include "FullDocEscTipWdg.h"

FullDocEscTipWdg::FullDocEscTipWdg(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground);
    setAutoFillBackground(false);
}

FullDocEscTipWdg::~FullDocEscTipWdg()
{
}
