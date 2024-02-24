// SPDX-FileCopyrightText: 2018-2023 Giovanni Dante Grazioli <deroad@libero.it>
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

typedef struct rz_core_t RzCore;

typedef struct exec_context_t {
	RzCore *core;
	void *bed;
	JSValue shared;
} ExecContext;

#undef RZ_API
#define RZ_API static
#undef RZ_IPI
#define RZ_IPI           static
#define SETDESC(x, y)    rz_config_node_desc(x, y)
#define SETPREF(x, y, z) SETDESC(rz_config_set(cfg, x, y), z)

#define name_args(name)    (internal_##name##_args)
#define name_help(name)    (internal_##name##_help)
#define name_handler(name) (internal_##name##_handler)
#define command_handler(name, arg) \
	RZ_IPI RzCmdStatus name_handler(name)(RzCore * core, int argc, const char **argv) { \
		if (argc != 1) { \
			return RZ_CMD_STATUS_WRONG_ARGS; \
		} \
		jsdec_main(core, arg); \
		return RZ_CMD_STATUS_OK; \
	}
#define static_description_no_args(command, description) \
	static const RzCmdDescArg name_args(command)[] = { \
		{ 0 }, \
	}; \
	static const RzCmdDescHelp name_help(command) = { \
		.summary = description, \
		.args = name_args(command), \
	}
#define rz_cmd_desc_argv_new_warn(rcmd, root, cmd) \
	rz_warn_if_fail(rz_cmd_desc_argv_new(rcmd, root, #cmd, name_handler(cmd), &name_help(cmd)))

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
	rz_iterator_foreach (iter, ab) {
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
	RzListIter *iter;
	st64 bbs_idx = 0;
	ut64 old_offset = core->offset;
	ut64 old_bsize = core->blocksize;
	rz_list_foreach (fcn->bbs, iter, bbi) {
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
	for (int i = 0; i < argc; ++i) {
		if (i != 0) {
			rz_cons_print(" ");
		}
		const char *str = JS_ToCString(ctx, argv[i]);
		if (!str) {
			return JS_EXCEPTION;
		}
		rz_cons_print(str);
		JS_FreeCString(ctx, str);
	}
	rz_cons_newline();
	rz_cons_flush();
	return JS_UNDEFINED;
}

static JSValue js_get_global(JSContext *ctx, JSValueConst jsThis, int argc, JSValueConst *argv) {
	ExecContext *ectx = (ExecContext *)JS_GetContextOpaque(ctx);
	return JS_GetPropertyStr(ctx, ectx->shared, "Shared");
}

static jsdec_t *jsdec_create(ExecContext *ec, const char *arg) {
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
	if (RZ_STR_ISNOTEMPTY(arg)) {
		JS_SetPropertyInt64(ctx, args, 0, JS_NewString(ctx, arg));
	}
	JS_SetPropertyStr(ctx, process, "args", args);

	JS_FreeValue(ctx, global);
	return dec;
}

static void jsdec_destroy(jsdec_t *dec, ExecContext *ec) {
	JSContext *ctx = jsdec_context(dec);
	JS_FreeValue(ctx, ec->shared);
	jsdec_free(dec);
}

static bool jsdec_main(RzCore *core, const char *arg) {
	ExecContext ectx;
	ectx.core = core;

	jsdec_t *dec = jsdec_create(&ectx, arg);
	if (!dec) {
		return false;
	}

	ectx.bed = rz_cons_sleep_begin();
	bool ret = jsdec_run(dec);
	rz_cons_sleep_end(ectx.bed);

	jsdec_destroy(dec, &ectx);
	return ret;
}

static const RzCmdDescHelp pdd_usage = {
	.summary = "Core plugin for jsdec",
};

// static_description_no_args(cmd_pdd_star,"decompiled code is returned to rizin as comment (via CCu)");
static_description_no_args(pdd, "decompile current function");
static_description_no_args(pddt, "lists the supported architectures");
static_description_no_args(pddc, "decompiled code is returned to rizin as 'file:line code' (via CL)");
static_description_no_args(pdda, "decompile current function with side assembly");
static_description_no_args(pddb, "decompile current function but show only scopes");
static_description_no_args(pddo, "decompile current function side by side with offsets");
static_description_no_args(pddj, "decompile current function as json");
static_description_no_args(pddA, "decompile current function with annotation output");
static_description_no_args(pddf, "decompile all functions");
static_description_no_args(pddi, "generate issue data");

command_handler(pdd, NULL);
command_handler(pddt, "--architectures");
command_handler(pdda, "--assembly");
command_handler(pddA, "--annotation");
command_handler(pddb, "--blocks");
command_handler(pddc, "--as-code-line");
command_handler(pddf, "--all-functions");
command_handler(pddi, "--issue");
command_handler(pddj, "--as-json");
command_handler(pddo, "--offsets");

static bool rz_cmd_pdd_init(RzCore *core) {
	RzCmd *rcmd = core->rcmd;
	RzConfig *cfg = core->config;
	RzCmdDesc *root_cd = rz_cmd_get_desc(rcmd, "pd");
	if (!root_cd) {
		rz_warn_if_reached();
		return false;
	}

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

	RzCmdDesc *pdd = rz_cmd_desc_group_new(rcmd, root_cd, "pdd", name_handler(pdd), &name_help(pdd), &pdd_usage);
	if (!pdd) {
		rz_warn_if_reached();
		return false;
	}

	rz_cmd_desc_argv_new_warn(rcmd, pdd, pddt);
	rz_cmd_desc_argv_new_warn(rcmd, pdd, pdda);
	rz_cmd_desc_argv_new_warn(rcmd, pdd, pddA);
	rz_cmd_desc_argv_new_warn(rcmd, pdd, pddb);
	rz_cmd_desc_argv_new_warn(rcmd, pdd, pddc);
	rz_cmd_desc_argv_new_warn(rcmd, pdd, pddf);
	rz_cmd_desc_argv_new_warn(rcmd, pdd, pddi);
	rz_cmd_desc_argv_new_warn(rcmd, pdd, pddj);
	rz_cmd_desc_argv_new_warn(rcmd, pdd, pddo);

	return true;
}

static bool rz_cmd_pdd_fini(RzCore *core) {
	RzCmd *rcmd = core->rcmd;
	RzCmdDesc *cd = rz_cmd_get_desc(rcmd, "pdd");
	return rz_cmd_desc_remove(rcmd, cd);
}

RzCorePlugin rz_core_plugin_jsdec = {
	.name = "jsdec",
	.author = "deroad",
	.desc = "Pseudo-code decompiler for rizin",
	.license = "BSD-3-Clause",
	.init = rz_cmd_pdd_init,
	.fini = rz_cmd_pdd_fini,
};

#ifdef _MSC_VER
#define _RZ_API __declspec(dllexport)
#else
#define _RZ_API
#endif

#ifndef CORELIB
_RZ_API RzLibStruct rizin_plugin = {
	.type = RZ_LIB_TYPE_CORE,
	.data = &rz_core_plugin_jsdec,
	.version = RZ_VERSION,
};
#endif
