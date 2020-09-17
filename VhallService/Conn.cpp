
#include "Conn.h"
#include "common/Defs.h"
//#include "common/charset.h"
#include <map>
#include "json.h"
#include "VhallService.h"
#include "common/Logging.h"

#include <winsock2.h>

using namespace VHJson;
using namespace std;
LPWSTR GetLastErrorText();

void saveAddress(const char *logfile, const char *fmt, ...)
{
	
	WCHAR logFilePath[_MAX_PATH];
	GetWindowsDirectoryW(logFilePath, _MAX_PATH);

	char output[_MAX_PATH];
	memset(output, 0, _MAX_PATH);
	sprintf_s(output, "%ws\\%s", logFilePath, logfile);
	//g_pLogger->logInfo("output:%s", output);

//	int fd;
	va_list ap;
	char msg[1024];
	//struct tm *nowp, now;

	ofstream ofs;  //提供写文件的功能

	ofs.open(output, ios::trunc);
	memset(msg, 0, sizeof(msg));
	va_start(ap, fmt);
	vsnprintf_s(msg, sizeof(msg), fmt, ap);
	va_end(ap);
	ofs << msg;

	ofs.close();
}

Conn::Conn(ConnManager* connManager, const SOCKET& sClient) :
mClientSocket(sClient),
mWorkerThread(NULL),
mRunning(false),
mConnManager(connManager) {
   ConnLogInfo(L"==============[Conn::Conn]==============");
   SetConnType(CONN_UNKNOW);
   mMissAs3ConnTime = 0;
   ConnLogInfo(L"==============[Conn::Conn return]==============");
}

Conn::~Conn() {
   ShutDown();
}

bool Conn::Start() {
   
   ConnLogInfo(L"==============[Conn::Start]==============");
   mRunning = true;
   mWorkerThread = CreateThread(0, 0, Conn::serviceThread, this,NULL, NULL);
   //accept client, and create thread.
   if (mWorkerThread == NULL) {
      ConnLogInfo(L"[Conn::Start] serviceThread failed  %s", GetLastErrorText());
      mRunning = false;
      return false;
   }
   
   ConnLogInfo(L"==============[Conn::Start] return==============");
   return true;
}

void Conn::ShutDown() {
   ConnLogInfo(L"==============[Conn::ShutDown]==============");
   ConnLogInfo(L"Conn::ShutDown socket %d  will close", mClientSocket);
   closesocket(mClientSocket);
   ConnLogInfo(L"==============[Conn::ShutDown] return==============");
}

