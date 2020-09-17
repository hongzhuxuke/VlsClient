#include "VSSAPIManager.h"
#include "ConfigSetting.h"
#include "pathManager.h"
#include "pathManage.h"
#include "DebugTrace.h"
#include "PublishInfo.h"
#include "Msg_CommonToolKit.h"
#include "ICommonData.h"
#include "VH_Macro.h"
#include "MainUIIns.h"
#include "Msg_MainUI.h"
#include "CRPluginDef.h"
VSSAPIManager::VSSAPIManager(QObject *parent)
    : QObject(parent)
{
}

VSSAPIManager::~VSSAPIManager()
{
}

void VSSAPIManager::InitAPIRequest(const std::string domain, const std::string vss_token, const std::string room_id) {
    mDomain = QString::fromStdString(domain);
    mVssToken = QString::fromStdString(vss_token);
    mRoomId = QString::fromStdString(room_id);
}

HTTP_GET_REQUEST VSSAPIManager::GetHttpRequest(const std::string url, bool post /*= false*/) {
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
    HTTP_GET_REQUEST request(url);
    request.SetHttpPost(true);
    return request;
}

//开始直播
void VSSAPIManager::VSSStartLive(VSSEventCallback callbackFun) {
    QString httpUrl = mDomain + vss_start_live + QString("?vss_token=%1&room_id=%2").arg(mVssToken).arg(mRoomId);
    TRACE6("%s url:%s\n", __FUNCTION__, httpUrl.toStdString().c_str());
    HTTP_GET_REQUEST httpRequest = GetHttpRequest(httpUrl.toStdString());
    httpRequest.SetHttpPost(true);
    GetHttpManagerInstance()->HttpGetRequest(httpRequest, [&, callbackFun](const std::string& msg, int code, const std::string userData) {
        callbackFun(code, msg, userData);
        TRACE6("%s code:%d msg:%s\n", __FUNCTION__, code, msg.c_str());
    });

   /* VH::CComPtr<ICommonData> pCommonData;
    DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
    VSSGetRoomInfo vssRoomInfo;
    pCommonData->GetVSSRoomBaseInfo(vssRoomInfo);
    QString paas_domain = ConfigSetting::ReadString(CPathManager::GetToolConfigPath(), GROUP_DEFAULT, KEY_PAAS_DOMAIN, VALUE_PAAS_DOMAIN);

    std::string baseInfo = std::string("client=windows&app_id=") + vssRoomInfo.app_id.toStdString() + std::string("&third_party_user_id=") + vssRoomInfo.third_party_user_id.toStdString() + std::string("&access_token=") + vssRoomInfo.paas_access_token.toStdString() + std::string("&package_check=") + std::string("windows");
    std::string url = paas_domain.toStdString() +
       std::string("/inav/push-another-config?") + baseInfo +
       string("&inav_id=") + vssRoomInfo.inav_id.toStdString() +
       string("&room_id=") + vssRoomInfo.room_id.toStdString() +
       string("&type=") + to_string(1);
    TRACE6("urld:%s", url.c_str());
    HTTP_GET_REQUEST paas_httpRequest = GetHttpRequest(url);
    paas_httpRequest.SetHttpPost(true);
    GetHttpManagerInstance()->HttpGetRequest(paas_httpRequest, [&, callbackFun](const std::string& msg, int code, const std::string userData) {
       callbackFun(code, msg, userData);
       TRACE6("%s code:%d msg:%s\n", __FUNCTION__, code, msg.c_str());
    });*/
}

