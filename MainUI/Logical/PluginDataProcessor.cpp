#include <QJsonDocument>
#include "PluginDataProcessor.h"
#include "DebugTrace.h"

#include "IOBSControlLogic.h"
#include "Msg_OBSControl.h"
#include "CRPluginDef.h"
#include "VH_ConstDeff.h"
#include "MainUIIns.h"

CPluginDataProcessor::CPluginDataProcessor(QObject *parent)
: QObject(parent) {
   m_pPushTimer = new QTimer();
   if (m_pPushTimer) {
      connect(m_pPushTimer,SIGNAL(timeout()),this,SLOT(Slot_PushStream()));
   }
}

CPluginDataProcessor::~CPluginDataProcessor() {
   if (m_pPushTimer){
      disconnect(m_pPushTimer, SIGNAL(timeout()), this, SLOT(Slot_PushStream()));
      m_pPushTimer->stop();
   }
   if (m_pPushTimer) {
      delete m_pPushTimer;
      m_pPushTimer = NULL;
   }
   TRACE6("%s delete m_pPushTimer end\n", __FUNCTION__);
}

int CPluginDataProcessor::OnRecvHttpData(const char* data, int dataLen) {
	if (!GetEnableRecvMsg()) {
		return 0;
	}
   QMutexLocker lock(&m_mutexStream);
   QString page_id = 0;
   if (isDocPluginData(data, dataLen, page_id)) {
      QString qmsg(data);
      qmsg = qmsg.replace("param=", "");
      qmsg = qmsg.replace("&type=doc", "");
      if (m_bHasPushStream) {
         PostPluginData(qmsg.toStdString().c_str(), qmsg.length());
      } else {
         InsertDocData(page_id, qmsg.toStdString().c_str(), qmsg.length());
      }
   } 
   else if (isWBPluginData(data, dataLen)) {
      QString qmsg(data);
      qmsg = qmsg.replace("param=", "");
      qmsg = qmsg.replace("&type=board", "");
      if (m_bHasPushStream) {
         PostPluginData(qmsg.toStdString().c_str(), qmsg.length());
      } else {
         InsertWBData(qmsg.toStdString().c_str(), qmsg.length());
      }
   }
   return 0;
}

void CPluginDataProcessor::SetEnableRecvMsg(bool enable) {
	QMutexLocker l(&m_enableMutex);
	mbIsEnableRecvMsg = enable;
}

bool CPluginDataProcessor::GetEnableRecvMsg() {
	QMutexLocker l(&m_enableMutex);
	return mbIsEnableRecvMsg;
}


void CPluginDataProcessor::SetPushStreamState(bool bStart) {
   TRACE6("%s\n", __FUNCTION__);
   QMutexLocker lock(&m_mutexStream);
   m_bHasPushStream = bStart;
   TRACE6("%s bstart %d\n", __FUNCTION__, bStart);
   if (m_bHasPushStream) {
      //如果设置了开始推流状态，那么发送数据队列遗留数据。
      //SendWBPluginData();
      //SendDocPluginData();
      m_pPushTimer->stop();
      m_pPushTimer->start(2000);
   }
   else {
      m_pPushTimer->stop();
   }
   TRACE6("%s end\n", __FUNCTION__);
}

void CPluginDataProcessor::PostPluginData(const char* data, int dataLen) {
   PLUGIN_DATA pluginData;
   pluginData.data = new char[dataLen + 1];
   memset(pluginData.data, 0, dataLen + 1);
   memcpy(pluginData.data, data, dataLen);
   pluginData.length = dataLen;
   SingletonMainUIIns::Instance().PostCRMessage(MSG_OBSCONTROL_PUSH_AMF0, &pluginData, sizeof(PLUGIN_DATA));
}

void CPluginDataProcessor::InsertDocData(QString page_id, const char* data, int dataLen) {
   if (m_nCurrentPage != page_id) {
      m_DocDataList.clear();
      m_nCurrentPage = page_id;
   }
   string* strData = new string(data, dataLen);
   if (strData == NULL) {
      TRACE6("%s new DocData data err\n", __FUNCTION__);
   }
   m_DocDataList.push_back(strData);
}

