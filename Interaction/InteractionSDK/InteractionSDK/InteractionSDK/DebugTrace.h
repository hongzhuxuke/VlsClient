// $_FILEHEADER_BEGIN ****************************
/*
����ӿں���˵��:

//���Ժ���.�����������ΪTRUE����ֶ���ʧ�ܶԻ���, ����ֹ��������;
//���а�����ʲôҲ����
//eg:  ASSERT(apStrVal != NULL);	
1��void ASSERT(bool) 

//��¼��־ levelΪ��־�ȼ�,argsΪ��־����	
//eg:  RecordLog(1,"����:" << lszName<<"����:"<< 26 << "�Ա�" << TRUE);	
//TRACE�ڷ��а���Ϊ�պ���
2��void TRACE/RecordLog(level, args) 

//��ӡ������Ϣ(0��6��ʾ������Ϣ�ļ���,0�����)�����а���ʲôҲ����
//eg:TRACE(2,"������%s,���䣺%d,�Ա�:%d",lszName,26,TRUE);
3��TRACE0 - TRACE6	

//���ô�ӡ��ֵ�ļ��𣨾�̬������
4��	void CDebugTrace::SetTraceLevel(int aiTraceLevel);

//������־�ļ�������̬������
5��	void CDebugTrace::SetLogFileName(char *aszLogFile);

// ����TRACEѡ�� .ע�⺯������ OR ��ѡ���̬������
6��	void CDebugTrace::SetTraceOptions(unsigned options);

//ȡ��TRACEѡ���̬������
7��	unsigned CDebugTrace::GetTraceOptions(void);	

�����÷���  

CDebugTrace::SetTraceLevel(6);	//���ÿɴ�ӡ����־����
//������־ѡ��
CDebugTrace::SetTraceOptions(CDebugTrace::GetTraceOptions() \
| CDebugTrace::Timestamp | CDebugTrace::LogLevel\
& ~CDebugTrace::FileAndLine | CDebugTrace::AppendToFile\
| CDebugTrace::PrintToConsole);

//������־�����ļ���
CDebugTrace::SetLogFileName("d:\\test.log");

//TRACE��TRACE0��TRACE1...TRACE6 ��DEBUG����Ч��
//��release����ʹ��RecordLog��RecordLog0��RecordLog1.....RecordLog6
//�Ƽ�ʹ��TRACE(1,"����:" << lszName<<"����:"<< 26 << "�Ա�" << TRUE)���ַ�ʽ�������ڴ�Խ��	


TRACE(1,"����:" << lszName<<"����:"<< 26 << "�Ա�" << TRUE);	
TRACE0("������%s,���䣺%d,�Ա�:%d",lszName,26,TRUE);

//���´���release����Ȼ��Ч
RecordLog(1,"����:" << lszName<<"����:"<< 26 << "�Ա�" << TRUE);	
RecordLog0("������%s,���䣺%d,�Ա�:%d",lszName,26,TRUE);


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
//�й�ASSERT�Ķ���
#ifdef ASSERT
#undef ASSERT		//ȡ��ASSERT��
#endif

//  ���¶���ASSERT��  
#ifdef _DEBUG		//���԰�
#define ASSERT(f)               \
	if(f)							\
	;							\
		else                            \
		CDebugTrace::AssertFail(#f,__FILE__, __LINE__)

#else			//���а�

#define ASSERT(f)

#endif

//////////////////////////////////////////////////////////////////////
//�й�TRACE�Ķ���

//RecordLog������ʽ����־���
//eg: RecordLog(1,"����:" << lszName<<"����:"<< 26 << "�Ա�" << TRUE);	

#define RecordLog(level, args) \
	if (!CDebugTrace::CanTrace(level)) 	;  else\
	CDebugTrace::EndTrace(CDebugTrace::BeginTrace(level,__FILE__,__LINE__) << args << '\n')

//TraceLogXϵ��������printf��ʽ���,X��ʾ��־����
//eg:TRACE0("������%s,���䣺%d,�Ա�:%d",lszName,26,TRUE);
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


//ȡ��TRACE����
#undef TRACE
#undef TRACE0
#undef TRACE1
#undef TRACE2
#undef TRACE3
#undef TRACE4
#undef TRACE5
#undef TRACE6

//���¶���TRACE���
//#ifndef _DEBUG			//���԰�

#define TRACE RecordLog	//��ӡ��־

#define TRACE0 TraceLog0	

#define TRACE1 TraceLog1

#define TRACE2 TraceLog2

#define TRACE3 TraceLog3

#define TRACE4 TraceLog4

#define TRACE5 TraceLog5

#define TRACE6 TraceLog6

// #else					//���а�
// 
// #define TRACE(level, args) //TRACE����Ϊ��
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
//������CDebugTrace��Ķ���
#define DEF_TRACE_BUFF_LEN 4096
class CDebugTrace
{
private:
	long		m_lDataLen;			 //���ݳ���
	char		mszPrintBuff[DEF_TRACE_BUFF_LEN];	 //��ӡ���ݻ���

	CCriticalSection m_oCriticalSection;     //�ٽ���
private:
	static int		   mnLogLevel;			 //��־�ȼ�
	static char	       mszLogFileName[512]; //��־�ļ�����
	static unsigned    muTraceOptions;		 //��ӡ��־ѡ��	

public:
	//��ӡѡ��
	enum Options 
	{
		/// ��ӡʱ��
		Timestamp = 1,
		/// ��ӡ��־����
		LogLevel = 2,
		/// ��ӡԴ�ļ������к�
		FileAndLine = 4,
		/// ����־׷�ӵ��ļ���
		AppendToFile = 8,
		///�����־������̨
		PrintToConsole = 16
	};    

	//���캯��	
	CDebugTrace();
	//��������
	~CDebugTrace();
public:
	//������־����(0�����,1���,��������,С�ڸõȼ�����־����ӡ)
	static void SetTraceLevel(int aiTraceLevel);

#ifdef UNICODE
	//������־�ļ���
	static void SetLogFileName(const TCHAR *aszLogFile);
#else
	//������־�ļ���
	static void SetLogFileName(const char *aszLogFile);
#endif

	// ����TRACEѡ�� .ע�⺯������ OR ��ѡ��
	static void SetTraceOptions(unsigned options /** New level for trace */ );

	//ȡ��TRACEѡ��
	static unsigned GetTraceOptions(void);

	//�жϸ��������Ƿ���Դ�ӡ
	static bool CanTrace(int aiLogLevel);					

	//��ʼ��ӡ
	static CDebugTrace& BeginTrace(int aiLogLevel,const char *apSrcFile,int aiSrcLine);	

	//������ӡ
	static void EndTrace(CDebugTrace &aoDebugTrace);

	//����ʧ�ܴ�����
	static void AssertFail(const char * strCond,char *strFile, unsigned uLine);

public:
	//���պ���printf�����Ƹ�ʽ��ӡ��־
	void TraceFormat(const char * pFmt,...);	

	//���·ֱ����������������
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
