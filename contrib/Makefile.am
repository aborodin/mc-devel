
noinst_DATA = README.xterm

SCRIPTS_IN = mc.csh.in mc.sh.in mc-wrapper.csh.in mc-wrapper.sh.in
SCRIPTS_OUT = mc.csh mc.sh mc-wrapper.csh mc-wrapper.sh

pkglibexec_SCRIPTS = $(SCRIPTS_OUT)

cfgdir = $(sysconfdir)/@PACKAGE@
cfg_DATA = \
	$(LIBFILES_ADD)

CLEANFILES = $(SCRIPTS_OUT)

# Files processed by configure don't need to be here
EXTRA_DIST = \
	$(SCRIPTS_IN) \
	$(noinst_DATA)

mc.csh: $(top_builddir)/config.status $(srcdir)/mc.csh.in
	$(SED) "s%@""pkglibexecdir@%$(pkglibexecdir)%" $(srcdir)/mc.csh.in > mc.csh

mc.sh: $(top_builddir)/config.status $(srcdir)/mc.sh.in
	$(SED) "s%@""pkglibexecdir@%$(pkglibexecdir)%" $(srcdir)/mc.sh.in > mc.sh

mc-wrapper.csh: $(top_builddir)/config.status $(srcdir)/mc-wrapper.csh.in
	$(SED) "s%@""bindir@%$(bindir)%" $(srcdir)/mc-wrapper.csh.in > mc-wrapper.csh

mc-wrapper.sh: $(top_builddir)/config.status $(srcdir)/mc-wrapper.sh.in
	$(SED) "s%@""bindir@%$(bindir)%" $(srcdir)/mc-wrapper.sh.in > mc-wrapper.sh

