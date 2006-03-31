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

EXTRA_DIST = Documentation Resources SoarLibrary 
dist-hook:
	rm -rf `find $(distdir) -name .svn`
	rm `find $(distdir) -name *.jar`
	wget http://winter.eecs.umich.edu/jars/motif/swt.jar
	mv swt.jar $(distdir)/SoarLibrary/bin
