/* rizin - GPL3 - Copyright 2020 deroad */

#include <stdlib.h>
#include <string.h>
#include <rz_types.h>
#include <rz_lib.h>
#include <rz_cmd.h>
#include <rz_core.h>
#include <rz_cons.h>
#include <rz_analysis.h>
#include "duktape.h"
#include "duk_console.h"
#include "duk_missing.h"

#include "jsdec_ctx.h"

#undef RZ_API
#define RZ_API static
#undef RZ_IPI
#define RZ_IPI static
#define SETDESC(x,y) rz_config_node_desc (x,y)
#define SETPREF(x,y,z) SETDESC (rz_config_set (cfg,x,y), z)

/* for compatibility. */
#ifndef RZ_HOME_DATADIR
#define RZ_HOME_DATADIR RZ_HOMEDIR
#endif

static char* jsdec_read_file(const char* file) {
	if (!file) {
		return 0;
	}
	char *homedir;
	char *env = rz_sys_getenv ("JSDEC_HOME");
	if (env) {
		homedir = env;
	} else {
#ifdef JSDEC_HOME
		homedir = rz_str_new (JSDEC_HOME);
#else
		homedir = rz_str_home (RZ_HOME_DATADIR RZ_SYS_DIR
			"rzpm" RZ_SYS_DIR "git" RZ_SYS_DIR "jsdec");
#endif
	}
	size_t len = 0;
	if (!homedir) {
		return 0;
	}
	char *filepath = rz_str_newf ("%s"RZ_SYS_DIR"%s", homedir, file);
	free (homedir);
	char* text = rz_file_slurp (filepath, &len);
	if (text && len > 0) {
		free (filepath);
		return text;
	}
	free (filepath);
	return 0;
}

static duk_ret_t duk_rzcmd(duk_context *ctx) {
	if (duk_is_string (ctx, 0)) {
		const char* command = duk_safe_to_string (ctx, 0);
	    //fprintf (stderr, "RZCMD: %s\n", command);
	    //fflush (stderr);
		JsDecCtx *jsdec_ctx = jsdec_ctx_get (ctx);
		rz_cons_sleep_end (jsdec_ctx->bed);
		char* output = rz_core_cmd_str (jsdec_ctx->core, command);
		jsdec_ctx->bed = rz_cons_sleep_begin ();
		duk_push_string (ctx, output);
		free (output);
		return 1;
	}
	return DUK_RET_TYPE_ERROR;
}

static duk_ret_t duk_internal_load(duk_context *ctx) {
	if (duk_is_string (ctx, 0)) {
		const char* fullname = duk_safe_to_string (ctx, 0);
		char* text = jsdec_read_file (fullname);
		if (text) {
			duk_push_string (ctx, text);
			free (text);
		} else {
			printf("Error: '%s' not found.\n", fullname);
			return DUK_RET_TYPE_ERROR;
		}
		return 1;
	}
	return DUK_RET_TYPE_ERROR;
}

static duk_ret_t duk_internal_require(duk_context *ctx) {
	char fullname[256];
	if (duk_is_string (ctx, 0)) {
		snprintf (fullname, sizeof(fullname), "%s.js", duk_safe_to_string (ctx, 0));
		char* text = jsdec_read_file (fullname);
		if (text) {
			duk_push_lstring (ctx, fullname, strlen (fullname));
			duk_eval_file (ctx, text);
			free (text);
		} else {
			printf("Error: '%s' not found.\n", fullname);
			return DUK_RET_TYPE_ERROR;
		}
		return 1;
	}
	return DUK_RET_TYPE_ERROR;
}

static void duk_jsdec_init(duk_context* ctx, JsDecCtx *jsdec_ctx) {
	duk_push_global_stash (ctx);
	duk_push_pointer (ctx, (void *)jsdec_ctx);
	duk_put_prop_string (ctx, -2, "jsdec_ctx");
	duk_pop (ctx);

	duk_push_c_function (ctx, duk_internal_require, 1);
	duk_put_global_string (ctx, "___internal_require");
	duk_push_c_function (ctx, duk_internal_load, 1);
	duk_put_global_string (ctx, "___internal_load");
	duk_push_c_function (ctx, duk_rzcmd, 1);
	duk_put_global_string (ctx, "rzcmd");
}

JsDecCtx *jsdec_ctx_get(duk_context *ctx) {
	duk_push_global_stash (ctx);
	duk_get_prop_string (ctx, -1, "jsdec_ctx");
	JsDecCtx *r = duk_require_pointer (ctx, -1);
	duk_pop_2 (ctx);
	return r;
}

//static void duk_rizin_debug_stack(duk_context* ctx) {
//	duk_push_context_dump(ctx);
//	printf("%s\n", duk_to_string(ctx, -1));
//	duk_pop(ctx);
//}

