gdkpixbuf_xlib_sources = [
  'gdk-pixbuf-xlib.c',
  'gdk-pixbuf-xlib-render.c',
  'gdk-pixbuf-xlib-drawable.c',
  'gdk-pixbuf-xlibrgb.c',
]

gdkpixbuf_xlib_headers = [
  'gdk-pixbuf-xlib.h',
  'gdk-pixbuf-xlibrgb.h',
]

gdkpixbuf_xlib_inc = include_directories('.')

gdkpixbuf_xlib_api_path = join_paths('gdk-pixbuf-@0@'.format(gdk_pixbuf_api_version), 'gdk-pixbuf-xlib')

install_headers(gdkpixbuf_xlib_headers, subdir: gdkpixbuf_xlib_api_path)

gdkpixbuf_xlib = library('gdk_pixbuf_xlib-2.0',
                         gdkpixbuf_xlib_sources,
                         soversion: soversion,
                         version: libversion,
                         c_args: common_cflags + gdk_pixbuf_cflags + [
                           '-DHAVE_CONFIG_H',
                           '-DSTDC_HEADERS',
                         ],
                         link_args: common_ldflags,
                         include_directories: [
                           root_inc,
                           include_directories('..')
                         ],
                         dependencies: gdk_pixbuf_deps + [ gdkpixbuf_dep, x11_dep ],
                         install: true)

# Compat variables for pkgconfig
pkgconf = configuration_data()
pkgconf.set('prefix', gdk_pixbuf_prefix)
pkgconf.set('exec_prefix', gdk_pixbuf_prefix)
pkgconf.set('libdir', gdk_pixbuf_libdir)
pkgconf.set('includedir', gdk_pixbuf_includedir)
pkgconf.set('GDK_PIXBUF_API_VERSION', gdk_pixbuf_api_version)
pkgconf.set('VERSION', meson.project_version())

configure_file(input: 'gdk-pixbuf-xlib-2.0.pc.in',
               output: 'gdk-pixbuf-xlib-2.0.pc',
               configuration: pkgconf,
               install: true,
               install_dir: join_paths(gdk_pixbuf_libdir, 'pkgconfig'))
