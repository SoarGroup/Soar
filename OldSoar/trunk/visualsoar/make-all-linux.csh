#!/bin/csh

# Set the version (BUGBUG: Should be from command line)
set ver=001

# Set the output folder
set output=`pwd`/Releases/$ver-Linux
echo $output
rm -r $output
mkdir $output

echo Build STI
pushd STI
rm *.so
make
cp *.so SoarToolJavaInterface
pushd SoarToolJavaInterface
make
popd
popd

echo Build Soar-8.3
# For now, copy STI lib down so linker will find it.
# Later, should be able to do this with a linker option
# but not sure how for shared libs.
rm Soar-8.3/interface/*.so
cp STI/*.so Soar-8.3/interface
pushd Soar-8.3
./make-soar
popd

echo Build VisualSoar
pushd VisualSoar/Source
rm edu/umich/visualsoar/*.class
javac edu/umich/visualsoar/VisualSoar.java

echo Build VisualSoar.jar file
rm VisualSoar.jar
jar cvfm VisualSoar.jar META-INF/MANIFEST.MF -C . * > $output/VSBuild.txt
popd

echo Copy to release point
# For now, copy existing release from soar and modify it
cp -r LinuxBase/visualsoar $output/visualsoar
cp -r LinuxBase/soar-8.3 $output/soar-8.3

# Overwrite the relevant parts
cp Soar-8.3/interface/libsoar*.so $output/soar-8.3/interface
cp Soar-8.3/interface/libsoar*.so $output/soar-8.3/library
cp Soar-8.3/library/tsi31/*.tcl $output/soar-8.3/library/tsi31
cp VisualSoar/Source/VisualSoar.jar $output/visualsoar/VisualSoar.jar

# Copy the STI files
mkdir $output/STI
cp STI/*.so $output/STI
cp STI/SoarToolJavaInterface/*.so $output/STI

# For Linux, write a small file to set the LD_LIBRARY_PATH
# for the user to the release directory.
# To use it type "source setenv.bsh"
pushd $output
echo -n "#!" > setenv.bsh
echo "/bin/bash" >> setenv.bsh
echo "LD_LIBRARY_PATH=$output/STI" >> setenv.bsh
echo "export LD_LIBRARY_PATH" >> setenv.bsh
popd





