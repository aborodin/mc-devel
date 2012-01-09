SUBDIRS = filemanager man2hlp

AM_CPPFLAGS = \
	-DLOCALEDIR=\""$(localedir)"\" \
	-DLIBEXECDIR=\""$(libexecdir)/@PACKAGE@/"\"

if CONS_SAVER
SUBDIRS += consaver
AM_CPPFLAGS += -DSAVERDIR=\""$(pkglibexecdir)"\"
endif

AM_CFLAGS = $(GLIB_CFLAGS) -I$(top_srcdir) $(PCRE_CFLAGS)

localedir = $(datadir)/locale
pkglibexecdir = $(libexecdir)/@PACKAGE@

bin_PROGRAMS = mc

mc_LDADD = \
	filemanager/libmcfilemanager.la \
	$(top_builddir)/lib/libmc.la

SRC_mc_conssaver = \
	cons.handler.c consaver/cons.saver.h

mc_SOURCES = \
	$(SRC_mc_conssaver) \
	args.c args.h \
	clipboard.c clipboard.h \
	events_init.c events_init.h \
	execute.c execute.h \
	help.c help.h \
	history.h \
	keybind-defaults.c keybind-defaults.h \
	learn.c learn.h \
	main.c main.h \
	setup.c setup.h \
	subshell.c subshell.h \
	textconf.c textconf.h \
	util.c util.h

if CHARSET
    mc_SOURCES += selcodepage.c selcodepage.h
endif

EXTRA_DIST = $(SRC_maintainer) $(SRC_charset)
