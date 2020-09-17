/**
 * Created by shen on 2016/10/21.
 */

;(function(window, $, undefined) {
    var vhChat = window.vhChat;

    /**
     * 以下是客户端的js函数，供调用:
     *
     * 函数: vhChat.fun.appendMessage(json)
     * 说明: 把聊天消息加入列表
     * 参数: 传过来的消息数据json格式
     *
     *
     * 函数: vhChat.fun.appendRewardMessage(json)
     * 说明: 把打赏消息加入列表
     * 参数: 传过来的消息数据json格式
     */

    /////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////

    //------------------------ 以下是 你需要完成的 ---------------------------//

    // face qq 图片地址
    vhChat.url.VH_CHAT_HOST = "//cnstatic01.e.vhall.com/static/";

    // 聊天列表显示名字时 多于下面值时 用省略号代替
    vhChat.constant.max_name_length = 20;

    vhChat.interfaces = {
        /**
         * chat页面 初始化函数
         * 没有什么要初始化的 就空着~~  
         */
        chatInit: function(sysinfo) {
            // todo
            var pageinfo_str = MainWindow.get_init();
            if (pageinfo_str != "") {
                getInitPage = 1;
                pageinfo = JSON.parse(pageinfo_str);
            }
        },

        // 获取是否全体禁言的状态
        chatInitForbidStatus: function (sysinfo) {
            // todo
            var pageinfo_str = MainWindow.get_initForbStatus();
            if (pageinfo_str == 1) {
                getInitForbid = 1;
                doSetForbidStatus();
            }
        },

        // @某人
        atSomeone: function (uname) {
            // todo
            var uname_str = "@" + uname + ":";
            MainWindow.insertAtSomeone(uname_str);
        },

        /**
         * 发送聊天消息
         * @param msg  {string}  聊天内容
         * @param to   {string}  发送给谁 值为uid, 如果没有特定目标,值为undefined
         */
        sendOutMsg: function (msg) {
            var msg_ = {
                text: msg
            };
            var msg_json = JSON.stringify(msg_);
            MainWindow.sendChatMsg(msg_json);
        },

        /**
         * 全体禁言 或 取消
         * @param aim  {boolean}  true - 要禁言   false - 要取消禁言
         */
        forbid_all: function (aim) {
            MainWindow.post_forbidAll(aim);
            // todo
        },

        /**
         * 聊天过滤
         * @param aim  {boolean}  true - 要过滤   false - 要取消过滤
         */
        chatFilter: function(aim) {
            MainWindow.open_filter_browser();
            // todo
        },
        
        /**
         * 禁言某人
         * @param aim  {boolean}  true - 要过滤   false - 要取消过滤
         */
        forbid_someone: function(user_id, aim) {
            MainWindow.forbid_someone(user_id, aim);
            // todo
        },
        
        /**
         * 提出某人
         * @param aim  {boolean}  true - 要过滤   false - 要取消过滤
         */
        kickout_someone: function(user_id, aim) {
            MainWindow.kickout_someone(user_id, aim);
            // todo
        }
        
    }

} (window, jQuery));
