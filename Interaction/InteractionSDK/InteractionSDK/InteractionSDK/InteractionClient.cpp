#include "stdafx.h"
#include "InteractionClient.h"
#include "BaseThread.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QEventLoop>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QTimer>
#include <QDateTime>
#include <ShlObj.h>
#include <tchar.h>
//#include "json.h"
#include "sio_socket.h"
#include "DebugTrace.h"
#include "vhHttpProxy.h"
#include <QNetworkProxy>
#include <QVariantMap>
#include "pathmanager.h"

#if defined(_MSC_VER) || defined(_MSC_EXTENSIONS)
#define DELTA_EPOCH_IN_MICROSECS  11644473600000000Ui64
#else
#define DELTA_EPOCH_IN_MICROSECS  11644473600000000ULL
#endif

#define VH_LOG_DIR L"\\vhlog\\"
#ifndef _TIMEZONE_DEFINED 
struct timezone {
   int  tz_minuteswest; /* minutes W of Greenwich */
   int  tz_dsttime;     /* type of dst correction */
};

#endif


CCriticalSection g_oEventInteraction;	         //事件队列锁
InteractionClient *g_client = NULL;
HANDLE g_hSioClosedEvent = NULL;
HANDLE g_hKeepLiveClosedHandle = NULL;
HANDLE g_hKeepLiveWaitHandle = NULL;
HANDLE g_hHttpTaskClosedEvent = NULL;
HANDLE g_hWSClosedEvent = NULL;


void SendSocketIOMsg(QString &msg) {
   CCriticalAutoLock loLock(g_oEventInteraction);
   if (g_client) {
      g_client->PubSocketIOSendMsg(msg);
   }
   else{
	   TRACE6("%s g_client is null", __FUNCTION__);
   }
}
void SendWebSocketMsg(QString &msg) {
   CCriticalAutoLock loLock(g_oEventInteraction);
   if (g_client) {
      g_client->PubWebSocketSendMsg(msg);
   }
}

void InitDebugTrace(std::wstring awstrDllName, int aiTraceLevel, bool abUseSameLog/* = false*/) {
	wchar_t lwzDllPath[MAX_PATH + 1] = { 0 };

	SHGetFolderPathW(NULL, CSIDL_MYDOCUMENTS, NULL, SHGFP_TYPE_CURRENT, lwzDllPath);
	wcscat_s(lwzDllPath, _MAX_PATH, L"\\VHallHelper");

	//首先设置日志打印选项
	CDebugTrace::SetTraceLevel(aiTraceLevel);

	CDebugTrace::SetTraceOptions(CDebugTrace::GetTraceOptions() \
		| CDebugTrace::Timestamp & ~CDebugTrace::LogLevel \
		& ~CDebugTrace::FileAndLine | CDebugTrace::AppendToFile\
		| CDebugTrace::PrintToConsole);

	//生成TRACE文件名
	SYSTEMTIME loSystemTime;
	GetLocalTime(&loSystemTime);

	wchar_t lwzLogFileName[255] = { 0 };

	if (abUseSameLog) {
		wsprintf(lwzLogFileName, L"%s%s%s_%4d_%02d_%02d_%02d_%02d%s", lwzDllPath, VH_LOG_DIR, L"GGLogFile", loSystemTime.wYear, loSystemTime.wMonth, loSystemTime.wDay, loSystemTime.wHour, loSystemTime.wMinute, L".log");
	}
	else {
		wsprintf(lwzLogFileName, L"%s%s%s_%4d_%02d_%02d_%02d_%02d%s", lwzDllPath, VH_LOG_DIR, awstrDllName.c_str(), loSystemTime.wYear, loSystemTime.wMonth, loSystemTime.wDay, loSystemTime.wHour, loSystemTime.wMinute, L".log");
	}

	CDebugTrace::SetLogFileName(lwzLogFileName);
}



