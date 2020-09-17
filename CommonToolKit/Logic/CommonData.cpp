#include "StdAfx.h"
#include <ShlObj.h>
#include <QJsonDocument>
#include <QJsonObject>
#include "CommonData.h"

#define USER_HOST "host"
#define USER_GUEST "guest"
#define USER_ASSISTANT "assistant"
#define USER_USER "user"

CommonData::CommonData()
: m_lRefCount(0) {
   m_ai64ReportID = 1;
   m_bStartPublish = false;
   mIStartMode = eStartMode_flashNoDispatch;
}

CommonData::~CommonData() {
   TRACE6("%s delete end\n", __FUNCTION__);
}

HRESULT STDMETHODCALLTYPE CommonData::QueryInterface(REFIID riid, void ** appvObject) {
   if (riid == IID_VHIUnknown) {
      *appvObject = (VH_IUnknown*)this;
      AddRef();
      return CRE_OK;
   } else if (IID_ICommonData == riid) {
      *appvObject = (ICommonData*)this;
      AddRef();
      return CRE_OK;
   }
   return CRE_NOINTERFACE;
}

ULONG STDMETHODCALLTYPE CommonData::AddRef(void) {
   return ::InterlockedIncrement(&m_lRefCount);
}

ULONG STDMETHODCALLTYPE CommonData::Release(void) {
   return ::InterlockedDecrement(&m_lRefCount);
}

void STDMETHODCALLTYPE CommonData::SetLoginRespInfo(const LoginRespInfo& loginRespInfo) {
   mLoginRespInfo = loginRespInfo;
}

LoginRespInfo STDMETHODCALLTYPE CommonData::GetLoginRespInfo() {
   return mLoginRespInfo;
}

void STDMETHODCALLTYPE CommonData::RegisterRuntimeInstance(QObject* obj) {
   mRuntimeInstanceObj = obj;
}

QObject* STDMETHODCALLTYPE CommonData::GetRuntimeInstanceObj() {
   return mRuntimeInstanceObj;
}

BOOL STDMETHODCALLTYPE CommonData::SetStreamInfo(PublishInfo* apStreamInfo) {
   m_oStreamInfo.mPubLineInfo.clear();
   m_oStreamInfo= *apStreamInfo;
   m_oStreamInfo.mPubLineInfo.clear();
   m_oStreamInfo.mUserName = apStreamInfo->mUserName;
   m_oStreamInfo.mToken = apStreamInfo->mToken;
   m_oStreamInfo.mStreamName = apStreamInfo->mStreamName;
   m_oStreamInfo.mChanelId = apStreamInfo->mChanelId;
   m_oStreamInfo.mRtmpUrlsStr = apStreamInfo->mRtmpUrlsStr;
   m_oStreamInfo.mCurRtmpUrl = apStreamInfo->mCurRtmpUrl;
   m_oStreamInfo.mCurRtmpUrlIndex = apStreamInfo->mCurRtmpUrlIndex;
   m_oStreamInfo.m_bHideLogo = apStreamInfo->m_bHideLogo;

   for (int i = 0; i < apStreamInfo->mPubLineInfo.size(); i++) {
      PubLineInfo pl;
      pl.mID = apStreamInfo->mPubLineInfo[i].mID;
      pl.mStrAlias = apStreamInfo->mPubLineInfo[i].mStrAlias;
      pl.mStrDomain = apStreamInfo->mPubLineInfo[i].mStrDomain;
      pl.mPingInfo = apStreamInfo->mPubLineInfo[i].mPingInfo;
      m_oStreamInfo.mPubLineInfo.insert(i, pl);
   }
   return TRUE;
}

