#pragma once

#include <set>

#include "IPluginManager.h"

#include "IUIThreadManager.h"

//#include "CRMessageWnd.h"

#include "Singleton.h"


#define DEF_WM_CHECK_BEGIN_TASK					(WM_USER + 0xFF01)
#define DEF_WM_CHECK_END_TASK					(WM_USER + 0xFF02)
#define DEF_WM_REG_MSG_EVENT						(WM_USER + 0xFF03)
#define DEF_WM_UNREG_MSG_EVENT					(WM_USER + 0xFF04)


typedef std::set<IUIThreadObject*>			PluginUIThreadDict;
typedef std::set<IUIThreadMessageEvent*>		PluginUIThreadMsgEventDict;
typedef std::set<IUIPreMessageEvent*>		PluginUIPreMsgEventDict;

class IQueryInterface;

class CRuntimeUIThreadManager : public IUIThreadManager
{
public:
	CRuntimeUIThreadManager(void);
	~CRuntimeUIThreadManager(void);

public:
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void ** ppvObject);

	virtual ULONG STDMETHODCALLTYPE AddRef(void);

	virtual ULONG STDMETHODCALLTYPE Release(void);

public:
	void Init(DWORD adwUIThreadID);

	void UnInit();

public:
	// 请求指定的UI线程启动工作，返回目标线程ID,0表示失败
	virtual DWORD STDMETHODCALLTYPE Begin(DWORD dwThreadIndex, IUIThreadObject* apMUIThreadObject);

	// 请求指定的UI线程结束工作
	virtual HRESULT STDMETHODCALLTYPE End(DWORD dwThreadIndex, IUIThreadObject* apMUIThreadObject);

	// 向指定的UI线程注册UI消息回
	virtual HRESULT STDMETHODCALLTYPE RegisterUIMessageEvent(DWORD dwThreadIndex, IUIThreadMessageEvent * apEvent);

	// 向指定的UI线程取消注册UI消息回调
	virtual HRESULT STDMETHODCALLTYPE UnRegisterUIMessageEvent(DWORD dwThreadIndex, IUIThreadMessageEvent * pEvent);

	// 向指定的UI线程注册UI预处理消息回调
	virtual HRESULT STDMETHODCALLTYPE RegisterUIPreMessageEvent(IUIPreMessageEvent * apEvent);

	// 向指定的UI线程取消UI预处理消息回调
	virtual HRESULT STDMETHODCALLTYPE UnRegisterUIPreMessageEvent(IUIPreMessageEvent * apEvent);

public:
	// 处理线程消息
	BOOL OnPreTranslateMessage(MSG* apMsg);

	// 时钟回调广播处理消息
	BOOL OnTimerBroadcastDealMessage();

	// 检查开始任务
	BOOL OnMessageCheckBeginTask(WPARAM wparam, LPARAM lparam);

	// 检查结束任务
	BOOL OnMessageCheckEndTask(WPARAM wparam, LPARAM lparam);

	// 注册消息事件
	BOOL OnMessageRegisterMsgEvent(WPARAM wparam, LPARAM lparam);

	// 移除消息事件
	BOOL OnMessageUnRegisterMsgEvent(WPARAM wparam, LPARAM lparam);

	// 注册UI预处理对象
	BOOL OnMessageRegisterMsgObject(WPARAM wparam, LPARAM lparam);

	// 移除UI预处理对象
	BOOL OnMessageUnRegisterMsgObject(WPARAM wparam, LPARAM lparam);

public:
	// 获取某个插件的UI线程对象接口
	BOOL GetPluginUIThread(IPluginObject* apPluginObject, IUIThreadObject** appPluginUIThread);

public:
	//----------------------------------------------------------------------------------------------------------------
	// IMPluginManagerEvent接口
	BEGIN_INTERFACE_PART(PluginManagerEvent, IPluginManagerEvent);
		INIT_INTERFACE_PART(CRuntimeUIThreadManager, PluginManagerEvent);

		// 通知某一个插件被加载到管理器中
		virtual HRESULT STDMETHODCALLTYPE OnPluginLoaded(IPluginObject* apPluginObject);

		// 通知某一个插件从管理器卸载之前
		virtual HRESULT STDMETHODCALLTYPE OnPluginUnLoadBefore(IPluginObject* apPluginObject);

		// 通知某一个插件已从管理器卸载
		virtual HRESULT STDMETHODCALLTYPE OnPluginUnLoaded(REFGUID aoCRPGuid);

		// 是否可以加载某一个插件
		virtual HRESULT STDMETHODCALLTYPE IsCanLoadPlugin(REFGUID aoCRPGuid);

		// 是否可以卸载
		virtual HRESULT STDMETHODCALLTYPE IsCanUnLoadPlugin(IPluginObject* apPluginObject);

	END_INTERFACE_PART(PluginManagerEvent);
	//----------------------------------------------------------------------------------------------------------------

private:
    //CCRMessageWnd* m_pMessageWnd;

	long m_lRefCount;

	DWORD m_dwUIThreadID;

	PluginUIThreadDict m_oPluginUIThreadDict;
	PluginUIThreadMsgEventDict m_oPluginUIThreadMsgEventDict;
	PluginUIPreMsgEventDict m_oPluginUIPreMsgEventDict;
};

typedef CSingleton<CRuntimeUIThreadManager> CSingletonMainUIThreadMgr;
