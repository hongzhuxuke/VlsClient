#include "InteractAPIManager.h"
#include "VSSAPIManager.h"
#include "ICommonData.h"
#include "CRPluginDef.h"
#include "MainUIIns.h"
#include "Msg_MainUI.h"
#include "pathManager.h"
#include "pathManage.h"
#include "VhallNetWorkInterface.h"
#include "ConfigSetting.h"
#include "VH_ConstDeff.h"
#include "DebugTrace.h"
#include "vhproxytest.h"

InteractAPIManager::InteractAPIManager(QObject *obj)
    : mpMainUILogic(obj)
{
    //初始化设置用户信息。
    VH::CComPtr<ICommonData> pCommonData;
    DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
    pCommonData->GetInitApiResp(mRespInitData);
    pCommonData->GetStartLiveUrl(mStartLiveData);
    if (mRespInitData.player == H5_LIVE_CREATE_TYPE) {
        //h5的活动 用户id 替换成 third_party_user_id
        VSSGetRoomInfo roomInfo;
        pCommonData->GetVSSRoomBaseInfo(roomInfo);
        mRespInitData.join_uid = roomInfo.third_party_user_id;
    }
}

InteractAPIManager::~InteractAPIManager()
{
}

void InteractAPIManager::HttpSendStartLive(int liveType, bool bScheduling/* = false*/) {
    QObject* recvObj = mpMainUILogic;
    if (mRespInitData.player == H5_LIVE_CREATE_TYPE) {
        TRACE6("%s VSSStartLive\n", __FUNCTION__);
        QString domain = ConfigSetting::ReadString(CPathManager::GetToolConfigPath(), GROUP_DEFAULT, KEY_VSS_URL_DOMAIN, vss_domainUrl);
        VSSAPIManager vssApi;
        vssApi.InitAPIRequest(domain.toStdString(), mRespInitData.vss_token.toStdString(), mRespInitData.vss_room_id.toStdString());
        vssApi.VSSStartLive([&, recvObj, liveType, bScheduling](int libCurlCode, const std::string& msg, const std::string userData) {
            if (recvObj) {
                if (liveType == eLiveType_Live) {
                    QHttpResponeEvent *eventMsg = new QHttpResponeEvent(CustomEvent_StartWebniar, libCurlCode, QString::fromStdString(msg));
                    if (eventMsg) {
                        eventMsg->mIsScheduling = bScheduling;
                        QCoreApplication::postEvent(recvObj, eventMsg);
                    }
                }
                else {
                    CustomOnHttpResMsgEvent* event = new CustomOnHttpResMsgEvent(CustomEvent_VSSStartLiveResp);
                    if (event) {
                        event->mCode = libCurlCode;
                        event->mMsg = QString::fromStdString(msg);
                        QCoreApplication::postEvent(recvObj, event, Qt::LowEventPriority);
                    }
                }
            }
        });
    }
    else {
        //代理配置
        QString configPath = CPathManager::GetConfigPath();
        QString proxyHost;
        QString proxyUsr;
        QString proxyPwd;
        int proxyPort;
        int is_http_proxy = ConfigSetting::ReadInt(configPath, GROUP_DEFAULT, PROXY_OPEN, 0);
        if (is_http_proxy) {
            TRACE6("%s is_http_proxy:%d\n", __FUNCTION__, is_http_proxy);
            proxyHost = ConfigSetting::ReadString(configPath, GROUP_DEFAULT, PROXY_HOST, "");
            proxyPort = ConfigSetting::ReadInt(configPath, GROUP_DEFAULT, PROXY_PORT, 80);
            proxyUsr = ConfigSetting::ReadString(configPath, GROUP_DEFAULT, PROXY_USERNAME, "");
            proxyPwd = ConfigSetting::ReadString(configPath, GROUP_DEFAULT, PROXY_PASSWORD, "");
        }
        QString domain = ConfigSetting::ReadString(CPathManager::GetToolConfigPath(), GROUP_DEFAULT, KEY_URL_DOMAIN, domainUrl);
        QString httpUrl = domain + "/api/client/v1/clientapi/startwebinar?token=" + mStartLiveData.msg_token;
        HTTP_GET_REQUEST request(httpUrl.toStdString());
        GetHttpManagerInstance()->HttpGetRequest(request, [&, recvObj, liveType, bScheduling](const std::string& msg, int libCurlCode, const std::string userData) {
            if (recvObj) {
                if (liveType == eLiveType_Live) {
                    QHttpResponeEvent *eventMsg = new QHttpResponeEvent(CustomEvent_StartWebniar, libCurlCode, QString::fromStdString(msg));
                    if (eventMsg) {
                        eventMsg->mIsScheduling = bScheduling;
                        QCoreApplication::postEvent(recvObj, eventMsg);
                    }
                }
                else {
                    CustomOnHttpResMsgEvent* event = new CustomOnHttpResMsgEvent(CustomEvent_VSSStartLiveResp);
                    if (event) {
                        event->mCode = libCurlCode;
                        event->mMsg = QString::fromStdString(msg);
                        TRACE6("%s msg:%s\n", __FUNCTION__, msg.c_str());
                        QCoreApplication::postEvent(recvObj, event, Qt::LowEventPriority);
                    }
                }
            }
        });
    }
}

