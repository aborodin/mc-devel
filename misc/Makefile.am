## Process this file with automake to create Makefile.in.

SUBDIRS = skins

noinst_DATA = xterm.ad

pkgdata_DATA = \
	mc.lib
if CHARSET
pkgdata_DATA += mc.charsets
endif

LIBFILES_CONST = \
	edit.indent.rc \
	edit.spell.rc \
	filehighlight.ini \
	mc.keymap \
	mc.keymap.default \
	mc.keymap.emacs

CLEANFILES = $(SCRIPTS_OUT)

cfgdir = $(sysconfdir)/@PACKAGE@
cfg_DATA = \
	$(LIBFILES_CONST)

# Files processed by configure don't need to be here
EXTRA_DIST = \
	$(LIBFILES_CONST) \
	$(SCRIPTS_IN) \
	$(noinst_DATA) \
	$(pkgdata_DATA)
#if CHARSET is defined, mc.charsets is already here
if !CHARSET
EXTRA_DIST += mc.charsets
endif
