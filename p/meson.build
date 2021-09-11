project('jsdec', 'c', meson_version: '>=0.46.0')

rizin = find_program('rizin', required: false)
cc = meson.get_compiler('c')

incs = []
deps = []

rizin_incdir = get_option('rizin_incdir')
if rizin_incdir == '' and rizin.found()
  rizin_incdir = run_command(rizin, '-H', 'RZ_INCDIR').stdout().strip()
endif

rizin_libdir = get_option('rizin_libdir')
if rizin_libdir == '' and rizin.found()
  rizin_libdir = run_command(rizin, '-H', 'RZ_LIBDIR').stdout().strip()
endif

libs = ['rz_core', 'rz_util', 'rz_cons', 'rz_config', 'rz_io']
foreach lib : libs
  deps += cc.find_library(lib, dirs: rizin_libdir)
endforeach

deps += cc.find_library('m', required: false)

if rizin_incdir != ''
  incs += rizin_incdir
  incs += rizin_incdir + '/sdb'
endif

incs += 'duktape'

files = [
  'core_pdd.c',
  'duktape/duktape.c',
  'duktape/duk_console.c'
]

shared_library('core_pdd', files,
  dependencies: deps,
  include_directories: include_directories(incs),
  implicit_include_directories: false,
  install: true,
  install_dir: get_option('libdir')
)

dst = join_paths(get_option('datadir'), 'jsdec')
install_subdir(join_paths('..', 'libdec'), install_dir: dst)
install_subdir(join_paths('..', 'themes'), install_dir: dst)
install_data(join_paths('..', 'jsdec-duk.js'), install_dir: dst)
install_data(join_paths('..', 'require.js'), install_dir: dst)