void Conn::HandleData(char *buff, int datasize) {
   ConnLogInfoA("================[Conn::HandleData]=======================");
   ConnLogInfoA("Conn::handleDataA %s  - [%u]", buff, mClientSocket);
   std::string msgType;
   char resbuff[MSG_SIZE];
   if (0 == lstrcmpiA(buff, POLICY_REQ)) {      
      ConnLogInfoA("[HandleData] is POLICY_REQ");
      Send(POLICY_RES, strlen(POLICY_RES));
      ConnLogInfoA("[HandleData] Send [%s]",POLICY_RES);
      Sleep(1000);      
      ConnLogInfoA("[HandleData] POLICY_RES and ShutDown");
      ShutDown();      
      ConnLogInfoA("================[Conn::HandleData] return end POLICY_RES===========");
      return;
   }
   
   try {
      std::string strTemp;
      VHJson::Reader reader;
      VHJson::Value value;
      if (!reader.parse(buff, value)) {
         ConnLogInfo(L"[Conn::HandleData] pan  failed, parser json failed - [%d]", mClientSocket);
         //send error msg         
         sprintf_s(resbuff, MSG_SIZE, REPLY_MSG_BADREQ, "unknown", " parser json failed ");
         Send(resbuff, strlen(resbuff));
      }
      
      msgType = value["type"].asString();
      if (0 == lstrcmpiA(msgType.c_str(), MSG_TYPE_ENGINE_START)) {
         ConnLogInfoA("[HandleData] msgType is [%s] == MSG_TYPE_ENGINE_START",msgType.c_str());

         bool ret = false;
         std::string uname;  //user name
         std::string address; //rtmp url, like: rtmp://192.168.1.7/vhall
         std::string sn;  //stream name
         std::string token;  //token auth
         std::string activeId;  //active id
         std::string activeName;  //active name
         std::string isHideLogo;

         std::string userId;
         std::string role;
         std::string webninartype;
         std::string accesstoken;
         std::string scheduler;

         uname = value["uname"].asString();
         address = value["address"].asString();
		 saveAddress("address.ini", "%s", address.c_str());

         sn = value["streamName"].asString();
         token = value["token"].asString();
         activeId = value["activeId"].asString();
         isHideLogo = value["hide_powered"].asString();
         activeName = value["activeName"].asString();
         
         userId = value["uid"].asString();
         role = value["role"].asString();
          
         bool isMix = value["ismix"].asBool();
         webninartype = isMix?"2":"1";
         
         accesstoken = value["accesstoken"].asString();
         scheduler = value["scheduler"].asString();

         //get helper client
         Conn* helperConn = mConnManager->GetConn(CONN_HLPER_CLIENT, NULL);
         if (helperConn == NULL) {
            ConnLogInfo(L"Conn::HandleData start vhall client , socket = %d", (int)mClientSocket);
            ret = StartVhallClient(
                  uname.c_str(), 
                  address.c_str(), 
                  token.c_str(), 
                  sn.c_str(), 
                  isHideLogo == "1",
                  
                  userId.c_str(),
                  role.c_str(),
                  webninartype.c_str(),
                  accesstoken.c_str(),
                  scheduler.c_str()
            );
         }
         
         SetConnType(CONN_AS3SOCKET);
         mStreamName = sn;
         mStartupParam = buff;
      }
      else if (0 == lstrcmpiA(msgType.c_str(), MSG_TYPE_ENGINE_STOP)) {
         // StopVhallClient();                                             
         ConnLogInfo(L"Handle Data MSG_TYPE_ENGINE_STOP\n");
         Conn* helperConn = mConnManager->GetConn(CONN_HLPER_CLIENT, mStreamName.c_str());
         if (helperConn){            
            sprintf_s(resbuff, MSG_SIZE, MSG_BODY, MSG_TYPE_ENGINE_STOP, mStreamName.c_str());
            ConnLogInfo(L"Handle Data MSG_TYPE_ENGINE_STOP sendTo VhallLive [%s]\n",resbuff);

            helperConn->Send(resbuff, strlen(resbuff));
            //helperConn->ShutDown();
         }
         else {
            ConnLogInfo(L"Handle Data MSG_TYPE_ENGINE_STOP not sendTo VhallLive \n");
         }
         
         SetConnType(CONN_AS3SOCKET);
      } else if (0 == lstrcmpiA(msgType.c_str(), MSG_TYPE_ENGINE_QUERY)) {
         SetConnType(CONN_AS3SOCKET);
         mStreamName = value["stream"].asString();
         Conn* helperConn = mConnManager->GetConn(CONN_HLPER_CLIENT, mStreamName.c_str());
         if (helperConn) {
            sprintf_s(resbuff, MSG_SIZE, MSG_BODY, MSG_TYPE_NOTIFY_HELPER_LIVE, mStreamName.c_str());
            ConnLogInfo(L"Conn::HandleData get helper connection. - [%d]", mClientSocket);
         } else {
            sprintf_s(resbuff, MSG_SIZE, MSG_BODY, MSG_TYPE_NOTIFY_HELPER_DIED, mStreamName.c_str());
            ConnLogInfo(L"Conn::HandleData miss helper connection. - [%d]", mClientSocket);
         }
         Send(resbuff, strlen(resbuff));
      } else if (0 == lstrcmpiA(msgType.c_str(), MSG_TYPE_NOTIFY_PUBLISH_SUCCESS)
                 || 0 == lstrcmpiA(msgType.c_str(), MSG_TYPE_NOTIFY_PUBLISH_FAILED)
                 || 0 == lstrcmpiA(msgType.c_str(), MSG_TYPE_NOTIFY_PUBLISH_STOPED)
                 || 0 == lstrcmpiA(msgType.c_str(), MSG_TYPE_CLOSE_STREAM)
         || 0 == lstrcmpiA(msgType.c_str(), MSG_TYPE_NOTIFY_PUBLISH_BEGIN)
         ) {
         
         SetConnType(CONN_HLPER_CLIENT);
         mStreamName = value["stream"].asString();
         //get as3 client
         Conn* as3Conn = mConnManager->GetConn(CONN_AS3SOCKET, mStreamName.c_str());
         if (as3Conn){
            as3Conn->Send(buff, datasize);
            Sleep(100);
         }
            
         else {
            //send 
            ConnLogInfo(L"Conn::HandleData miss AS3 connectio %d - [%d]", mMissAs3ConnTime, mClientSocket);
            //wait for 2 time hearbeat,so ignore this            
         }
         //response for helper application
         sprintf_s(resbuff, MSG_SIZE, REPLY_MSG_SUCESS, msgType.c_str(), mStreamName.c_str());
         Send(resbuff, strlen(resbuff));
      } 
      else if (0 == lstrcmpiA(msgType.c_str(), MSG_TYPE_HEARBEAT)) {        
         ConnLogInfoA("[Conn::HandleData] MSG_TYPE_HEARBEAT");
         
         SetConnType(CONN_HLPER_CLIENT);
         mStreamName = value["stream"].asString();
         
         ConnLogInfoA("[Conn::HandleData] MSG_TYPE_HEARBEAT streamName (%s)",mStreamName.c_str());
         Conn* as3Conn = mConnManager->GetConn(CONN_AS3SOCKET, mStreamName.c_str());
         if (as3Conn == NULL) {            
            ConnLogInfoA("[Conn::HandleData] MSG_TYPE_HEARBEAT as3Conn == NULL");
            mMissAs3ConnTime++;
            ConnLogInfoA("[Conn::HandleData] miss AS3 connection %d - [%d]", mMissAs3ConnTime, mClientSocket);
            if (mMissAs3ConnTime >= 15){
               ShutDown();
            }
            
            sprintf_s(resbuff, MSG_SIZE, REPLY_MSG_BADREQ, msgType.c_str(), mStreamName.c_str());
            Send(resbuff, strlen(resbuff));
            
         } 
         else {            
            ConnLogInfoA("[Conn::HandleData] MSG_TYPE_HEARBEAT as3Conn is not NULL");
            mMissAs3ConnTime = 0;
            Send(as3Conn->mStartupParam.c_str(), as3Conn->mStartupParam.length());
         }
         
         ConnLogInfoA("[Conn::HandleData] MSG_TYPE_HEARBEAT End");
      }
   }
   catch (exception &ex) {
      ConnLogInfo(L"[Conn::HandleData] JsonStringToStructData exception %s. - [%u]", ex.what(), mClientSocket);
      sprintf_s(resbuff, MSG_SIZE, REPLY_MSG_ERROR, "unknown", "json parser failed ");
      Send(resbuff, strlen(resbuff));
   }
}
SOCKET Conn::GetSocket() {
   return mClientSocket;
}
DWORD  Conn::GetConnKey() {
   return mConnKey;
}
bool Conn::Send(const char *sendBuff, const int& dataSize) {
   ConnLogInfoA("==============[Conn::Send] ==============");
   ConnLogInfoA("[Conn::Send][%s]",sendBuff);
   int ret = send(mClientSocket, sendBuff, dataSize, 0);
   if (ret == SOCKET_ERROR) {
      ConnLogInfo(L"Conn::Send() send failed  %s - [%d]", GetLastErrorText(), mClientSocket);
      return false;
   }
   
   ConnLogInfoA("==============[Conn::Send] return ==============");
   return true;
}
bool stringToWString(const string& narrowStr, wstring& wStr);
BOOL WcharToChar(const WCHAR *wStr,
			char **mbStr)
{
	size_t mbstrLen;
	size_t WcharLen;
	BOOL   bUsedDefault = FALSE;
	if (!wStr) {
		*mbStr = NULL;
		return FALSE;
	}
	WcharLen = wcslen(wStr);
	if (WcharLen == 0) {
		*mbStr = NULL;
		return FALSE;
	}

	/* Get the buffer size. */
	mbstrLen = WideCharToMultiByte(CP_ACP, 0, wStr, -1, NULL, 0,NULL,&bUsedDefault);
	if (mbstrLen == 0) {
		fprintf(stderr, "WcharToChar: Error getting buffer size: %d\n", 
			GetLastError());
		*mbStr = NULL;
		return FALSE;
	}

	*mbStr = (char *)malloc(sizeof(char) * (mbstrLen + 1));
	//   assert(*wStr);

	mbstrLen = WideCharToMultiByte(CP_ACP, 0, wStr, -1,
		*mbStr, (int)mbstrLen,NULL,&bUsedDefault);
	if (mbstrLen == 0) {
		fprintf(stderr, "WcharToChar WideCharToMultiByte error: %d\n", 
			GetLastError());
		free(*mbStr);
		*mbStr = NULL;
		return FALSE;
	}
	(*mbStr)[mbstrLen] = '\0';
	return TRUE;
}

