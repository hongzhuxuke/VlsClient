/**
 * Created by shen on 2016/10/20.
 */

var pageinfo2 = {};
var getInitPage = 0;
var getInitForbid = 0;

var pageinfo = {
    'join_uid': '',
    'join_uname': '',
    'user_avatar': ''
};

$(document).ready(function () {
    $(".tip-bar").click(function () {
        chatlist = $(".chatlist-box");
        chatlist.mCustomScrollbar("update");
        chatlist.mCustomScrollbar("scrollTo", "last");
    });
});

;(function (window, $, undefined) {

    var vhChat = window.vhChat;
    vhChat.fun = {
        /**
         * 把聊天消息加入列表
         * @param json  传过来的数据
         */
        appendMessage: function (json) {
            //var obj = json["text"];
            var obj = json;
            if (obj["user_id"] == pageinfo.join_uid) { //当前是本人发的消息，不继续
                return;
            }
            if (!obj["avatar"]) {
                obj["avatar"] = "qrc:/chat/chat_html/images/head50.jpg";
            }
            var type, typeImg;
            switch (obj["role"]) {
                case "host":
                    type = "host";
                    typeImg = "qrc:/chat/chat_html/images/host.png";
                    break;
                case "guest":
                    type = "guest";
                    typeImg = "qrc:/chat/chat_html/images/guest.png";
                    break;
                case "assistant":
                    type = "assistant";
                    typeImg = "qrc:/chat/chat_html/images/assistant.png";
                    break;
                case "user":
                    type = "user";
                    break;
            }

            var hasMenu = type === "host" ? "" : "menu"; // 主持人没有右键菜单

            var msg = obj["data"]["text"];
            msg = replace_em(msg, "qrc:/chat/chat_html/images");
            /*
            var isat = msg.indexOf("@");
            if (isat == 0) {
                // @someone:msg，blue style
                if (replace_text(msg)) {
                    var str = msg;
                    //this.get_chat(str, msg.user_name);
                    //return;
                    str = msg.match(/\@([\s\S]*)\:/g);
                    for (var i = 0; i < 1; i++) {
                        msg = msg.replace(str[i], '<span style="color:#3485FC;">' + str[i] + '</span>');
                    }
                }
            }
            */
            var uname = obj["user_name"];
            var uname_abbr = uname; // 名字大于规定的字符数用省略号
            if (uname_abbr.length > vhChat.constant.max_name_length) {
                uname_abbr = uname.slice(0, vhChat.constant.max_name_length) + "...";
            }

            var html = [];
            html.push('<li uid="', obj["user_id"], '" uname="', uname, '">');
            html.push('<span class="wsstatus"></span>');
            html.push('<a class="uhead fl" title="', obj["user_name"], '"', hasMenu, '>');
            html.push('<img src="', obj["avatar"], '" alt="', uname, '的头像"></a>');
            html.push('<div class="ccon"><p class="ctop">');
            html.push('<a class="nickname nowrap gray fl" title="', uname, '"', hasMenu, '>');
            typeImg && html.push('<img src="', typeImg, '" width="46" height="18">');
            html.push(uname_abbr, '</a>');
            //html.push('<time class="ctime">', obj["time"].split(" ")[1], '</time>');
            html.push('</p>');
            html.push('<p class="cwords wordswrap">', msg, '</p>');

            html.push('</div>');
            html.push('</li>');

            $("#chatlist").append(html.join(""));

            vhChat.fun.tipBar(obj["data"]["text"], uname);
            vhChat.fun.checkItems();
        },
        
        /**
         * 把本地聊天消息加入列表
         * @param json  传过来的数据
         */
        appendlocalMessage: function (json) {
            var obj = json;
            var type, typeImg;
            switch (pageinfo.role){
                case "host":
                    type = "host";
                    typeImg = "qrc:/chat/chat_html/images/host.png";
                    break;
                case "guest":
                    type = "guest";
                    typeImg = "qrc:/chat/chat_html/images/guest.png";
                    break;
                case "assistant":
                    type = "assistant";
                    typeImg = "qrc:/chat/chat_html/images/assistant.png";
                    break;
                case "user":
                    type = "user";
                    break;
            }
                    
            var hasMenu = type === "host" ? "" : "menu"; // 主持人没有右键菜单
            //var msg = replace_em(obj["text"], vhChat.url.VH_CHAT_HOST);
            var msg = obj["text"];
            var uname = obj["_user_name"];
            var uname_abbr = uname; // 名字大于规定的字符数用省略号
            if (uname_abbr.length > vhChat.constant.max_name_length) {
                uname_abbr = uname.slice(0, vhChat.constant.max_name_length) + "...";
            }

            var html = [];
            html.push('<li uid="', obj["user_id"], '" uname="', uname, '">');
            html.push('<span class="wsstatus"></span>');
            html.push('<a class="uhead fl" title="', uname, '"', hasMenu, '>');
            html.push('<img src="', obj["avatar"], '" alt="', uname, '的头像"></a>');
            html.push('<div class="ccon"><p class="ctop">');
            html.push('<a class="nickname nowrap gray fl" title="', uname, '"', hasMenu, '>');
            typeImg && html.push('<img src="', typeImg, '" width="46" height="18">');
            html.push(uname_abbr, '</a>');
            //html.push('<time class="ctime">', obj["time"], '</time>');
            html.push('</p>');
            html.push('<p class="cwords wordswrap">', msg, '</p>');
            html.push('</div>');
            html.push('</li>');

            $("#chatlist").append(html.join(""));

            var chatlist = $(".chatlist-box");
            chatlist.mCustomScrollbar("update");
            chatlist.mCustomScrollbar("scrollTo", "last");
            vhChat.fun.checkItems();
        },

        /**
         * 打赏消息加入列表
         * @param json
         */
        appendRewardMessage: function (json) {
            var obj = json;
            //var msg = replace_em(obj["data"]["note"], "qrc:/chat/chat_html/images");
            var msg = obj["data"]["note"];
            var uname = obj["user_name"];

            var html = [];
            html.push('<li uid="', obj["user_id"], '">');
            html.push('<a class="uhead fl">');
            html.push('<img src="qrc:/chat/chat_html/images/bell.png" alt="', uname, '的红包打赏" width="26" height="26"></a>');
            html.push('<div class="ccon"><p class="ctop">');
            html.push('<a class="nickname nowrap gray fl" title="', uname, '的红包打赏">红包打赏</a>');
            //html.push('<time class="ctime">', obj["time"].split(" ")[1], '</time>');
            html.push('</p>');
            html.push('<p class="creward">用户', uname, '打赏了一个红包 <img src="qrc:/chat/chat_html/images/reward.png" /></p>');
            if (msg.length > 0) {
                html.push('<p class="cwords wordswrap reward">', msg, '</p>');
            }
            html.push('</div>');
            html.push('</li>');

            $("#chatlist").append(html.join(""));
            vhChat.fun.tipBar(msg, uname);
            vhChat.fun.checkItems();
        },

        /**
         * 聊天列表不在最底下时如有新消息, 显示tip bar
         * @param msg
         * @param uname
         */
        tipBar: function (msg, uname) {
            var tip = $(".tip-bar"),
                chatlist = $(".chatlist-box"),
                mCSB_container = chatlist.find('.mCSB_container');

            // 滚动条不再最下时 显示提示栏
            if (mCSB_container.height() + parseInt(mCSB_container.css('top'), 10) >= chatlist.height() + 100) {
                var html = [];
                //html.push('<span>', uname, ":<span class='tip-msg'>", msg, '</span></span>');
                html.push('<p><a class="name" href="javascript:;">', uname, '</a>');
                html.push(':', '<span class="content">', msg, '</span></p>');
                tip.html(html.join("")).show();

                setTimeout(function () {
                    tip.hide("slow");
                }, 3000);
            } else {
                chatlist.mCustomScrollbar("update");
                chatlist.mCustomScrollbar("scrollTo", "last");
            }
        },

        /**
         * 检查消息列表条目数 大于200, 删除以前的
         */
        checkItems: function () {
            var chatlist = $("#chatlist");
            var lis = chatlist.find("li");
            if (lis.length > 100) {
                lis.eq(0).remove();
                //var chatlistBox = $(".chatlist-box");
                //chatlistBox.mCustomScrollbar("update");
                //chatlistBox.mCustomScrollbar("scrollTo", "last");
            }
        },

        /**
         * textarea发送消息
         * @param ta textarea
         */
        sendMessage: function (ta) {
            // 去除开头 @xxx:,中间的@xxx不去除
            //val = val.replace(/^(@.*?:)/, "");
            var val;
            val = vhChat.utils.unicode(ta);
            
            // TODO: 发送消息服务器
            vhChat.interfaces.sendOutMsg(val);
            
        }

    };

    function removeMenuFunc() {
        $("#rightMenu").remove();
    };

    function init() {
        var H = $('.vh-chat-box').height();
        $(".chatlist-box").height(H - 36 - 4);

        $('.mCustomScrollbar').mCustomScrollbar({
            mouseWheel:true,
            mouseWheelPixels: 100,
            scrollButtons:{
                scrollType:'continuous',
                enable:true,
                scrollSpeed:20,
                scrollAmount:40
            },
            scrollInertia: 0,
            theme: "minimal",
            callbacks: {
                onScroll: function (e) {
                    removeMenuFunc();
                }
            }
        });
    }

    function events() {
        // disable backspace
        $(document).keydown(function(e) {
            var elid = $(document.activeElement).is('INPUT, TEXTAREA') ;
            if (e.keyCode === 8 && !elid) {
                 e.preventDefault();
                 return false;
            }
        });

        // qq face
        var faceUrl = "qrc:/chat/chat_html/imgages/arclist/";
        if ($('.expression').length) {
            $('.expression').qqFace({
                id: 'facebox',
                assign: 'mywords',
                path: faceUrl //表情存放的路径
            });
        }

        // 右键菜单
        function _menu(e) {
            e.preventDefault();
            var target = e.currentTarget; // <a>
            var li = $(target).parents("li");
            var uid = li.attr("uid"); // uid
            //var forbid = li.attr("forbid"); // 禁言 undefined
            //var kickout = li.attr("kickout"); // 踢出 undefined
            var forbid, kickout;
            forbid = MainWindow.getForbidStatus(uid);
            kickout = MainWindow.getKickoutStatus(uid);
            var wide = forbid || kickout; // 有其中一个用宽的图片

            var x = e.clientX;
            var y = e.clientY;

            removeMenu();
            _showMenu();

            function _showMenu() {
                var m = $("#rightMenu");
                if (!m.length) {
                    var html = [];
                    html.push('<div id="rightMenu" class="menu ', wide ? "wide" : "", '" style="top:', y, 'px;left:', x, 'px">');
                    html.push('<ul>');
                    html.push('<li data-uid="', uid, '" data-action="reply" class="icon icon-reply">回复</li>');
                    if (!forbid) {
                        html.push('<li data-uid="', uid, '" data-action="forbid" class="icon icon-forbid">禁言</li>');
                    } else {
                        html.push('<li data-uid="', uid, '" data-action="forbidCancel" class="icon icon-forbid-cancel">取消禁言</li>');
                    }
                    if (!kickout) {
                        html.push('<li data-uid="', uid, '" data-action="kickOut" class="icon icon-kick">踢出</li>');
                    } else {
                        html.push('<li data-uid="', uid, '" data-action="kickOutCancel" class="icon icon-kick-cancel">取消踢出</li>');
                    }
                    html.push('</ul>');
                    html.push('</div>');
                    $(document.body).append(html.join(""));
                    m = $("#rightMenu");
                } else {
                    m.css({
                        top: y + "px",
                        left: x + "px"
                    });
                }
                if (y > 404) {
                    m.css({
                        top: "404px",
                        left: x + "px"
                    });
                }
                    

                m.show();
            }

        }

        // 去除右键菜单
        function removeMenu() {
            $("#rightMenu").remove();
        }

        // 右键菜单 li事件
        $(document).on("click", "#rightMenu li", function(e) {
            var target = $(e.currentTarget);
            var uid = target.attr("data-uid"); // uid
            var li = $("#chatlist").find("li[uid=" + uid + "]");
            var action = target.attr("data-action");
            switch(action) {
                case "reply": // 回复
                    var uname = li.attr("uname");
                    //var ta = $("#mywords");
                    //ta.attr("data-to", uid);
                    //ta.focus();
                    //ta.val("@" + uname + ":");
                    vhChat.interfaces.atSomeone(uname);
                    break;
                case "forbid": // 禁言
                    li.attr("forbid", 1);
                    vhChat.interfaces.forbid_someone(uid, true);
                    break;
                case "forbidCancel": // 取消禁言
                    li.removeAttr("forbid");
                    vhChat.interfaces.forbid_someone(uid, false);
                    break;
                case "kickOut": // 踢出
                    li.attr("kickout", 1);
                    vhChat.interfaces.kickout_someone(uid, true);
                    break;
                case "kickOutCancel": // 取消踢出
                    li.removeAttr("kickout");
                    vhChat.interfaces.kickout_someone(uid, false);
                    break;
                default:
                    break;
            }
            removeMenu();
        });

        $("#chatlist").on("contextmenu", "a[menu]", _menu);

        // textarea 输入框回车
        $("#mywords").on("keypress", function(e) {
            if (e.keyCode == 13 && !e.shiftKey) {
                // 直接增加本地显示
                var msg = {};
                var time = new Date();
                msg.time = (time.getHours() > 9 ? time.getHours() : ('0' + time.getHours())) + ':' + (time.getMinutes() > 9 ? time.getMinutes() : ('0' + time.getMinutes()));
                msg.avatar = pageinfo.user_avatar;
                var text = $.trim($('#mywords').val());
                if (!text)
                    return false;
                msg.text = $.trim($('#mywords').val());
                msg.text = replace_em(msg.text, "qrc:/chat/chat_html/images");
                msg.user_name = pageinfo.join_uname;
                msg.user_id = pageinfo.join_uid;
                var role = "<img src='qrc:/chat/chat_html/images/";
                switch(pageinfo.role) {
                    case 'host' : 
                        role += "role_host.png' width='38' height='15'>&nbsp;";
                        break;
                    case 'guest' :
                        role += "role_guest.png' width='38' height='15'>&nbsp;";
                        break;
                    case 'assistant' :
                        role += "role_assistant.png' width='38' height='15'>&nbsp;";
                        break;
                    default :
                        role = "";
                        break;
                }
                msg._user_name = msg.user_name;
                msg.user_name = role + msg.user_name;
                if (parseInt(pageinfo.matchTheLink)) {
                    msg.text = msgHand.replace_link(msg.text);
                }
                if (!msg.avatar) {
                    msg.avatar = "qrc:/chat/chat_html/images/head50.jpg";
                }
                if ( pageinfo.inituser && parseInt(pageinfo.inituser) == 1 ) {
                    msg.inituser = 1;
                    msg.avatar = '';
                }
                vhChat.fun.appendlocalMessage(msg);
                
                // 开始发送数据
                vhChat.fun.sendMessage($(e.currentTarget));
                return false;
            }
            return true;
        });

        // 全体禁言
        $("#forbid_all").on("click", function (e) {
            var target = $(e.currentTarget);
            var aim;
            if (target.hasClass("chat-ctr-active")) { // 已经禁言
                aim = false;
                //target.removeClass("chat-ctr-active");
            } else {
                aim = true;
                //target.addClass("chat-ctr-active");
            }
            vhChat.interfaces.forbid_all(aim);
        });

        // 聊天过滤
        $(".chat-ctr-filter").on("click", function(e) {
            var target = $(e.currentTarget);
            var aim;
            if (target.hasClass("chat-ctr-active")) { // 不要过滤
                aim = false;
                //target.removeClass("chat-ctr-active");
            } else {
                aim = true;
                //target.addClass("chat-ctr-active");
            }
            vhChat.interfaces.chatFilter(aim);
        });

        // 单击任意处 取消右键菜单
        $(document).on("click", function() {
            removeMenu();
        });

        // resize
        $(window).on("resize", function() {
            var chatlist = $(".chatlist-box");
            chatlist.mCustomScrollbar("update");
        });
    }

    // 初始化
    $(function() {
        init();
        events();
        // init pageinfo for loop
        var interval = setInterval(function () {
            vhChat.interfaces.chatInit()
            if (getInitPage != 0) {
                clearInterval(interval);
                return;
            }
        }, 200);

        //init forbidAll status for loop
        var intervalF = setInterval(function () {
            vhChat.interfaces.chatInitForbidStatus()
            if (getInitForbid != 0) {
                clearInterval(intervalF);
                return;
            }
        }, 200);
        //vhChat.interfaces.chatInit();

        // test
        //_test();
    });
})(window, jQuery);

///////////////////////////////////////

function _test() {
    appendChatMsg(test_host);
    appendChatMsg(test_user);
    appendChatMsg(test_host);
    appendChatMsg(test_reward);
    appendChatMsg(test_host);
    appendChatMsg(test_user);
    appendChatMsg(test_host);
    appendChatMsg(test_user);
    appendChatMsg(test_reward);
    appendChatMsg(test_host);
    var chatlist = $(".chatlist-box");
    chatlist.mCustomScrollbar("update");
    chatlist.mCustomScrollbar("scrollTo", "last");
};

function appendChatMsg(msg) {
    var msg_json = JSON.parse(msg);
    var event = msg_json["event"];
    switch (event) {
        case "msg":
            vhChat.fun.appendMessage(msg_json);
            break;
        case "pay":
            vhChat.fun.appendRewardMessage(msg_json);
            break;
        default:
            break;
    }
};

function preSendChatMsg(text) {
    // 直接增加本地显示
    var msg = {};
    var time = new Date();
    msg.time = (time.getHours() > 9 ? time.getHours() : ('0' + time.getHours())) + ':' + (time.getMinutes() > 9 ? time.getMinutes() : ('0' + time.getMinutes()));
    msg.avatar = pageinfo.user_avatar;
    msg.text = text;
    msg.text = replace_em(msg.text, "qrc:/chat/chat_html/images");
    msg.user_name = pageinfo.join_uname;
    msg.user_id = pageinfo.join_uid;
    var role = "<img src='qrc:/chat/chat_html/images/";
    switch (pageinfo.role) {
        case 'host':
            role += "role_host.png' width='38' height='15'>&nbsp;";
            break;
        case 'guest':
            role += "role_guest.png' width='38' height='15'>&nbsp;";
            break;
        case 'assistant':
            role += "role_assistant.png' width='38' height='15'>&nbsp;";
            break;
        default:
            role = "";
            break;
    }
    msg._user_name = msg.user_name;
    msg.user_name = role + msg.user_name;
    if (parseInt(pageinfo.matchTheLink)) {
        msg.text = msgHand.replace_link(msg.text);
    }
    if (!msg.avatar) {
        msg.avatar = "qrc:/chat/chat_html/images/head50.jpg";
    }
    if (pageinfo.inituser && parseInt(pageinfo.inituser) == 1) {
        msg.inituser = 1;
        msg.avatar = '';
    }
    vhChat.fun.appendlocalMessage(msg);

    // 开始发送数据
    vhChat.fun.sendMessage(text);
    return false;
}

