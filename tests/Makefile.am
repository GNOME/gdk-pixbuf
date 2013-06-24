include $(top_srcdir)/glib.mk

INCLUDES =				\
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
	$(NULL)

test_programs = 			\
	animation			\
	pixbuf-icon-serialize 		\
	pixbuf-randomly-modified	\
	pixbuf-threads			\
	$(NULL)

dist_test_data = 			\
	test-image.png			\
	test-animation.gif		\
	test-animation.ani		\
	$(wildcard $(srcdir)/test-images/*)

-include $(top_srcdir)/git.mk
