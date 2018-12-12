# Resources; we cannot use gnome.compile_resources() here, because we need to
# override the environment in order to use the utilities we just built instead
# of the system ones
resources_c = custom_target('resources.c',
  input: 'resources.gresource.xml',
  output: 'resources.c',
  command: [
    gen_resources,
    '--pixdata=@0@'.format(gdk_pixbuf_pixdata.full_path()),
    '--loaders=@0@'.format(loaders_cache.full_path()),
    '--sourcedir=@0@'.format(meson.current_source_dir()),
    '--source',
    '@INPUT@',
    '@OUTPUT@',
  ],
  depends: [
    gdk_pixbuf_pixdata,
    loaders_cache,
  ],
)

resources_h = custom_target('resources.h',
  input: 'resources.gresource.xml',
  output: 'resources.h',
  command: [
    gen_resources,
    '--pixdata=@0@'.format(gdk_pixbuf_pixdata.full_path()),
    '--loaders=@0@'.format(loaders_cache.full_path()),
    '--sourcedir=@0@'.format(meson.current_source_dir()),
    '--header',
    '@INPUT@',
    '@OUTPUT@',
  ],
  depends: [
    gdk_pixbuf_pixdata,
    loaders_cache,
  ],
)

# [ test name, [ test suites ], needs_resources (opt) ]
#
# test suites:
#  - conform: Behavior conformance test
#  - security: CVEs and the like
#  - slow: Needs special timeout value
#  - format: Per-format test
#  - io: Loading/saving
#  - ops: Pixel operations
installed_tests = [
  [ 'animation', ['format'], ],
  [ 'cve-2015-4491', ['security'], true ],
  [ 'pixbuf-fail', ['conform', 'slow'], ],
  [ 'pixbuf-icon-serialize', ['conform'], ],
  [ 'pixbuf-randomly-modified', ['slow'], ],
  [ 'pixbuf-threads', ['io'], ],
  [ 'pixbuf-gif', ['io'], ],
  [ 'pixbuf-icc', ['io'], ],
  [ 'pixbuf-jpeg', ['format'], ],
  [ 'pixbuf-dpi', ['io'], ],
  [ 'pixbuf-pixdata', ['format'], true ],
  [ 'pixbuf-stream', ['io'], ],
  [ 'pixbuf-reftest', ['conform'], ],
  [ 'pixbuf-resource', ['io'], true ],
  [ 'pixbuf-scale', ['ops'], ],
  [ 'pixbuf-scale-two-step', ['ops'], ],
  [ 'pixbuf-short-gif-write', ['format'], ],
  [ 'pixbuf-gif-circular-table', ['format'], ],
  [ 'pixbuf-save', ['io'] ],
  [ 'pixbuf-readonly-to-mutable', ['conform'], ],
  [ 'pixbuf-composite', ['ops'], ],
  [ 'pixbuf-area-updated', ['slow'], ],
]

test_data = [
  'test-image.png',
  'test-animation.gif',
  '1_partyanimsm2.gif',
  'test-animation.ani',
  'icc-profile.jpeg',
  'icc-profile.png',
  'dpi.jpeg',
  'dpi.png',
  'dpi.tiff',
  'premature-end.png',
  'premature-end.jpg',
  'bug143608-comment.jpg',
  'bug725582-testrotate.jpg',
  'bug725582-testrotate.png',
  'bug753605-atsize.jpg',
  'cve-2015-4491.bmp',
  'large.png',
  'large.jpg',
  'bug775218.jpg',
  'test-image.pixdata',
  'test-image-rle.pixdata',
  'bug775693.pixdata',
  'bug775229.pixdata',
  'aero.gif',
]

installed_test_bindir = join_paths(gdk_pixbuf_libexecdir, 'installed-tests', meson.project_name())
installed_test_datadir = join_paths(gdk_pixbuf_datadir, 'installed-tests', meson.project_name())

if get_option('installed_tests')
  install_data(test_data, install_dir: installed_test_bindir)
  install_subdir('test-images', install_dir: installed_test_bindir)
endif

test_deps = gdk_pixbuf_deps + [ gdkpixbuf_dep ]

foreach t: installed_tests
  test_name = t[0]
  test_sources = [ test_name + '.c', 'test-common.c' ]
  test_suites = t[1]
  needs_resources = t.get(2, false)
  if needs_resources
    test_sources += [ resources_c, resources_h ]
  endif

  custom_target(test_name + '.test',
    output: test_name + '.test',
    command: [
      gen_installed_test,
      '--testbindir=@0@'.format(installed_test_bindir),
      '--testbin=@0@'.format(test_name),
      '@OUTPUT@',
    ],
    install: get_option('installed_tests'),
    install_dir: installed_test_datadir,
  )

  test_bin = executable(test_name, test_sources,
    dependencies: test_deps,
    include_directories: [
      root_inc,
      gdk_pixbuf_inc,
    ],
    c_args: common_cflags,
    install: get_option('installed_tests'),
    install_dir: installed_test_bindir,
  )

  # Two particularly slow tests
  if test_suites.contains('slow')
    timeout = 300
  else
    timeout = 30
  endif

  test(test_name, test_bin,
    suite: test_suites,
    args: [ '-k', '--tap' ],
    env: [
      'G_TEST_SRCDIR=@0@'.format(meson.current_source_dir()),
      'G_TEST_BUILDDIR=@0@'.format(meson.current_build_dir()),
      'GDK_PIXBUF_MODULE_FILE=@0@'.format(loaders_cache.full_path()),
    ],
    timeout: timeout,
  )
endforeach

executable('pixbuf-read',
  'pixbuf-read.c',
  dependencies: test_deps,
  include_directories: [
    root_inc,
    include_directories('../gdk-pixbuf')
  ],
  c_args: common_cflags,
)
