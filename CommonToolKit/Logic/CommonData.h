#ifndef __COMMON_DATA__H_INCLUDE__
#define __COMMON_DATA__H_INCLUDE__

#pragma once
#include "ICommonData.h"

class CommonData :public ICommonData
{
public:
   CommonData();
   ~CommonData();

public:
   /////////////////////////////IUnknown接口/////////////////////////////////////////
   virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void ** appvObject);
   virtual ULONG STDMETHODCALLTYPE   AddRef(void);
   virtual ULONG STDMETHODCALLTYPE   Release(void);

   virtual void STDMETHODCALLTYPE RegisterRuntimeInstance(QObject* obj);
   virtual QObject* STDMETHODCALLTYPE GetRuntimeInstanceObj();

   virtual void STDMETHODCALLTYPE SetLoginRespInfo(const LoginRespInfo& loginRespInfo);
   virtual LoginRespInfo STDMETHODCALLTYPE GetLoginRespInfo();

   virtual BOOL STDMETHODCALLTYPE SetStreamInfo(PublishInfo* apStreamInfo);
   virtual BOOL STDMETHODCALLTYPE GetStreamInfo(PublishInfo& oStreamInfo);

   virtual void STDMETHODCALLTYPE SetCurVersion(wstring wsCurVersion);
   virtual void STDMETHODCALLTYPE GetCurVersion(wstring& wsCurVersion);

   virtual void STDMETHODCALLTYPE SetPublishState(bool bStart);
   virtual bool STDMETHODCALLTYPE GetPublishState();

   virtual void STDMETHODCALLTYPE GetOutputInfo(void *);
   virtual void STDMETHODCALLTYPE SetOutputInfo(void *);

   virtual long STDMETHODCALLTYPE GetReportID();

   virtual void STDMETHODCALLTYPE ReportEvent(string wsEvent);

   virtual string STDMETHODCALLTYPE GetEvents();

   virtual void STDMETHODCALLTYPE SetStartMode(const int startMode);
   virtual int STDMETHODCALLTYPE GetStartMode();

   virtual void SetShowChatFilterBtn(bool bEnable);
   virtual bool IsShowChatFilterBtn();
   virtual void SetMembersManager(bool bEnable);
   virtual bool GetMembersManagerState();
   virtual void SetWebinarNotice(bool bEnable);
   virtual bool GetWebinarNoticeState();
   virtual void SetShowChatForbidBtn(bool bEnable);
   virtual bool IsShowChatForbidBtn();
   virtual void SetLoginUserHostType(int type);
   virtual int GetLoginUserHostType();

   virtual void SetLiveType(int type);
   virtual int GetLiveType();
   virtual void SetUserID(const char *id) ;
   virtual char* GetUserID() ;

   virtual void SetCutRecordState(int state);
   virtual int GetCurRecordState();

   //开始直播时，页面返回的信息
   virtual void SetStartLiveUrl(const StartLiveUrl& liveUrl);
   virtual void SetStartLiveParam(const QString streamToken, const QString streamName, const QString msgToken,bool hidelogo);
   virtual void GetStartLiveUrl(StartLiveUrl& liveUrl);

   //开始直播时，页面返回响应的信息
   virtual void ParamToInitApiResp(const QString& respMsg);
   virtual void GetInitApiResp(ClientApiInitResp& initApiResp);
   virtual void SetSelectLiveType(int liveType);
   
   virtual void ParamToGetVSSRoomBaseInfo(const QString& respMsg);
   virtual void GetVSSRoomBaseInfo(VSSGetRoomInfo & roomInfo);
   
   virtual void ParamToVSSRoomAttributes(const QString& respMsg);
   virtual void GetVSSRoomAttributes(VSSRoomAttributes & Attributes);

   virtual void ParamToVSSRoomPushUrl(const QString& respMsg);
   virtual void GetVSSRoomPushUrl(VSSRoomPushUrl & pushinfo);

   virtual void SetPrivateStartParam(const QString& privateStartInfo);
   virtual QString GetPrivateStartParam();

   virtual void SetJoinActiveFromLoginPage(bool isLoginPage);
   virtual bool GetJoinActiveState();

   virtual void SetMainUiLoginObj(QObject* obj);
   virtual QObject* MainUiLoginObj();

   virtual void ParamToLineParam(const QString& param, UserOnLineParam& lineParam);

private:
   long  m_lRefCount;
   LoginRespInfo mLoginRespInfo;//用户登录时响应信息
   PublishInfo m_oStreamInfo; //流信息
   long m_ai64ReportID;
   wstring m_wzCurVersion;
   bool m_bStartPublish;
   string m_wzEvents;
   OBSOutPutInfo m_outputInfo; //码率分辨率等信息
   bool mbIsShowChatFilterBtn = true;
   bool mbIsEnableMemberKickOut = true;
   bool mbIsEnableWebniarNotice = true;
   bool mbIsShowChatForbidBtn = true;
   int mIStartMode;
   int mHostType = 0;
   int mLiveType = 0;
   string mUserID;
   QMutex mCutRecordMutex;
   int mCutRecordState;
   QObject* mRuntimeInstanceObj = nullptr;
   QObject* mainUiLoginObj = nullptr;

   QString mPrivateStartInfo; //从网页启动的私有协议
   StartLiveUrl mStartLiveUrl; //点击开播返回的信息
   ClientApiInitResp mInitApiResp;
   VSSGetRoomInfo mGetRoomInfo;
   VSSRoomAttributes mVSSRoomAttributes;
   VSSRoomPushUrl mVSSRoomPushUrl;
   std::atomic_bool mbJoinByLoginPage;
};


#endif //__COMMON_DATA__H_INCLUDE__