BOOL STDMETHODCALLTYPE CommonData::GetStreamInfo(PublishInfo& oStreamInfo) {
   oStreamInfo = m_oStreamInfo;
   oStreamInfo.mPubLineInfo.clear();
   oStreamInfo.mUserName = m_oStreamInfo.mUserName;
   oStreamInfo.mToken = m_oStreamInfo.mToken;
   oStreamInfo.mStreamName = m_oStreamInfo.mStreamName;
   oStreamInfo.mChanelId = m_oStreamInfo.mChanelId;
   oStreamInfo.mRtmpUrlsStr = m_oStreamInfo.mRtmpUrlsStr;
   oStreamInfo.mCurRtmpUrl = m_oStreamInfo.mCurRtmpUrl;
   oStreamInfo.mCurRtmpUrlIndex = m_oStreamInfo.mCurRtmpUrlIndex;
   oStreamInfo.m_bHideLogo = m_oStreamInfo.m_bHideLogo;
   for (int i = 0; i < m_oStreamInfo.mPubLineInfo.size(); i++) {
      PubLineInfo pl;
      pl.mID = m_oStreamInfo.mPubLineInfo[i].mID;
      pl.mStrAlias = m_oStreamInfo.mPubLineInfo[i].mStrAlias;
      pl.mStrDomain = m_oStreamInfo.mPubLineInfo[i].mStrDomain;
      pl.mPingInfo = m_oStreamInfo.mPubLineInfo[i].mPingInfo;
      oStreamInfo.mPubLineInfo.insert(i, pl);
   }
   return TRUE;
}

void STDMETHODCALLTYPE CommonData::SetCurVersion(wstring wsCurVersion) {
    m_wzCurVersion = wsCurVersion;
}

void STDMETHODCALLTYPE CommonData::GetCurVersion(wstring& wsCurVersion) {
   wsCurVersion = m_wzCurVersion;
}

void STDMETHODCALLTYPE CommonData::GetOutputInfo(void *pOutputInfo) {
   *(OBSOutPutInfo *)pOutputInfo = m_outputInfo;
}

void STDMETHODCALLTYPE CommonData::SetOutputInfo(void *pOutputInfo) {
   m_outputInfo = *(OBSOutPutInfo *)pOutputInfo;
}

long STDMETHODCALLTYPE CommonData::GetReportID() {
   return ::InterlockedIncrement(&m_ai64ReportID);
}

void STDMETHODCALLTYPE CommonData::SetPublishState(bool bStart) {
   m_bStartPublish = bStart;
}

bool STDMETHODCALLTYPE CommonData::GetPublishState() {
    return m_bStartPublish;
}

void STDMETHODCALLTYPE CommonData::ReportEvent(string wsEvent) {
//    Json::StyledWriter writer;
//    Json::Value valueArray, value;
//    //value
//    value[wsEvent] = wsEvent;
//    valueArray.append(value);
//    valueArray.append(value);
//    valueArray.append(value);
// 
//    string s = valueArray.toStyledString();

   m_wzEvents = m_wzEvents + " " + wsEvent;
}

string STDMETHODCALLTYPE CommonData::GetEvents() {
    return m_wzEvents;
}

void CommonData::SetShowChatFilterBtn(bool bEnable) {
   mbIsShowChatFilterBtn = bEnable;
}

bool CommonData::IsShowChatFilterBtn() {
   return mbIsShowChatFilterBtn;
}

void CommonData::SetMembersManager(bool bEnable) {
   mbIsEnableMemberKickOut = bEnable;
}

bool CommonData::GetMembersManagerState() {
   return mbIsEnableMemberKickOut;
}

void CommonData::SetWebinarNotice(bool bEnable) {
   mbIsEnableWebniarNotice = bEnable;
}

bool CommonData::GetWebinarNoticeState() {
   return mbIsEnableWebniarNotice;
}

void CommonData::SetShowChatForbidBtn(bool bEnable) {
   mbIsShowChatForbidBtn = bEnable;
}

bool CommonData::IsShowChatForbidBtn() {
   return mbIsShowChatForbidBtn;
}

void STDMETHODCALLTYPE CommonData::SetStartMode(const int startMode)
{
	mIStartMode = startMode;
}

