SUBDIRS = filemanager man2hlp

AM_CPPFLAGS = \
	-DLOCALEDIR=\""$(localedir)"\" \
	-DLIBEXECDIR=\""$(libexecdir)/@PACKAGE@/"\"

AM_CFLAGS = $(GLIB_CFLAGS) -I$(top_srcdir) $(PCRE_CFLAGS)

localedir = $(datadir)/locale
pkglibexecdir = $(libexecdir)/@PACKAGE@

bin_PROGRAMS = mc

mc_LDADD = \
	filemanager/libmcfilemanager.la \
	$(top_builddir)/lib/libmc.la

mc_SOURCES = \
	args.c args.h \
	events_init.c events_init.h \
	help.c help.h \
	keybind-defaults.c keybind-defaults.h \
	main.c main.h \
	setup.c setup.h \
	textconf.c textconf.h

if CHARSET
    mc_SOURCES += selcodepage.c selcodepage.h
endif

EXTRA_DIST = $(SRC_maintainer) $(SRC_charset)
