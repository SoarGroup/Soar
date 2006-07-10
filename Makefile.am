###########################################
# Author: Jonathan Voigt, voigtjr@gmail.com
# Date: 2006
###########################################
SUBDIRS = Core Tools Environments

java:
if SWIGJAVA
	Tools/LoggerJava/build.sh
	Tools/SoarJavaDebugger/build.sh
	Tools/TestJavaSML/build.sh
	Tools/VisualSoar/build.sh
	Environments/JavaBaseEnvironment/build.sh
	Environments/JavaEaters/build.sh
	Environments/JavaMissionaries/build.sh
	Environments/JavaTOH/build.sh
	Environments/JavaTankSoar/build.sh
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
	$(INSTALL) SoarLibrary/bin/libcairo-swt.so
	$(INSTALL) SoarLibrary/bin/libswt-agl-carbon-3232.jnilib
	$(INSTALL) SoarLibrary/bin/libswt-atk-gtk-3232.so
	$(INSTALL) SoarLibrary/bin/libswt-awt-gtk-3232.so
	$(INSTALL) SoarLibrary/bin/libswt-cairo-gtk-3232.so
	$(INSTALL) SoarLibrary/bin/libswt-carbon-3232.jnilib
	$(INSTALL) SoarLibrary/bin/libswt-glx-gtk-3232.so
	$(INSTALL) SoarLibrary/bin/libswt-gnome-gtk-3232.so
	$(INSTALL) SoarLibrary/bin/libswt-gtk-3232.so
	$(INSTALL) SoarLibrary/bin/libswt-mozilla-gcc3-gtk-3232.so
	$(INSTALL) SoarLibrary/bin/libswt-mozilla-gtk-3232.so
	$(INSTALL) SoarLibrary/bin/libswt-pi-carbon-3232.jnilib
	$(INSTALL) SoarLibrary/bin/libswt-pi-gtk-3232.so
	$(INSTALL) SoarLibrary/bin/libswt-webkit-carbon-3232.jnilib