int STDMETHODCALLTYPE CommonData::GetStartMode()
{
	return mIStartMode;
}

void CommonData::SetLoginUserHostType(int type) {
   mHostType = type;
}

int CommonData::GetLoginUserHostType() {
   return mHostType;
}

void CommonData::SetLiveType(int type) {
   mLiveType = type;
}

int CommonData::GetLiveType() {
   return mLiveType;
}

void CommonData::SetUserID(const char *id) {
	mUserID = id;
}

char* CommonData::GetUserID() {
   return (char*)mUserID.c_str();
}

void CommonData::SetCutRecordState(int state) {
   QMutexLocker l(&mCutRecordMutex);
   mCutRecordState = state;
}

int CommonData::GetCurRecordState() {
   QMutexLocker l(&mCutRecordMutex);
   return mCutRecordState;
}

void CommonData::SetStartLiveUrl(const StartLiveUrl& liveUrl) {
    mStartLiveUrl = liveUrl;
}

void CommonData::SetStartLiveParam(const QString streamToken, const QString streamName, const QString msgToken, bool hidelogo) {
    mStartLiveUrl.msg_token = msgToken;
    mStartLiveUrl.stream_name = streamName;
    mStartLiveUrl.stream_token = streamToken;
    mStartLiveUrl.hide_logo = hidelogo;
}

void CommonData::GetStartLiveUrl(StartLiveUrl& liveUrl) {
    liveUrl = mStartLiveUrl;
}

void CommonData::ParamToGetVSSRoomBaseInfo(const QString& respMsg) {
    int nCode = 0;
    QJsonParseError json_error;
    QJsonDocument parse_doucment = QJsonDocument::fromJson(QByteArray(respMsg.toStdString().c_str()), &json_error);
    if (json_error.error == QJsonParseError::NoError) {
        if (parse_doucment.isObject()) {
            QJsonObject obj = parse_doucment.object();
            if (obj.contains("data") && obj["data"].isObject()) {
                QJsonObject data = obj["data"].toObject();
                if (data.contains("paas_access_token") && data["paas_access_token"].isString()) {
                    mGetRoomInfo.paas_access_token = data["paas_access_token"].toString();
                }
                if (data.contains("inav_id") && data["inav_id"].isString()) {
                    mGetRoomInfo.inav_id = data["inav_id"].toString();
                }
                if (data.contains("room_id") && data["room_id"].isString()) {
                    mGetRoomInfo.room_id = data["room_id"].toString();
                }
                if (data.contains("account_id")) {
                    if (data["account_id"].isString()) {
                        mGetRoomInfo.account_id = data["account_id"].toString();
                    }
                    else {
                        mGetRoomInfo.account_id = QString::number(data["account_id"].toInt());
                    }
                }

                if (data.contains("third_party_user_id")) {
                    if (data["third_party_user_id"].isString()) {
                        mGetRoomInfo.third_party_user_id = data["third_party_user_id"].toString();
                    }
                    else {
                        mGetRoomInfo.third_party_user_id = QString::number(data["third_party_user_id"].toInt());
                    }
                }

                if (data.contains("app_id") && data["app_id"].isString()) {
                    mGetRoomInfo.app_id = data["app_id"].toString();
                }
                if (data.contains("status") ) {
                    if (data["status"].isString()) {
                        mGetRoomInfo.status = data["status"].toString().toInt();
                    }
                    else {
                        mGetRoomInfo.status = data["status"].toInt();
                    }
                }

                std::string channel_id;
                if (data.contains("channel_id") && data["channel_id"].isString()) {
                    mGetRoomInfo.channel_id = data["channel_id"].toString();
                }
            }
        }
    }
}

