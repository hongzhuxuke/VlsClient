#include "stdafx.h"
#include "ConfigSetting.h"
#include "pathManager.h"

#pragma comment (lib, "Version.lib")

string gStreamName = "549362291";
vector<RECT> gMonitor;

#define OUTPUTINFOINDEX
#define OUTPUTINFOINDEX_360P     0
#define OUTPUTINFOINDEX_540P     1
#define OUTPUTINFOINDEX_720P     2
#define OUTPUTINFOINDEX_1080P    3
#define OUTPUTINFOINDEX_AUTO     4

OBSOutPutInfo GOutputInfo[]={
   { { 1280, 720 }, { 640, 360 },  KBPS_360P, false },
   { { 1920, 1080 }, { 960, 540 },  KBPS_540P, false },
   { { 1280, 720 }, { 1280, 720 },  KBPS_720P, false },
   { { 1920, 1080 }, { 1920, 1080 },  KBPS_1080P, false },
   //Ô¤Áô×Ô¶¨Òå
   { { 1280, 720 }, { 1280, 720 },  KBPS_1080P, true }
};

SIZE gBaseSize = { 1920, 1080 };
float gDownScale[4] = { 3.0f, 2.0f, 1.5f ,1.0f};
bool gEnable1080p=false;
bool gIsHideLogo = false;

BOOL CALLBACK MonitorInfoEnumProc(HMONITOR, HDC, LPRECT lprcMonitor, vector<RECT>& monitor) {
   monitor.push_back(*lprcMonitor);
   return TRUE;
}

static DWORD CountSetBits(ULONG_PTR bitMask){
    DWORD LSHIFT = sizeof(ULONG_PTR)*8 - 1;
    DWORD bitSetCount = 0;
    ULONG_PTR bitTest = (ULONG_PTR)1 << LSHIFT;    
    DWORD i;

    for (i = 0; i <= LSHIFT; ++i){
        bitSetCount += ((bitMask & bitTest)?1:0);
        bitTest/=2;
    }

    return bitSetCount;
}

void GetProcessor(int&coreCount, int& logicalCores, DWORD& dwProcessorSpeed) {
   dwProcessorSpeed = 1000; //default 
   coreCount = 1;
   logicalCores = 1;
   PSYSTEM_LOGICAL_PROCESSOR_INFORMATION pInfo = NULL, pTemp = NULL;
   DWORD dwLen = 0;
   if (!GetLogicalProcessorInformation(pInfo, &dwLen)) {
      if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
         pInfo = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION)malloc(dwLen);
         if (GetLogicalProcessorInformation(pInfo, &dwLen)) {
            pTemp = pInfo;
            DWORD dwNum = dwLen / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);
            coreCount = 0;
            logicalCores = 0;
            for (UINT i = 0; i < dwNum; i++) {
               if (pTemp->Relationship == RelationProcessorCore) {
                  coreCount++;
                  logicalCores += CountSetBits(pTemp->ProcessorMask);
               }
               pTemp++;
            }
         }
         free(pInfo);
      }
   }
   HKEY key;
   TCHAR data[1024];
   DWORD dwSize;
   ZeroMemory(data, 1024);
   if (RegOpenKey(HKEY_LOCAL_MACHINE, TEXT("HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0"), &key) != ERROR_SUCCESS) {
      TRACE6("Could not open system information registry key");
      return;
   }
   dwSize = 1024;
   RegQueryValueEx(key, TEXT("ProcessorNameString"), NULL, NULL, (LPBYTE)data, &dwSize);
   dwSize = 4;
   RegQueryValueEx(key, TEXT("~MHz"), NULL, NULL, (LPBYTE)&dwProcessorSpeed, &dwSize);
   TRACE6("CPU Speed: %dMHz", dwProcessorSpeed);
   RegCloseKey(key);
}