//收到消息时，更细按钮状态
function forbidAll() {
    if ($("#forbid_all").hasClass("chat-ctr-forbid"))
        $("#forbid_all").removeClass("chat-ctr-forbid");
    if (!$("#forbid_all").hasClass("chat-ctr-active"))
        $("#forbid_all").addClass("chat-ctr-active");
    $("#forbid_all").attr("title", "取消全体禁言");
}

function cancelForbidAll() {
    if ($("#forbid_all").hasClass("chat-ctr-active"))
        $("#forbid_all").removeClass("chat-ctr-active");
    if (!$("#forbid_all").hasClass("chat-ctr-forbid"))
        $("#forbid_all").addClass("chat-ctr-forbid");
    $("#forbid_all").attr("title", "全体禁言");
}

//第一次进入时，同步按钮状态
function doSetForbidStatus() {
    //if (!$("#forbid_all").hasClass("chat-ctr-active")) {
    $("#forbid_all").removeClass("chat-ctr-forbid");
    $("#forbid_all").addClass("chat-ctr-active");
    $("#forbid_all").attr("title","取消全体禁言");
    //}
}
function docancelForbidStatus() {

}

function hideMenuFunc() {
    $("#rightMenu").remove();
};

var test_host = "{\"user_id\": \"7347133\",\"account_id\": \"46616\",\"user_name\": \"\u90dd\u4ed8\u58ee\",\"avatar\": \"http:\/\/cnstatic01.e.vhall.com\/upload\/user_avatar\/1d\/6e\/1d6edbf3d3895794bab2b53fb84dc3b9\",\"room\": \"277189719\",\"role\": \"host\",\"event\": \"msg\",\"data\": {\"text\": \"[\u9119\u89c6][\u9634\u9669]\"},\"time\": \"2016-10-20 16:07:43\",\"real_room\": \"277189719\",\"app\": \"vhall\",\"to\": \"\"}";

