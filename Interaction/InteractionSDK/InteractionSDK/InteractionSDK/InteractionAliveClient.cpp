#include "stdafx.h"
#include "InteractionAliveClient.h"
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
#include "vhalluserinfomanager.h"

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

CCriticalSection g_oEventAliveInteraction;	         //�¼�������
InteractionAliveClient *g_AliveClient = NULL;
HANDLE g_hAliveSioClosedEvent = NULL;
HANDLE g_hKeepLiveClosedEvent = NULL;
HANDLE g_hKeepLiveWaitEvent = NULL;
HANDLE g_hAliveHttpTaskClosedEvent = NULL;

void SendAliveSocketIOMsg(QString &msg) {
   CCriticalAutoLock loLock(g_oEventAliveInteraction);
   if (g_AliveClient) {
      g_AliveClient->PubSocketIOSendMsg(msg);
   }
   else {
      TRACE6("%s g_client is null", __FUNCTION__);
   }
}

void InitAliveDebugTrace(std::wstring awstrDllName, int aiTraceLevel, bool abUseSameLog/* = false*/) {
   wchar_t lwzDllPath[MAX_PATH + 1] = { 0 };

   SHGetFolderPathW(NULL, CSIDL_MYDOCUMENTS, NULL, SHGFP_TYPE_CURRENT, lwzDllPath);
   wcscat_s(lwzDllPath, _MAX_PATH, L"\\VHallHelper");

   //����������־��ӡѡ��
   CDebugTrace::SetTraceLevel(aiTraceLevel);

   CDebugTrace::SetTraceOptions(CDebugTrace::GetTraceOptions() \
      | CDebugTrace::Timestamp & ~CDebugTrace::LogLevel \
      & ~CDebugTrace::FileAndLine | CDebugTrace::AppendToFile\
      | CDebugTrace::PrintToConsole);

   //����TRACE�ļ���
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

QJsonObject VHAliveSocketIOGetJson(char *head) {
   QJsonDocument doc = QJsonDocument::fromJson(QByteArray(head));
   QJsonObject obj = doc.object();
   QString t = QJsonDocument(obj).toJson();
   return obj;
}

bool VHAliveSocketIOGetKV(char *head, QString &key, QString &value) {
   bool paramSuc = true;
   QJsonParseError json_error;
   QString name;
   QJsonDocument parse_doucment = QJsonDocument::fromJson(QByteArray(head), &json_error);
   if (json_error.error == QJsonParseError::NoError) {
      if (parse_doucment.isArray()) {
         QJsonArray array = parse_doucment.array();
         int size = array.size();
         if (size == 2) {
            QJsonValue keyData = array.at(0);
            QJsonValue valueData = array.at(1);
            if (keyData.isString() && valueData.isString()) {
               key = keyData.toString();
               value = valueData.toString();
            }
            else if (keyData.isString() && valueData.isObject()) {
               key = keyData.toString();
               value = QJsonDocument(valueData.toObject()).toJson();
            }
            else {
               paramSuc = false;
            }
         }
         else {
            paramSuc = false;
         }
      }
   }
   return true;
}

InteractionAliveClient::InteractionAliveClient()
   : m_hHttpEvent(NULL)
   , m_pDelegate(NULL) {
   CCriticalAutoLock loLock(g_oEventAliveInteraction);
   g_AliveClient = this;
   InitAliveDebugTrace(L"InteractionAliveClient", 6, false);
}

InteractionAliveClient::~InteractionAliveClient() {
   if (g_hAliveSioClosedEvent) {
      CloseHandle(g_hAliveSioClosedEvent);
      g_hAliveSioClosedEvent = NULL;
   }
   if (g_hAliveHttpTaskClosedEvent) {
      CloseHandle(g_hAliveHttpTaskClosedEvent);
      g_hAliveHttpTaskClosedEvent = NULL;
   }
   CCriticalAutoLock loLock(g_oEventAliveInteraction);
   if (g_AliveClient) {
      g_AliveClient = NULL;
   }
}

void InteractionAliveClient::RegisterDelegate(Delegate* pDelegate) {
   m_pDelegate = pDelegate;
}

BOOL InteractionAliveClient::Init(Delegate* pDelegate, Key_Data& oKeyData) {
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
      m_bKeepLiveHeartBeatRun = true;
      if (!InitHttp()) {
         break;
      }

      if (!InitSocketIO()) {
         break;
      }

      if (!InitKeepLiveThread()) {
         break;
      }

      TRACE6("%s ok end\n", __FUNCTION__);
      return TRUE;
   } while (FALSE);
   m_bHttpLoopRun = false;
   m_bKeepLiveHeartBeatRun = false;
   TRACE6("%s err end\n", __FUNCTION__);
   return FALSE;
}

BOOL InteractionAliveClient::InitHttp() {
   TRACE6("%s start\n", __FUNCTION__);
   if (g_hAliveHttpTaskClosedEvent == NULL) {
      g_hAliveHttpTaskClosedEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
   }
   CBaseThread loHttpThread;
   loHttpThread.BeginThread(InteractionAliveClient::HttpReqThread, this);
   TRACE6("%s end\n", __FUNCTION__);
   return TRUE;
}

void InteractionAliveClient::UnInitHttp() {
   if (g_hAliveHttpTaskClosedEvent != NULL) {
      WaitForSingleObject(g_hAliveHttpTaskClosedEvent, 60000);
      CloseHandle(g_hAliveHttpTaskClosedEvent);
      g_hAliveHttpTaskClosedEvent = NULL;
   }
}

