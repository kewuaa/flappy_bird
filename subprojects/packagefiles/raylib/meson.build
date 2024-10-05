project('raylib', 'c')

default_library = get_option('default_library')

make = find_program('make', required: true)
cmd = [make, '-C', 'src', 'clean', 'all']
if default_library == 'shared'
  cmd += 'RAYLIB_LIBTYPE=SHARED'
endif
run_command(
  cmd,
  check: true
)

cc = meson.get_compiler('c')
raylib_include_dirs = [include_directories('src')]
raylib = cc.find_library(
  'raylib',
  dirs: meson.project_source_root() / 'src',
  has_headers: ['raylib.h'],
  header_include_directories: raylib_include_dirs,
  required: true
)

raylib_dep = declare_dependency(
  dependencies: raylib,
  include_directories: raylib_include_dirs
)
