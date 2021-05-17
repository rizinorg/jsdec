/* 
 * Copyright (C) 2019 deroad
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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