void InteractAPIManager::HttpSendStopLive(int liveType, bool close /*= false*/) {
    QObject* recvObj = mpMainUILogic;
    if (mRespInitData.player == H5_LIVE_CREATE_TYPE) {
        TRACE6("%s VSSStartLive\n", __FUNCTION__);
        QString domain = ConfigSetting::ReadString(CPathManager::GetToolConfigPath(), GROUP_DEFAULT, KEY_VSS_URL_DOMAIN, vss_domainUrl);
        VSSAPIManager vssApi;
        vssApi.InitAPIRequest(domain.toStdString(), mRespInitData.vss_token.toStdString(), mRespInitData.vss_room_id.toStdString());
        vssApi.VSSStopLive([&, recvObj, liveType, close](int libCurlCode, const std::string& msg, const std::string userData) {
            if (recvObj) {
                if (liveType == eLiveType_Live) {
                    QHttpResponeEvent* stopEvent = new QHttpResponeEvent(CustomEvent_StopWebniar, libCurlCode, QString::fromStdString(msg));
                    if (stopEvent) {
                        stopEvent->mIsClose = close;
                        QCoreApplication::postEvent(recvObj, stopEvent);
                    }
                }
                else {
                    CustomOnHttpResMsgEvent* event = new CustomOnHttpResMsgEvent(CustomEvent_VSSStopLive);
                    if (event) {
                        event->mCode = libCurlCode;
                        event->mMsg = QString::fromStdString(msg);
                        QCoreApplication::postEvent(recvObj, event, Qt::LowEventPriority);
                    }
                }
            }
        });
    }
    else {
        //异步处理结束请求。
        QString confPath = CPathManager::GetConfigPath();
        QString domain = ConfigSetting::ReadString(CPathManager::GetToolConfigPath(), GROUP_DEFAULT, KEY_URL_DOMAIN, domainUrl);
        QString httpUrl = QString("%1/api/client/v1/clientapi/stopwebinar?token=%2").arg(domain).arg(mStartLiveData.msg_token);
        TRACE6("%s enter url:%s\n", __FUNCTION__, httpUrl.toStdString().c_str());
        //代理配置
        QString configPath = CPathManager::GetConfigPath();
        QString proxyHost;
        QString proxyUsr;
        QString proxyPwd;
        int proxyPort;
        int is_http_proxy = ConfigSetting::ReadInt(configPath, GROUP_DEFAULT, PROXY_OPEN, 0);
        if (is_http_proxy) {
            TRACE6("%s is_http_proxy:%d\n", __FUNCTION__, is_http_proxy);
            proxyHost = ConfigSetting::ReadString(configPath, GROUP_DEFAULT, PROXY_HOST, "");
            proxyPort = ConfigSetting::ReadInt(configPath, GROUP_DEFAULT, PROXY_PORT, 80);
            proxyUsr = ConfigSetting::ReadString(configPath, GROUP_DEFAULT, PROXY_USERNAME, "");
            proxyPwd = ConfigSetting::ReadString(configPath, GROUP_DEFAULT, PROXY_PASSWORD, "");
        }
        HTTP_GET_REQUEST request(httpUrl.toStdString());
        GetHttpManagerInstance()->HttpGetRequest(request, [&, recvObj, liveType, close](const std::string& msg, int libCurlCode, const std::string userData) {
            if (recvObj) {
                if (liveType == eLiveType_Live) {
                    QHttpResponeEvent* stopEvent = new QHttpResponeEvent(CustomEvent_StopWebniar, libCurlCode, QString::fromStdString(msg));
                    if (stopEvent) {
                        stopEvent->mIsClose = close;
                        QCoreApplication::postEvent(recvObj, stopEvent);
                    }
                }
                else {
                    CustomOnHttpResMsgEvent* event = new CustomOnHttpResMsgEvent(CustomEvent_VSSStopLive);
                    if (event) {
                        event->mCode = libCurlCode;
                        event->mMsg = QString::fromStdString(msg);
                        TRACE6("%s msg:%s\n", __FUNCTION__, msg.c_str());
                        QCoreApplication::postEvent(recvObj, event, Qt::LowEventPriority);
                    }
                }
            }
        });
    }
}

void InteractAPIManager::HttpSendNotSpeak(QString uid, bool showNotice) {
    QObject* recvObj = mpMainUILogic;
    if (mRespInitData.player == H5_LIVE_CREATE_TYPE) {
        TRACE6("%s VSSNoSpeak\n", __FUNCTION__);
        QString domain = ConfigSetting::ReadString(CPathManager::GetToolConfigPath(), GROUP_DEFAULT, KEY_VSS_URL_DOMAIN, vss_domainUrl);
        VSSAPIManager vssApi;
        vssApi.InitAPIRequest(domain.toStdString(), mRespInitData.vss_token.toStdString(), mRespInitData.vss_room_id.toStdString());
        vssApi.VSSNoSpeak(uid.toStdString(), [&, showNotice, recvObj](int libCurlCode, const std::string& msg, const std::string receive_join_id) {
            if (recvObj) {
                CustomOnHttpResMsgEvent* event = new CustomOnHttpResMsgEvent(CustomEvent_VSSNoSpeakResp);
                if (event) {
                    event->mCode = libCurlCode;
                    event->mMsg = QString::fromStdString(msg);
                    event->mRecvUserId = QString::fromStdString(receive_join_id);
                    event->mbNoticeMsg = showNotice;
                    QCoreApplication::postEvent(recvObj, event, Qt::LowEventPriority);
                }
            }
        });
    }
    else {
        QString domain = ConfigSetting::ReadString(CPathManager::GetToolConfigPath(), GROUP_DEFAULT, KEY_URL_DOMAIN, domainUrl);
        QString url = QString("%1/api/client/v1/interact/not-speak?receive_uid=%2&token=%3").arg(domain).arg(uid).arg(mStartLiveData.msg_token);
        TRACE6("%s mbIsGuestSpeaking enter url:%s\n", __FUNCTION__, url.toStdString().c_str());
        HTTP_GET_REQUEST request(url.toStdString());
        request.mbIsNeedSyncWork = true;
        GetHttpManagerInstance()->HttpGetRequest(request, [&, uid, recvObj, showNotice](const std::string& msg, int libCurlCode, const std::string userData) {
            if (recvObj) {
                CustomOnHttpResMsgEvent* event = new CustomOnHttpResMsgEvent(CustomEvent_VSSNoSpeakResp);
                if (event) {
                    event->mCode = libCurlCode;
                    event->mMsg = QString::fromStdString(msg);
                    event->mRecvUserId = uid;
                    event->mbNoticeMsg = showNotice;
                    TRACE6("%s msg:%s\n", __FUNCTION__, msg.c_str());
                    QCoreApplication::postEvent(recvObj, event, Qt::LowEventPriority);
                }
            }
        });
    }
}


void InteractAPIManager::HttpSendSpeak() {
    QObject* recvObj = mpMainUILogic;
    QString join_uid = mRespInitData.join_uid;
    if (mRespInitData.player == H5_LIVE_CREATE_TYPE) {
        TRACE6("%s VSSSpeak\n", __FUNCTION__);
        QString domain = ConfigSetting::ReadString(CPathManager::GetToolConfigPath(), GROUP_DEFAULT, KEY_VSS_URL_DOMAIN, vss_domainUrl);
        VSSAPIManager vssApi;
        vssApi.InitAPIRequest(domain.toStdString(), mRespInitData.vss_token.toStdString(), mRespInitData.vss_room_id.toStdString());
        vssApi.VSSSpeak(mRespInitData.join_uid.toStdString(), [&, recvObj, join_uid](int libCurlCode, const std::string& msg, const std::string userData) {
            if (recvObj) {
                CustomOnHttpResMsgEvent* event = new CustomOnHttpResMsgEvent(CustomEvent_VSSSpeak);
                if (event) {
                    event->mCode = libCurlCode;
                    event->mMsg = QString::fromStdString(msg);
                    event->mRecvUserId = join_uid;
                    QCoreApplication::postEvent(recvObj, event, Qt::LowEventPriority);
                }
            }
        });
    }
    else {
        QString domain = ConfigSetting::ReadString(CPathManager::GetToolConfigPath(), GROUP_DEFAULT, KEY_URL_DOMAIN, domainUrl);
        QString url = QString("%1/api/client/v1/interact/change-to-speaker?token=%2").arg(domain).arg(mStartLiveData.msg_token);
        TRACE6("%s tospeak url:%s\n", __FUNCTION__, url.toStdString().c_str());
        HTTP_GET_REQUEST request(url.toStdString());
        GetHttpManagerInstance()->HttpGetRequest(request, [&, recvObj, join_uid](const std::string& msg, int libCurlCode, const std::string userData) {
            if (recvObj) {
                CustomOnHttpResMsgEvent* event = new CustomOnHttpResMsgEvent(CustomEvent_VSSSpeak);
                if (event) {
                    event->mCode = libCurlCode;
                    event->mMsg = QString::fromStdString(msg);
                    event->mRecvUserId = join_uid;
                    TRACE6("%s msg:%s\n", __FUNCTION__, msg.c_str());
                    QCoreApplication::postEvent(recvObj, event, Qt::LowEventPriority);
                }
            }
        });
    }
}

