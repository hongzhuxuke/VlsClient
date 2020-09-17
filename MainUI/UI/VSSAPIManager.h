#pragma once
#include <functional>
#include <QObject>
#include "VhallNetWorkInterface.h"

class OnLineMemberDetail{
public:
   OnLineMemberDetail(QString join_id, QString nickname, QString role_name, QString avatar, int is_banned, int is_kick, int device_type, int device_status) {
      this->join_id = join_id;
      this->nickname = nickname;
      this->role_name = role_name;
      this->avatar = avatar;
      this->is_banned = is_banned;
      this->is_kick = is_kick;
      this->device_type = device_type;
      this->device_status = device_status;
   }
   QString join_id;
   QString nickname;
   QString role_name;//角色
   QString avatar;//头像
   int is_banned; //是否禁言，1是0否
   int is_kick;  //是否踢出，1是0否
   int device_type;//设备类型
   int device_status;//设备状态
};

typedef std::function<void(int libCurlCode, const std::string& msg, const std::string userData)> VSSEventCallback,VSSSpeakEventCallBack, VSSNoSpeakEventCallBack;
typedef std::function<void(int libCurlCode, const std::string& msg, const std::string layout, const std::string definition)> VSSSetStreamEventCallback;
typedef std::function<void(int libCurlCode, const std::string& msg, int page,int pagesize)> VSSMemberListEventCallback;
typedef std::function<void(int libCurlCode, const std::string& msg, const std::string receive_join_id, int status)> VSSMemberKickEventCallback, VSSMemberSetBannedEventCallback;
typedef std::function<void(int libCurlCode, const std::string& msg, const std::string receive_join_id, const std::string join_id)> \
VSSSetMainViewEventCallback,VSSSetDocPermissionEventCallback, VSSRejectApplyEventCallback, VSSInviteJoinEventCallback,VSSRejectInviteEventCallback, VSSAgreeInviteEventCallback, VSSAgreeApplyEventCallback;
typedef std::function<void(int libCurlCode, const std::string& msg, const std::string join_id, int status)> VSSSetHandsupEventCallback;
typedef std::function<void(int libCurlCode, const std::string& msg, const std::string receive_join_id, const std::string join_id, int device, int status)> VSSSetDeviceStatusEventCallback;
typedef std::function<void(int errorCode,const std::string& msg)> OnErrorCallback;
typedef std::function<void(std::list<OnLineMemberDetail>)> OnSearchOnlineMemberCallback;


#define vss_start_live                  "/cmpt/room/start-live"             //开始直播
#define vss_end_live                    "/cmpt/room/end-live"               //结束直播
#define vss_get_attributes              "/cmpt/room/get-attributes"         //获取属性状态
#define vss_get                         "/cmpt/room/get"                    //获取房间信息
#define vss_set_device                  "/cmpt/inav/set-device"             //设置设备检测结果
#define vss_set_stream                  "/cmpt/inav/set-stream"             //设置观看端布局/清晰度
#define vss_agree_apply                 "/cmpt/inav/agree-apply"            //同意用户上麦申请
#define vss_get_user_status             "/cmpt/inav/get-user-status"        //获取用户状态
#define vss_get_banned_list             "/cmpt/inav/get-banned-list"        //获取禁言用户列表
#define vss_get_kicked_list             "/cmpt/inav/get-kicked-list"        //获取踢出用户列表
#define vss_get_online_list             "/cmpt/inav/get-online-list"        //获取在线用户列表
#define vss_get_special_list            "/cmpt/inav/get-special-list"       //获取特殊用户列表
#define vss_set_kicked                  "/cmpt/inav/set-kicked"             //踢出/取消踢出
#define vss_set_main_screen             "/cmpt/inav/set-main-screen"        //设置用户演示状态
#define vss_set_doc_permission          "/cmpt/inav/set-doc-permission"     //设置文档白板权限
#define vss_reject_apply                "/cmpt/inav/reject-apply"           //拒绝用户上麦申请
#define vss_set_handsup                 "/cmpt/inav/set-handsup"            //设置观众申请上麦许可（举手）
#define vss_apply                       "/cmpt/inav/apply"                  //用户申请上麦
#define vss_cancel_apply                "/cmpt/inav/cancel-apply"        //用户取消申请上麦
#define vss_invite                      "/cmpt/inav/invite"                 //邀请用户上麦
#define vss_reject_invite               "/cmpt/inav/reject-invite"          //拒绝上麦邀请
#define vss_speak                       "/cmpt/inav/speak"                  //用户上麦
#define vss_nospeak                     "/cmpt/inav/nospeak"                //用户下麦
#define vss_agree_invite                "/cmpt/inav/agree-invite"           //同意上麦邀请
#define vss_cancel_apply                "/cmpt/inav/cancel-apply"           //用户取消申请上麦
#define vss_set_banned                  "/cmpt/inav/set-banned"             //禁言/取消禁言
#define vss_set_device_status           "/cmpt/inav/set-device-status"      //设置设备开关状态
#define vss_get_push_info               "/cmpt/room/get-push-info"          //获取推流信息
#define vss_send_notice                 "/cmpt/inav/send-notice"             //发送公告
#define vss_chage_web_watch_layout      "/cmpt/inav/set-desktop"             //切换观看端的大小屏

