// SPDX-FileCopyrightText: 2017-2021 Giovanni Dante Grazioli <deroad@libero.it>
// SPDX-License-Identifier: BSD-3-Clause

/**
 * Global data accessible from everywhere.
 * @type {Object}
 */
var Global = {
    context: null,
    evars: null,
    printer: null
};

/**
 * Imports.
 */
var libdec = require('libdec/libdec');
var rzutil = require('libdec/rzutil');

const rzcmd = null;

/**
 * jsdec main function.
 * @param  {String} filename - Issue filename to analyze (relative/fullpath)
 */
function jsdec_main(filename) { // lgtm [js/unused-local-variable]
    try {
        // imports
        var Printer = require('libdec/printer');
        if (filename && !rzcmd) {
            var jsonstr = read_file(filename).trim();
            var data = rzutil.dataTestSuite(jsonstr);
            Global.evars = new rzutil.evarsTestSuite(data);
            Global.argdb = data.argdb;
            Global.printer = new Printer();

            var architecture = libdec.archs[data.arch];
            Global.context = new libdec.context();
            // af seems to break renaming.
            /* asm.pseudo breaks things.. */
            if (data.graph && data.graph.length > 0) {
                var p = new libdec.core.session(data, architecture);
                var arch_context = architecture.context(data);
                libdec.core.analysis.pre(p, architecture, arch_context);
                libdec.core.decompile(p, architecture, arch_context);
                libdec.core.analysis.post(p, architecture, arch_context);
                libdec.core.print(p);
                Global.printer.flushOutput(Global.context.lines, Global.context.errors, Global.context.log, Global.evars.extra);
            } else {
                console.log('Error: no data available.\nPlease analyze the function/binary first.');
            }
        } else {
            console.log('missing JSON to test.');
        }
    } catch (e) {
        console.log('Exception:', e.stack);
    }
}