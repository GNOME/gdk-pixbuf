include $(top_srcdir)/glib-tap.mk

AM_CPPFLAGS =				\
	-I$(top_srcdir)			\
	-I$(top_srcdir)/gdk-pixbuf	\
	$(GDK_PIXBUF_DEBUG_FLAGS)	\
	$(GDK_PIXBUF_DEP_CFLAGS)	\
	$(NULL)

DEPS = \
	$(top_builddir)/gdk-pixbuf/libgdk_pixbuf-$(GDK_PIXBUF_API_VERSION).la

LDADD = \
	$(top_builddir)/gdk-pixbuf/libgdk_pixbuf-$(GDK_PIXBUF_API_VERSION).la 	\
	$(GDK_PIXBUF_DEP_LIBS) \
	$(GLIB_LIBS) \
	$(NULL)

noinst_PROGRAMS = 			\
	animation			\
	pixbuf-read			\
	pixbuf-lowmem			\
	pixbuf-randomly-modified	\
	pixbuf-random			\
	pixbuf-threads			\
	pixbuf-icon-serialize		\
	pixbuf-icc			\
	pixbuf-stream			\
	pixbuf-resource			\
	pixbuf-scale			\
	pixbuf-save			\
	$(NULL)

test_programs = 			\
	animation			\
	pixbuf-icon-serialize 		\
	pixbuf-randomly-modified	\
	pixbuf-threads			\
	pixbuf-icc			\
	pixbuf-stream			\
	pixbuf-resource			\
	pixbuf-scale			\
	pixbuf-save			\
	$(NULL)

dist_test_data = 			\
	test-image.png			\
	test-animation.gif		\
	test-animation.ani		\
	icc-profile.jpeg		\
	icc-profile.png			\
	$(wildcard $(srcdir)/test-images/*)

pixbuf_resource_SOURCES = 		\
	pixbuf-resource.c		\
	resources.h			\
	resources.c

BUILT_SOURCES = resources.h resources.c
DISTCLEANFILES = resources.h resources.c
EXTRADIST = resources.gresource.xml

resources.h: resources.gresource.xml
	$(AM_V_GEN) GDK_PIXBUF_MODULE_FILE=$(top_builddir)/gdk-pixbuf/loaders.cache GDK_PIXBUF_PIXDATA=$(top_builddir)/gdk-pixbuf/gdk-pixbuf-pixdata $(GLIB_COMPILE_RESOURCES) $(srcdir)/resources.gresource.xml --target=$@ --sourcedir=$(srcdir) --generate-header

resources.c: resources.gresource.xml
	$(AM_V_GEN) GDK_PIXBUF_MODULE_FILE=$(top_builddir)/gdk-pixbuf/loaders.cache GDK_PIXBUF_PIXDATA=$(top_builddir)/gdk-pixbuf/gdk-pixbuf-pixdata $(GLIB_COMPILE_RESOURCES) $(srcdir)/resources.gresource.xml --target=$@ --sourcedir=$(srcdir) --generate-source

-include $(top_srcdir)/git.mk
