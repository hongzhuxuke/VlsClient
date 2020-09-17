//#include "stdafx.h"

#ifdef WIN32
//#include <windows.h>
#include <tchar.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <sys/timeb.h>
#include <stdarg.h>
#include <string.h>

#include "DebugTrace.h"


int CDebugTrace::mnLogLevel = 3;	
char CDebugTrace::mszLogFileName[512] = {0};
unsigned CDebugTrace::muTraceOptions = Timestamp;
static CDebugTrace goDebugTrace;

// $_FUNCTION_BEGIN ******************************
// 函数名称: CDebugTrace( )
// 函数参数: 
// 返 回 值: 
// 函数说明: 构造函数
// $_FUNCTION_END ********************************
CDebugTrace::CDebugTrace( )
{
    m_lDataLen = 0;	
}


// $_FUNCTION_BEGIN ******************************
// 函数名称: ~CDebugTrace( )
// 函数参数: 
// 返 回 值: 
// 函数说明: 析构函数
// $_FUNCTION_END ********************************
CDebugTrace::~CDebugTrace( )
{
    
}


// $_FUNCTION_BEGIN ******************************
// 函数名称: CDebugTrace& operator << ( char acCharVal )
// 函数参数: char acCharVal：要打印的字符
// 返 回 值: 
// 函数说明: 打印单个字符
// $_FUNCTION_END ********************************
CDebugTrace& CDebugTrace::operator << ( char acCharVal )
{
    char * lpWritePtr = mszPrintBuff + m_lDataLen;
    
    m_lDataLen += sprintf( lpWritePtr,"%c",acCharVal );
    
    ASSERT( m_lDataLen <= DEF_TRACE_BUFF_LEN );
    return *this;
}



// $_FUNCTION_BEGIN ******************************
// 函数名称: CDebugTrace& operator << ( bool abBoolVal )
// 函数参数: bool abBoolVal:要打印的BOOL型数据
// 返 回 值: 
// 函数说明: 打印bool值
// $_FUNCTION_END ********************************
CDebugTrace& CDebugTrace::operator << ( bool abBoolVal )
{
    char * lpWritePtr = mszPrintBuff + m_lDataLen;
    if ( abBoolVal )
    {
        m_lDataLen += sprintf( lpWritePtr,"%s","TRUE" );
    }
    else
    {
        m_lDataLen += sprintf( lpWritePtr,"%s","FALSE" );
    }
    ASSERT( m_lDataLen <= DEF_TRACE_BUFF_LEN );
    return *this;
}



// $_FUNCTION_BEGIN ******************************
// 函数名称: CDebugTrace& operator << ( short asShortVal )
// 函数参数: short asShortVal：要打印的short型数据
// 返 回 值: 
// 函数说明: 打印short值
// $_FUNCTION_END ********************************
CDebugTrace& CDebugTrace::operator << ( short asShortVal )
{
    char * lpWritePtr = mszPrintBuff + m_lDataLen;
    
    m_lDataLen += sprintf( lpWritePtr,"%d",asShortVal );
    
    ASSERT( m_lDataLen <= DEF_TRACE_BUFF_LEN );
    return *this;
}


// $_FUNCTION_BEGIN ******************************
// 函数名称: CDebugTrace& operator << ( unsigned short asShortVal )
// 函数参数: unsigned short asShortVal：要打印的 unsigned short型数据
// 返 回 值: 
// 函数说明: 打印unsigned short值
// $_FUNCTION_END ********************************
CDebugTrace& CDebugTrace::operator << ( unsigned short asShortVal )
{
    char * lpWritePtr = mszPrintBuff + m_lDataLen;
    
    m_lDataLen += sprintf( lpWritePtr,"%u",asShortVal );
    
    ASSERT( m_lDataLen <= DEF_TRACE_BUFF_LEN );
    return *this;
}

