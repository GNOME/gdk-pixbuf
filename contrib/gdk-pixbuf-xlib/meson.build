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
                           '-DSTDC_HEADERS',
                         ],
                         link_args: common_ldflags,
                         include_directories: [
                           root_inc,
                           include_directories('..')
                         ],
                         dependencies: gdk_pixbuf_deps + [ gdkpixbuf_dep, x11_dep ],
                         install: true)

pkgconfig.generate(
  name: 'GdkPixbuf Xlib',
  description: 'GdkPixbuf rendering for Xlib',
  version: meson.project_version(),
  libraries: gdkpixbuf_xlib,
  requires: 'gdk-pixbuf-2.0',
  filebase: 'gdk-pixbuf-xlib-2.0',
  subdirs: gdk_pixbuf_api_name,
  install_dir: join_paths(gdk_pixbuf_libdir, 'pkgconfig'),
)