//结束直播
void VSSAPIManager::VSSStopLive(VSSEventCallback callbackFun) {
    QString httpUrl = mDomain + vss_end_live + QString("?vss_token=%1&room_id=%2").arg(mVssToken).arg(mRoomId);
    TRACE6("%s url:%s\n", __FUNCTION__, httpUrl.toStdString().c_str());
    HTTP_GET_REQUEST httpRequest = GetHttpRequest(httpUrl.toStdString());
    httpRequest.SetHttpPost(true);
    GetHttpManagerInstance()->HttpGetRequest(httpRequest, [&, callbackFun](const std::string& msg, int code, const std::string userData) {
        callbackFun(code, msg, userData);
        TRACE6("%s code:%d msg:%s\n", __FUNCTION__, code, msg.c_str());
    });
}

void VSSAPIManager::VSSGetPushInfo(VSSEventCallback callbackFun) {
    QString httpUrl = mDomain + vss_get_push_info + QString("?vss_token=%1&room_id=%2").arg(mVssToken).arg(mRoomId);
    TRACE6("%s url:%s\n", __FUNCTION__, httpUrl.toStdString().c_str());
    HTTP_GET_REQUEST httpRequest = GetHttpRequest(httpUrl.toStdString());
    httpRequest.SetHttpPost(true);
    GetHttpManagerInstance()->HttpGetRequest(httpRequest, [&, callbackFun](const std::string& msg, int code, const std::string userData) {
        callbackFun(code, msg, userData);
        TRACE6("%s code:%d msg:%s\n", __FUNCTION__, code, msg.c_str());
    });
}

//获取属性状态
void VSSAPIManager::VSSGetRoomAttributes(VSSEventCallback callbackFun) {
    QString httpUrl = mDomain + vss_get_attributes + QString("?vss_token=%1&room_id=%2").arg(mVssToken).arg(mRoomId);
    TRACE6("%s url:%s\n", __FUNCTION__, httpUrl.toStdString().c_str());
    HTTP_GET_REQUEST httpRequest = GetHttpRequest(httpUrl.toStdString());
    httpRequest.SetHttpPost(true);
    GetHttpManagerInstance()->HttpGetRequest(httpRequest, [&, callbackFun](const std::string& msg, int code, const std::string userData) {
        callbackFun(code, msg, userData);
        TRACE6("%s code:%d msg:%s\n", __FUNCTION__, code, msg.c_str());
    });
}

void VSSAPIManager::VSSGetRoomBaseInfo(VSSEventCallback callbackFun) {
    QString httpUrl = mDomain + vss_get + QString("?vss_token=%1&room_id=%2").arg(mVssToken).arg(mRoomId);
    TRACE6("%s url:%s\n", __FUNCTION__, httpUrl.toStdString().c_str());
    HTTP_GET_REQUEST httpRequest = GetHttpRequest(httpUrl.toStdString());
    httpRequest.SetHttpPost(true);
    GetHttpManagerInstance()->HttpGetRequest(httpRequest, [&, callbackFun](const std::string& msg, int code, const std::string userData) {
        callbackFun(code, msg, userData);
        TRACE6("%s code:%d\n", __FUNCTION__, code);
    });
}

void VSSAPIManager::VSSSetDevice(const int status, const int type, VSSEventCallback callbackFun) {
    QString httpUrl = mDomain + vss_set_device + QString("?vss_token=%1&room_id=%2&status=%3&type=%4").arg(mVssToken).arg(mRoomId).arg(status).arg(type);
    TRACE6("%s url:%s\n", __FUNCTION__, httpUrl.toStdString().c_str());
    HTTP_GET_REQUEST httpRequest = GetHttpRequest(httpUrl.toStdString());
    httpRequest.SetHttpPost(true);
    GetHttpManagerInstance()->HttpGetRequest(httpRequest, [&, callbackFun](const std::string& msg, int code, const std::string userData) {
        callbackFun(code, msg, userData);
        TRACE6("%s code:%d msg:%s\n", __FUNCTION__, code, msg.c_str());
    });
}