BOOL InteractionAliveClient::InitSocketIO() {
   //SIO�ر�֪ͨ�¼�
   if (g_hAliveSioClosedEvent == NULL) {
      g_hAliveSioClosedEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
   }
   TRACE6("%s start\n", __FUNCTION__);
   CBaseThread loSocketIOThread;
   loSocketIOThread.BeginThread(InteractionAliveClient::SocketIOReqThread, this);
   TRACE6("%s end\n", __FUNCTION__);
   return TRUE;
}

void InteractionAliveClient::UnInitSocketIO() {
   TRACE6("%s start\n", __FUNCTION__);
   if (g_hAliveSioClosedEvent) {
      WaitForSingleObject(g_hAliveSioClosedEvent, 60000);
      CloseHandle(g_hAliveSioClosedEvent);
      g_hAliveSioClosedEvent = NULL;
   }
   TRACE6("%s end\n", __FUNCTION__);
}

BOOL InteractionAliveClient::InitKeepLiveThread() {
   if (g_hKeepLiveClosedEvent == NULL) {
      g_hKeepLiveClosedEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
   }
   if (g_hKeepLiveWaitEvent == NULL) {
      g_hKeepLiveWaitEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
   }
   TRACE6("%s start\n", __FUNCTION__);
   CBaseThread loSocketIOThread;
   loSocketIOThread.BeginThread(InteractionAliveClient::KeepLiveThread, this);
   TRACE6("%s end\n", __FUNCTION__);
   return TRUE;
}
void InteractionAliveClient::UnInitKeepLiveThread() {
   TRACE6("%s start\n", __FUNCTION__);
   if (g_hKeepLiveWaitEvent) {
      ::SetEvent(g_hKeepLiveWaitEvent);
   }
   if (g_hKeepLiveClosedEvent) {
      WaitForSingleObject(g_hKeepLiveClosedEvent, 60000);
      CloseHandle(g_hKeepLiveClosedEvent);
      g_hKeepLiveClosedEvent = NULL;
   }
   TRACE6("%s end\n", __FUNCTION__);
}

static const char HEX[] = "0123456789abcdef";

char hex_encode(unsigned char val) {
   return (val < 16) ? HEX[val] : '!';
}
const unsigned char URL_UNSAFE = 0x1; // 0-33 "#$%&+,/:;<=>?@[\]^`{|} 127
const unsigned char ASCII_CLASS[128] = {
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,0,3,1,1,1,3,2,0,0,0,1,1,0,0,1,0,0,0,0,0,0,0,0,0,0,1,1,3,1,3,1,
    1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,
    1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,1,
};

bool hex_decode(char ch, unsigned char* val) {
   if ((ch >= '0') && (ch <= '9')) {
      *val = ch - '0';
   }
   else if ((ch >= 'A') && (ch <= 'Z')) {
      *val = (ch - 'A') + 10;
   }
   else if ((ch >= 'a') && (ch <= 'z')) {
      *val = (ch - 'a') + 10;
   }
   else {
      return false;
   }
   return true;
}

size_t url_encode(char * buffer, size_t buflen,
   const char * source, size_t srclen) {
   if (NULL == buffer)
      return srclen * 3 + 1;
   if (buflen <= 0)
      return 0;

   size_t srcpos = 0, bufpos = 0;
   while ((srcpos < srclen) && (bufpos + 1 < buflen)) {
      unsigned char ch = source[srcpos++];
      if ((ch < 128) && (ASCII_CLASS[ch] & URL_UNSAFE)) {
         if (bufpos + 3 >= buflen) {
            break;
         }
         buffer[bufpos + 0] = '%';
         buffer[bufpos + 1] = hex_encode((ch >> 4) & 0xF);
         buffer[bufpos + 2] = hex_encode((ch) & 0xF);
         bufpos += 3;
      }
      else {
         buffer[bufpos++] = ch;
      }
   }
   buffer[bufpos] = '\0';
   return bufpos;
}


size_t url_decode(char * buffer, size_t buflen,
   const char * source, size_t srclen) {
   if (NULL == buffer)
      return srclen + 1;
   if (buflen <= 0)
      return 0;

   unsigned char h1, h2;
   size_t srcpos = 0, bufpos = 0;
   while ((srcpos < srclen) && (bufpos + 1 < buflen)) {
      unsigned char ch = source[srcpos++];
      if (ch == '+') {
         buffer[bufpos++] = ' ';
      }
      else if ((ch == '%')
         && (srcpos + 1 < srclen)
         && hex_decode(source[srcpos], &h1)
         && hex_decode(source[srcpos + 1], &h2))
      {
         buffer[bufpos++] = (h1 << 4) | h2;
         srcpos += 2;
      }
      else {
         buffer[bufpos++] = ch;
      }
   }
   buffer[bufpos] = '\0';
   return bufpos;
}


