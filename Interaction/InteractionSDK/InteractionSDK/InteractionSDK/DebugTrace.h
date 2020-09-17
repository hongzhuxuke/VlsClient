// $_FILEHEADER_BEGIN ****************************
/*
对外接口函数说明:

//断言函数.若传入参数不为TRUE则出现断言失败对话框, 并终止程序运行;
//发行版中则什么也不做
//eg:  ASSERT(apStrVal != NULL);	
1、void ASSERT(bool) 

//记录日志 level为日志等级,args为日志内容	
//eg:  RecordLog(1,"姓名:" << lszName<<"年龄:"<< 26 << "性别" << TRUE);	
//TRACE在发行版中为空函数
2、void TRACE/RecordLog(level, args) 

//打印调试信息(0－6表示调试信息的级别,0级最高)，发行版中什么也不做
//eg:TRACE(2,"姓名：%s,年龄：%d,性别:%d",lszName,26,TRUE);
3、TRACE0 - TRACE6	

//设置打印日值的级别（静态函数）
4、	void CDebugTrace::SetTraceLevel(int aiTraceLevel);

//设置日志文件名（静态函数）
5、	void CDebugTrace::SetLogFileName(char *aszLogFile);

// 设置TRACE选项 .注意函数可以 OR 各选项（静态函数）
6、	void CDebugTrace::SetTraceOptions(unsigned options);

//取得TRACE选项（静态函数）
7、	unsigned CDebugTrace::GetTraceOptions(void);	

典型用法：  

CDebugTrace::SetTraceLevel(6);	//设置可打印的日志级别
//设置日志选项
CDebugTrace::SetTraceOptions(CDebugTrace::GetTraceOptions() \
| CDebugTrace::Timestamp | CDebugTrace::LogLevel\
& ~CDebugTrace::FileAndLine | CDebugTrace::AppendToFile\
| CDebugTrace::PrintToConsole);

//设置日志保存文件名
CDebugTrace::SetLogFileName("d:\\test.log");

//TRACE、TRACE0、TRACE1...TRACE6 仅DEBUG下有效，
//在release下请使用RecordLog、RecordLog0、RecordLog1.....RecordLog6
//推荐使用TRACE(1,"姓名:" << lszName<<"年龄:"<< 26 << "性别" << TRUE)这种方式，以免内存越界	


TRACE(1,"姓名:" << lszName<<"年龄:"<< 26 << "性别" << TRUE);	
TRACE0("姓名：%s,年龄：%d,性别:%d",lszName,26,TRUE);

//以下代码release下依然有效
RecordLog(1,"姓名:" << lszName<<"年龄:"<< 26 << "性别" << TRUE);	
RecordLog0("姓名：%s,年龄：%d,性别:%d",lszName,26,TRUE);


ASSERT(apStrVal != NULL);	
*/
// $_FILEHEADER_END ******************************

#ifndef _DEBUGTRACE_HEAD_
#define _DEBUGTRACE_HEAD_

#ifdef WIN32
//#include <windows.h>
#pragma warning(disable:4141 4996)
#endif

#include "CriticalSection.h"
#include "TypeDeff.h"
//////////////////////////////////////////////////////////////////////
//有关ASSERT的定义
#ifdef ASSERT
#undef ASSERT		//取消ASSERT宏
#endif

