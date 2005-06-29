#!/bin/sh

# Set up directories
DIST_ROOT=${HOME}/soar/dist
DEV_ROOT=${HOME}/soar/dev
PACKAGE_ROOT=${DIST_ROOT}/Soar-Suite-8.6.1-r2
REVISION=HEAD

# Out with the old
rm -rf ${PACKAGE_ROOT}
mkdir ${PACKAGE_ROOT}
cd ${PACKAGE_ROOT}

# Check out modules
cvs -q -d :ext:winter:/cvsroot/soar export -r $REVISION SoarKernel
cvs -q -d :ext:winter:/cvsroot/soar export -r $REVISION gSKI
cvs -q -d :ext:winter:/cvsroot/soar export -r $REVISION SoarIO
cvs -q -d :ext:winter:/cvsroot/soar export -r $REVISION soar-library
cvs -q -d :ext:winter:/cvsroot/soar export -r $REVISION Documentation
cvs -q -d :ext:winter:/cvsroot/soar export -r $REVISION SoarJavaDebugger
cvs -q -d :ext:winter:/cvsroot/soar export -r $REVISION JavaTOH
cvs -q -d :ext:winter:/cvsroot/soar export -r $REVISION JavaMissionaries
cvs -q -d :ext:winter:/cvsroot/soar export -r $REVISION tclenvironments
cvs -q -d :ext:toolshed@cvs.sourceforge.net:/cvsroot/soar export -r $REVISION visualsoar

# Move dist files to top level
mv ${PACKAGE_ROOT}/SoarIO/dist/configure.ac ${PACKAGE_ROOT}
mv ${PACKAGE_ROOT}/SoarIO/dist/install-sh ${PACKAGE_ROOT}
mv ${PACKAGE_ROOT}/SoarIO/dist/Makefile.am ${PACKAGE_ROOT}
mv ${PACKAGE_ROOT}/SoarIO/dist/build-everything.sh ${PACKAGE_ROOT}
mv ${PACKAGE_ROOT}/SoarIO/dist/missing ${PACKAGE_ROOT}
rm -rf ${PACKAGE_ROOT}/SoarIO/dist

# Initialize configure scripts (this recurses)
cd ${PACKAGE_ROOT}; autoreconf 2>/dev/null

# Add binaries and data
cp ${DEV_ROOT}/soar-library/swt.jar ${PACKAGE_ROOT}/soar-library
cp ${DEV_ROOT}/soar-library/SoarJavaDebugger.jar ${PACKAGE_ROOT}/soar-library
cp ${DEV_ROOT}/soar-library/TestJavaSML.jar ${PACKAGE_ROOT}/soar-library
cp ${DEV_ROOT}/soar-library/mac.jar ${PACKAGE_ROOT}/soar-library
cp ${DEV_ROOT}/soar-library/toh.jar ${PACKAGE_ROOT}/soar-library
mkdir ${PACKAGE_ROOT}/soar-library/mac
cp ${DEV_ROOT}/soar-library/mac/* ${PACKAGE_ROOT}/soar-library/mac
cp ${DEV_ROOT}/soar-library/VisualSoar.jar ${PACKAGE_ROOT}/soar-library

# Remove unwanted files
rm -rf ${PACKAGE_ROOT}/soar-library/*.dll
rm -rf ${PACKAGE_ROOT}/soar-library/java_swt ${PACKAGE_ROOT}/soar-library/libswt-carbon-* ${PACKAGE_ROOT}/soar-library/libswt-pi-carbon-* ${PACKAGE_ROOT}/soar-library/libswt-webkit-carbon-* ${PACKAGE_ROOT}/soar-library/swt-carbon.jar ${PACKAGE_ROOT}/soar-library/swt-pi-carbon.jar

# Create the tarball
cd ${DIST_ROOT}
tar cfj Soar-Suite-8.6.1-r2.tar.bz2 Soar-Suite-8.6.1-r2