void InteractionAliveClient::OnHttpMsg(const QString& cmd, const QString& value) {
   QJsonObject obj;
   QJsonDocument flashdoc;
   QJsonDocument doc;
   QString minString = value;
   obj["key"] = cmd;
   obj["value"] = value;
   flashdoc.setObject(obj);
   doc.setObject(obj);
   doc = QJsonDocument::fromJson(value.toUtf8());
   obj = doc.object();

   Event oEvent;
   oEvent.m_eventType = e_eventType_msg;
   QString type = CPathManager::GetObjStrValue(obj, "type");
   if (cmd == "flashMsg") {
      if (value.length() < 1024) {
         TRACE6("%s flashMsg: %s\n", __FUNCTION__, value.toStdString().c_str());
      }
      QString msgSocketIO;
      msgSocketIO = flashdoc.toJson(QJsonDocument::Compact);
      SendAliveSocketIOMsg(msgSocketIO);
      return;
   }
   else if (cmd == "msg") {
      if (doc.isObject()) {
         QJsonObject msgObj = doc.object();
         if (msgObj.contains("data")) {
            QString data_obj = msgObj["data"].toString();
            QJsonDocument data_obj_json = QJsonDocument::fromJson(data_obj.toUtf8());
            if (data_obj_json.isObject()){
               QJsonObject data_type_obj = data_obj_json.object();
               if (data_type_obj.contains("type") && data_type_obj["type"].toString() == "room_announcement") {
                  if (data_type_obj.contains("room_announcement_text")) {
                     QString room_announcement_text = data_type_obj["room_announcement_text"].toString();
                     oEvent.m_eMsgType = e_RQ_ReleaseAnnouncement;
                     oEvent.m_eventType = e_eventType_msg;
                     oEvent.context = room_announcement_text;
                  }
               }
            }
            if (msgObj.contains("date_time")) {
               oEvent.pushTime = msgObj["date_time"].toString();
            }
            ExecuteCallBackTask(oEvent);
         }
      }
   }
   else if (cmd == "cmd") {
      QString user_id = CPathManager::GetObjStrValue(obj, "user_id");
      QString user_name = CPathManager::GetObjStrValue(obj, "nick_name");
      QString role_name = CPathManager::GetObjStrValue(obj, "role_name");
      QString headImage = CPathManager::GetObjStrValue(obj, "avatar");
      oEvent.m_oUserInfo.userName = user_name.toStdWString();
      oEvent.m_oUserInfo.role = role_name.toStdWString();
      oEvent.m_oUserInfo.headImage = headImage.toStdWString();        
      oEvent.m_oUserInfo.userId = user_id.toStdWString();
      if (type == "*kickout") {
         CreateKickout(oEvent, obj);
      }
      else if (type == "*kickoutrestore") {
         CreatKickoutrestore(oEvent, obj);
      }
      else if (type == "*disablechat") {
         CreatDisablechat(oEvent, obj);
      }
      else if (type == "*permitchat") {
         CreatPermitchat(oEvent, obj);
      }
      else if (type == "*forbidchat") {
         CreatForbidchat(oEvent, obj, value);
         return;
      }
      else if ("*whiteBoard" == type || "doc_convert_jpeg" == type || "doc_convert" == type)
      {
         CreatWhiteBoard(/*oEvent, obj, */value);
         return;
      }
      else if (type == "*switchHandsup")//���ֿ���
      {
         CreatSwitchHandsup(oEvent, obj);
      }
      else if (type == "*handsUp"){ //����
         CreatHandsUp(oEvent, obj);
         QString is_gag = CPathManager::GetObjStrValue(obj, "is_gag");//obj["status"].toString();
         if (is_gag == "1") {
            return;
         }
      }
      else if (type == "*setDefinition") {//�޸ķֱ���
         CreatSetDefinition(oEvent, obj);
      }
      else if (type == "*replyInvite")//�ظ�����
      {
         CreateReplyInvite(oEvent, obj);
      }
      else if (type == "*switchDevice")//�豸����
      {
         CreateSwitchDevice(oEvent, obj);
      }
      else if (type == "*notSpeak")//����
      {
         CreateNotSpeak(oEvent, obj);
      }
      else if (type == "*sendInvite")// ��������
      {
         CreateSendInvite(oEvent, obj);
      }
      else if (type == "*addSpeaker")//���� / ������ͨ��
      {
         CreateAddSpeaker(oEvent, obj);
      }
      else if (type == "*agreeSpeak") {//������ͬ������
         CreateAgreeSpeaker(oEvent, obj);
      }
      else if (type == "*setMainShow")//����������
      {
         CreateSetMainShow(oEvent, obj);
      }
      else if (type == "*setMainSpeaker")//����������
      {
         CreateSetMainSpeaker(oEvent, obj);
      }
      else if (type == "*publishStart" || value.contains("*publishStart"))//
      {
         CreatePublishStart(oEvent, obj);
      }
      else if (type == "*over")//����ֱ��
      {
         CreateOver(oEvent, obj);
      }
      else if ("*rejectSpeak" == type)//�����˾ܾ�
      {
         CreateRejectSpeak(oEvent, obj);
      }
      ExecuteCallBackTask(oEvent);
   }
}

void InteractionAliveClient::UnInit() {
   {
      CCriticalAutoLock oAutoLock(m_oSafeLock);
      m_pDelegate = NULL;
   }
   m_bKeepLiveHeartBeatRun = false;
   m_httpKeepLive.ExitLoop();
   UnInitKeepLiveThread();
   m_bHttpLoopRun = false;
   m_httpSocketIOProxy.ExitLoop();
   m_httpTaskProxy.ExitLoop();
   UnInitHttp();
   UnInitSocketIO();
   TRACE6("%s exit\n", __FUNCTION__);
}


void InteractionAliveClient::ConnectServer() {
   CCriticalAutoLock oAutoLock(m_oHttpProxyInfoLock);
   memcpy(m_httpProxyInfo.ip, m_oKeyData.m_proxy_ip, sizeof(m_oKeyData.m_proxy_ip));
   memcpy(m_httpProxyInfo.user, m_oKeyData.m_proxy_username, sizeof(m_oKeyData.m_proxy_username));
   memcpy(m_httpProxyInfo.pwd, m_oKeyData.m_proxy_password, sizeof(m_oKeyData.m_proxy_password));
   m_httpProxyInfo.port = QString(m_oKeyData.m_proxy_port).toInt();
}

