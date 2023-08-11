// SPDX-FileCopyrightText: 2023 Giovanni Dante Grazioli <deroad@libero.it>
// SPDX-License-Identifier: BSD-3-Clause

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "jsdec.h"
#include "js/bytecode.h"

struct jsdec_s {
	JSRuntime *runtime;
	JSContext *context;
};

#define macro_str(s) #s
#define errorf(...)  fprintf(stderr, __VA_ARGS__)

void js_print_exception(JSContext *ctx, JSValueConst val) {
	const char *strval = JS_ToCString(ctx, val);
	if (strval) {
		fprintf(stderr, "%s\n", strval);
		JS_FreeCString(ctx, strval);
	} else {
		fprintf(stderr, "[exception]\n");
	}
}

void jsdec_handle_exception(JSContext *ctx) {
	JSValue exception = JS_GetException(ctx);
	int is_error = JS_IsError(ctx, exception);

	js_print_exception(ctx, exception);
	if (!is_error) {
		JS_FreeValue(ctx, exception);
		return;
	}

	JSValue stack = JS_GetPropertyStr(ctx, exception, "stack");
	if (!JS_IsUndefined(stack)) {
		js_print_exception(ctx, stack);
	}
	JS_FreeValue(ctx, stack);
	JS_FreeValue(ctx, exception);
}

#if 0
static void print_jsval(JSValue val) {
	if(JS_IsNumber(val)) {
		errorf("JS_IsNumber\n");
	} else if(JS_IsBigInt(NULL, val)) {
		errorf("JS_IsBigInt\n");
	} else if(JS_IsBigFloat(val)) {
		errorf("JS_IsBigFloat\n");
	} else if(JS_IsBigDecimal(val)) {
		errorf("JS_IsBigDecimal\n");
	} else if(JS_IsBool(val)) {
		errorf("JS_IsBool\n");
	} else if(JS_IsNull(val)) {
		errorf("JS_IsNull\n");
	} else if(JS_IsUndefined(val)) {
		errorf("JS_IsUndefined\n");
	} else if(JS_IsException(val)) {
		errorf("JS_IsException\n");
	} else if(JS_IsUninitialized(val)) {
		errorf("JS_IsUninitialized\n");
	} else if(JS_IsString(val)) {
		errorf("JS_IsString\n");
	} else if(JS_IsSymbol(val)) {
		errorf("JS_IsSymbol\n");
	} else if(JS_IsObject(val)) {
		errorf("JS_IsObject\n");
	} else if(JS_VALUE_GET_TAG(val) == JS_TAG_MODULE) {
		errorf("Module\n");
	} else if(JS_VALUE_GET_TAG(val) == JS_TAG_FUNCTION_BYTECODE) {
		errorf("Function bytecode\n");
	} else {
		errorf("No idea...\n");
	}
}
#endif

static int js_add_module(JSContext *ctx, const uint8_t *bytecode, const uint32_t size) {
	JSValue obj = JS_ReadObject(ctx, bytecode, size, JS_READ_OBJ_BYTECODE);
	if (JS_IsException(obj)) {
		jsdec_handle_exception(ctx);
		return 0;
	}
	return 1;
}

static int js_load_module(JSContext *ctx, const uint8_t *bytes, const uint32_t size) {
	JSValue obj = JS_ReadObject(ctx, bytes, size, JS_READ_OBJ_BYTECODE);
	if (JS_IsException(obj)) {
		jsdec_handle_exception(ctx);
		return 0;
	}

	if (JS_VALUE_GET_TAG(obj) == JS_TAG_MODULE &&
		JS_ResolveModule(ctx, obj) < 0) {
		JS_FreeValue(ctx, obj);
		errorf("Error: failed to resolve jsdec module\n");
		return 0;
	}

	JSValue val = JS_EvalFunction(ctx, obj);
	if (JS_IsException(val)) {
		jsdec_handle_exception(ctx);
		return 0;
	}
	JS_FreeValue(ctx, val);
	return 1;
}

#include "js/bytecode_mod.h"

void jsdec_free(jsdec_t *dec) {
	if (!dec) {
		return;
	}

	JS_FreeContext(dec->context);
	JS_FreeRuntime(dec->runtime);
	free(dec);
}

jsdec_t *jsdec_new() {
	JSRuntime *rt = JS_NewRuntime();
	if (!rt) {
		errorf("Error: failed to create qjs runtime\n");
		return NULL;
	}

	JSContext *ctx = JS_NewContextRaw(rt);
	if (!ctx) {
		errorf("Error: failed to create qjs context\n");
		JS_FreeRuntime(rt);
		return NULL;
	}

	// initialize all intrisic
	JS_AddIntrinsicBaseObjects(ctx);
	JS_AddIntrinsicRegExp(ctx);
	JS_AddIntrinsicJSON(ctx);
	JS_AddIntrinsicMapSet(ctx);
	JS_AddIntrinsicPromise(ctx);
	JS_AddIntrinsicBigInt(ctx);
	JS_AddIntrinsicBigFloat(ctx);
	JS_AddIntrinsicBigDecimal(ctx);
	JS_AddIntrinsicOperators(ctx);
	JS_AddIntrinsicEval(ctx);
	JS_EnableBignumExt(ctx, 1);

	if (!js_load_all_modules(ctx)) {
		JS_FreeContext(ctx);
		JS_FreeRuntime(rt);
		return NULL;
	}

	jsdec_t *dec = malloc(sizeof(jsdec_t));
	if (!dec) {
		errorf("Error: failed to allocate jsdec_t\n");
		JS_FreeContext(ctx);
		JS_FreeRuntime(rt);
		return NULL;
	}

	dec->context = ctx;
	dec->runtime = rt;
	return dec;
}

JSContext *jsdec_context(const jsdec_t *dec) {
	return dec->context;
}

int jsdec_run(const jsdec_t *dec) {
	return js_load_module(dec->context, main_bytecode, main_bytecode_size);
}
