#include "vhalldebugform.h"
#include "ui_vhalldebugform.h"
#include <QDebug>
#include "DebugTrace.h"
VhallDebugForm *VhallDebugForm::_this = NULL;
QMutex VhallDebugForm::mutex;

void customMessageHandler(QtMsgType, const QMessageLogContext &, const QString &msg) {
   VhallDebugForm::mutex.lock();
   if(VhallDebugForm::_this) {
      VhallDebugForm::_this->Msg(msg);
   }
   VhallDebugForm::mutex.unlock();
}
void customMessageHandlerStatic(QtMsgType, const QMessageLogContext &, const QString &msg) {
   VhallDebugForm::mutex.lock();
   TRACE6("%s\r\n",msg.toLocal8Bit().data());
   VhallDebugForm::mutex.unlock();
}

VhallDebugForm::VhallDebugForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::VhallDebugForm) {
    ui->setupUi(this);
    connect(this,SIGNAL(SigMsg(QString)),this,SLOT(SlotMsg(QString))); 

    VhallDebugForm::mutex.lock();
    if(VhallDebugForm::_this == NULL) {
       VhallDebugForm::_this=this;
    }
    VhallDebugForm::mutex.unlock();
}

VhallDebugForm::~VhallDebugForm() {
   VhallDebugForm::mutex.lock();
   VhallDebugForm::_this = NULL;
   VhallDebugForm::mutex.unlock();

   delete ui;
}
void VhallDebugForm::HookQDebug() {
   qInstallMessageHandler(customMessageHandler);
   show();
}
void VhallDebugForm::Msg(QString msg) {
   if(!this->isHidden()) {
      emit SigMsg(msg);   
   }
}
void VhallDebugForm::SlotMsg(QString msg) {
   ui->textEdit->append(msg);
}

void VhallDebugForm::HookQDebugToFile(){
   qInstallMessageHandler(customMessageHandlerStatic);   
}

