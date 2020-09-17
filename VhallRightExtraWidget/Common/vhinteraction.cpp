#include "vhinteraction.h"
#include <QDebug>
#include <QJsonArray>
#include "DebugTrace.h"
VHInteraction::VHInteraction(QObject *parent)
:QObject(parent){
}

VHInteraction::~VHInteraction() {   
   m_mutex.lock();
   if(m_clinet) {
      DestroyInteractionClient(&m_clinet);
      m_clinet = NULL;
   }   
   m_mutex.unlock();
}


bool AnalysisEventObject(QJsonObject &obj,Event &e) {
   if(obj.empty()) {
      return false;
   }
   e.m_eMsgType = (MsgRQType)obj["m_eMsgType"].toInt();
   e.m_iPageCount = obj["m_iPageCount"].toInt();
   e.m_currentPage = obj["m_currentPage"].toInt();
   e.m_sumNumber = obj["m_sumNumber"].toInt();
   
   e.m_bResult = obj["m_bResult"].toBool();

   QJsonArray m_oUserList = obj["m_oUserList"].toArray();

   for(int i=0;i<m_oUserList.count();i++) {
      QJsonObject userObj = m_oUserList[i].toObject();
      QString szUserName = userObj["m_szUserName"].toString();
      QString szUserID = userObj["m_szUserID"].toString();
      QString szRole = userObj["m_szRole"].toString();
      UserInfo userInfo;
      memset(&userInfo,0,sizeof(UserInfo));
      szUserName.toWCharArray(userInfo.m_szUserName);
      szUserID.toWCharArray(userInfo.m_szUserID);
      szRole.toWCharArray(userInfo.m_szRole);
      e.m_oUserList.push_back(userInfo);
   }

   QJsonObject userInfo = obj["userInfo"].toObject();
   QString szUserName = userInfo["m_szUserName"].toString();
   QString szUserID = userInfo["m_szUserID"].toString();
   QString szRole = userInfo["m_szRole"].toString();
   bool m_isGag=obj["m_isGag"].toBool();

   
   szUserName.toWCharArray(e.m_oUserInfo.m_szUserName);
   szUserID.toWCharArray(e.m_oUserInfo.m_szUserID);
   szRole.toWCharArray(e.m_oUserInfo.m_szRole);
   e.m_isGag = m_isGag;
   QString m_wzText=obj["m_wzText"].toString();
   strcpy(e.m_wzText,m_wzText.toUtf8().data());
   QJsonArray m_oImageList=obj["m_oImageList"].toArray();
   for(int i=0 ;i<m_oImageList.count();i++) {
      QJsonObject img = m_oImageList[i].toObject();
      if(img.keys().count() > 0) {
         std::string key = img.keys()[0].toStdString();
         std::string value =img.value(QString::fromStdString(key)).toString().toStdString();
         e.m_oImageList[value]=key;
      }
   }
   return true;
}

QJsonObject MakeEventObject(Event *e) {
   QJsonObject obj;
   obj["m_eMsgType"]=(int)e->m_eMsgType;
   obj["m_iPageCount"]=e->m_iPageCount;
   obj["m_currentPage"]=e->m_currentPage;
   obj["m_sumNumber"]=e->m_sumNumber;
   obj["m_bResult"]=e->m_bResult;
   QJsonArray m_oUserList;
   for(int i=0;i<e->m_oUserList.size();i++) {
      QJsonObject userObj;
      userObj["m_szUserName"] = QString::fromWCharArray(e->m_oUserList[i].m_szUserName);
      userObj["m_szUserID"] = QString::fromWCharArray(e->m_oUserList[i].m_szUserID);
      userObj["m_szRole"] = QString::fromWCharArray(e->m_oUserList[i].m_szRole);
      m_oUserList.append(userObj);
   }

   obj["m_oUserList"]=m_oUserList;
   QJsonObject userInfo;
   userInfo["m_szUserName"]=QString::fromWCharArray(e->m_oUserInfo.m_szUserName);
   userInfo["m_szUserID"]=QString::fromWCharArray(e->m_oUserInfo.m_szUserID);
   userInfo["m_szRole"]=QString::fromWCharArray(e->m_oUserInfo.m_szRole);
   obj["m_isGag"]=e->m_isGag;
   obj["userInfo"]=userInfo;
   obj["m_wzText"]=e->m_wzText;

   QJsonArray m_oImageList;
   if(e->m_oImageList.size()>0) {
      for(auto itor=e->m_oImageList.begin();itor!=e->m_oImageList.end();itor++) {
         QJsonObject img;
         img[QString::fromStdString(itor->second)]=QString::fromStdString(itor->first);
         m_oImageList.append(img);
         qDebug()<<"img:"<<img;
      }
   }
   obj["m_oImageList"]=m_oImageList;
   return obj;
}


void VHInteraction::onConnected(Event* event) {
	TRACE6("VHInteraction::onConnected TRUE!\n");
	emit SigOnConnected();
}