// $_FUNCTION_BEGIN ******************************
// 函数名称: CDebugTrace& operator << ( int aiIntVal )
// 函数参数: int aiIntVal：要打印的 int 型数据
// 返 回 值: 
// 函数说明: 打印int值
// $_FUNCTION_END ********************************
CDebugTrace& CDebugTrace::operator << ( int aiIntVal )
{
    char * lpWritePtr = mszPrintBuff + m_lDataLen;
    
    m_lDataLen += sprintf( lpWritePtr,"%d",aiIntVal );
    
    ASSERT( m_lDataLen <= DEF_TRACE_BUFF_LEN );
    return *this;
}


// $_FUNCTION_BEGIN ******************************
// 函数名称: CDebugTrace& operator << ( unsigned int aiIntVal )
// 函数参数: unsigned int aiIntVal：要打印的 unsigned int 型数据
// 返 回 值: 
// 函数说明: 打印 unsigned int值
// $_FUNCTION_END ********************************
CDebugTrace& CDebugTrace::operator << ( unsigned int aiIntVal )
{
    char * lpWritePtr = mszPrintBuff + m_lDataLen;
    
    m_lDataLen += sprintf( lpWritePtr,"%u",aiIntVal );
    
    ASSERT( m_lDataLen <= DEF_TRACE_BUFF_LEN );
    return *this;
}

// $_FUNCTION_BEGIN ******************************
// 函数名称: CDebugTrace& operator << ( long aiIntVal )
// 函数参数: long aiIntVal：要打印的 long 型数据
// 返 回 值: 
// 函数说明: 打印 long值
// $_FUNCTION_END ********************************
CDebugTrace& CDebugTrace::operator << ( long alLongVal )
{
    char * lpWritePtr = mszPrintBuff + m_lDataLen;
    
    m_lDataLen += sprintf( lpWritePtr,"%ld",alLongVal );
    
    ASSERT( m_lDataLen <= DEF_TRACE_BUFF_LEN );
    return *this;
}

// $_FUNCTION_BEGIN ******************************
// 函数名称: CDebugTrace& operator << ( unsigned long aiIntVal )
// 函数参数: unsigned long aiIntVal：要打印的 unsigned long 型数据
// 返 回 值: 
// 函数说明: 打印 unsigned long
// $_FUNCTION_END ********************************
CDebugTrace& CDebugTrace::operator << ( unsigned long alLongVal )
{
    char * lpWritePtr = mszPrintBuff + m_lDataLen;
    
    m_lDataLen += sprintf( lpWritePtr,"%lu",alLongVal );
    
    ASSERT( m_lDataLen <= DEF_TRACE_BUFF_LEN );
    return *this;
}

// $_FUNCTION_BEGIN ******************************
// 函数名称: CDebugTrace& operator << ( float afFloatVal )
// 函数参数: float afFloatVal：要打印的 float 型数据
// 返 回 值: 
// 函数说明: 打印float值
// $_FUNCTION_END ********************************
CDebugTrace& CDebugTrace::operator << ( float afFloatVal )
{
    char * lpWritePtr = mszPrintBuff + m_lDataLen;
    
    m_lDataLen += sprintf( lpWritePtr,"%f",afFloatVal );
    
    ASSERT( m_lDataLen <= DEF_TRACE_BUFF_LEN );
    return *this;
}

// $_FUNCTION_BEGIN ******************************
// 函数名称: CDebugTrace& operator << ( double afdoubleVal )
// 函数参数: double afdoubleVal：要打印的 double 型数据
// 返 回 值: 
// 函数说明: 打印double值
// $_FUNCTION_END ********************************
CDebugTrace& CDebugTrace::operator << ( double afdoubleVal )
{
    char * lpWritePtr = mszPrintBuff + m_lDataLen;
    
    m_lDataLen += sprintf( lpWritePtr,"%f",afdoubleVal );
    
    ASSERT( m_lDataLen <= DEF_TRACE_BUFF_LEN );
    return *this;
}


