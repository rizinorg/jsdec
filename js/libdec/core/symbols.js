// SPDX-FileCopyrightText: 2018-2021 Giovanni Dante Grazioli <deroad@libero.it>
// SPDX-License-Identifier: BSD-3-Clause

(function() { // lgtm [js/useless-expression]
    const Long = require('libdec/long');
    const rzpipe = require('libdec/rzpipe');
    const Utils = require('libdec/core/utils');

    var _compare = function(a, b) {
        if (a.eq(b.location)) {
            return 0;
        } else if (a.lt(b.location)) {
            return 1;
        }
        return -1;
    };

    var _virtual_compare = function(a, b) {
        return a.vaddr.lt(b.vaddr) ? -1 : (a.vaddr.eq(b.vaddr) ? 0 : 1);
    };

    var _physical_compare = function(a, b) {
        return a.paddr.lt(b.paddr) ? -1 : (a.paddr.eq(b.paddr) ? 0 : 1);
    };

    var _sanitize = function(x) {
        return x.paddr || x.vaddr;
    };

    /*
     * Expects the isj json as input.
     */
    return function(isj) {
        this.data = isj.filter(_sanitize).sort(Global.evars.honor.paddr ? _physical_compare : _virtual_compare).map(function(x) {
            return {
                location: Global.evars.honor.paddr ? x.paddr : x.vaddr,
                value: (x.demname && x.demname.length > 0) ? x.demname : x.name,
            };
        });
        this.search = function(address) {
            if (address) {
                if (!Global.evars.extra.slow) {
                    var def = {
                        symbols: {}
                    };
                    var x = rzpipe.json64('is.j @ 0x' + address.toString(16), def);
                    if (x.length != 1) {
                        return null;
                    }
                    x = x[0];
                    var loc = (Global.evars.honor.paddr ? x.paddr : x.vaddr) || Long.MAX_UNSIGNED_VALUE;
                    return address.eq(loc) && !Long.MAX_UNSIGNED_VALUE.eq(loc) ? ((x.demname && x.demname.length > 0) ? x.demname : x.name) : null;
                }
                var r = Utils.search(address, this.data, _compare);
                return r ? r.value : null;
            }
            return null;
        };
    };
});