void InteractionClient::ParseWebsocketData(char* data) {
   QString websocketMsg = QString(data);
   SendWebSocketMsg(websocketMsg);
   QJsonObject objvalue = CPathManager::GetJsonObjectFromString(websocketMsg);
   //Json::Reader reader;
   //Json::Value value, text, dataText;
   //if (!reader.parse(data, value)) {
   //   return;
   //}
   //string stext = value["text"].asString();
   //string decodeString = QByteArray::fromPercentEncoding(stext.c_str());

   //if (!reader.parse(decodeString, text)) {
   //   return;
   //}
   QString stext = objvalue.value("text").toString();
   QString decodeString = QByteArray::fromPercentEncoding(stext.toStdString().c_str());
   QJsonObject objText = CPathManager::GetJsonObjectFromString(decodeString);

   //string user_id = text["user_id"].asString();
   //string user_name = text["user_name"].asString();
   //string event = text["event"].asString();
   //dataText = text["data"];
   QString user_id = objText.value("user_id").toString();
   QString user_name = objText.value("user_name").toString();
   QString event = objText.value("event").toString();

   if (objText.value("data").isObject())
   {

	   //QString strDataText = objText.value("data").toString();
	   //QJsonObject objDataText = CPathManager::GetJsonObjectFromString(strDataText);
	   QJsonObject objDataText = objText.value("data").toObject();
	   Event oEvent;
	   oEvent.m_eventType = e_eventType_msg;
	   if (event == "online") {
		   oEvent.m_eMsgType = e_Ntf_UserOnline;
		   QString user_role = objDataText.value("role").toString();
		   wcscpy(oEvent.m_oUserInfo.m_szUserID, user_id.toStdWString().c_str());
		   wcscpy(oEvent.m_oUserInfo.m_szUserName, user_name.toStdWString().c_str());
		   wcscpy(oEvent.m_oUserInfo.m_szRole, user_role.toStdWString().c_str());
		   int  is_gag = objDataText.value("is_gag").toInt();
		   oEvent.m_isGag = (is_gag != 0);
		   oEvent.m_sumNumber = objDataText.value("concurrent_user").toInt();

	   }
	   else if (event == "offline") {
		   oEvent.m_eMsgType = e_Ntf_UserOffline;
		   QString user_role = objDataText.value("role").toString();
		   wcscpy(oEvent.m_oUserInfo.m_szUserID, user_id.toStdWString().c_str());
		   wcscpy(oEvent.m_oUserInfo.m_szUserName, user_name.toStdWString().c_str());
		   wcscpy(oEvent.m_oUserInfo.m_szRole, user_role.toStdWString().c_str());
		   oEvent.m_sumNumber = objDataText.value("concurrent_user").toInt();
	   }
	   else if (event == "msg" || event == "pay") {

		   string chatText = QByteArray::fromPercentEncoding(stext.toStdString().c_str());

		   oEvent.m_eMsgType = e_RQ_UserChat;
		   wcscpy(oEvent.m_oUserInfo.m_szUserID, user_id.toStdWString().c_str());
		   strcpy(oEvent.m_wzText, chatText.c_str());
		   if (chatText.length() < 2048) {
			   TRACE6("%s recv chatText:%s", __FUNCTION__, chatText.c_str());
		   }
	   }
	   else {
		   return;
	   }
	   ExecuteCallBackTask(oEvent);
   }
   else
   {
	   TRACE6("%s : objText.value(\"data\") is NOT Object\n", __FUNCTION__);
   }
}

InteractionClient::InteractionClient()
: m_hHttpEvent(NULL)
, m_pDelegate(NULL){
   CCriticalAutoLock loLock(g_oEventInteraction);
   g_client = this;
   InitDebugTrace(L"InteractionClient", 6, false);
}

InteractionClient::~InteractionClient() {
   if (g_hSioClosedEvent) {
      CloseHandle(g_hSioClosedEvent);
      g_hSioClosedEvent = NULL;
   }
   if (g_hHttpTaskClosedEvent) {
      CloseHandle(g_hHttpTaskClosedEvent); 
      g_hHttpTaskClosedEvent = NULL;
   }
   if (g_hWSClosedEvent){
      CloseHandle(g_hWSClosedEvent);
      g_hWSClosedEvent = NULL;
   }
   CCriticalAutoLock loLock(g_oEventInteraction);
   if (g_client) {
      g_client = NULL;
   }
}

BOOL InteractionClient::Init(Delegate* pDelegate, Key_Data& oKeyData) {
   TRACE6("%s start\n", __FUNCTION__);
   CCriticalAutoLock oAutoLock(m_oSafeLock);
   do {
      m_pDelegate = pDelegate;
      m_oKeyData = oKeyData;
      {
         //HTTP Proxy User HTTP pulling.
         CCriticalAutoLock oAutoLock(m_oHttpProxyInfoLock);
         if (strlen(oKeyData.m_proxy_ip) != 0) {
            memcpy(m_httpProxyInfo.ip, oKeyData.m_proxy_ip, sizeof(oKeyData.m_proxy_ip));
            memcpy(m_httpProxyInfo.user, oKeyData.m_proxy_username, sizeof(oKeyData.m_proxy_username));
            memcpy(m_httpProxyInfo.pwd, oKeyData.m_proxy_password, sizeof(oKeyData.m_proxy_password));
            m_httpProxyInfo.port = QString(oKeyData.m_proxy_port).toInt();
            TRACE6("%s use proxy http pulling\n", __FUNCTION__);
         }
         else {
            memset(&m_httpProxyInfo, 0, sizeof(m_httpProxyInfo));
         }
      }

      m_bHttpLoopRun = true;
      if (!InitHttp()) {
         break;
      }

      if (!InitSocketIO()) {
         break;
      }

      if (!InitWebSocket()) {
         break;
      }

      if (!InitKeepLiveThread()) {
         break;
      }

      TRACE6("%s ok end\n", __FUNCTION__);
      return TRUE;
   } while (FALSE);
   m_bHttpLoopRun = false;
   TRACE6("%s err end\n", __FUNCTION__);
   return FALSE;
}

BOOL InteractionClient::InitHttp() {
   TRACE6("%s start\n", __FUNCTION__);
   if (g_hHttpTaskClosedEvent == NULL) {
      g_hHttpTaskClosedEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
   }
   CBaseThread loHttpThread;
   loHttpThread.BeginThread(InteractionClient::HttpReqThread, this);
   TRACE6("%s end\n", __FUNCTION__);
   return TRUE;
}

void InteractionClient::UnInitHttp() {
   if (g_hHttpTaskClosedEvent != NULL){
      WaitForSingleObject(g_hHttpTaskClosedEvent, 60000);
      CloseHandle(g_hHttpTaskClosedEvent);
      g_hHttpTaskClosedEvent = NULL;
   }
}

BOOL InteractionClient::InitSocketIO() {
   //SIO关闭通知事件
   if (g_hSioClosedEvent == NULL) {
      g_hSioClosedEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
   }
   TRACE6("%s start\n", __FUNCTION__);
   CBaseThread loSocketIOThread;
   loSocketIOThread.BeginThread(InteractionClient::SocketIOReqThread, this);
   TRACE6("%s end\n", __FUNCTION__);
   return TRUE;
}

