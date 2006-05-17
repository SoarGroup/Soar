###########################################
# Author: Jonathan Voigt, voigtjr@gmail.com
# Date: 2006
###########################################
SUBDIRS = Core Tools Environments

java:
if SWIGJAVA
	pushd Tools/LoggerJava; ./build.sh; popd;
	pushd Tools/SoarJavaDebugger; ./build.sh; popd;
	pushd Tools/TestJavaSML; ./build.sh; popd;
	pushd Tools/VisualSoar; ./build.sh; popd;
	pushd Environments/JavaBaseEnvironment; ./build.sh; popd;
	pushd Environments/JavaEaters; ./build.sh; popd;
	pushd Environments/JavaMissionaries; ./build.sh; popd;
	pushd Environments/JavaTOH; ./build.sh; popd;
	pushd Environments/JavaTankSoar; ./build.sh; popd;
else
endif

EXTRA_DIST = Documentation Resources SoarLibrary acsite.m4
dist-hook:
	find $(distdir) -iname .svn -depth -exec rm -rfv {} \;
	find $(distdir) -iname *.jar -exec rm -rfv {} \;
	find $(distdir) -iname *.ppt -exec rm -rfv {} \;
	find $(distdir) -iname *.doc -exec rm -rfv {} \;
	find $(distdir) -iname *.tex -exec rm -rfv {} \;
	find $(distdir) -iname *.dll -exec rm -rfv {} \;
	rm -rfv $(distdir)/Documentation/ManualSource
	rm -rfv $(distdir)/Documentation/Old
	rm -rfv $(distdir)/Resources/Templates
	rm -rfv $(distdir)/SoarLibrary/CLIHelp/Scripts
	wget http://winter.eecs.umich.edu/jars/gtk/swt.jar
	mv swt.jar $(distdir)/SoarLibrary/bin

install-data-am:
	$(INSTALL) SoarLibrary/bin/libswt-atk-gtk-3139.so $(libdir)
	$(INSTALL) SoarLibrary/bin/libswt-awt-gtk-3139.so $(libdir)
	$(INSTALL) SoarLibrary/bin/libswt-cairo-gtk-3139.so $(libdir)
	$(INSTALL) SoarLibrary/bin/libswt-carbon-3139.jnilib $(libdir)
	$(INSTALL) SoarLibrary/bin/libswt-gnome-gtk-3139.so $(libdir)
	$(INSTALL) SoarLibrary/bin/libswt-gtk-3139.so $(libdir)
	$(INSTALL) SoarLibrary/bin/libswt-mozilla-gtk-3139.so $(libdir)
	$(INSTALL) SoarLibrary/bin/libswt-pi-carbon-3139.jnilib $(libdir)
	$(INSTALL) SoarLibrary/bin/libswt-pi-gtk-3139.so $(libdir)
	$(INSTALL) SoarLibrary/bin/libswt-webkit-carbon-3139.jnilib $(libdir)