void InteractionAliveClient::DisConnect() {

}

void InteractionAliveClient::MessageRQ(const MsgRQType& eMsgType, RQData* vData /*= NULL*/) {
   //HTTP
   WCHAR wzRQUrl[2048] = { 0 };
   switch (eMsgType) {
   case e_RQ_UserOnlineList: {//�����б�   
      TRACE6("%s e_RQ_UserOnlineList\n", __FUNCTION__);
      if (vData) {
         vData->m_iCurPage = vData->m_iCurPage <= 0 ? 1 : vData->m_iCurPage;
         _snwprintf_s(wzRQUrl, 2048, L"%s%s?token=%s&curr_page=%d", m_oKeyData.m_MSG_SERVER_URL, L"onlineusers", m_oKeyData.m_szMsgToken, vData->m_iCurPage);
         break;
      }
   }                 
   case e_RQ_UserProhibitSpeakList: {//�����б�
      TRACE6("%s e_RQ_UserProhibitSpeakList\n", __FUNCTION__);
      _snwprintf_s(wzRQUrl, 2048, L"%s%s?token=%s", m_oKeyData.m_MSG_SERVER_URL, L"silencedusers", m_oKeyData.m_szMsgToken);
      break;
   }                       
   case e_RQ_UserKickOutList: {//�߳��б�
      TRACE6("%s e_RQ_UserProhibitSpeakList\n", __FUNCTION__);
      _snwprintf_s(wzRQUrl, 2048, L"%s%s?token=%s", m_oKeyData.m_MSG_SERVER_URL, L"kickedusers", m_oKeyData.m_szMsgToken);
      break;
   }
   case e_RQ_UserSpecialList: {//�����ˡ��α�������
      TRACE6("%s e_RQ_UserProhibitSpeakList\n", __FUNCTION__);
      _snwprintf_s(wzRQUrl, 2048, L"%s%s?token=%s", m_oKeyData.m_MSG_SERVER_URL, L"specialusers", m_oKeyData.m_szMsgToken);
      break;
   }
   case e_RQ_UserProhibitSpeakOneUser: {
      if (vData) {
         _snwprintf_s(wzRQUrl, 2048, L"%s%s?token=%s&user_id=%s&type=1", m_oKeyData.m_MSG_SERVER_URL, L"silence", m_oKeyData.m_szMsgToken, vData->m_oUserInfo.userId.c_str());
      }
      break;
   }                                   
   case e_RQ_UserAllowSpeakOneUser: {
      if (vData) {
         _snwprintf_s(wzRQUrl, 2048, L"%s%s?token=%s&user_id=%s&type=2", m_oKeyData.m_MSG_SERVER_URL, L"silence", m_oKeyData.m_szMsgToken, vData->m_oUserInfo.userId.c_str());
      }
      break;
   }                            
   case e_RQ_UserKickOutOneUser: {
      if (vData) {
         _snwprintf_s(wzRQUrl, 2048, L"%s%s?token=%s&user_id=%s", m_oKeyData.m_MSG_SERVER_URL, L"kick", m_oKeyData.m_szMsgToken, vData->m_oUserInfo.userId.c_str());
      }
      break;
   }                   
   case e_RQ_UserAllowJoinOneUser: {
      if (vData) {
         _snwprintf_s(wzRQUrl, 2048, L"%s%s?token=%s&user_id=%s", m_oKeyData.m_MSG_SERVER_URL, L"restorekicked", m_oKeyData.m_szMsgToken, vData->m_oUserInfo.userId.c_str());
      }
      break;
   }
   case e_RQ_ReleaseAnnouncement: {
      QString url = QString(m_oKeyData.m_domain) + "/api/webinar/v1/webinar/sendmsg?token=" + QString::fromWCharArray(m_oKeyData.m_szMsgToken) + "&event=cmd&msg=" + QString(vData->m_wzText);
      url.toWCharArray(wzRQUrl);
      break;
   }
                                 
   default:
      break;
   }

   HttpReq oHttpReq;
   oHttpReq.m_eType = eMsgType;
   wcscpy(oHttpReq.m_wzRequestUrl, wzRQUrl);
   AddHttpTask(oHttpReq);
}

unsigned int InteractionAliveClient::HttpReqThread(void * apParam) {
   InteractionAliveClient* pThis = static_cast<InteractionAliveClient*>(apParam);
   TRACE6("%s start\n", __FUNCTION__);
   if (pThis) {
      pThis->HttpReqWork();
   }
   if (g_hAliveHttpTaskClosedEvent) {
      ::SetEvent(g_hAliveHttpTaskClosedEvent);
   }
   TRACE6("%s end\n", __FUNCTION__);
   return 0;
}

unsigned int InteractionAliveClient::HttpReqWork() {
   //�����¼����(�Զ�����)
   TRACE6("%s  start\n", __FUNCTION__);
   while (m_bHttpLoopRun) {
      //ȡ��һ������
      HttpReq oReq;
      if (GetHttpTask(oReq)) {
         //ִ������
         ExecuteHttpTask(oReq);
      }
      Sleep(100);
   }
   TRACE6("%s  end\n", __FUNCTION__);
   return 0;
}

void InteractionAliveClient::AddHttpTask(HttpReq& aoReq) {
   CCriticalAutoLock loLock(m_oHttpTaskLock);
   //��ӵ��������
   m_oHttpList.push_back(aoReq);
}

