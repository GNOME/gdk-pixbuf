bin_PROGRAMS = gdk-pixbuf-thumbnailer
noinst_PROGRAMS = gdk-pixbuf-print-mime-types

gdk_pixbuf_thumbnailer_SOURCES = gdk-pixbuf-thumbnailer.c gnome-thumbnailer-skeleton.c gnome-thumbnailer-skeleton.h
gdk_pixbuf_thumbnailer_CPPFLAGS =			\
	-I$(top_srcdir)					\
	-I$(top_srcdir)/gdk-pixbuf			\
	$(GDK_PIXBUF_DEP_CFLAGS)			\
	-DTHUMBNAILER_RETURNS_PIXBUF			\
	-DTHUMBNAILER_USAGE="\"Thumbnail images\""	\
	$(WARN_CFLAGS)
gdk_pixbuf_thumbnailer_LDADD =				\
	$(GDK_PIXBUF_DEP_LIBS)				\
	$(top_builddir)/gdk-pixbuf/libgdk_pixbuf-$(GDK_PIXBUF_API_VERSION).la

gdk_pixbuf_print_mime_types_SOURCES = gdk-pixbuf-print-mime-types.c
gdk_pixbuf_print_mime_types_CPPFLAGS =			\
	-I$(top_srcdir)					\
	-I$(top_srcdir)/gdk-pixbuf			\
	$(GDK_PIXBUF_DEP_CFLAGS)			\
	$(WARN_CFLAGS)
gdk_pixbuf_print_mime_types_LDADD =			\
	$(GDK_PIXBUF_DEP_LIBS)				\
	$(top_builddir)/gdk-pixbuf/libgdk_pixbuf-$(GDK_PIXBUF_API_VERSION).la

thumbnailerdir = $(datadir)/thumbnailers/
thumbnailer_DATA = gdk-pixbuf-thumbnailer.thumbnailer
gdk-pixbuf-thumbnailer.thumbnailer: gdk-pixbuf-thumbnailer.thumbnailer.in Makefile gdk-pixbuf-print-mime-types$(EXEEXT) $(top_builddir)/gdk-pixbuf/loaders.cache
	$(AM_V_GEN) $(SED) -e "s|\@bindir\@|$(bindir)|"						\
	-e "s|\@mimetypes\@|`GDK_PIXBUF_MODULE_FILE=$(top_builddir)/gdk-pixbuf/loaders.cache GDK_PIXBUF_PIXDATA=$(top_builddir)/gdk-pixbuf/gdk-pixbuf-pixdata $(builddir)/gdk-pixbuf-print-mime-types`|" \
	$< > $@

EXTRA_DIST = gdk-pixbuf-thumbnailer.thumbnailer.in

CLEANFILES = $(thumbnailer_DATA)