void CommonData::ParamToVSSRoomAttributes(const QString& respMsg) {
    QByteArray dataByte(respMsg.toStdString().c_str(), respMsg.toStdString().length());
    QJsonParseError json_error;
    QJsonDocument doc = QJsonDocument::fromJson(dataByte,&json_error);
    QJsonObject obj = doc.object();
    QJsonObject data = obj["data"].toObject();

    if (data.contains("is_handsup")) {
        if (data["is_handsup"].isString()) {
            mVSSRoomAttributes.is_handsup = data["is_handsup"].toString().toInt();
        }
        else {
            mVSSRoomAttributes.is_handsup = data["is_handsup"].toInt();
        }
    }
    if (data.contains("main_screen")) {
        if (data["main_screen"].isString()) {
            mVSSRoomAttributes.main_screen = data["main_screen"].toString();
        }
        else {
            mVSSRoomAttributes.main_screen = QString::number(data["main_screen"].toInt());
        }
    }

    if (data.contains("speaker_list")) {
        if (data["speaker_list"].isArray()) {
            QJsonDocument speaker_doc;
            QJsonArray speaker_list = data["speaker_list"].toArray();
            speaker_doc.setArray(speaker_list);
            mVSSRoomAttributes.speaker_list = QString::fromUtf8(speaker_doc.toJson(QJsonDocument::Indented).data());
        }
    }

    if (data.contains("stream")) {
        if (data["stream"].isObject()) {
            QJsonObject streamObj = data["stream"].toObject();
            if (streamObj.contains("layout")) {
                if (streamObj["layout"].isString()) {
                    mVSSRoomAttributes.layout = streamObj["layout"].toString();
                }
            }
            if (streamObj.contains("definition")) {
                if (streamObj["definition"].isString()) {
                    mVSSRoomAttributes.definition = streamObj["definition"].toString();
                }
            }
        }
    }
}

void CommonData::SetJoinActiveFromLoginPage(bool isLoginPage) {
    mbJoinByLoginPage = isLoginPage;
}

bool CommonData::GetJoinActiveState() {
    return mbJoinByLoginPage;
}

void CommonData::GetVSSRoomAttributes(VSSRoomAttributes & attributes) {
    attributes = mVSSRoomAttributes;
}

void CommonData::GetVSSRoomBaseInfo(VSSGetRoomInfo & roomInfo) {
    roomInfo = mGetRoomInfo;
 }

