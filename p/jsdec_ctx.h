
#ifndef JSDEC_CTX_H
#define JSDEC_CTX_H

#include <duktape.h>

typedef struct rz_core_t RzCore;

typedef struct jsdec_ctx_t {
	RzCore *core;
	void *bed;
} JsDecCtx;

JsDecCtx *jsdec_ctx_get(duk_context *ctx);

#endif //JSDEC_CTX_H