void VSSAPIManager::VSSSetStream(const std::string layout, const std::string definition, VSSSetStreamEventCallback callbackFun) {
    QString httpUrl = mDomain + vss_set_stream + QString("?vss_token=%1&room_id=%2&layout=%3&definition=%4").arg(mVssToken).arg(mRoomId).arg(QString::fromStdString(layout)).arg(QString::fromStdString(definition));
    TRACE6("%s url:%s\n", __FUNCTION__, httpUrl.toStdString().c_str());
    HTTP_GET_REQUEST httpRequest = GetHttpRequest(httpUrl.toStdString());
    httpRequest.SetHttpPost(true);
    GetHttpManagerInstance()->HttpGetRequest(httpRequest, [&, callbackFun, layout, definition](const std::string& msg, int code, const std::string userData) {
        callbackFun(code, msg, layout, definition);
        TRACE6("%s code:%d msg:%s\n", __FUNCTION__, code, msg.c_str());
    });
}

void VSSAPIManager::VSSAgreeApply(const std::string receive_join_id, const std::string join_id, VSSAgreeApplyEventCallback callbackFun) {
    QString httpUrl = mDomain + vss_agree_apply + QString("?vss_token=%1&room_id=%2&receive_account_id=%3&account_id=%4").arg(mVssToken).arg(mRoomId).arg(QString::fromStdString(receive_join_id)).arg(QString::fromStdString(join_id));
    TRACE6("%s url:%s\n", __FUNCTION__, httpUrl.toStdString().c_str());
    HTTP_GET_REQUEST httpRequest = GetHttpRequest(httpUrl.toStdString());
    httpRequest.SetHttpPost(true);
    GetHttpManagerInstance()->HttpGetRequest(httpRequest, [&, callbackFun, receive_join_id, join_id](const std::string& msg, int code, const std::string userData) {
        callbackFun(code, msg, receive_join_id, join_id);
        TRACE6("%s code:%d msg:%s\n", __FUNCTION__, code, msg.c_str());
    });
}

void VSSAPIManager::VSSGetUserSStatus(const std::string account_id, VSSEventCallback callbackFun) {
    QString httpUrl = mDomain + vss_get_user_status + QString("?vss_token=%1&room_id=%2&account_id=%3").arg(mVssToken).arg(mRoomId).arg(QString::fromStdString(account_id));
    TRACE6("%s url:%s\n", __FUNCTION__, httpUrl.toStdString().c_str());
    HTTP_GET_REQUEST httpRequest = GetHttpRequest(httpUrl.toStdString());
    httpRequest.SetHttpPost(true);
    GetHttpManagerInstance()->HttpGetRequest(httpRequest, [&, callbackFun, account_id](const std::string& msg, int code, const std::string userData) {
        callbackFun(code, msg, account_id);
        TRACE6("%s code:%d msg:%s\n", __FUNCTION__, code, msg.c_str());
    });
}

void VSSAPIManager::VSSGetBannedList(int page, int pagesize, VSSMemberListEventCallback callbackFun) {
    QString httpUrl = mDomain + vss_get_banned_list + QString("?vss_token=%1&room_id=%2&page=%3&pagesize=%4").arg(mVssToken).arg(mRoomId).arg(page).arg(pagesize);
    TRACE6("%s url:%s\n", __FUNCTION__, httpUrl.toStdString().c_str());
    HTTP_GET_REQUEST httpRequest = GetHttpRequest(httpUrl.toStdString());
    httpRequest.SetHttpPost(true);
    GetHttpManagerInstance()->HttpGetRequest(httpRequest, [&, callbackFun, page, pagesize](const std::string& msg, int code, const std::string userData) {
        callbackFun(code, msg, page, pagesize);
    });
}

