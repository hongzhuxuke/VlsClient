#include "NoticeSendWdg.h"
#include "NoticeItem.h"
//#include "NoticeShowWdg.h"
#include "ICommonData.h"
#include "CRPluginDef.h"
#include "MainUIIns.h"
#include "Msg_MainUI.h"
#include "pathManager.h"
#include "pathManage.h"
#include "VhallNetWorkInterface.h"
#include "ConfigSetting.h"
#include "VH_ConstDeff.h"
#include "DebugTrace.h"
#include "VSSAPIManager.h"
#include <QDate>
#include <QScrollBar> 
#include "LiveToolDlg.h"

#define CanNotSend  QStringLiteral("QPushButton{color:rgb(255, 255, 255);\
border:0px;border-top-right-radius:4px;border-bottom-right-radius:4px;background-color:rgb(166, 166, 168);font: 12px \"微软雅黑\";}")

#define CanSend  QStringLiteral("QPushButton{color:rgb(255, 255, 255);\
border:0px;border-top-right-radius:4px;border-bottom-right-radius:4px;background-color:rgb(252, 86, 89);font: 12px \"微软雅黑\";}")


NoticeSendWdg::NoticeSendWdg(QWidget *parent)
    : QDialog(parent)
{
    ui.setupUi(this);
    connect(ui.textEdit, SIGNAL(textChanged()), this, SLOT(slot_textChanged()));
    connect(ui.pushButton_send, SIGNAL(clicked()), this, SLOT(slot_sendMsg()));
    ui.pushButton_send->setEnabled(false);
	ui.textEdit->horizontalScrollBar()->hide();
	ui.textEdit->verticalScrollBar()->hide();
   connect(ui.listWidget->verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(onSliderChanged(int)));
}

NoticeSendWdg::~NoticeSendWdg()
{
}

void NoticeSendWdg::slot_sendMsg() {
    QString sendMsg = ui.textEdit->toPlainText();
    QString text = QStringLiteral("[公告] %1").arg(sendMsg);
    //初始化设置用户信息。
    VH::CComPtr<ICommonData> pCommonData;
    DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
    ClientApiInitResp mRespInitData;
    StartLiveUrl mStartLiveData;
    pCommonData->GetInitApiResp(mRespInitData);
    pCommonData->GetStartLiveUrl(mStartLiveData);
    QObject* recvObj = pCommonData->MainUiLoginObj();
    QString domain = ConfigSetting::ReadString(CPathManager::GetToolConfigPath(), GROUP_DEFAULT, KEY_VSS_URL_DOMAIN, vss_domainUrl);
    if (mRespInitData.player == H5_LIVE_CREATE_TYPE) {
       VSSAPIManager vssApi;
       vssApi.InitAPIRequest(domain.toStdString(), mRespInitData.vss_token.toStdString(), mRespInitData.vss_room_id.toStdString());
       vssApi.VSSSendNotice(sendMsg, [&, sendMsg, recvObj](int libCurlCode, const std::string& msg, const std::string userData) {
          if (recvObj) {
             CustomOnHttpResMsgEvent* event = new CustomOnHttpResMsgEvent(CustomEvent_SendNotice);
             if (event) {
                event->mCode = libCurlCode;
                event->mMsg = QString::fromStdString(msg);
                event->mData = sendMsg.simplified();
                QCoreApplication::postEvent(recvObj, event, Qt::LowEventPriority);
             }
          }
       });
    }
    else {
       QString domain_Url = ConfigSetting::ReadString(CPathManager::GetToolConfigPath(), GROUP_DEFAULT, KEY_URL_DOMAIN, domainUrl);
       QJsonObject obj;
       QByteArray ba = sendMsg.toUtf8();    
       QByteArray byteArrayPercentEncoded = ba.toPercentEncoding();

       sendMsg = sendMsg.simplified();
       obj.insert("content", sendMsg);
       obj.insert("type", "*announcement");
       QString json_str = QString(QJsonDocument(obj).toJson());
       QJsonDocument json_doc;
       json_doc.setObject(obj);
       string str = json_doc.toJson(QJsonDocument::Compact);
       QString httpUrl = domain_Url + QString("/api/webinar/v1/webinar/sendmsg?token=%1&event=cmd&msg=%2").arg(mStartLiveData.msg_token).arg(QString::fromStdString(str));
       TRACE6("%s url:%s\n", __FUNCTION__, httpUrl.toStdString().c_str());
       HTTP_GET_REQUEST request(httpUrl.toStdString());
       request.SetHttpPost(true);
       GetHttpManagerInstance()->HttpGetRequest(request, [&, sendMsg, recvObj](const std::string& msg, int code, const std::string userData) {
          TRACE6("%s code:%d msg:%s\n", __FUNCTION__, code, msg.c_str());
       });
    }

    ui.textEdit->clear();
}

void NoticeSendWdg::AppendMsg(QString msg, const QString& strTime) {
    NoticeItem *notice = new NoticeItem(ui.listWidget);
    QString localTime = strTime;
	if (localTime.trimmed().isEmpty()){
		QDateTime local(QDateTime::currentDateTime());
		localTime = local.toString("yyyy-MM-dd hh:mm:ss");
	}

    int newHeight = notice->InsertNoticeMsg(msg, localTime);
    QListWidgetItem *item = new QListWidgetItem(ui.listWidget);
    ui.listWidget->setItemWidget(item, notice);
    int count = ui.listWidget->count();
    ui.listWidget->insertItem(count,item);
    item->setSizeHint(QSize(320, newHeight));
    ui.listWidget->scrollToBottom();
}

void NoticeSendWdg::onSliderChanged(int p) {
   //if (p == ui.listWidget->verticalScrollBar()->maximum())
   //{
   //   //QMessageBox::information(this, "Warning", "You reached the bottom of the vertical scroll bar!");
   //   int a = 0;
   //   a = 0;
   //}
}

void NoticeSendWdg::slot_textChanged() {
    QString input_msg = ui.textEdit->toPlainText();
    if (input_msg.isEmpty()) {
        ui.pushButton_send->setEnabled(false);
        ui.pushButton_send->setStyleSheet(CanNotSend);
    }
    else {
        ui.pushButton_send->setEnabled(true);
        if (input_msg.length() > 300){
            QTextCursor cursor = ui.textEdit->textCursor();
            cursor.movePosition(QTextCursor::End);
            if (cursor.hasSelection()) {
                cursor.clearSelection();
            }
            cursor.deletePreviousChar();
            //设置当前的光标为更改后的光标
            ui.textEdit->setTextCursor(cursor);
        }
        ui.pushButton_send->setStyleSheet(CanSend);
    }
}