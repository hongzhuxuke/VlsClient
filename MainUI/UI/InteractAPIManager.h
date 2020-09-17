#pragma once

#include <QObject>
#include "PublishInfo.h"
#include <QObject>

class InteractAPIManager : public QObject
{
    Q_OBJECT

public:
    InteractAPIManager(QObject *recvObj);
    ~InteractAPIManager();

    void HttpSendStartLive(int liveType, bool bScheduling = false);
    void HttpSendStopLive(int liveType, bool close = false);
    void HttpSendNotSpeak(QString uid, bool showNotice);
    void HttpSendSpeak();
    void HttpSendInvite(QString uid, bool showNotice);
    void HttpSendAgreeInvite();
    void HttpSendRejectInvite();
    void HttpSendApplyToSpeak();
    void HttpSendCancelApplyToSpeak();

    void HttpSendAgreeToSpeak(const QString& strId);
    void HttpSendRejectSpeak(const QString& strId);
    void HttpSendAllowRaiseHands(const bool enable);
    void HttpSendSwitchSpeaker(QString receive_uid, QString role, int enableNotice);
    void HttpSendDeviceStatus(QString receiveJoinId, int device, int type);
    void HttpSendMixLayOutAndProfile(QString strLayout, QString videoProfile);
    void HttpSendGetSpeakUserList();
    void HttpSendSetDevice(const int device_status);
    void HttpSendSetDocPermission(const QString& strId);
    void HttpSendChangeWebWatchLayout(int status); //×´Ì¬ 0¹Ø±Õ 1¿ªÆô
    void HttpSendDispatchPublish(const QString& url, int dwPublishState, bool mediaCoreStreamErr, bool isAlreadyLive);
    void HttpSendFeedBack(int type, QString context);

public:
    static void ParamHttpResp(std::string msg, int& code, std::string& respMsgOut);

private:
    ClientApiInitResp mRespInitData;
    StartLiveUrl mStartLiveData;
    QObject* mpMainUILogic = nullptr;
};
