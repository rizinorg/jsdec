// SPDX-FileCopyrightText: 2024 Giovanni Dante Grazioli <deroad@libero.it>
// SPDX-License-Identifier: BSD-3-Clause

#include <stdlib.h>
#include <string.h>
#include <rz_types.h>
#include <rz_lib.h>
#include <rz_cmd.h>
#include <rz_core.h>
#include <rz_cons.h>
#include <rz_analysis.h>

#include "jsdec.h"
#include "jsdec-cutter.h"

typedef struct rz_core_t RzCore;

typedef struct exec_context_t {
	RzCore *core;
	void *bed;
	RzStrBuf *anno;
	JSValue shared;
} ExecContext;

static JSValue js_analysis_bytes(JSContext *ctx, RzCore *core, RzAnalysisBytes *ab) {
	RzAnalysisOp *aop = ab->op;
	JSValue op = JS_NewObject(ctx);
	JS_SetPropertyStr(ctx, op, "offset", JS_NewBigUint64(ctx, aop->addr));
	if (aop->ptr != UT64_MAX) {
		JS_SetPropertyStr(ctx, op, "ptr", JS_NewBigUint64(ctx, aop->ptr));
	}
	if (aop->val != UT64_MAX) {
		JS_SetPropertyStr(ctx, op, "val", JS_NewBigUint64(ctx, aop->val));
	}
	JS_SetPropertyStr(ctx, op, "opcode", JS_NewString(ctx, rz_str_get_null(ab->opcode)));
	JS_SetPropertyStr(ctx, op, "disasm", JS_NewString(ctx, rz_str_get_null(ab->disasm)));
	JS_SetPropertyStr(ctx, op, "type", JS_NewString(ctx, rz_analysis_optype_to_string(aop->type)));
	if (aop->jump != UT64_MAX) {
		JS_SetPropertyStr(ctx, op, "jump", JS_NewBigInt64(ctx, aop->jump));
	}
	if (aop->fail != UT64_MAX) {
		JS_SetPropertyStr(ctx, op, "fail", JS_NewBigUint64(ctx, aop->fail));
	}
	const char *comment = rz_meta_get_string(core->analysis, RZ_META_TYPE_COMMENT, aop->addr);
	if (RZ_STR_ISNOTEMPTY(comment)) {
		JS_SetPropertyStr(ctx, op, "comment", JS_NewString(ctx, comment));
	}
	return op;
}

static JSValue js_analysis_opcodes(JSContext *ctx, RzCore *core) {
	RzAnalysisBytes *ab;
	JSValue ops = JS_NewArray(ctx);
	st64 op_idx = 0;

	RzIterator *iter = rz_core_analysis_bytes(core, core->offset, core->block, core->blocksize, 0);
	if (!iter) {
		return ops;
	}
	rz_iterator_foreach(iter, ab) {
		if (!ab || !ab->op || !strcmp(ab->opcode, "nop")) {
			continue;
		}
		JSValue op = js_analysis_bytes(ctx, core, ab);
		JS_SetPropertyInt64(ctx, ops, op_idx, op);
		op_idx++;
	}
	rz_iterator_free(iter);
	return ops;
}

static JSValue js_function_bbs(JSContext *ctx, RzCore *core, RzAnalysisFunction *fcn) {
	JSValue bbs = JS_NewArray(ctx);
	RzAnalysisBlock *bbi;
	void **iter;
	st64 bbs_idx = 0;
	ut64 old_offset = core->offset;
	ut64 old_bsize = core->blocksize;
	rz_pvector_foreach (fcn->bbs, iter) {
		bbi = (RzAnalysisBlock *)*iter;
		rz_core_block_size(core, bbi->size);
		rz_core_seek(core, bbi->addr, true);

		JSValue bb = JS_NewObject(ctx);
		JS_SetPropertyStr(ctx, bb, "address", JS_NewBigUint64(ctx, bbi->addr));
		if (bbi->jump != UT64_MAX) {
			JS_SetPropertyStr(ctx, bb, "jump", JS_NewBigUint64(ctx, bbi->jump));
		}
		if (bbi->fail != UT64_MAX) {
			JS_SetPropertyStr(ctx, bb, "fail", JS_NewBigUint64(ctx, bbi->fail));
		}
		JSValue ops = js_analysis_opcodes(ctx, core);
		JS_SetPropertyStr(ctx, bb, "ops", ops);
		JS_SetPropertyInt64(ctx, bbs, bbs_idx, bb);
		bbs_idx++;
	}
	rz_core_block_size(core, old_bsize);
	rz_core_seek(core, old_offset, true);
	return bbs;
}

