project(
  'flappy_bird',
  'cpp',
  version : '0.1',
  default_options : [
    'warning_level=3',
    'cpp_std=c++20'
  ]
)

system = host_machine.system()

raylib_dep = dependency('raylib', required: false, allow_fallback: false)
static_link_raylib = get_option('static_link_raylib')
if static_link_raylib or not raylib_dep.found()
  cmake = import('cmake')
  raylib_opts = cmake.subproject_options()
  raylib_opts.add_cmake_defines({
    'BUILD_EXAMPLES': false,
    'BUILD_SHARED_LIBS': not static_link_raylib,
    'CMAKE_BUILD_TYPE': 'release',
  })
  raylib = cmake.subproject('raylib', options: raylib_opts)
  dependencies = [
    raylib.dependency('raylib'),
    raylib.dependency('glfw')
  ]
  cc = meson.get_compiler('c')
  if system == 'windows'
    dependencies += cc.find_library('winmm', required: true)
  endif
  raylib_dep = declare_dependency(
    dependencies: dependencies
  )
endif

flappy_bird = executable(
  'flappy_bird',
  sources: 'src/flappy_bird.cpp',
  dependencies: [raylib_dep],
  install: true,
  install_dir: '.'
)

test(
  'test_flappy_bird',
  flappy_bird,
  timeout: -1
)
