#ifndef H_PRICONST_H
#define H_PRICONST_H

enum eHandsUpState
{
    eHandsUpState_No = 0,
    eHandsUpState_existence
};

typedef enum VhallInteractionRenderType_e {
	RenderType_Video = 0,//默认 视频，包括摄像头和桌面共享
	RenderType_Wait,//等待
	RenderType_Finished,//结束
	RenderType_MasterExit,//主持人退出
	RenderType_Screen,//桌面共享
	RenderType_Photo,//照片
}VhallInteractionRenderType;

//用户列表tab页
enum eTabMember
{
	eTabMember_onLine = 0,   // 在线
	eTabMember_raiseHands,      // 举手
	eTabMember_limit,      //受限
   eTabMember_SearchPage, //搜索
};

enum eWebOpType
{
	eWebOpType_questionnaire = 100012,   //  问卷     
	eWebOpType_MemberManager = 100013,   //  成员模块
	eWebOpType_Luck = 100014,   //  抽奖    
	eWebOpType_Questions = 100016,   //  问答      
	eWebOpType_envelopes = 100018,   //  红包      
	eWebOpType_PublicNotice = 100019,   //  公告模块
	eWebOpType_Signin = 100022,   //  签到      

  /*  100001 = > "logo下显示登录注册",
	100002 = > "直播助手",
	100003 = > "浏览器顶部微吼图标",
	100004 = > "只发给主持人勾选框（主持人聊天设置）", (线上互动直播已废弃)
	100005 = > "分享",
	100008 = > "观众列表",
	100009 = > "在线用户人数",
	100010 = > "举手开关",
	100011 = > "文档模块",
	100012 = > "问卷模块",
	100013 = > "成员模块",
	100014 = > "抽奖模块",
	100015 = > "聊天过滤模块",
	100016 = > "问答模块",
	100017 = > "全体禁言模块",
	100018 = > "红包模块",
	100019 = > "公告模块",
	100020 = > "录制",
	100021 = > "结束直播",
	100022 = > "签到模块",
	100023 = > "第三方发起",
	100024 = > "举手列表",
	100025 = > "分屏",
	100026 = > "受限列表",
	100027 = > "转播模块"*/
};


//const int PageSpanNum = 5;
#endif//H_PRICONST_H