static JSValue js_command(JSContext *ctx, JSValueConst jsThis, int argc, JSValueConst *argv) {
	if (argc != 1) {
		return JS_EXCEPTION;
	}

	const char *command = JS_ToCString(ctx, argv[0]);
	if (!command) {
		return JS_EXCEPTION;
	}

	ExecContext *ectx = (ExecContext *)JS_GetContextOpaque(ctx);
	RzCore *core = ectx->core;
	rz_cons_sleep_end(ectx->bed);

	char *output = rz_core_cmd_str(core, command);
	JS_FreeCString(ctx, command);
	JSValue result = JS_NewString(ctx, output ? output : "");
	free(output);

	ectx->bed = rz_cons_sleep_begin();
	return result;
}

static JSValue js_graph(JSContext *ctx, JSValueConst jsThis, int argc, JSValueConst *argv) {
	if (argc != 0) {
		return JS_EXCEPTION;
	}
	ExecContext *ectx = (ExecContext *)JS_GetContextOpaque(ctx);
	RzCore *core = ectx->core;
	rz_cons_sleep_end(ectx->bed);

	RzAnalysisFunction *fcn = rz_analysis_get_fcn_in(core->analysis, core->offset, -1);
	if (!fcn) {
		ectx->bed = rz_cons_sleep_begin();
		return JS_ThrowInternalError(ctx, "Cannot find function at 0x%08" PFMT64x, core->offset);
	}

	JSValue graph = JS_NewArray(ctx);
	JSValue object = JS_NewObject(ctx);
	JS_SetPropertyInt64(ctx, graph, 0, object);
	JS_SetPropertyStr(ctx, object, "name", JS_NewString(ctx, rz_str_get_null(fcn->name)));
	JSValue blocks = js_function_bbs(ctx, core, fcn);
	JS_SetPropertyStr(ctx, object, "blocks", blocks);
	ectx->bed = rz_cons_sleep_begin();
	return graph;
}

static JSValue js_console_log(JSContext *ctx, JSValueConst jsThis, int argc, JSValueConst *argv) {
	ExecContext *ectx = (ExecContext *)JS_GetContextOpaque(ctx);
	for (int i = 0; i < argc; ++i) {
		if (i != 0) {
			rz_strbuf_append_n(ectx->anno, " ", 1);
		}
		const char *str = JS_ToCString(ctx, argv[i]);
		if (!str) {
			return JS_EXCEPTION;
		}
		rz_strbuf_append(ectx->anno, str);
		JS_FreeCString(ctx, str);
	}
	rz_strbuf_append_n(ectx->anno, "\n", 1);
	return JS_UNDEFINED;
}

static JSValue js_get_global(JSContext *ctx, JSValueConst jsThis, int argc, JSValueConst *argv) {
	ExecContext *ectx = (ExecContext *)JS_GetContextOpaque(ctx);
	return JS_GetPropertyStr(ctx, ectx->shared, "Shared");
}

static jsdec_t *jsdec_create(ExecContext *ec) {
	jsdec_t *dec = jsdec_new();
	if (!dec) {
		return NULL;
	}

	JSContext *ctx = jsdec_context(dec);
	JS_SetContextOpaque(ctx, ec);
	ec->shared = JS_NewObject(ctx);
	JS_SetPropertyStr(ctx, ec->shared, "Shared", JS_NewObject(ctx));

	JSValue global = JS_GetGlobalObject(ctx);
	JS_SetPropertyStr(ctx, global, "Global", JS_NewCFunction(ctx, js_get_global, "Global", 1));

	JSValue console = JS_NewObject(ctx);
	JS_SetPropertyStr(ctx, global, "console", console);
	JS_SetPropertyStr(ctx, console, "log", JS_NewCFunction(ctx, js_console_log, "log", 1));

	JSValue rizin = JS_NewObject(ctx);
	JS_SetPropertyStr(ctx, global, "rizin", rizin);
	JS_SetPropertyStr(ctx, rizin, "command", JS_NewCFunction(ctx, js_command, "command", 1));
	JS_SetPropertyStr(ctx, rizin, "graph", JS_NewCFunction(ctx, js_graph, "graph", 1));

	JSValue process = JS_NewObject(ctx);
	JS_SetPropertyStr(ctx, global, "process", process);
	JSValue args = JS_NewArray(ctx);
	JS_SetPropertyInt64(ctx, args, 0, JS_NewString(ctx, "--annotation"));
	JS_SetPropertyStr(ctx, process, "args", args);

	JS_FreeValue(ctx, global);
	return dec;
}

