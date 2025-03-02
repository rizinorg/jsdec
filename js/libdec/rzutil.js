// SPDX-FileCopyrightText: 2018-2023 Giovanni Dante Grazioli <deroad@libero.it>
// SPDX-License-Identifier: BSD-3-Clause

import rzpipe from './rzpipe.js';
import JSONex from './JSONex.js';
import libdec from './libdec.js';

export default (function() {
	var __line_cnt = 0;

	function rz_sanitize(value, expected) {
		return value.length == 0 ? expected : value;
	}

	function jsdec_sanitize(enable, evar, oldstatus, newstatus) {
		if (enable) {
			rizin.command('e ' + evar + '=' + newstatus);
		} else {
			rizin.command('e ' + evar + '=' + oldstatus);
		}
	}

	function merge_arrays(input) {
		input = input.replace(/\n/g, ',');
		var array = '[' + input + ']';
		return array;
	}

	function merge_arrays_json(input) {
		return JSONex.parse(merge_arrays(input));
	}

	var padding = '                   ';
	var usages = {
		"--help": "this help message",
		"--architectures": "lists the supported architectures",
		"--all-functions": "decompile all functions",
		"--assembly": "shows pseudo next to the assembly",
		"--blocks": "shows only scopes blocks",
		"--casts": "shows all casts in the pseudo code",
		"--colors": "enables syntax colors",
		"--debug": "do not catch exceptions",
		"--issue": "generates the json used for the test suite",
		"--offsets": "shows pseudo next to the assembly offset",
		"--paddr": "all xrefs uses physical addresses instead of virtual addresses",
		"--xrefs": "shows also instruction xrefs in the pseudo code",
		"--highlight-current": "highlights the current address.",
		"--as-comment": "the decompiled code is returned to r2 as comment (via CCu)",
		"--as-code-line": "the decompiled code is returned to r2 as 'file:line code' (via CL)",
		"--as-json": "the decompiled code lines are returned as JSON",
		"--annotation": "the decompiled code lines are returned with the annotation format"
	};

	function has_option(args, name) {
		return (args.indexOf(name) >= 0);
	}

	function has_invalid_args(args) {
		for (var i = 0; i < args.length; i++) {
			if (args[i] != '' && !usages[args[i]]) {
				console.log('Invalid argument \'' + args[i] + '\'\n');
				return true;
			}
		}
		return false;
	}

	function usage() {
		console.log("jsdec [options]");
		for (var key in usages) {
			var cmd = key + padding.substr(key.length, padding.length);
			console.log("       " + cmd + " | " + usages[key]);
		}
	}

	function print_issue() {
		var xrefs = rz_sanitize(rzpipe.string('isj'), '[]');
		var strings = rz_sanitize(rzpipe.string('Cslj'), '[]');
		var functions = rz_sanitize(rzpipe.string('aflj'), '[]');
		var classes = rz_sanitize(rzpipe.string('icj'), '[]');
		var data = JSONex.stringify(rizin.graph());
		var farguments = rz_sanitize(rzpipe.string('afvlj', true), '{"stack":[],"reg":[]}');
		var arch = rz_sanitize(rzpipe.string('e asm.arch'), '');
		var archbits = rz_sanitize(rzpipe.string('e asm.bits'), '32');
		var database = rz_sanitize(rzpipe.custom('afsj @@i', null, merge_arrays), '[]');
		console.log('{"name":"issue_' + (new Date()).getTime() +
			'","arch":"' + arch +
			'","archbits":' + archbits +
			',"graph":' + data +
			',"isj":' + xrefs +
			',"Csj":' + strings +
			',"icj":' + classes +
			',"afvj":' + farguments +
			',"afcfj":' + database +
			',"aflj":' + functions + '}');
	}

	function print_archs() {
		var archs = Object.keys(libdec.archs);
		archs.sort();
		console.log('Supported architectures: ' + archs.join(', '));
	}

	var rzutil = {
		check_args: function(args) {
			if (has_invalid_args(args)) {
				args.push('--help');
			}
			if (has_option(args, '--help')) {
				usage();
				return true;
			}
			if (has_option(args, '--issue')) {
				print_issue();
				return true;
			}
			if (has_option(args, '--architectures')) {
				print_archs();
				return true;
			}
			return false;
		},
		evars: function(args) {
			let o = {};
			o.arch = rzpipe.string('e asm.arch');
			o.archbits = rzpipe.int('e asm.bits', 32);
			o.honor = {
				casts: rzpipe.bool('e jsdec.casts') || has_option(args, '--casts'),
				assembly: rzpipe.bool('e jsdec.asm') || has_option(args, '--assembly'),
				blocks: rzpipe.bool('e jsdec.blocks') || has_option(args, '--blocks'),
				xrefs: rzpipe.bool('e jsdec.xrefs') || has_option(args, '--xrefs'),
				paddr: rzpipe.bool('e jsdec.paddr') || has_option(args, '--paddr'),
				offsets: has_option(args, '--offsets'),
				color: rzpipe.int('e scr.color', 0) > 0 || has_option(args, '--colors')
			};
			o.sanitize = {
				ucase: rzpipe.bool('e asm.ucase'),
				pseudo: rzpipe.bool('e asm.pseudo'),
				capitalize: rzpipe.bool('e asm.capitalize'),
				html: rzpipe.bool('e scr.html'),
				syntax: rzpipe.string('e asm.syntax'),
			};
			o.extra = {
				allfunctions: has_option(args, '--all-functions'),
				ascodeline: has_option(args, '--as-code-line'),
				ascomment: has_option(args, '--as-comment'),
				debug: rzpipe.bool('e jsdec.debug') || has_option(args, '--debug'),
				file: rzpipe.string('i~^file[1:0]'),
				highlights: rzpipe.bool('e jsdec.highlight') || has_option(args, '--highlight-current'),
				json: has_option(args, '--as-json'),
				offset: rzpipe.long('s'),
				slow: rzpipe.bool('e jsdec.slow'),
				annotation: has_option(args, '--annotation'),
			};
			o.add_comment = function(comment, offset) {
				if (!comment || comment.length < 1) {
					return;
				}
				rizin.command('CC- @ 0x' + offset.toString(16));
				rizin.command('CCu base64:' + btoa(comment) + ' @ 0x' + offset.toString(16));
			};
			o.add_code_line = function(comment, offset) {
				if (comment.trim().length < 1) {
					return;
				}
				var line = __line_cnt++;
				rizin.command('"CL 0x' + offset.toString(16) + ' jsdec.c:' + line + ' ' + comment.replace(/\n/g, '; ').replace(/\\/g, '\\\\').replace(/"/g, '\\"') + ';"');
			};

			if (o.extra.ascomment || o.extra.ascodeline) {
				o.honor.assembly = false;
				o.honor.blocks = false;
				o.honor.offsets = false;
				o.extra.json = false;
				o.honor.color = false;
				o.extra.highlights = false;
				o.extra.annotation = false;
			}

			if (o.extra.allfunctions) {
				o.extra.ascomment = false;
				o.extra.ascodeline = false;
				o.honor.assembly = false;
				o.honor.blocks = false;
				o.honor.offsets = false;
				o.extra.json = false;
				o.extra.highlights = false;
				o.extra.annotation = false;
			}

			if (o.extra.annotation) {
				o.extra.ascodeline = false;
				o.extra.ascomment = false;
				o.extra.highlights = false;
				o.extra.html = false;
				o.extra.json = false;
				o.honor.assembly = false;
				o.honor.blocks = false;
				o.honor.color = false;
				o.honor.offsets = false;
			}

			if (o.sanitize.html || !o.honor.color) {
				o.extra.highlights = false;
			}
			return o;
		},
		data: function(isfast) {
			if (isfast === undefined) {
				isfast = !rzpipe.bool('e jsdec.slow');
			}
			this.arch = rzpipe.string('e asm.arch');
			this.bits = rzpipe.int('e asm.bits', 32);
			this.xrefs = {
				symbols: (isfast ? [] : rzpipe.json('isj', [])),
				strings: (isfast ? [] : rzpipe.json('Cslj', [])),
				functions: (isfast ? [] : rzpipe.json('aflj', [])),
				classes: rzpipe.json('icj', []),
				arguments: rzpipe.json('afvlj', {
					"stack": [],
					"reg": []
				})
			};
			this.xrefs.arguments.stack = this.xrefs.arguments.stack || [];
			this.xrefs.arguments.reg = this.xrefs.arguments.reg || [];
			this.graph = rizin.graph();
			this.argdb = rzpipe.custom('afsj @@i', null, merge_arrays_json);
		},
		sanitize: function(enable, evars) {
			if (!evars) {
				return;
			}
			var s = evars.sanitize;
			jsdec_sanitize(enable, 'asm.ucase', s.ucase, 'false');
			jsdec_sanitize(enable, 'asm.pseudo', s.pseudo, 'false');
			jsdec_sanitize(enable, 'asm.capitalize', s.capitalize, 'false');
			jsdec_sanitize(enable, 'scr.html', s.html, 'false');
			if (evars.arch == 'x86') {
				jsdec_sanitize(enable, 'asm.syntax', s.syntax, 'intel');
			}
		},
		debug: function(evars, exception) {
			rzutil.sanitize(false, evars);
			if (!evars || evars.extra.debug) {
				var msg = exception.message + ' (' + exception.name + ')';
				return 'Exception: ' + msg + '\n' + exception.stack;
			} else {
				return '\n\njsdec has crashed (info: ' + rzpipe.string('i~^file[1:0]') + ' @ ' + rzpipe.string('s') + ').\n' +
					'Please report the bug at https://github.com/rizinorg/jsdec/issues\n' +
					'Use the option \'--issue\' or the command \'pddi\' to generate \n' +
					'the needed data for the issue.';
			}
		}
	};
	return rzutil;
})();
