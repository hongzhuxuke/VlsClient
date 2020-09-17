#include "VhallConfigBroadCast.h"
#include "ICommonData.h"
#include "Msg_VhallRightExtraWidget.h"
#include "CRPluginDef.h"
#include "MainUIIns.h"
#include "Msg_MainUI.h"
#include "VH_ConstDeff.h"
#include "VhallUI_define.h"
#include "DebugTrace.h"



VhallConfigBroadCast::VhallConfigBroadCast(QObject *parent)
    : QObject(parent)
{
   connect(&mConfigBroadCastTimer, SIGNAL(timeout()), this, SLOT(slot_OnConfigBoradCastTimer()));
}

VhallConfigBroadCast::~VhallConfigBroadCast()
{
   mConfigBroadCastTimer.stop();
}

void VhallConfigBroadCast::InitBroadCast(LayoutMode layout, BroadCastVideoProfileIndex profile, QObject *listener) {
   //初始化设置用户信息。
   //如果调用了，就不再处理开启旁路。
   mbIsNeedConfig = false;
   mMainObject = listener;
   VH::CComPtr<ICommonData> pCommonData;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
   ClientApiInitResp respInitData;
   pCommonData->GetInitApiResp(respInitData);
   if (respInitData.player == FLASH_LIVE_CREATE_TYPE) {
      //flash 活动 进下开启旁路配置
      int nRet = vlive::GetWebRtcSDKInstance()->StartConfigBroadCast(layout, profile, true, std::string("0x666666"), std::string("0x000000"), [&, listener](const std::string& result, const std::string& msg, int errorCode)->void {
         TRACE6("%s StartConfigBroadCast result:%s\n", __FUNCTION__, result.c_str());
         QString errMsg = QString::fromStdString(result);
         if (listener) {
            QCoreApplication::postEvent(listener, new CustomRecvMsgEvent(CustomEvent_ConfigBoradCast, errMsg));
         }
      });
   }
   else {
      VH::CComPtr<ICommonData> pCommonData;
      DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
      VSSGetRoomInfo roomInfo;
      pCommonData->GetVSSRoomBaseInfo(roomInfo);
      vlive::GetPaasSDKInstance()->StartPublishInavAnother(roomInfo.room_id.toStdString(), layout, profile);
      TRACE6("%s StartPublishInavAnother room_id:%s paasLayout:%d", __FUNCTION__, roomInfo.room_id.toStdString().c_str(), layout);
   }
}

void VhallConfigBroadCast::HandleBroadCast(LayoutMode layout, BroadCastVideoProfileIndex profile, bool success) {
   mLayout = layout;
   mProfile = profile;
 if (!success) {
      mConfigBroadCastTimer.start(2000);
      mConfigBroadCastTimer.setSingleShot(true);
   }
}

void VhallConfigBroadCast::slot_OnConfigBoradCastTimer() {
   InitBroadCast(mLayout, mProfile, mMainObject);
   mConfigBroadCastTimer.stop();
}

void VhallConfigBroadCast::SetNeedConfigBroadCast(bool need) {
   mbIsNeedConfig = need;
}

bool VhallConfigBroadCast::IsNeedConfigBroadCast() {
   return mbIsNeedConfig;
}