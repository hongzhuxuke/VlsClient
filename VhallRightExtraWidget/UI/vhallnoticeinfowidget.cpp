#include "vhallnoticeinfowidget.h"
#include "ui_vhallnoticeinfowidget.h"
#include "VhallRightExtraWidgetIns.h"
#include "Msg_VhallRightExtraWidget.h"   
#include "IMainUILogic.h"
#include <QDesktopServices>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

VhallNoticeInfoWidget::VhallNoticeInfoWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::VhallNoticeInfoWidget)
{
    ui->setupUi(this);
    connect(ui->textEdit, SIGNAL(textChanged()), this, SLOT(slotsTextChanged()));
    ui->textEdit->installEventFilter(this);
    ui->textEdit->setContextMenuPolicy(Qt::NoContextMenu);
}

VhallNoticeInfoWidget::~VhallNoticeInfoWidget()
{
   if (m_pWebChannel && m_pWebEngineView) {
      m_pWebEngineView->stop();
      ui->verticalLayout_web->removeWidget(m_pWebEngineView);

      delete m_pWebChannel;
      m_pWebChannel = NULL;

      delete m_pWebEngineView;
      m_pWebEngineView = NULL;

   }
   delete ui;
}

bool VhallNoticeInfoWidget::eventFilter(QObject *obj, QEvent *e)
{
   if (obj == ui->textEdit){
      if (e->type() == QEvent::KeyPress){
         QKeyEvent *event = static_cast<QKeyEvent*>(e);
         if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return){
            QString pText = ui->textEdit->toPlainText().trimmed();
            if (pText.isEmpty()){
               ui->textEdit->clear();
               ui->textEdit->setFocus();
               return true;
            }
            pText.replace("\"", "\\\"");
            doSendNotice(pText);
            ui->textEdit->clear();
            return true;
         }
      }
      else if (e->type() == QEvent::FocusIn) {
         if (ui->textEdit->toPlainText().length() == 0) {
            ui->textEdit->setPlaceholderText("");
         }
      } else if (e->type() == QEvent::FocusOut) {
         ui->textEdit->setPlaceholderText(QStringLiteral("向观众发布系统公告吧，按“回车键”发送"));
      }
   }
   return false;
}

void VhallNoticeInfoWidget::doSendNotice(QString msg){
   VH::CComPtr<IMainUILogic> pMainUILogic = NULL;
   DEF_GET_INTERFACE_PTR(SingletonVhallRightExtraWidgetIns, PID_IMainUI, IID_IMainUILogic, pMainUILogic, return);
   if(pMainUILogic) {
      if(pMainUILogic->IsShowMainWidget()) {
         QString method = QString("preSendNoticeMsg(\"%1\")").arg(msg);
         if (m_pWebEngineView) {
            m_pWebEngineView->page()->runJavaScript(method);
         }
      }
      else {
         QJsonObject obj;
         obj ["type"] = "*announcement";
         obj ["content"] = msg;
         QJsonDocument doc ;
         doc.setObject(obj);
         sendNotice(doc.toJson(QJsonDocument::Compact));
      }
   }
}
void VhallNoticeInfoWidget::loadFinished(bool ok) {
   if(ok) {
      m_pWebEngineView->show();
   }
   mIsLoadUrlFinished = true;
}

void VhallNoticeInfoWidget::appendNoticeMsg(QString msg){
   VH::CComPtr<IMainUILogic> pMainUILogic = NULL;
   DEF_GET_INTERFACE_PTR(SingletonVhallRightExtraWidgetIns, PID_IMainUI, IID_IMainUILogic, pMainUILogic, return);
   if (pMainUILogic) {
      if (pMainUILogic->IsShowMainWidget()) {
         return;
      }
   }

   QJsonObject obj;
   obj ["type"] = "announcement";
   obj ["content"] = msg;

   QJsonDocument doc ;
   doc.setObject(obj);
   QString str = doc.toJson(QJsonDocument::Compact);

   msg = str;
   msg.replace("\"", "\\\"");
   QString method = QString("appendChatMsg(\"%1\")").arg(msg);
   qDebug()<<"VhallNoticeInfoWidget::appendNoticeMsg "<<method;
   if (m_pWebEngineView) {
      m_pWebEngineView->page()->runJavaScript(method);
   }
}

