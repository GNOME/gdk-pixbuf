GENERATED_ITEMS = \
	gdk-pixbuf.vcproj	\
	gdk-pixbuf-install.vsprops

EXTRA_DIST =	\
	README.txt	\
	gdk-pixbuf.sln	\
	gdk-pixbuf-version-paths.vsprops	\
	gdk-pixbuf-build-defines.vsprops	\
	gdk-pixbuf-gen-srcs.vsprops	\
	gdk-pixbuf-install.vspropsin	\
	gdk-pixbuf.vcprojin	\
	gdk-pixbuf-csource.vcproj	\
	gdk-pixbuf-query-loaders.vcproj	\
	gdk-pixbuf-pixdata.vcproj	\
	gdk-pixbuf-install.vcproj	\
	gdk-pixbuf-introspect.vcproj	\
	$(GENERATED_ITEMS)

gdk-pixbuf-install.vsprops: $(top_srcdir)/win32/vs9/gdk-pixbuf-install.vspropsin gdk-pixbuf.headers
	$(CPP) -P - <$(top_srcdir)/win32/vs9/gdk-pixbuf-install.vspropsin >$@
	rm gdk-pixbuf.headers

DISTCLEANFILES = $(GENERATED_ITEMS)

-include $(top_srcdir)/git.mk
