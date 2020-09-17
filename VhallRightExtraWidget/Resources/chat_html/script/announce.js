/**
 * Created by shen on 2016/10/24.
 */

;(function (window, $, undefined) {

    var pageinfo = {
                'domain':'//e.vhall.com',
                'static_url':'//cnstatic01.e.vhall.com/static',
                'upload_url':'//cnstatic01.e.vhall.com/upload',
                'is_host':'1',
                'webinar_id':'106350882',
                'webinar_type':'1',
                'host_uid':'46616',
                'join_uid':'18137392',
                'join_uname':'郝付壮',
                'curr_presenter':'18137392',
                'curr_step':'',
                'curr_page':'',
                'curr_file':'0',
                'totalPage':'',
                'msg_srv':'msg02.e.vhall.com:80',
                'msg_token':'eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzI1NiJ9.eyJpZCI6IjE4MTM3MzkyIiwibmFtZSI6Ilx1OTBkZFx1NGVkOFx1NThlZSIsInJvb20iOiIxMDYzNTA4ODIiLCJyb2xlIjoiaG9zdCIsInVpZCI6IjQ2NjE2IiwidHJhY2tJZCI6MzQ4NjMyMDd9.T_uVu-Rwn0pqfiAtdxZEwFmKQ18e1ilBbI7MSbDJuNU',
                'chat_srv':'chat02.e.vhall.com',
                'chat_port':'80',
                'chat_url':'//api-msg.e.vhall.com:80/pub/index',
                'log_service':'http://log.vhall.com:9101/gelf',
                'is_gag':'0',
                'hide_user_list':'0',
                'hide_live_end':'0',
                'doc_default':'',
                'doc_default_page':'0',
                'matchTheLink':'0',
                'role':'host',
                'kickOut':'0',
                'apiuser':'0',
                'appName':'',
                'user_avatar':'//cnstatic01.e.vhall.com/upload/user_avatar/1d/6e/1d6edbf3d3895794bab2b53fb84dc3b9',
        'webinar_layout': 1 ,
        'isVip' :  1 ,
        'is_gaglist' : [],
        'liveTime' : 0,
        'webinar_type' : '1',
        'FlashShowType' :  0 , //当前文档区域显示的是什么 ppt=0,白板=1，没有=-1 
		'is_question' :  0     
    }
    
    function checkTime(i)
    {
        if (i<10) {
            i="0" + i;
        }
        return i;
    }


    var vhChat = window.vhChat || {};
    vhChat.announce = {
        appendAnnouncement: function(json) {
            var obj = json;
            var time = new Date();
            var time_str = time.getFullYear() + "-" + checkTime(time.getMonth() + 1) + "-" + checkTime(time.getDate()) + " " + checkTime(time.getHours()) + ':' + checkTime(time.getMinutes()) + ':' + checkTime(time.getSeconds());

            var html = [];
            html.push('<li>');
            html.push('<span class="announce-msg">', obj["content"], '</span>');
            html.push('<time class="announce-time"><span>', time_str.split(" ")[0], '</span><span>',
                time_str.split(" ")[1], '</span></time>');
            html.push('</li>');
            $("#announcelist").append(html.join(""));

            vhChat.announce.checkItems();
        },

        /**
         * 检查
         */
        checkItems: function() {
            var announceBox = $(".announce-box");
            announceBox.mCustomScrollbar("update");
            announceBox.mCustomScrollbar("scrollTo", "last");
        },

        /**
         * textarea发送消息
         * @param ta textarea
         */
        sendAnnounce: function (text) {
            var val;
            val = vhChat.utils.unicode(text);
            vhChat.interfaces.sendAnnouncement(val);
        }
    };

    function init() {
        var H = $('.vh-announce-box').height();
        $(".announce-box").height(H - 8);

        $('.mCustomScrollbar').mCustomScrollbar({
            mouseWheel: true,
            mouseWheelPixels: 100,
            scrollButtons: {
                scrollType: 'continuous',
                enable: true,
                scrollSpeed: 20,
                scrollAmount: 40
            },
            scrollInertia: 0,
            theme: "minimal"
        });
    }

    function events() {
        // disable backspace
        $(document).keydown(function (e) {
            var elid = $(document.activeElement).is('INPUT, TEXTAREA');
            if (e.keyCode === 8 && !elid) {
                e.preventDefault();
                return false;
            }
        });

        // textarea 输入框回车
        $("#mywords").on("keypress", function(e) {
            if (e.keyCode == 13 && !e.shiftKey) {
                // 直接增加本地显示
                var text = $.trim($('#mywords').val());
                if (!text) {
                    $('#mywords').val("");
                    return false;
                }
                var msg = {};
                msg.type = "announcement";
                msg.content = $.trim($('#mywords').val());
                //先回显
                vhChat.announce.appendAnnouncement(msg);
                //再发送
                vhChat.announce.sendAnnounce($(e.currentTarget));
                return false;
            }
            return true;
        });
    }

    // 初始化
    $(function() {
        init();
        events();
        vhChat.interfaces.announceInit();
        // test
        //_test();
    });

})(window, jQuery);

///////////////////////////////////////

function _test() {
    appendChatMsg(test_announce);
    appendChatMsg(test_announce);
}
function appendChatMsg(msg) {
    var msg_json = JSON.parse(msg);
    vhChat.announce.appendAnnouncement(msg_json);
}

function preSendNoticeMsg(text) {
    // 直接增加本地显示
    var msg = {};
    msg.type = "announcement";
    msg.content = text;
    //先回显
    vhChat.announce.appendAnnouncement(msg);
    //再发送
    vhChat.announce.sendAnnounce(text);
    return false;
}

var test_announce = "{\"type\": \"announcement\",\"content\": \"公告内容\"}";