std::string	wstringToString(std::wstring str)
{
	char* pCha = NULL;
	std::string s;
	WcharToChar(str.c_str(),&pCha);;
	if(pCha) 
	{
		s = pCha;
		free(pCha);
	}
	return s;
}

DWORD __stdcall Conn::serviceThreadProc() {
   char recvBuffer[SIZE_RECV]={0};
   while (true) {
      int ret;
      memset(recvBuffer, 0, SIZE_RECV);
      ConnLogInfoA("==============[Conn::serviceThreadProc] ==============");
      ConnLogInfo(L"[Conn::serviceThreadProc] recv before");
      ret = recv(mClientSocket, (char *)recvBuffer, SIZE_RECV, 0);
      ConnLogInfo(L"[Conn::serviceThreadProc] recv end");
      
      if (ret == SOCKET_ERROR) {
         ConnLogInfo(L"[Conn::serviceThreadProc] recv failed  [%s] - [%u]", GetLastErrorText(), mClientSocket);
         break;
      } 
      else if (ret == 0) {
         ConnLogInfo(L"[Conn::serviceThreadProc] recv none   [%s] - [%u]", GetLastErrorText(), mClientSocket);
         Sleep(1000);
         break;
      }
      else  if (ret > 0) {         
         ConnLogInfo(L"Conn::serviceThreadProc() receive ret = [%d] ",ret);         
         HandleData(recvBuffer, ret);
      }
   }
   
   if (GetConnType() == CONN_HLPER_CLIENT) {
      //notify 
      Conn* as3Conn = mConnManager->GetConn(CONN_AS3SOCKET, mStreamName.c_str());
      if (as3Conn) {
         ConnLogInfo(L"Conn::serviceThreadProc get AS3 connection, will notify helperdied - [%d]", GetSocket());
         char resbuff[MSG_SIZE];
         sprintf_s(resbuff, MSG_SIZE, MSG_BODY, MSG_TYPE_NOTIFY_HELPER_DIED, "");
         as3Conn->Send(resbuff, strlen(resbuff));
      }
      else {
         //send 
         ConnLogInfo(L"Conn::serviceThreadProc miss AS3 connectio %d [%d]", mMissAs3ConnTime, GetSocket());
      }
   }
   
   ConnLogInfo(L"Conn::serviceThreadProc socket exit - [%d]", mClientSocket);
   return 0;
}
DWORD __stdcall Conn::serviceThread(void * obj) {
   Conn *conn = (Conn *)obj;
   conn->serviceThreadProc();
   //delete conn;
   conn->mConnManager->RemoveConn(conn);
   
   return 0;
}
void Conn::SetConnType(EN_CONN_TYPE type){
   this->_mConnType = type;
   switch(type) {
      case CONN_AS3SOCKET: {
         mConnTypeStr = L"[TYPE_AS3]";
         break;
      }
      case CONN_HLPER_CLIENT: {
         mConnTypeStr = L"[TYPE_CLIENT]";
         break;
      }
      default:{         
         mConnTypeStr = L"[TYPE_UNKNOW]";
         break;
      }
   }
}
EN_CONN_TYPE Conn::GetConnType(){
   return this->_mConnType;
}
std::wstring Conn::GetConnTypeStr(){
   wchar_t typeStr[256]={0};
   swprintf_s(typeStr,256,L"[%p][%d][%lu]",this,mClientSocket,mConnKey);
   std::wstring str = typeStr;
   str = str + mConnTypeStr;
   return str;
}
void Conn::ConnLogInfo(wchar_t * fmtStr, ...){
   if(!gLogger) {
      return ;
   }
   
   wchar_t logBuf[1024]={0};   
   wchar_t logBuf2[1024]={0};   

   va_list args;
   va_start(args, fmtStr);
   vswprintf_s(logBuf,1024,fmtStr,args);   
	va_end(args);
   
   swprintf_s(logBuf2,1024,L"%s%s",GetConnTypeStr().c_str(),logBuf);   
   gLogger->logInfo(logBuf2);
}
void Conn::ConnLogInfoA(char *fmtStr, ...)
{
   char logBuf[1024] = {0};
   char logBuf2[1024] = {0};
   va_list args;
   va_start(args, fmtStr);
   vsprintf_s(logBuf,1024,fmtStr,args);   
	va_end(args);
   std::wstring wstr = GetConnTypeStr().c_str();
   std::string str = wstringToString(wstr);
   
   sprintf_s(logBuf2,1024,"%s%s",str.c_str(),logBuf);   
   gLogger->logInfo(logBuf2);
}
ConnManager::ConnManager() {
   InitializeCriticalSection(&mClientListCS);
}
ConnManager::~ConnManager() {
   DeleteCriticalSection(&mClientListCS);
}
Conn* ConnManager::CreateNewConn(SOCKET sock) {
   Conn* newConn = new Conn(this, sock);
   
   newConn->ConnLogInfo(L"================[ConnManager::CreateNewConn]====================");
   int tryTime = 1000;
   EnterCriticalSection(&mClientListCS);
   DWORD dwKey = GetTickCount();   
   newConn->ConnLogInfo(L"[ConnManager::CreateNewConn] GetTickCount %lu",dwKey);
   while (mConnList.find(dwKey) != mConnList.end() && tryTime > 0) {
      Sleep(100);
      tryTime--;
      dwKey = GetTickCount();
      newConn->ConnLogInfo(L"[ConnManager::CreateNewConn] GetTickCount again %lu",dwKey);
   }
   
   newConn->mConnKey = dwKey;
   mConnList[dwKey] = newConn;
   
   newConn->ConnLogInfo(L"[ConnManager::CreateNewConn] mConnList[%lu] = [%p]",dwKey,newConn);

   LeaveCriticalSection(&mClientListCS);
   if (dwKey == 0){
      newConn->ConnLogInfo(L"[ConnManager::CreateNewConn] has no free key.");
   }
   
   newConn->ConnLogInfo(L"================[ConnManager::CreateNewConn] return ====================");

   return newConn;
}
void ConnManager::RemoveConn(Conn* conn) {
   conn->ConnLogInfo(L"================[ConnManager::RemoveConn]====================");
   EnterCriticalSection(&mClientListCS);
   mConnList.erase(conn->GetConnKey());
   LeaveCriticalSection(&mClientListCS);
   //delete conn;      //memory leak
   conn->ConnLogInfo(L"ConnManager::RemoveConn socket=[%d] key=[%d].", conn->GetSocket(), conn->GetConnKey());
   conn->ConnLogInfo(L"================[ConnManager::RemoveConn] return ====================");
}

