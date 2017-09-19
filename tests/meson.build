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
                            ])
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
                            ])

installed_tests = [
  [ 'animation' ],
  [ 'cve-2015-4491', true ],
  [ 'pixbuf-fail' ],
  [ 'pixbuf-icon-serialize' ],
  [ 'pixbuf-randomly-modified' ],
  [ 'pixbuf-threads' ],
  [ 'pixbuf-icc' ],
  [ 'pixbuf-jpeg' ],
  [ 'pixbuf-dpi' ],
  [ 'pixbuf-pixdata', true ],
  [ 'pixbuf-stream' ],
  [ 'pixbuf-reftest' ],
  [ 'pixbuf-resource', true ],
  [ 'pixbuf-scale' ],
  [ 'pixbuf-scale-two-step' ],
  [ 'pixbuf-short-gif-write' ],
  [ 'pixbuf-save' ],
  [ 'pixbuf-readonly-to-mutable' ],
  [ 'pixbuf-composite' ],
  [ 'pixbuf-area-updated' ],
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
  'cve-2015-4491.bmp',
  'large.png',
  'large.jpg',
  'bug775218.jpg',
  'test-image.pixdata',
  'test-image-rle.pixdata',
  'bug775693.pixdata',
  'aero.gif',
]

installed_test_bindir = join_paths(gdk_pixbuf_libexecdir, 'installed-tests', gdk_pixbuf_api_name)
installed_test_datadir = join_paths(gdk_pixbuf_datadir, 'installed-tests', gdk_pixbuf_api_name)

install_data(test_data, install_dir: installed_test_bindir)

test_deps = gdk_pixbuf_deps + [ gdkpixbuf_dep ]

foreach t: installed_tests
  test_name = t[0]
  test_sources = [ test_name + '.c', 'test-common.c' ]
  needs_resources = t.get(1, false)
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
                install: true,
                install_dir: installed_test_datadir)

  test_bin = executable(test_name, test_sources,
                        dependencies: test_deps,
                        include_directories: [
                          root_inc,
                          gdk_pixbuf_inc,
                        ],
                        c_args: common_cflags)

  # Two particularly slow tests
  if test_name == 'pixbuf-area-updated' or test_name == 'pixbuf-randomly-modified'
    timeout = 300
  else
    timeout = 30
  endif

  test(test_name, test_bin,
       args: [ '-k', '--tap' ],
       env: [
         'G_TEST_SRCDIR=@0@'.format(meson.current_source_dir()),
         'G_TEST_BUILDDIR=@0@'.format(meson.current_build_dir()),
         'GDK_PIXBUF_MODULE_FILE=@0@'.format(loaders_cache.full_path()),
       ],
       timeout: timeout)
endforeach

executable('pixbuf-read',
           'pixbuf-read.c',
           dependencies: test_deps,
           include_directories: [
             root_inc,
             include_directories('../gdk-pixbuf')
           ],
           c_args: common_cflags)
