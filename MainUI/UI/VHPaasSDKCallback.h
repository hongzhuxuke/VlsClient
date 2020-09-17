#pragma once
#include <atomic>
#include <QObject>
#include "VHPaasInteractionRoom.h"


class VHPaasSDKCallback : public QObject, public vlive::VLiveRoomMsgEventDelegate, public VRtcEngineEventDelegate
{
    Q_OBJECT

public:
    VHPaasSDKCallback(QObject *parent = nullptr);
    ~VHPaasSDKCallback();

    void SetEventReciver(QObject* mainLogic);

    void SetVLiveExit(bool exit);

    /*
    *   监听互动房间内HTTP业务API调用成功事件
    *   code: RoomEvent 事件类型
    *   userData: 附带的json数据
    */
    virtual void OnSuccessedEvent(vlive::RoomEvent code, std::string userData = std::string());
    /*
    *   监听互动房间内HTTP业务API调用失败事件
    *   respCode: 错误码
    *   msg：错误信息
    *   userData: 附带的json数据
    */                            
    virtual void OnFailedEvent(vlive::RoomEvent code, int respCode, std::string msg);
    /*
    *   获取互动房间成员列表回调
    **/
    virtual void OnGetVHRoomMembers(const std::string& third_party_user_id, std::list<vlive::VHRoomMember>&);
    /*
    *   获取被踢出互动房间人列表回调
    **/
    virtual void OnGetVHRoomKickOutMembers(std::list<vlive::VHRoomMember>&);
    /*
    *   接收CMD或这Flash消息
    */
    virtual void OnRecvSocketIOMsg(vlive::SocketIOMsgType msgType, std::string);
    /*
 *   收到申请上麦消息 ,消息为广播消息，收到消息后，通过进入互动房间时获取的权限列表，判断用户是否有审核权限
 */
    virtual void OnRecvApplyInavPublishMsg(const std::wstring& third_party_user_id);
    /*
    *   收到审核上麦消息 ,消息为广播消息，收到同意上麦后执行上麦操作
    */
    virtual void OnRecvAuditInavPublishMsg(const std::wstring& third_party_user_id, vlive::AuditPublish);
    /*
    *   邀请上麦消息  消息为广播消息，收到消息后，提示邀请信息
    */
    virtual void OnRecvAskforInavPublishMsg(const std::wstring& third_party_user_id);
    /*
    *   踢出流消息  消息为广播消息，收到消息后，执行踢出流
    */
    virtual void OnRecvKickInavStreamMsg(const std::wstring& third_party_user_id);
    /*
    *   踢出互动房间 , 消息为广播消息，收到消息后，判断是当前用户后，执行踢出房间操作
    */
    virtual void OnRecvKickInavMsg(const std::wstring& third_party_user_id);
    /*
    *   上/下/拒绝上麦消息 消息为广播消息
    */
    virtual void OnUserPublishCallback(const std::wstring& third_party_user_id, const std::string& stream_id, vlive::PushStreamEvent event);
    /*
    *   互动房间关闭消息  接受到该消息后，所有在互动房间的人员，全部下麦，退出房间
    */
    virtual void OnRecvInavCloseMsg();
    /*
    *   用户上下线通知
    *   online： true 用户上线/ false 用户下线
    *   user_id： 用户id
    */
    virtual void OnRecvChatCtrlMsg(const vlive::ChatMsgType msgType, const char* msg);
    /*
    *   房间链接事件
    */
    virtual void OnRtcRoomNetStateCallback(const vlive::RtcRoomNetState);
    /*
    *   本地打开采集设备,包括摄像头、桌面共享、开始插播视频回调,
    */
    virtual void OnOpenCaptureCallback(vlive::VHStreamType streamType, vlive::VHCapture code, bool hasVideo, bool hasAudio);
    /*
    *   推流成功回调
    */
    virtual void OnPushStreamSuc(vlive::VHStreamType streamType, std::string& streamid, bool hasVideo, bool hasAudio, std::string& attributes);
    /*
    *   推流失败回调
    */
    virtual void OnPushStreamError(vlive::VHStreamType streamType, const int codeErr = 0, const std::string& msg = std::string());
    /*
    *   本地网络重连成功之后重推流，流ID发生改变通知
    **/
    virtual void OnRePublishStreamIDChanged(vlive::VHStreamType streamType, const std::wstring& user_id, const std::string& old_streamid, const std::string& new_streamid);
    /*
    *   停止推流回调
    *   code: 0成功， 其他失败
    */
    virtual void OnStopPushStreamCallback(vlive::VHStreamType streamType, int code, const std::string& msg);
    /**
    *   接收到远端推流。
    */
    virtual void OnRemoteStreamAdd(const std::string& user_id, const std::string& stream_id, vlive::VHStreamType streamType);
    /**
    *   订阅流失败。
    */
    virtual void OnSubScribeStreamErr(const std::string& user_id, const std::string& msg, int errorCode, const std::string& join_id = std::string());
    /*
    *    订阅成功之后，接收到远端的媒体流  hasVideo ：是否包含视频
    *    当接收到远端媒体流包括插播视频流或者桌面共享流，如果本地已经打开插播或桌面共享时自动停止。
    */
    virtual void OnReciveRemoteUserLiveStream(const std::wstring& user, const std::string& streamid, const vlive::VHStreamType type, bool hasVideo, bool hasAudio, const std::string user_data);
    /*
    *   远端的媒体流退出了
    */
    virtual void OnRemoteUserLiveStreamRemoved(const std::wstring& user, const std::string& streamid, const vlive::VHStreamType type) ;
    /*
    *   订阅大小流回调
    */
    virtual void OnChangeSubScribeUserSimulCast(const std::wstring& user_id, vlive::VHSimulCastType type, int code, std::string msg);

    /*
    *    开启旁路混流事件回调
    */
    virtual void OnConfigBroadCast(const int layout, const int profile, const std::string& result, const std::string& msg){};
    /*
    *    停止旁路混流事件回调
    */
    virtual void OnStopConfigBroadCast(const std::string& result, const std::string& msg){};
    /*
    *    设置旁路事件回调
    */
    virtual void OnChangeConfigBroadCastLayout(const int layout, const std::string& result, const std::string& msg){};
    /*
    *    设置大画面布局
    */
    virtual void OnSetMainView(const std::string main_view_stream_id, int stream_type, const std::string& result, const std::string& msg){};

private:
    QObject* mainLogicReciver = nullptr;
    QObject* vliveMainUI = nullptr;
    QObject* settingUI = nullptr;

    std::atomic_bool mbIsExit = false;

};