void InteractionClient::UnInitSocketIO() {
   TRACE6("%s start\n", __FUNCTION__);
   if (g_hSioClosedEvent){
      WaitForSingleObject(g_hSioClosedEvent, 60000);
      CloseHandle(g_hSioClosedEvent);
      g_hSioClosedEvent = NULL;
   }
   TRACE6("%s end\n", __FUNCTION__);
}

BOOL InteractionClient::InitKeepLiveThread() {
   if (g_hKeepLiveClosedHandle == NULL) {
      g_hKeepLiveClosedHandle = ::CreateEvent(NULL, FALSE, FALSE, NULL);
   }
   if (g_hKeepLiveWaitHandle == NULL) {
      g_hKeepLiveWaitHandle = ::CreateEvent(NULL, FALSE, FALSE, NULL);
   }
   TRACE6("%s start\n", __FUNCTION__);
   CBaseThread loSocketIOThread;
   loSocketIOThread.BeginThread(InteractionClient::KeepLiveThread, this);
   TRACE6("%s end\n", __FUNCTION__);
   return TRUE;
}

void InteractionClient::UnInitKeepLiveThread() {
   TRACE6("%s start\n", __FUNCTION__);
   if (g_hKeepLiveWaitHandle) {
      ::SetEvent(g_hKeepLiveWaitHandle);
   }
   if (g_hKeepLiveClosedHandle) {
      WaitForSingleObject(g_hKeepLiveClosedHandle, 60000);
      CloseHandle(g_hKeepLiveClosedHandle);
      g_hKeepLiveClosedHandle = NULL;
   }
   TRACE6("%s end\n", __FUNCTION__);
}

void InteractionClient::OnHttpMsg(QString cmd, QString value) {
   QJsonObject obj;
   QJsonDocument doc;
   QString msgSocketIO;

   obj["key"] = cmd;
   obj["value"] = value;

   doc.setObject(obj);
   msgSocketIO = doc.toJson(QJsonDocument::Compact);
   

   doc = QJsonDocument::fromJson(value.toUtf8());
   obj = doc.object();

   QString type = obj["type"].toString();
   QString user_id = obj["user_id"].toString();
   QString status = obj["status"].toString();

   qDebug() << "OnHttpMsg " << obj << " " << type << " " << user_id;

   Event oEvent;
   oEvent.m_eventType = e_eventType_msg;

   if (type != "" && user_id != "") {
      if (type == "*kickout") {
         oEvent.m_eMsgType = e_RQ_UserKickOutOneUser;
         wcscpy(oEvent.m_oUserInfo.m_szUserID, user_id.toStdWString().c_str());
      } else if (type == "*kickoutrestore") {
         oEvent.m_eMsgType = e_RQ_UserAllowJoinOneUser;
         wcscpy(oEvent.m_oUserInfo.m_szUserID, user_id.toStdWString().c_str());
      } else if (type == "*disablechat") {
         oEvent.m_eMsgType = e_RQ_UserProhibitSpeakOneUser;
         wcscpy(oEvent.m_oUserInfo.m_szUserID, user_id.toStdWString().c_str());
      } else if (type == "*permitchat") {
         oEvent.m_eMsgType = e_RQ_UserAllowSpeakOneUser;
         wcscpy(oEvent.m_oUserInfo.m_szUserID, user_id.toStdWString().c_str());
      } else if (type == "online") {
         //主持人上线/下线消息有websocket处理，此处不做处理
         return;
      } else if (type == "offline") {
         //主持人上线/下线消息有websocket处理，此处不做处理
         return;
      }
   }
   else if (type == "*forbidchat") {
      if (status == "0") {
         oEvent.m_eMsgType = e_RQ_UserAllowSpeakAllUser;
      } else if (status == "1") {
         oEvent.m_eMsgType = e_RQ_UserProhibitSpeakAllUser;
      }
   }
   else if(cmd == "cmd"){
      //value = value.replace("\\\"", "\"");
      //value = value.left(value.length() - 1);
      //value = value.right(value.length() - 1);
	   
	   if (value.length() < 1024) {
		   TRACE6("%s cmd: %s\n", __FUNCTION__, value.toStdString().c_str());
	   }
      QJsonObject obj;
      QJsonDocument doc;
      QString msgSocketIO;
      obj["value"] = value;
      doc.setObject(obj);
      msgSocketIO = doc.toJson(QJsonDocument::Compact);
      SendSocketIOMsg(msgSocketIO);
      doc = QJsonDocument::fromJson(value.toUtf8());
      obj = doc.object();
      QString type = obj["type"].toString();
      QString user_id = obj["user_id"].toString();
      QString status = obj["status"].toString();

      if ("*announcement" == type) {
         oEvent.m_eMsgType = e_RQ_ReleaseAnnouncement;
         QString conntect = obj["content"].toString();
         strcpy(oEvent.m_wzText, conntect.toStdString().c_str());
         //oEvent.m_eventType = e_eventType_msg;
      } 
   }
   else if (cmd == "flashMsg") {
	   if (value.length() < 1024) {
		   TRACE6("%s flashMsg: %s\n", __FUNCTION__, value.toStdString().c_str());
	   }
	   oEvent.m_eMsgType = e_RQ_flashMsg;
	   SendSocketIOMsg(msgSocketIO);
	   return;
   }
   ExecuteCallBackTask(oEvent);
}