void InteractAPIManager::HttpSendInvite(QString uid, bool showNotice) {
    QObject* recvObj = mpMainUILogic;
    QString joinUid = mRespInitData.join_uid;
    if (mRespInitData.player == H5_LIVE_CREATE_TYPE) {
        QString domain = ConfigSetting::ReadString(CPathManager::GetToolConfigPath(), GROUP_DEFAULT, KEY_VSS_URL_DOMAIN, vss_domainUrl);
        VSSAPIManager vssApi;
        vssApi.InitAPIRequest(domain.toStdString(), mRespInitData.vss_token.toStdString(), mRespInitData.vss_room_id.toStdString());
        vssApi.VSSInvite(uid.toStdString(), joinUid.toStdString(), [&, recvObj](int libCurlCode, const std::string& msg, const std::string receive_join_id, const std::string join_id) {
            if (recvObj) {
                CustomOnHttpResMsgEvent* event = new CustomOnHttpResMsgEvent(CustomEvent_VSSSendInvite);
                if (event) {
                    event->mCode = libCurlCode;
                    event->mMsg = QString::fromStdString(msg);
                    event->mRecvUserId = QString::fromStdString(receive_join_id);
                    event->mUid = QString::fromStdString(join_id);
                    QCoreApplication::postEvent(recvObj, event, Qt::LowEventPriority);
                }
            }
        });
    }
    else {
        QString domain = ConfigSetting::ReadString(CPathManager::GetToolConfigPath(), GROUP_DEFAULT, KEY_URL_DOMAIN, domainUrl);
        QString url = QString("%1/api/client/v1/interact/send-invite?receive_uid=%2&token=%3").arg(domain).arg(uid).arg(mStartLiveData.msg_token);
        TRACE6("%s url:%s\n", __FUNCTION__, uid.toStdString().c_str());
        HTTP_GET_REQUEST request(url.toStdString());
        GetHttpManagerInstance()->HttpGetRequest(request, [&, uid, showNotice, recvObj, joinUid](const std::string& msg, int libCurlCode, const std::string userData) {
            if (recvObj) {
                CustomOnHttpResMsgEvent* event = new CustomOnHttpResMsgEvent(CustomEvent_VSSSendInvite);
                if (event) {
                    event->mCode = libCurlCode;
                    event->mMsg = QString::fromStdString(msg);
                    event->mRecvUserId = uid;
                    event->mUid = joinUid;
                    event->mbNoticeMsg = showNotice;
                    TRACE6("%s msg:%s\n", __FUNCTION__, msg.c_str());
                    QCoreApplication::postEvent(recvObj, event, Qt::LowEventPriority);
                }
            }
        });
    }
}

void InteractAPIManager::HttpSendAgreeInvite() {
    //同意上麦邀请
    QObject* recvObj = mpMainUILogic;
    QString joinUid = mRespInitData.join_uid;
    if (mRespInitData.player == H5_LIVE_CREATE_TYPE) {
        TRACE6("%s HttpSendAgreeInvite\n", __FUNCTION__);
        QString domain = ConfigSetting::ReadString(CPathManager::GetToolConfigPath(), GROUP_DEFAULT, KEY_VSS_URL_DOMAIN, vss_domainUrl);
        VSSAPIManager vssApi;
        vssApi.InitAPIRequest(domain.toStdString(), mRespInitData.vss_token.toStdString(), mRespInitData.vss_room_id.toStdString());
        vssApi.VSSAgreeInvite(joinUid.toStdString(), joinUid.toStdString(), [&, recvObj](int libCurlCode, const std::string& msg, const std::string receive_join_id, const std::string join_id) {
            if (recvObj) {
                CustomOnHttpResMsgEvent* event = new CustomOnHttpResMsgEvent(CustomEvent_VSSAgreeInvite);
                if (event) {
                    event->mCode = libCurlCode;
                    event->mMsg = QString::fromStdString(msg);
                    event->mRecvUserId = QString::fromStdString(receive_join_id);
                    event->mIsAgree = true;
                    QCoreApplication::postEvent(recvObj, event, Qt::LowEventPriority);
                }
            }
        });
    }
    else {
        QString domain = ConfigSetting::ReadString(CPathManager::GetToolConfigPath(), GROUP_DEFAULT, KEY_URL_DOMAIN, domainUrl);
        QString url = QString("%1/api/client/v1/interact/reply-invite?type=%2&token=%3").arg(domain).arg(ResponseInviteType_Accept).arg(mStartLiveData.msg_token);
        TRACE6("%s  url:%s\n", __FUNCTION__, url.toStdString().c_str());
        HTTP_GET_REQUEST request(url.toStdString());
        GetHttpManagerInstance()->HttpGetRequest(request, [&, recvObj, joinUid](const std::string& msg, int libCurlCode, const std::string userData) {
            if (recvObj) {
                CustomOnHttpResMsgEvent* event = new CustomOnHttpResMsgEvent(CustomEvent_VSSAgreeInvite);
                if (event) {
                    event->mCode = libCurlCode;
                    event->mMsg = QString::fromStdString(msg);
                    event->mRecvUserId = joinUid;
                    event->mIsAgree = true;
                    TRACE6("%s  msg:%s\n", __FUNCTION__, msg.c_str());
                    QCoreApplication::postEvent(recvObj, event, Qt::LowEventPriority);
                }
            }
        });
    }
}

