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
// ��������: CDebugTrace( )
// ��������: 
// �� �� ֵ: 
// ����˵��: ���캯��
// $_FUNCTION_END ********************************
CDebugTrace::CDebugTrace( )
{
    m_lDataLen = 0;	
}


// $_FUNCTION_BEGIN ******************************
// ��������: ~CDebugTrace( )
// ��������: 
// �� �� ֵ: 
// ����˵��: ��������
// $_FUNCTION_END ********************************
CDebugTrace::~CDebugTrace( )
{
    
}


// $_FUNCTION_BEGIN ******************************
// ��������: CDebugTrace& operator << ( char acCharVal )
// ��������: char acCharVal��Ҫ��ӡ���ַ�
// �� �� ֵ: 
// ����˵��: ��ӡ�����ַ�
// $_FUNCTION_END ********************************
CDebugTrace& CDebugTrace::operator << ( char acCharVal )
{
    char * lpWritePtr = mszPrintBuff + m_lDataLen;
    
    m_lDataLen += sprintf( lpWritePtr,"%c",acCharVal );
    
    ASSERT( m_lDataLen <= DEF_TRACE_BUFF_LEN );
    return *this;
}



// $_FUNCTION_BEGIN ******************************
// ��������: CDebugTrace& operator << ( bool abBoolVal )
// ��������: bool abBoolVal:Ҫ��ӡ��BOOL������
// �� �� ֵ: 
// ����˵��: ��ӡboolֵ
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
// ��������: CDebugTrace& operator << ( short asShortVal )
// ��������: short asShortVal��Ҫ��ӡ��short������
// �� �� ֵ: 
// ����˵��: ��ӡshortֵ
// $_FUNCTION_END ********************************
CDebugTrace& CDebugTrace::operator << ( short asShortVal )
{
    char * lpWritePtr = mszPrintBuff + m_lDataLen;
    
    m_lDataLen += sprintf( lpWritePtr,"%d",asShortVal );
    
    ASSERT( m_lDataLen <= DEF_TRACE_BUFF_LEN );
    return *this;
}


// $_FUNCTION_BEGIN ******************************
// ��������: CDebugTrace& operator << ( unsigned short asShortVal )
// ��������: unsigned short asShortVal��Ҫ��ӡ�� unsigned short������
// �� �� ֵ: 
// ����˵��: ��ӡunsigned shortֵ
// $_FUNCTION_END ********************************
CDebugTrace& CDebugTrace::operator << ( unsigned short asShortVal )
{
    char * lpWritePtr = mszPrintBuff + m_lDataLen;
    
    m_lDataLen += sprintf( lpWritePtr,"%u",asShortVal );
    
    ASSERT( m_lDataLen <= DEF_TRACE_BUFF_LEN );
    return *this;
}

// $_FUNCTION_BEGIN ******************************
// ��������: CDebugTrace& operator << ( int aiIntVal )
// ��������: int aiIntVal��Ҫ��ӡ�� int ������
// �� �� ֵ: 
// ����˵��: ��ӡintֵ
// $_FUNCTION_END ********************************
CDebugTrace& CDebugTrace::operator << ( int aiIntVal )
{
    char * lpWritePtr = mszPrintBuff + m_lDataLen;
    
    m_lDataLen += sprintf( lpWritePtr,"%d",aiIntVal );
    
    ASSERT( m_lDataLen <= DEF_TRACE_BUFF_LEN );
    return *this;
}


// $_FUNCTION_BEGIN ******************************
// ��������: CDebugTrace& operator << ( unsigned int aiIntVal )
// ��������: unsigned int aiIntVal��Ҫ��ӡ�� unsigned int ������
// �� �� ֵ: 
// ����˵��: ��ӡ unsigned intֵ
// $_FUNCTION_END ********************************
CDebugTrace& CDebugTrace::operator << ( unsigned int aiIntVal )
{
    char * lpWritePtr = mszPrintBuff + m_lDataLen;
    
    m_lDataLen += sprintf( lpWritePtr,"%u",aiIntVal );
    
    ASSERT( m_lDataLen <= DEF_TRACE_BUFF_LEN );
    return *this;
}

