#!/bin/sh

# Set up directories
DIST_ROOT=${HOME}/soar/dist
DEV_ROOT=${HOME}/soar/dev
PACKAGE_ROOT=${DIST_ROOT}/Soar-Suite-8.6.1-r2

# Out with the old
rm -rf ${PACKAGE_ROOT}
mkdir ${PACKAGE_ROOT}
cd ${PACKAGE_ROOT}

# Check out modules
cvs -q -d :ext:winter.eecs.umich.edu:/cvsroot/soar export -r HEAD SoarKernel
cvs -q -d :ext:winter.eecs.umich.edu:/cvsroot/soar export -r HEAD gSKI
cvs -q -d :ext:winter.eecs.umich.edu:/cvsroot/soar export -r HEAD SoarIO
cvs -q -d :ext:winter.eecs.umich.edu:/cvsroot/soar export -r HEAD soar-library
cvs -q -d :ext:winter.eecs.umich.edu:/cvsroot/soar export -r HEAD Documentation
cvs -q -d :ext:winter.eecs.umich.edu:/cvsroot/soar export -r HEAD SoarJavaDebugger
cvs -q -d :ext:winter.eecs.umich.edu:/cvsroot/soar export -r HEAD JavaTOH
cvs -q -d :ext:winter.eecs.umich.edu:/cvsroot/soar export -r HEAD JavaMissionaries
#cvs -q -d :ext:winter.eecs.umich.edu:/cvsroot/soar export -r HEAD tclenvironments

# Move dist files to top level
mv ${PACKAGE_ROOT}/SoarIO/dist/* ${PACKAGE_ROOT}
rm -rf ${PACKAGE_ROOT}/SoarIO/dist ${PACKAGE_ROOT}/README

# Initialize configure scripts (this recurses)
cd ${PACKAGE_ROOT}; autoreconf

# Add binaries and data
cp ${DEV_ROOT}/soar-library/swt.jar ${PACKAGE_ROOT}/soar-library
cp ${DEV_ROOT}/soar-library/SoarJavaDebugger.jar ${PACKAGE_ROOT}/soar-library
cp ${DEV_ROOT}/soar-library/TestJavaSML.jar ${PACKAGE_ROOT}/soar-library
cp ${DEV_ROOT}/soar-library/mac.jar ${PACKAGE_ROOT}/soar-library
cp ${DEV_ROOT}/soar-library/toh.jar ${PACKAGE_ROOT}/soar-library
mkdir ${PACKAGE_ROOT}/soar-library/mac
cp ${DEV_ROOT}/soar-library/mac/* ${PACKAGE_ROOT}/soar-library/mac

# Remove windows dlls
rm -rf ${PACKAGE_ROOT}/soar-library/*.dll

# Create the tarball
cd ${DIST_ROOT}
tar cfj Soar-Suite-8.6.1-r2.tar.bz2 Soar-Suite-8.6.1-r2