static void eval_file(duk_context* ctx, const char* file) {
    //fprintf (stderr, "REQUIRE: %s\n", file);
    //fflush (stderr);
	char* text = jsdec_read_file (file);
	if (text) {
		duk_push_lstring (ctx, file, strlen (file));
		duk_eval_file_noresult (ctx, text);
		free (text);
	}
}

static void jsdec_fatal_function (void *udata, const char *msg) {
    fprintf (stderr, "*** FATAL ERROR: %s\n", (msg ? msg : "no message"));
    fflush (stderr);
    abort ();
}

static void duk_jsdec(RzCore *core, const char *input) {
	char args[1024] = {0};
	JsDecCtx jsdec_ctx;
	jsdec_ctx.core = core;
	jsdec_ctx.bed = rz_cons_sleep_begin ();
	duk_context *ctx = duk_create_heap (0, 0, 0, 0, jsdec_fatal_function);
	duk_console_init (ctx, 0);
//	Long_init (ctx);
	duk_jsdec_init (ctx, &jsdec_ctx);
	eval_file (ctx, "require.js");
	eval_file (ctx, "jsdec-duk.js");
	if (*input) {
		snprintf (args, sizeof(args), "try{if(typeof jsdec_main == 'function'){jsdec_main(\"%s\".split(/\\s+/));}else{console.log('Fatal error. Cannot use RZ_HOME_DATADIR or JSDEC_HOME.');}}catch(_____e){console.log(_____e.stack||_____e);}", input);
	} else {
		snprintf (args, sizeof(args), "try{if(typeof jsdec_main == 'function'){jsdec_main([]);}else{console.log('Fatal error. Cannot use RZ_HOME_DATADIR or JSDEC_HOME.');}}catch(_____e){console.log(_____e.stack||_____e);}");
	}
	duk_eval_string_noresult (ctx, args);
	//duk_rizin_debug_stack(ctx);
	duk_destroy_heap (ctx);
	rz_cons_sleep_end(jsdec_ctx.bed);
}

static void usage(const RzCore* const core) {
	const char* help[] = {
		"Usage: pdd[*abousi]", "",	"# Core plugin for jsdec",
		"pdd",	"",        "decompile current function",
		"pdd*",	"",        "decompiled code is returned to rizin as comment (via CCu)",
		"pddc",	"",        "decompiled code is returned to rizin as 'file:line code' (via CL)",
		"pdda",	"",        "decompile current function with side assembly",
		"pddb",	"",        "decompile current function but show only scopes",
		"pddo",	"",        "decompile current function side by side with offsets",
		"pddj", "",        "decompile current function as json",
		"pddA", "",        "decompile current function with annotation output",
		"pddf", "",        "decompile all functions",
		"pddi",	"",        "generate issue data",

		// "Evaluable Variables:", "", "",
		// "jsdec.casts",	"",	"if false, hides all casts in the pseudo code",
		// "jsdec.asm",	"",	"if true, shows pseudo next to the assembly",
		// "jsdec.blocks",	"",	"if true, shows only scopes blocks",
		// "jsdec.xrefs",	"",	"if true, shows all xrefs in the pseudo code",
		// "jsdec.paddr",	"",	"if true, all xrefs uses physical addresses compare",
		// "jsdec.theme",	"",	"defines the color theme to be used on jsdec",

		// "Environment:", "", "",
		// "JSDEC_HOME",	"",	"defaults to the root directory of the jsdec repo",

		NULL
	};

	rz_cons_cmd_help(help, core->print->flags & RZ_PRINT_FLAGS_COLOR);
}


static void _cmd_pdd(RzCore *core, const char *input) {
	switch (*input) {
	case '\0':
		duk_jsdec (core, input);
		break;
	case ' ':
		duk_jsdec (core, input + 1);
		break;
	case 'i':
		// --issue
		duk_jsdec (core, "--issue");
		break;
	case 'a':
		// --assembly
		duk_jsdec (core, "--assembly");
		break;
	case 'o':
		// --offsets
		duk_jsdec (core, "--offsets");
		break;
	case 'b':
		// --blocks
		duk_jsdec (core, "--blocks");
		break;
	case 'c':
		// --as-code-line
		duk_jsdec (core, "--as-code-line");
		break;
	case 'f':
		duk_jsdec (core, "--all-functions");
		break;
	case '*':
		// --as-comment
		duk_jsdec (core, "--as-comment");
		break;
	case 'j':
		// --as-json
		duk_jsdec (core, "--as-json");
		break;
	case 'A':
		// --as-json
		duk_jsdec (core, "--annotation");
		break;
	case '?':
	default:
		usage(core);
		break;
	}
}

static int rz_cmd_pdd(void *user, const char *input) {
	RzCore *core = (RzCore *) user;
	if (!strncmp (input, "pdd", 3)) {
		_cmd_pdd (core, input + 3);
		return true;
	}
	return false;
}

