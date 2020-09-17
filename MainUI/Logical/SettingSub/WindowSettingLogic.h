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

   // ������Ϣ
   void DealMessage(DWORD adwMessageID, void* apData, DWORD adwLen);

   // ����Click����
   void DealClickControl(void* apData, DWORD adwLen);

   // ����������Դ
   void DealAddWindowSrc(void* apData, DWORD adwLen);

   // �����޸�ͼƬ
   void DealModifyImage(void* apData, DWORD adwLen);
private:
   // ������Դ
   void AddWindowSrc();

   //����ͼƬ
   void ProcessImages(DWORD dwType);         //dwType 1��� 2�޸�
public slots:
   void slotReFlush();
private:
   ImageSettingDlg* m_pImageSettingDlg;
   WindowSettingDlg* m_pWindowSettingDlg;
};

#endif //__WINDOW_SETTING_LOGIC__H_INCLUDE__