void InteractAPIManager::HttpSendRejectInvite() {
    //拒绝上麦邀请 
    QObject* recvObj = mpMainUILogic;
    QString joinUid = mRespInitData.join_uid;
    if (mRespInitData.player == H5_LIVE_CREATE_TYPE) {
        TRACE6("%s HttpSendRejectInvite\n", __FUNCTION__);
        QString domain = ConfigSetting::ReadString(CPathManager::GetToolConfigPath(), GROUP_DEFAULT, KEY_VSS_URL_DOMAIN, vss_domainUrl);
        VSSAPIManager vssApi;
        vssApi.InitAPIRequest(domain.toStdString(), mRespInitData.vss_token.toStdString(), mRespInitData.vss_room_id.toStdString());
        vssApi.VSSRejectInvite(joinUid.toStdString(), joinUid.toStdString(), [&, recvObj](int libCurlCode, const std::string& msg, const std::string receive_join_id, const std::string join_id) {
            if (recvObj) {
                CustomOnHttpResMsgEvent* event = new CustomOnHttpResMsgEvent(CustomEvent_VSSAgreeInvite);
                if (event) {
                    event->mCode = libCurlCode;
                    event->mMsg = QString::fromStdString(msg);
                    event->mIsAgree = false;
                    QCoreApplication::postEvent(recvObj, event, Qt::LowEventPriority);
                }
            }
        });
    }
    else {
        QString domain = ConfigSetting::ReadString(CPathManager::GetToolConfigPath(), GROUP_DEFAULT, KEY_URL_DOMAIN, domainUrl);
        QString url = QString("%1/api/client/v1/interact/reply-invite?type=%2&token=%3").arg(domain).arg(ResponseInviteType_Refuse).arg(mStartLiveData.msg_token);
        TRACE6("%s  url:%s\n", __FUNCTION__, url.toStdString().c_str());
        HTTP_GET_REQUEST request(url.toStdString());
        GetHttpManagerInstance()->HttpGetRequest(request, [&, recvObj](const std::string& msg, int libCurlCode, const std::string userData) {
            if (recvObj) {
                CustomOnHttpResMsgEvent* event = new CustomOnHttpResMsgEvent(CustomEvent_VSSAgreeInvite);
                if (event) {
                    event->mCode = libCurlCode;
                    event->mMsg = QString::fromStdString(msg);
                    event->mIsAgree = false;
                    TRACE6("%s msg:%s\n", __FUNCTION__, msg.c_str());
                    QCoreApplication::postEvent(recvObj, event, Qt::LowEventPriority);
                }
            }
        });
    }
}

void InteractAPIManager::HttpSendCancelApplyToSpeak() {

   QObject* recvObj = mpMainUILogic;
   QString joinUid = mRespInitData.join_uid;
   if (mRespInitData.player == H5_LIVE_CREATE_TYPE) {//h5
      TRACE6("%s\n", __FUNCTION__);
      QString domain = ConfigSetting::ReadString(CPathManager::GetToolConfigPath(), GROUP_DEFAULT, KEY_VSS_URL_DOMAIN, vss_domainUrl);
      VSSAPIManager vssApi;
      vssApi.InitAPIRequest(domain.toStdString(), mRespInitData.vss_token.toStdString(), mRespInitData.vss_room_id.toStdString());
      vssApi.VSSCancleApply([&, recvObj](int libCurlCode, const std::string& msg, const std::string userData) {
         if (recvObj) {
            CustomOnHttpResMsgEvent* event = new CustomOnHttpResMsgEvent(CustomEvent_VSSCancleApply);
            if (event) {
               event->mCode = libCurlCode;
               event->mMsg = QString::fromStdString(msg);
               QCoreApplication::postEvent(recvObj, event, Qt::LowEventPriority);
            }
         }
      });
   }
   else { //flash
      QString domain = ConfigSetting::ReadString(CPathManager::GetToolConfigPath(), GROUP_DEFAULT, KEY_URL_DOMAIN, domainUrl);
      QString url = QString("%1/api/client/v1/interact/handsup?type=0&token=%2").arg(domain).arg(mStartLiveData.msg_token);
      TRACE6("%s tospeak url:%s\n", __FUNCTION__, url.toStdString().c_str());
      HTTP_GET_REQUEST request(url.toStdString());
      GetHttpManagerInstance()->HttpGetRequest(request, [&, recvObj](const std::string& msg, int libCurlCode, const std::string userData) {
         if (recvObj) {
            CustomOnHttpResMsgEvent* event = new CustomOnHttpResMsgEvent(CustomEvent_VSSCancleApply);
            if (event) {
               event->mCode = libCurlCode;
               event->mMsg = QString::fromStdString(msg);
               TRACE6("%s msg:%s\n", __FUNCTION__, msg.c_str());
               QCoreApplication::postEvent(recvObj, event, Qt::LowEventPriority);
            }
         }
      });
   }

}

void InteractAPIManager::HttpSendApplyToSpeak() {
    QObject* recvObj = mpMainUILogic;
    QString joinUid = mRespInitData.join_uid;
    if (mRespInitData.player == H5_LIVE_CREATE_TYPE) {
        TRACE6("%s\n", __FUNCTION__);
        QString domain = ConfigSetting::ReadString(CPathManager::GetToolConfigPath(), GROUP_DEFAULT, KEY_VSS_URL_DOMAIN, vss_domainUrl);
        VSSAPIManager vssApi;
        vssApi.InitAPIRequest(domain.toStdString(), mRespInitData.vss_token.toStdString(), mRespInitData.vss_room_id.toStdString());
        vssApi.VSSApply([&, recvObj](int libCurlCode, const std::string& msg, const std::string userData) {
            if (recvObj) {
                CustomOnHttpResMsgEvent* event = new CustomOnHttpResMsgEvent(CustomEvent_VSSApply);
                if (event) {
                    event->mCode = libCurlCode;
                    event->mMsg = QString::fromStdString(msg);
                    QCoreApplication::postEvent(recvObj, event, Qt::LowEventPriority);
                }
            }
        });
    }
    else {
        QString domain = ConfigSetting::ReadString(CPathManager::GetToolConfigPath(), GROUP_DEFAULT, KEY_URL_DOMAIN, domainUrl);
        QString url = QString("%1/api/client/v1/interact/handsup?type=1&token=%2").arg(domain).arg(mStartLiveData.msg_token);
        TRACE6("%s tospeak url:%s\n", __FUNCTION__, url.toStdString().c_str());
        HTTP_GET_REQUEST request(url.toStdString());
        GetHttpManagerInstance()->HttpGetRequest(request, [&, recvObj](const std::string& msg, int libCurlCode, const std::string userData) {
            if (recvObj) {
                CustomOnHttpResMsgEvent* event = new CustomOnHttpResMsgEvent(CustomEvent_VSSApply);
                if (event) {
                    event->mCode = libCurlCode;
                    event->mMsg = QString::fromStdString(msg);
                    TRACE6("%s msg:%s\n", __FUNCTION__, msg.c_str());
                    QCoreApplication::postEvent(recvObj, event, Qt::LowEventPriority);
                }
            }
        });
    }
}