// $_FUNCTION_BEGIN ******************************
// ��������: CDebugTrace& operator << ( long aiIntVal )
// ��������: long aiIntVal��Ҫ��ӡ�� long ������
// �� �� ֵ: 
// ����˵��: ��ӡ longֵ
// $_FUNCTION_END ********************************
CDebugTrace& CDebugTrace::operator << ( long alLongVal )
{
    char * lpWritePtr = mszPrintBuff + m_lDataLen;
    
    m_lDataLen += sprintf( lpWritePtr,"%ld",alLongVal );
    
    ASSERT( m_lDataLen <= DEF_TRACE_BUFF_LEN );
    return *this;
}

// $_FUNCTION_BEGIN ******************************
// ��������: CDebugTrace& operator << ( unsigned long aiIntVal )
// ��������: unsigned long aiIntVal��Ҫ��ӡ�� unsigned long ������
// �� �� ֵ: 
// ����˵��: ��ӡ unsigned long
// $_FUNCTION_END ********************************
CDebugTrace& CDebugTrace::operator << ( unsigned long alLongVal )
{
    char * lpWritePtr = mszPrintBuff + m_lDataLen;
    
    m_lDataLen += sprintf( lpWritePtr,"%lu",alLongVal );
    
    ASSERT( m_lDataLen <= DEF_TRACE_BUFF_LEN );
    return *this;
}

// $_FUNCTION_BEGIN ******************************
// ��������: CDebugTrace& operator << ( float afFloatVal )
// ��������: float afFloatVal��Ҫ��ӡ�� float ������
// �� �� ֵ: 
// ����˵��: ��ӡfloatֵ
// $_FUNCTION_END ********************************
CDebugTrace& CDebugTrace::operator << ( float afFloatVal )
{
    char * lpWritePtr = mszPrintBuff + m_lDataLen;
    
    m_lDataLen += sprintf( lpWritePtr,"%f",afFloatVal );
    
    ASSERT( m_lDataLen <= DEF_TRACE_BUFF_LEN );
    return *this;
}

// $_FUNCTION_BEGIN ******************************
// ��������: CDebugTrace& operator << ( double afdoubleVal )
// ��������: double afdoubleVal��Ҫ��ӡ�� double ������
// �� �� ֵ: 
// ����˵��: ��ӡdoubleֵ
// $_FUNCTION_END ********************************
CDebugTrace& CDebugTrace::operator << ( double afdoubleVal )
{
    char * lpWritePtr = mszPrintBuff + m_lDataLen;
    
    m_lDataLen += sprintf( lpWritePtr,"%f",afdoubleVal );
    
    ASSERT( m_lDataLen <= DEF_TRACE_BUFF_LEN );
    return *this;
}


// $_FUNCTION_BEGIN ******************************
// ��������: CDebugTrace& operator << ( INT64 aiInt64Val )
// ��������: INT64 aiInt64Val��Ҫ��ӡ�� INT64  ������
// �� �� ֵ: 
// ����˵��: ��ӡ64λ����
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
// ��������: CDebugTrace& operator << ( INT64 aiInt64Val )
// ��������: INT64 aiInt64Val��Ҫ��ӡ�� INT64  ������
// �� �� ֵ: 
// ����˵��: ��ӡ64λ����
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
// ��������: CDebugTrace& operator << ( const char *apStrVal )
// ��������: const char *apStrVal��Ҫ��ӡ��\0��β���ַ���
// �� �� ֵ: 
// ����˵��: ��ӡ��\0��β���ַ���ֵ
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
// ��������: CDebugTrace& operator << ( const char *apStrVal )
// ��������: const char *apStrVal��Ҫ��ӡ��\0��β���ַ���
// �� �� ֵ: 
// ����˵��: ��ӡ��\0��β���ַ���ֵ
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
// ��������: void TraceFormat( const char * pFmt,... )
// ��������: const char * pFmt,...( �ɱ䳤�Ȳ���,��:"����:%s,����:%d","zht",26 )
// �� �� ֵ: void
// ����˵��: ����printf�����ĸ�ʽ��ӡһ����־
// $_FUNCTION_END ********************************
void CDebugTrace::TraceFormat( const char * pFmt,... )
{
#ifndef WIN32
	//��%I64d�滻��%lld
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
	//��strPrintBuff׷�ӵ�mszPrintBuff��
	char * lpWritePtr = mszPrintBuff + m_lDataLen;
	va_start( argptr,pPtr1 );
	m_lDataLen += vsprintf( lpWritePtr, pPtr1, argptr );
	va_end( argptr );

#else
	va_list argptr;
	//��strPrintBuff׷�ӵ�mszPrintBuff��
	char * lpWritePtr = mszPrintBuff + m_lDataLen;

	//�ѿɱ�������л���strPrintBuff��
    va_start( argptr,pFmt );
	m_lDataLen += vsprintf( lpWritePtr, pFmt, argptr );	
    va_end( argptr );
#endif

    //����EndTrace������ӡ
    EndTrace( *this );
}