//获取踢出用户列表
void VSSAPIManager::VSSGetKickedList(int page, int pagesize, VSSMemberListEventCallback callbackFun) {
    QString httpUrl = mDomain + vss_get_kicked_list + QString("?vss_token=%1&room_id=%2&page=%3&pagesize=%4").arg(mVssToken).arg(mRoomId).arg(page).arg(pagesize);
    TRACE6("%s url:%s\n", __FUNCTION__, httpUrl.toStdString().c_str());
    HTTP_GET_REQUEST httpRequest = GetHttpRequest(httpUrl.toStdString());
    httpRequest.SetHttpPost(true);
    GetHttpManagerInstance()->HttpGetRequest(httpRequest, [&, callbackFun, page, pagesize](const std::string& msg, int code, const std::string userData) {
        callbackFun(code, msg, page, pagesize);
    });
}
//获取在线用户列表
void VSSAPIManager::VSSGetOnlineList(int page, int pagesize, VSSMemberListEventCallback callbackFun) {
    QString httpUrl = mDomain + vss_get_online_list + QString("?vss_token=%1&room_id=%2&page=%3&pagesize=%4").arg(mVssToken).arg(mRoomId).arg(page).arg(pagesize);
    TRACE6("%s url:%s\n", __FUNCTION__, httpUrl.toStdString().c_str());
    HTTP_GET_REQUEST httpRequest = GetHttpRequest(httpUrl.toStdString());
    httpRequest.SetHttpPost(true);
    GetHttpManagerInstance()->HttpGetRequest(httpRequest, [&, callbackFun, page, pagesize](const std::string& msg, int code, const std::string userData) {
        callbackFun(code, msg, page, pagesize);
    });
}

void VSSAPIManager::VSSSearchOnLineMember(const std::string nickname, VSSMemberListEventCallback callback) {
   QString httpUrl = mDomain + vss_get_online_list + QString("?vss_token=%1&room_id=%2&page=%3&pagesize=%4&nickname=%5").arg(mVssToken).arg(mRoomId).arg(1).arg(20).arg(QString::fromStdString(nickname));
   TRACE6("%s url:%s\n", __FUNCTION__, httpUrl.toStdString().c_str());
   HTTP_GET_REQUEST httpRequest = GetHttpRequest(httpUrl.toStdString());
   httpRequest.SetHttpPost(true);
   GetHttpManagerInstance()->HttpGetRequest(httpRequest, [&, callback](const std::string& msg, int code, const std::string userData) {
      if (callback) {
         callback(code, msg,1,20);
      }
   });
}

//获取特殊用户列表
void VSSAPIManager::VSSGetSpecialList(int page, int pagesize, VSSMemberListEventCallback callbackFun) {
    QString httpUrl = mDomain + vss_get_special_list + QString("?vss_token=%1&room_id=%2&page=%3&pagesize=%4").arg(mVssToken).arg(mRoomId).arg(page).arg(pagesize);
    TRACE6("%s url:%s\n", __FUNCTION__, httpUrl.toStdString().c_str());
    HTTP_GET_REQUEST httpRequest = GetHttpRequest(httpUrl.toStdString());
    httpRequest.SetHttpPost(true);
    GetHttpManagerInstance()->HttpGetRequest(httpRequest, [&, callbackFun, page, pagesize](const std::string& msg, int code, const std::string userData) {
        callbackFun(code, msg, page, pagesize);
    });
}

void VSSAPIManager::VSSSetKicked(const std::string receive_join_id, int status, VSSMemberKickEventCallback callbackFun) {
    QString httpUrl = mDomain + vss_set_kicked + QString("?vss_token=%1&room_id=%2&receive_account_id=%3&status=%4").arg(mVssToken).arg(mRoomId).arg(QString::fromStdString(receive_join_id)).arg(status);
    TRACE6("%s url:%s\n", __FUNCTION__, httpUrl.toStdString().c_str());
    HTTP_GET_REQUEST httpRequest = GetHttpRequest(httpUrl.toStdString());
    httpRequest.SetHttpPost(true);
    GetHttpManagerInstance()->HttpGetRequest(httpRequest, [&, callbackFun, receive_join_id, status](const std::string& msg, int code, const std::string userData) {
        callbackFun(code, msg, receive_join_id, status);
        TRACE6("%s code:%d msg:%s\n", __FUNCTION__, code, msg.c_str());
    });
}

