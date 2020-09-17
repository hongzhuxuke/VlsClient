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
    // ������Ϣ
    void DealMessage(DWORD adwMessageID, void* apData, DWORD adwLen);

public:
    // ����Click����
    void DealClickControl(void* apData, DWORD adwLen);
    // �����޸��ı�
    void DealModifyText(void* apData, DWORD adwLen);
private:
    void CreateTextUI();
    //�������
    void AddText();

private:
    TextSettingDlg* m_pTextSettingDlg;
};

#endif //__TEXT_SETTING_LOGIC__H_INCLUDE__