void Optimization() {
    gMonitor.clear();
    EnumDisplayMonitors(NULL, NULL, (MONITORENUMPROC)MonitorInfoEnumProc, (LPARAM)&gMonitor);
    initStreamBitrate();
    if (gMonitor.size() > 0) {
        SIZE tmpSize;
        tmpSize.cx = gMonitor[0].right - gMonitor[0].left;
        tmpSize.cy = gMonitor[0].bottom - gMonitor[0].top;
      
        SIZE monitorSize = {gMonitor[0].right - gMonitor[0].left,gMonitor[0].bottom - gMonitor[0].top};
        monitorSize.cx = monitorSize.cx & 0xFFFFFFFC;
        monitorSize.cy = monitorSize.cy & 0xFFFFFFFC;
        unsigned int vedioBit = KBPS_720P;

        //wstring confPath = GetAppPath() + CONFIGPATH;
        QString qsConfPath = CPathManager::GetConfigPath();
        QString customSizeString = ConfigSetting::ReadString(qsConfPath, GROUP_DEFAULT, KEY_CUSTOM_RESOLUTION, "");
        unsigned int customBit = ConfigSetting::ReadInt(qsConfPath, GROUP_DEFAULT, KEY_CUSTOM_BITRATE, KBPS_720P);
        bool useCustomSize = false;
        SIZE curtomSize;
        if(customSizeString!="") {
            QStringList sizeList=customSizeString.split("x");
            if(sizeList.count()==2) {
                int w= sizeList[0].toInt();
                int h= sizeList[1].toInt();
                if(w > 0 && h > 0) {
                    curtomSize.cx=w;
                    curtomSize.cy=h;
                    useCustomSize=true;
                    if(customBit<100) {
                        customBit=100;
                    }
                }
            }
        }

        if (tmpSize.cx >= 1920 && tmpSize.cy >= 1080) {
            gEnable1080p = true;
            //GOutputInfo[3] = { { 1920, 1080 }, { 1920, 1080 },  KBPS_1080P, false };
            if(useCustomSize) {
                customBit = KBPS_1080P;
                curtomSize.cx = 1920;
                curtomSize.cy = 1080;
                GOutputInfo[OUTPUTINFOINDEX_AUTO] = { curtomSize, curtomSize,  customBit, true };
            }
            else {
                vedioBit = KBPS_1080P;
                monitorSize.cx = 1920;
                monitorSize.cy = 1080;
                GOutputInfo[OUTPUTINFOINDEX_AUTO] = { monitorSize, monitorSize,  vedioBit, true };
            }
        } else {
            vedioBit =  ConfigSetting::GetAutoBitRate(monitorSize.cx, monitorSize.cy);
            GOutputInfo[3] = { monitorSize, monitorSize,  vedioBit, true };
        }
    }

    int coreCount;
    int logicalCores;
    DWORD dwProcessorSpeed;
    GetProcessor(coreCount, logicalCores, dwProcessorSpeed);
}

bool GetIsEnabled1080p(){
   return gEnable1080p;
}

void initStreamBitrate(){
    QString qsConfPath = CPathManager::GetToolConfigPath();
    unsigned int ret= 0 ;
    ret = GOutputInfo[OUTPUTINFOINDEX_360P].videoBits = (long)ConfigSetting::ReadInt(qsConfPath, GROUP_DEFAULT, KEY_BITRATE_360P, GOutputInfo[OUTPUTINFOINDEX_360P].videoBits);
    ret = GOutputInfo[OUTPUTINFOINDEX_540P].videoBits = (long)ConfigSetting::ReadInt(qsConfPath, GROUP_DEFAULT, KEY_BITRATE_540P, GOutputInfo[OUTPUTINFOINDEX_540P].videoBits);
    ret = GOutputInfo[OUTPUTINFOINDEX_720P].videoBits = (long)ConfigSetting::ReadInt(qsConfPath, GROUP_DEFAULT, KEY_BITRATE_720P, GOutputInfo[OUTPUTINFOINDEX_720P].videoBits);
    ret = GOutputInfo[OUTPUTINFOINDEX_1080P].videoBits = (long)ConfigSetting::ReadInt(qsConfPath, GROUP_DEFAULT, KEY_BITRATE_1080P, GOutputInfo[OUTPUTINFOINDEX_1080P].videoBits);
    ret = GOutputInfo[OUTPUTINFOINDEX_AUTO].videoBits = (long)ConfigSetting::ReadInt(qsConfPath, GROUP_DEFAULT, KEY_BITRATE_AUDO, GOutputInfo[OUTPUTINFOINDEX_AUTO].videoBits);
    TRACE6("bitrate_360p=%lld, bitrate_540p=%lld, bitrate_720p=%lld, bitrate_1080p=%lld,auto=%lld",
        GOutputInfo[OUTPUTINFOINDEX_360P].videoBits ,
        GOutputInfo[OUTPUTINFOINDEX_540P].videoBits  ,
        GOutputInfo[OUTPUTINFOINDEX_720P].videoBits ,
        GOutputInfo[OUTPUTINFOINDEX_1080P].videoBits ,
        GOutputInfo[OUTPUTINFOINDEX_AUTO].videoBits);
}