void VSSAPIManager::VSSSetMainScreen(const std::string receive_join_id, const std::string join_id, VSSSetMainViewEventCallback callbackFun) {
    QString httpUrl = mDomain + vss_set_main_screen + QString("?vss_token=%1&room_id=%2&receive_account_id=%3&account_id=%4").arg(mVssToken).arg(mRoomId).arg(QString::fromStdString(receive_join_id)).arg(QString::fromStdString(join_id));
    TRACE6("%s url:%s\n", __FUNCTION__, httpUrl.toStdString().c_str());

    HTTP_GET_REQUEST httpRequest = GetHttpRequest(httpUrl.toStdString());
    httpRequest.SetHttpPost(true);
    GetHttpManagerInstance()->HttpGetRequest(httpRequest, [&, callbackFun, receive_join_id, join_id](const std::string& msg, int code, const std::string userData) {
        callbackFun(code, msg, receive_join_id, join_id);
        TRACE6("%s code:%d msg:%s\n", __FUNCTION__, code, msg.c_str());
    });
}

void VSSAPIManager::VSSSetDocPermission(const std::string receive_join_id, const std::string join_id, VSSSetDocPermissionEventCallback callbackFun) {
    QString httpUrl = mDomain + vss_set_doc_permission + QString("?vss_token=%1&room_id=%2&receive_account_id=%3&account_id=%4").arg(mVssToken).arg(mRoomId).arg(QString::fromStdString(receive_join_id)).arg(QString::fromStdString(join_id));
    TRACE6("%s url:%s\n", __FUNCTION__, httpUrl.toStdString().c_str());

    HTTP_GET_REQUEST httpRequest = GetHttpRequest(httpUrl.toStdString());
    httpRequest.SetHttpPost(true);
    GetHttpManagerInstance()->HttpGetRequest(httpRequest, [&, callbackFun, receive_join_id, join_id](const std::string& msg, int code, const std::string userData) {
        callbackFun(code, msg, receive_join_id, join_id);
        TRACE6("%s code:%d msg:%s\n", __FUNCTION__, code, msg.c_str());
    });
}

void VSSAPIManager::VSSRejectApply(const std::string receive_join_id, const std::string join_id, VSSRejectApplyEventCallback callbackFun) {
    QString httpUrl = mDomain + vss_reject_apply + QString("?vss_token=%1&room_id=%2&receive_account_id=%3&account_id=%4").arg(mVssToken).arg(mRoomId).arg(QString::fromStdString(receive_join_id)).arg(QString::fromStdString(join_id));
    TRACE6("%s url:%s\n", __FUNCTION__, httpUrl.toStdString().c_str());

    HTTP_GET_REQUEST httpRequest = GetHttpRequest(httpUrl.toStdString());
    httpRequest.SetHttpPost(true);
    GetHttpManagerInstance()->HttpGetRequest(httpRequest, [&, callbackFun, receive_join_id, join_id](const std::string& msg, int code, const std::string userData) {
        callbackFun(code, msg, receive_join_id, join_id);
        TRACE6("%s code:%d msg:%s\n", __FUNCTION__, code, msg.c_str());
    });
}

void VSSAPIManager::VSSSetHandsup(const std::string join_id, int status, VSSSetHandsupEventCallback callbackFun) {
    QString httpUrl = mDomain + vss_set_handsup + QString("?vss_token=%1&room_id=%2&account_id=%3&status=%4").arg(mVssToken).arg(mRoomId).arg(QString::fromStdString(join_id)).arg(status);
    TRACE6("%s url:%s\n", __FUNCTION__, httpUrl.toStdString().c_str());

    HTTP_GET_REQUEST httpRequest = GetHttpRequest(httpUrl.toStdString());
    httpRequest.SetHttpPost(true);
    GetHttpManagerInstance()->HttpGetRequest(httpRequest, [&, callbackFun, join_id, status](const std::string& msg, int code, const std::string userData) {
        callbackFun(code, msg, join_id, status);
        TRACE6("%s code:%d msg:%s\n", __FUNCTION__, code, msg.c_str());
    });
}