BOOL InteractionAliveClient::GetHttpTask(HttpReq& aoReq) {
   CCriticalAutoLock loLock(m_oHttpTaskLock);
   if (m_oHttpList.size() < 1) {
      return FALSE;
   }

   aoReq = m_oHttpList.front();
   m_oHttpList.pop_front();
   return TRUE;
}

BOOL InteractionAliveClient::ExecuteHttpTask(HttpReq& oTask) {//�����û��б���Ϣ��ȡ
   TRACE6("%s -------- start\n", __FUNCTION__);
   QString taskUrl = QString::fromWCharArray(oTask.m_wzRequestUrl);
   {
      CCriticalAutoLock oAutoLock(m_oHttpProxyInfoLock);
      if (strlen(m_httpProxyInfo.ip) == 0) {
         m_httpTaskProxy.SetHttpProxyInfo(false);
      }
      else {
         m_httpTaskProxy.SetHttpProxyInfo(true, m_httpProxyInfo.ip, m_httpProxyInfo.port, m_httpProxyInfo.user, m_httpProxyInfo.pwd);
      }
   }
   QByteArray dataArray;
   int errCode = -1;
   if (m_httpTaskProxy.VHGetRet2(taskUrl, DEF_HTTPLOOP_MAX_TIMEOUT, dataArray, errCode)) {
      if (!dataArray.isEmpty()) {

         qDebug() << taskUrl;
         //			add_log("xk", "%s REQ:%s", __FUNCTION__, taskUrl.toStdString().c_str());
         QString qStrResult = QString::fromStdString(dataArray.toStdString());
         qDebug() << qStrResult;
         //			add_log("xk", "%s RES:%s", __FUNCTION__, qStrResult.toStdString().c_str());
         Event oReq;
         oReq.Init();
         ParseJSon(oTask.m_eType, qStrResult, oReq);
         //�����������
         ExecuteCallBackTask(oReq);
      }

   }
   TRACE6("%s -------- end\n", __FUNCTION__);
   return TRUE;
}

unsigned int InteractionAliveClient::KeepLiveThread(void *Param) {
   InteractionAliveClient* pThis = static_cast<InteractionAliveClient*>(Param);
   TRACE6("%s  start\n", __FUNCTION__);
   if (pThis) {
      pThis->KeepLiveThreadWork();
      if (g_hKeepLiveClosedEvent) {
         ::SetEvent(g_hKeepLiveClosedEvent);
      }
   }
   TRACE6("%s  end\n", __FUNCTION__);
   return 0;
}
unsigned int InteractionAliveClient::KeepLiveThreadWork() {
   QString cookies;
   QString requestUrl;
   while (m_bKeepLiveHeartBeatRun) {
      QByteArray data;
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
      if (!cookies.isEmpty()) {//�ǿ� ÿ��10s����һ������
         m_httpKeepLive.VHPostRet(requestUrl, QString("1:2"), cookies);
      }
      if (g_hKeepLiveWaitEvent) {
         WaitForSingleObject(g_hKeepLiveWaitEvent, 10000);
      }
   }
   if (!cookies.isEmpty()) {
      m_httpKeepLive.VHPostRet(requestUrl, QString("2:41"), cookies); //�Ͽ�����
      m_httpKeepLive.VHPostRet(requestUrl, QString("1:1"), cookies);
   }
   return true;
}

unsigned int InteractionAliveClient::SocketIOReqThread(void * apParam) {
   InteractionAliveClient* pThis = static_cast<InteractionAliveClient*>(apParam);
   TRACE6("%s  start\n", __FUNCTION__);
   if (pThis) {
      pThis->SocketIOHttpReqWork();
      if (g_hAliveSioClosedEvent) {
         ::SetEvent(g_hAliveSioClosedEvent);
      }
   }
   TRACE6("%s  end\n", __FUNCTION__);
   return 0;
}

unsigned int InteractionAliveClient::SocketIOHttpReqWork() {
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
      QString togetUrl = url.arg(msg_URL).arg(qtoken).arg(curT).arg(index++) + sid; //sidΪ��ʱ  ����tempId����ֵ��ͽ���
      QUrl qUrl = QUrl(togetUrl);
      qUrl.setScheme("http");
      QString requestUrl = qUrl.toString();
      {
         CCriticalAutoLock oAutoLock(m_oHttpProxyInfoLock);
         if (strlen(m_httpProxyInfo.ip) == 0) {
            m_httpSocketIOProxy.SetHttpProxyInfo(false);
         }
         else {
            m_httpSocketIOProxy.SetHttpProxyInfo(true, m_httpProxyInfo.ip, m_httpProxyInfo.port, m_httpProxyInfo.user, m_httpProxyInfo.pwd);
         }
      }
      QString cookis = "io=" + cookio;
      QByteArray dataArray;
      if (!sid.isEmpty()) {//sid�ǿ�
         QMutexLocker l(&mKeepLiveMutex);
         mURL = msg_URL;
         mToken = qtoken;
         mIndex = index;
         mCookio = cookio;
      }
      int errCode = -1;
      TRACE6("%s ���� = %s\n", __FUNCTION__, requestUrl.toStdString().c_str());
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
            }
            else if (head[i] == '[') {
               json = false;
               ptr = (char *)head + i;
               break;
            }
         }

         if (json) {//�յ���ʼ��tempsid
            QJsonObject obj = VHAliveSocketIOGetJson(ptr);
            qDebug() << "json:" << obj;
            if (sid == "") {
               QString tmpsid = obj["sid"].toString();
               if (tmpsid != "") {
                  sid = "&sid=" + tmpsid;//�յ�tempsid��� ��ʼ��sid  
                  cookio = tmpsid;//��ʼ�� cookio �Դ˷�������
                  TRACE6("%s cookio = %s\n", __FUNCTION__, cookio.toStdString().c_str());
                  Event oEvent;
                  oEvent.m_eventType = e_eventType_opened;
                  ExecuteCallBackTask(oEvent);//��sidΪ��ʱ����һ�γ�Ա�б�����
               }
            }

            int code = obj["code"].toInt();
            if (code == 1) {//�յ�����ֵ code = 1,���tempsid ��������
               sid = "";
               cookio = "";
               index = 0;
               TRACE6("%s code = 1\n", __FUNCTION__);
            }
         }
         else {//�յ����������Ϣ�����ԡ��߳������桢flash�ȣ�
            QString key, value;
            if (VHAliveSocketIOGetKV(ptr, key, value)) {
               OnHttpMsg(key, value);
            }
         }
      }
      //������ٱ���˵�������쳣�����½�������
      TRACE6("%s current socketio http request errCode:%d cookis:%s\n", __FUNCTION__, errCode, mCookio.toStdString().c_str());
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
   TRACE6("%s exit thread \n", __FUNCTION__);
   return 0;
}

