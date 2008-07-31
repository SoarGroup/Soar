
if INCLUDE_LIBPLAYERC_PY

noinst_DATA = pythonbuild
PYTHONINSTALL = pythoninstall

# files that must be built first
BUILT_SOURCES = playerc_oo.i srcbuilddirs.py

endif

dist_bin_SCRIPTS = playerc_swig_parse.py

# files that go in distribution
EXTRA_DIST = playerc.i setup.py libplayerc_py.html playercpy_example.py

# Generate a OO interface file for libplayerc.  This is tweaked to
# enable VPAPTH (distcheck) builds; the header files needed by swig
# are copied into the build tree.
playerc_oo.i: ../../playerc.h $(top_srcdir)/libplayercore/player.h $(srcdir)/playerc_swig_parse.py
	if test ! -s ../../playerc.h; then cp $(srcdir)/../../playerc.h ../../; fi
	if test ! -s $(top_builddir)/libplayercore/player.h; then cp $(top_srcdir)/libplayercore/player.h $(top_builddir)/libplayercore/; fi
	if test ! -s $(top_builddir)/libplayercore/player_interfaces.h; then cp $(top_srcdir)/libplayercore/player_interfaces.h $(top_builddir)/libplayercore/; fi
	if test ! -s playerc.i; then cp $(srcdir)/playerc.i .; fi
	python $(srcdir)/playerc_swig_parse.py ../../playerc.h playerc_oo.i

# Dump src and build paths into a Python file so that we can get them in
# setup.py (distutils doesn't seem to allow extra command-line options to
# the setup.py script)
srcbuilddirs.py:
	echo "srcdir = '$(srcdir)'" > $(top_builddir)/client_libs/libplayerc/bindings/python/srcbuilddirs.py
	echo "top_srcdir = '$(top_srcdir)'" >> $(top_builddir)/client_libs/libplayerc/bindings/python/srcbuilddirs.py
	echo "builddir = '$(builddir)'" >> $(top_builddir)/client_libs/libplayerc/bindings/python/srcbuilddirs.py
	echo "top_builddir = '$(top_builddir)'" >> $(top_builddir)/client_libs/libplayerc/bindings/python/srcbuilddirs.py

# Use distutils to build module
pythonbuild: playerc_oo.i ../../playerc.h srcbuilddirs.py
	if test ! -s setup.py; then cp $(srcdir)/setup.py setup.py; fi
	python setup.py build

# For distcheck builds, clean up the files we created in the build tree
# This generates warnings, so I'm relying on the top-level Makefile.am override.
#distclean:
#	rm -f ../../playerc.h
#	rm -f $(top_builddir)/server/player.h
#	rm -f playerc_oo.i playerc.i playerc_wrap.c Makefile

# Normal cleanup
clean-local:
	rm -f playerc.py *.pyc playerc_oo.i $(top_builddir)/client_libs/libplayerc/bindings/python/srcbuilddirs.py
	rm -rf build

install-data-hook: $(PYTHONINSTALL)

# use distutils to install
pythoninstall:
	if test ! -s setup.py; then cp $(srcdir)/setup.py setup.py; fi
	python setup.py install --prefix=$(DESTDIR)/$(prefix)

# seems that distutils doesn't support 'uninstall', oh well
uninstall-local:
	echo 'Uninstall not implemented (sorry)'

