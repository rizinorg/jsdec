// SPDX-FileCopyrightText: 2018-2021 Giovanni Dante Grazioli <deroad@libero.it>
// SPDX-License-Identifier: BSD-3-Clause

(function() { // lgtm [js/useless-expression]
    var Color = function(name) {
        var fn = function(x) {
            return x;
        };
        return fn;
    };
    Color.make = function(theme) {
        var g = {};
        for (var key in theme) {
            g[key] = Color(theme[key]);
        }
        return g;
    };
    return Color;
});
