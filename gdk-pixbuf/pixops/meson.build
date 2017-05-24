pixops = static_library('pixops', 'pixops.c',
                        include_directories: root_inc,
                        dependencies: gdk_pixbuf_deps)

pixops_dep = declare_dependency(link_with: pixops,
                                include_directories: [ root_inc, include_directories('.') ],
                                dependencies: gdk_pixbuf_deps)

executable('timescale', 'timescale.c', dependencies: pixops_dep)
