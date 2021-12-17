project('jsdec', 'c', meson_version: '>=0.51.0')

pyth = import('python').find_installation()
cc = meson.get_compiler('c')

incs = ['.', 'duktape']
deps = []
c_args = []

rz_core_dep = dependency('rz_core')
deps += rz_core_dep
deps += dependency('rz_util')
deps += dependency('rz_cons')
deps += dependency('rz_config')
deps += dependency('rz_io')
deps += cc.find_library('m', required: false)

rizin_plugdir = get_option('rizin_plugdir')
if rizin_plugdir == ''
  rizin_plugdir = rz_core_dep.get_variable(pkgconfig: 'plugindir', cmake: 'rz_core_PLUGINDIR')
  plugin_jsdec_dir = join_paths(get_option('prefix'), rizin_plugdir, 'jsdec')
else
  plugin_jsdec_dir = join_paths(rizin_plugdir, 'jsdec')
endif


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

message('Rizin Plugin Dir:  ' + plugin_jsdec_dir)
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
