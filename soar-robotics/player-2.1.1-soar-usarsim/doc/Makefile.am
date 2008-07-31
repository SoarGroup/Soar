SUBDIRS = images

EXTRA_DIST =  \
README \
architecture.txt \
configfile.html \
footer.html \
footer.php \
header.html \
header.php \
install.txt \
player.dox \
player.txt \
playerweb.dox \
plugin_driver.html \
quick_start.txt \
supported_hardware.txt \
tutorial_args.txt \
tutorial_automake.txt \
tutorial_cameras.txt \
tutorial_config.txt \
tutorial_crosscompiling.txt \
tutorial_datalog.txt \
tutorial_devices.txt \
tutorial_interface.txt \
tutorial_maps.txt \
tutorial_migration.txt \
tutorial_plugins.txt \
upload_www.sh

CVS_DEST=Player-cvs
NAME=player

#dist-hook: doc
#	cp -r -p -v player-docs $(distdir)/player-docs

doc:
	doxygen player.dox

docdir = $(prefix)/share/doc

#install-data-hook:
doc-install:
	 if test -d $(srcdir)/player-docs; then mkdir -p $(DESTDIR)$(docdir); cp -r -p -v $(srcdir)/player-docs $(DESTDIR)$(docdir)/player; fi

#uninstall-local:
doc-uninstall:
	chmod -R u+w $(DESTDIR)$(docdir)/player
	rm -rf $(DESTDIR)$(docdir)/player

#doxygen playerweb.dox
webdoc:
	doxygen player.dox

upload_cvs: webdoc
	./upload_www.sh $(CVS_DEST) $(NAME)
