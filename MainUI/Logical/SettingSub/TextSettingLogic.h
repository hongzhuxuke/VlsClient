#ifndef __TEXT_SETTING_LOGIC__H_INCLUDE__
#define __TEXT_SETTING_LOGIC__H_INCLUDE__

#pragma once

class TextSettingDlg;
class TextSettingLogic {
public:
   TextSettingLogic(void);
   ~TextSettingLogic(void);

public:
    BOOL Create();
    void Destroy();
    // 处理消息
    void DealMessage(DWORD adwMessageID, void* apData, DWORD adwLen);

public:
    // 处理Click控制
    void DealClickControl(void* apData, DWORD adwLen);
    // 处理修改文本
    void DealModifyText(void* apData, DWORD adwLen);
private:
    void CreateTextUI();
    //添加文字
    void AddText();

private:
    TextSettingDlg* m_pTextSettingDlg;
};

#endif //__TEXT_SETTING_LOGIC__H_INCLUDE__