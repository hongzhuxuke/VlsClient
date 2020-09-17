// QQ表情插件
(function($) {
    $.fn.qqFace = function(options) {
        var defaults = {
            id: 'facebox',
            path: 'face/',
            assign: 'content',
            tip: 'em_'
        };
        var option = $.extend(defaults, options);
        var assign = $('#' + option.assign);
        var id = option.id;
        var path = option.path;
        var tip = option.tip;
        //if (assign.length <= 0) {
        //    alert('缺少表情赋值对象。');
        //    return false;
        //}

        $(this).click(function(e) {
            var strFace, labFace, offset, top;
            if($(this).hasClass('disabled')){
                return;
            }
            var dom = $(this).parent().find('.' + id);
            if (dom.length <= 0) {
                if (IsPC()) {
                    $("body").append("<style>.facebox{background: #f7f7f7;padding: 2px;border: 1px #afafaf solid;}.facebox table td{padding:0px;}.facebox table td img{cursor:pointer;border:1px #f7f7f7 solid;}.facebox table td img:hover{border:1px #0066cc solid;}</style>");
                    strFace = '<div class="' + id + '" style="position:absolute;display:none;z-index:1002;top:-266px;left:-5px;" class="qqFace mCustomScrollbar">' +
                        '<table border="0" cellspacing="0" cellpadding="0"><tr>';
                    for (var i = 1; i <= 90; i++) {
                        labFace = getFace_index(i); //'['+tip+i+']';
                        strFace += '<td><img width="24" src="' + path + 'Expression_' + i + '@2x.png" onclick="$(\'#' + option.assign + '\').setCaret();$(\'#' + option.assign + '\').insertAtCaret(\'' + labFace + '\');" /></td>';
                        if (i % 9 === 0) strFace += '</tr><tr>';
                    }
                    strFace += '</tr></table></div>';
                    $(this).parent().append(strFace);
                    offset = $(this).position();
                    /*top = offset.top + $(this).outerHeight();
                    dom.css('top', -266);
                    dom.css('left', -5);*/
                } else {
                    $("body").append("<style>.facebox {background: #fff;border-top: 1px solid #ddd;}.facebox .qqFace{float: left;text-align: left;}.facebox li{display: inline-block;padding: 10px 0;width: 14%;text-align: center;}.facebox .text-center a{width:10px;height: 10px;border-radius: 100%;background: #ddd;border: none;color: #fff;margin: 5px 15px 0 0;display: inline-block;text-decoration: none;}.facebox .text-center a.active{background: #ff3334;}</style>");
                    var windowWidth = $(window).width(),
                        control = '';
                    strFace = '<div class="' + id + '" style="position:absolute;display:none;z-index:1002;width:' + windowWidth + 'px;overflow:hidden;height:152px;padding: 0 1%;top:-152px;"><div class="qqFace-box" style="width:' + windowWidth * 5 + 'px">';
                    for (var i = 1; i <= 5; i++) {
                        strFace += '<div class="qqFace" style="width:' + windowWidth + 'px">';
                        for (var j = 1; j <= 20; j++) {
                            var index = (i - 1) * 20 + j;
                            labFace = getFace_index(index);
                            if (!labFace) {
                                continue;
                            }
                            strFace += '<li onclick="$(\'#' + option.assign + '\').setCaret();$(\'#' + option.assign + '\').insertAtCaret(\'' + labFace + '\');"><img width="24" src="' + path + 'Expression_' + index + '@2x.png" /></li>';
                        }
                        strFace += '<li onclick="$(\'#' + option.assign + '\').deleteCaret();" ><img width="24" src="' + path + 'faceDelete@2x.png" /></li></div>';
                        if (i === 1) {
                            control += "<a class='active'></a>";
                        } else {
                            control += "<a></a>";
                        }

                    }
                    strFace += "<div style='clear:both'></div></div><div class='text-center'>" + control + "</div></div>";
                    $(this).parent().append(strFace);
                    offset = $(this).position();
                    dom = $(this).parent().find('.' + id);
                    /*top = offset.top + $(this).outerHeight();
                    dom.css('top', -152);*/
                    //dom.css('left', -5);
                    dom.data('data', {
                        index: 0
                    });
                    var isDrag = false,
                        tx, x, el = dom.find(".qqFace-box").eq(0).get(0);
                    dom[0].addEventListener('touchstart', function(e) {
                        //e.preventDefault();
                        //isDrag = true;
                        tx = e.touches[0].pageX;
                    }, false);
                    dom[0].addEventListener('touchmove', function(e) {
                        e.preventDefault();	
                        x = e.touches[0].pageX - tx;
                        var width = $(this).data('data').index * windowWidth;
                        var value = 'translate3d(' + (x - width) + 'px, 0, 0)';
                        el.style.webkitTransform = value;
                        el.style.mozTransform = value;
                        el.style.transform = value;
                        isDrag = true;	
                    }, false);
                    dom[0].addEventListener('touchend', function(e) {
                        if (isDrag) {
                            var index = $(this).data('data').index,
                                width = index * windowWidth;
                            if (x < -50) { //左滑
                                if (index < 4) {
                                    index = index + 1;
                                    $(this).data('data', {
                                        index: index
                                    });
                                    width += windowWidth;
                                }
                            } else if (x > 50) {
                                if (index >= 1) {
                                    index = index -1 ;
                                    $(this).data('data', {
                                        index: index
                                    });
                                    width -= windowWidth;
                                }
                            }
                            dom.find(".text-center a").removeClass('active').eq(index).addClass('active');
                            var value = 'translate3d(-' + width + 'px, 0, 0)';
                            el.style.webkitTransform = value;
                            el.style.mozTransform = value;
                            el.style.transform = value;
                        }
                        isDrag = false;
                    }, false);
                    dom.on("click","li",function(e){
                        e.stopPropagation();
                    });
                }
            }

            $(this).parent().find('.' + id).toggle();
            e.stopPropagation();
        });

        $(document).click(function() {
            $('.' + id).hide();
            //$('#' + id).remove();
        });
    };

})(jQuery);
jQuery.fn.extend({
    selectContents: function() {
        $(this).each(function(i) {
            var node = this;
            var selection, range, doc, win;
            if ((doc = node.ownerDocument) && (win = doc.defaultView) && typeof win.getSelection != 'undefined' && typeof doc.createRange != 'undefined' && (selection = window.getSelection()) && typeof selection.removeAllRanges != 'undefined') {
                range = doc.createRange();
                range.selectNode(node);
                if (i === 0) {
                    selection.removeAllRanges();
                }
                selection.addRange(range);
            } else if (document.body && typeof document.body.createTextRange != 'undefined' && (range = document.body.createTextRange())) {
                range.moveToElementText(node);
                range.select();
            }
        });
    },

    setCaret: function() {
        if (!(/msie/.test(navigator.userAgent.toLowerCase()))) return;
        var initSetCaret = function() {
            var textObj = $(this).get(0);
            textObj.caretPos = document.selection.createRange().duplicate();
        };
        $(this).click(initSetCaret).select(initSetCaret).keyup(initSetCaret);
    },

    insertAtCaret: function(textFeildValue) {
        var textObj = $(this).get(0);
        if (document.all && textObj.createTextRange && textObj.caretPos) {
            var caretPos = textObj.caretPos;
            caretPos.text = caretPos.text.charAt(caretPos.text.length - 1) === '' ?
                textFeildValue + '' : textFeildValue;
        } else if (textObj.setSelectionRange) {
            var rangeStart = textObj.selectionStart;
            var rangeEnd = textObj.selectionEnd;
            var tempStr1 = textObj.value.substring(0, rangeStart);
            var tempStr2 = textObj.value.substring(rangeEnd);
            textObj.value = tempStr1 + textFeildValue + tempStr2;
            var len = textFeildValue.length;
            textObj.setSelectionRange(rangeStart + len, rangeStart + len);
            //textObj.blur(); //注释点为了让选择表情后输入框聚焦
            if(!IsPC()){
               $(this).blur();
            }else{
               $(this).focus(); 
            }
        } else {
            textObj.value += textFeildValue;
        }
    },
    /**
     * [deleteCaret description]
     * @return {[type]} [description]
     */
    deleteCaret: function() {
        var textObj = $(this),
            val = textObj.val(),
            reg = /(\[[^@]{1,3}\])$/;
        if (reg.test(val))
            val = val.replace(reg, '');
        else
            val = val.substring(0, val.length - 1);
        textObj.val(val);
        textObj.blur();
    }
});

/**
 * [IsPC description]

 */
function IsPC() {
    var userAgentInfo = navigator.userAgent;
    var Agents = ["Android", "iPhone",
        "SymbianOS", "Windows Phone",
        "iPad", "iPod"
    ];
    var flag = true;
    for (var v = 0; v < Agents.length; v++) {
        if (userAgentInfo.indexOf(Agents[v]) > 0) {
            flag = false;
            break;
        }
    }
    return flag;
}

/**
 *	get array key or value
 *	@param val,string or num
 **/
function getFace_index(val) {
    var list = [],
        face_list = {
            '[微笑]': 1,
            '[撇嘴]': 2,
            '[色]': 3,
            '[发呆]': 4,
            '[得意]': 5,
            '[流泪]': 6,
            '[害羞]': 7,
            '[闭嘴]': 8,
            '[睡]': 9,
            '[哭]': 10,
            '[尴尬]': 11,
            '[发怒]': 12,
            '[调皮]': 13,
            '[呲牙]': 14,
            '[惊讶]': 15,
            '[难过]': 16,
            '[酷]': 17,
            '[汗]': 18,
            '[抓狂]': 19,
            '[吐]': 20,
            '[偷笑]': 21,
            '[愉快]': 22,
            '[白眼]': 23,
            '[傲慢]': 24,
            '[饥饿]': 25,
            '[困]': 26,
            '[惊恐]': 27,
            '[流汗]': 28,
            '[憨笑]': 29,
            '[悠闲]': 30,
            '[奋斗]': 31,
            '[咒骂]': 32,
            '[疑问]': 33,
            '[嘘]': 34,
            '[晕]': 35,
            '[疯了]': 36,
            '[衰]': 37,
            '[骷髅]': 38,
            '[敲打]': 39,
            '[再见]': 40,
            '[擦汗]': 41,
            '[抠鼻]': 42,
            '[鼓掌]': 43,
            '[糗大了]': 44,
            '[坏笑]': 45,
            '[左哼哼]': 46,
            '[右哼哼]': 47,
            '[哈欠]': 48,
            '[鄙视]': 49,
            '[委屈]': 50,
            '[快哭了]': 51,
            '[阴险]': 52,
            '[亲亲]': 53,
            '[吓]': 54,
            '[可怜]': 55,
            '[菜刀]': 56,
            '[西瓜]': 57,
            '[啤酒]': 58,
            '[篮球]': 59,
            '[乒乓]': 60,
            '[咖啡]': 61,
            '[饭]': 62,
            '[猪头]': 63,
            '[玫瑰]': 64,
            '[凋谢]': 65,
            '[嘴唇]': 66,
            '[爱心]': 67,
            '[心碎]': 68,
            '[蛋糕]': 69,
            '[闪电]': 70,
            '[炸弹]': 71,
            '[刀]': 72,
            '[足球]': 73,
            '[瓢虫]': 74,
            '[便便]': 75,
            '[月亮]': 76,
            '[太阳]': 77,
            '[礼物]': 78,
            '[拥抱]': 79,
            '[强]': 80,
            '[弱]': 81,
            '[握手]': 82,
            '[胜利]': 83,
            '[抱拳]': 84,
            '[勾引]': 85,
            '[拳头]': 86,
            '[差劲]': 87,
            '[爱你]': 88,
            '[NO]': 89,
            '[OK]': 90
        };

    if (typeof(val) === 'string') {
        return face_list[val];
    }
    if (typeof(val) === 'number') {
        for (var i in face_list) {
            list[face_list[i]] = i;
        }
        return list[val];
    }
}


//显示表情

function replace_em(str, url) {
    str = str.replace(/\</g, '&lt;');
    str = str.replace(/\>/g, '&gt;');
    str = str.replace(/\n/g, '<br/>');
    var reg = str.match(/\[[^@]{1,3}\]/g);
    if (reg !== null) {
        for (var i = 0; i < reg.length; i++) {
            var img_url = getFace_index(reg[i]);
            if (img_url) {
                str = str.replace(reg[i], '<img width="24" src="' + url + '/arclist/Expression_' + img_url + '@2x.png" border="0" />');
            }
        }
    }
    return str;
}
