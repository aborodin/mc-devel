
noinst_LTLIBRARIES = libmcfilemanager.la

libmcfilemanager_la_SOURCES = \
	midnight.h midnight.c

libmcfilemanager_la_CFLAGS = -I$(top_srcdir) \
	$(GLIB_CFLAGS) $(PCRE_CFLAGS) \
	-DDATADIR=\""$(pkgdatadir)/"\" -DLOCALEDIR=\""$(localedir)"\"

libmcfilemanager_la_LIBADD = ../../lib/libmc.la
