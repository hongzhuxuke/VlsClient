#include "Msg_VhallRightExtraWidget.h"
#include "Msg_OBSControl.h"
#include "Msg_CommonToolKit.h"
#include "Msg_MainUI.h"

#include "VhallRightExtraWidgetIns.h"
#include "VhallRightExtraWidgetLogic.h"
#include "IMainUILogic.h"
#include "AlertDlg.h"
#include "VHHttp.h"
#include "DebugTrace.h"
#include <windows.h>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>

#include "VhallRightExtraWidgetIns.h"
#include "ICommonData.h"
#include "ConfigSetting.h" 
#include "pathmanager.h "
#include "IOBSControlLogic.h"
#include "pub.Const.h"

VhallRightExtraWidgetLogic::VhallRightExtraWidgetLogic(void) :
QObject(NULL)
, m_lRefCount(0)
, m_pCreateRecordTimer(NULL)
, m_bCreateRecord(false){
}
VhallRightExtraWidgetLogic::~VhallRightExtraWidgetLogic(void) {
   TRACE6("%s delete end\n", __FUNCTION__);
}

BOOL VhallRightExtraWidgetLogic::Create() {
	VH::CComPtr<IMainUILogic> pMainUILogic = NULL;
	DEF_GET_INTERFACE_PTR(SingletonVhallRightExtraWidgetIns, PID_IMainUI, IID_IMainUILogic, pMainUILogic, return FALSE);
	m_pCreateRecordTimer = new QTimer(this);
	if (m_pCreateRecordTimer){
		connect(m_pCreateRecordTimer, SIGNAL(timeout()), this, SLOT(SlotCreateRecordTimeOut()));
	}

	SingletonVhallRightExtraWidgetIns::Instance().PostCRMessage(MSG_VHALLRIGHTEXTRAWIDGET_CREATE, NULL, 0);
	connect(this, SIGNAL(SigAskServerPlayback(bool)), this, SLOT(SlotAskServerPlayback(bool)));
	connect(this, SIGNAL(SigCanNotCreateRecord(bool)), this, SLOT(SlotCanNotCreateRecord(bool)));
	return TRUE;
}


void VhallRightExtraWidgetLogic::Destroy() {
	
}

/////////////////////////////IUnknown接口/////////////////////////////////////////
HRESULT STDMETHODCALLTYPE VhallRightExtraWidgetLogic::QueryInterface(REFIID riid, void ** appvObject) {
	if (riid == IID_VHIUnknown) {
		*appvObject = (VH_IUnknown*)this;
		AddRef();
		return CRE_OK;
	}
	else if (IID_IVhallRightExtraWidgetLogic == riid) {
		*appvObject = (IVhallRightExtraWidgetLogic*)this;
		AddRef();
		return CRE_OK;
	}
	return CRE_NOINTERFACE;
}
ULONG STDMETHODCALLTYPE VhallRightExtraWidgetLogic::AddRef(void) {
	return ::InterlockedIncrement(&m_lRefCount);
}

ULONG STDMETHODCALLTYPE VhallRightExtraWidgetLogic::Release(void) {
	return ::InterlockedDecrement(&m_lRefCount);
}

// 处理消息
void VhallRightExtraWidgetLogic::DealMessage(DWORD adwMessageID, void* apData, DWORD adwLen) {
	switch (adwMessageID)
	{
	   case MSG_VHALLRIGHTEXTRAWIDGET_STOP_CREATE_RECORD_TIMER: {
			StopCreateRecordTimer();
		}
		   break;
	 //  case MSG_OBSCONTROL_PUBLISH:  {//处理推流信息
		//	DealStartPublish(apData, adwLen);
		//}
		//   break;
	   case MSG_VHALLRIGHTEXTRAWIDGET_INITCOMMONINFO:{//初始化通用信息
			DealInitCommonInfo(apData, adwLen);
		}
		break;
	   case MSG_VHALLRIGHTEXTRAWIDGET_DO_REFRESH:{//执行刷新操作
			Refresh();
		}
		   break;
   default:
      break;
	}
}