BOOL InteractionClient::InitWebSocket() {
   CBaseThread loSocketIOThread;
   if (g_hWSClosedEvent == NULL) {
      g_hWSClosedEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
   }
   loSocketIOThread.BeginThread(InteractionClient::WebSocketThread, this);
   return TRUE;
}

void InteractionClient::UnInitWebSocket() {
   if (g_hWSClosedEvent){
      WaitForSingleObject(g_hWSClosedEvent, 60000);
      CloseHandle(g_hWSClosedEvent);
      g_hWSClosedEvent = NULL;
   }
}

void InteractionClient::UnInit() {
   CCriticalAutoLock oAutoLock(m_oSafeLock);
   m_bHttpLoopRun = false;
   m_pDelegate = NULL;
   m_httpSocketIOProxy.ExitLoop();
   m_httpWebSocketProxy.ExitLoop();
   m_httpTaskProxy.ExitLoop();
   m_httpKeepLive.ExitLoop();
   UnInitHttp();
   UnInitSocketIO();
   UnInitWebSocket(); 
   UnInitKeepLiveThread();
   TRACE6("%s exit\n", __FUNCTION__);
}


void InteractionClient::ConnectServer() {
   CCriticalAutoLock oAutoLock(m_oHttpProxyInfoLock);
   memcpy(m_httpProxyInfo.ip, m_oKeyData.m_proxy_ip, sizeof(m_oKeyData.m_proxy_ip));
   memcpy(m_httpProxyInfo.user, m_oKeyData.m_proxy_username, sizeof(m_oKeyData.m_proxy_username));
   memcpy(m_httpProxyInfo.pwd, m_oKeyData.m_proxy_password, sizeof(m_oKeyData.m_proxy_password));
   m_httpProxyInfo.port = QString(m_oKeyData.m_proxy_port).toInt();
}

void InteractionClient::DisConnect() {

}

void InteractionClient::MessageRQ(const MsgRQType& eMsgType, RQData* vData /*= NULL*/) {
   //HTTP
   WCHAR wzRQUrl[2048] = { 0 };
   switch (eMsgType) {
   case e_RQ_UserOnlineList:{//在线列表   
      TRACE6("%s e_RQ_UserOnlineList\n",__FUNCTION__);
      if (vData) {
         vData->m_iCurPage = vData->m_iCurPage <= 0 ? 1 : vData->m_iCurPage;
         _snwprintf_s(wzRQUrl, 2048, L"%s%s?token=%s&curr_page=%d", m_oKeyData.m_MSG_SERVER_URL, L"onlineusers", m_oKeyData.m_szMsgToken, vData->m_iCurPage);
      }
   }
   break;
   case e_RQ_UserProhibitSpeakList: {//禁言列表
      TRACE6("%s e_RQ_UserProhibitSpeakList\n", __FUNCTION__);
      _snwprintf_s(wzRQUrl, 2048, L"%s%s?token=%s", m_oKeyData.m_MSG_SERVER_URL, L"silencedusers", m_oKeyData.m_szMsgToken);
   }
   break;
   case e_RQ_UserKickOutList:{//踢出列表
      TRACE6("%s e_RQ_UserProhibitSpeakList\n", __FUNCTION__);
      _snwprintf_s(wzRQUrl, 2048, L"%s%s?token=%s", m_oKeyData.m_MSG_SERVER_URL, L"kickedusers", m_oKeyData.m_szMsgToken);
   }
   break;
   case e_RQ_UserSpecialList:{//主持人、嘉宾、助理
      TRACE6("%s e_RQ_UserProhibitSpeakList\n", __FUNCTION__);
      _snwprintf_s(wzRQUrl, 2048, L"%s%s?token=%s", m_oKeyData.m_MSG_SERVER_URL, L"specialusers", m_oKeyData.m_szMsgToken);
   }
   break;
   case e_RQ_UserProhibitSpeakAllUser:{//发送全体禁言
      TRACE6("%s e_RQ_UserProhibitSpeakList\n", __FUNCTION__);
      _snwprintf_s(wzRQUrl, 2048, L"%s%s?token=%s&type=1", m_oKeyData.m_MSG_SERVER_URL, L"forbidchat", m_oKeyData.m_szMsgToken);
   }
   break;
   case e_RQ_UserAllowSpeakAllUser:{//发送取消全体禁言
      TRACE6("%s e_RQ_UserAllowSpeakAllUser\n", __FUNCTION__);
      _snwprintf_s(wzRQUrl, 2048, L"%s%s?token=%s&type=0", m_oKeyData.m_MSG_SERVER_URL, L"forbidchat", m_oKeyData.m_szMsgToken);
   }
   break;
   case e_RQ_ChatEmotionList:{//请求表情列表
      TRACE6("%s e_RQ_ChatEmotionList\n", __FUNCTION__);
      _snwprintf_s(wzRQUrl, 2048, L"%s%s?token=%s", m_oKeyData.m_MSG_SERVER_URL, L"emoji", m_oKeyData.m_szMsgToken);
   }
   break;
   case e_RQ_UserProhibitSpeakOneUser:{
      if (vData) {
         _snwprintf_s(wzRQUrl, 2048, L"%s%s?token=%s&user_id=%s&type=1", m_oKeyData.m_MSG_SERVER_URL, L"silence", m_oKeyData.m_szMsgToken, vData->m_oUserInfo.m_szUserID);
      }
   }
   break;
   case e_RQ_UserAllowSpeakOneUser:{
      if (vData) {
         _snwprintf_s(wzRQUrl, 2048, L"%s%s?token=%s&user_id=%s&type=2", m_oKeyData.m_MSG_SERVER_URL, L"silence", m_oKeyData.m_szMsgToken, vData->m_oUserInfo.m_szUserID);
      }
   }
   break;
   case e_RQ_UserKickOutOneUser:{
      if (vData) {
         _snwprintf_s(wzRQUrl, 2048, L"%s%s?token=%s&user_id=%s", m_oKeyData.m_MSG_SERVER_URL, L"kick", m_oKeyData.m_szMsgToken, vData->m_oUserInfo.m_szUserID);
      }
   }
   break;
   case e_RQ_UserAllowJoinOneUser:{
      if (vData) {
         _snwprintf_s(wzRQUrl, 2048, L"%s%s?token=%s&user_id=%s", m_oKeyData.m_MSG_SERVER_URL, L"restorekicked", m_oKeyData.m_szMsgToken, vData->m_oUserInfo.m_szUserID);
      }
   }
   break;
   case e_RQ_UserChat:{
      if (vData) {
         QString qText = QString::fromUtf8(vData->m_wzText);
         wchar_t wzText[2048] = { 0 };
         qText.toWCharArray(wzText);
         _snwprintf_s(wzRQUrl, 2048, L"%s?app_name=vhall&token=%s&data=%s&event=msg", m_oKeyData.m_CHAT_SERVER_URL, m_oKeyData.m_szMsgToken, wzText);

      }
   }
   break;
   case e_RQ_ReleaseAnnouncement: {
      QString url = QString(m_oKeyData.m_domain) + "/api/webinar/v1/webinar/sendmsg?token=" + QString::fromWCharArray(m_oKeyData.m_szMsgToken) + "&event=cmd&msg=" + QString(vData->m_wzText);
      url.toWCharArray(wzRQUrl);
   }
   break;
   default:
      break;
   }

   HttpReq oHttpReq;
   oHttpReq.m_eType = eMsgType;
   wcscpy(oHttpReq.m_wzRequestUrl, wzRQUrl);
   AddHttpTask(oHttpReq);
}

