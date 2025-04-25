// SPDX-FileCopyrightText: 2024 Giovanni Dante Grazioli <deroad@libero.it>
// SPDX-License-Identifier: BSD-3-Clause

#ifndef JSDEC_CUTTER_H
#define JSDEC_CUTTER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <rz_util/rz_annotated_code.h>

RZ_API void jsdec_init_config(RzCore *core);
RZ_API RzAnnotatedCode *jsdec_as_annotation(RzCore *core, ut64 addr);

#ifdef __cplusplus
}
#endif

#endif /* JSDEC_CUTTER_H */