var test_user = "{\"user_id\": \"19092607\",\"account_id\": \"0\",\"user_name\": \"vuser1432861897\",\"avatar\": \"\",\"room\": \"277189719\",\"role\":\"user\",\"event\": \"msg\",\"data\": {\"text\": \"999999999999\"},\"time\": \"2016-10-20 16:13:12\",\"real_room\": \"277189719\",\"app\": \"vhall\",\"to\": \"\"}";

var test_reward = "{\"user_id\": \"1\",\"account_id\": \"48256\",\"user_name\":\"\u7af9\u77f3\",\"avatar\":\"./images/bell.png\",\"room\":\"106350882\",\"role\": \"system\",\"event\": \"pay\",\"data\":{\"icon\": \"http:\/\/cnstatic01.e.vhall.com\/upload\/user\/avatar\/92\/fa\/92fa27246f6a9492fa9dc1d5ad279a1e.jpg?size=200x200\",\"nick_name\": \"\u7af9\u77f3\",\"fee\": \"0.01\",\"pay_id\": \"48256\",\"take_id\": \"46616\",\"note\": \"\u5f88\u7cbe\u5f69\uff0c\u8d5e\u4e00\u4e2a\uff01\",\"text\": \"\u5f88\u7cbe\u5f69\uff0c\u8d5e\u4e00\u4e2a\uff01\",\"pay_num\": 1},\"time\": \"2016-10-21 11:14:52\",\"real_room\": \"106350882\",\"app\": \"vhall\",\"to\": \"\"}";

