

#ifndef DEF_TYPE_DEFF_H
#define DEF_TYPE_DEFF_H

typedef unsigned char       BYTE;
typedef unsigned short      WORD;

typedef unsigned int        UINT;
typedef unsigned long       DWORD;

typedef int					BOOL;


#ifndef WIN32	//LINUX

typedef int                 SOCKET;
typedef long long           INT64;
typedef unsigned long long    UINT64;
typedef float               FLOAT;
typedef WORD				WCHAR;
//typedef int				socklen_t;  

//2011-12-30，张亚伟
//在Linux平台上，未定义UNICODE时，TCHAR当作char使用
#ifdef UNICODE
typedef	short				TCHAR;
#else
typedef	char				TCHAR;
#endif


#ifndef INVALID_SOCKET
#define INVALID_SOCKET  ~0
#endif
#define SOCKET_ERROR (-1)  
#define closesocket(s) close(s)

#define MAKEWORD(a, b)      ((WORD)(((BYTE)((DWORD)(a) & 0xff)) | ((WORD)((BYTE)((DWORD)(b) & 0xff))) << 8))
#define MAKELONG(a, b)      ((LONG)(((WORD)((DWORD)(a) & 0xffff)) | ((DWORD)((WORD)((DWORD)(b) & 0xffff))) << 16))
#define LOWORD(l)           ((WORD)((DWORD)(l) & 0xffff))
#define HIWORD(l)           ((WORD)((DWORD)(l) >> 16))
#define LOBYTE(w)           ((BYTE)((DWORD)(w) & 0xff))
#define HIBYTE(w)           ((BYTE)((DWORD)(w) >> 8))

#define __TEXT(quote) quote         // r_winnt


#else	//WIN32

typedef __int64             INT64;
typedef int					socklen_t;  
typedef double				DOUBLE;

#endif

#ifndef NULL
#define NULL 0
#endif

#ifndef FALSE
#define FALSE               0
#endif

#ifndef TRUE
#define TRUE                1
#endif

#define CP_GBK				936		//简体中文gb2312的编码

#endif
