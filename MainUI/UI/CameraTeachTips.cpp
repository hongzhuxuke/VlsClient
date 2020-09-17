#include "CameraTeachTips.h"

CameraTeachTips::CameraTeachTips(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
    connect(ui.pushButton, SIGNAL(clicked()), this, SLOT(slot_OnClose()));
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground, true);
}

CameraTeachTips::~CameraTeachTips()
{
}

void CameraTeachTips::slot_OnClose() {
    close();
}
