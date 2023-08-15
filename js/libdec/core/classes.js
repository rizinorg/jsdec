// SPDX-FileCopyrightText: 2019-2021 Giovanni Dante Grazioli <deroad@libero.it>
// SPDX-License-Identifier: BSD-3-Clause

(function() { // lgtm [js/useless-expression]
    /*
     * Expects the icj json as input.
     */
    return function(icj) {
        var data = {};
        icj.forEach(function(x) {
            data[x.addr.toString()] = x.classname;
        });
        this.data = data;
        this.search = function(address) {
            if (address) {
                return this.data[address.toString()];
            }
            return null;
        };
    };
});