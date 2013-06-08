include $(top_srcdir)/Makefile.decl

NULL =

INCLUDES =				\
	-I$(top_srcdir)			\
	-I$(top_srcdir)/gdk-pixbuf	\
	$(GDK_PIXBUF_DEBUG_FLAGS)	\
	$(GDK_PIXBUF_DEP_CFLAGS)

AM_CFLAGS = '-DABS_SRCDIR="$(abs_srcdir)"'

DEPS = \
	$(top_builddir)/gdk-pixbuf/libgdk_pixbuf-$(GDK_PIXBUF_API_VERSION).la

LDADDS = \
	$(top_builddir)/gdk-pixbuf/libgdk_pixbuf-$(GDK_PIXBUF_API_VERSION).la \
	$(GDK_PIXBUF_DEP_LIBS)

noinst_PROGRAMS = \
	animation			\
	pixbuf-read			\
	pixbuf-lowmem			\
	pixbuf-randomly-modified	\
	pixbuf-random			\
	pixbuf-threads			\
	pixbuf-icon-serialize

animation_LDADD = $(LDADDS)
pixbuf_read_LDADD = $(LDADDS)
pixbuf_lowmem_LDADD = $(LDADDS)
pixbuf_randomly_modified_LDADD = $(LDADDS)
pixbuf_random_LDADD = $(LDADDS)
pixbuf_threads_LDADD = $(LDADDS) $(GLIB_LIBS)
pixbuf_icon_serialize_LDADD = $(LDADDS) $(GLIB_LIBS)

if BUILDOPT_INSTALL_TESTS
insttestdir = $(pkglibexecdir)/installed-tests
insttest_PROGRAMS = 			\
	animation			\
	pixbuf-icon-serialize 		\
	pixbuf-randomly-modified	\
	pixbuf-threads			\
	$(NULL)

insttest_DATA = 			\
	test-image.png			\
	test-animation.gif		\
	test-animation.ani		\
	$(NULL)

testimagedir = $(insttestdir)/test-images
testimage_DATA = $(wildcard $(srcdir)/test-images/*)

substitutions = \
	-e s,@pkglibexecdir\@,$(pkglibexecdir),g \
	$(NULL)

test_in_files = $(insttest_PROGRAMS:=.test.in)
test_files = $(test_in_files:.test.in=.test)

$(test_files): %.test: %.test.in
	$(AM_V_GEN) sed $(substitutions) $< > $@.tmp && mv $@.tmp $@

EXTRA_DIST += $(test_in_files)

testmetadir = $(datadir)/installed-tests/$(PACKAGE)
testmeta_DATA = $(test_files)

endif

-include $(top_srcdir)/git.mk