void VhallRightExtraWidgetLogic::DealInitCommonInfo(void* apData, DWORD adwLen) {
	qDebug() << "##VhallRightExtraWidgetLogic::DealInitCommonInfo##";
	DEF_CR_MESSAGE_DATA_DECLARE_CA(STRU_VHALLRIGHTEXTRAWIDGET_COMMONINFO, loMessage, return);
	m_msgToken = loMessage.msgToken;
	m_msgUrl = loMessage.msgUrl;
}

bool VhallRightExtraWidgetLogic::commitRecord()
{
	bool bRef = false;
	VH::CComPtr<IMainUILogic> pMainUILogic;
	DEF_GET_INTERFACE_PTR(SingletonVhallRightExtraWidgetIns, PID_IMainUI, IID_IMainUILogic, pMainUILogic, return false, ASSERT(FALSE));
	QString strToken = pMainUILogic->GetToken();

	if (pMainUILogic->IsStopRecord()){
		bRef = true;
	}
	else {
		//结束直播时不用再次通知结束录制，只需要更改本地状态即可。
		m_lastGetError = "";
		m_lastGetCode = 200;
		bRef = true;
		VH::CComPtr<IOBSControlLogic> pOBSControlLogic;
		DEF_GET_INTERFACE_PTR(SingletonVhallRightExtraWidgetIns, PID_IOBSControl, IID_IOBSControlLogic, pOBSControlLogic, return false);
		pOBSControlLogic->StopRecord();
		pMainUILogic->SetRecordState(eRecordState_Stop);
	}
	return bRef;
}

bool VhallRightExtraWidgetLogic::Get(QString method, QString param, bool bExit) {
	QString url = m_msgUrl + method + "?token=" + m_msgToken + param;
	TRACE6("%s url:%s\n", __FUNCTION__, url.toStdString().c_str());
	m_lastGetError = "";
	m_lastGetCode = 0;
	qDebug() << url;
	if (bExit || ("createrecord" == method) || ("stopwebinar" == method)) {
		VHHttp http;
		QJsonObject res = http.Get(url, 15000);
		if (!res.isEmpty()) {
			if ("stopwebinar" == method) {
				QString code = res["code"].toString();
            TRACE6("%s method:%s code:%d\n", __FUNCTION__, method.toStdString().c_str(), code);
				if (code == "200") {
					if ("createrecord" == method) {
						QJsonObject data = res["data"].toObject();
						int id = data["id"].toInt();
						m_stopId = QString::number(id);
					}
					m_lastGetCode = 200;
				}
				else {
					m_lastGetCode = code.toInt();
					return false;
				}
			}
			else if ("startwebinar" == method) {
				QString code = res["code"].toString();
            TRACE6("%s startwebinar code:%d\n", __FUNCTION__, code);
				if (code == "200") {
					m_lastGetCode = 200;
					return true;
				}
				m_lastGetCode = code.toInt();
				m_lastGetError = res["msg"].toString();

				qDebug() << "VhallRightExtraWidgetLogic::Get startwebinar Failed!" << res << m_lastGetError;
				return false;
			}
			else {
				int code = res["code"].toInt();
            TRACE6("%s code:%d\n", __FUNCTION__, code);
				m_lastGetCode = code;
				if (code == 200) {
					if ("createrecord" == method) {
						QJsonObject data = res["data"].toObject();
						int id = data["id"].toInt();
						m_stopId = QString::number(id);
					}
				}
				else {
					return false;
				}
			}
		}
		else {
         TRACE6("%s return false\n", __FUNCTION__);
			return false;
		}
	}
	else {
		STRU_HTTPCENTER_HTTP_RQ loRQ;
		loRQ.m_dwPluginId = ENUM_PLUGIN_OBSCONTROL;
		wcsncpy(loRQ.m_wzRequestUrl, url.toStdWString().c_str(), DEF_MAX_HTTP_URL_LEN);
		SingletonVhallRightExtraWidgetIns::Instance().PostCRMessage(MSG_HTTPCENTER_HTTP_RQ, &loRQ, sizeof(loRQ));
      TRACE6("%s PostCRMessage\n", __FUNCTION__);
	}
	return true;
}
bool STDMETHODCALLTYPE VhallRightExtraWidgetLogic::GenRecord() {
	if (Get("stopwebinar", "", true)){
		return Get("createrecord", "", true);
	}
	return false;
}

