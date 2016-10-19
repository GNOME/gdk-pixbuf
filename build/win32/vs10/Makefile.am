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
	gdk-pixbuf-install.vcxproj.filters	\
	$(GENERATED_ITEMS)		\
	math.h

gdk-pixbuf-install.props: $(top_srcdir)/build/win32/vs10/gdk-pixbuf-install.propsin gdk-pixbuf.vs10.headers
	-$(RM) $(top_builddir)/build/win32/vs11/gdk-pixbuf-install.props
	-$(RM) $(top_builddir)/build/win32/vs12/gdk-pixbuf-install.props
	-$(RM) $(top_builddir)/build/win32/vs14/gdk-pixbuf-install.props
	$(CPP) -P - <$(top_srcdir)/build/win32/vs10/gdk-pixbuf-install.propsin >$@
	rm gdk-pixbuf.vs10.headers

math.h: $(top_srcdir)/build/win32/vs9/math.h
	cp $< $@

DISTCLEANFILES = $(GENERATED_ITEMS) math.h

-include $(top_srcdir)/git.mk
