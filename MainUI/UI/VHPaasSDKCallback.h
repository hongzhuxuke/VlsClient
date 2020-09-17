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
    *   ��������������HTTPҵ��API���óɹ��¼�
    *   code: RoomEvent �¼�����
    *   userData: ������json����
    */
    virtual void OnSuccessedEvent(vlive::RoomEvent code, std::string userData = std::string());
    /*
    *   ��������������HTTPҵ��API����ʧ���¼�
    *   respCode: ������
    *   msg��������Ϣ
    *   userData: ������json����
    */                            
    virtual void OnFailedEvent(vlive::RoomEvent code, int respCode, std::string msg);
    /*
    *   ��ȡ���������Ա�б�ص�
    **/
    virtual void OnGetVHRoomMembers(const std::string& third_party_user_id, std::list<vlive::VHRoomMember>&);
    /*
    *   ��ȡ���߳������������б�ص�
    **/
    virtual void OnGetVHRoomKickOutMembers(std::list<vlive::VHRoomMember>&);
    /*
    *   ����CMD����Flash��Ϣ
    */
    virtual void OnRecvSocketIOMsg(vlive::SocketIOMsgType msgType, std::string);
    /*
 *   �յ�����������Ϣ ,��ϢΪ�㲥��Ϣ���յ���Ϣ��ͨ�����뻥������ʱ��ȡ��Ȩ���б��ж��û��Ƿ������Ȩ��
 */
    virtual void OnRecvApplyInavPublishMsg(const std::wstring& third_party_user_id);
    /*
    *   �յ����������Ϣ ,��ϢΪ�㲥��Ϣ���յ�ͬ�������ִ���������
    */
    virtual void OnRecvAuditInavPublishMsg(const std::wstring& third_party_user_id, vlive::AuditPublish);
    /*
    *   ����������Ϣ  ��ϢΪ�㲥��Ϣ���յ���Ϣ����ʾ������Ϣ
    */
    virtual void OnRecvAskforInavPublishMsg(const std::wstring& third_party_user_id);
    /*
    *   �߳�����Ϣ  ��ϢΪ�㲥��Ϣ���յ���Ϣ��ִ���߳���
    */
    virtual void OnRecvKickInavStreamMsg(const std::wstring& third_party_user_id);
    /*
    *   �߳��������� , ��ϢΪ�㲥��Ϣ���յ���Ϣ���ж��ǵ�ǰ�û���ִ���߳��������
    */
    virtual void OnRecvKickInavMsg(const std::wstring& third_party_user_id);
    /*
    *   ��/��/�ܾ�������Ϣ ��ϢΪ�㲥��Ϣ
    */
    virtual void OnUserPublishCallback(const std::wstring& third_party_user_id, const std::string& stream_id, vlive::PushStreamEvent event);
    /*
    *   ��������ر���Ϣ  ���ܵ�����Ϣ�������ڻ����������Ա��ȫ�������˳�����
    */
    virtual void OnRecvInavCloseMsg();
    /*
    *   �û�������֪ͨ
    *   online�� true �û�����/ false �û�����
    *   user_id�� �û�id
    */
    virtual void OnRecvChatCtrlMsg(const vlive::ChatMsgType msgType, const char* msg);
    /*
    *   ���������¼�
    */
    virtual void OnRtcRoomNetStateCallback(const vlive::RtcRoomNetState);
    /*
    *   ���ش򿪲ɼ��豸,��������ͷ�����湲����ʼ�岥��Ƶ�ص�,
    */
    virtual void OnOpenCaptureCallback(vlive::VHStreamType streamType, vlive::VHCapture code, bool hasVideo, bool hasAudio);
    /*
    *   �����ɹ��ص�
    */
    virtual void OnPushStreamSuc(vlive::VHStreamType streamType, std::string& streamid, bool hasVideo, bool hasAudio, std::string& attributes);
    /*
    *   ����ʧ�ܻص�
    */
    virtual void OnPushStreamError(vlive::VHStreamType streamType, const int codeErr = 0, const std::string& msg = std::string());
    /*
    *   �������������ɹ�֮������������ID�����ı�֪ͨ
    **/
    virtual void OnRePublishStreamIDChanged(vlive::VHStreamType streamType, const std::wstring& user_id, const std::string& old_streamid, const std::string& new_streamid);
    /*
    *   ֹͣ�����ص�
    *   code: 0�ɹ��� ����ʧ��
    */
    virtual void OnStopPushStreamCallback(vlive::VHStreamType streamType, int code, const std::string& msg);
    /**
    *   ���յ�Զ��������
    */
    virtual void OnRemoteStreamAdd(const std::string& user_id, const std::string& stream_id, vlive::VHStreamType streamType);
    /**
    *   ������ʧ�ܡ�
    */
    virtual void OnSubScribeStreamErr(const std::string& user_id, const std::string& msg, int errorCode, const std::string& join_id = std::string());
    /*
    *    ���ĳɹ�֮�󣬽��յ�Զ�˵�ý����  hasVideo ���Ƿ������Ƶ
    *    �����յ�Զ��ý���������岥��Ƶ���������湲��������������Ѿ��򿪲岥�����湲��ʱ�Զ�ֹͣ��
    */
    virtual void OnReciveRemoteUserLiveStream(const std::wstring& user, const std::string& streamid, const vlive::VHStreamType type, bool hasVideo, bool hasAudio, const std::string user_data);
    /*
    *   Զ�˵�ý�����˳���
    */
    virtual void OnRemoteUserLiveStreamRemoved(const std::wstring& user, const std::string& streamid, const vlive::VHStreamType type) ;
    /*
    *   ���Ĵ�С���ص�
    */
    virtual void OnChangeSubScribeUserSimulCast(const std::wstring& user_id, vlive::VHSimulCastType type, int code, std::string msg);

    /*
    *    ������·�����¼��ص�
    */
    virtual void OnConfigBroadCast(const int layout, const int profile, const std::string& result, const std::string& msg){};
    /*
    *    ֹͣ��·�����¼��ص�
    */
    virtual void OnStopConfigBroadCast(const std::string& result, const std::string& msg){};
    /*
    *    ������·�¼��ص�
    */
    virtual void OnChangeConfigBroadCastLayout(const int layout, const std::string& result, const std::string& msg){};
    /*
    *    ���ô��沼��
    */
    virtual void OnSetMainView(const std::string main_view_stream_id, int stream_type, const std::string& result, const std::string& msg){};

private:
    QObject* mainLogicReciver = nullptr;
    QObject* vliveMainUI = nullptr;
    QObject* settingUI = nullptr;

    std::atomic_bool mbIsExit = false;

};
