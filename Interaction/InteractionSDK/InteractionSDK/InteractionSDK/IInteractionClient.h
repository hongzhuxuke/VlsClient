#ifndef _IMESSAGECLIENT_H_
#define _IMESSAGECLIENT_H_

#ifdef  VHALL_EXPORT
#define VHALL_EXPORT     __declspec(dllimport)
#else
#define VHALL_EXPORT     __declspec(dllexport)
#endif
#include <QString>
#include <vector>
#include <map>
#include <functional>
#include "vhalluserinfomanager.h"

using namespace std;

//��Ϣ��������
enum MsgRQType {
   //�������
   //��������
   e_RQ_None = 0,
   e_RQ_UserOnlineList,
   e_RQ_UserProhibitSpeakList,
   e_RQ_UserKickOutList,
   e_RQ_UserSpecialList,                 //�����ˡ��α�������
   e_RQ_UserProhibitSpeakAllUser, //ȫ�����
   e_RQ_UserAllowSpeakAllUser, //ȡ��ȫ�����
   e_RQ_ChatEmotionList,
   e_RQ_SearchOnLineMember,      //�������߳�Ա

   //������
   e_RQ_UserChat,
   e_RQ_UserProhibitSpeakOneUser,//����
   e_RQ_UserAllowSpeakOneUser,//ȡ������
   e_RQ_UserKickOutOneUser,   //����
   e_RQ_UserAllowJoinOneUser, //ȡ������
   e_RQ_ReleaseAnnouncement,

   //֪ͨ�������ߣ�
   e_Ntf_UserOnline,
   e_Ntf_UserOffline,
   //�ĵ����
   e_RQ_flashMsg,//Flash��ز���
   e_RQ_switchHandsup_open,//���ֿ���-��
   e_RQ_switchHandsup_close,//���ֿ���-��
   e_RQ_handsUp,//����
   e_RQ_CancelHandsUp,//ȡ������
   e_RQ_setDefinition,//ȡ������
   e_RQ_replyInvite_Suc,//�ظ�����
   e_RQ_replyInvite_Fail,//�ظ�����
   e_RQ_replyInvite_Error,//�ظ�����

   e_RQ_switchDevice_close,//�豸����-�ر�
   e_RQ_switchDevice_open,//�豸����-��
   e_RQ_notSpeak,//����
   e_RQ_sendInvite,//��������
   e_RQ_addSpeaker,//����
   e_RQ_agreeSpeaker,//ͬ����������
   e_RQ_setMainShow,//����������
   e_RQ_setMainSpeaker,//����������
   e_RQ_setPublishStart,
   e_RQ_WhiteBoard_open,//��/�رհװ�
   e_RQ_RejectSpeak,   //�ܾ���������
   e_RQ_setOver,
   e_RQ_question_answer_open, //�����ʴ�
};

//key���ݣ���ʼ��SDK����
struct Key_Data {
   //�ID
   long m_lRoomID;
   //�û�ID
   long m_lUserID;
   // msg_token
   WCHAR m_szMsgToken[1024];
   //�û���ɫ,δ��
   char m_szRole[256];
   //δ��
   char m_szProxyURL[1024];

   //msg��ַ,��"ws://msg01.t.vhall.com/socket.io/"
   WCHAR m_msgURL[1024];
   //msg host����"msg01.t.vhall.com"
   WCHAR m_msgHOST[1024];
   //msg app����"vhall"
   WCHAR m_msgAPP[256];
   //chat �˿�
   int m_chatPORT;
   //chat ��ַ����"chat01.t.vhall.com"
   WCHAR m_chatURL[1024];
   //msg server url����"http://t.vhall.com/api/client/v1/clientapi/"
   WCHAR m_MSG_SERVER_URL[1024];
   //chat server url,��"http://api.t.vhall.com/pub/index"
   WCHAR m_CHAT_SERVER_URL[1024];

   char m_proxy_ip[256];
   char m_proxy_username[256];
   char m_proxy_password[256];
   char m_proxy_port[32];
   char m_domain[256];

   Key_Data() {
      memset(this, 0, sizeof(this));
   }
};

//�û��б�
typedef std::vector<VhallAudienceUserInfo> UserList;


typedef enum selectType {
   SELECT_FROM_NONE = -1,
   SELECT_FROM_RENDER_WND = 0,   //����Ⱦ�����Ҽ���
   SELECT_FROM_USER_LIST = 1,    //�ӳ�Ա�б��Ҽ���
}SELECT_TYPE;        