int rz_cmd_pdd_init(void *user, const char *cmd) {
	RzCmd *rcmd = (RzCmd*) user;
	RzCore *core = (RzCore *) rcmd->data;
	RzConfig *cfg = core->config;
	rz_config_lock (cfg, false);
	SETPREF("jsdec.asm", "false", "if true, shows pseudo next to the assembly.");
	SETPREF("jsdec.blocks", "false", "if true, shows only scopes blocks.");
	SETPREF("jsdec.casts", "false", "if false, hides all casts in the pseudo code.");
	SETPREF("jsdec.debug", "false", "do not catch exceptions in jsdec.");
	SETPREF("jsdec.highlight", "default", "highlights the current address.");
	SETPREF("jsdec.paddr", "false", "if true, all xrefs uses physical addresses compare.");
	SETPREF("jsdec.slow", "false", "load all the data before to avoid multirequests to rizin.");
	SETPREF("jsdec.theme", "default", "defines the color theme to be used on jsdec.");
	SETPREF("jsdec.xrefs", "false", "if true, shows all xrefs in the pseudo code.");
	rz_config_lock (cfg, true);

	// autocomplete here..
	RzCoreAutocomplete *pdd = rz_core_autocomplete_add (core->autocomplete, "pdd", RZ_CORE_AUTOCMPLT_DFLT, true);
	rz_core_autocomplete_add (core->autocomplete, "pdd?", RZ_CORE_AUTOCMPLT_DFLT, true);
	rz_core_autocomplete_add (core->autocomplete, "pdd*", RZ_CORE_AUTOCMPLT_DFLT, true);
	rz_core_autocomplete_add (core->autocomplete, "pdda", RZ_CORE_AUTOCMPLT_DFLT, true);
	rz_core_autocomplete_add (core->autocomplete, "pddb", RZ_CORE_AUTOCMPLT_DFLT, true);
	rz_core_autocomplete_add (core->autocomplete, "pddc", RZ_CORE_AUTOCMPLT_DFLT, true);
	rz_core_autocomplete_add (core->autocomplete, "pddf", RZ_CORE_AUTOCMPLT_DFLT, true);
	rz_core_autocomplete_add (core->autocomplete, "pddi", RZ_CORE_AUTOCMPLT_DFLT, true);
	rz_core_autocomplete_add (core->autocomplete, "pdds", RZ_CORE_AUTOCMPLT_DFLT, true);
	rz_core_autocomplete_add (core->autocomplete, "pddu", RZ_CORE_AUTOCMPLT_DFLT, true);
	rz_core_autocomplete_add (pdd, "--all-functions", RZ_CORE_AUTOCMPLT_OPTN, true);
	rz_core_autocomplete_add (pdd, "--as-code-line", RZ_CORE_AUTOCMPLT_OPTN, true);
	rz_core_autocomplete_add (pdd, "--as-comment", RZ_CORE_AUTOCMPLT_OPTN, true);
	rz_core_autocomplete_add (pdd, "--assembly", RZ_CORE_AUTOCMPLT_OPTN, true);
	rz_core_autocomplete_add (pdd, "--blocks", RZ_CORE_AUTOCMPLT_OPTN, true);
	rz_core_autocomplete_add (pdd, "--casts", RZ_CORE_AUTOCMPLT_OPTN, true);
	rz_core_autocomplete_add (pdd, "--colors", RZ_CORE_AUTOCMPLT_OPTN, true);
	rz_core_autocomplete_add (pdd, "--debug", RZ_CORE_AUTOCMPLT_OPTN, true);
	rz_core_autocomplete_add (pdd, "--html", RZ_CORE_AUTOCMPLT_OPTN, true);
	rz_core_autocomplete_add (pdd, "--issue", RZ_CORE_AUTOCMPLT_OPTN, true);
	rz_core_autocomplete_add (pdd, "--offsets", RZ_CORE_AUTOCMPLT_OPTN, true);
	rz_core_autocomplete_add (pdd, "--paddr", RZ_CORE_AUTOCMPLT_OPTN, true);
	rz_core_autocomplete_add (pdd, "--xrefs", RZ_CORE_AUTOCMPLT_OPTN, true);
	return true;
}

RzCorePlugin rz_core_plugin_test = {
	.name = "jsdec",
	.desc = "Pseudo-code decompiler for rizin",
	.license = "GPL3",
	.call = rz_cmd_pdd,
	.init = rz_cmd_pdd_init
};

#ifdef _MSC_VER
#define _RZ_API __declspec(dllexport)
#else
#define _RZ_API
#endif

#ifndef CORELIB
_RZ_API RzLibStruct rizin_plugin = {
	.type = RZ_LIB_TYPE_CORE,
	.data = &rz_core_plugin_test,
	.version = RZ_VERSION,
	.pkgname = "jsdec"
};
#endif