void InteractAPIManager::HttpSendAgreeToSpeak(const QString& strId) {
    QObject* recvObj = mpMainUILogic;
    QString joinUid = mRespInitData.join_uid;
    if (mRespInitData.player == H5_LIVE_CREATE_TYPE) {
        TRACE6("%s HttpSendAgreeToSpeak\n", __FUNCTION__);
        QString domain = ConfigSetting::ReadString(CPathManager::GetToolConfigPath(), GROUP_DEFAULT, KEY_VSS_URL_DOMAIN, vss_domainUrl);
        VSSAPIManager vssApi;
        vssApi.InitAPIRequest(domain.toStdString(), mRespInitData.vss_token.toStdString(), mRespInitData.vss_room_id.toStdString());
        vssApi.VSSAgreeApply(strId.toStdString(), joinUid.toStdString(), [&, recvObj](int libCurlCode, const std::string& msg, const std::string receive_join_id, const std::string join_id) {
            if (recvObj) {
                CustomOnHttpResMsgEvent* event = new CustomOnHttpResMsgEvent(CustomEvent_VSSAgreeApply);
                if (event) {
                    event->mCode = libCurlCode;
                    event->mMsg = QString::fromStdString(msg);
                    event->mRecvUserId = QString::fromStdString(receive_join_id);
                    event->mUid = QString::fromStdString(join_id);
                    QCoreApplication::postEvent(recvObj, event, Qt::LowEventPriority);
                }
            }
        });
    }
    else {
        QString domain = ConfigSetting::ReadString(CPathManager::GetToolConfigPath(), GROUP_DEFAULT, KEY_URL_DOMAIN, domainUrl);
        QString url = QString("%1/api/client/v1/interact/agree-speak?receive_uid=%2&token=%3").arg(domain).arg(strId).arg(mStartLiveData.msg_token);
        TRACE6("%s url:%s\n", __FUNCTION__, url.toStdString().c_str());
        HTTP_GET_REQUEST request(url.toStdString());
        GetHttpManagerInstance()->HttpGetRequest(request, [&, strId, recvObj, joinUid](const std::string& msg, int libCurlCode, const std::string userData) {
            if (recvObj) {
                CustomOnHttpResMsgEvent* event = new CustomOnHttpResMsgEvent(CustomEvent_VSSAgreeApply);
                if (event) {
                    event->mCode = libCurlCode;
                    event->mMsg = QString::fromStdString(msg);
                    event->mRecvUserId = strId;
                    event->mUid = joinUid;
                    TRACE6("%s msg:%s\n", __FUNCTION__, msg.c_str());
                    QCoreApplication::postEvent(recvObj, event, Qt::LowEventPriority);
                }
            }
        });
    }
}

void InteractAPIManager::HttpSendRejectSpeak(const QString& strId) {
    QObject* recvObj = mpMainUILogic;
    QString joinUid = mRespInitData.join_uid;
    //拒绝用户上麦
    if (mRespInitData.player == H5_LIVE_CREATE_TYPE) {
        TRACE6("%s VSSRejectInvite\n", __FUNCTION__);
        QString domain = ConfigSetting::ReadString(CPathManager::GetToolConfigPath(), GROUP_DEFAULT, KEY_VSS_URL_DOMAIN, vss_domainUrl);
        VSSAPIManager vssApi;
        vssApi.InitAPIRequest(domain.toStdString(), mRespInitData.vss_token.toStdString(), mRespInitData.vss_room_id.toStdString());
        vssApi.VSSRejectApply(strId.toStdString(), joinUid.toStdString(), [&, recvObj](int libCurlCode, const std::string& msg, const std::string receive_join_id, const std::string join_id) {
            if (recvObj) {
                CustomOnHttpResMsgEvent* event = new CustomOnHttpResMsgEvent(CustomEvent_VSSRejectApply);
                if (event) {
                    event->mCode = libCurlCode;
                    event->mMsg = QString::fromStdString(msg);
                    event->mRecvUserId = QString::fromStdString(receive_join_id);
                    event->mUid = QString::fromStdString(join_id);
                    QCoreApplication::postEvent(recvObj, event, Qt::LowEventPriority);
                }
            }
        });
    }
    else {
        TRACE6("%s url:reject-speak\n", __FUNCTION__);
        QString domain = ConfigSetting::ReadString(CPathManager::GetToolConfigPath(), GROUP_DEFAULT, KEY_URL_DOMAIN, domainUrl);
        QString url = QString("%1/api/client/v1/interact/reject-speak?receive_uid=%2&token=%3").arg(domain).arg(strId).arg(mStartLiveData.msg_token);
        TRACE6("%s url:%s\n", __FUNCTION__, url.toStdString().c_str());
        HTTP_GET_REQUEST request(url.toStdString());
        GetHttpManagerInstance()->HttpGetRequest(request, [&, strId, recvObj, joinUid](const std::string& msg, int libCurlCode, const std::string userData) {
            if (recvObj) {
                CustomOnHttpResMsgEvent* event = new CustomOnHttpResMsgEvent(CustomEvent_VSSRejectApply);
                if (event) {
                    event->mCode = libCurlCode;
                    event->mMsg = QString::fromStdString(msg);
                    event->mRecvUserId = strId;
                    event->mUid = joinUid;
                    TRACE6("%s msg:%s\n", __FUNCTION__, msg.c_str());
                    QCoreApplication::postEvent(recvObj, event, Qt::LowEventPriority);
                }
            }
        });
    }
}