int STDMETHODCALLTYPE VhallRightExtraWidgetLogic::CreateRecord() {
   if (Get("createrecord", "", true)) {
      AskGenerateRecord();
   }
   return 0;
}

int STDMETHODCALLTYPE VhallRightExtraWidgetLogic::AskGenerateRecord(const char* stopId /*= NULL*/) {
   QString strStopId = m_stopId;
   if (stopId != NULL) {
      strStopId = QString(stopId);
   }
	AlertDlg tip(QString::fromWCharArray(L"自动生成回放成功，是否设置为默认回放？"), true, NULL);
	if (tip.exec() == QDialog::Accepted) {
		Get("default-record", "&id=" + strStopId, true);
		return QDialog::Accepted;
	}
	return QDialog::Rejected;
}

int STDMETHODCALLTYPE VhallRightExtraWidgetLogic::StopWebnair(bool bIsCheckShortTime /*= true*/) {
	int iRef = eStopWebNair_Fail;
   TRACE6("%s stopwebinar request start\n", __FUNCTION__);
	if (Get("stopwebinar", "", true)) {
		commitRecord();
		iRef = eStopWebNair_SUC;
		if (bIsCheckShortTime && !GetCreateRecordState()) {
			VH::CComPtr<IMainUILogic> pMainUILogic;
			DEF_GET_INTERFACE_PTR(SingletonVhallRightExtraWidgetIns, PID_IMainUI, IID_IMainUILogic, pMainUILogic, return iRef);
			QWidget *pMainUIRenderWidget = (QWidget *)pMainUILogic->GetParentWndForTips();
			AlertDlg alertDlg(QString::fromWCharArray(L"直播时长过短，不支持生成回放"), false, NULL);
			alertDlg.CenterWindow(pMainUIRenderWidget);
			alertDlg.exec();   
        TRACE6("%s stopwebinar iRef:%d\n", __FUNCTION__, iRef);
			return   eStopWebNair_SUC;
		}

		if (Get("createrecord", "", true)) {
			AskGenerateRecord();
        TRACE6("%s stopwebinar iRef:%d\n", __FUNCTION__, iRef);
			return iRef;
		}
		else
		{
			StopCreateRecordTimer();
		}
	}
	
   TRACE6("%s stopwebinar iRef:%d\n", __FUNCTION__, iRef);
	return iRef;
}

bool STDMETHODCALLTYPE VhallRightExtraWidgetLogic::StartWebnair() {
	if (Get("startwebinar", "", true)) {
		StartCreateRecordTimer();
		return true;
	}

	return false;
}
void STDMETHODCALLTYPE VhallRightExtraWidgetLogic::GetLastError(wchar_t *msg) {
	qDebug() << "VhallRightExtraWidgetLogic::GetLastError() " << m_lastGetError;
	m_lastGetError.toWCharArray(msg);
}
int STDMETHODCALLTYPE VhallRightExtraWidgetLogic::GetLastCode() {
	qDebug() << "VhallRightExtraWidgetLogic::GetLastCode() " << m_lastGetCode;
	return m_lastGetCode;
}
void STDMETHODCALLTYPE VhallRightExtraWidgetLogic::StartTimer() {
	StartCreateRecordTimer();
}