function InitPageInfo(userName,userImageUrl,userId) {

}

function replace_text(str) { //去除@信息
    var reg = new RegExp(/\@([\s\S]*)\:/);
    return reg.test(str);
}
function get_chat(str, user_name) { //给被@用户发送消息
    if (str.indexOf('@' + pageinfo.join_uname + ':') >= 0 || str.indexOf('@' + pageinfo.join_uname + '主持人:') >= 0) { //判断是否为@人
        str = str.replace(/\@([\s\S]*)\:/g, '').replace(/wenfengleivhall123/g,'@2x.png');
        var dom = $('<div style="background:#fff;box-shadow:0 0 2px;color:#333;padding:2px 5px;"><p><a href="javascipt:;" style="max-width:150px;width:auto;line-height:12px;display:inline-block;color:#3485FC;" class="textofover nowrap nickname" title="' + user_name + '">' + user_name + '</a>@你：' + str + '</p></div>');
        $("#get_info-box").append(dom);
        var timer = setTimeout(function() {
            dom.remove();
        }, 3000);
    }
}
/*
var test_host = {
    "id": 24,
    "channel": "277189719",
    "text": {
        "user_id": "7347133",
        "account_id": "46616",
        "user_name": "\u90dd\u4ed8\u58ee",
        "avatar": "http:\/\/cnstatic01.e.vhall.com\/upload\/user_avatar\/1d\/6e\/1d6edbf3d3895794bab2b53fb84dc3b9",
        "room": "277189719",
        "role": "host",
        "event": "msg",
        "data": {
            "text": "[\u9119\u89c6][\u9634\u9669]"
        },
        "time": "2016-10-20 16:07:43",
        "real_room": "277189719",
        "app": "vhall",
        "to": ""
    },
    "tag": "3",
    "time": "Thu, 20 Oct 2016 08:07:43 GMT"
};

var test_user = {
    "id": 26,
    "channel": "277189719",
    "text": {
        "user_id": "19092607",
        "account_id": "0",
        "user_name": "vuser1432861897",
        "avatar": "",
        "room": "277189719",
        "role": "user",
        "event": "msg",
        "data": {
            "text": "999999999999"
        },
        "time": "2016-10-20 16:13:12",
        "real_room": "277189719",
        "app": "vhall",
        "to": ""
    },
    "tag": "1",
    "time": "Thu, 20 Oct 2016 08:13:12 GMT"
};

var test_reward = {
    "id": 19,
    "channel": "106350882",
    "text": {
        "user_id": "1",
        "account_id": "48256",
        "user_name": "\u7af9\u77f3",
        "avatar": "qrc:/chat/chat_html/images/bell.png",
        "room": "106350882",
        "role": "system",
        "event": "pay",
        "data": {
            "icon": "http:\/\/cnstatic01.e.vhall.com\/upload\/user\/avatar\/92\/fa\/92fa27246f6a9492fa9dc1d5ad279a1e.jpg?size=200x200",
            "nick_name": "\u7af9\u77f3",
            "fee": "0.01",
            "pay_id": "48256",
            "take_id": "46616",
            "note": "\u5f88\u7cbe\u5f69\uff0c\u8d5e\u4e00\u4e2a\uff01",
            "text": "\u5f88\u7cbe\u5f69\uff0c\u8d5e\u4e00\u4e2a\uff01",
            "pay_num": 1
        },
        "time": "2016-10-21 11:14:52",
        "real_room": "106350882",
        "app": "vhall",
        "to": ""
    },
    "tag": "4",
    "time": "Fri, 21 Oct 2016 03:14:52 GMT"
};
*/