void InteractAPIManager::HttpSendAllowRaiseHands(const bool enable) {
    QObject* recvObj = mpMainUILogic;
    QString joinUid = mRespInitData.join_uid;
    if (mRespInitData.player == H5_LIVE_CREATE_TYPE) {
        TRACE6("%s VSSSetHandsup\n", __FUNCTION__);
        QString domain = ConfigSetting::ReadString(CPathManager::GetToolConfigPath(), GROUP_DEFAULT, KEY_VSS_URL_DOMAIN, vss_domainUrl);
        VSSAPIManager vssApi;
        vssApi.InitAPIRequest(domain.toStdString(), mRespInitData.vss_token.toStdString(), mRespInitData.vss_room_id.toStdString());
        vssApi.VSSSetHandsup(joinUid.toStdString(), enable == true ? 1 : 0, [&, recvObj, enable](int libCurlCode, const std::string& msg, const std::string join_id, int status) {
            if (recvObj) {
                CustomOnHttpResMsgEvent* event = new CustomOnHttpResMsgEvent(CustomEvent_VSSSetHandsUp);
                if (event) {
                    event->mCode = libCurlCode;
                    event->mMsg = QString::fromStdString(msg);
                    event->is_open = enable;
                    QCoreApplication::postEvent(recvObj, event, Qt::LowEventPriority);
                }
            }
        });
    }
    else {
        QString confPath = CPathManager::GetConfigPath();
        QString domain = ConfigSetting::ReadString(CPathManager::GetToolConfigPath(), GROUP_DEFAULT, KEY_URL_DOMAIN, domainUrl);
        QString url = QString("%1/api/client/v1/interact/switch-handsup?type=%2&token=%3").arg(domain).arg(enable == true ? 1 : 0).arg(mStartLiveData.msg_token);
        TRACE6("%s url:%s\n", __FUNCTION__, url.toStdString().c_str());
        HTTP_GET_REQUEST request(url.toStdString());
        GetHttpManagerInstance()->HttpGetRequest(request, [&, recvObj, enable](const std::string& msg, int libCurlCode, const std::string userData) {
            if (recvObj) {
                CustomOnHttpResMsgEvent* event = new CustomOnHttpResMsgEvent(CustomEvent_VSSSetHandsUp);
                if (event) {
                    event->mCode = libCurlCode;
                    event->mMsg = QString::fromStdString(msg);
                    event->is_open = enable;
                    TRACE6("%s msg:%s\n", __FUNCTION__, msg.c_str());
                    QCoreApplication::postEvent(recvObj, event, Qt::LowEventPriority);
                }
            }
        });
    }
}

void InteractAPIManager::HttpSendSwitchSpeaker(QString receive_uid, QString role, int enableNotice) {
    QObject* recvObj = mpMainUILogic;
    QString joinUid = mRespInitData.join_uid;
    if (mRespInitData.player == H5_LIVE_CREATE_TYPE) {
        TRACE6("%s VSSSetMainScreen\n", __FUNCTION__);
        QString domain = ConfigSetting::ReadString(CPathManager::GetToolConfigPath(), GROUP_DEFAULT, KEY_VSS_URL_DOMAIN, vss_domainUrl);
        VSSAPIManager vssApi;
        vssApi.InitAPIRequest(domain.toStdString(), mRespInitData.vss_token.toStdString(), mRespInitData.vss_room_id.toStdString());
        vssApi.VSSSetMainScreen(receive_uid.toStdString(), joinUid.toStdString(), [&, role, recvObj](int libCurlCode, const std::string& msg, const std::string receive_join_id, const std::string join_id) {
            if (recvObj) {
                CustomOnHttpResMsgEvent* event = new CustomOnHttpResMsgEvent(CustomEvent_VSSSetMainScreenResp);
                event->mCode = libCurlCode;
                event->mMsg = QString::fromStdString(msg);
                event->mRecvUserId = QString::fromStdString(receive_join_id);
                event->mUid = QString::fromStdString(join_id);
                event->mRoleName = role;
                QCoreApplication::postEvent(recvObj, event, Qt::LowEventPriority);
            }
        });
    }
    else {
        QString domain = ConfigSetting::ReadString(CPathManager::GetToolConfigPath(), GROUP_DEFAULT, KEY_URL_DOMAIN, domainUrl);
        QString url = QString("%1/api/client/v1/interact/switch-speaker?receive_uid=%2&token=%3").arg(domain).arg(receive_uid).arg(mStartLiveData.msg_token);
        TRACE6("%s url:%s\n", __FUNCTION__, url.toStdString().c_str());

        HTTP_GET_REQUEST request(url.toStdString());
        GetHttpManagerInstance()->HttpGetRequest(request, [&, receive_uid, role, recvObj, joinUid](const std::string& msg, int libCurlCode, const std::string userData) {
            if (recvObj) {
                CustomOnHttpResMsgEvent* event = new CustomOnHttpResMsgEvent(CustomEvent_VSSSetMainScreenResp);
                if (event) {
                    event->mCode = libCurlCode;
                    event->mMsg = QString::fromStdString(msg);
                    event->mRecvUserId = receive_uid;
                    event->mUid = joinUid;
                    event->mRoleName = role;
                    TRACE6("%s msg:%s\n", __FUNCTION__, msg.c_str());
                    QCoreApplication::postEvent(recvObj, event, Qt::LowEventPriority);
                }
            }
        });
    }
}

void InteractAPIManager::HttpSendDeviceStatus(QString receiveJoinId, int device, int type) {
    //此接口会影响上麦列表中 video 状态。
    QObject* recvObj = mpMainUILogic;
    QString joinUid = mRespInitData.join_uid;
    if (mRespInitData.player == H5_LIVE_CREATE_TYPE) {
        QString domain = ConfigSetting::ReadString(CPathManager::GetToolConfigPath(), GROUP_DEFAULT, KEY_VSS_URL_DOMAIN, vss_domainUrl);
        VSSAPIManager vssApi;
        vssApi.InitAPIRequest(domain.toStdString(), mRespInitData.vss_token.toStdString(), mRespInitData.vss_room_id.toStdString());
        vssApi.VSSSetDeviceStatus(receiveJoinId.toStdString(), joinUid.toStdString(), device, type, [&](int libCurlCode, const std::string& msg, const std::string receive_join_id, const std::string join_id, int device, int status) {
            TRACE6("%s msg:%s libCurlCode:%d\n", __FUNCTION__, msg.c_str(), libCurlCode);
        });
    }
    else {
        QString domain = ConfigSetting::ReadString(CPathManager::GetToolConfigPath(), GROUP_DEFAULT, KEY_URL_DOMAIN, domainUrl);
        QString url = QString("%1/api/client/v1/interact/switch-device?device=%2&type=%3&receive_uid=%4&token=%5").arg(domain).arg(device).arg(type).arg(receiveJoinId).arg(mStartLiveData.msg_token);
        TRACE6("%s url:%s\n", __FUNCTION__, url.toStdString().c_str());
        HTTP_GET_REQUEST request(url.toStdString());
        request.mbIsNeedSyncWork = true;
        GetHttpManagerInstance()->HttpGetRequest(request, [&](const std::string& msg, int libCurlCode, const std::string userData) {
            TRACE6("%s msg:%s libCurlCode:%d\n", __FUNCTION__, msg.c_str(), libCurlCode);
        });
    }
}