////////////////////////////////////////////////
//�����Ǿ�̬����

// $_FUNCTION_BEGIN ******************************
// ��������: void CDebugTrace::SetTraceLevel( int aiTraceLevel )
// ��������: int aiTraceLevel:��־����
// �� �� ֵ: void
// ����˵��: ����TRACE�ȼ�( 0�����,1���,��������,С�ڸõȼ�����־����ӡ )
// $_FUNCTION_END ********************************
void CDebugTrace::SetTraceLevel( int aiTraceLevel )
{
    CDebugTrace::mnLogLevel = aiTraceLevel;
}



// $_FUNCTION_BEGIN ******************************
// ��������: void CDebugTrace::SetLogFileName( TCHAR *aszLogFile )
// ��������: TCHAR *aszLogFile:��־�ļ���
// �� �� ֵ: void
// ����˵��: ���������־�ļ���
// $_FUNCTION_END ********************************
#ifdef UNICODE
//������־�ļ���
void CDebugTrace::SetLogFileName( const TCHAR *aszLogFile )
#else
//������־�ļ���
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
// ��������: void CDebugTrace::SetTraceOptions( unsigned options /** New level for trace */ )
// ��������: unsigned options:��ӡѡ�� ��ѡ��������
// �� �� ֵ: void
// ����˵��: ����TRACEѡ��
// $_FUNCTION_END ********************************
void CDebugTrace::SetTraceOptions( unsigned options /** New level for trace */ )
{
    CDebugTrace::muTraceOptions = options;
}

//
// $_FUNCTION_BEGIN ******************************
// ��������: unsigned CDebugTrace::GetTraceOptions( void )
// ��������: 
// �� �� ֵ: unsigned ��ӡѡ��
// ����˵��: ȡ��TRACEѡ��
// $_FUNCTION_END ********************************
unsigned CDebugTrace::GetTraceOptions( void )
{
    return CDebugTrace::muTraceOptions;
}

// $_FUNCTION_BEGIN ******************************
// ��������: bool CDebugTrace::CanTrace( int aiLogLevel )
// ��������: int aiLogLevel:��־����
// �� �� ֵ: bool
// ����˵��: �жϸ��������Ƿ���Դ�ӡ
// $_FUNCTION_END ********************************
bool CDebugTrace::CanTrace( int aiLogLevel )
{
    return ( aiLogLevel <= CDebugTrace::mnLogLevel );
}