#define MIC_DEVICE      1
#define CAMERA_DEVICE   2

class VSSAPIManager : public QObject
{
    Q_OBJECT

public:
    VSSAPIManager(QObject *parent = nullptr);
    ~VSSAPIManager();
	static  HTTP_GET_REQUEST GetHttpRequest(const std::string url, bool post = false);
    void InitAPIRequest(const std::string domain, const std::string vss_token, const std::string room_id);
    //开始直播
    void VSSStartLive(VSSEventCallback);
    //结束直播
    void VSSStopLive(VSSEventCallback);     
    //获取推流信息
    void VSSGetPushInfo(VSSEventCallback);
    //获取属性状态
    void VSSGetRoomAttributes(VSSEventCallback);
    //获取房间信息
    void VSSGetRoomBaseInfo(VSSEventCallback);
    //设置设备检测结果
    //status :设备检测状态，0未检测1可以上麦2不可以上麦
    //type  : 设备类型，0其它1手机端2PC
    void VSSSetDevice(const int status,const int type,VSSEventCallback);
    //设置观看端布局/清晰度
    void VSSSetStream(const std::string layout,const std::string definition, VSSSetStreamEventCallback);
    //同意用户上麦申请
    void VSSAgreeApply(const std::string receive_join_id, const std::string join_id, VSSAgreeApplyEventCallback);
    //获取用户状态
    void VSSGetUserSStatus(const std::string account_id, VSSEventCallback);
    //获取禁言用户列表
    //page:页码
    //pagesize:每页数量
    void VSSGetBannedList(int page, int pagesize, VSSMemberListEventCallback);
    //获取踢出用户列表
    void VSSGetKickedList(int page, int pagesize, VSSMemberListEventCallback);
    //获取在线用户列表
    void VSSGetOnlineList(int page, int pagesize, VSSMemberListEventCallback);
    //获取特殊用户列表
    void VSSGetSpecialList(int page, int pagesize, VSSMemberListEventCallback);
    //昵称精确搜索在线成员
    void VSSSearchOnLineMember(const std::string nickname, VSSMemberListEventCallback);
    //踢出/取消踢出
    void VSSSetKicked(const std::string receive_join_id, int status, VSSMemberKickEventCallback);
    //设置用户演示状态（主画面）
    void VSSSetMainScreen(const std::string receive_join_id, const std::string join_id, VSSSetMainViewEventCallback);
    //设置文档白板权限
    void VSSSetDocPermission(const std::string receive_join_id, const std::string join_id, VSSSetDocPermissionEventCallback);
    //拒绝用户上麦申请
    void VSSRejectApply(const std::string receive_join_id, const std::string join_id, VSSRejectApplyEventCallback);
    //设置观众申请上麦许可（举手）
    void VSSSetHandsup(const std::string join_id, int status, VSSSetHandsupEventCallback);
    //用户申请上麦
    void VSSApply(VSSEventCallback);
    //用户取消申请上麦
    void VSSCancleApply(VSSEventCallback callbackFun);
    //邀请用户上麦
    void VSSInvite(const std::string receive_join_id, const std::string join_id, VSSInviteJoinEventCallback);
    //拒绝上麦邀请
    void VSSRejectInvite(const std::string receive_join_id, const std::string join_id, VSSRejectInviteEventCallback);
    //用户上麦
    void VSSSpeak(const std::string join_id, VSSSpeakEventCallBack);
    //用户下麦
    void VSSNoSpeak(const std::string receive_join_id, VSSNoSpeakEventCallBack);
    //同意上麦邀请
    void VSSAgreeInvite(const std::string receive_join_id, const std::string join_id, VSSAgreeInviteEventCallback);
    //用户取消申请上麦
    void VSSCancelApply(const std::string join_id, VSSEventCallback);
    //禁言/取消禁言  1禁言0恢复
    void VSSSetBanned(const std::string receive_join_id, int status, VSSMemberSetBannedEventCallback);
    //设置设备开关状态
    //devic  1麦克风，2摄像头
    //status 0关闭1打开
    void VSSSetDeviceStatus(const std::string receive_join_id, const std::string join_id,int device, int status, VSSSetDeviceStatusEventCallback);
    //发送公告
    void VSSSendNotice(QString content, VSSEventCallback callback);
    //
    void HttpSendChangeWebWatchLayout(int status, VSSEventCallback callback);
private:
   

private:
    QString mVssToken;
    QString mRoomId;
    QString mDomain;
};