Conn* ConnManager::GetConn(EN_CONN_TYPE enConn, const char* streamname) {
   g_pLogger->logInfo(L"================ConnManager::GetConn================ mConnList.size() = %d",mConnList.size());
   Conn* conn = NULL;
   EnterCriticalSection(&mClientListCS);
   for (map<DWORD, Conn*>::iterator iter = mConnList.begin();
        iter != mConnList.end(); iter++) {
      /*
       1. connectin type
       2.if  streamname == NULL or streamname == old streamname
       */
      if (iter->second->GetConnType() == enConn
          && (streamname == NULL
          || (streamname != NULL && strcmp(streamname, iter->second->mStreamName.c_str()) == 0))) {
         conn = iter->second;
         g_pLogger->logInfo(L"[ConnManager::GetConn] socket=[%d] key=[%d].", conn->GetSocket(), conn->GetConnKey());
         break;
      }
   }
        
   LeaveCriticalSection(&mClientListCS);   
   if(conn==NULL) {
      g_pLogger->logInfo(L"================ConnManager::GetConn conn = NULL================");
   }
   else {
      conn->ConnLogInfo(L"[ConnManager::GetConn]");
   }
   
   g_pLogger->logInfo(L"================ConnManager::GetConn End================");
   
   return conn;
}