//开始直播时，页面返回响应的信息
void CommonData::ParamToInitApiResp(const QString& initApiResp) {
    QByteArray dataByte(initApiResp.toStdString().c_str(), initApiResp.length());
    QJsonParseError json_error;
    QJsonDocument doc = QJsonDocument::fromJson(dataByte,&json_error);
    if (json_error.error != QJsonParseError::NoError) {
        return;
    }
    QJsonObject obj = doc.object();
    QJsonObject data = obj["data"].toObject();

    if (data.contains("accesstoken") && data["accesstoken"].isString()) {
        mInitApiResp.accesstoken = data["accesstoken"].toString();
    }
    if (data.contains("ismix")) {
        mInitApiResp.ismix = data["ismix"].toInt();
    }
    if (data.contains("switchDomain") && data["switchDomain"].isString()) {
        mInitApiResp.switchDomain = data["switchDomain"].toString();
    }
    if (data.contains("vod_create_url") && data["vod_create_url"].isString()) {
        mInitApiResp.vod_create_url = data["vod_create_url"].toString();
    }
    if (data.contains("forbidchat")) {
        mInitApiResp.forbidchat = data["forbidchat"].toInt();
    }
    if (data.contains("filterurl") && data["filterurl"].isString()) {
        mInitApiResp.filterurl = data["filterurl"].toString();
    }
    if (data.contains("msg_srv") && data["msg_srv"].isString()) {
        mInitApiResp.msg_srv = data["msg_srv"].toString();
    }
    if (data.contains("msg_port") && data["msg_port"].isString()) {
        mInitApiResp.msg_port = data["msg_port"].toString();
    }
    if (data.contains("chat_srv") && data["chat_srv"].isString()) {
        mInitApiResp.chat_srv = data["chat_srv"].toString();
    }
    if (data.contains("chat_port") && data["chat_port"].isString()) {
        mInitApiResp.chat_port = data["chat_port"].toString();
    }
    if (data.contains("chat_url") && data["chat_url"].isString()) {
        mInitApiResp.chat_url = data["chat_url"].toString();
    }
    if (data.contains("is_webinar_plug")) {
        mInitApiResp.is_webinar_plug = data["is_webinar_plug"].toInt();
    }
    if (data.contains("webinar_name") && data["webinar_name"].isString()) {
        mInitApiResp.webinar_name = data["webinar_name"].toString();
    }
    if (data.contains("3rdstream_url") && data["3rdstream_url"].isString()) {
        mInitApiResp.thirdstream_url = data["3rdstream_url"].toString();
    }
    if (data.contains("protocol_type")) {
        mInitApiResp.protocol_type = data["protocol_type"].toInt();
    }
    if (data.contains("isPrivilege")) {
        mInitApiResp.isPrivilege = data["isPrivilege"].toInt();
    }
    if (data.contains("isGuestChange")) {
        mInitApiResp.isGuestChange = data["isGuestChange"].toInt();
    }
    if (data.contains("live_time")) {
        mInitApiResp.live_time = data["live_time"].toInt();
    }
    if (data.contains("isVoice")) {
        mInitApiResp.isVoice = data["isVoice"].toInt();
    }
    if (data.contains("display_plugin")) {
        mInitApiResp.display_plugin = data["display_plugin"].toInt();
    }
    if (data.contains("display_cut_record")) {
        mInitApiResp.display_cut_record = data["display_cut_record"].toInt();
    }
    if (data.contains("host_type")) {
        mInitApiResp.host_type = data["host_type"].toInt();
    }
    if (data.contains("interact_plugins_url") && data["interact_plugins_url"].isString()) {
        mInitApiResp.interact_plugins_url = data["interact_plugins_url"].toString();
    }
    if (data.contains("assistant_token") && data["assistant_token"].isString()) {
        mInitApiResp.assistant_token = data["assistant_token"].toString();
    }
    if (data.contains("plugins_url") && data["plugins_url"].isString()) {
        mInitApiResp.plugins_url = data["plugins_url"].toString();
    }
    if (data.contains("interact_type")) {
        mInitApiResp.interact_type = data["interact_type"].toInt();
    }
    if (data.contains("show_checkbox")) {
        mInitApiResp.show_checkbox = data["show_checkbox"].toInt();
    }
    if (data.contains("join_uid") && data["join_uid"].isString()) {
        mInitApiResp.join_uid = data["join_uid"].toString();
    }
    if (data.contains("nick_name") && data["nick_name"].isString()) {
        mInitApiResp.nick_name = data["nick_name"].toString();
    }
    if (data.contains("role_name") && data["role_name"].isString()) {
        mInitApiResp.role_name = data["role_name"].toString();
    }
    if (data.contains("curr_presenter") && data["curr_presenter"].isString()) {
        mInitApiResp.curr_presenter = data["curr_presenter"].toString();
    }
    if (data.contains("handsUp")) {
        if (data["handsUp"].isString()) {
            mInitApiResp.handsUp = data["handsUp"].toString().toInt();
        }
        else {
            mInitApiResp.handsUp = data["handsUp"].toInt();
        }
    }
    if (data.contains("bizRole")) {
        mInitApiResp.bizRole = data["bizRole"].toInt();
    }
    if (data.contains("reportUrl") && data["reportUrl"].isString()) {
        mInitApiResp.reportUrl = data["reportUrl"].toString();
    }
    if (data.contains("mainShow") && data["mainShow"].isString()) {
        mInitApiResp.mainShow = data["mainShow"].toString();
    }

    if (data.contains("device_audio")) {
        mInitApiResp.device_audio = data["device_audio"].toInt();
    }
    if (data.contains("device_video")) {
        mInitApiResp.device_video = data["device_video"].toInt();
    }

    if (data.contains("against_url") && data["against_url"].isString()) {
        mInitApiResp.against_url = data["against_url"].toString();
    }
    if (data.contains("thirdStreamToken") && data["thirdStreamToken"].isString()) {
        mInitApiResp.thirdStreamToken = data["thirdStreamToken"].toString();
    }
    if (data.contains("webinar_type")) {
        mInitApiResp.webinar_type = data["webinar_type"].toInt();
    }
    if (data.contains("room_token") && data["room_token"].isString()) {
        mInitApiResp.room_token = data["room_token"].toString();
    }
    if (data.contains("watchLayout") && data["watchLayout"].isString()) {
        mInitApiResp.watchLayout = data["watchLayout"].toString();
    }
    if (data.contains("definition") && data["definition"].isString()) {
        mInitApiResp.definition = data["definition"].toString();
    }
    if (data.contains("player")) {
        mInitApiResp.player = data["player"].toInt();
    }
    else {
        mInitApiResp.player = FLASH_LIVE_CREATE_TYPE;
    }
    if (data.contains("vss_token") && data["vss_token"].isString()) {
        mInitApiResp.vss_token = data["vss_token"].toString();
    }
    if (data.contains("vss_room_id") && data["vss_room_id"].isString()) {
        mInitApiResp.vss_room_id = data["vss_room_id"].toString();
    }
    if (data.contains("record_tip")) {
       mInitApiResp.defualt_record = data["record_tip"].toInt();
    }
    if (data.contains("params_verify_token") && data["params_verify_token"].isString()) {
        mInitApiResp.params_verify_token = data["params_verify_token"].toString();
    }

    if (data.contains("web_chat_url") && data["web_chat_url"].isString()) {
        mInitApiResp.web_chat_url = data["web_chat_url"].toString();
    }
    if (data.contains("web_doc_url") && data["web_doc_url"].isString()) {
        mInitApiResp.web_doc_url = data["web_doc_url"].toString();
    }
    if (data.contains("web_question_url") && data["web_question_url"].isString()) {
        mInitApiResp.web_question_url = data["web_question_url"].toString();
    }

    if (data.contains("user") && data["user"].isObject()) {
        QJsonObject userObj = data["user"].toObject();
        if (userObj.contains("user_id") && userObj["user_id"].isString()) {
            if (userObj["user_id"].isString()) {
                mInitApiResp.user_id = userObj["user_id"].toString();
            }
            else {
                mInitApiResp.user_id = QString::number(userObj["user_id"].toInt());
            }
        }
        if (userObj.contains("join_uid")) {
            if (userObj["join_uid"].isString()) {
                mInitApiResp.join_uid = userObj["join_uid"].toString();
            }
            else {
                mInitApiResp.join_uid = QString::number(userObj["join_uid"].toInt());
            }
        }
        if (userObj.contains("nick_name") && userObj["nick_name"].isString()) {
            mInitApiResp.nick_name = userObj["nick_name"].toString();
        }

        if (userObj.contains("saas_join_uid") && userObj["saas_join_uid"].isString()) {
            mInitApiResp.saas_join_uid = userObj["saas_join_uid"].toString();
        }

        if (userObj.contains("avatar") && userObj["avatar"].isString()) {
            mInitApiResp.avatar = userObj["avatar"].toString();
        }
        if (userObj.contains("role") && userObj["role"].isString()) {
            mInitApiResp.role_name = userObj["role"].toString();
        }
    }
    mInitApiResp.pushStreamList.clear();
    if (data.contains("address") && data["address"].isArray()) {
        QJsonArray addrArray = data["address"].toArray();
        for (int i = 0; i < addrArray.count(); i++) {
            PushStreamAddress pushAddr;
            QJsonObject addrObj = addrArray.at(i).toObject();
            if (addrObj.contains("name")) {
                pushAddr.name = addrObj["name"].toString();
            }
            if (addrObj.contains("srv")) {
                pushAddr.srv = addrObj["srv"].toString();
            }
            mInitApiResp.pushStreamList.push_back(pushAddr);
        }
    }
    mInitApiResp.permission_data.clear();
    if (data.contains("permission_data") && data["permission_data"].isArray()) {
        QJsonArray permission_data = data["permission_data"].toArray();
        for (int i = 0; i < permission_data.count(); i++) {
            if (permission_data.at(i).isString()) {
                QString per = permission_data.at(i).toString();
                //if (per != "100018") {    //不处理红包
                //    mInitApiResp.permission_data.push_back(per);
                //}
                mInitApiResp.permission_data.push_back(per);
            }
            else {
                QString per = QString::number(permission_data.at(i).toInt());
                mInitApiResp.permission_data.push_back(per);
                //if (per != "100018") {
                //    mInitApiResp.permission_data.push_back(per);
                //}
            }
        }
    }

    QJsonDocument speaker_doc;
    QJsonArray speaker_list = data["speaker_list"].toArray();
    speaker_doc.setArray(speaker_list);
    //如果是vhall互动，主持人登录会获取正在邀请的用户列表。
    QJsonDocument invite_doc;
    QJsonArray invite_list = data["invite_list"].toArray();
    invite_doc.setArray(invite_list);

    mInitApiResp.speaker_list = QString::fromUtf8(speaker_doc.toJson(QJsonDocument::Indented).data());
    mInitApiResp.invite_list = QString::fromUtf8(invite_doc.toJson(QJsonDocument::Indented).data());

}
void CommonData::GetInitApiResp(ClientApiInitResp& initApiResp) {
    initApiResp = mInitApiResp;
}

