# Resources; we cannot use gnome.compile_resources() here, because we need to
# override the environment in order to use the utilities we just built instead
# of the system ones
gen_resources = find_program('gen-resources.py')
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

test_deps = gdk_pixbuf_deps + [ gdkpixbuf_dep ]
foreach t: installed_tests
  test_name = t[0]
  test_sources = [ test_name + '.c', 'test-common.c' ]
  needs_resources = t.get(1, false)
  if needs_resources
    test_sources += [ resources_c, resources_h ]
  endif

  test_bin = executable(test_name, test_sources,
                        dependencies: test_deps,
                        include_directories: [ root_inc, include_directories('../gdk-pixbuf') ],
                        c_args: common_cflags)

  test(test_name, test_bin,
       args: [ '-k', '--tap' ],
       env: [
         'G_TEST_SRCDIR=@0@'.format(meson.current_source_dir()),
         'G_TEST_BUILDDIR=@0@'.format(meson.current_build_dir()),
         'GDK_PIXBUF_MODULE_FILE=@0@'.format(loaders_cache.full_path()),
       ])
endforeach
