gidocgen_dep = dependency('gi-docgen',
  version: '>= 2021.1',
  fallback: ['gi-docgen', 'dummy_dep'],
  required: get_option('gtk_doc'),
)


toml_conf = configuration_data()
toml_conf.set('VERSION', meson.project_version())
toml = configure_file(input: 'gdk-pixbuf.toml.in', output: 'gdk-pixbuf.toml', configuration: toml_conf)

gidocgen = find_program('gi-docgen', required: get_option('gtk_doc'))

docs_dir = gdk_pixbuf_datadir / 'doc/gdk-pixbuf/reference'

expand_content_md_files = [
]

build_docs = get_option('gtk_doc')
if get_option('docs')
  warning('The docs option is deprecated; use -Dgtk_doc=true')
  build_docs = true
endif

if build_docs
  custom_target('gdk-pixbuf-doc',
    input: [ toml, gdkpixbuf_gir[0] ],
    output: 'gdk-pixbuf',
    command: [
      gidocgen,
      'generate',
      '--quiet',
      '--add-include-path=@0@'.format(meson.current_build_dir() / '../gdk-pixbuf'),
      '--config=@INPUT0@',
      '--output-dir=@OUTPUT@',
      '--no-namespace-dir',
      '--content-dir=@0@'.format(meson.current_source_dir()),
      '@INPUT1@',
    ],
    depend_files: [ expand_content_md_files ],
    build_by_default: true,
    install: true,
    install_dir: docs_dir,
  )
endif

xsltproc = find_program('xsltproc', required: false)
if get_option('man') and xsltproc.found()
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