//*****************************************************************************
//	����ԭ�ͣ�	BeginTrace( int aiLogLevel,char *apSrcFile,int aiSrcLine )
//  ����˵��:   ��ʼ��ӡһ����־
//  ������      int aiLogLevel( ��־�ļ��� ),char *apSrcFile( Դ�ļ��� ),
//				int aiSrcLine( Դ���� )
//  ����ֵ��    �������������
//  �÷���		BeginTrace( 3,__FILE__,__LINE__ )
//*****************************************************************************
CDebugTrace& CDebugTrace::BeginTrace( int aiLogLevel,const char *apSrcFile,int aiSrcLine )
{
    CDebugTrace *lpDebugTrace = NULL;
    //lpDebugTrace = new CDebugTrace( );
    lpDebugTrace = &goDebugTrace;
    lpDebugTrace->m_oCriticalSection.Lock();

    ASSERT( lpDebugTrace != NULL );
    lpDebugTrace->m_lDataLen = 0;	 //�Ѵ�ӡ�����ݳ�����0
    
    //���Ҫ�����ʱ��,������־�������־������ʱ��( ��:��:���� )
    if ( CDebugTrace::muTraceOptions & Timestamp ) 
    {
        struct timeb loTimeb;
        ftime( &loTimeb ); 
        struct tm * lstrutime = localtime( &loTimeb.time );
        
		//����ϵͳʱ�䳬��2038������²�������쳣����Ҳû��������ʱ������ˡ�
		//�����޸ıȽϼ򵥣��Ͼ�ϵͳʱ����ִ����������٣��浽2038����˵�ɡ�
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
    //���Ҫ�������־����,������־�������־����
    if ( CDebugTrace::muTraceOptions & LogLevel )
    {
        *lpDebugTrace << aiLogLevel << ' ';
    }
    
    //���Ҫ�����Դ�ļ������к�,������־�����Դ�ļ������к�
    if ( CDebugTrace::muTraceOptions & FileAndLine ) 
    {
        *lpDebugTrace << apSrcFile << "( " << aiSrcLine << " ) ";
    }
    
    //���ض�������
    return *lpDebugTrace;
}


//*****************************************************************************
//	����ԭ�ͣ�	EndTrace( CDebugTrace &aoDebugTrace )
//  ����˵��:   ������ӡһ����־
//  ������      CDebugTrace &aoDebugTrace( CDebugTrace �������� )
//  ����ֵ��    
//  �÷���		
//*****************************************************************************
void CDebugTrace::EndTrace( CDebugTrace &aoDebugTrace )		//������ӡ
{
#ifdef WIN32
	#ifdef _DEBUG
		#ifdef UNICODE
			WCHAR		szWideBuff[DEF_TRACE_BUFF_LEN];	 //��ӡ���ݻ���
			int nWideLen = ::MultiByteToWideChar(CP_GBK, 0, aoDebugTrace.mszPrintBuff, strlen(aoDebugTrace.mszPrintBuff), szWideBuff, DEF_TRACE_BUFF_LEN);
			szWideBuff[nWideLen] = 0;

			//��������Դ�����
			OutputDebugStringW( szWideBuff ); 
		#else
			OutputDebugStringA( aoDebugTrace.mszPrintBuff ); 
		#endif
	#endif
#endif	

    //��Ҫ�����������̨,�����־��Ϣ�ڿ���̨Ҳ��ӡһ��
    if ( CDebugTrace::muTraceOptions & PrintToConsole ) 
        printf( "%s",aoDebugTrace.mszPrintBuff );
    
    //��Ҫ��д�ļ�����������־�ļ���,�����־��Ϣд���ļ���
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
// ��������: void CDebugTrace::AssertFail( char * strCond,char *strFile, unsigned uLine )
// ��������: char * strCond: �������ʽ
//           char *strFile:Դ�ļ���
//           unsigned uLine���к�
// �� �� ֵ: void
// ����˵��: ����ASSERT������Ϣ
// $_FUNCTION_END ********************************
void CDebugTrace::AssertFail(const char * strCond,char *strFile, unsigned uLine )
{
#ifdef WIN32    
      
#ifdef UNICODE
	WCHAR szMessage[1024] = {0};
	char		szMultiBuff[1024];	 //��ӡ���ݻ���
	
	sprintf( szMultiBuff, "\nDebug Assertion Failed!\nFile: %s \nLine:%u \nCond: ASSERT( %s ); \n",strFile,uLine,strCond );    
	int nWideLen = ::MultiByteToWideChar(CP_GBK, 0, szMultiBuff, strlen(szMultiBuff), szMessage, 1024);
	szMessage[nWideLen] = 0;

	//��������Դ�����
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

	//��������Դ�����
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