struct SelectUserInfo {
public:
   VhallAudienceUserInfo userInfo;
   int selectType;
   int renderWndType;
};

//��������(����������ʹ��)
struct RQData {
   MsgRQType m_eMsgType;

   //�û��б�,��ǰҳ(����)
   int m_iCurPage;

   //�û���Ϣ(�û�������)
   VhallAudienceUserInfo m_oUserInfo;

   // ���棬����
   char m_wzText[10240];

   RQData() {
      m_eMsgType = e_RQ_None;
      memset(m_wzText, 0, sizeof(m_wzText));
   }
};

typedef map<string, string> ImageList;

enum eventType {
   e_eventType_none = 0,
   e_eventType_opened,
   e_eventType_msg,
   e_eventType_closed,
};


enum eDevice
{
	eDevice_Mic = 0, //���
	eDevice_Camera //����ͷ
};

enum eJesinDevice
{
	eJesinDevice_eDevice_Mic = 1,//���
	eJesinDevice_Camera = 2 //����ͷ
};
//֪ͨ�¼�
struct Event {
   long m_lRoomID;
   MsgRQType m_eMsgType;

   //�û��б�,��ҳ��(����)
   int m_iPageCount;
   int m_currentPage;
   int m_sumNumber;
   int m_bSendMsg;
   int m_memberSlider = 0;

   //�¼����
   bool m_bResult;

   //�û��б�
   UserList m_oUserList;

   //�û���Ϣ(�û�������)
   VhallAudienceUserInfo m_oUserInfo;
   bool m_isGag;
   bool m_bAudioState;
   bool m_bVideoState;
   bool m_bIsRtcPublisher;
   // ���棬����,�װ����״̬
   char m_wzText[2048];

   // ����
   ImageList m_oImageList;
   eventType  m_eventType;
   eDevice meDevice;
   QString context;
   QString pushTime;
   Event() {
	    m_bAudioState = false;
	    m_bVideoState = false;
       m_bIsRtcPublisher = false;
        m_bSendMsg = 0;
        m_isGag = false;
        m_eMsgType = e_RQ_None;
        m_eventType = e_eventType_none;
        m_oUserList.clear();
        m_oImageList.clear();
        memset(&m_oUserInfo, 0, sizeof(m_oUserInfo));
        memset(m_wzText, 0, sizeof(m_wzText));
   }

   void Init()
   {
	   m_iPageCount = 0;
	   m_currentPage = 0;
	   m_sumNumber = 0;
      m_bSendMsg = 0;
	   m_isGag = false;
	   m_eMsgType = e_RQ_None;
	   m_eventType = e_eventType_none;
	   m_oUserList.clear();
	   m_oImageList.clear();
	   memset(m_wzText, 0, sizeof(m_wzText));
   }
};

class Delegate {
public:
   virtual ~Delegate() {}

   virtual void onConnected(Event* event) = 0;

   virtual void onMessaged(Event* event) = 0;

   //virtual void onDisconnected(Event* event) = 0;

   virtual void onError(Event* event) = 0;
};

typedef std::function<void (char *)> InteractionMsgCallBack;
//�����ͻ��˽ӿ�
class VHALL_EXPORT IInteractionClient {
public:

   virtual ~IInteractionClient(){};

   //��ʼ��
   virtual BOOL Init(Delegate* pDelegate, Key_Data& oKeyData) = 0;

   virtual void RegisterDelegate(Delegate* pDelegate) = 0;

   //����ʼ��
   virtual void UnInit() = 0;

   //���ӷ�����()
   virtual void ConnectServer() = 0;
   //�Ͽ�����
   virtual void DisConnect() = 0;

   //��������
   virtual void MessageRQ(const MsgRQType&  eMsgType, RQData* vData = NULL) = 0;

   //socketIO �ص�
   virtual void SetMessageSocketIOCallBack(InteractionMsgCallBack) = 0;

   //websocket �ص�
   virtual void SetMessageWebSocketCallBack(InteractionMsgCallBack) = 0;

   virtual void SetProxyAddr(const bool&  enable, const char *proxy_ip, const char* proxy_port, const char *proxy_username, const char *proxy_password) = 0;

   virtual void OnHttpMsg(const QString& cmd, const QString& value) = 0;
};

VHALL_EXPORT IInteractionClient* CreateInteractionClient();
VHALL_EXPORT  void DestroyInteractionClient(IInteractionClient** interactionClient);
VHALL_EXPORT IInteractionClient* CreateAliveInteractionClient();
VHALL_EXPORT void DestroyAliveInteractionClient(IInteractionClient** interactionClient);
#endif
