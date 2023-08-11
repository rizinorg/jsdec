// SPDX-FileCopyrightText: 2017-2021 Giovanni Dante Grazioli <deroad@libero.it>
// SPDX-License-Identifier: BSD-3-Clause

var include = function(x) {
    return ___internal_load(x);
};
var require = function(x) {
    try {
        if (arguments.callee.stack[x]) {
            console.log("Circular dependency found.", x);
            return null;
        }
        if (arguments.callee.loaded[x]) {
            return arguments.callee.loaded[x];
        }
        arguments.callee.stack[x] = true;
        var src = ___internal_require(x);
        arguments.callee.loaded[x] = (src)();
        arguments.callee.stack[x] = false;
        return arguments.callee.loaded[x];
    } catch (ee) {
        console.log('Exception from ' + x);
        console.log(ee.stack);
    }
};
require.loaded = {};
require.stack = {};

/**
 * https://github.com/svaarala/duktape/blob/master/doc/error-objects.rst
 * 
 * this is required to be the first thing to be setup
 * when there is an exception i want to have the whole
 * stack to be printed.
 */
Duktape.errCreate = function(err) {
    try {
        if (typeof err === 'object') {
            var exception = {
                message: '' + err.message,
                stack: '' + err.stack,
                lineNumber: '' + err.lineNumber,
            };
            return exception;
        }
    } catch (e) {
        console.log('Duktape.errCreate exception.');
        console.log(e.stack);
    }
    return err;
};