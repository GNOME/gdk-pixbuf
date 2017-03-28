include $(top_srcdir)/glib-tap.mk

.NOTPARALLEL:

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

noinst_PROGRAMS += 			\
	pixbuf-read			\
	pixbuf-random			\
	pixbuf-lowmem			\
	pixbuf-save-ref			\
	$(NULL)

test_programs = 			\
	animation			\
	cve-2015-4491			\
	pixbuf-fail			\
	pixbuf-icon-serialize 		\
	pixbuf-randomly-modified	\
	pixbuf-threads			\
	pixbuf-icc			\
	pixbuf-jpeg			\
	pixbuf-dpi			\
	pixbuf-pixdata			\
	pixbuf-stream			\
	pixbuf-reftest			\
	pixbuf-resource			\
	pixbuf-scale			\
	pixbuf-scale-two-step		\
	pixbuf-short-gif-write		\
	pixbuf-save			\
	pixbuf-readonly-to-mutable	\
	pixbuf-composite		\
	pixbuf-area-updated		\
	$(NULL)

dist_installed_test_data =		\
	test-image.png			\
	test-animation.gif		\
	1_partyanimsm2.gif		\
	test-animation.ani		\
	icc-profile.jpeg		\
	icc-profile.png			\
	dpi.jpeg			\
	dpi.png				\
	dpi.tiff			\
	premature-end.png		\
	premature-end.jpg		\
	bug143608-comment.jpg		\
	bug725582-testrotate.jpg	\
	bug725582-testrotate.png	\
	cve-2015-4491.bmp		\
	large.png			\
	large.jpg			\
	bug775218.jpg			\
	test-image.pixdata		\
	test-image-rle.pixdata		\
	bug775693.pixdata		\
	aero.gif			\
	$(wildcard $(srcdir)/test-images/fail/*) \
	$(wildcard $(srcdir)/test-images/randomly-modified/*) \
	$(wildcard $(srcdir)/test-images/reftests/*.*) \
	$(wildcard $(srcdir)/test-images/reftests/tga/*) \
	$(NULL)

pixbuf_area_updated_SOURCES =		\
	pixbuf-area-updated.c		\
	test-common.c			\
	test-common.h			\
	$(NULL)

pixbuf_randomly_modified_SOURCES =	\
	pixbuf-randomly-modified.c	\
	test-common.c			\
	test-common.h			\
	$(NULL)

cve_2015_4491_SOURCES =			\
	cve-2015-4491.c			\
	test-common.c			\
	test-common.h			\
	resources.h			\
	resources.c			\
	$(NULL)

pixbuf_icc_SOURCES = 			\
	pixbuf-icc.c			\
	test-common.c			\
	test-common.h			\
	$(NULL)

pixbuf_dpi_SOURCES = 			\
	pixbuf-dpi.c			\
	test-common.c			\
	test-common.h			\
	$(NULL)

pixbuf_fail_SOURCES =			\
	pixbuf-fail.c			\
	test-common.c			\
	test-common.h			\
	$(NULL)

pixbuf_scale_SOURCES = 			\
	pixbuf-scale.c			\
	test-common.c			\
	test-common.h			\
	$(NULL)

pixbuf_scale_two_step_SOURCES =		\
	pixbuf-scale-two-step.c		\
	test-common.c			\
	test-common.h			\
	$(NULL)

pixbuf_stream_SOURCES =			\
	pixbuf-stream.c			\
	test-common.c			\
	test-common.h			\
	$(NULL)

pixbuf_threads_SOURCES =		\
	pixbuf-threads.c		\
	test-common.c			\
	test-common.h			\
	$(NULL)

pixbuf_icon_serialize_SOURCES =		\
	pixbuf-icon-serialize.c		\
	test-common.c			\
	test-common.h			\
	$(NULL)

pixbuf_save_SOURCES =			\
	pixbuf-save.c			\
	test-common.c			\
	test-common.h			\
	$(NULL)

pixbuf_reftest_SOURCES =		\
	pixbuf-reftest.c		\
	test-common.c			\
	test-common.h			\
	$(NULL)

pixbuf_jpeg_SOURCES =			\
	pixbuf-jpeg.c			\
	test-common.c			\
	test-common.h			\
	$(NULL)

pixbuf_short_gif_write_SOURCES =	\
	pixbuf-short-gif-write.c

pixbuf_readonly_to_mutable_SOURCES =	\
	pixbuf-readonly-to-mutable.c	\
	test-common.c			\
	test-common.h			\
	$(NULL)

pixbuf_composite_SOURCES =		\
	pixbuf-composite.c		\
	test-common.c			\
	test-common.h			\
	$(NULL)

pixbuf_resource_SOURCES = 		\
	pixbuf-resource.c		\
	test-common.c			\
	test-common.h			\
	resources.h			\
	resources.c			\
	$(NULL)

pixbuf_pixdata_SOURCES =		\
	pixbuf-pixdata.c		\
	test-common.c			\
	test-common.h			\
	resources.h			\
	resources.c			\
	$(NULL)

BUILT_SOURCES += resources.h resources.c
DISTCLEANFILES += 			\
	resources.h resources.c 	\
	pixbuf-save-options		\
	pixbuf-randomly-modified-image	\
	pixbuf-save-roundtrip		\
	$(NULL)
	
EXTRA_DIST += resources.gresource.xml
TESTS_ENVIRONMENT += GDK_PIXBUF_MODULE_FILE=$(top_builddir)/gdk-pixbuf/loaders.cache

resources.h: resources.gresource.xml
	$(AM_V_GEN) GDK_PIXBUF_MODULE_FILE=$(top_builddir)/gdk-pixbuf/loaders.cache GDK_PIXBUF_PIXDATA=$(top_builddir)/gdk-pixbuf/gdk-pixbuf-pixdata $(GLIB_COMPILE_RESOURCES) $(srcdir)/resources.gresource.xml --target=$@ --sourcedir=$(srcdir) --generate-header

resources.c: resources.gresource.xml
	$(AM_V_GEN) GDK_PIXBUF_MODULE_FILE=$(top_builddir)/gdk-pixbuf/loaders.cache GDK_PIXBUF_PIXDATA=$(top_builddir)/gdk-pixbuf/gdk-pixbuf-pixdata $(GLIB_COMPILE_RESOURCES) $(srcdir)/resources.gresource.xml --target=$@ --sourcedir=$(srcdir) --generate-source

-include $(top_srcdir)/git.mk
