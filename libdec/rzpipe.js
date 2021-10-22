// SPDX-FileCopyrightText: 2019-2021 Giovanni Dante Grazioli <deroad@libero.it>
// SPDX-License-Identifier: BSD-3-Clause

(function() { // lgtm [js/useless-expression]
    var JSON64 = require('libdec/json64');
    var Long = require('libdec/long');

    function rzcustom(value, regex, function_fix) {
        var x = rzcmd(value) || "";
        if (regex) {
            x = x.replace(regex, '');
        }
        return function_fix ? function_fix(x.trim()) : x.trim();
    }

    function rzstr(value, multiline) {
        var x = rzcmd(value) || "";
        if (multiline) {
            x = x.replace(/\n/g, '');
        }
        return x.trim();
    }

    function rzjson(m, def) {
        var x = rzstr(m, true);
        try {
            return x.length > 0 ? JSON.parse(x) : def;
        } catch(e){}
        return def;
    }

    function rzjson64(m, def) {
        var x = rzstr(m, true);
        try {
            return x.length > 0 ? JSON64.parse(x) : def;
        } catch(e){}
        return def;
    }

    function rzint(value, def) {
        var x = rzstr(value);
        if (x != '') {
            try {
                return parseInt(x);
            } catch (e) {}
        }
        return def || 0;
    }

    function rzlong(value, def) {
        var x = rzstr(value);
        if (x != '') {
            try {
                return Long.fromString(x, true, x.startsWith('0x') ? 16 : 10);
            } catch (e) {}
        }
        return def || Long.UZERO;
    }

    function rzbool(value) {
        var x = rzstr(value);
        return x == 'true' || x == '1';
    }

    return {
        custom: rzcustom,
        string: rzstr,
        json64: rzjson64,
        json: rzjson,
        int: rzint,
        long: rzlong,
        bool: rzbool,
    };
});