void InteractAPIManager::HttpSendSetDevice(const int device_status) {
    QObject* recvObj = mpMainUILogic;
    QString joinUid = mRespInitData.join_uid;
    if (mRespInitData.player == H5_LIVE_CREATE_TYPE) {
        QString domain = ConfigSetting::ReadString(CPathManager::GetToolConfigPath(), GROUP_DEFAULT, KEY_VSS_URL_DOMAIN, vss_domainUrl);
        VSSAPIManager vssApi;
        vssApi.InitAPIRequest(domain.toStdString(), mRespInitData.vss_token.toStdString(), mRespInitData.vss_room_id.toStdString());
        vssApi.VSSSetDevice(device_status, 2,[&](int libCurlCode, const std::string& msg, const std::string userData) {
            TRACE6("%s msg:%s libCurlCode:%d\n", __FUNCTION__, msg.c_str(), libCurlCode);
        });
    }
    else {
       QString domain = ConfigSetting::ReadString(CPathManager::GetToolConfigPath(), GROUP_DEFAULT, KEY_URL_DOMAIN, domainUrl);
       QString url = QString("%1/api/client/v1/interact/device-check?webinar_id=%2&device_type=2&join_uid=%3&token=%4&device_status=%5")
            .arg(domain)
            .arg(mStartLiveData.stream_name)
            .arg(joinUid)
            .arg(mStartLiveData.msg_token)
            .arg(device_status);
       TRACE6("%s url:%s\n", __FUNCTION__, url.toStdString().c_str());
       HTTP_GET_REQUEST request(url.toStdString());
       request.mbIsNeedSyncWork = true;
       GetHttpManagerInstance()->HttpGetRequest(request, [&](const std::string& msg, int libCurlCode, const std::string userData) {
          TRACE6("%s msg:%s libCurlCode:%d\n", __FUNCTION__, msg.c_str(), libCurlCode);
       });
    }
}

void InteractAPIManager::HttpSendSetDocPermission(const QString& strId){
    QObject* recvObj = mpMainUILogic;
    QString joinUid = mRespInitData.join_uid;
    if (mRespInitData.player == H5_LIVE_CREATE_TYPE) {
        QString domain = ConfigSetting::ReadString(CPathManager::GetToolConfigPath(), GROUP_DEFAULT, KEY_VSS_URL_DOMAIN, vss_domainUrl);
        VSSAPIManager vssApi;
        vssApi.InitAPIRequest(domain.toStdString(), mRespInitData.vss_token.toStdString(), mRespInitData.vss_room_id.toStdString());
        vssApi.VSSSetDocPermission(strId.toStdString(), joinUid.toStdString(), [&](int libCurlCode, const std::string& msg, const std::string receive_join_id, const std::string join_id) {
            TRACE6("%s msg:%s libCurlCode:%d\n", __FUNCTION__, msg.c_str(), libCurlCode);
        });
    }
}


void InteractAPIManager::HttpSendMixLayOutAndProfile(QString strLayout, QString videoProfile) {
    QObject* recvObj = mpMainUILogic;
    QString joinUid = mRespInitData.join_uid;
    if (mRespInitData.player == H5_LIVE_CREATE_TYPE) {
        QString domain = ConfigSetting::ReadString(CPathManager::GetToolConfigPath(), GROUP_DEFAULT, KEY_VSS_URL_DOMAIN, vss_domainUrl);
        VSSAPIManager vssApi;
        vssApi.InitAPIRequest(domain.toStdString(), mRespInitData.vss_token.toStdString(), mRespInitData.vss_room_id.toStdString());
        vssApi.VSSSetStream(strLayout.toStdString(), videoProfile.toStdString(), [&, recvObj](int libCurlCode, const std::string& msg, const std::string layout, const std::string definition) {
            TRACE6("%s code:%d msg:%s\n", __FUNCTION__, libCurlCode, msg.c_str());
        });
    }
    else {
        QString domain = ConfigSetting::ReadString(CPathManager::GetToolConfigPath(), GROUP_DEFAULT, KEY_URL_DOMAIN, domainUrl);
        QString url = QString("%1/api/client/v1/interact/set-watch-layout?layout=%2&definition=%4&token=%3").arg(domain).arg(strLayout).arg(mStartLiveData.msg_token).arg(videoProfile);
        TRACE6("%s url:%s\n", __FUNCTION__, url.toStdString().c_str());
        HTTP_GET_REQUEST request(url.toStdString());
        GetHttpManagerInstance()->HttpGetRequest(request, [&, strLayout, videoProfile, recvObj](const std::string& msg, int libCurlCode, const std::string userData) {
            TRACE6("%s code:%d msg:%s\n", __FUNCTION__, libCurlCode, msg.c_str());
        });
    }
}

void InteractAPIManager::HttpSendGetSpeakUserList() {
    QObject* recvObj = mpMainUILogic;
    QString joinUid = mRespInitData.join_uid;
    if (mRespInitData.player == H5_LIVE_CREATE_TYPE) {
        QString domain = ConfigSetting::ReadString(CPathManager::GetToolConfigPath(), GROUP_DEFAULT, KEY_VSS_URL_DOMAIN, vss_domainUrl);
        VSSAPIManager vssApi;
        vssApi.InitAPIRequest(domain.toStdString(), mRespInitData.vss_token.toStdString(), mRespInitData.vss_room_id.toStdString());
        vssApi.VSSGetRoomAttributes([&, recvObj](int libCurlCode, const std::string& msg, const std::string userData) {
            if (recvObj) {
                CustomOnHttpResMsgEvent *eventMsg = new CustomOnHttpResMsgEvent(CustomEvent_VSSGetRoomSpeakList);
                if (eventMsg) {
                    eventMsg->mMsg = QString::fromStdString(msg);
                    eventMsg->mCode = libCurlCode;
                    QCoreApplication::postEvent(recvObj, eventMsg);
                }
            }
        });
    }
    else {
        QString confPath = CPathManager::GetConfigPath();
        QString domain = ConfigSetting::ReadString(CPathManager::GetToolConfigPath(), GROUP_DEFAULT, KEY_URL_DOMAIN, domainUrl);
        QString url = QString("%1/api/client/v1/interact/speaker-list?token=%2").arg(domain).arg(mStartLiveData.msg_token);
        TRACE6("%s url:%s\n", __FUNCTION__, url.toStdString().c_str());

        HTTP_GET_REQUEST request(url.toStdString());
        GetHttpManagerInstance()->HttpGetRequest(request, [&, recvObj](const std::string& msg, int libCurlCode, const std::string userData) {
            if (recvObj) {
                CustomOnHttpResMsgEvent *eventMsg = new CustomOnHttpResMsgEvent(CustomEvent_VSSGetRoomSpeakList);
                if (eventMsg) {
                    eventMsg->mMsg = QString::fromStdString(msg);
                    eventMsg->mCode = libCurlCode;
                    TRACE6("%s msg:%s\n", __FUNCTION__, msg.c_str());
                    QCoreApplication::postEvent(recvObj, eventMsg);
                }
            }
        });
    }
}

