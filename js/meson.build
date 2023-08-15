# SPDX-FileCopyrightText: 2023 Giovanni Dante Grazioli <deroad@libero.it>
# SPDX-License-Identifier: BSD-3-Clause

jsdec_plugin = 'jsdec-plugin.js'
jsdec_testsuite = 'jsdec-testsuite.js'

if get_option('standalone')
  bytecode_h = custom_target(
      'bytecode.h',
      output : 'bytecode.h',
      input : jsdec_testsuite,
      command : [qjsc, '@INPUT@', '-m', '-N', 'main_bytecode', '-c', '-o', '@OUTPUT@'],
  )
else
  bytecode_h = custom_target(
      'bytecode.h',
      output : 'bytecode.h',
      input : jsdec_plugin,
      command : [qjsc, '@INPUT@', '-m', '-N', 'main_bytecode', '-c', '-o', '@OUTPUT@'],
  )
endif

bytecode_mod_h = custom_target(
    'bytecode_mod.h',
    output : 'bytecode_mod.h',
    input : bytecode_h,
    command : [modjs_gen, '@INPUT@', '@OUTPUT@'],
)