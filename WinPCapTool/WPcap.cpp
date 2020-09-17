#include "WPcap.h"
#include <pcap.h>
#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#include <Iphlpapi.h>
#include <WinSock2.h>
#include <QString>
#include <atomic>
#include <iostream>
#include <string>
#include "DebugTrace.h"
#pragma comment(lib,"ws2_32")
#pragma comment(lib,"Iphlpapi.lib") //需要Iphlpapi.lib库 ,devc++中 项目 -> 项目选项-> 参数 -> 添加链接库 libiphlpapi.a （D:\Program Files (x86)\Dev-Cpp\MinGW64\x86_64-w64-mingw32\lib\libiphlpapi.a） 

static int ADAPTERNUM = 10;
static int sCaptureLen;

static std::atomic_int sSendCountLen = 0;
static std::atomic_int sRecvCountLen = 0;
static std::atomic_bool isExitCapture = false;
static std::atomic_bool isExitCheckCapture = false;

using namespace std;


typedef struct sockaddr_in sockad;

/* 从tcptraceroute数字IP地址转换为字符串 */
#define IPTOSBUFFERS    12
char *iptos(u_long in)
{
   static char output[IPTOSBUFFERS][3 * 4 + 3 + 1];
   static short which;
   u_char *p;

   p = (u_char *)&in;
   which = (which + 1 == IPTOSBUFFERS ? 0 : which + 1);
   sprintf(output[which], "%d.%d.%d.%d", p[0], p[1], p[2], p[3]);
   return output[which];
}


#define Ethernet_IPv4 0x0800
#define Ethernet_ARP 0x0806

//Ethernet帧 size:14(Ethernet II)
struct EthernetHeader
{
   BYTE byDestMac[6];
   BYTE bySrcMac[6];
   USHORT usType;
};

//TCP 头信息 size:20
struct TCPHeader {
   USHORT m_sSourPort;   // 源端口号16bit
   USHORT m_sDestPort;   // 目的端口号16bit
   ULONG m_uiSequNum;// 序列号32bit
   ULONG m_uiAcknowledgeNum;  // 确认号32bit
   USHORT m_sHeaderLenAndFlag;// 前4位：TCP头长度；中6位：保留；后6位：标志位
   USHORT m_sWindowSize;// 窗口大小16bit
   USHORT m_sCheckSum;// 检验和16bit
   USHORT m_surgentPointer;// 紧急数据偏移量16bit
};

//ARP 头信息
struct ARPHeader {
   USHORT usHardwareType;//Ethernet(1)
   USHORT ProtocolType;//IPv4(0x0800)
   BYTE byHardwareSize;
   BYTE byProtocolSize;
   BYTE byOpcode;//request(1)
   BYTE bySenderMAC[6];
   DWORD dwSenderIPaddress;
   BYTE byTargetMAC[6];
   DWORD dwTargetIPaddress;
};

/* 4字节的IP地址 */
typedef struct _IPAddress {
   u_char byte1;
   u_char byte2;
   u_char byte3;
   u_char byte4;
} IPAddress;

/* IPv4 首部 */
typedef struct _IPHeader {
   BYTE m_byVerHLen;     //4位版本+4位首部长度
   BYTE m_byTOS;                    //服务类型
   USHORT m_usTotalLen; //总长度
   USHORT m_usID; //标识
   USHORT m_usFlagFragOffset; //3位标志+13位片偏移
   BYTE m_byTTL; //TTL
   BYTE m_byProtocol; //协议
   USHORT m_usHChecksum; //首部检验和
   DWORD m_ulSrcIP; //源IP地址
   DWORD m_ulDestIP; //目的IP地址
}IPHeader;

/* IPv4 首部 */
typedef struct _IPHeader2 {
   BYTE m_byVerHLen;     //4位版本+4位首部长度
   BYTE m_byTOS;                    //服务类型
   USHORT m_usTotalLen; //总长度
   USHORT m_usID; //标识
   USHORT m_usFlagFragOffset; //3位标志+13位片偏移
   BYTE m_byTTL; //TTL
   BYTE m_byProtocol; //协议
   USHORT m_usHChecksum; //首部检验和
   IPAddress m_ulSrcIP; //源IP地址
   IPAddress m_ulDestIP; //目的IP地址
}IPHeader2;

/* UDP 首部*/
typedef struct _UDPHeader {
   USHORT src_port;          // 源端口(Source port) 
   USHORT dest_port;          // 目的端口(Destination port) 
   USHORT datalen;            // UDP数据包长度(Datagram length) 
   USHORT checksum;            // 校验和(Checksum) 
} UDPHeader;


WPcap::WPcap() {

}

WPcap::~WPcap() {
   StopCaptureThread();
}