unsigned int InteractionClient::HttpReqThread(void * apParam) {
   InteractionClient* pThis = static_cast<InteractionClient*>(apParam);
   TRACE6("%s start\n",__FUNCTION__);
   if (pThis) {
      pThis->HttpReqWork();
   }
   if (g_hHttpTaskClosedEvent) {
      ::SetEvent(g_hHttpTaskClosedEvent);
   }
   TRACE6("%s end\n", __FUNCTION__);
   return 0;
}

unsigned int InteractionClient::HttpReqWork() {
   //创建事件句柄(自动重置)
   TRACE6("%s  start\n", __FUNCTION__);
   while (m_bHttpLoopRun) {
      //取得一个任务
      HttpReq oReq;
      if (GetHttpTask(oReq)) {
         //执行请求
         ExecuteHttpTask(oReq);
      }
      Sleep(100);
   }
   TRACE6("%s  end\n", __FUNCTION__);
   return 0;
}

void InteractionClient::AddHttpTask(HttpReq& aoReq) {
   CCriticalAutoLock loLock(m_oHttpTaskLock);
   //添加到请求队列
   m_oHttpList.push_back(aoReq);
}

BOOL InteractionClient::GetHttpTask(HttpReq& aoReq) {
   CCriticalAutoLock loLock(m_oHttpTaskLock);
   if (m_oHttpList.size() < 1) {
      return FALSE;
   }

   aoReq = m_oHttpList.front();
   m_oHttpList.pop_front();
   return TRUE;
}

BOOL InteractionClient::ExecuteHttpTask(HttpReq& oTask) {//各类用户列表信息拉取
   QString taskUrl = QString::fromWCharArray(oTask.m_wzRequestUrl);
   {
      CCriticalAutoLock oAutoLock(m_oHttpProxyInfoLock);
      if (strlen(m_httpProxyInfo.ip) == 0) {
         m_httpTaskProxy.SetHttpProxyInfo(false);
      } else {
         m_httpTaskProxy.SetHttpProxyInfo(true, m_httpProxyInfo.ip, m_httpProxyInfo.port, m_httpProxyInfo.user, m_httpProxyInfo.pwd);
      }
   }
   QByteArray dataArray;
   int errCode = -1;
   if (m_httpTaskProxy.VHGetRet2(taskUrl, DEF_HTTPLOOP_MAX_TIMEOUT, dataArray, errCode)) {
      if (!dataArray.isEmpty()) {
         QString qStrResult = QString::fromStdString(dataArray.toStdString());
         Event oReq;
         ParseJSon(oTask.m_eType, qStrResult, oReq);
         //放入解析队列
         ExecuteCallBackTask(oReq);
      }
   }
   return TRUE;
}

unsigned int InteractionClient::SocketIOReqThread(void * apParam) {
   InteractionClient* pThis = static_cast<InteractionClient*>(apParam);
   TRACE6("%s  start\n", __FUNCTION__);
   if (pThis) {
      pThis->SocketIOHttpReqWork();
      if (g_hSioClosedEvent) {
         ::SetEvent(g_hSioClosedEvent);
      }
   }
   TRACE6("%s  end\n", __FUNCTION__);
   return 0;
}

unsigned int InteractionClient::WebSocketThread(void * apParam) {
   InteractionClient* pThis = static_cast<InteractionClient*>(apParam);
   TRACE6("%s start\n", __FUNCTION__);
   int nRet = 0;
   if (pThis) {
      pThis->WebSocketLoopWork();
   }
   if (g_hWSClosedEvent) {
      ::SetEvent(g_hWSClosedEvent);
   }
   TRACE6("%s end\n", __FUNCTION__);
   return 0;
}

