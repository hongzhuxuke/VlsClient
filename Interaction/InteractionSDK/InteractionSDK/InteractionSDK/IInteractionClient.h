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

//消息请求类型
enum MsgRQType {
   //聊天相关
   //不带参数
   e_RQ_None = 0,
   e_RQ_UserOnlineList,
   e_RQ_UserProhibitSpeakList,
   e_RQ_UserKickOutList,
   e_RQ_UserSpecialList,                 //主持人、嘉宾、助理
   e_RQ_UserProhibitSpeakAllUser, //全体禁言
   e_RQ_UserAllowSpeakAllUser, //取消全体禁言
   e_RQ_ChatEmotionList,
   e_RQ_SearchOnLineMember,      //搜索在线成员

   //带参数
   e_RQ_UserChat,
   e_RQ_UserProhibitSpeakOneUser,//禁言
   e_RQ_UserAllowSpeakOneUser,//取消禁言
   e_RQ_UserKickOutOneUser,   //踢人
   e_RQ_UserAllowJoinOneUser, //取消踢人
   e_RQ_ReleaseAnnouncement,

   //通知（上下线）
   e_Ntf_UserOnline,
   e_Ntf_UserOffline,
   //文档相关
   e_RQ_flashMsg,//Flash相关操作
   e_RQ_switchHandsup_open,//举手开关-开
   e_RQ_switchHandsup_close,//举手开关-关
   e_RQ_handsUp,//举手
   e_RQ_CancelHandsUp,//取消举手
   e_RQ_setDefinition,//取消举手
   e_RQ_replyInvite_Suc,//回复邀请
   e_RQ_replyInvite_Fail,//回复邀请
   e_RQ_replyInvite_Error,//回复邀请

   e_RQ_switchDevice_close,//设备开关-关闭
   e_RQ_switchDevice_open,//设备开关-开
   e_RQ_notSpeak,//下麦
   e_RQ_sendInvite,//邀请上麦
   e_RQ_addSpeaker,//上麦
   e_RQ_agreeSpeaker,//同意申请上麦
   e_RQ_setMainShow,//设置主画面
   e_RQ_setMainSpeaker,//设置主讲人
   e_RQ_setPublishStart,
   e_RQ_WhiteBoard_open,//打开/关闭白板
   e_RQ_RejectSpeak,   //拒绝申请上麦
   e_RQ_setOver,
   e_RQ_question_answer_open, //开启问答
};

//key数据，初始化SDK传入
struct Key_Data {
   //活动ID
   long m_lRoomID;
   //用户ID
   long m_lUserID;
   // msg_token
   WCHAR m_szMsgToken[1024];
   //用户角色,未用
   char m_szRole[256];
   //未用
   char m_szProxyURL[1024];

   //msg地址,如"ws://msg01.t.vhall.com/socket.io/"
   WCHAR m_msgURL[1024];
   //msg host。如"msg01.t.vhall.com"
   WCHAR m_msgHOST[1024];
   //msg app。如"vhall"
   WCHAR m_msgAPP[256];
   //chat 端口
   int m_chatPORT;
   //chat 地址，如"chat01.t.vhall.com"
   WCHAR m_chatURL[1024];
   //msg server url，如"http://t.vhall.com/api/client/v1/clientapi/"
   WCHAR m_MSG_SERVER_URL[1024];
   //chat server url,如"http://api.t.vhall.com/pub/index"
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

//用户列表
typedef std::vector<VhallAudienceUserInfo> UserList;


typedef enum selectType {
   SELECT_FROM_NONE = -1,
   SELECT_FROM_RENDER_WND = 0,   //从渲染窗口右键。
   SELECT_FROM_USER_LIST = 1,    //从成员列表右键。
}SELECT_TYPE;        

struct SelectUserInfo {
public:
   VhallAudienceUserInfo userInfo;
   int selectType;
   int renderWndType;
};

//请求数据(带参数请求使用)
struct RQData {
   MsgRQType m_eMsgType;

   //用户列表,当前页(请求)
   int m_iCurPage;

   //用户信息(用户上下线)
   VhallAudienceUserInfo m_oUserInfo;

   // 公告，聊天
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
	eDevice_Mic = 0, //麦克
	eDevice_Camera //摄像头
};

enum eJesinDevice
{
	eJesinDevice_eDevice_Mic = 1,//麦克
	eJesinDevice_Camera = 2 //摄像头
};
//通知事件
struct Event {
   long m_lRoomID;
   MsgRQType m_eMsgType;

   //用户列表,总页数(返回)
   int m_iPageCount;
   int m_currentPage;
   int m_sumNumber;
   int m_bSendMsg;
   int m_memberSlider = 0;

   //事件结果
   bool m_bResult;

   //用户列表
   UserList m_oUserList;

   //用户信息(用户上下线)
   VhallAudienceUserInfo m_oUserInfo;
   bool m_isGag;
   bool m_bAudioState;
   bool m_bVideoState;
   bool m_bIsRtcPublisher;
   // 公告，聊天,白板操作状态
   char m_wzText[2048];

   // 表情
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
//互动客户端接口
class VHALL_EXPORT IInteractionClient {
public:

   virtual ~IInteractionClient(){};

   //初始化
   virtual BOOL Init(Delegate* pDelegate, Key_Data& oKeyData) = 0;

   virtual void RegisterDelegate(Delegate* pDelegate) = 0;

   //反初始化
   virtual void UnInit() = 0;

   //连接服务器()
   virtual void ConnectServer() = 0;
   //断开链接
   virtual void DisConnect() = 0;

   //发送请求
   virtual void MessageRQ(const MsgRQType&  eMsgType, RQData* vData = NULL) = 0;

   //socketIO 回调
   virtual void SetMessageSocketIOCallBack(InteractionMsgCallBack) = 0;

   //websocket 回调
   virtual void SetMessageWebSocketCallBack(InteractionMsgCallBack) = 0;

   virtual void SetProxyAddr(const bool&  enable, const char *proxy_ip, const char* proxy_port, const char *proxy_username, const char *proxy_password) = 0;

   virtual void OnHttpMsg(const QString& cmd, const QString& value) = 0;
};

VHALL_EXPORT IInteractionClient* CreateInteractionClient();
VHALL_EXPORT  void DestroyInteractionClient(IInteractionClient** interactionClient);
VHALL_EXPORT IInteractionClient* CreateAliveInteractionClient();
VHALL_EXPORT void DestroyAliveInteractionClient(IInteractionClient** interactionClient);
#endif