void VhallNoticeInfoWidget::sendNotice(QString msg)
{
   qDebug()<<"VhallNoticeInfoWidget::sendNotice"<<msg;
   RQData oData;
   oData.m_eMsgType = e_RQ_ReleaseAnnouncement;
   strcpy(oData.m_wzText, msg.toUtf8().data());
   SingletonVhallRightExtraWidgetIns::Instance().PostCRMessage(
      MSG_VHALLRIGHTEXTRAWIDGET_SENDMSG, &oData, sizeof(RQData));
}

void VhallNoticeInfoWidget::slotsTextChanged(){
   QString textContent = ui->textEdit->toPlainText();
   int length = textContent.count();
   int maxLength = 140; // 最大字符数
   if (length > maxLength)
   {
      int position = ui->textEdit->textCursor().position();
      QTextCursor textCursor = ui->textEdit->textCursor();
      textContent.remove(position - (length - maxLength), length - maxLength);
      ui->textEdit->setText(textContent);
      textCursor.setPosition(position - (length - maxLength));
      ui->textEdit->setTextCursor(textCursor);
   }
}
void VhallNoticeInfoWidget::Refresh(bool bInit) {
   m_bIsFirstShow = true;
   if (m_pWebChannel != NULL && m_pWebEngineView != NULL) {
      m_pWebEngineView->load(QUrl(tr("qrc:/chat/chat_html/announcement.html")));
      mIsLoadUrlFinished = false;
   }
}


void VhallNoticeInfoWidget::AddWebEngineView() {
   if (m_pWebChannel == NULL && m_pWebEngineView == NULL) {
      m_pWebEngineView = new QWebEngineView(this);
      m_pWebChannel = new QWebChannel(this);
      if (m_pWebEngineView) {
         connect(m_pWebEngineView, SIGNAL(loadFinished(bool)), this, SLOT(loadFinished(bool)));
         connect(m_pWebEngineView->page(), SIGNAL(renderProcessTerminated(QWebEnginePage::RenderProcessTerminationStatus, int)), this, SLOT(slot_Render(QWebEnginePage::RenderProcessTerminationStatus, int)));
         QWebEngineSettings* pWebSettings = m_pWebEngineView->settings();
         if (pWebSettings) {
            pWebSettings->setAttribute(QWebEngineSettings::AutoLoadImages, true);
            pWebSettings->setAttribute(QWebEngineSettings::JavascriptEnabled, true);
            pWebSettings->setAttribute(QWebEngineSettings::PluginsEnabled, true);
            pWebSettings->setAttribute(QWebEngineSettings::ScreenCaptureEnabled, true);
         }
         //如果不设置这就话，加载URL时会先显示以下白色背景。
         m_pWebEngineView->page()->setBackgroundColor(Qt::transparent);
         ui->verticalLayout_web->addWidget(m_pWebEngineView);
         m_pWebEngineView->setContextMenuPolicy(Qt::NoContextMenu);
      }
      if (m_pWebChannel && m_pWebEngineView) {
         m_pWebChannel->registerObject("MainWindow", this);
         m_pWebEngineView->page()->setWebChannel(m_pWebChannel);
         m_pWebEngineView->load(QUrl(tr("qrc:/chat/chat_html/announcement.html")));
         mIsLoadUrlFinished = false;
      }
   }
}

void VhallNoticeInfoWidget::ShowNoticeWnd() {
   AddWebEngineView();
   if (m_pWebChannel != NULL && m_pWebEngineView != NULL && m_bIsFirstShow) {
      m_pWebEngineView->load(QUrl(tr("qrc:/chat/chat_html/announcement.html")));
      mIsLoadUrlFinished = false;
      m_bIsFirstShow = false;
   }
}

bool VhallNoticeInfoWidget::IsLoadUrlFinished() {
   return mIsLoadUrlFinished;
}