void VSSAPIManager::VSSApply(VSSEventCallback callbackFun) {
    QString httpUrl = mDomain + vss_apply + QString("?vss_token=%1&room_id=%2").arg(mVssToken).arg(mRoomId);
    TRACE6("%s url:%s\n", __FUNCTION__, httpUrl.toStdString().c_str());

    HTTP_GET_REQUEST httpRequest = GetHttpRequest(httpUrl.toStdString());
    httpRequest.SetHttpPost(true);
    GetHttpManagerInstance()->HttpGetRequest(httpRequest, [&, callbackFun](const std::string& msg, int code, const std::string userData) {
        callbackFun(code, msg, userData);
        TRACE6("%s code:%d msg:%s\n", __FUNCTION__, code, msg.c_str());
    });
}

void VSSAPIManager::VSSCancleApply(VSSEventCallback callbackFun) {
   QString httpUrl = mDomain + vss_cancel_apply + QString("?vss_token=%1&room_id=%2").arg(mVssToken).arg(mRoomId);
   TRACE6("%s url:%s\n", __FUNCTION__, httpUrl.toStdString().c_str());

   HTTP_GET_REQUEST httpRequest = GetHttpRequest(httpUrl.toStdString());
   httpRequest.SetHttpPost(true);
   GetHttpManagerInstance()->HttpGetRequest(httpRequest, [&, callbackFun](const std::string& msg, int code, const std::string userData) {
      callbackFun(code, msg, userData);
      TRACE6("%s code:%d msg:%s\n", __FUNCTION__, code, msg.c_str());
   });
}

//邀请用户上麦
void VSSAPIManager::VSSInvite(const std::string receive_join_id, const std::string join_id, VSSInviteJoinEventCallback callbackFun) {
    QString httpUrl = mDomain + vss_invite + QString("?vss_token=%1&room_id=%2&receive_account_id=%3&account_id=%4").arg(mVssToken).arg(mRoomId).arg(QString::fromStdString(receive_join_id)).arg(QString::fromStdString(join_id));
    TRACE6("%s url:%s\n", __FUNCTION__, httpUrl.toStdString().c_str());

    HTTP_GET_REQUEST httpRequest = GetHttpRequest(httpUrl.toStdString());
    httpRequest.SetHttpPost(true);
    GetHttpManagerInstance()->HttpGetRequest(httpRequest, [&, callbackFun, receive_join_id, join_id](const std::string& msg, int code, const std::string userData) {
        callbackFun(code, msg, receive_join_id, join_id);
        TRACE6("%s code:%d msg:%s\n", __FUNCTION__, code, msg.c_str());
    });
}

void VSSAPIManager::VSSRejectInvite(const std::string receive_join_id, const std::string join_id, VSSRejectInviteEventCallback callbackFun) {
    QString httpUrl = mDomain + vss_reject_invite + QString("?vss_token=%1&room_id=%2&receive_account_id=%3&account_id=%4").arg(mVssToken).arg(mRoomId).arg(QString::fromStdString(receive_join_id)).arg(QString::fromStdString(join_id));
    TRACE6("%s url:%s\n", __FUNCTION__, httpUrl.toStdString().c_str());

    HTTP_GET_REQUEST httpRequest = GetHttpRequest(httpUrl.toStdString());
    httpRequest.SetHttpPost(true);
    GetHttpManagerInstance()->HttpGetRequest(httpRequest, [&, callbackFun, receive_join_id, join_id](const std::string& msg, int code, const std::string userData) {
        callbackFun(code, msg, receive_join_id, join_id);
        TRACE6("%s code:%d msg:%s\n", __FUNCTION__, code, msg.c_str());
    });
}