BOOL InteractionAliveClient::ExecuteCallBackTask(Event& oTask) {
   CCriticalAutoLock oAutoLock(m_oSafeLock);
   if (m_pDelegate && oTask.m_eMsgType != e_RQ_None) {
      oTask.m_lRoomID = m_oKeyData.m_lRoomID;
      if (oTask.m_eventType == e_eventType_opened) {
         m_pDelegate->onConnected(&oTask);// ֪ͨ UI���У����ߡ����ԡ��߳��������û��������֡��α����������������
      }
      else if (oTask.m_eventType == e_eventType_msg) {
         m_pDelegate->onMessaged(&oTask);//�����յ��� ���졢���顢���桢���ˡ����ԡ������û��б���������Ϣ
      }
   }
   return TRUE;
}

void InteractionAliveClient::CreateAnnouncement(Event& oEvent, const QJsonObject& obj, QString& msgSocketIO)
{
   SendAliveSocketIOMsg(msgSocketIO);
   //oEvent.m_eMsgType = e_RQ_ReleaseAnnouncement;
   //QString conntect = CPathManager::GetObjStrValue(obj, "content"); //obj["content"].toString();
   //strcpy(oEvent.m_wzText, conntect.toStdString().c_str());
   //oEvent.m_eventType = e_eventType_msg;
}

void InteractionAliveClient::CreateKickout(Event& oEvent, const QJsonObject& obj)
{
   QString user_id = CPathManager::GetObjStrValue(obj, "user_id");// obj["user_id"].toString();
   if (user_id != ""){
      oEvent.m_eMsgType = e_RQ_UserKickOutOneUser;
      oEvent.m_oUserInfo.userId = user_id.toStdWString();
   }
}

void InteractionAliveClient::CreatKickoutrestore(Event& oEvent, const QJsonObject& obj)
{
   QString user_id = CPathManager::GetObjStrValue(obj, "user_id");//obj["user_id"].toString();
   if (user_id != ""){
      oEvent.m_eMsgType = e_RQ_UserAllowJoinOneUser;
      oEvent.m_oUserInfo.userId = user_id.toStdWString();
   }
}

void InteractionAliveClient::CreatDisablechat(Event& oEvent, const QJsonObject& obj)
{
   QString user_id = CPathManager::GetObjStrValue(obj, "user_id");//obj["user_id"].toString();
   if (user_id != ""){
      oEvent.m_eMsgType = e_RQ_UserProhibitSpeakOneUser;
      oEvent.m_oUserInfo.userId = user_id.toStdWString();
   }
}

void InteractionAliveClient::CreatPermitchat(Event& oEvent, const QJsonObject& obj){
   QString user_id = CPathManager::GetObjStrValue(obj, "user_id");
   if (user_id != ""){
      oEvent.m_eMsgType = e_RQ_UserAllowSpeakOneUser;
      oEvent.m_oUserInfo.userId = user_id.toStdWString();
   }
}

void InteractionAliveClient::CreatForbidchat(Event& oEvent, const QJsonObject& obj, const QString& strValue)
{
   QJsonObject cmdObj;
   cmdObj["key"] = "cmd";
   cmdObj["value"] = strValue;

   QJsonDocument doc;
   doc.setObject(cmdObj);
   QString msgSocketIO;
   msgSocketIO = doc.toJson(QJsonDocument::Compact);
   SendAliveSocketIOMsg(msgSocketIO);

   int  iStatus = CPathManager::GetObjStrValue(obj, "status").toInt();
   if (1 == iStatus)
      oEvent.m_eMsgType = e_RQ_UserProhibitSpeakAllUser;
   else
      oEvent.m_eMsgType = e_RQ_UserAllowSpeakAllUser;
}

void InteractionAliveClient::CreatWhiteBoard(/*Event& oEvent, const QJsonObject& obj,*/ const QString& strValue)
{
   QJsonObject cmdObj;
   cmdObj["key"] = "cmd";
   cmdObj["value"] = strValue;

   QJsonDocument doc;
   doc.setObject(cmdObj);
   QString  strSocketIo = doc.toJson(QJsonDocument::Compact);
   SendAliveSocketIOMsg(strSocketIo);
}

