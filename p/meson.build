project('jsdec', 'c', meson_version: '>=0.46.0')

pyth = import('python').find_installation()
rizin = find_program('rizin', required: false)
cc = meson.get_compiler('c')

incs = ['.']
deps = []
c_args = []

rizin_incdir = get_option('rizin_incdir')
if rizin_incdir == '' and rizin.found()
  rizin_incdir = run_command(rizin, '-H', 'RZ_INCDIR').stdout().strip()
endif

rizin_libdir = get_option('rizin_libdir')
if rizin_libdir == '' and rizin.found()
  rizin_libdir = run_command(rizin, '-H', 'RZ_LIBDIR').stdout().strip()
endif

rizin_plugdir = get_option('rizin_plugdir')
if rizin_plugdir == '' and rizin.found()
  rizin_plugdir = run_command(rizin, '-H', 'RZ_USER_PLUGINS').stdout().strip()
  if rizin_plugdir == ''
    rizin_plugdir = get_option('libdir')
  endif
endif

plugin_jsdec_dir = join_paths(rizin_plugdir, 'jsdec')

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

c_args += ('-DJSDEC_HOME="' + plugin_jsdec_dir + '"')

jsc_folder = get_option('jsc_folder')
if jsc_folder != ''
  c_args += '-DUSE_JSC'
  generator_jsc = files('make_jsc.py')
  rc = run_command(pyth, generator_jsc, jsc_folder)
  if rc.returncode() != 0
    error('failed to generate jsc files.')
  endif
  jsdec_jsc = configuration_data()
  jsdec_jsc.set('JSC_SOURCES', rc.stdout())
  rz_version_h = configure_file(
    input: 'jsdec_jsc.c.in',
    output: 'jsdec_jsc.c',
    configuration: jsdec_jsc,
  )
endif

message('Rizin Include Dir: ' + rizin_incdir)
message('Rizin Library Dir: ' + rizin_libdir)
message('Rizin Plugin Dir:  ' + rizin_plugdir)
if jsc_folder != ''
  message('JS to C Folder:    ' + jsc_folder)
endif

shared_library('core_pdd', files,
  c_args : c_args,
  dependencies: deps,
  include_directories: include_directories(incs),
  implicit_include_directories: false,
  install: true,
  install_dir: rizin_plugdir
)

if jsc_folder == ''
  install_subdir(join_paths('..', 'libdec'), install_dir: plugin_jsdec_dir)
  install_subdir(join_paths('..', 'themes'), install_dir: plugin_jsdec_dir)
  install_data(join_paths('..', 'jsdec-duk.js'), install_dir: plugin_jsdec_dir)
  install_data(join_paths('..', 'require.js'), install_dir: plugin_jsdec_dir)
endif