//void VhallRightExtraWidgetLogic::DealStartPublish(void* apData, DWORD adwLen) {
//	return;
//	DEF_CR_MESSAGE_DATA_DECLARE_CA(STRU_OBSCONTROL_PUBLISH, loMessage, return);
//	if (loMessage.m_bIsStartPublish) {
//		Get("startwebinar", "", loMessage.m_bExit);
//		StartCreateRecordTimer();
//	}
//	else {
//		return;
//		Get("stopwebinar", "", loMessage.m_bExit);
//		TRACE6("VhallRightExtraWidgetLogic::DealStartPublish stopwebinar\n");
//		if (!m_bCreateRecord && (!loMessage.m_bExit || loMessage.m_bIsCloseAppWithPushing)) {
//			if (!loMessage.m_bExit){
//				emit SigCanNotCreateRecord(loMessage.m_bExit);
//			}
//		}
//		else if (loMessage.m_bExit) {
//			// 正直播&&关闭程序时，生成回放接口
//			if (loMessage.m_bServerPlayback){
//				Get("createrecord", "", loMessage.m_bExit);
//				TRACE6("VhallRightExtraWidgetLogic::DealStartPublish: loMessage.m_bExit createrecord\n");
//			}
//			SingletonVhallRightExtraWidgetIns::Instance().PostCRMessage(MSG_MAINUI_DO_CLOSE_MAINWINDOW, NULL, 0);
//		}
//		else{
//			// 停止直播按钮时，生成回放接口
//			if (loMessage.m_bServerPlayback) {
//				Get("createrecord", "", loMessage.m_bExit);
//				TRACE6("VhallRightExtraWidgetLogic::DealStartPublish: loMessage.m_bServerPlayback createrecord\n");
//				AskGenerateRecord();
//			}
//			else {
//				SlotAskServerPlayback(loMessage.m_bExit);
//				TRACE6("VhallRightExtraWidgetLogic::DealStartPublish: loMessage.m_bServerPlayback SlotAskServerPlayback\n");
//			}
//		}
//		StopCreateRecordTimer();
//	}
//}

void VhallRightExtraWidgetLogic::SlotAskServerPlayback(bool bExit) {
	VH::CComPtr<IMainUILogic> pMainUILogic;
	DEF_GET_INTERFACE_PTR(SingletonVhallRightExtraWidgetIns, PID_IMainUI, IID_IMainUILogic, pMainUILogic, return);
	QWidget *pMainUIWidget = NULL;
	pMainUILogic->GetMainUIWidget((void **)&pMainUIWidget);

	if (this->GenRecord()) {
		this->AskGenerateRecord();
	}
	else {
		AlertDlg tip(QString::fromWCharArray(L"回放生成失败，请前往SaaS平台查看"), false, NULL);
		tip.exec();
	}

	if (bExit) {
		SingletonVhallRightExtraWidgetIns::Instance().PostCRMessage(MSG_MAINUI_DO_CLOSE_MAINWINDOW, NULL, 0);
	}
}

//void STDMETHODCALLTYPE VhallRightExtraWidgetLogic::MouseMoved() {
//
//}

//void VhallRightExtraWidgetLogic::ExBarClicked(REBarShowMode mode) {
//	VH::CComPtr<IMainUILogic> pMainUILogic = NULL;
//	DEF_GET_INTERFACE_PTR(SingletonVhallRightExtraWidgetIns, PID_IMainUI, IID_IMainUILogic, pMainUILogic, return);
//	if (REBarShowMode_Shrink == mode) {
//		//m_exWidget->SetShrink(false);
//		if (pMainUILogic) {
//			pMainUILogic->SetShrink(false);
//		}
//	}
//	else if (REBarShowMode_Display == mode) {
//		//m_exWidget->SetShrink(true);
//		if (pMainUILogic) {
//			pMainUILogic->SetShrink(true);
//		}
//	}
//	//SlotClickedTest();
//}

//void VhallRightExtraWidgetLogic::SlotClickedTest() {
//	qDebug() << "VhallRightExtraWidgetLogic::SlotClickedTest";
//	
//}