static void jsdec_destroy(jsdec_t *dec, ExecContext *ec) {
	JSContext *ctx = jsdec_context(dec);
	JS_FreeValue(ctx, ec->shared);
	jsdec_free(dec);
}

static char *json_to_strdup(const RzJson *object, const char *key) {
	const RzJson *j = rz_json_get(object, key);
	if (!j || j->type != RZ_JSON_STRING) {
		return NULL;
	}
	return rz_str_dup(j->str_value);
}

static const char *json_as_string(const RzJson *object, const char *key) {
	const RzJson *j = rz_json_get(object, key);
	if (!j || j->type != RZ_JSON_STRING) {
		return NULL;
	}
	return j->str_value;
}

static ut64 json_as_ut64(const RzJson *object, const char *key) {
	const RzJson *j = rz_json_get(object, key);
	if (!j || (j->type != RZ_JSON_STRING && j->type != RZ_JSON_INTEGER)) {
		return 0;
	}
	if (j->type == RZ_JSON_INTEGER) {
		return j->num.u_value;
	} else if (rz_num_is_valid_input(NULL, j->str_value)) {
		return rz_num_get_input_value(NULL, j->str_value);
	}
	return 0;
}

static RzAnnotatedCode *parse_json(RzStrBuf *sb) {
	char *text = rz_strbuf_drain(sb);
	if (RZ_STR_ISEMPTY(text)) {
		free(text);
		return NULL;
	}

	// text is modified by rz_json_parse and
	// is freed by rz_json_free
	RzJson *json = rz_json_parse(text);
	if (!json) {
		RZ_LOG_ERROR("failed to parse from json string\n");
		return NULL;
	}

	char *raw_code = json_to_strdup(json, "code");
	if (!raw_code) {
		rz_json_free(json);
		RZ_LOG_ERROR("failed to dup code\n");
		return NULL;
	}

	RzAnnotatedCode *code = rz_annotated_code_new(raw_code);
	if (!code) {
		rz_json_free(json);
		RZ_LOG_ERROR("failed to create RzAnnotatedCode\n");
		return NULL;
	}

	const RzJson *annotations = rz_json_get(json, "annotations");
	const RzJson *element = NULL;
	for (size_t idx = 0; annotations && (element = rz_json_item(annotations, idx)); idx++) {
		RzCodeAnnotation annotation = { 0 };
		annotation.start = json_as_ut64(element, "start");
		annotation.end = json_as_ut64(element, "end");
		const char *type = json_as_string(element, "type");
		if (!strcmp(type, "offset")) {
			annotation.type = RZ_CODE_ANNOTATION_TYPE_OFFSET;
			annotation.offset.offset = json_as_ut64(element, "offset");
		} else if (!strcmp(type, "function_name")) {
			annotation.type = RZ_CODE_ANNOTATION_TYPE_FUNCTION_NAME;
			annotation.reference.name = json_to_strdup(element, "name");
			annotation.reference.offset = json_as_ut64(element, "offset");
		} else if (!strcmp(type, "global_variable")) {
			annotation.type = RZ_CODE_ANNOTATION_TYPE_GLOBAL_VARIABLE;
			annotation.reference.offset = json_as_ut64(element, "offset");
		} else if (!strcmp(type, "constant_variable")) {
			annotation.type = RZ_CODE_ANNOTATION_TYPE_CONSTANT_VARIABLE;
			annotation.reference.offset = json_as_ut64(element, "offset");
		} else if (!strcmp(type, "local_variable")) {
			annotation.type = RZ_CODE_ANNOTATION_TYPE_LOCAL_VARIABLE;
			annotation.variable.name = json_to_strdup(element, "name");
		} else if (!strcmp(type, "function_parameter")) {
			annotation.type = RZ_CODE_ANNOTATION_TYPE_FUNCTION_PARAMETER;
			annotation.variable.name = json_to_strdup(element, "name");
		} else if (!strcmp(type, "syntax_highlight")) {
			annotation.type = RZ_CODE_ANNOTATION_TYPE_SYNTAX_HIGHLIGHT;
			const char *highlightType = json_as_string(element, "syntax_highlight");
			if (!strcmp(highlightType, "keyword")) {
				annotation.syntax_highlight.type = RZ_SYNTAX_HIGHLIGHT_TYPE_KEYWORD;
			} else if (!strcmp(highlightType, "comment")) {
				annotation.syntax_highlight.type = RZ_SYNTAX_HIGHLIGHT_TYPE_COMMENT;
			} else if (!strcmp(highlightType, "datatype")) {
				annotation.syntax_highlight.type = RZ_SYNTAX_HIGHLIGHT_TYPE_DATATYPE;
			} else if (!strcmp(highlightType, "function_name")) {
				annotation.syntax_highlight.type = RZ_SYNTAX_HIGHLIGHT_TYPE_FUNCTION_NAME;
			} else if (!strcmp(highlightType, "function_parameter")) {
				annotation.syntax_highlight.type = RZ_SYNTAX_HIGHLIGHT_TYPE_FUNCTION_PARAMETER;
			} else if (!strcmp(highlightType, "local_variable")) {
				annotation.syntax_highlight.type = RZ_SYNTAX_HIGHLIGHT_TYPE_LOCAL_VARIABLE;
			} else if (!strcmp(highlightType, "constant_variable")) {
				annotation.syntax_highlight.type = RZ_SYNTAX_HIGHLIGHT_TYPE_CONSTANT_VARIABLE;
			} else if (!strcmp(highlightType, "global_variable")) {
				annotation.syntax_highlight.type = RZ_SYNTAX_HIGHLIGHT_TYPE_GLOBAL_VARIABLE;
			}
		}
		rz_annotated_code_add_annotation(code, &annotation);
	}

	rz_json_free(json);
	return code;
}

