/**
 * Created by shen on 2016/10/24.
 */
;(function (window, $, undefined) {
    window.vhChat = {
        url: {},
        constant: {},
        interfaces: {},
        utils: {
            unicode: function (str) {
                return decodeURI(str).replace(/%u/gi, '\\u');
            }
        }
    };
})(window, jQuery);