void VhallRightExtraWidgetLogic::GetStartStreamUrl(wchar_t *buf){
	QString url = m_msgUrl + "startwebinar?token=" + m_msgToken;
	url.toWCharArray(buf);
}

void VhallRightExtraWidgetLogic::Refresh() {
	StopCreateRecordTimer();
}
void VhallRightExtraWidgetLogic::SlotUnInitFinished() {
	qDebug() << "VhallRightExtraWidgetLogic::SlotUnInitFinished()";
	SingletonVhallRightExtraWidgetIns::Instance().PostCRMessage(MSG_VHALLRIGHTEXTRAWIDGET_END_REFRESH, NULL, 0);
}

void VhallRightExtraWidgetLogic::SlotCreateRecordTimeOut() {
	m_bCreateRecord = true;
	if (m_pCreateRecordTimer) {
		m_pCreateRecordTimer->stop();
	}
}

void VhallRightExtraWidgetLogic::SlotCanNotCreateRecord(bool bExit) {
	VH::CComPtr<IMainUILogic> pMainUILogic;
	DEF_GET_INTERFACE_PTR(SingletonVhallRightExtraWidgetIns, PID_IMainUI, IID_IMainUILogic, pMainUILogic, return);
	QWidget *pMainUIRenderWidget = NULL;
	pMainUILogic->GetContentWidget((void **)&pMainUIRenderWidget);
	if (pMainUIRenderWidget){
		QString tips = QString::fromWCharArray(L"直播时长过短，不支持生成回放");
		AlertDlg alertDlg(tips, false, NULL);
		alertDlg.setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint | Qt::WindowStaysOnTopHint);
		alertDlg.CenterWindow(pMainUIRenderWidget);
		if (alertDlg.exec() == QDialog::Accepted) {
			qDebug() << "###########VhallRightExtraWidgetLogic::SlotCanNotCreateRecord #############";
		}

		if (bExit) {
			SingletonVhallRightExtraWidgetIns::Instance().PostCRMessage(MSG_MAINUI_DO_CLOSE_MAINWINDOW, NULL, 0);
		}
	}
}

bool STDMETHODCALLTYPE VhallRightExtraWidgetLogic::GetCreateRecordState() {
	return m_bCreateRecord;
}

void VhallRightExtraWidgetLogic::StopCreateRecordTimer() {
	if (m_pCreateRecordTimer) {
		m_pCreateRecordTimer->stop();
	}
	m_bCreateRecord = false;
}

void VhallRightExtraWidgetLogic::StartCreateRecordTimer() {
	if (m_pCreateRecordTimer) {
		m_pCreateRecordTimer->stop();
		m_pCreateRecordTimer->start(CREATE_RECORD_TIME);
	}
	m_bCreateRecord = false;
}

//void STDMETHODCALLTYPE VhallRightExtraWidgetLogic::ShowExtraWidget(bool bShow /*= true*/) {
//
//}

char* STDMETHODCALLTYPE VhallRightExtraWidgetLogic::GetMsgToken() {
	char *msgToken = NULL;
	if (!m_msgToken.isEmpty()) {
		int len = m_msgToken.toLocal8Bit().size();
		msgToken = new char[len + 1];
		memset(msgToken, 0, len + 1);
		memcpy(msgToken, m_msgToken.toLocal8Bit().data(), len);
	}
	return msgToken;
}

bool VhallRightExtraWidgetLogic::IsStopRecordState() {
	bool bRef = false;
	VH::CComPtr<IMainUILogic> pMainUILogic;
	DEF_GET_INTERFACE_PTR(SingletonVhallRightExtraWidgetIns, PID_IMainUI, IID_IMainUILogic, pMainUILogic, return false, ASSERT(FALSE));
	QString strToken = pMainUILogic->GetToken();

	if (pMainUILogic->IsStopRecord()) {
		bRef = true;
	}
	return bRef;
}