QJsonObject VHSocketIOGetJson(char *head) {
   QJsonDocument doc = QJsonDocument::fromJson(QByteArray(head));
   QJsonObject obj = doc.object();
   QString t = QJsonDocument(obj).toJson();
   return obj;
}

bool VHSocketIOGetKV(char *head, QString &key, QString &value) {
   bool paramSuc = true;
   //QJsonParseError json_error;
   //QString name;
   //QJsonDocument parse_doucment = QJsonDocument::fromJson(QByteArray(head), &json_error);
   //if (json_error.error == QJsonParseError::NoError) {
   //   if (parse_doucment.isArray()) {
   //      QJsonArray array = parse_doucment.array();
   //      int size = array.size();
   //      if (size == 2) {
   //         QJsonValue keyData = array.at(0);
   //         QJsonValue valueData = array.at(1);
   //         if (keyData.isString() && valueData.isString()) {
   //            key = keyData.toString();
   //            value = valueData.toString();
   //         } else if (keyData.isString() && valueData.isObject()) {
   //            key = keyData.toString();
   //            value = QJsonDocument(valueData.toObject()).toJson();
   //         } else {
   //            paramSuc = false;
   //         }
   //      } else {
   //         paramSuc = false;
   //      }
   //   }
   //}

   if (paramSuc){
      QString headStr = head;
      headStr = headStr.left(headStr.length() - 1);
      headStr = headStr.right(headStr.length() - 1);
      int firstComma = headStr.indexOf(",");

      QString leftStr = headStr.left(firstComma);
      leftStr = leftStr.left(leftStr.length() - 1);
      leftStr = leftStr.right(leftStr.length() - 1);
      QString rightStr = headStr.right(headStr.length() - firstComma - 1);

      key = leftStr;
      value = rightStr;
   }
   return true;
}

unsigned int InteractionClient::KeepLiveThread(void *Param) {
   InteractionClient* pThis = static_cast<InteractionClient*>(Param);
   TRACE6("%s  start\n", __FUNCTION__);
   if (pThis) {
      pThis->KeepLiveThreadWork();
      if (g_hKeepLiveClosedHandle) {
         ::SetEvent(g_hKeepLiveClosedHandle);
      }
   }
   TRACE6("%s  end\n", __FUNCTION__);
   return 0;
}
unsigned int InteractionClient::KeepLiveThreadWork() {
   while (m_bHttpLoopRun) {
      QString requestUrl;
      QByteArray data;
      QString cookies;
      {
         CCriticalAutoLock oAutoLock(m_oHttpProxyInfoLock);
         if (strlen(m_httpProxyInfo.ip) == 0) {
            m_httpKeepLive.SetHttpProxyInfo(false);
         }
         else {
            m_httpKeepLive.SetHttpProxyInfo(true, m_httpProxyInfo.ip, m_httpProxyInfo.port, m_httpProxyInfo.user, m_httpProxyInfo.pwd);
         }
      }
      {
         QMutexLocker l(&mKeepLiveMutex);
         QString url = "%0?token=%1&app=vhall&EIO=3&transport=polling&t=%2-%3";      //1497847412349
         QDateTime datatime;
         qint64 curT = datatime.currentDateTimeUtc().toMSecsSinceEpoch();
         QString togetUrl = url.arg(mURL).arg(mToken).arg(curT).arg(mIndex) + "&sid=" + mCookio;
         QUrl qUrl = QUrl(togetUrl);
         qUrl.setScheme("http");
         requestUrl = qUrl.toString();
         cookies = mCookio;
      }
      if (!cookies.isEmpty()) {
         m_httpKeepLive.VHPostRet(requestUrl, QString("1:2"), cookies);
      }
      if (g_hKeepLiveWaitHandle) {
         WaitForSingleObject(g_hKeepLiveWaitHandle, 10000);
      }
   }
   return true;
}
unsigned int InteractionClient::SocketIOHttpReqWork() {
   QString sid = "";
   QString cookio = "vEzvLzrbV7NF-4bJAFGh";
   QString url = "%0?token=%1&app=vhall&EIO=3&transport=polling&t=%2-%3";      //1497847412349
   QString msg_URL = QString::fromWCharArray(m_oKeyData.m_msgURL);
   //unsigned long curtime = 1497847412349;
   QString qtoken = QString::fromWCharArray(m_oKeyData.m_szMsgToken);
   int index = 0;
   while (m_bHttpLoopRun) {
      QDateTime datatime;
      qint64 curT = datatime.currentDateTimeUtc().toMSecsSinceEpoch();
      QString togetUrl = url.arg(msg_URL).arg(qtoken).arg(curT).arg(index++) + sid;
      QUrl qUrl = QUrl(togetUrl);
      qUrl.setScheme("http");
      QString requestUrl = qUrl.toString();
      {
         CCriticalAutoLock oAutoLock(m_oHttpProxyInfoLock);
         if (strlen(m_httpProxyInfo.ip) == 0) {
            m_httpSocketIOProxy.SetHttpProxyInfo(false);
         } else {
            m_httpSocketIOProxy.SetHttpProxyInfo(true, m_httpProxyInfo.ip, m_httpProxyInfo.port, m_httpProxyInfo.user, m_httpProxyInfo.pwd);
         }
      }
      QString cookis = "io=" + cookio;
      QByteArray dataArray;
      if (!sid.isEmpty()) {
         QMutexLocker l(&mKeepLiveMutex);
         mURL = msg_URL;
         mToken = qtoken;
         mIndex = index;
         mCookio = cookio;
      }
      int errCode = -1;
      if (m_httpSocketIOProxy.VHGetRet2(requestUrl, 50000, dataArray, errCode, false, cookis)) {

      }
      if (!m_bHttpLoopRun) {
         break;
      }
      Sleep(20);
      if (!dataArray.isEmpty()) {
         //parse socket.io msg
         unsigned char *head = (unsigned char *)dataArray.data();
         bool json = true;
         char *ptr = (char *)head;
         for (int i = 0; i < dataArray.length(); i++) {
            if (head[i] == '{') {
               ptr = (char *)head + i;
               break;
            } else if (head[i] == '[') {
               json = false;
               ptr = (char *)head + i;
               break;
            }
         }

         if (json) {
            QJsonObject obj = VHSocketIOGetJson(ptr);
            qDebug() << "json:" << obj;
            if (sid == "") {
               QString tmpsid = obj["sid"].toString();
               if (tmpsid != "") {
                  sid = "&sid=" + tmpsid;
                  cookio = tmpsid;
                  TRACE6("%s cookio = %s\n", __FUNCTION__, cookio.toStdString().c_str());
                  Event oEvent;
                  oEvent.m_eventType = e_eventType_opened;
                  ExecuteCallBackTask(oEvent);
               }
            }

            int code = obj["code"].toInt();
            if (code == 1) {
               sid = "";
               cookio = "";
               index = 0;
               TRACE6("%s code = 1\n",__FUNCTION__);
            }
         } else {
            QString key, value;
            if (VHSocketIOGetKV(ptr, key, value)) {
               OnHttpMsg(key, value);
            }
         }
      }
      //如果不再保活说明发送异常，重新进行请求。
      TRACE6("%s current socketio http request errCode:%d\n", __FUNCTION__, errCode);
      if (errCode == QNetworkReply::UnknownNetworkError || errCode == QNetworkReply::TimeoutError || errCode == QNetworkReply::NetworkSessionFailedError) {
         m_httpSocketIOProxy.StopSendPing();
         m_httpSocketIOProxy.ExitLoop();
         m_httpSocketIOProxy.Release();
         index = 0;
         sid = "";
         cookio = "vEzvLzrbV7NF-4bJAFGh";
         TRACE6("%s reset end\n", __FUNCTION__);
      }
   }
   m_httpSocketIOProxy.StopSendPing();
   m_httpSocketIOProxy.Release();
   TRACE6("%s exit thread \n",__FUNCTION__);
   return 0;
}