// $_FUNCTION_BEGIN ******************************
// 函数名称: CDebugTrace& operator << ( INT64 aiInt64Val )
// 函数参数: INT64 aiInt64Val：要打印的 INT64  型数据
// 返 回 值: 
// 函数说明: 打印64位整数
// $_FUNCTION_END ********************************
CDebugTrace& CDebugTrace::operator << ( INT64 aiInt64Val )
{
    char *lpWritePtr = mszPrintBuff + m_lDataLen;
    
#ifdef WIN32
    m_lDataLen += sprintf( lpWritePtr,"%I64d",aiInt64Val );
#else    
	m_lDataLen += sprintf( lpWritePtr,"%lld",aiInt64Val );
#endif

    ASSERT( m_lDataLen <= DEF_TRACE_BUFF_LEN );
    return *this;
}



// $_FUNCTION_BEGIN ******************************
// 函数名称: CDebugTrace& operator << ( INT64 aiInt64Val )
// 函数参数: INT64 aiInt64Val：要打印的 INT64  型数据
// 返 回 值: 
// 函数说明: 打印64位整数
// $_FUNCTION_END ********************************
CDebugTrace& CDebugTrace::operator << (UINT64 aiInt64Val )
{
    char *lpWritePtr = mszPrintBuff + m_lDataLen;
    
#ifdef WIN32
    m_lDataLen += sprintf( lpWritePtr,"%I64d",aiInt64Val );
#else    
	m_lDataLen += sprintf( lpWritePtr,"%lld",aiInt64Val );
#endif

    ASSERT( m_lDataLen <= DEF_TRACE_BUFF_LEN );
    return *this;
}


// $_FUNCTION_BEGIN ******************************
// 函数名称: CDebugTrace& operator << ( const char *apStrVal )
// 函数参数: const char *apStrVal：要打印的\0结尾的字符串
// 返 回 值: 
// 函数说明: 打印以\0结尾的字符串值
// $_FUNCTION_END ********************************
CDebugTrace& CDebugTrace::operator << ( const char *apStrVal )
{
    char * lpWritePtr = mszPrintBuff + m_lDataLen;
    if ( apStrVal == 0 )
    {
        m_lDataLen += sprintf( lpWritePtr,"%s","NULL" );
    }
    else
    {
		m_lDataLen += sprintf( lpWritePtr,"%s",apStrVal );
    }
    
    ASSERT( m_lDataLen <= DEF_TRACE_BUFF_LEN );
    return *this;
}


// $_FUNCTION_BEGIN ******************************
// 函数名称: CDebugTrace& operator << ( const char *apStrVal )
// 函数参数: const char *apStrVal：要打印的\0结尾的字符串
// 返 回 值: 
// 函数说明: 打印以\0结尾的字符串值
// $_FUNCTION_END ********************************
#ifdef UNICODE
CDebugTrace& CDebugTrace::operator << ( const WCHAR *apStrVal )
{
    char * lpWritePtr = mszPrintBuff + m_lDataLen;
    if ( apStrVal == 0 )
    {
        m_lDataLen += sprintf( lpWritePtr,"%s","NULL" );
    }
    else
    {
		m_lDataLen += ::WideCharToMultiByte(CP_GBK, 0, apStrVal, wcslen(apStrVal), lpWritePtr, DEF_TRACE_BUFF_LEN - m_lDataLen,NULL,NULL);
    }
    
    ASSERT( m_lDataLen <= DEF_TRACE_BUFF_LEN );
    return *this;
}
#endif

