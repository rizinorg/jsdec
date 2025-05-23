# SPDX-FileCopyrightText: 2023-2024 Giovanni Dante Grazioli <deroad@libero.it>
# SPDX-License-Identifier: BSD-3-Clause

project('jsdec', 'c', meson_version: '>=0.56.0',
  default_options: [
    'buildtype=release',
    'b_vscrt=from_buildtype',
    'warning_level=1',
    'c_std=c11'
  ])

cc = meson.get_compiler('c')

jsdec_incs = ['.', 'c', 'include']
jsdec_c_args = []
jsdec_src = [
    'c' / 'jsdec.c',
    'c' / 'base64.c',
]

libquickjs_proj = subproject('libquickjs', default_options: ['default_library=static'])
libquickjs_dep = libquickjs_proj.get_variable('quickjs_dep')

jsdec_deps = [ libquickjs_dep ]

modjs_gen = executable('modjs_gen', 'tools' / 'modjs_gen.c',
  dependencies: [],
  include_directories: [],
  implicit_include_directories: false,
  install: false,
)

qjsc = executable('qjsc', 'tools' / 'qjsc_mod.c',
  dependencies: libquickjs_dep,
  c_args: ['-D_GNU_SOURCE=1'],
  include_directories: [],
  implicit_include_directories: false,
  install: false,
)

subdir('js')

jsdec_link_args = []
foreach flag: [
    '-Wl,--exclude-libs,ALL',
  ]
  if cc.has_link_argument(flag)
    jsdec_link_args += flag
  endif
endforeach

build_type = get_option('build_type')
if build_type == 'standalone'
  jsdec_src += [
    bytecode_h,
    bytecode_mod_h,
    'c' / 'jsdec-standalone.c',
  ]

  jsdec_standalone = executable('jsdec-standalone', jsdec_src,
    c_args : jsdec_c_args,
    dependencies: jsdec_deps,
    link_args: jsdec_link_args,
    gnu_symbol_visibility: 'hidden',
    include_directories: include_directories(jsdec_incs),
    implicit_include_directories: false,
    install: false,
  )
elif build_type == 'cutter'
  jsdec_deps = [ libquickjs_dep.as_link_whole() ]
  jsdec_deps += dependency('rz_core')
  jsdec_deps += dependency('rz_util')
  jsdec_deps += dependency('rz_cons')
  jsdec_deps += dependency('rz_config')
  jsdec_deps += dependency('rz_io')

  jsdec_src += [
    bytecode_h,
    bytecode_mod_h,
    'c' / 'jsdec-cutter.c'
  ]

  if cc.has_argument('-fvisibility=hidden')
    jsdec_c_args += '-fvisibility=hidden'
  endif
  if cc.has_argument('-flto')
    jsdec_c_args += '-flto'
  endif

  libjsdec = static_library('jsdec', jsdec_src,
    c_args : jsdec_c_args,
    dependencies: jsdec_deps,
    include_directories: include_directories(jsdec_incs),
    link_args: jsdec_link_args,
    gnu_symbol_visibility: 'hidden',
    implicit_include_directories: false,
    install: false,
  )
  libjsdec_dep = declare_dependency(
    link_with: libjsdec,
    include_directories: include_directories(['include']),
  )
elif build_type == 'rizin'
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
  endif

  jsdec_src += [
    bytecode_h,
    bytecode_mod_h,
    'c' / 'jsdec-plugin.c'
  ]

  if cc.has_argument('-fvisibility=hidden')
    jsdec_c_args += '-fvisibility=hidden'
  endif
  if cc.has_argument('-flto')
    jsdec_c_args += '-flto'
  endif

  shared_library('core_pdd', jsdec_src,
    c_args : jsdec_c_args,
    dependencies: jsdec_deps,
    include_directories: include_directories(jsdec_incs),
    link_args: jsdec_link_args,
    gnu_symbol_visibility: 'hidden',
    implicit_include_directories: false,
    install: true,
    install_dir: rizin_plugdir
  )
else
  error('Invalid `build_type`.')
endif
