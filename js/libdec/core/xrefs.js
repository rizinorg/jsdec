// SPDX-FileCopyrightText: 2019-2021 Giovanni Dante Grazioli <deroad@libero.it>
// SPDX-License-Identifier: BSD-3-Clause

(function() { // lgtm [js/useless-expression]
    var XReference = function(strings, symbols, classes) {
        this.db_strings = strings;
        this.db_symbols = symbols;
        this.db_classes = classes;
        this.find_symbol = function(address) {
            return this.db_symbols.search(address);
        };
        this.find_string = function(value) {
            if (typeof value == 'string') {
                return this.db_strings.search_by_flag(value);
            }
            return this.db_strings.search(value);
        };
        this.find_class = function(address) {
            return this.db_classes.search(address);
        };
    };
    return XReference;
});