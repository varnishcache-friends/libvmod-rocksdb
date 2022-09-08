
TESTS_ENVIRONMENT = PATH=$(VARNISH_BINDIR):$(VARNISH_SBINDIR):$$PATH

vmoddir = $(VMOD_DIR)
vmod_LTLIBRARIES = libvmod_rocksdb.la

libvmod_rocksdb_la_CFLAGS = $(VMOD_INCLUDES) $(ROCKSDB_CFLAGS) \
	-DROCKSDB_VERSION=\"$(ROCKSDB_VERSION)\"
libvmod_rocksdb_la_LDFLAGS = -module -export-dynamic -avoid-version -shared \
	$(LIBS) $(ROCKSDB_LIBS)

libvmod_rocksdb_la_SOURCES = \
	vmod_rocksdb.c
nodist_libvmod_rocksdb_la_SOURCES = \
	vcc_if.c \
	vcc_if.h

dist_man_MANS = vmod_rocksdb.3

VTC_LOG_COMPILER = $(VARNISHTEST) -v \
	-Dvmod_topbuild=$(abs_top_builddir) -Dvmod_topsrc=$(abs_top_srcdir)
TEST_EXTENSIONS = .vtc
TESTS = @VMOD_TESTS@

$(libvmod_rocksdb_la_OBJECTS): vcc_if.h

vcc_if.h vmod_rocksdb.man.rst: vcc_if.c

vcc_if.c: $(VMODTOOL) $(top_srcdir)/src/vmod_rocksdb.vcc
	$(PYTHON) $(VMODTOOL) $(top_srcdir)/src/vmod_rocksdb.vcc

vmod_rocksdb.3: vmod_rocksdb.man.rst
	$(RST2MAN) $? $@

EXTRA_DIST = vmod_rocksdb.vcc \
	$(TESTS)

CLEANFILES = $(builddir)/vcc_if.c \
	$(builddir)/vcc_if.h \
	$(builddir)/vmod_rocksdb.man.rst \
	$(builddir)/vmod_rocksdb.rst \
	$(builddir)/$(dist_man_MANS)

if USE_LCOV
check-local:
	$(LCOV) -c -d .libs -o .libs/$(PACKAGE).info
	$(GENHTML) -o lcov .libs/$(PACKAGE).info
	$(GCOV) -o .libs/libvmod_rocksdb_la-vcc_if.gcno vmod_rocksdb.c

clean-local:
	rm -rf lcov
endif
