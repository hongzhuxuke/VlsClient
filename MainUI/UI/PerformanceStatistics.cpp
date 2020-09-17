#include "PerformanceStatistics.h"
#include <QDebug>
#include <Iphlpapi.h>
#include "DebugTrace.h"
#pragma comment(lib,"Iphlpapi.lib") //需要添加Iphlpapi.lib库
#pragma comment(lib,"Pdh.lib")

std::wstring String2WString(const std::string& str)
{
    int num = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
    wchar_t *wide = new wchar_t[num];
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, wide, num);
    std::wstring w_str(wide);
    delete[] wide;
    return w_str;
}


unsigned long long FileTimeSub(FILETIME ftEndTime, FILETIME ftStartTime)
{
    unsigned long long nDeltaTime;

    unsigned long long nEndTime = (unsigned long long)ftEndTime.dwHighDateTime << 32 | ftEndTime.dwLowDateTime;
    unsigned long long nStartTime = (unsigned long long)ftStartTime.dwHighDateTime << 32 | ftStartTime.dwLowDateTime;

    nDeltaTime = nEndTime - nStartTime;

    return nDeltaTime;
}

unsigned long long GetCPUUsageRate()
{
    FILETIME ftStartIdleTime, ftStartKernelTime, ftStartUserTime;
    FILETIME ftEndIdleTime, ftEndKernelTime, ftEndUserTime;

    GetSystemTimes(&ftStartIdleTime, &ftStartKernelTime, &ftStartUserTime);
    Sleep(1000);
    GetSystemTimes(&ftEndIdleTime, &ftEndKernelTime, &ftEndUserTime);

    unsigned long long nDeltaIdleTime = FileTimeSub(ftEndIdleTime, ftStartIdleTime);
    unsigned long long nDeltaKernelTime = FileTimeSub(ftEndKernelTime, ftStartKernelTime);
    unsigned long long nDeltaUserTime = FileTimeSub(ftEndUserTime, ftStartUserTime);

    //qDebug() << "kernel time: " << nDeltaKernelTime << endl;
    //qDebug() << "user time:   " << nDeltaUserTime << endl;
    //qDebug() << "idle time:   " << nDeltaIdleTime << endl;

    if (nDeltaKernelTime + nDeltaUserTime == 0){
        return 0;
    }

    unsigned long long nCPUUsageRate = ((nDeltaKernelTime + nDeltaUserTime - nDeltaIdleTime) * 100) / (nDeltaKernelTime + nDeltaUserTime);
    return nCPUUsageRate;
}

PerformanceStatistics::PerformanceStatistics(QObject *parent)
    :QObject(parent)
{
}

PerformanceStatistics::~PerformanceStatistics()
{
}


int PerformanceStatistics::GetCPUValue() {
   return mCpuStatistics;
}

void PerformanceStatistics::Start() {
    TRACE6("%s\n", __FUNCTION__);
    if (mProcessThread == nullptr) {
        mbIsRuning = true;
        mProcessThread = new std::thread(ThreadProFun,this);
        TRACE6("%s  new thread\n", __FUNCTION__);
    }
    TRACE6("%s end\n", __FUNCTION__);
}

void PerformanceStatistics::Stop() {
    TRACE6("%s \n", __FUNCTION__);
    if (mProcessThread) {
        mbIsRuning = false;
        TRACE6("%s join\n", __FUNCTION__);
        mProcessThread->join();
        TRACE6("%s join end\n", __FUNCTION__);
        delete mProcessThread;
        mProcessThread = nullptr;
    }
    TRACE6("%s end\n", __FUNCTION__);
}

void PerformanceStatistics::ThreadProFun(void *obj) {
    TRACE6("%s \n", __FUNCTION__);
    if (obj) {
        PerformanceStatistics *runObj = (PerformanceStatistics*)(obj);
        runObj->ProcessFun();
    }
    TRACE6("%s end\n", __FUNCTION__);
}

void PerformanceStatistics::ProcessFun() {
    TRACE6("%s \n", __FUNCTION__);
    while (mbIsRuning) {
        WorkFun();
    }
    TRACE6("%s end\n", __FUNCTION__);
}

void PerformanceStatistics::ProcessCPU() {
    mCpuStatistics = GetCPUUsageRate();
}

void PerformanceStatistics::ProcessNetWork() {
    mNetworkSend = 0;
    mNetworkRecv = 0;
    ProcessSelectAdapter();
}

void PerformanceStatistics::ProcessSelectAdapter() {
    HQUERY query = nullptr;
    double dbVal;
    long iVal;
    PDH_STATUS status = PdhOpenQuery(NULL, NULL, &query);
    if (ERROR_SUCCESS != status)
    {
        return;
    }

    std::vector<QString> adpters;
    GetAdapters(adpters);

    HCOUNTER cntNetDownload[64] = { nullptr };
    HCOUNTER cntNetUpload[64] = { nullptr };
    for (int i = 0; i < adpters.size(); i++) {
        QString strNetReveive, strNetSent;
        strNetReveive = QString("\\Network Interface(%1)\\Bytes Received/sec").arg(adpters.at(i));
        strNetSent = QString("\\Network Interface(%1)\\Bytes Sent/sec").arg(adpters.at(i));
        status = PdhAddCounter(query, String2WString(strNetReveive.toStdString()).c_str(), NULL, &cntNetDownload[i]);
        status = PdhAddCounter(query, String2WString(strNetSent.toStdString()).c_str(), NULL, &cntNetUpload[i]);
        if (ERROR_SUCCESS != status) {
            return;
        }
    }

    PdhCollectQueryData(query);
    Sleep(1000);              //这里要有延时不然结果相当不准确  
    PdhCollectQueryData(query);
    if (ERROR_SUCCESS != status){
        return;
    }

    for (int i = 0; i < adpters.size(); i++) {
        PDH_FMT_COUNTERVALUE pdhValue;
        DWORD dwValue;
        RealtimeData realtimeData;
        if (cntNetDownload[i]) {
            status = PdhGetFormattedCounterValue(cntNetDownload[i], PDH_FMT_LARGE, &dwValue, &pdhValue);
            if (ERROR_SUCCESS != status){
                //printf("性能计数器查询，获取数据失败，下载速率。");
            }
            else {
                dbVal = pdhValue.largeValue;
                realtimeData.dbDownloadSpeed = (double)((dbVal / (1024 * 1.0f)));
                mNetworkRecv += realtimeData.dbDownloadSpeed;                 
                qDebug() << adpters.at(i) << ": recv " << realtimeData.dbDownloadSpeed ;
            }
        }

        if (cntNetUpload[i]) {
            status = PdhGetFormattedCounterValue(cntNetUpload[i], PDH_FMT_LARGE, &dwValue, &pdhValue);
            if (ERROR_SUCCESS != status)
            {
                ///printf("性能计数器查询，获取数据失败，上传速率。");
            }
            else{
                dbVal = pdhValue.largeValue;
                realtimeData.dbUploadSpeed = (double)((dbVal / (1024 * 1.0f)));
                mNetworkSend += realtimeData.dbUploadSpeed;
                qDebug() << adpters.at(i) << ": send " << realtimeData.dbUploadSpeed;
            }
        }
    }

    qDebug() << " recv " << mNetworkRecv << "send" << mNetworkSend;

    for (int i = 0; i < adpters.size(); i++) {
        if (cntNetDownload[i]) {
            PdhRemoveCounter(cntNetDownload[i]);
        }
        if (cntNetUpload[i]) {
            PdhRemoveCounter(cntNetUpload[i]);
        }
    }
    if (query) {
        PdhCloseQuery(query);
    }
}

void PerformanceStatistics::WorkFun() {
    ProcessCPU();
    //ProcessNetWork();
}


void PerformanceStatistics::GetAdapters(std::vector<QString>& adpters) {
    IP_ADAPTER_INFO* pAdapterInfo = (IP_ADAPTER_INFO *)malloc(sizeof(IP_ADAPTER_INFO));
    ULONG ulOutBufLen = sizeof(IP_ADAPTER_INFO);

    // Make an initial call to GetAdaptersInfo to get the necessary size into the ulOutBufLen variable
    if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW)
    {
        free(pAdapterInfo);
        pAdapterInfo = (IP_ADAPTER_INFO *)malloc(ulOutBufLen);
    }

    if (::GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == ERROR_SUCCESS)
    {
        IP_ADAPTER_INFO* p = pAdapterInfo;
        while (p)
        {
            //WIN_Adapter na;
            //memcpy(na.mac, p->Address, 6);  // MAC地址
            //strcpy(na.name, p->AdapterName);
            //strcpy(na.description, p->Description);
            //strcpy(na.ip, p->IpAddressList.IpAddress.String);
            //strcpy(na.mask, p->IpAddressList.IpMask.String);
            //strcpy(na.gateway, p->GatewayList.IpAddress.String);
            QString AdapterDescription = QString(p->Description);
            adpters.push_back(AdapterDescription);
            p = p->Next;
        }
    }

    free(pAdapterInfo);
}
