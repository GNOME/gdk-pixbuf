GENERATED_ITEMS = \
	gdk-pixbuf.vcxproj	\
	gdk-pixbuf.vcxproj.filters	\
	gdk-pixbuf-install.props

EXTRA_DIST = \
	README.txt	\
	gdk-pixbuf.sln	\
	gdk-pixbuf-version-paths.props	\
	gdk-pixbuf-build-defines.props	\
	gdk-pixbuf-gen-srcs.props	\
	gdk-pixbuf-install.propsin	\
	gdk-pixbuf.vcxprojin	\
	gdk-pixbuf.vcxproj.filtersin	\
	gdk-pixbuf-csource.vcxproj	\
	gdk-pixbuf-csource.vcxproj.filters	\
	gdk-pixbuf-query-loaders.vcxproj	\
	gdk-pixbuf-query-loaders.vcxproj.filters	\
	gdk-pixbuf-pixdata.vcxproj	\
	gdk-pixbuf-pixdata.vcxproj.filters	\
	gdk-pixbuf-install.vcxproj	\
	$(GENERATED_ITEMS)

gdk-pixbuf-install.props: $(top_srcdir)/build/win32/vs10/gdk-pixbuf-install.propsin gdk-pixbuf.vs10.headers
	$(CPP) -P - <$(top_srcdir)/build/win32/vs10/gdk-pixbuf-install.propsin >$@
	rm gdk-pixbuf.vs10.headers

DISTCLEANFILES = $(GENERATED_ITEMS)

-include $(top_srcdir)/git.mk
