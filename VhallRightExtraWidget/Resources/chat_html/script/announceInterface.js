/**
 * Created by shen on 2016/10/24.
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

    $.extend(vhChat.interfaces || {}, {
        /**
         * 公告页面 初始化函数
         * 没有什么要初始化的 就空着~~
         */
        announceInit: function() {
            // todo
        },

        /**
         * 发送公告
         * @param msg  {string}  公告内容
         */
        sendAnnouncement: function (msg) {
            var msg_ = {
                type: "*announcement",
                content: msg
            };
            var msg_json = JSON.stringify(msg_);
            MainWindow.sendNotice(msg_json);
            // todo
        }
    });

} (window, jQuery));
