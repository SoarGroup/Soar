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
	rm -rf `find $(distdir) -name .svn`
	rm `find $(distdir) -name *.jar`
	rm -rf $(distdir)/Documentation/ManualSource
	rm -rf $(distdir)/Documentation/Old
	rm -rf `find $(distdir)/Documentation -name *.ppt`
	rm -rf `find $(distdir)/Documentation -name *.doc`
	rm -rf $(distdir)/Resources/Templates
	rm -rf `find $(distdir)/SoarLibrary/CLIHelp -name *.tex`
	rm -rf $(distdir)/SoarLibrary/CLIHelp/Scripts
	rm -rf `find $(distdir)/SoarLibrary/bin -name *.dll`
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
