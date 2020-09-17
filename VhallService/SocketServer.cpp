#include "SocketServer.h"
#include "common\Logging.h"
#include "json.h"
#include "Conn.h"
LPWSTR GetLastErrorText();
ConnManager*SocketServer::mConnManager = NULL;


//-------------------------------------------
SocketServer::SocketServer(u_short port) :mPort(port) {
   g_pLogger->logInfo(L"==================[SocketServer::SocketServer]================= port(%u)\n",port);
   mAcceptThread = NULL;
   mServerSocket = INVALID_SOCKET;
   mServerRunning = false;
   mStartFlag = false;
   mConnManager = new ConnManager();   
   g_pLogger->logInfo(L"============[SocketServer::SocketServer] End============");
}


SocketServer::~SocketServer() {

}

bool SocketServer::initSocket() {
   
   g_pLogger->logInfo(L"============[SocketServer::initSocket]============");
   SOCKADDR_IN addrServ;
   WCHAR szErr[256] = L"";
   int retVal = 0;
   
   g_pLogger->logInfo(L"[SocketServer::initSocket] socket()\n");
   mServerSocket = socket(AF_INET, SOCK_STREAM, 0);
   if (mServerSocket == INVALID_SOCKET) {
      g_pLogger->logError(L"[SocketServer::initSocket] socket() failed %u - %s", mPort, GetLastErrorText());
      return false;
   }

   //set ip and port
   addrServ.sin_family = AF_INET;
   addrServ.sin_port = htons(mPort);
   addrServ.sin_addr.S_un.S_addr = INADDR_ANY;
   
   //bind
   g_pLogger->logInfo(L"[SocketServer::initSocket] bind()\n");
   retVal = bind(mServerSocket, (SOCKADDR *)&addrServ, sizeof(addrServ));
   if (retVal == SOCKET_ERROR) {
      g_pLogger->logError(L"[SocketServer::initSocket] bind() failed %u - %s", mPort,GetLastErrorText());
      return false;
   }

   //listen
   g_pLogger->logInfo(L"[SocketServer::initSocket] listen()\n");
   retVal = listen(mServerSocket, 5);
   if (retVal == SOCKET_ERROR) {
      g_pLogger->logError(L"[SocketServer::initSocket] listen failed %u - %s", mPort,GetLastErrorText());
      return false;
   }
   
   g_pLogger->logInfo(L"============[SocketServer::initSocket] return ============");
   return true;
}


bool SocketServer::StartServer() {   
   g_pLogger->logInfo(L"============[SocketServer::StartServer] ============");
   mStartFlag = true;
   //initialize socket
   if (initSocket() == false){
      g_pLogger->logInfo(L"[SocketServer::StartServer] initSocket Failed!");
      return false;
   }

   mServerRunning = true;
   mAcceptThread = CreateThread(0, 0, SocketServer::acceptThread, this,NULL, NULL);
   //accept client, and create thread.
   if (mAcceptThread == NULL) {
      g_pLogger->logError(L"[SocketServer::StartServer] CreateThread SocketServer::acceptThread failed %u - %s", mPort,GetLastErrorText());
      mServerRunning = false;
      return false;
   }
   
   g_pLogger->logInfo(L"============[SocketServer::StartServer] return============");
   return true;
}

void SocketServer::ShutDown() {
   g_pLogger->logInfo(L"============[SocketServer::ShutDown]============");
   mStartFlag = false;
   closesocket(mServerSocket);
   g_pLogger->logInfo(L"============[SocketServer::ShutDown] return============");
}

DWORD __stdcall SocketServer::acceptThreadProc() {
   
   g_pLogger->logInfo(L"============[SocketServer::acceptThreadProc]============");
   bool ret = false;
   while (mServerRunning) {
      SOCKADDR_IN addr;
      int len = sizeof(addr);
      SOCKET lientSocket = accept(mServerSocket, (SOCKADDR *)&addr, &len);
      if (lientSocket == INVALID_SOCKET) {
         g_pLogger->logWarning(L"[SocketServer::acceptThreadProc] accept  failed %u - %s", mPort,GetLastErrorText());
         if (mStartFlag == true) {
            g_pLogger->logWarning(L"[SocketServer::acceptThreadProc] mStartFlag true");
            if (initSocket() == false) {
               g_pLogger->logWarning(L"[SocketServer::acceptThreadProc] initSocket()  failed");
               mServerRunning = false;
               return false;
            }
            
            g_pLogger->logWarning(L"[SocketServer::acceptThreadProc] initSocket()  successed!");
            //continue
         }
         else {
            g_pLogger->logWarning(L"[SocketServer::acceptThreadProc] mStartFlag false");
            mServerRunning = false;
            return 0;
         }
      }
      else{
         g_pLogger->logInfo(L"[SocketServer::acceptThreadProc] accept Successed!");
      }
//      g_pLogger->logInfo("accept from ip:%s - %d", inet_ntoa(addr.sin_addr), lientSocket);
      // createNewClientInfo(lientSocket);
      Conn* newConn = mConnManager->CreateNewConn(lientSocket);
      if (newConn) {         
         g_pLogger->logInfo(L"[SocketServer::acceptThreadProc] mConnManager->CreateNewConn Successed!");
         ret = newConn->Start();
         if (ret == false) {            
            g_pLogger->logInfo(L"[SocketServer::acceptThreadProc] m newConn->Start Failed! Will ShutDown");
            newConn->ShutDown();
         }
         else{
            g_pLogger->logInfo(L"[SocketServer::acceptThreadProc] m newConn->Start Successed!");
         }
      }
      else {
         g_pLogger->logInfo(L"[SocketServer::acceptThreadProc] mConnManager->CreateNewConn Failed!");
         closesocket(lientSocket);
      }
   }

   g_pLogger->logInfo(L"============[SocketServer::acceptThreadProc] return============");
   return 0;
}
DWORD __stdcall SocketServer::acceptThread(void *pParam) {
   SocketServer *s = (SocketServer *)pParam;
   return  s->acceptThreadProc();
}



