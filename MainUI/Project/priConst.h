#ifndef H_PRICONST_H
#define H_PRICONST_H

enum eHandsUpState
{
    eHandsUpState_No = 0,
    eHandsUpState_existence
};

typedef enum VhallInteractionRenderType_e {
	RenderType_Video = 0,//Ĭ�� ��Ƶ����������ͷ�����湲��
	RenderType_Wait,//�ȴ�
	RenderType_Finished,//����
	RenderType_MasterExit,//�������˳�
	RenderType_Screen,//���湲��
	RenderType_Photo,//��Ƭ
}VhallInteractionRenderType;

//�û��б�tabҳ
enum eTabMember
{
	eTabMember_onLine = 0,   // ����
	eTabMember_raiseHands,      // ����
	eTabMember_limit,      //����
   eTabMember_SearchPage, //����
};

enum eWebOpType
{
	eWebOpType_questionnaire = 100012,   //  �ʾ�     
	eWebOpType_MemberManager = 100013,   //  ��Աģ��
	eWebOpType_Luck = 100014,   //  �齱    
	eWebOpType_Questions = 100016,   //  �ʴ�      
	eWebOpType_envelopes = 100018,   //  ���      
	eWebOpType_PublicNotice = 100019,   //  ����ģ��
	eWebOpType_Signin = 100022,   //  ǩ��      

  /*  100001 = > "logo����ʾ��¼ע��",
	100002 = > "ֱ������",
	100003 = > "���������΢��ͼ��",
	100004 = > "ֻ���������˹�ѡ���������������ã�", (���ϻ���ֱ���ѷ���)
	100005 = > "����",
	100008 = > "�����б�",
	100009 = > "�����û�����",
	100010 = > "���ֿ���",
	100011 = > "�ĵ�ģ��",
	100012 = > "�ʾ�ģ��",
	100013 = > "��Աģ��",
	100014 = > "�齱ģ��",
	100015 = > "�������ģ��",
	100016 = > "�ʴ�ģ��",
	100017 = > "ȫ�����ģ��",
	100018 = > "���ģ��",
	100019 = > "����ģ��",
	100020 = > "¼��",
	100021 = > "����ֱ��",
	100022 = > "ǩ��ģ��",
	100023 = > "����������",
	100024 = > "�����б�",
	100025 = > "����",
	100026 = > "�����б�",
	100027 = > "ת��ģ��"*/
};


//const int PageSpanNum = 5;
#endif//H_PRICONST_H