void VSSAPIManager::VSSSpeak(const std::string join_id, VSSSpeakEventCallBack callbackFun) {
    QString httpUrl = mDomain + vss_speak + QString("?vss_token=%1&room_id=%2&account_id=%3").arg(mVssToken).arg(mRoomId).arg(QString::fromStdString(join_id));
    TRACE6("%s url:%s\n", __FUNCTION__, httpUrl.toStdString().c_str());

    HTTP_GET_REQUEST httpRequest = GetHttpRequest(httpUrl.toStdString());
    httpRequest.SetHttpPost(true);
    GetHttpManagerInstance()->HttpGetRequest(httpRequest, [&, callbackFun, join_id](const std::string& msg, int code, const std::string userData) {
        callbackFun(code, msg, join_id);
        TRACE6("%s code:%d msg:%s\n", __FUNCTION__, code, msg.c_str());
    });
}

void VSSAPIManager::VSSNoSpeak(const std::string receive_join_id, VSSNoSpeakEventCallBack callbackFun) {
    QString httpUrl = mDomain + vss_nospeak + QString("?vss_token=%1&room_id=%2&receive_account_id=%3").arg(mVssToken).arg(mRoomId).arg(QString::fromStdString(receive_join_id));
    TRACE6("%s url:%s\n", __FUNCTION__, httpUrl.toStdString().c_str());

    HTTP_GET_REQUEST httpRequest = GetHttpRequest(httpUrl.toStdString());
    httpRequest.SetHttpPost(true);
    httpRequest.mbIsNeedSyncWork = true;
    GetHttpManagerInstance()->HttpGetRequest(httpRequest, [&, callbackFun, receive_join_id](const std::string& msg, int code, const std::string userData) {
        callbackFun(code, msg, receive_join_id);
        TRACE6("%s code:%d msg:%s\n", __FUNCTION__, code, msg.c_str());
    });
}

void VSSAPIManager::VSSAgreeInvite(const std::string receive_join_id, const std::string join_id, VSSAgreeInviteEventCallback callbackFun) {
    QString httpUrl = mDomain + vss_agree_invite + QString("?vss_token=%1&room_id=%2&receive_account_id=%3&account_id=%4").arg(mVssToken).arg(mRoomId).arg(QString::fromStdString(receive_join_id)).arg(QString::fromStdString(join_id));
    TRACE6("%s url:%s\n", __FUNCTION__, httpUrl.toStdString().c_str());

    HTTP_GET_REQUEST httpRequest = GetHttpRequest(httpUrl.toStdString());
    httpRequest.SetHttpPost(true);
    GetHttpManagerInstance()->HttpGetRequest(httpRequest, [&, callbackFun, receive_join_id, join_id](const std::string& msg, int code, const std::string userData) {
        callbackFun(code, msg, receive_join_id, join_id);
        TRACE6("%s code:%d msg:%s\n", __FUNCTION__, code, msg.c_str());
    });
}

void VSSAPIManager::VSSCancelApply(const std::string join_id, VSSEventCallback callbackFun) {
    QString httpUrl = mDomain + vss_cancel_apply + QString("?vss_token=%1&room_id=%2&account_id=%3").arg(mVssToken).arg(mRoomId).arg(QString::fromStdString(join_id));
    TRACE6("%s url:%s\n", __FUNCTION__, httpUrl.toStdString().c_str());

    HTTP_GET_REQUEST httpRequest = GetHttpRequest(httpUrl.toStdString());
    httpRequest.SetHttpPost(true);
    GetHttpManagerInstance()->HttpGetRequest(httpRequest, [&, callbackFun, join_id](const std::string& msg, int code, const std::string userData) {
        callbackFun(code, msg, join_id);
        TRACE6("%s code:%d msg:%s\n", __FUNCTION__, code, msg.c_str());
    });
}

