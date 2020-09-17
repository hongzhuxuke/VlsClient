#ifndef _SOCKET_SERVER_INCLUDE_H__
#define _SOCKET_SERVER_INCLUDE_H__


#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <WinSock2.h>

#include <stdio.h>
#include <tchar.h>
#include <iostream>
#include <process.h>
#include <string> 
#include <fstream>


#include <map>
//using namespace std;
class ConnManager;
class SocketServer {
public:
   SocketServer(u_short port);
   virtual ~SocketServer();
   bool StartServer();
   void ShutDown();
private:
   bool initSocket();
   DWORD __stdcall acceptThreadProc();
   static DWORD __stdcall acceptThread(void *);

private:
   HANDLE mAcceptThread;
   bool mServerRunning;
   bool mStartFlag;
   SOCKET mServerSocket;
   u_short mPort;
public:
   static ConnManager* mConnManager;
};


#endif