unsigned int InteractionClient::WebSocketLoopWork() {
   TRACE6("%s start\n", __FUNCTION__);
   while (m_bHttpLoopRun) {
      QDateTime datatime;
      qint64 curT = datatime.currentDateTimeUtc().toMSecsSinceEpoch();
      QString url = QString("http://" + QString::fromWCharArray(m_oKeyData.m_chatURL) + "/lp/" + QString::number(m_oKeyData.m_lRoomID) + "?_=%0&tag=0&time=&eventid=").arg(curT);
      {
         CCriticalAutoLock oAutoLock(m_oHttpProxyInfoLock);
         if (strlen(m_httpProxyInfo.ip) == 0) {
            m_httpWebSocketProxy.SetHttpProxyInfo(false);
         } else {
            m_httpWebSocketProxy.SetHttpProxyInfo(true, m_httpProxyInfo.ip, m_httpProxyInfo.port, m_httpProxyInfo.user, m_httpProxyInfo.pwd);
         }
      }
      int errCode = -1;
      QByteArray dataArray;

	  if (url.length()<1024)
	  {
		  TRACE6("%s VHGetRet:%s\n", __FUNCTION__, url.toStdString().c_str());
	  }
	  else
	  {
		  TRACE6("%s VHGetRet:%s\n", __FUNCTION__, url.mid(0, 1024).toStdString().c_str());
	  }

      if (m_httpWebSocketProxy.VHGetRet2(url, 60000, dataArray, errCode)) {

      }
      if (!m_bHttpLoopRun) {
         break;
      }      
      if (!dataArray.isEmpty()) {
         string retData = dataArray.toStdString();
         ParseWebsocketData((char*)retData.c_str()); 
      }
      TRACE6("%s websocket errCode:%d\n", __FUNCTION__, errCode);
      if (errCode != 0 && errCode <= QNetworkReply::UnknownNetworkError) {
         m_httpWebSocketProxy.Release();
      }
      Sleep(20);
   }
   m_httpWebSocketProxy.Release();
   TRACE6("%s end\n", __FUNCTION__);
   return 0;
}


BOOL InteractionClient::ExecuteCallBackTask(Event& oTask) {
   CCriticalAutoLock oAutoLock(m_oSafeLock);
   if (m_pDelegate) {
      oTask.m_lRoomID = m_oKeyData.m_lRoomID;
      if (oTask.m_eventType == e_eventType_opened) {
         m_pDelegate->onConnected(&oTask);// 用户列表（在线、禁言、提出、主持、嘉宾、助理）相关请求
      } else if (oTask.m_eventType == e_eventType_msg) {
         m_pDelegate->onMessaged(&oTask);//聊天公告、踢人、禁言相关消息处理
         //m_pDelegate->onDisconnected(&oTask);
	  } /*else if (oTask.m_eventType == e_eventType_closed) {
		 m_pDelegate->onDisconnected(&oTask);
		 }*/

   }
   return TRUE;
}

