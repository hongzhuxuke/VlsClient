#pragma once
#include <thread>
#include <windef.h>
#include <list>
#include <map>
#include <atomic>

class WPcap
{
public:
    WPcap();
    ~WPcap();
    void StartCaptureThread();
    void StopCaptureThread();

    void ProcessTask(std::string name, std::map<std::string, std::string> macAddrs);
    static DWORD WINAPI ThreadProcess(LPVOID p, std::string name, std::map<std::string, std::string>);

    static DWORD WINAPI DevCheckThreadProcess(LPVOID p);
    void ProcessDevChanged();

    void GetSendAndRecvDatLen(int& recv, int& send);

    bool IsNetDevChanged();

    
private:
    void getAllAdapterInfo();
    void initNetDev();
    bool GetMacByGetAdaptersAddresses(std::map<std::string, std::string>& macOUT);
private:
    std::list<std::string> mCurIplist;
    std::list<std::string> mWcapDevList;
    std::list<std::thread*> mpCaptureThreadList;
    std::map<std::string, std::string> mNetMacMap;
    std::thread* mDevCheckThread = nullptr;
    std::atomic_bool mIsDevChanged = false;

};

