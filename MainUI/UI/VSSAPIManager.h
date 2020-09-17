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
   QString role_name;//��ɫ
   QString avatar;//ͷ��
   int is_banned; //�Ƿ���ԣ�1��0��
   int is_kick;  //�Ƿ��߳���1��0��
   int device_type;//�豸����
   int device_status;//�豸״̬
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


#define vss_start_live                  "/cmpt/room/start-live"             //��ʼֱ��
#define vss_end_live                    "/cmpt/room/end-live"               //����ֱ��
#define vss_get_attributes              "/cmpt/room/get-attributes"         //��ȡ����״̬
#define vss_get                         "/cmpt/room/get"                    //��ȡ������Ϣ
#define vss_set_device                  "/cmpt/inav/set-device"             //�����豸�����
#define vss_set_stream                  "/cmpt/inav/set-stream"             //���ùۿ��˲���/������
#define vss_agree_apply                 "/cmpt/inav/agree-apply"            //ͬ���û���������
#define vss_get_user_status             "/cmpt/inav/get-user-status"        //��ȡ�û�״̬
#define vss_get_banned_list             "/cmpt/inav/get-banned-list"        //��ȡ�����û��б�
#define vss_get_kicked_list             "/cmpt/inav/get-kicked-list"        //��ȡ�߳��û��б�
#define vss_get_online_list             "/cmpt/inav/get-online-list"        //��ȡ�����û��б�
#define vss_get_special_list            "/cmpt/inav/get-special-list"       //��ȡ�����û��б�
#define vss_set_kicked                  "/cmpt/inav/set-kicked"             //�߳�/ȡ���߳�
#define vss_set_main_screen             "/cmpt/inav/set-main-screen"        //�����û���ʾ״̬
#define vss_set_doc_permission          "/cmpt/inav/set-doc-permission"     //�����ĵ��װ�Ȩ��
#define vss_reject_apply                "/cmpt/inav/reject-apply"           //�ܾ��û���������
#define vss_set_handsup                 "/cmpt/inav/set-handsup"            //���ù�������������ɣ����֣�
#define vss_apply                       "/cmpt/inav/apply"                  //�û���������
#define vss_cancel_apply                "/cmpt/inav/cancel-apply"        //�û�ȡ����������
#define vss_invite                      "/cmpt/inav/invite"                 //�����û�����
#define vss_reject_invite               "/cmpt/inav/reject-invite"          //�ܾ���������
#define vss_speak                       "/cmpt/inav/speak"                  //�û�����
#define vss_nospeak                     "/cmpt/inav/nospeak"                //�û�����
#define vss_agree_invite                "/cmpt/inav/agree-invite"           //ͬ����������
#define vss_cancel_apply                "/cmpt/inav/cancel-apply"           //�û�ȡ����������
#define vss_set_banned                  "/cmpt/inav/set-banned"             //����/ȡ������
#define vss_set_device_status           "/cmpt/inav/set-device-status"      //�����豸����״̬
#define vss_get_push_info               "/cmpt/room/get-push-info"          //��ȡ������Ϣ
#define vss_send_notice                 "/cmpt/inav/send-notice"             //���͹���
#define vss_chage_web_watch_layout      "/cmpt/inav/set-desktop"             //�л��ۿ��˵Ĵ�С��

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
    //��ʼֱ��
    void VSSStartLive(VSSEventCallback);
    //����ֱ��
    void VSSStopLive(VSSEventCallback);     
    //��ȡ������Ϣ
    void VSSGetPushInfo(VSSEventCallback);
    //��ȡ����״̬
    void VSSGetRoomAttributes(VSSEventCallback);
    //��ȡ������Ϣ
    void VSSGetRoomBaseInfo(VSSEventCallback);
    //�����豸�����
    //status :�豸���״̬��0δ���1��������2����������
    //type  : �豸���ͣ�0����1�ֻ���2PC
    void VSSSetDevice(const int status,const int type,VSSEventCallback);
    //���ùۿ��˲���/������
    void VSSSetStream(const std::string layout,const std::string definition, VSSSetStreamEventCallback);
    //ͬ���û���������
    void VSSAgreeApply(const std::string receive_join_id, const std::string join_id, VSSAgreeApplyEventCallback);
    //��ȡ�û�״̬
    void VSSGetUserSStatus(const std::string account_id, VSSEventCallback);
    //��ȡ�����û��б�
    //page:ҳ��
    //pagesize:ÿҳ����
    void VSSGetBannedList(int page, int pagesize, VSSMemberListEventCallback);
    //��ȡ�߳��û��б�
    void VSSGetKickedList(int page, int pagesize, VSSMemberListEventCallback);
    //��ȡ�����û��б�
    void VSSGetOnlineList(int page, int pagesize, VSSMemberListEventCallback);
    //��ȡ�����û��б�
    void VSSGetSpecialList(int page, int pagesize, VSSMemberListEventCallback);
    //�ǳƾ�ȷ�������߳�Ա
    void VSSSearchOnLineMember(const std::string nickname, VSSMemberListEventCallback);
    //�߳�/ȡ���߳�
    void VSSSetKicked(const std::string receive_join_id, int status, VSSMemberKickEventCallback);
    //�����û���ʾ״̬�������棩
    void VSSSetMainScreen(const std::string receive_join_id, const std::string join_id, VSSSetMainViewEventCallback);
    //�����ĵ��װ�Ȩ��
    void VSSSetDocPermission(const std::string receive_join_id, const std::string join_id, VSSSetDocPermissionEventCallback);
    //�ܾ��û���������
    void VSSRejectApply(const std::string receive_join_id, const std::string join_id, VSSRejectApplyEventCallback);
    //���ù�������������ɣ����֣�
    void VSSSetHandsup(const std::string join_id, int status, VSSSetHandsupEventCallback);
    //�û���������
    void VSSApply(VSSEventCallback);
    //�û�ȡ����������
    void VSSCancleApply(VSSEventCallback callbackFun);
    //�����û�����
    void VSSInvite(const std::string receive_join_id, const std::string join_id, VSSInviteJoinEventCallback);
    //�ܾ���������
    void VSSRejectInvite(const std::string receive_join_id, const std::string join_id, VSSRejectInviteEventCallback);
    //�û�����
    void VSSSpeak(const std::string join_id, VSSSpeakEventCallBack);
    //�û�����
    void VSSNoSpeak(const std::string receive_join_id, VSSNoSpeakEventCallBack);
    //ͬ����������
    void VSSAgreeInvite(const std::string receive_join_id, const std::string join_id, VSSAgreeInviteEventCallback);
    //�û�ȡ����������
    void VSSCancelApply(const std::string join_id, VSSEventCallback);
    //����/ȡ������  1����0�ָ�
    void VSSSetBanned(const std::string receive_join_id, int status, VSSMemberSetBannedEventCallback);
    //�����豸����״̬
    //devic  1��˷磬2����ͷ
    //status 0�ر�1��
    void VSSSetDeviceStatus(const std::string receive_join_id, const std::string join_id,int device, int status, VSSSetDeviceStatusEventCallback);
    //���͹���
    void VSSSendNotice(QString content, VSSEventCallback callback);
    //
    void HttpSendChangeWebWatchLayout(int status, VSSEventCallback callback);
private:
   

private:
    QString mVssToken;
    QString mRoomId;
    QString mDomain;
};
