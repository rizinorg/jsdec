# SPDX-FileCopyrightText: 2023 Giovanni Dante Grazioli <deroad@libero.it>
# SPDX-License-Identifier: BSD-3-Clause

project('jsdec', 'c', meson_version: '>=0.55.0')

cc = meson.get_compiler('c')

libquickjs_proj = subproject('libquickjs', default_options: ['default_library=static'])
libquickjs_dep = libquickjs_proj.get_variable('quickjs_dep')
qjsc = libquickjs_proj.get_variable('qjsc')

jsdec_incs = ['.', 'c']
jsdec_c_args = []
jsdec_src = [
    'c' / 'jsdec.c',
    'c' / 'base64.c',
]
jsdec_deps = [
  libquickjs_dep,
]

if get_option('standalone')
  add_project_arguments('-DJSDEC_STANDALONE', language: 'c')
endif

modjs_gen = executable('modjs_gen', 'tools' / 'modjs_gen.c',
  dependencies: [],
  include_directories: [],
  implicit_include_directories: false,
  install: false,
)

subdir('js')

if get_option('standalone')

  jsdec_src += [
    bytecode_h,
    bytecode_mod_h,
    'c' / 'jsdec-standalone.c',
  ]

  jsdec_standalone = executable('jsdec-standalone', jsdec_src,
    c_args : jsdec_c_args,
    dependencies: jsdec_deps,
    include_directories: include_directories(jsdec_incs),
    implicit_include_directories: false,
    install: false,
  )
else
  # build plugin for Rizin
  rz_core_dep = dependency('rz_core')
  jsdec_deps += rz_core_dep
  jsdec_deps += dependency('rz_util')
  jsdec_deps += dependency('rz_cons')
  jsdec_deps += dependency('rz_config')
  jsdec_deps += dependency('rz_io')
  rizin_plugdir = get_option('rizin_plugdir')
  if rizin_plugdir == ''
    rizin_plugdir = rz_core_dep.get_variable(pkgconfig: 'plugindir', cmake: 'rz_core_PLUGINDIR')
    plugin_jsdec_dir = join_paths(get_option('prefix'), rizin_plugdir, 'jsdec')
  else
    plugin_jsdec_dir = join_paths(rizin_plugdir, 'jsdec')
  endif

  jsdec_src += [
    bytecode_h,
    bytecode_mod_h,
    'c' / 'jsdec-plugin.c'
  ]

  shared_library('core_pdd', jsdec_src,
    c_args : jsdec_c_args,
    dependencies: jsdec_deps,
    include_directories: include_directories(jsdec_incs),
    implicit_include_directories: false,
    install: true,
    install_dir: rizin_plugdir
  )
endif
