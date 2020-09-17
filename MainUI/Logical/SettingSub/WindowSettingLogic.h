#ifndef __WINDOW_SETTING_LOGIC__H_INCLUDE__
#define __WINDOW_SETTING_LOGIC__H_INCLUDE__

#pragma once

#include <string>
#include "ImageSettingDlg.h"
using namespace std;


typedef std::vector<std::wstring> StringVector;

class WindowSettingDlg;
class WindowSettingLogic {
public:
   WindowSettingLogic(void);
   ~WindowSettingLogic(void);

public:
   BOOL Create();
   void Destroy();

   // 处理消息
   void DealMessage(DWORD adwMessageID, void* apData, DWORD adwLen);

   // 处理Click控制
   void DealClickControl(void* apData, DWORD adwLen);

   // 处理添加软件源
   void DealAddWindowSrc(void* apData, DWORD adwLen);

   // 处理修改图片
   void DealModifyImage(void* apData, DWORD adwLen);
private:
   // 添加软件源
   void AddWindowSrc();

   //处理图片
   void ProcessImages(DWORD dwType);         //dwType 1添加 2修改
public slots:
   void slotReFlush();
private:
   ImageSettingDlg* m_pImageSettingDlg;
   WindowSettingDlg* m_pWindowSettingDlg;
};

#endif //__WINDOW_SETTING_LOGIC__H_INCLUDE__