//  重新定义ASSERT宏  
#ifdef _DEBUG		//测试版
#define ASSERT(f)               \
	if(f)							\
	;							\
		else                            \
		CDebugTrace::AssertFail(#f,__FILE__, __LINE__)

#else			//发行版

#define ASSERT(f)

#endif

//////////////////////////////////////////////////////////////////////
//有关TRACE的定义

//RecordLog是流方式的日志输出
//eg: RecordLog(1,"姓名:" << lszName<<"年龄:"<< 26 << "性别" << TRUE);	

#define RecordLog(level, args) \
	if (!CDebugTrace::CanTrace(level)) 	;  else\
	CDebugTrace::EndTrace(CDebugTrace::BeginTrace(level,__FILE__,__LINE__) << args << '\n')

//TraceLogX系列是类似printf方式输出,X表示日志级别
//eg:TRACE0("姓名：%s,年龄：%d,性别:%d",lszName,26,TRUE);
#define TraceLog0 \
	if (!CDebugTrace::CanTrace(0)) 	;  else\
	CDebugTrace::BeginTrace(0,__FILE__,__LINE__).TraceFormat

#define TraceLog1 \
	if (!CDebugTrace::CanTrace(1)) 	;  else\
	CDebugTrace::BeginTrace(1,__FILE__,__LINE__).TraceFormat

#define TraceLog2 \
	if (!CDebugTrace::CanTrace(2)) 	;  else\
	CDebugTrace::BeginTrace(2,__FILE__,__LINE__).TraceFormat

#define TraceLog3 \
	if (!CDebugTrace::CanTrace(3)) 	;  else\
	CDebugTrace::BeginTrace(3,__FILE__,__LINE__).TraceFormat

#define TraceLog4 \
	if (!CDebugTrace::CanTrace(4)) 	;  else\
	CDebugTrace::BeginTrace(4,__FILE__,__LINE__).TraceFormat

#define TraceLog5 \
	if (!CDebugTrace::CanTrace(5)) 	;  else\
	CDebugTrace::BeginTrace(5,__FILE__,__LINE__).TraceFormat

#define TraceLog6 \
	if (!CDebugTrace::CanTrace(6)) 	;  else\
	CDebugTrace::BeginTrace(6,__FILE__,__LINE__).TraceFormat


//取消TRACE定义
#undef TRACE
#undef TRACE0
#undef TRACE1
#undef TRACE2
#undef TRACE3
#undef TRACE4
#undef TRACE5
#undef TRACE6

//重新定义TRACE语句
//#ifndef _DEBUG			//调试版

#define TRACE RecordLog	//打印日志

#define TRACE0 TraceLog0	

#define TRACE1 TraceLog1

#define TRACE2 TraceLog2

#define TRACE3 TraceLog3

#define TRACE4 TraceLog4

#define TRACE5 TraceLog5

#define TRACE6 TraceLog6

// #else					//发行版
// 
// #define TRACE(level, args) //TRACE函数为空
// 
// #define TRACE0 ;
// #define TRACE1 ;
// #define TRACE2 ;
// #define TRACE3 ;
// #define TRACE4 ;
// #define TRACE5 ;
// #define TRACE6 ;
// 
// #endif //_DEBUG

//////////////////////////////////////////////////////////////////
//下面是CDebugTrace类的定义
#define DEF_TRACE_BUFF_LEN 4096
class CDebugTrace
{
private:
	long		m_lDataLen;			 //数据长度
	char		mszPrintBuff[DEF_TRACE_BUFF_LEN];	 //打印数据缓存

	CCriticalSection m_oCriticalSection;     //临界区
private:
	static int		   mnLogLevel;			 //日志等级
	static char	       mszLogFileName[512]; //日志文件名称
	static unsigned    muTraceOptions;		 //打印日志选项	

public:
	//打印选项
	enum Options 
	{
		/// 打印时间
		Timestamp = 1,
		/// 打印日志级别
		LogLevel = 2,
		/// 打印源文件名和行号
		FileAndLine = 4,
		/// 把日志追加到文件中
		AppendToFile = 8,
		///输出日志到控制台
		PrintToConsole = 16
	};    

	//构造函数	
	CDebugTrace();
	//析够函数
	~CDebugTrace();
public:
	//设置日志级别(0级最高,1其次,依次类推,小于该等级的日志不打印)
	static void SetTraceLevel(int aiTraceLevel);

#ifdef UNICODE
	//设置日志文件名
	static void SetLogFileName(const TCHAR *aszLogFile);
#else
	//设置日志文件名
	static void SetLogFileName(const char *aszLogFile);
#endif

	// 设置TRACE选项 .注意函数可以 OR 各选项
	static void SetTraceOptions(unsigned options /** New level for trace */ );

	//取得TRACE选项
	static unsigned GetTraceOptions(void);

	//判断给定级别是否可以打印
	static bool CanTrace(int aiLogLevel);					

	//开始打印
	static CDebugTrace& BeginTrace(int aiLogLevel,const char *apSrcFile,int aiSrcLine);	

	//结束打印
	static void EndTrace(CDebugTrace &aoDebugTrace);

	//断言失败处理函数
	static void AssertFail(const char * strCond,char *strFile, unsigned uLine);

public:
	//按照函数printf的类似格式打印日志
	void TraceFormat(const char * pFmt,...);	

	//以下分别输出各种数据类型
	CDebugTrace& operator << (char acCharVal);	
	CDebugTrace& operator << (bool abBoolVal);
	CDebugTrace& operator << (short asShortVal);
	CDebugTrace& operator << (unsigned short asShortVal);
	CDebugTrace& operator << (int aiIntVal);
	CDebugTrace& operator << (unsigned int aiIntVal);
	CDebugTrace& operator << (long alLongVal);
	CDebugTrace& operator << (unsigned long alLongVal);
	CDebugTrace& operator << (float afFloatVal);
	CDebugTrace& operator << (double afdoubleVal);
	CDebugTrace& operator << (INT64 aiInt64Val);
	CDebugTrace& operator << (UINT64 aiInt64Val);
	CDebugTrace& operator << (const char *apStrVal);	
#ifdef UNICODE
	CDebugTrace& operator << (const WCHAR *apStrVal);	
#endif
};

#endif//_DEBUGTRACE_HEAD_
