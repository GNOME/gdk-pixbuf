installed_tests = [
  [ 'animation' ],
#  [ 'cve-2015-4491' ], # FIXME: Needs resources
  [ 'pixbuf-fail' ],
  [ 'pixbuf-icon-serialize' ],
  [ 'pixbuf-randomly-modified' ],
  [ 'pixbuf-threads' ],
  [ 'pixbuf-icc' ],
  [ 'pixbuf-jpeg' ],
  [ 'pixbuf-dpi' ],
#  [ 'pixbuf-pixdata' ], # FIXME: Needs resources
  [ 'pixbuf-stream' ],
  [ 'pixbuf-reftest' ],
#  [ 'pixbuf-resource' ], # FIXME: Needs resources
  [ 'pixbuf-scale' ],
  [ 'pixbuf-scale-two-step' ],
  [ 'pixbuf-short-gif-write' ],
  [ 'pixbuf-save' ],
  [ 'pixbuf-readonly-to-mutable' ],
  [ 'pixbuf-composite' ],
  [ 'pixbuf-area-updated' ],
]

foreach t: installed_tests
  test_name = t[0]
  test_sources = [ test_name + '.c', 'test-common.c' ]

  test_bin = executable(test_name, test_sources,
                        dependencies: gdk_pixbuf_deps + [ gdkpixbuf_dep ],
                        include_directories: [ root_inc, include_directories('../gdk-pixbuf') ],
                        c_args: common_cflags)

  test(test_name, test_bin,
       args: [ '-k', '--tap' ],
       env: [
         'G_TEST_SRCDIR=@0@'.format(meson.current_source_dir()),
         'G_TEST_BUILDDIR=@0@'.format(meson.current_build_dir()),
         'GDK_PIXBUF_MODULE_FILE=@0@'.format(join_paths(meson.current_build_dir(), '../gdk-pixbuf/loaders.cache'))
       ])
endforeach
