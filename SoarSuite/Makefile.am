###########################################
# Author: Jonathan Voigt, voigtjr@gmail.com
# Date: 2006
###########################################
SUBDIRS = Core Tools Environments

java:
if SWIGJAVA
	cd Tools && $(MAKE) $(MAKEOPTS) java
	cd Environments && $(MAKE) $(MAKEOPTS) java
else
endif

EXTRA_DIST = Documentation Resources SoarLibrary 
dist-hook:
	rm -rf `find $(distdir) -name .svn`
	rm `find $(distdir) -name *.jar`
	wget http://winter.eecs.umich.edu/jars/motif/swt.jar
	mv swt.jar $(distdir)/SoarLibrary/bin
