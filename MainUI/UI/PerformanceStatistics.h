#ifndef H_PERFORMANCESTATISTICS_H
#define H_PERFORMANCESTATISTICS_H

#include <Pdh.h>
#include <QObject>
#include <functional>
#include<thread>

class RealtimeData 
{
public:
    int	iCPUUsage;			// CPU使用率
    double	dbDownloadSpeed;		// 下载速率
    double	dbUploadSpeed;			// 上传速率
    int	iMemory;			// 可用内存(M)
    double	dbDiskRead;			// 磁盘读速率
    double	dbDiskWrite;			// 磁盘写速率
    INT64	time;				// 本次搜集的本机时间
};


typedef std::function<void(int cpu, int sendNetData, int recvNetData)> StatisticsDataFun;

class PerformanceStatistics : public QObject
{
    Q_OBJECT

public:
    PerformanceStatistics(QObject *parent = nullptr);
    ~PerformanceStatistics();

    void Start();
    void Stop();
   
    int GetCPUValue();
 
    static void ThreadProFun(void *);
    void ProcessFun();

private:
    void WorkFun();
    void ProcessCPU();
    void ProcessNetWork();

    void GetAdapters(std::vector<QString>&);
    void ProcessSelectAdapter();

    std::atomic_int32_t mCpuStatistics = 0;
    std::atomic_int64_t mNetworkSend = 0;
    std::atomic_int64_t mNetworkRecv = 0;

    std::thread* mProcessThread = nullptr;
    std::atomic_bool mbIsRuning = false;
};

#endif//H_PERFORMANCESTATISTICS_H