// $_FUNCTION_BEGIN ******************************
// 函数名称: void TraceFormat( const char * pFmt,... )
// 函数参数: const char * pFmt,...( 可变长度参数,如:"姓名:%s,年龄:%d","zht",26 )
// 返 回 值: void
// 函数说明: 类似printf函数的格式打印一串日志
// $_FUNCTION_END ********************************
void CDebugTrace::TraceFormat( const char * pFmt,... )
{
#ifndef WIN32
	//把%I64d替换成%lld
	char *pPtr1 = (char *) pFmt;
	char szFormat[512] = {0};

	char *pPtr2 = strstr(pPtr1,"%I64d");	
	if (pPtr2 != NULL)
	{
		int nCopyLen = 0;
		while (pPtr2 != NULL)
		{
			memcpy(szFormat+nCopyLen,pPtr1,pPtr2-pPtr1);
			nCopyLen += (pPtr2-pPtr1);

			strcpy(szFormat + nCopyLen, "%lld");
			nCopyLen += strlen("%lld");
				
			pPtr1 = pPtr2 + strlen("%I64d");
			pPtr2 = strstr(pPtr1,"%I64d");
		}
		strcpy(szFormat+nCopyLen,pPtr1);
		pPtr1 = szFormat;
	}

	va_list argptr;
	//把strPrintBuff追加到mszPrintBuff中
	char * lpWritePtr = mszPrintBuff + m_lDataLen;
	va_start( argptr,pPtr1 );
	m_lDataLen += vsprintf( lpWritePtr, pPtr1, argptr );
	va_end( argptr );

#else
	va_list argptr;
	//把strPrintBuff追加到mszPrintBuff中
	char * lpWritePtr = mszPrintBuff + m_lDataLen;

	//把可变参数序列化到strPrintBuff中
    va_start( argptr,pFmt );
	m_lDataLen += vsprintf( lpWritePtr, pFmt, argptr );	
    va_end( argptr );
#endif

    //调用EndTrace结束打印
    EndTrace( *this );
}

////////////////////////////////////////////////
//以下是静态函数

// $_FUNCTION_BEGIN ******************************
// 函数名称: void CDebugTrace::SetTraceLevel( int aiTraceLevel )
// 函数参数: int aiTraceLevel:日志级别
// 返 回 值: void
// 函数说明: 设置TRACE等级( 0级最高,1其次,依次类推,小于该等级的日志不打印 )
// $_FUNCTION_END ********************************
void CDebugTrace::SetTraceLevel( int aiTraceLevel )
{
    CDebugTrace::mnLogLevel = aiTraceLevel;
}



// $_FUNCTION_BEGIN ******************************
// 函数名称: void CDebugTrace::SetLogFileName( TCHAR *aszLogFile )
// 函数参数: TCHAR *aszLogFile:日志文件名
// 返 回 值: void
// 函数说明: 设置输出日志文件名
// $_FUNCTION_END ********************************
#ifdef UNICODE
//设置日志文件名
void CDebugTrace::SetLogFileName( const TCHAR *aszLogFile )
#else
//设置日志文件名
void CDebugTrace::SetLogFileName( const char *aszLogFile )
#endif
{
    if ( aszLogFile != NULL )
    {
#ifdef UNICODE
		::WideCharToMultiByte(CP_GBK, 0, aszLogFile, (int)wcslen(aszLogFile), CDebugTrace::mszLogFileName, 512,NULL,NULL);
#else
		strcpy(CDebugTrace::mszLogFileName,aszLogFile );
#endif
    }
}

// $_FUNCTION_BEGIN ******************************
// 函数名称: void CDebugTrace::SetTraceOptions( unsigned options /** New level for trace */ )
// 函数参数: unsigned options:打印选项 各选项可以相加
// 返 回 值: void
// 函数说明: 设置TRACE选项
// $_FUNCTION_END ********************************
void CDebugTrace::SetTraceOptions( unsigned options /** New level for trace */ )
{
    CDebugTrace::muTraceOptions = options;
}

//
// $_FUNCTION_BEGIN ******************************
// 函数名称: unsigned CDebugTrace::GetTraceOptions( void )
// 函数参数: 
// 返 回 值: unsigned 打印选项
// 函数说明: 取得TRACE选项
// $_FUNCTION_END ********************************
unsigned CDebugTrace::GetTraceOptions( void )
{
    return CDebugTrace::muTraceOptions;
}

