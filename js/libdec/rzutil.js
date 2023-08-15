// SPDX-FileCopyrightText: 2018-2021 Giovanni Dante Grazioli <deroad@libero.it>
// SPDX-License-Identifier: BSD-3-Clause

(function() { // lgtm [js/useless-expression]
    const rzpipe = require('libdec/rzpipe');
    const _JSON = require('libdec/json64');
    const Long = require('libdec/long');
    var __line_cnt = 0;

    function rz_sanitize(value, expected) {
        return value.length == 0 ? expected : value;
    }

    function jsdec_sanitize(enable, evar, oldstatus, newstatus) {
        if (enable) {
            rzcmd('e ' + evar + '=' + newstatus);
        } else {
            rzcmd('e ' + evar + '=' + oldstatus);
        }
    }

    function merge_arrays(input) {
        input = input.replace(/\n/g, ',');
        var array = '[' + input + ']';
        return array;
    }

    function merge_arrays_json(input) {
        return _JSON.parse(merge_arrays(input));
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
        var data = rz_func_graph();
        var farguments = rz_sanitize(rzpipe.string('afvj', true), '{"sp":[],"bp":[],"reg":[]}');
        var arch = rz_sanitize(rzpipe.string('e asm.arch'), '');
        var archbits = rz_sanitize(rzpipe.string('e asm.bits'), '32');
        var database = rz_sanitize(rzpipe.custom('afsj @@i', null, merge_arrays), '[]');
        console.log('{"name":"issue_' + (new Date()).getTime() +
            '","arch":"' + arch +
            '","archbits":' + archbits +
            ',"agj":' + data +
            ',"isj":' + xrefs +
            ',"Csj":' + strings +
            ',"icj":' + classes +
            ',"afvj":' + farguments +
            ',"afcfj":' + database +
            ',"aflj":' + functions + '}');
    }

    function print_archs() {
        var libdec = require('libdec/libdec');
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
        evarsTestSuite: function(data) {
            this.arch = data.arch;
            this.archbits = data.bits;
            this.honor = {
                assembly: true,
                blocks: false,
                casts: true,
                offsets: false,
                paddr: false,
                pseudo: false,
                xrefs: false,
            };
            this.extra = {
                allfuncs: false,
                ascodeline: false,
                ascomment: false,
                debug: true,
                file: 'testsuite',
                highlights: false,
                offset: Long.ZERO,
                slow: true,
                theme: 'default',
                annotation: false,
            };
        },
        dataTestSuite: function(x) {
            var o = _JSON.parse(x);
            if (!o.arch) {
                throw new Error('missing architecture in JSON.');
            }
            var bits = o.archbits;
            if (bits) {
                // if bits is in the issue then it has been decoded as a Long object.
                // to override this is required to be converted to just an integer.
                bits = bits.low;
            }
            return {
                arch: o.arch,
                bits: bits || 32,
                graph: o.agj || [],
                xrefs: {
                    symbols: o.isj || [],
                    strings: o.Csj || o.izj || [],
                    functions: o.aflj || [],
                    classes: o.icj || [],
                    arguments: o.afvj || {
                        "stack": [],
                        "reg": []
                    }
                },
                argdb: o.afcfj
            };
        },
        evars: function(args) {
            this.arch = rzpipe.string('e asm.arch');
            this.archbits = rzpipe.int('e asm.bits', 32);
            this.honor = {
                casts: rzpipe.bool('e jsdec.casts') || has_option(args, '--casts'),
                assembly: rzpipe.bool('e jsdec.asm') || has_option(args, '--assembly'),
                blocks: rzpipe.bool('e jsdec.blocks') || has_option(args, '--blocks'),
                xrefs: rzpipe.bool('e jsdec.xrefs') || has_option(args, '--xrefs'),
                paddr: rzpipe.bool('e jsdec.paddr') || has_option(args, '--paddr'),
                offsets: has_option(args, '--offsets'),
                color: rzpipe.int('e scr.color', 0) > 0 || has_option(args, '--colors')
            };
            this.sanitize = {
                ucase: rzpipe.bool('e asm.ucase'),
                pseudo: rzpipe.bool('e asm.pseudo'),
                capitalize: rzpipe.bool('e asm.capitalize'),
                html: rzpipe.bool('e scr.html'),
                syntax: rzpipe.string('e asm.syntax'),
            };
            this.extra = {
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
            this.add_comment = function(comment, offset) {
                if (!comment || comment.length < 1) {
                    return;
                }
                rzcmd('CC- @ 0x' + offset.toString(16));
                rzcmd('CCu base64:' + Duktape.enc('base64', comment) + ' @ 0x' + offset.toString(16));
            };
            this.add_code_line = function(comment, offset) {
                if (comment.trim().length < 1) {
                    return;
                }
                var line = __line_cnt++;
                rzcmd('"CL 0x' + offset.toString(16) + ' jsdec.c:' + line + ' ' + comment.replace(/\n/g, '; ').replace(/\\/g, '\\\\').replace(/"/g, '\\"') + ';"');
            };

            if (this.extra.ascomment || this.extra.ascodeline) {
                this.honor.assembly = false;
                this.honor.blocks = false;
                this.honor.offsets = false;
                this.extra.json = false;
                this.honor.color = false;
                this.extra.highlights = false;
                this.extra.annotation = false;
            }

            if (this.extra.allfunctions) {
                this.extra.ascomment = false;
                this.extra.ascodeline = false;
                this.honor.assembly = false;
                this.honor.blocks = false;
                this.honor.offsets = false;
                this.extra.json = false;
                this.extra.highlights = false;
                this.extra.annotation = false;
            }

            if (this.extra.annotation) {
                this.extra.ascodeline = false;
                this.extra.ascomment = false;
                this.extra.highlights = false;
                this.extra.html = false;
                this.extra.json = false;
                this.honor.assembly = false;
                this.honor.blocks = false;
                this.honor.color = false;
                this.honor.offsets = false;
            }

            if (this.sanitize.html || !this.honor.color) {
                this.extra.highlights = false;
            }
        },
        data: function(isfast) {
            if (isfast === undefined) {
                isfast = !rzpipe.bool('e jsdec.slow');
            }
            this.arch = rzpipe.string('e asm.arch');
            this.bits = rzpipe.int('e asm.bits', 32);
            this.xrefs = {
                symbols: (isfast ? [] : rzpipe.json64('isj', [])),
                strings: (isfast ? [] : rzpipe.json64('Cslj', [])),
                functions: (isfast ? [] : rzpipe.json64('aflj', [])),
                classes: rzpipe.json64('icj', []),
                arguments: rzpipe.json64('afvj', {
                    "stack": [],
                    "reg": []
                })
            };
            this.graph = _JSON.parse(rz_func_graph());
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
                if (exception.stack) {
                    return 'Exception: ' + exception.stack;
                }
                var msg = exception.name + ' ' + exception.message + ' line:' + exception.at;
                return 'Exception: ' + msg + '\n' + exception.text;
            } else {
                return '\n\njsdec has crashed (info: ' + rzpipe.string('i~^file[1:0]') + ' @ ' + rzpipe.string('s') + ').\n' +
                    'Please report the bug at https://github.com/rizinorg/jsdec/issues\n' +
                    'Use the option \'--issue\' or the command \'pddi\' to generate \n' +
                    'the needed data for the issue.';
            }
        }
    };
    return rzutil;
});
