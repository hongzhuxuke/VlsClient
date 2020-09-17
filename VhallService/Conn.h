#ifndef __CLIENTINFO_INCLUDE_H__
#define __CLIENTINFO_INCLUDE_H__
#define READ 1
#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <WinSock2.h>
#include <map>
#include <mutex>
#include <fstream>

//À©Õ¹ÖØµþIO²Ù×÷
typedef struct io_operation_date {
   OVERLAPPED overlapper;
   WSABUF dataBuff;
   char buff[1024];
   BYTE type;
}IO_OPERATION, *PIO_OPERATION;

typedef enum EN_CONN_TYPE {
   CONN_UNKNOW = 0,
   CONN_AS3SOCKET,
   CONN_HLPER_CLIENT
}*PEN_CONN_TYPE;

class ConnManager;
class Conn {
   friend ConnManager;
public:
   Conn(ConnManager* socketServer, const SOCKET& sClient);
   virtual ~Conn();
   virtual bool Start();
   virtual void ShutDown();
   virtual void HandleData(char *, int);
   SOCKET GetSocket();
   DWORD  GetConnKey();
   
private:
   bool Send(const char *sendBuff, const int& dataSize);
   DWORD __stdcall serviceThreadProc();
   static DWORD __stdcall serviceThread(void *);
   void SetConnType(EN_CONN_TYPE type);
   EN_CONN_TYPE GetConnType();
   std::wstring GetConnTypeStr();
   void ConnLogInfo(wchar_t *fmtStr, ...);
   void ConnLogInfoA(char *fmtStr, ...);
   
private:
   DWORD  mConnKey;
   SOCKET mClientSocket;
   HANDLE mWorkerThread;
   bool mRunning;
   EN_CONN_TYPE _mConnType;
   std::wstring mConnTypeStr;
   
   ConnManager* mConnManager;
   int mMissAs3ConnTime;
   std::string mStreamName;
   std::string mStartupParam;
};

class ConnManager {
public:
   ConnManager();
   ~ConnManager();
   Conn* CreateNewConn(SOCKET sock);
   void RemoveConn(Conn* conn);
   Conn* GetConn(EN_CONN_TYPE enConn, const char* streamname);
private:
   CRITICAL_SECTION mClientListCS;
   std::map<DWORD, Conn*>mConnList;
};
#endif
