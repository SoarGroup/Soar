
SUBDIRS = replace libplayerjpeg libplayerxdr libplayercore libplayertcp libplayersd rtk2 server libplayercore/bindings libplayertcp/bindings config client_libs examples utils doc 

EXTRA_DIST = config.simulation config.p2os COPYING.lib

clean-local:
	rm -f config.cache

# This tell distcheck to ignore files left over from distclean
# TODO: fix the python install script to clean up after itself.
distcleancheck:
	@:

# This tell distcheck to ignore files left over from uninstall
distuninstallcheck:
	@:

