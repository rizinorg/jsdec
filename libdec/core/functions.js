/* 
 * Copyright (C) 2017-2018 deroad
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
    var rzpipe = require('libdec/rzpipe');
    var Utils = require('libdec/core/utils');

    var _compare_search = function(a, b) {
        if (a.eq(b.offset)) {
            return 0;
        } else if (a.lt(b.offset)) {
            return 1;
        }
        return -1;
    };

    var _compare_offsets = function(a, b) {
        return a.offset.lt(b.offset) ? -1 : (a.offset.eq(b.offset) ? 0 : 1);
    };

    var _compare_name = function(a, b) {
        return a.name < b.name ? -1 : (a.name == b.name ? 0 : 1);
    };

    var create_fcn_data = function(x) {
        if (!x) {
            return null;
        }
        return {
            offset: x.offset,
            name: x.name,
            calltype: x.calltype,
            nargs: x.nargs
        };
    };

    /*
     * Expects the aflj json as input.
     */
    var Functions = function(aflj, sort_by_name) {
        this.data = aflj.sort(sort_by_name ? _compare_name : _compare_offsets).map(function(x) {
            return create_fcn_data(x);
        });

        this.search = function(offset) {
            if (!Global.evars.extra.slow && offset) {
                var x = rzpipe.json64('afij @ 0x' + offset.toString(16), [])[0];
                return create_fcn_data(x);
            }
            return offset ? Utils.search(offset, this.data, _compare_search) : null;
        };
    };
    return Functions;
});