void WPcap::ProcessDevChanged() {
   pcap_if_t *alldevs, *d;
   char errbuf[PCAP_ERRBUF_SIZE];
   int res = 0;
   res = pcap_findalldevs(&alldevs, errbuf);
   if (res == -1) {
      TRACE6("%s pcap_findalldevs err\n", __FUNCTION__);
      mIsDevChanged = false;
      return;
   }
   std::list<std::string> dev_list;
   for (d = alldevs; d; d = d->next) {
      if (d->name && mNetMacMap.size() > 0) {
         dev_list.push_back(std::string(d->name));
      }
   }
   pcap_freealldevs(alldevs);
   if (dev_list == mWcapDevList) {
      mIsDevChanged = false;
      return;
   }
   TRACE6("%s IsNetDevChanged\n", __FUNCTION__);
   mIsDevChanged = true;
}

bool WPcap::IsNetDevChanged() {
   if (mDevCheckThread) {
      mDevCheckThread->join();
      delete mDevCheckThread;
      mDevCheckThread = nullptr;
   }
   if (mDevCheckThread == nullptr) {
      isExitCheckCapture = false;
      mDevCheckThread = new std::thread(WPcap::DevCheckThreadProcess, this);
   }
   return mIsDevChanged;
}

void WPcap::StartCaptureThread() {

   TRACE6("%s\n", __FUNCTION__);
   isExitCapture = false;
   getAllAdapterInfo();
   initNetDev();
   TRACE6("%s end\n", __FUNCTION__);
}

void WPcap::StopCaptureThread() {
   TRACE6("%s\n", __FUNCTION__);
   isExitCapture = true;
   isExitCheckCapture = true;
   mIsDevChanged = false;
   if (mDevCheckThread) {
      mDevCheckThread->join();
      delete mDevCheckThread;
      mDevCheckThread = nullptr;
   }

   std::list<std::thread*>::iterator iter = mpCaptureThreadList.begin();
   if (mpCaptureThreadList.size() > 0) {
      while (iter != mpCaptureThreadList.end()) {
         (*iter)->join();
         delete (*iter);
         iter++;
      }
   }
   mWcapDevList.clear();
   mpCaptureThreadList.clear();
   TRACE6("%s end\n", __FUNCTION__);
}

void WPcap::initNetDev() {
   mpCaptureThreadList.clear();
   pcap_if_t *alldevs, *d;
   char errbuf[PCAP_ERRBUF_SIZE];
   int res = 0;
   //TRACE6("%s\n", __FUNCTION__);
   /* The user didn't provide a packet source: Retrieve the local device list */
   res = pcap_findalldevs(&alldevs, errbuf);
   //TRACE6("%s res %d\n", __FUNCTION__, res);
   if (res == -1) {
      TRACE6("%s pcap_findalldevs err\n", __FUNCTION__);
      return;
   }

   bool bfind = false;
   for (d = alldevs; d; d = d->next) {
      bfind = true;
      if(d->name) {
         TRACE6("%s make thread devName %s \n", __FUNCTION__, d->name);
      }
      if (d->name && mNetMacMap.size() > 0) {

         mWcapDevList.push_back(std::string(d->name));
         std::thread* pthread = new std::thread(WPcap::ThreadProcess, this, std::string(d->name), mNetMacMap);
         mpCaptureThreadList.push_back(pthread);
      }
   }
   TRACE6("%s bfind :%d\n", __FUNCTION__, bfind);
   pcap_freealldevs(alldevs);
}

DWORD WINAPI WPcap::DevCheckThreadProcess(LPVOID p) {
   WPcap* sdk = (WPcap*)(p);
   if (sdk) {
      sdk->ProcessDevChanged();
   }
   return 0;
}

DWORD WINAPI WPcap::ThreadProcess(LPVOID p, std::string name, std::map<std::string, std::string> macAddrs) {
   while (!isExitCapture) {
      WPcap* sdk = (WPcap*)(p);
      if (sdk) {
         sdk->ProcessTask(name, macAddrs);
      }
   }
   return 0;
}

void WPcap::GetSendAndRecvDatLen(int& recv, int& send) {
   recv = sRecvCountLen;
   send = sSendCountLen;
   sRecvCountLen = 0;
   sSendCountLen = 0;
}