void VSSAPIManager::VSSSetBanned(const std::string receive_join_id, int status, VSSMemberSetBannedEventCallback callbackFun) {
    QString httpUrl = mDomain + vss_set_banned + QString("?vss_token=%1&room_id=%2&receive_account_id=%3&status=%4").arg(mVssToken).arg(mRoomId).arg(QString::fromStdString(receive_join_id)).arg(status);
    TRACE6("%s url:%s\n", __FUNCTION__, httpUrl.toStdString().c_str());

    HTTP_GET_REQUEST httpRequest = GetHttpRequest(httpUrl.toStdString());
    httpRequest.SetHttpPost(true);
    GetHttpManagerInstance()->HttpGetRequest(httpRequest, [&, callbackFun, receive_join_id, status](const std::string& msg, int code, const std::string userData) {
        callbackFun(code, msg, receive_join_id, status);
        TRACE6("%s code:%d msg:%s\n", __FUNCTION__, code, msg.c_str());
    });
}

void VSSAPIManager::VSSSetDeviceStatus(const std::string receive_join_id, const std::string join_id, int device, int status, VSSSetDeviceStatusEventCallback callbackFun) {
    QString httpUrl = mDomain + vss_set_device_status + QString("?vss_token=%1&room_id=%2&receive_account_id=%3&account_id=%4&device=%5&status=%6").arg(mVssToken).arg(mRoomId).arg(QString::fromStdString(receive_join_id)).arg(QString::fromStdString(join_id)).arg(device).arg(status);
    TRACE6("%s url:%s\n", __FUNCTION__, httpUrl.toStdString().c_str());

    HTTP_GET_REQUEST httpRequest = GetHttpRequest(httpUrl.toStdString());
    httpRequest.SetHttpPost(true);
    httpRequest.mbIsNeedSyncWork = true;
    GetHttpManagerInstance()->HttpGetRequest(httpRequest, [&, callbackFun, receive_join_id, join_id, device, status](const std::string& msg, int code, const std::string userData) {
        callbackFun(code, msg, receive_join_id, join_id, device, status);
        TRACE6("%s code:%d msg:%s\n", __FUNCTION__, code, msg.c_str());
    });
}

void VSSAPIManager::VSSSendNotice(QString content, VSSEventCallback callbackFun) {
    QString httpUrl = mDomain + vss_send_notice + QString("?vss_token=%1&room_id=%2&content=%3").arg(mVssToken).arg(mRoomId).arg(content);
    TRACE6("%s url:%s\n", __FUNCTION__, httpUrl.toStdString().c_str());

    HTTP_GET_REQUEST httpRequest = GetHttpRequest(httpUrl.toStdString());
    httpRequest.SetHttpPost(true);
    GetHttpManagerInstance()->HttpGetRequest(httpRequest, [&, callbackFun](const std::string& msg, int code, const std::string userData) {
        callbackFun(code, msg, userData);
        TRACE6("%s code:%d msg:%s\n", __FUNCTION__, code, msg.c_str());
    });
}

void VSSAPIManager::HttpSendChangeWebWatchLayout(int status, VSSEventCallback callbackFun) {
    QString httpUrl = mDomain + vss_chage_web_watch_layout + QString("?vss_token=%1&room_id=%2&status=%3").arg(mVssToken).arg(mRoomId).arg(status);
    TRACE6("%s url:%s\n", __FUNCTION__, httpUrl.toStdString().c_str());

    HTTP_GET_REQUEST httpRequest = GetHttpRequest(httpUrl.toStdString());
    httpRequest.SetHttpPost(true);
    GetHttpManagerInstance()->HttpGetRequest(httpRequest, [&, callbackFun](const std::string& msg, int code, const std::string userData) {
        callbackFun(code, msg, userData);
        TRACE6("%s code:%d msg:%s\n", __FUNCTION__, code, msg.c_str());
    });
}