// $_FUNCTION_BEGIN ******************************
// 函数名称: bool CDebugTrace::CanTrace( int aiLogLevel )
// 函数参数: int aiLogLevel:日志级别
// 返 回 值: bool
// 函数说明: 判断给定级别是否可以打印
// $_FUNCTION_END ********************************
bool CDebugTrace::CanTrace( int aiLogLevel )
{
    return ( aiLogLevel <= CDebugTrace::mnLogLevel );
}

//*****************************************************************************
//	函数原型：	BeginTrace( int aiLogLevel,char *apSrcFile,int aiSrcLine )
//  函数说明:   开始打印一个日志
//  参数：      int aiLogLevel( 日志的级别 ),char *apSrcFile( 源文件名 ),
//				int aiSrcLine( 源行数 )
//  返回值：    返回类对象自身
//  用法：		BeginTrace( 3,__FILE__,__LINE__ )
//*****************************************************************************
CDebugTrace& CDebugTrace::BeginTrace( int aiLogLevel,const char *apSrcFile,int aiSrcLine )
{
    CDebugTrace *lpDebugTrace = NULL;
    //lpDebugTrace = new CDebugTrace( );
    lpDebugTrace = &goDebugTrace;
    lpDebugTrace->m_oCriticalSection.Lock();

    ASSERT( lpDebugTrace != NULL );
    lpDebugTrace->m_lDataLen = 0;	 //已打印的数据长度清0
    
    //如果要求输出时间,则在日志中输出日志产生的时间( 分:秒:毫秒 )
    if ( CDebugTrace::muTraceOptions & Timestamp ) 
    {
        struct timeb loTimeb;
        ftime( &loTimeb ); 
        struct tm * lstrutime = localtime( &loTimeb.time );
        
		//保护系统时间超过2038年情况下不会出现异常，但也没有正常的时间输出了。
		//这样修改比较简单，毕竟系统时间出现错误的情况较少，真到2038年再说吧。
		if (NULL != lstrutime)	
		{
			char lszTraceBuff[20];
#ifdef WIN32
	        _snprintf( lszTraceBuff, 20, "%02d:%02d:%02d:%03d ",\
		        lstrutime->tm_hour,lstrutime->tm_min,lstrutime->tm_sec,loTimeb.millitm );
#else
			snprintf( lszTraceBuff, 20, "%02d:%02d:%02d:%03d ",\
				lstrutime->tm_hour,lstrutime->tm_min,lstrutime->tm_sec,loTimeb.millitm );
#endif
			*lpDebugTrace << lszTraceBuff;
		}
	}
    //如果要求输出日志级别,则在日志中输出日志级别
    if ( CDebugTrace::muTraceOptions & LogLevel )
    {
        *lpDebugTrace << aiLogLevel << ' ';
    }
    
    //如果要求输出源文件名和行号,则在日志中输出源文件名和行号
    if ( CDebugTrace::muTraceOptions & FileAndLine ) 
    {
        *lpDebugTrace << apSrcFile << "( " << aiSrcLine << " ) ";
    }
    
    //返回对象引用
    return *lpDebugTrace;
}


//*****************************************************************************
//	函数原型：	EndTrace( CDebugTrace &aoDebugTrace )
//  函数说明:   结束打印一个日志
//  参数：      CDebugTrace &aoDebugTrace( CDebugTrace 对象引用 )
//  返回值：    
//  用法：		
//*****************************************************************************
void CDebugTrace::EndTrace( CDebugTrace &aoDebugTrace )		//结束打印
{
#ifdef WIN32
	#ifdef _DEBUG
		#ifdef UNICODE
			WCHAR		szWideBuff[DEF_TRACE_BUFF_LEN];	 //打印数据缓存
			int nWideLen = ::MultiByteToWideChar(CP_GBK, 0, aoDebugTrace.mszPrintBuff, strlen(aoDebugTrace.mszPrintBuff), szWideBuff, DEF_TRACE_BUFF_LEN);
			szWideBuff[nWideLen] = 0;

			//输出到调试窗口中
			OutputDebugStringW( szWideBuff ); 
		#else
			OutputDebugStringA( aoDebugTrace.mszPrintBuff ); 
		#endif
	#endif
#endif	

    //若要求输出到控制台,则把日志信息在控制台也打印一下
    if ( CDebugTrace::muTraceOptions & PrintToConsole ) 
        printf( "%s",aoDebugTrace.mszPrintBuff );
    
    //若要求写文件且设置了日志文件名,则把日志信息写入文件中
    if ( ( CDebugTrace::muTraceOptions & AppendToFile ) \
        && ( strlen( CDebugTrace::mszLogFileName ) > 1 ) )
    {
        FILE * lfpTraceFile = NULL;
        lfpTraceFile = fopen( CDebugTrace::mszLogFileName,"a" ); 	
        if ( lfpTraceFile != NULL )
        {
            fprintf( lfpTraceFile,"%s",aoDebugTrace.mszPrintBuff );
            fclose( lfpTraceFile );
        }
    }

    aoDebugTrace.m_oCriticalSection.UnLock();
    //delete &aoDebugTrace;	
}