void InteractionAliveClient::CreatSwitchHandsup(Event& oEvent, const QJsonObject& obj)
{
   QString status = CPathManager::GetObjStrValue(obj, "status");//obj["status"].toString();
   if (status == "0") {
      oEvent.m_eMsgType = e_RQ_switchHandsup_close;
   }
   else if (status == "1") {
      oEvent.m_eMsgType = e_RQ_switchHandsup_open;
   }
   oEvent.m_eventType = e_eventType_msg;
}

void InteractionAliveClient::CreatHandsUp(Event& oEvent, const QJsonObject& obj)
{
   analysisUseInfo(oEvent, obj);
   QString status = CPathManager::GetObjStrValue(obj, "status");//obj["status"].toString();
   if (status == "0") {
      oEvent.m_eMsgType = e_RQ_CancelHandsUp;
   }
   else if (status == "1") {
      oEvent.m_eMsgType = e_RQ_handsUp;
   }
   oEvent.m_eventType = e_eventType_msg;
}

void InteractionAliveClient::CreatSetDefinition(Event& oEvent, const QJsonObject& obj) {
   analysisUseInfo(oEvent, obj);
   QString definition = CPathManager::GetObjStrValue(obj, "definition");
   memcpy(oEvent.m_wzText, definition.toStdString().c_str(), definition.toStdString().length());
   oEvent.m_eMsgType = e_RQ_setDefinition;
   oEvent.m_eventType = e_eventType_msg;
}

void InteractionAliveClient::CreateReplyInvite(Event& oEvent, const QJsonObject& obj)
{
   analysisUseInfo(oEvent, obj);
   QString status = CPathManager::GetObjStrValue(obj, "status");//obj["status"].toString();
   if (status == "1") {
      oEvent.m_eMsgType = e_RQ_replyInvite_Suc;
   }
   else if (status == "2") {
      oEvent.m_eMsgType = e_RQ_replyInvite_Fail;
   }
   else if (status == "3") {
      oEvent.m_eMsgType = e_RQ_replyInvite_Error;
   }

   oEvent.m_eventType = e_eventType_msg;
}

void InteractionAliveClient::CreateSwitchDevice(Event& oEvent, const QJsonObject& obj)
{
   analysisUseInfo(oEvent, obj);
   QString status = CPathManager::GetObjStrValue(obj, "status");//obj["status"].toString();
   if (status == "0") {
      oEvent.m_eMsgType = e_RQ_switchDevice_close;
   }
   else if (status == "1") {
      oEvent.m_eMsgType = e_RQ_switchDevice_open;
   }

   int iDevice = obj["device"].toInt();
   if (eJesinDevice_eDevice_Mic == iDevice)
   {
      oEvent.meDevice = eDevice_Mic;
   }
   else
   {
      oEvent.meDevice = eDevice_Camera;
   }

   oEvent.m_eventType = e_eventType_msg;
}

void InteractionAliveClient::CreateNotSpeak(Event& oEvent, const QJsonObject& obj)
{
   analysisUseInfo(oEvent, obj);
   oEvent.m_eMsgType = e_RQ_notSpeak;
   oEvent.m_eventType = e_eventType_msg;
}

void InteractionAliveClient::CreateSendInvite(Event& oEvent, const QJsonObject& obj)
{
   analysisUseInfo(oEvent, obj);
   oEvent.m_eMsgType = e_RQ_sendInvite;
   oEvent.m_eventType = e_eventType_msg;
}

void InteractionAliveClient::CreateAddSpeaker(Event& oEvent, const QJsonObject& obj)
{
   analysisUseInfo(oEvent, obj);
   oEvent.m_eMsgType = e_RQ_addSpeaker;
   oEvent.m_eventType = e_eventType_msg;
}

void InteractionAliveClient::CreateAgreeSpeaker(Event& oEvent, const QJsonObject& obj) {
   analysisUseInfo(oEvent, obj);
   oEvent.m_eMsgType = e_RQ_agreeSpeaker;
   oEvent.m_eventType = e_eventType_msg;
}

void InteractionAliveClient::CreateSetMainShow(Event& oEvent, const QJsonObject& obj)
{
   analysisUseInfo(oEvent, obj);
   oEvent.m_eMsgType = e_RQ_setMainShow;
   oEvent.m_eventType = e_eventType_msg;
}

void InteractionAliveClient::CreateSetMainSpeaker(Event& oEvent, const QJsonObject& obj)
{
   analysisUseInfo(oEvent, obj);
   oEvent.m_eMsgType = e_RQ_setMainSpeaker;
   oEvent.m_eventType = e_eventType_msg;
}

void InteractionAliveClient::CreatePublishStart(Event& oEvent, const QJsonObject& obj)
{
   analysisUseInfo(oEvent, obj);
   oEvent.m_eMsgType = e_RQ_setPublishStart;
   oEvent.m_eventType = e_eventType_msg;
}

void InteractionAliveClient::CreateOver(Event& oEvent, const QJsonObject& obj)
{
   analysisUseInfo(oEvent, obj);
   oEvent.m_eMsgType = e_RQ_setOver;
   oEvent.m_eventType = e_eventType_msg;
}

//void InteractionAliveClient::CreateAddSpeak(Event& oEvent, const QJsonObject& obj)
//{
//	analysisUseInfo(oEvent, obj);
//	oEvent.m_eMsgType = e_RQ_AddSpeaker;
//	oEvent.m_eventType = e_eventType_msg;
//}

void InteractionAliveClient::CreateRejectSpeak(Event& oEvent, const QJsonObject& obj)
{
   analysisUseInfo(oEvent, obj);
   oEvent.m_eMsgType = e_RQ_RejectSpeak;
   oEvent.m_eventType = e_eventType_msg;
}