RZ_API void jsdec_init_config(RzCore *core) {
	RzConfig *cfg = core->config;

	rz_config_lock(cfg, false);
	SETPREF("jsdec.asm", "false", "if true, shows pseudo next to the assembly.");
	SETPREF("jsdec.blocks", "false", "if true, shows only scopes blocks.");
	SETPREF("jsdec.casts", "false", "if false, hides all casts in the pseudo code.");
	SETPREF("jsdec.debug", "false", "do not catch exceptions in jsdec.");
	SETPREF("jsdec.highlight", "default", "highlights the current address.");
	SETPREF("jsdec.paddr", "false", "if true, all xrefs uses physical addresses compare.");
	SETPREF("jsdec.slow", "false", "load all the data before to avoid multirequests to rizin.");
	SETPREF("jsdec.xrefs", "false", "if true, shows all xrefs in the pseudo code.");
	rz_config_lock(cfg, true);
}

RZ_API RzAnnotatedCode *jsdec_as_annotation(RzCore *core, ut64 addr) {
	ExecContext ectx;
	ectx.core = core;
	ectx.anno = rz_strbuf_new("");
	if (!ectx.anno) {
		rz_strbuf_free(ectx.anno);
		RZ_LOG_ERROR("failed to create RzStrBuf\n");
		return NULL;
	}

	jsdec_t *dec = jsdec_create(&ectx);
	if (!dec) {
		rz_strbuf_free(ectx.anno);
		RZ_LOG_ERROR("failed to create jsdec_t\n");
		return NULL;
	}

	ut64 offset = core->offset;
	if (offset != addr) {
		rz_core_seek(core, addr, true);
	}

	ectx.bed = rz_cons_sleep_begin();
	bool ret = jsdec_run(dec);
	rz_cons_sleep_end(ectx.bed);

	if (offset != addr) {
		rz_core_seek(core, offset, true);
	}

	jsdec_destroy(dec, &ectx);
	if (!ret) {
		rz_strbuf_free(ectx.anno);
		RZ_LOG_ERROR("jsdec_run returned false\n");
		return NULL;
	}

	return parse_json(ectx.anno);
}