void CPluginDataProcessor::InsertWBData(const char* data, int dataLen) {
   string* wbData = new string(data, dataLen);
   if (wbData == NULL) {
      TRACE6("%s new wb data err\n",__FUNCTION__);
   }
   m_DocDataList.push_back(wbData);
}

void CPluginDataProcessor::SendWBPluginData() {
   TRACE6("%s msg count:%d\n", __FUNCTION__, m_WbDataList.size());
   QList<string*>::iterator iter = m_WbDataList.begin();
   while (iter != m_WbDataList.end()) {
      string data = **iter;
      PostPluginData(data.c_str(), data.length());
      delete *iter;
      iter++;
   }
   m_WbDataList.clear();
}

void CPluginDataProcessor::SendDocPluginData() {
   TRACE6("%s msg count:%d\n", __FUNCTION__, m_DocDataList.size());
   QList<string*>::iterator iter = m_DocDataList.begin();
   while (iter != m_DocDataList.end()) {
      string data = **iter;
      PostPluginData(data.c_str(), data.length());
      delete *iter;
      iter++;
   }
   m_DocDataList.clear();
}

void CPluginDataProcessor::ClearAllData() {
   TRACE6("%s\n", __FUNCTION__);
   QMutexLocker lock(&m_mutexStream);
   for (int i = 0; i < m_DocDataList.size(); i++) {
      delete m_DocDataList.at(i);
   }
   for (int i = 0; i < m_WbDataList.size(); i++) {
      delete m_WbDataList.at(i);
   }
   m_DocDataList.clear();
   m_WbDataList.clear();
   m_nCurrentPage = -1;
   TRACE6("%s ClearAllData\n", __FUNCTION__);
}

bool CPluginDataProcessor::isDocPluginData(const char* data, const int dataLen, QString &pageid) {
   bool isDocData = false;
   pageid = m_nCurrentPage;
   QString qmsg(data);
   int nPos = qmsg.lastIndexOf("&");
   QString msgType = qmsg.mid(nPos, dataLen);
   if (msgType.contains("type=doc")) {
      isDocData = true;
      qmsg = qmsg.replace("param=", "");
      qmsg = qmsg.replace("&type=doc", "");
      QJsonDocument doc = QJsonDocument::fromJson(qmsg.toLocal8Bit());
      if (doc.isObject()) {
         QJsonObject obj = doc.object();
         QVariantMap varmap = obj.toVariantMap();
         QString szActionParam;
         if (varmap.contains("content")) {
            QString contentJson = varmap.value("content").toString();
            QJsonDocument contentDoc = QJsonDocument::fromJson(contentJson.toLocal8Bit());
            if (contentDoc.isObject()) {
               QJsonObject contentObj = contentDoc.object();
               QVariantMap varmap = contentObj.toVariantMap();
               QString page;
               QString doc;
               if (varmap.contains("page")) {
                  int npage = varmap.value("page").toInt();
                  page = QString::number(npage);
               }
               if (varmap.contains("doc")) {
                  doc = varmap.value("doc").toString();
               }
               if (!doc.isEmpty() && !page.isEmpty()) {
                  pageid = doc + QString("/") + page;
               }
            }
         }
      }
   }
   return isDocData;
}

bool CPluginDataProcessor::isWBPluginData(const char* data, const int dataLen) {
   bool isWBData = false;
   QString qmsg(data);
   int nPos = qmsg.lastIndexOf("&");
   QString msgType = qmsg.mid(nPos, dataLen);
   if (msgType.contains("type=board")) {
      isWBData = true;
   }
   return isWBData;
}

bool CPluginDataProcessor::HasPushStreamState() {
   QMutexLocker lock(&m_mutexStream);
   return m_bHasPushStream;
}

void CPluginDataProcessor::Slot_PushStream() {
   QMutexLocker lock(&m_mutexStream);
   TRACE6("%s\n", __FUNCTION__);
   if (m_bHasPushStream) {
      //如果设置了开始推流状态，那么发送数据队列遗留数据。
      SendWBPluginData();
      SendDocPluginData();
   }
   if (m_pPushTimer) {
      m_pPushTimer->stop();
   }
}