void VHInteraction::onMessaged(Event* event) {
   if(m_roomId != event->m_lRoomID) {
      qDebug()<<"VHInteraction::onMessaged roomId not same "<<m_roomId<<event->m_lRoomID;
      return ;
   }
   QJsonObject obj = MakeEventObject(event);;
   emit SigMsg(obj);
}

//void VHInteraction::onDisconnected(Event* event) {
//   qDebug()<<"#################################VHInteraction::onDisconnected#################################";
//}

void VHInteraction::onError(Event* event) {
   qDebug()<<"#################################VHInteraction::onError#################################";
}
BOOL VHInteraction::Create(){
	SetMessageSocketIOCallBack(nullptr);
	SetMessageWebSocketCallBack(nullptr);
   return TRUE;
}

void VHInteraction::Destory() {
   m_mutex.lock();
   if(m_clinet) {
      DestroyInteractionClient(&m_clinet);
      m_clinet = NULL;
   }   
   m_mutex.unlock();
}

BOOL VHInteraction::Init(Key_Data &oKeyData) {
   TRACE6("%s enter\n",__FUNCTION__);
   m_roomId = oKeyData.m_lRoomID;
   TRACE6("VHInteraction::Init\n");
   QByteArray ba = QByteArray((char *)&oKeyData,sizeof(Key_Data));
   InitSocketThread(ba);
   TRACE6("%s Leave\n",__FUNCTION__);
   return true;
}

void VHInteraction::UnInit() {
   TRACE6("%s enter\n", __FUNCTION__);
   m_mutex.lock();
   if (m_clinet) {
      TRACE6("%s DisConnect\n",__FUNCTION__);
      m_clinet->DisConnect();
      TRACE6("VHInteraction DisConnect ok \n");
      m_clinet->UnInit();
      TRACE6("VHInteraction::run() end \n");
   }
   m_mutex.unlock();
   emit this->SigUnInitFinished();
   TRACE6("%s leave\n", __FUNCTION__);
}

void VHInteraction::InitSocketThread(QByteArray pKeyData) {
   TRACE6("VHInteraction::SlotInit\n");
   if (m_bInit) {
      TRACE6("VHInteraction::SlotInit m_bInit true\n");
      UnInitSocketThread();
      m_mutex.lock();
      if (m_clinet) {
         m_clinet->UnInit();
         TRACE6("%s uninit end \n",__FUNCTION__);
         DestroyInteractionClient(&m_clinet);
         m_clinet = NULL;
      }
      m_mutex.unlock();
   }
   TRACE6("VHInteraction::SlotInit CreateInteractionClient\n");
   m_mutex.lock();
   if (m_clinet != NULL) {
      DestroyInteractionClient(&m_clinet);
      m_clinet = NULL;
   }
   m_clinet = CreateInteractionClient();
   m_clinet->SetMessageSocketIOCallBack(m_socketIOCallBack);
   m_clinet->SetMessageWebSocketCallBack(m_websocketCallBack);
   Key_Data oKeyData = *(Key_Data *)pKeyData.data();

   TRACE6("VHInteraction::SlotInit CreateInteractionClient Init\n");
   m_clinet->Init(this, oKeyData);
   TRACE6("VHInteraction::SlotInit CreateInteractionClient ConnectServer\n");
   m_clinet->ConnectServer();
   m_mutex.unlock();
   m_bInit = true;
   TRACE6("VHInteraction::SlotInit CreateInteractionClient return\n");
}

void VHInteraction::UnInitSocketThread() {
   TRACE6("VHInteraction::SlotUnInit\n");
   if (!m_clinet || m_bInit != true) {
      TRACE6("VHInteraction::SlotUnInit Failed!\n");
      return;
   }

   m_bInit = false;
   TRACE6("VHInteraction::SlotUnInit TRUE!\n");
}

void VHInteraction::MessageRQ(MsgRQType eMsgType, RQData* vData) {   
   m_mutex.lock();
   if(!m_clinet || !m_bInit) {      
      m_mutex.unlock();
      return ;
   }
   m_clinet->MessageRQ(eMsgType,vData);   
   m_mutex.unlock();
}

//socketIO 回调
void VHInteraction::SetMessageSocketIOCallBack(InteractionMsgCallBack callback) {
   m_mutex.lock();
   if (NULL == callback)
   {
	   TRACE6("%s :callback IS NULL\n", __FUNCTION__);
   }
   m_socketIOCallBack = callback;
   if(m_clinet) {
      m_clinet->SetMessageSocketIOCallBack(m_socketIOCallBack);
   }   
   m_mutex.unlock();
}

//websocket 回调
void VHInteraction::SetMessageWebSocketCallBack(InteractionMsgCallBack callback) {
   m_mutex.lock();
   m_websocketCallBack = callback;
   if(m_clinet) {
      m_clinet->SetMessageWebSocketCallBack(m_websocketCallBack);
   }   
   m_mutex.unlock();
}

void VHInteraction::SetHttpProxy(bool enable, const char *proxy_ip, const char* proxy_port, const char *proxy_username, const char *proxy_password) {
   if (m_clinet) {
      m_clinet->SetProxyAddr(enable, proxy_ip, proxy_port, proxy_username, proxy_password);
   }
}