// $_FUNCTION_BEGIN ******************************
// 函数名称: void CDebugTrace::AssertFail( char * strCond,char *strFile, unsigned uLine )
// 函数参数: char * strCond: 函数表达式
//           char *strFile:源文件名
//           unsigned uLine：行号
// 返 回 值: void
// 函数说明: 弹出ASSERT出错信息
// $_FUNCTION_END ********************************
void CDebugTrace::AssertFail(const char * strCond,char *strFile, unsigned uLine )
{
#ifdef WIN32    
      
#ifdef UNICODE
	WCHAR szMessage[1024] = {0};
	char		szMultiBuff[1024];	 //打印数据缓存
	
	sprintf( szMultiBuff, "\nDebug Assertion Failed!\nFile: %s \nLine:%u \nCond: ASSERT( %s ); \n",strFile,uLine,strCond );    
	int nWideLen = ::MultiByteToWideChar(CP_GBK, 0, szMultiBuff, strlen(szMultiBuff), szMessage, 1024);
	szMessage[nWideLen] = 0;

	//输出到调试窗口中
	OutputDebugString( szMessage ); 

	sprintf( szMultiBuff, " Debug Assertion Failed!\n\n Condition:  ASSERT( %s );       \n SourceFile: %s       \n LineNum:    %u \n\n Continue?",
		strCond,strFile,uLine );
	nWideLen = ::MultiByteToWideChar(CP_GBK, 0, szMultiBuff, strlen(szMultiBuff), szMessage, 1024);
	szMessage[nWideLen] = 0;

	int nResult = MessageBox( NULL,szMessage,TEXT("Assert failure"),MB_OKCANCEL+MB_ICONERROR );    
	if ( nResult == IDCANCEL )
	{
		FatalExit( -1 );
	}
	else
	{
		DebugBreak();
	}

#else
	char szMessage[1024];
	sprintf( szMessage, "\nDebug Assertion Failed!\nFile: %s \nLine:%d \nCond: ASSERT( %s ); \n",strFile,uLine,strCond );    

	//输出到调试窗口中
	OutputDebugString( szMessage ); 

	sprintf( szMessage, " Debug Assertion Failed!\n\n Condition:  ASSERT( %s );       \n SourceFile: %s       \n LineNum:    %d \n\n Continue?",
		strCond,strFile,uLine );
	int nResult = MessageBox( NULL,szMessage,TEXT("Assert failure"),MB_OKCANCEL+MB_ICONERROR );    
	if ( nResult == IDCANCEL )
	{
		FatalExit( -1 );
	}
	else
	{
		DebugBreak();
	}

#endif
#else    
	//LINUX
    printf( "Debug Assertion Failed!\n\nCondition:  ASSERT( %s );       \nSourceFile: %s       \nLineNum:    %d \n\nContinue?",
        strCond,strFile,uLine );		
    
    char lcUserInput =  getchar( );	
    if ( lcUserInput != 'y' && lcUserInput != 'Y' )
    {
        exit( -1 );
    }
#endif 
}
