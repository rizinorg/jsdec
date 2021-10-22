// SPDX-FileCopyrightText: 2020-2021 Giovanni Dante Grazioli <deroad@libero.it>
// SPDX-License-Identifier: BSD-3-Clause

(function() { // lgtm [js/useless-expression]
    const Long = require('libdec/long');
    const rzutil = require('libdec/rzutil');
    const Strings = require('libdec/core/strings');
    const Functions = require('libdec/core/functions');
    const Extra = require('libdec/core/extra');

    const _re_keywords = /\bif\b|\belse\b|\bwhile\b|\bfor\b|\bdo\b|\breturn\b|\bthrow\b|\bnew\b|\b__asm\b/g;
    const _re_types = /\b[ui]+nt[123468]+_t\b|\bvoid\b|\bconst\b|\bsizeof\b|\bfloat\b|\bdouble\b|\bchar\b|\bwchar_t\b|\bextern\b|\bstruct\b|\bsize_t\b|\btime_t\b|\bboolean\b/g;
    const _re_numbers = /\b0x[0-9a-fA-F]+\b|\b\d+\b/g;
    const _re_string = /"[^"]+"/;
    const _re_objcall = /[\w.]+\s?\([^)]+\)|[\w.]+\s?\(\)/;

    function Annotation(start, length, value, pointer, type, syntax_type) {
        this.start = start;
        this.end = start + length;
        this.type = type;
        if (["function_name", "function_parameter", "local_variable"].indexOf(this.type) >= 0) {
            this.name = value;
        }
        if (["function_name", "offset", "global_variable", "constant_variable"].indexOf(this.type) >= 0) {
            this.offset = pointer.toString();
        }
        if (this.type == "syntax_highlight") {
            this.syntax_highlight = syntax_type;
        }
    }

    function _sort_by_start(x, y) {
        if (x.start == y.start) {
            if (x.type == "offset") {
                return -1;
            } else if (x.type == "syntax_highlight") {
                return 0;
            }
            return 1;
        }
        return x.start - y.start;
    }

    function _match_annotation(regex, line, callback) {
        var tokens, matches;
        matches = line.match(regex);
        tokens = line.split(regex);
        if (!matches) {
            return;
        }
        for (var i = 0, pos = 0; i < matches.length; i++) {
            pos += tokens[i].length;
            callback(pos, matches[i]);
            pos += matches[i].length;
        }
    }

    return {
        context: function() {
            var data = new rzutil.data(false);
            return {
                comment: false,
                arch: data.arch,
                strings: new Strings(data.xrefs.strings, true).data,
                functions: new Functions(data.xrefs.functions, true).data.map(function(x) {
                    return {
                        location: x.offset,
                        value: Extra.replace.call(x.name),
                        name: x.name
                    };
                }),
                variables: data.arch == "dalvik" ? [] : Array.prototype.concat(data.xrefs.arguments.bp, data.xrefs.arguments.sp, data.xrefs.arguments.reg)
            };
        },
        parse: function(current, line, context) {
            var pos, old, i, a = [],
                it, tmp, ll = line.str.length;
            var string_beg = ll,
                string_end = -1;
            a.push(new Annotation(current, ll, null, line.offset, "offset"));
            if (line.str.trim().startsWith('/*') || context.comment) {
                context.comment = !line.str.endsWith('*/');
                a.push(new Annotation(current, ll, line.str, null, "syntax_highlight", "comment"));
                return a;
            } else if (line.str.startsWith('//')) {
                a.push(new Annotation(current, ll, line.str, null, "syntax_highlight", "comment"));
                return a;
            } else if (line.str.startsWith('#')) {
                a.push(new Annotation(current, ll, line.str, null, "syntax_highlight", "comment"));
                return a;
            }
            /* strings */
            for (i = context.strings.length - 1; i >= 0; i--) {
                it = context.strings[i];
                var str = '"' + it.value + '"';
                pos = line.str.indexOf(str);
                if (pos > 0) {
                    // include the ""
                    a.push(new Annotation(current + pos, str.length, str, it.location, "global_variable", null));
                    a.push(new Annotation(current + pos, str.length, str, null, "syntax_highlight", "global_variable"));
                    string_beg = pos;
                    string_end = string_beg + str.length;
                    break;
                }
            }
            /* strings with regex */
            if (string_end < 0) {
                tmp = line.str.match(_re_string) || [];
                for (i = tmp.length - 1, old = 0; i >= 0; i--) {
                    pos = line.str.indexOf(tmp[i]);
                    a.push(new Annotation(current + pos, tmp[i].length, tmp[i], null, "syntax_highlight", "global_variable"));
                    string_beg = pos;
                    string_end = pos + tmp[i].length;
                    break;
                }
            }
            /* function names */
            if (context.arch == "dalvik") {
                pos = line.str.search(_re_objcall);
                if (pos >= 0 && !(pos >= string_beg && pos <= string_end)) {
                    var callname = line.str.substr(pos, line.str.substr(pos).indexOf('(')).trim();
                    if (['__asm'].indexOf(callname) < 0) {
                        a.push(new Annotation(current + pos, callname.length, callname, null, "syntax_highlight", "function_name"));
                    }
                }
            } else {
                for (i = context.functions.length - 1; i >= 0; i--) {
                    it = context.functions[i];
                    pos = line.str.search(new RegExp('\\b' + it.value + '\\b'));
                    if (pos >= 0 && !(pos >= string_beg && pos <= string_end)) {
                        a.push(new Annotation(current + pos, it.value.length, it.name, it.location, "function_name", null));
                        a.push(new Annotation(current + pos, it.value.length, it.value, null, "syntax_highlight", "function_name"));
                        break;
                    }
                }
            }
            /* variables */
            for (i = context.variables.length - 1, old = 0; i >= 0; i--) {
                it = context.variables[i];
                pos = it.type.match(_re_types) ? -1 : line.str.search(new RegExp('\\b' + it.type + '\\b'));
                if (pos >= 0 && !(pos >= string_beg && pos <= string_end)) {
                    a.push(new Annotation(current + pos, it.type.length, it.type, null, "syntax_highlight", "datatype"));
                }
                pos = line.str.search(new RegExp('\\b' + it.name + '\\b'));
                if (pos >= 0 && !(pos >= string_beg && pos <= string_end)) {
                    var type = (it.kind === 'arg' || it.kind === 'reg') ? "function_parameter" : "local_variable";
                    a.push(new Annotation(current + pos, it.name.length, it.name, line.offset, type, null));
                    a.push(new Annotation(current + pos, it.name.length, it.name, null, "syntax_highlight", type));
                }
            }
            /* keywords */
            _match_annotation(_re_keywords, line.str, function(pos, match) {
                if (pos >= 0 && !(pos >= string_beg && pos <= string_end)) {
                    a.push(new Annotation(current + pos, match.length, match, null, "syntax_highlight", "keyword"));
                }
            });
            /* numbers */
            _match_annotation(_re_numbers, line.str, function(pos, match) {
                if (match.length > 1 && match.charAt(0) == '0' && match.charAt(1) != 'x') {
                    return;
                }
                if (match != '0') {
                    pos = old + line.str.substr(old, ll).indexOf(match);
                } else {
                    pos = old + line.str.substr(old, ll).search(/\b0\b/);
                }
                if (pos >= 0 && !(pos >= string_beg && pos <= string_end)) {
                    var location = Long.fromString(match);
                    a.push(new Annotation(current + pos, match.length, match, location, "constant_variable", null));
                    a.push(new Annotation(current + pos, match.length, match, null, "syntax_highlight", "constant_variable"));
                }
            });
            /* types */
            _match_annotation(_re_types, line.str, function(pos, match) {
                if (pos >= 0 && !(pos >= string_beg && pos <= string_end)) {
                    a.push(new Annotation(current + pos, match.length, match, null, "syntax_highlight", "datatype"));
                }
            });
            return a.sort(_sort_by_start);
        }
    };
});