void WPcap::ProcessTask(std::string devName, std::map<std::string, std::string> macAddrs) {
   if (devName.length() == 0) {
      return;
   }
   pcap_t *fp;
   char errbuf[PCAP_ERRBUF_SIZE];
   int res;
   struct pcap_pkthdr *header = nullptr;
   const u_char *pkt_data = nullptr;

   /* Open the adapter */  //打开网络端口
   if ((fp = pcap_open_live(devName.c_str(),	// name of the device
      65536,							// portion of the packet to capture. 
                                      // 65536 grants that the whole packet will be captured on all the MACs.
      0,								// promiscuous mode (nonzero means promiscuous)
      1000,							// read timeout
      errbuf							// error buffer
   )) == NULL) {
      Sleep(1000);
      TRACE6("%s err devName :%s\n", __FUNCTION__, devName.c_str());
      return;
   }

   if (fp == NULL) {
      Sleep(1000);
      TRACE6("%s err devName :%s\n", __FUNCTION__, devName.c_str());
      return;
   }
   TRACE6("%s success devName :%s\n", __FUNCTION__, devName.c_str());
   HANDLE gTaskEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
   //开始读取获取的数据包
   /* Read the packets */
   while ((res = pcap_next_ex(fp, &header, &pkt_data)) >= 0 && !isExitCapture) {
      //DWORD ret = WaitForSingleObject(gTaskEvent, 100);
      if (res == 0) {
         /* Timeout elapsed */
         continue;
      }
      else if (res == 1 && pkt_data != nullptr && header != nullptr) {
         EthernetHeader *ethernet = (EthernetHeader*)pkt_data;
         char srcMAC[32];
         sprintf(srcMAC, "%02X-%02X-%02X-%02X-%02X-%02X",
            int(ethernet->bySrcMac[0]),
            int(ethernet->bySrcMac[1]),
            int(ethernet->bySrcMac[2]),
            int(ethernet->bySrcMac[3]),
            int(ethernet->bySrcMac[4]),
            int(ethernet->bySrcMac[5]));
         std::string macSrcAddr = srcMAC;

         char dstMAC[32];
         sprintf(dstMAC, "%02X-%02X-%02X-%02X-%02X-%02X",
            int(ethernet->byDestMac[0]),
            int(ethernet->byDestMac[1]),
            int(ethernet->byDestMac[2]),
            int(ethernet->byDestMac[3]),
            int(ethernet->byDestMac[4]),
            int(ethernet->byDestMac[5]));
         std::string macDstAddr = dstMAC;


         //从IP层开始读取数据！！偏移 14字节
         //14为以太网帧头部长度
         //获取原始的IP层协议
         IPHeader2* iphdr = (IPHeader2*)(pkt_data + sizeof(EthernetHeader));

         if (iphdr->m_byProtocol == 6 || iphdr->m_byProtocol == 17) {
            if (iphdr->m_byProtocol == 6) {
               sCaptureLen = header->caplen - sizeof(EthernetHeader) - sizeof(IPHeader2) - sizeof(TCPHeader);
            }
            else if (iphdr->m_byProtocol == 17) {
               sCaptureLen = header->caplen - sizeof(EthernetHeader) - sizeof(IPHeader2) - sizeof(_UDPHeader);
            }

            if (sCaptureLen > 0) {
               std::map<std::string, std::string>::iterator iter = macAddrs.begin();
               while (iter != macAddrs.end()) {
                  string curMac = iter->first;
                  if (curMac == macSrcAddr) {
                     sSendCountLen += sCaptureLen;
                  }
                  else if (curMac == macDstAddr) {
                     sRecvCountLen += sCaptureLen;
                  }
                  iter++;
               }
            }
         }
      }
   }

   //关闭指针
   if (fp) {
      pcap_close(fp);
   }
}

bool  WPcap::GetMacByGetAdaptersAddresses(std::map<std::string, std::string>& macOUT)
{
   bool ret = false;

   ULONG outBufLen = sizeof(IP_ADAPTER_ADDRESSES);
   PIP_ADAPTER_ADDRESSES pAddresses = (IP_ADAPTER_ADDRESSES*)malloc(outBufLen);
   if (pAddresses == NULL)
      return false;
   // Make an initial call to GetAdaptersAddresses to get the necessary size into the ulOutBufLen variable
   if (GetAdaptersAddresses(AF_UNSPEC, 0, NULL, pAddresses, &outBufLen) == ERROR_BUFFER_OVERFLOW)
   {
      free(pAddresses);
      pAddresses = (IP_ADAPTER_ADDRESSES*)malloc(outBufLen);
      if (pAddresses == NULL)
         return false;
   }

   if (GetAdaptersAddresses(AF_UNSPEC, 0, NULL, pAddresses, &outBufLen) == NO_ERROR)
   {
      // If successful, output some information from the data we received
      for (PIP_ADAPTER_ADDRESSES pCurrAddresses = pAddresses; pCurrAddresses != NULL; pCurrAddresses = pCurrAddresses->Next)
      {
         // 确保MAC地址的长度为 00-00-00-00-00-00
         if (pCurrAddresses->PhysicalAddressLength != 6)
            continue;
         char acMAC[32];
         sprintf(acMAC, "%02X-%02X-%02X-%02X-%02X-%02X",
            int(pCurrAddresses->PhysicalAddress[0]),
            int(pCurrAddresses->PhysicalAddress[1]),
            int(pCurrAddresses->PhysicalAddress[2]),
            int(pCurrAddresses->PhysicalAddress[3]),
            int(pCurrAddresses->PhysicalAddress[4]),
            int(pCurrAddresses->PhysicalAddress[5]));
         std::string macAddr = acMAC;
         macOUT.insert(std::pair<std::string, std::string>(macAddr, pCurrAddresses->AdapterName));
         TRACE6("%s macAddr:%s %ws AdapterName %s\n", __FUNCTION__, macAddr.c_str(), pCurrAddresses->Description, pCurrAddresses->AdapterName);
         ret = true;
      }
   }

   free(pAddresses);
   return ret;
}

void WPcap::getAllAdapterInfo() {
   mNetMacMap.clear();
   GetMacByGetAdaptersAddresses(mNetMacMap);
}