void InteractAPIManager::HttpSendChangeWebWatchLayout(int status) {
    QObject* recvObj = mpMainUILogic;
    QString joinUid = mRespInitData.join_uid;
    if (mRespInitData.player == H5_LIVE_CREATE_TYPE) {
        QString domain = ConfigSetting::ReadString(CPathManager::GetToolConfigPath(), GROUP_DEFAULT, KEY_VSS_URL_DOMAIN, vss_domainUrl);
        VSSAPIManager vssApi;
        vssApi.InitAPIRequest(domain.toStdString(), mRespInitData.vss_token.toStdString(), mRespInitData.vss_room_id.toStdString());
        vssApi.HttpSendChangeWebWatchLayout(status, [&, recvObj](int libCurlCode, const std::string& msg, const std::string userData) {
            TRACE6("%s msg:%s\n", __FUNCTION__, msg.c_str());
        });
    }
    else {
        QString confPath = CPathManager::GetConfigPath();
        QString domain = ConfigSetting::ReadString(CPathManager::GetToolConfigPath(), GROUP_DEFAULT, KEY_URL_DOMAIN, domainUrl);
        QString url = QString("%1/api/client/v1/clientapi/desktop?token=%2&status=%3").arg(domain).arg(mStartLiveData.msg_token).arg(status);
        TRACE6("%s url:%s\n", __FUNCTION__, url.toStdString().c_str());
        HTTP_GET_REQUEST request(url.toStdString());
        request.SetHttpPost(true);
        GetHttpManagerInstance()->HttpGetRequest(request, [&, recvObj](const std::string& msg, int libCurlCode, const std::string userData) {
            TRACE6("%s msg:%s\n", __FUNCTION__, msg.c_str());
        });
    }
}

void InteractAPIManager::HttpSendFeedBack(int type, QString context) {
    QString domain = ConfigSetting::ReadString(CPathManager::GetToolConfigPath(), GROUP_DEFAULT, KEY_URL_DOMAIN, domainUrl);
    QString httpUrl = domain + QString("/api/client/v1/util/feed-back?report_type=%1&report_content=%2&webinar_id=%3").arg(type).arg(context).arg(mStartLiveData.stream_name);
    QObject* recvObj = mpMainUILogic;
    QString confPath = CPathManager::GetConfigPath();
    TRACE6("%s enter url:%s\n", __FUNCTION__, httpUrl.toStdString().c_str());
    //代理配置
    QString configPath = CPathManager::GetConfigPath();
    QString proxyHost;
    QString proxyUsr;
    QString proxyPwd;
    int proxyPort;
    int is_http_proxy = ConfigSetting::ReadInt(configPath, GROUP_DEFAULT, PROXY_OPEN, 0);
    if (is_http_proxy) {
        TRACE6("%s is_http_proxy:%d\n", __FUNCTION__, is_http_proxy);
        proxyHost = ConfigSetting::ReadString(configPath, GROUP_DEFAULT, PROXY_HOST, "");
        proxyPort = ConfigSetting::ReadInt(configPath, GROUP_DEFAULT, PROXY_PORT, 80);
        proxyUsr = ConfigSetting::ReadString(configPath, GROUP_DEFAULT, PROXY_USERNAME, "");
        proxyPwd = ConfigSetting::ReadString(configPath, GROUP_DEFAULT, PROXY_PASSWORD, "");
    }
    HTTP_GET_REQUEST request(httpUrl.toStdString());
    GetHttpManagerInstance()->HttpGetRequest(request, [&](const std::string& msg, int libCurlCode, const std::string userData) {

    });
}

void InteractAPIManager::HttpSendDispatchPublish(const QString& url, int dwPublishState, bool mediaCoreStreamErr, bool isAlreadyLive) {
    QObject* recvObj = mpMainUILogic;
    QString confPath = CPathManager::GetConfigPath();
    QString httpUrl = url;
    TRACE6("%s enter url:%s\n", __FUNCTION__, httpUrl.toStdString().c_str());
    //代理配置
    QString configPath = CPathManager::GetConfigPath();
    QString proxyHost;
    QString proxyUsr;
    QString proxyPwd;
    int proxyPort;
    int is_http_proxy = ConfigSetting::ReadInt(configPath, GROUP_DEFAULT, PROXY_OPEN, 0);
    if (is_http_proxy) {
        TRACE6("%s is_http_proxy:%d\n", __FUNCTION__, is_http_proxy);
        proxyHost = ConfigSetting::ReadString(configPath, GROUP_DEFAULT, PROXY_HOST, "");
        proxyPort = ConfigSetting::ReadInt(configPath, GROUP_DEFAULT, PROXY_PORT, 80);
        proxyUsr = ConfigSetting::ReadString(configPath, GROUP_DEFAULT, PROXY_USERNAME, "");
        proxyPwd = ConfigSetting::ReadString(configPath, GROUP_DEFAULT, PROXY_PASSWORD, "");
    }
    HTTP_GET_REQUEST request(httpUrl.toStdString());
    GetHttpManagerInstance()->HttpGetRequest(request, [&, recvObj, dwPublishState, mediaCoreStreamErr, isAlreadyLive](const std::string& msg, int libCurlCode, const std::string userData) {
        if (recvObj) {
            CustomOnHttpResMsgEvent* event = new CustomOnHttpResMsgEvent(CustomEvent_Dispatch_Publish);
            if (event) {
                event->mCode = libCurlCode;
                event->mMsg = QString::fromStdString(msg);
                event->mediaCoreStreamErr = mediaCoreStreamErr;
                event->bIsActiveLive = isAlreadyLive;
                event->status = dwPublishState;
                TRACE6("%s msg:%s\n", __FUNCTION__, msg.c_str());
                QCoreApplication::postEvent(recvObj, event, Qt::LowEventPriority);
            }
        }
    });
}


void InteractAPIManager::ParamHttpResp(std::string msg, int& code, std::string& respMsgOut) {
    int nCode = 0;
    QString respMsg;
    QJsonParseError json_error;
    QJsonArray speak_array;
    int arraySize = 0;
    QString name;
    QJsonDocument parse_doucment = QJsonDocument::fromJson(QByteArray(msg.c_str()), &json_error);
    if (json_error.error == QJsonParseError::NoError) {
        if (parse_doucment.isObject()) {
            QJsonObject obj = parse_doucment.object();
            if (obj.contains("code") && obj["code"].isString()) {
                nCode = obj["code"].toString().toInt();
            }
            else if (obj.contains("code")) {
                nCode = obj["code"].toInt();
            }
            if (obj.contains("msg") && obj["msg"].isString()) {
                respMsg = obj["msg"].toString();
            }
        }
    }
    if (nCode != 200) {
        if (respMsg.isEmpty()) {
            respMsg = NETWORK_REQUEST_ERR;
        }
    }
    code = nCode;
    respMsgOut = respMsg.toStdString();
}