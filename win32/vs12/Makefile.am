EXTRA_DIST = \
	README.txt      \
	gdk-pixbuf.sln  \
	gdk-pixbuf-version-paths.props  \
	gdk-pixbuf-build-defines.props  \
	gdk-pixbuf-gen-srcs.props	\
	gdk-pixbuf-install.props	 \
	gdk-pixbuf.vcxproj      \
	gdk-pixbuf.vcxproj.filters      \
	gdk-pixbuf-csource.vcxproj      \
	gdk-pixbuf-csource.vcxproj.filters      \
	gdk-pixbuf-query-loaders.vcxproj	 \
	gdk-pixbuf-query-loaders.vcxproj.filters	 \
	gdk-pixbuf-pixdata.vcxproj      \
	gdk-pixbuf-pixdata.vcxproj.filters      \
	gdk-pixbuf-install.vcxproj		\
	gdk-pixbuf-install.vcxproj.filters	\
	gdk-pixbuf-introspect.vcxproj

DISTCLEANFILES = $(EXTRA_DIST)

MSVC_BASE_VER = 10
MSVC_BASE_VER_LONG = 2010
MSVC_VER = 12
MSVC_VER_LONG = 2013

include $(top_srcdir)/win32/Makefile-newvs.am

-include $(top_srcdir)/git.mk
