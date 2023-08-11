// SPDX-FileCopyrightText: 2023 Giovanni Dante Grazioli <deroad@libero.it>
// SPDX-License-Identifier: BSD-3-Clause

#ifndef JSDEC_H
#define JSDEC_H

#include <quickjs.h>

typedef struct jsdec_s jsdec_t;

void jsdec_free(jsdec_t *dec);
jsdec_t *jsdec_new();
JSContext *jsdec_context(const jsdec_t *dec);
int jsdec_run(const jsdec_t *dec);
void jsdec_handle_exception(JSContext *ctx);

#endif /* JSDEC_H */
