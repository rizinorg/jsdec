// SPDX-FileCopyrightText: 2017-2023 Giovanni Dante Grazioli <deroad@libero.it>
// SPDX-FileCopyrightText: 2017-2018 pancake <pancake@nopcode.org>
// SPDX-License-Identifier: BSD-3-Clause


import libdec from './libdec/libdec.js';
import rzutil from './libdec/rzutil.js';
import rzpipe from './libdec/rzpipe.js';
import Warning from './libdec/warning.js';
import Printer from './libdec/printer.js';

/**
 * Shared data accessible from everywhere.
 * @type {Object}
 */
var Shared = Global();
Shared.evars = null;
Shared.context = null;
Shared.printer = null;
Shared.argdb = null;
Shared.xrefs = null;
Shared.warning = Warning;

function decompile_offset(architecture, fcnname) {
	var data = new rzutil.data();
	Shared.argdb = data.argdb;
	// af seems to break renaming.
	/* asm.pseudo breaks things.. */
	if (data.graph && data.graph.length > 0) {
		var p = new libdec.core.session(data, architecture);
		var arch_context = architecture.context(data);
		libdec.core.analysis.pre(p, architecture, arch_context);
		libdec.core.decompile(p, architecture, arch_context);
		libdec.core.analysis.post(p, architecture, arch_context);
		libdec.core.print(p);
	} else if (Shared.evars.extra.allfunctions) {
		Shared.context.printLog('Error: Please analyze the ' + fcnname + ' first.', true);
	} else {
		Shared.context.printLog('Error: no data available.\nPlease analyze the function/binary first.', true);
	}
}

function main() {
	var lines = null;
	var errors = [];
	var log = [];
	try {
		Shared.evars = rzutil.evars(process.args);
		rzutil.sanitize(true, Shared.evars);
		if (rzutil.check_args(process.args)) {
			rzutil.sanitize(false, Shared.evars);
			return;
		}

		// theme (requires to be initialized after evars)
		Shared.printer = new Printer();

		var architecture = libdec.archs[Shared.evars.arch];

		if (architecture) {
			Shared.context = new libdec.context();
			var current = rzpipe.long('s');
			if (Shared.evars.extra.allfunctions) {
				var functions = rzpipe.json('aflj');
				functions.forEach(function(x) {
					if (x.name.startsWith('sym.imp.') || x.name.startsWith('loc.imp.')) {
						return;
					}
					rzpipe.string('s 0x' + x.offset.toString(16));
					Shared.context.printLine("", x.offset);
					Shared.context.printLine(Shared.printer.theme.comment('/* name: ' + x.name + ' @ 0x' + x.offset.toString(16) + ' */'), x.offset);
					decompile_offset(architecture, x.name);
				});
				rzpipe.string('s 0x' + current.toString(16));
				var o = Shared.context;
				Shared.context = new libdec.context();
				Shared.context.macros = o.macros;
				Shared.context.dependencies = o.dependencies;
				Shared.context.printLine(Shared.printer.theme.comment('/* jsdec pseudo code output */'), current);
				Shared.context.printLine(Shared.printer.theme.comment('/* ' + Shared.evars.extra.file + ' */'), current);
				if (['java', 'dalvik'].indexOf(Shared.evars.arch) < 0) {
					Shared.context.printMacros(true);
					Shared.context.printDependencies(true);
				}
				o.lines = Shared.context.lines.concat(o.lines);
				Shared.context = o;
			} else {
				decompile_offset(architecture, null);
			}
			errors = errors.concat(Shared.context.errors);
			log = log.concat(Shared.context.log);
			lines = Shared.context.lines;
		} else {
			errors.push(Shared.evars.arch + ' is not currently supported.\n' +
				'Please open an enhancement issue at https://github.com/rizinorg/jsdec/issues\n' +
				libdec.supported());
		}
		rzutil.sanitize(false, Shared.evars);
	} catch (e) {
		errors.push(rzutil.debug(Shared.evars, e));
	}

	if (!Shared.printer) {
		Shared.printer = new Printer();
	}
	Shared.printer.flushOutput(lines, errors, log);
}

main();
