version_xml = configuration_data()
version_xml.set('GDK_PIXBUF_VERSION', meson.project_version())
configure_file(input: 'version.xml.in',
               output: 'version.xml',
               configuration: version_xml)

glib_prefix = dependency('glib-2.0').get_pkgconfig_variable('prefix')
glib_docpath = join_paths(glib_prefix, 'share', 'gtk-doc', 'html')
docpath = join_paths(gdk_pixbuf_datadir, 'gtk-doc', 'html')

private_headers = [
  'pixops',
  'gdk-pixbuf-alias.h',
  'gdk-pixbuf-marshal.h',
  'gdk-pixbuf-private.h',
  'gdk-pixbuf-xlib-private.h',
  'io-gif-animation.h',
  'io-ani-animation.h',
  'xpm-color-table.h',
  'test-images.h',
]

if get_option('with_docs')
  # This should not be needed, but gnome.gtkdoc() does not copy the
  # .types file into the builddir, if one is found
  configure_file(input: 'gdk-pixbuf.types',
                 output: 'gdk-pixbuf.types',
                 configuration: configuration_data())

  gnome.gtkdoc('gdk-pixbuf',
               main_xml: 'gdk-pixbuf.xml',
               src_dir: [
                 gdk_pixbuf_inc,
                 gdkpixbuf_xlib_inc,
               ],
               dependencies: gdkpixbuf_dep,
               gobject_typesfile: 'gdk-pixbuf.types',
               scan_args: [
                 '--deprecated-guards="GDK_PIXBUF_ENABLE_BROKEN|GDK_PIXBUF_DISABLE_DEPRECATED"',
                 '--ignore-headers=' + ' '.join(private_headers),
               ],
               fixxref_args: [
                 '--html-dir=@0@'.format(docpath),
                 '--extra-dir=@0@'.format(join_paths(glib_docpath, 'glib')),
                 '--extra-dir=@0@'.format(join_paths(glib_docpath, 'gobject')),
                 '--extra-dir=@0@'.format(join_paths(glib_docpath, 'gio')),
               ],
               html_images: [
                 'composite.png',
               ],
               content_files: [
                 'gdk-pixbuf-from-drawables.xml',
                 'gdk-pixbuf-rendering.xml',
                 'gdk-pixbuf.xml',
                 'gdk-pixbuf-csource.xml',
                 'gdk-pixbuf-query-loaders.xml',
               ],
               install: true)
endif

xsltproc = find_program('xsltproc', required: false)
if get_option('with_man') and xsltproc.found()
  xlstproc_flags = [
    '--nonet',
    '--stringparam', 'man.output.quietly', '1',
    '--stringparam', 'funcsynopsis.style', 'ansi',
    '--stringparam', 'man.th.extra1.suppress', '1',
    '--stringparam', 'man.authors.section.enabled', '0',
    '--stringparam', 'man.copyright.section.enabled', '0',
  ]

  man_files = [
    'gdk-pixbuf-csource',
    'gdk-pixbuf-query-loaders',
  ]

  foreach m: man_files
    custom_target(m + '-man',
                  input: '@0@.xml'.format(m),
                  output: '@0@.1'.format(m),
                  command: [
                    xsltproc,
                    xlstproc_flags,
                    '-o', '@OUTPUT@',
                    'http://docbook.sourceforge.net/release/xsl/current/manpages/docbook.xsl',
                    '@INPUT@',
                  ],
                  install: true,
                  install_dir: join_paths(gdk_pixbuf_mandir, 'man1'))
  endforeach
endif
