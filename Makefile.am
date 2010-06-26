include $(top_srcdir)/Makefile.decl

SUBDIRS = gdk-pixbuf po docs

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = gdk-pixbuf-2.0.pc

ACLOCAL_AMFLAGS = -I m4

DISTCLEANFILES = 		\
	gdk-pixbuf-2.0.pc

DISTCHECK_CONFIGURE_FLAGS = 	\
	--enable-gtk-doc	\
	--enable-man		\
	--enable-introspection

-include $(top_srcdir)/git.mk