void CommonData::SetSelectLiveType(int liveType) {
    mInitApiResp.selectLiveType = liveType;
}

void CommonData::ParamToVSSRoomPushUrl(const QString& respMsg) {
    QByteArray dataByte(respMsg.toStdString().c_str(), respMsg.length());
    QJsonDocument doc = QJsonDocument::fromJson(dataByte);
    QJsonObject obj = doc.object();
    QJsonObject data = obj["data"].toObject();

    if (data.contains("stream_number")) {
        mVSSRoomPushUrl.stream_number = data["stream_number"].toString();
    }
    if (data.contains("push_address")) {
        mVSSRoomPushUrl.push_address = data["push_address"].toString();
    }
}

void CommonData::GetVSSRoomPushUrl(VSSRoomPushUrl & pushinfo) {
    pushinfo = mVSSRoomPushUrl;
}

void CommonData::SetPrivateStartParam(const QString& privateStartInfo) {
    mPrivateStartInfo = privateStartInfo;
}

QString CommonData::GetPrivateStartParam() {
    return mPrivateStartInfo;
}

void CommonData::SetMainUiLoginObj(QObject* obj) {
    mainUiLoginObj = obj;
}

QObject* CommonData::MainUiLoginObj() {
    return mainUiLoginObj;
}

void CommonData::ParamToLineParam(const QString& param, UserOnLineParam& lineParam) {
    QJsonParseError json_error;
    QString paramMsg = param;
    QJsonDocument parse_doucment = QJsonDocument::fromJson(QByteArray(paramMsg.toStdString().c_str()), &json_error);
    if (json_error.error == QJsonParseError::NoError) {
        if (parse_doucment.isObject()) {
            QJsonObject obj = parse_doucment.object();
            if (obj.contains("type") && obj["type"].isString()) {
                QString type = obj["type"].toString();
                lineParam.type = type;
                // 聊天上报，只处理上线下、禁言、全员禁言，取消禁言，取消全员禁言
                if (type == "Leave" || type == "Join") {
                    if (obj.contains("msg") && obj["msg"].isObject()) {
                        QJsonObject msg = obj["msg"].toObject();
                        if (msg.contains("sender_id") && msg["sender_id"].isString()) {
                            lineParam.userId = msg["sender_id"].toString();
                        }
                        else if (msg.contains("sender_id")) {
                            lineParam.userId = QString::number(msg["sender_id"].toInt());
                        }

                        if (msg.contains("target_id") && msg["target_id"].isString()) {
                           lineParam.userId = msg["target_id"].toString();
                        }
                        else if (msg.contains("target_id")) {
                           lineParam.userId = QString::number(msg["target_id"].toInt());
                        }

                        if (msg.contains("uv")) {
                           if (msg["uv"].isString()) {
                              lineParam.mUV = msg["uv"].toString().toInt();
                           }
                           else {
                              lineParam.mUV = msg["uv"].toInt();
                           }
                        }


                        if (msg.contains("context") && msg["context"].isObject()) {
                            QJsonObject context = msg["context"].toObject();
                            if (context.contains("role_name")) {
                               if (context["role_name"].isString()) {
                                  lineParam.userRole = context["role_name"].toString();
                               }
                               else {
                                  lineParam.userRole = QString::number(context["role_name"].toInt());
                               }
                                if (lineParam.userRole == "1") {
                                    lineParam.userRole = USER_HOST;
                                }
                                else if (lineParam.userRole == "2") {
                                    lineParam.userRole = USER_USER;
                                }
                                else if (lineParam.userRole == "3") {
                                    lineParam.userRole = USER_ASSISTANT;
                                }
                                else if (lineParam.userRole == "4") {
                                    lineParam.userRole = USER_GUEST;
                                }
                            }
                            if (context.contains("nickname")) {
                                lineParam.userName = context["nickname"].toString();
                            }
                            if (context.contains("is_banned")) {
                                lineParam.is_banned = context["is_banned"].toBool();
                            }
                            if (context.contains("avatar")) {
                               lineParam.headImage = context["avatar"].toString();
                            }
                            if (context.contains("audience")) {
                                 //包含此字段，表示是关闭了聊天过滤或者关闭了观看页面。所以不下麦
                               lineParam.audience = true;
                            }
                            if (context.contains("device_type")) {
                               if (context["device_type"].isString()) {
                                  lineParam.mDevType = context["device_type"].toString().toInt();
                               }
                               else {
                                  lineParam.mDevType = context["device_type"].toInt();
                               }
                            }
                            
                        }
                    }
                }
                else if (type == "disable" || type == "permit") {
                    if (obj.contains("msg") && obj["msg"].isObject()) {
                        QJsonObject msg = obj["msg"].toObject();
                        if (msg.contains("data") && msg["data"].isObject()) {
                            QJsonObject dataObj = msg["data"].toObject();
                            if (dataObj.contains("target_id")) {
                                lineParam.userId = dataObj["target_id"].toString();
                            }
                            if (dataObj.contains("nick_name")) {
                                lineParam.userName = dataObj["nick_name"].toString();
                            }
                            if (dataObj.contains("avatar")) {
                               lineParam.headImage = dataObj["avatar"].toString();
                            }
                            if (dataObj.contains("role_name")) {
                               lineParam.userRole = dataObj["role_name"].toString();
                               if (lineParam.userRole == "1") {
                                  lineParam.userRole = USER_HOST;
                               }
                               else if (lineParam.userRole == "2") {
                                  lineParam.userRole = USER_USER;
                               }
                               else if (lineParam.userRole == "3") {
                                  lineParam.userRole = USER_ASSISTANT;
                               }
                               else if (lineParam.userRole == "4") {
                                  lineParam.userRole = USER_GUEST;
                               }
                            }
                        }
                    }
                }
                else if (type == "docFullscreen" || type == "docmouseenter") {
                   if (obj.contains("msg") && obj["msg"].isBool()) {
                      bool isfull = obj["msg"].toBool();
                      lineParam.is_full = isfull;
                   }
                }
                else if (type == "page") {
                   if (obj.contains("msg")) {
                      int pageIndex = obj["msg"].toInt();
                      lineParam.pageIndex = pageIndex;
                   }
                }
            }
        }
    }
}