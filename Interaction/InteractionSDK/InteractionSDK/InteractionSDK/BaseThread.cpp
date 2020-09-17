#include "stdafx.h"
#include "BaseThread.h"
#include "DebugTrace.h"

#include <sys/types.h> 
#include <sys/timeb.h>

#ifdef DEF_USE_MEDIA_TIMER
#include <MMSystem.h>
#pragma comment(lib, "Winmm.lib")
#endif //DEF_USE_MEDIA_TIMER

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBaseThread::CBaseThread()
{

}

CBaseThread::~CBaseThread()
{

}

BOOL CBaseThread::BeginThread(unsigned int (*apStartFunc)(void *),void *apParam)
{
    STRU_THREAD_INFO *lpThreadInfo = NULL;
    lpThreadInfo = new struct STRU_THREAD_INFO;
    if (NULL == lpThreadInfo)
    {
        return FALSE;
    }
    lpThreadInfo->mpUserParam = apParam;
    lpThreadInfo->mpThreadFunc = apStartFunc;

#ifdef WIN32

    unsigned int luThreadID = 0;
    m_hThreadHandle = _beginthreadex(NULL,0,Win32ThreadFunc,lpThreadInfo,0,&luThreadID);
    if (-1 == m_hThreadHandle)
    {
        return FALSE;
    }

    TRACE4("CBaseThread::BeginThread, luThreadID = %u !\n", luThreadID);

    CloseHandle((HANDLE)m_hThreadHandle);
    return TRUE;

#else
	pthread_attr_t attr;
	pthread_attr_init(&attr);
    pthread_attr_setscope(&attr,PTHREAD_SCOPE_SYSTEM);
    pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
	pthread_create(&m_thread,&attr,LinuxThreadFunc,lpThreadInfo);
    return TRUE;

#endif
}

#ifdef WIN32
unsigned int CBaseThread::Win32ThreadFunc(void * lpParam)
{
    STRU_THREAD_INFO *lpThreadInfo = (STRU_THREAD_INFO *)lpParam;

    ASSERT(lpThreadInfo != NULL);
    ASSERT(lpThreadInfo->mpThreadFunc != NULL);

	lpThreadInfo->mpThreadFunc(lpThreadInfo->mpUserParam);

	if (lpThreadInfo != NULL)
	{
		delete lpThreadInfo;
		lpThreadInfo = NULL;
	}
    _endthreadex(0);
    return 1;
}

#else

void * CBaseThread::LinuxThreadFunc(void * lpParam)
{
    struct STRU_THREAD_INFO *lpThreadInfo = (struct STRU_THREAD_INFO *)lpParam;

    ASSERT(lpThreadInfo != NULL);
    ASSERT(lpThreadInfo->mpThreadFunc != NULL);

    lpThreadInfo->mpThreadFunc(lpThreadInfo->mpUserParam);

	if (lpThreadInfo != NULL)
	{
		delete lpThreadInfo;
		lpThreadInfo = NULL;
	}
    return NULL;
}
#endif

#ifdef DEF_USE_MEDIA_TIMER
void WINAPI OnTimeEvent(UINT auTimerID, UINT auMsg, DWORD dwUser, DWORD dwl,DWORD dw2)
{
	HANDLE hEvnet = (HANDLE)dwUser;
	if (hEvnet)
	{
		SetEvent(hEvnet);
	}
	timeKillEvent(auTimerID);
}
#endif

//����
void CBaseThread::Sleep(DWORD dwMilliseconds)
{
#ifdef _WIN32
#ifndef DEF_USE_MEDIA_TIMER
    ::Sleep(dwMilliseconds);
#else
	HANDLE hSleepEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (NULL != hSleepEvent)
	{
		timeSetEvent(dwMilliseconds, 1, OnTimeEvent, (DWORD_PTR)hSleepEvent, TIME_ONESHOT);

		WaitForSingleObject(hSleepEvent, dwMilliseconds);
		CloseHandle(hSleepEvent);
	}
	else
	{
		::Sleep(dwMilliseconds);
	}

#endif //DEF_USE_MEDIA_TIMER

#else
	DWORD ldwSleep = 1000*dwMilliseconds;
  	usleep(ldwSleep);
#endif	//_WIN32
}


INT64 CBaseThread::GetSystemTime()
{
    struct timeb loTimeb;
    //memset(&loTimeb, 0 , sizeof(timeb));
    ftime(&loTimeb);
    return ((INT64)loTimeb.time * 1000) + loTimeb.millitm;
}