void InteractionClient::ParseJSon(MsgRQType type, QString qsJson, Event& oEvent) {
	oEvent.m_eventType = e_eventType_msg;
	QJsonObject ObjValue = CPathManager::GetJsonObjectFromString(qsJson);

	switch (type) {
	case e_RQ_UserOnlineList:				 //上线列表
	case e_RQ_UserProhibitSpeakList:  //禁言用户列表
	case e_RQ_UserKickOutList:			//踢出列表
	case e_RQ_UserSpecialList: {         //麦上列表
		QString code = ObjValue.value("code").toString();
		QString msg = ObjValue.value("msg").toString();
		if ("200" == code && "success" == msg) {
			if (ObjValue.value("data").isObject())
			{
				QJsonObject ObjData = ObjValue.value("data").toObject();
				if (e_RQ_UserOnlineList == type) {
					int pageCount = ObjData.value("total_page").toInt();
					oEvent.m_iPageCount = pageCount;
					int currentPage = ObjData.value("curr_page").toInt();
					oEvent.m_currentPage = currentPage;
					int sumNumber = ObjData.value("count").toInt();
					oEvent.m_sumNumber = sumNumber;
				}

				if (ObjData.value("users").isArray())
				{
					QJsonArray jsonUsers = ObjData.value("users").toArray();

					int iSize = jsonUsers.size();
					QJsonObject tmp;
					for (int i = 0; i < iSize; i++) {
						tmp = jsonUsers.at(i).toObject();
						QString user_id = tmp.value("id").toString();
						QString nick_name = tmp.value("nick_name").toString();
						QString role = tmp.value("role_name").toString();

						UserInfo oUserInfo;
						wcscpy(oUserInfo.m_szUserName, nick_name.toStdWString().c_str());
						wcscpy(oUserInfo.m_szUserID, user_id.toStdWString().c_str());
						wcscpy(oUserInfo.m_szRole, role.toStdWString().c_str());
						oEvent.m_oUserList.push_back(oUserInfo);
					}
				}
				else
				{
					TRACE6("%s : ObjData.value(\"users\") is NOT Array\n", __FUNCTION__);
				}
			}
			else
			{
				TRACE6("%s : ObjValue.value(\"data\").is NOT Object() \n", __FUNCTION__);
			}
		}
		else if ("500" == code) {
		}
		else {
		}
		oEvent.m_eMsgType = type;
	}
							   break;
	case e_RQ_ChatEmotionList: { //表情列表
		QString code = ObjValue.value("code").toString();
		QString msg = ObjValue.value("msg").toString();
		if ("200" == code && "success" == msg) {
			if (ObjValue.value("data").isArray())
			{
				QJsonArray ObjData = ObjValue.value("data").toArray();
				int iSize = ObjData.size();
				QJsonObject ObjTmp;
				for (int i = 0; i < iSize; i++)
				{
					ObjTmp = ObjData.at(i).toObject();
					QStringList names = ObjTmp.keys();
					QString name = names[0];
					QString strPath = ObjTmp.value(name).toString();
					oEvent.m_oImageList[name.toStdString()] = strPath.toStdString();
				}
				oEvent.m_eMsgType = type;
			}
			else
			{
				TRACE6("%s : ObjData.value(\"users\") is NOT Array\n", __FUNCTION__);
			}
		}
		else if ("500" == code) {

		}
	}
							   break;
	default:
		break;
	}
}

//socketIO 回调
void InteractionClient::SetMessageSocketIOCallBack(InteractionMsgCallBack callback) {
   m_socketIOMutex.lock();
   m_socketIOCallback = callback;
   m_socketIOMutex.unlock();
}

//websocket 回调
void InteractionClient::SetMessageWebSocketCallBack(InteractionMsgCallBack callback) {
   m_websocketMutex.lock();
   m_websocketCallback = callback;
   m_websocketMutex.unlock();
}

void InteractionClient::PubSocketIOSendMsg(QString &msg) {
	TRACE6("%s m_socketIOMutex.lock\n", __FUNCTION__);
   m_socketIOMutex.lock();
   if (m_socketIOCallback) {
	   TRACE6("%s : m_socketIOCallback is not null\n", __FUNCTION__);
      m_socketIOCallback(msg.toUtf8().data());
   }
   m_socketIOMutex.unlock();
   TRACE6("%s : m_socketIOMutex.unlock\n", __FUNCTION__);
}

void InteractionClient::PubWebSocketSendMsg(QString &msg) {
   m_websocketMutex.lock();
   if (m_websocketCallback) {
      m_websocketCallback(msg.toUtf8().data());
   }
   m_websocketMutex.unlock();
}

void InteractionClient::SetProxyAddr(const bool&  enable, const char *proxy_ip, const char* proxy_port, const char *proxy_username, const char *proxy_password) {
   //HTTP Proxy User HTTP pulling.
   CCriticalAutoLock oAutoLock(m_oHttpProxyInfoLock);
   if (enable) {
      memcpy(m_httpProxyInfo.ip, proxy_ip, sizeof(m_httpProxyInfo.ip));
      memcpy(m_httpProxyInfo.user, proxy_username, sizeof(m_httpProxyInfo.user));
      memcpy(m_httpProxyInfo.pwd, proxy_password, sizeof(m_httpProxyInfo.pwd));
      m_httpProxyInfo.port = QString(proxy_port).toInt();
      TRACE6("%s use proxy http pulling\n", __FUNCTION__);
   } else {
      memset(&m_httpProxyInfo, 0, sizeof(m_httpProxyInfo));
   }
}



