
vmoddir = $(VMOD_DIR)
vmod_LTLIBRARIES = libvmod_leveldb.la

libvmod_leveldb_la_CFLAGS = $(VMOD_INCLUDES)
libvmod_leveldb_la_LDFLAGS = -module -export-dynamic -avoid-version -shared \
	$(LIBS)

libvmod_leveldb_la_SOURCES = \
	vmod_leveldb.c
nodist_libvmod_leveldb_la_SOURCES = \
	vcc_if.c \
	vcc_if.h

dist_man_MANS = vmod_leveldb.3

VMOD_TESTS = tests/*.vtc

$(libvmod_leveldb_la_OBJECTS): vcc_if.h

vcc_if.c vcc_if.h vmod_leveldb.man.rst: $(VMODTOOL) $(top_srcdir)/src/vmod.vcc
	$(PYTHON) $(VMODTOOL) $(top_srcdir)/src/vmod.vcc

vmod_leveldb.3: vmod_leveldb.man.rst
	${RST2MAN} $? $@

check:
	$(VARNISHTEST) -Dvarnishd=$(VARNISHD) \
	    -Dvmod_topbuild=$(abs_top_builddir) $(VMOD_TESTS)

EXTRA_DIST = vmod.vcc \
	$(VMOD_TESTS)

CLEANFILES = $(builddir)/vcc_if.c \
	$(builddir)/vcc_if.h \
	$(builddir)/vmod_leveldb.rst \
	$(builddir)/vmod_leveldb.man.rst
