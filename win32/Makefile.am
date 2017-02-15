if HAVE_INTROSPECTION
GENERATED_ITEMS = \
	introspection.body.mak	\
	GdkPixbuf_2_0_gir_list

introspection.body.mak: GdkPixbuf-2.0.gir.msvc.introspect
	-$(RM) introspection.body.mak
	for F in `ls *.msvc.introspect`; do \
		case $$F in \
			*)	cat $(top_builddir)/win32/$$F >>$@ \
			;; \
		esac; \
	done
	$(RM) GdkPixbuf-2.0.gir.msvc.introspect

DISTCLEANFILES = $(GENERATED_ITEMS)

else
GENERATED_ITEMS =
endif

SUBDIRS =	\
	vs9	\
	vs10	\
	vs11	\
	vs12	\
	vs14	\
	vs15

EXTRA_DIST =	\
	detectenv-msvc.mak			\
	introspection-msvc.mak			\
	gdk-pixbuf-introspection-msvc.mak	\
	gdkpixbuf-pc.py				\
	pc_base.py				\
	replace.py				\
	$(GENERATED_ITEMS)

-include $(top_srcdir)/git.mk
