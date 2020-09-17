#pragma once

#include <QObject>
#include "WebRtcSDKInterface.h"
#include "VHPaasSDKCallback.h"
#include <QTimer>

using namespace vlive;
class VhallConfigBroadCast : public QObject
{
   Q_OBJECT

public:
    VhallConfigBroadCast(QObject *parent);
    ~VhallConfigBroadCast();

   void InitBroadCast(LayoutMode layout, BroadCastVideoProfileIndex profile, QObject *listener);
   void HandleBroadCast(LayoutMode layout, BroadCastVideoProfileIndex profile, bool success);
   
   //
   void SetNeedConfigBroadCast(bool need);
   bool IsNeedConfigBroadCast();
private slots:
   void slot_OnConfigBoradCastTimer();
private:
   QObject* mMainObject = nullptr;

   LayoutMode mLayout;
   BroadCastVideoProfileIndex mProfile;
   QTimer mConfigBroadCastTimer;
   bool mbIsNeedConfig = false;
};
