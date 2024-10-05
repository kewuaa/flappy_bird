project(
  'flappy_bird',
  'cpp',
  version : '0.1',
  default_options : [
    'warning_level=3',
    'cpp_std=c++20'
  ]
)

raylib_dep = dependency('raylib', required: false, allow_fallback: false)
static_link_raylib = get_option('static_link_raylib')
if not raylib_dep.found() or static_link_raylib
  cmake = import('cmake')
  raylib_opts = cmake.subproject_options()
  raylib_opts.add_cmake_defines({
    'BUILD_EXAMPLES': false,
    'BUILD_SHARED_LIBS': not static_link_raylib,
    'CMAKE_BUILD_TYPE': 'release',
    'GLFW_BUILD_WAYLAND': run_command('sh', '-c', 'echo $WAYLAND_DISPLAY').stdout() != ''
  })
  raylib = cmake.subproject('raylib', options: raylib_opts)
  raylib_dep = declare_dependency(
    dependencies: [
      raylib.dependency('raylib'),
      raylib.dependency('glfw')
    ]
  )
endif

flappy_bird = executable(
  'flappy_bird',
  sources: 'src/flappy_bird.cpp',
  dependencies: [raylib_dep],
  install: true,
  install_dir: '.'
)
install_data('resource/background.bmp')

test(
  'test_flappy_bird',
  flappy_bird,
  timeout: -1
)
