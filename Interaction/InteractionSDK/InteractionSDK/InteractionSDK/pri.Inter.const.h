#ifndef H_PRI_INTER_CONST_H
#define H_PRI_INTER_CONST_H
#include <list>
#include "IInteractionClient.h"
#include <windows.h> //I've omitted context line

using namespace std;


#define DEF_HTTPLOOP_MAX_TIMEOUT    20000
#define DEF_MAX_RECONNECT_TIME         4
#define DEF_MAX_HTTP_URL_LEN					2048			// 最大HTTP请求URL长度
#define DEF_SEVER_URL					   L"http://t.vhall.com/api/client/v1/clientapi/"			// 服务器URL
#define DEF_SEVER_CHAT_URL					   L"http://api-msg.t.vhall.com/pub/index"			// 服务器URL

struct HttpReq {
	MsgRQType m_eType;
	WCHAR m_wzRequestUrl[DEF_MAX_HTTP_URL_LEN + 1];            //请求URL地址
	HttpReq() {
		memset(this, 0, sizeof(this));
	}
};

typedef list<HttpReq> HttpReqList;

struct SocketIOReq {
	MsgRQType m_eType;
	char m_szOrder[256];             //请求命令
	char m_szParam[10240];             //请求参数
	SocketIOReq() {
		memset(this, 0, sizeof(this));
	}
};

typedef list<Event> EventList;

struct HttpProxyInfo {
	char ip[256];
	char user[256];
	char pwd[256];
	int port;
	HttpProxyInfo() {
		memset(this, 0, sizeof(HttpProxyInfo));
	};
};

#endif//H_PRI_INTER_CONST_H