//void InteractionAliveClient::CreateWhiteBoard(Event& oEvent, const QJsonObject& obj)
//{
//	analysisUseInfo(oEvent, obj);
//	QString status = CPathManager::GetObjStrValue(obj, "status");
//	strcpy(oEvent.m_wzText, status.toLocal8Bit.data());
//	oEvent.m_eMsgType = e_RQ_WhiteBoard_open;
//	oEvent.m_eventType = e_eventType_msg;
//}

void InteractionAliveClient::analysisUseInfo(Event& oEvent, const QJsonObject& obj)
{
   QString strJoinUid = CPathManager::GetObjStrValue(obj, "join_uid");//obj["join_uid"].toInt();
   QString strNickName = CPathManager::GetObjStrValue(obj, "nick_name");//obj["nick_name"].toInt();
   QString strRoleName = CPathManager::GetObjStrValue(obj, "role_name");//obj["role_name"].toInt();

   int iAudioState = obj["audio"].toInt();
   int iVideoState = obj["video"].toInt();

   oEvent.m_bAudioState = (1 == iAudioState);
   oEvent.m_bVideoState = (1 == iVideoState);
   oEvent.m_oUserInfo.userId = strJoinUid.toStdWString();
   oEvent.m_oUserInfo.role = strRoleName.toStdWString();
   oEvent.m_oUserInfo.userName = strNickName.toStdWString();
}

void InteractionAliveClient::ParseJSon(const MsgRQType& type, const QString& qsJson, Event& oEvent) {
   oEvent.m_eventType = e_eventType_msg;
   QJsonObject ObjValue = CPathManager::GetJsonObjectFromString(qsJson);

   switch (type) {
   case e_RQ_UserOnlineList:				 //�����б�
   case e_RQ_UserProhibitSpeakList:  //�����û��б�
   case e_RQ_UserKickOutList:			//�߳��б�
   case e_RQ_UserSpecialList: {         //�����б�
      QString code = ObjValue.value("code").toString();
      QString msg = ObjValue.value("msg").toString();
      if ("200" == code && "success" == msg) {
         if (ObjValue.value("data").isObject())
         {
            QJsonObject ObjData = ObjValue.value("data").toObject();
            if (e_RQ_UserOnlineList == type) {
               int pageCount = ObjData.value("total_page").toInt();
               oEvent.m_iPageCount = pageCount;
               int currentPage = CPathManager::GetObjStrValue(ObjData, "curr_page").toInt();// ObjData.value("curr_page").toInt();
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

                  VhallAudienceUserInfo oUserInfo;
                  oUserInfo.userName = nick_name.toStdWString();
                  oUserInfo.userId = user_id.toStdWString();
                  oUserInfo.role = role.toStdWString();
                  oEvent.m_oUserList.push_back(oUserInfo);
               }
            }
            else
            {
               TRACE6("%s : ObjData.value(users) is NOT Array\n", __FUNCTION__);
            }
         }
         else
         {
            TRACE6("%s : ObjValue.value(data).is NOT Object() \n", __FUNCTION__);
         }
      }
      else if ("500" == code) {
         if (e_RQ_UserOnlineList == type)
         {
            QJsonObject ObjData = ObjValue.value("data").toObject();
            int sumNumber = ObjData.value("count").toInt();
            oEvent.m_sumNumber = sumNumber;
         }
      }
      else {
      }
      oEvent.m_eMsgType = type;
   }
                              break;
   case e_RQ_ChatEmotionList: { //�����б�
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
            TRACE6("%s : ObjData.value(users) is NOT Arrayn", __FUNCTION__);
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
//socketIO �ص�
void InteractionAliveClient::SetMessageSocketIOCallBack(InteractionMsgCallBack callback) {
   m_socketIOMutex.lock();
   m_socketIOCallback = callback;
   m_socketIOMutex.unlock();
}

//websocket �ص�
void InteractionAliveClient::SetMessageWebSocketCallBack(InteractionMsgCallBack callback) {

}

void InteractionAliveClient::PubSocketIOSendMsg(QString &msg) {
   TRACE6("%s m_socketIOMutex.lock\n", __FUNCTION__);
   m_socketIOMutex.lock();
   if (m_socketIOCallback) {
      TRACE6("%s : m_socketIOCallback is not null\n", __FUNCTION__);
      m_socketIOCallback(msg.toUtf8().data());
   }
   m_socketIOMutex.unlock();
   TRACE6("%s : m_socketIOMutex.unlock\n", __FUNCTION__);
}

void InteractionAliveClient::SetProxyAddr(const bool& enable, const char *proxy_ip, const char* proxy_port, const char *proxy_username, const char *proxy_password) {
   //HTTP Proxy User HTTP pulling.
   CCriticalAutoLock oAutoLock(m_oHttpProxyInfoLock);
   if (enable) {
      memcpy(m_httpProxyInfo.ip, proxy_ip, sizeof(m_httpProxyInfo.ip));
      memcpy(m_httpProxyInfo.user, proxy_username, sizeof(m_httpProxyInfo.user));
      memcpy(m_httpProxyInfo.pwd, proxy_password, sizeof(m_httpProxyInfo.pwd));
      m_httpProxyInfo.port = QString(proxy_port).toInt();
      TRACE6("%s use proxy http pulling\n", __FUNCTION__);
   }
   else {
      memset(&m_httpProxyInfo, 0, sizeof(m_httpProxyInfo));
   }
}
