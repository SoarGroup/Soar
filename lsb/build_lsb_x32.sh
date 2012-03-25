#!/bin/bash

DIR_TAIL=$(echo $(pwd) | sed 's/.*\///')
if [ "$DIR_TAIL" == "lsb" ]
then
  cd ..
fi

mkdir -p out

scons \
  --cxx=$(pwd)/lsb/i686-ccache-lsb-g++-4.4.sh \
  --lnflags="--lsb-shared-libs=python2.7 --lsb-shared-libpath=out" \
  --arch=32 \
  all

cp out/java/sml.jar ../AgentDevelopmentTools/VisualSoar/lib/
pushd ../AgentDevelopmentTools/VisualSoar
ant
cp java/soar-visualsoar-snapshot.jar ../../SoarSuite/out/VisualSoar.jar
popd

rm out/Python_sml_ClientInterface.py out/_Python_sml_ClientInterface.so

/opt/lsb32/bin/lsbappchk --no-journal --missing-symbols --lsb-version=4.0 --shared-libpath=out out/cli
/opt/lsb32/bin/lsbappchk --no-journal --missing-symbols --lsb-version=4.0 --shared-libpath=out out/TestCLI
/opt/lsb32/bin/lsbappchk --no-journal --missing-symbols --lsb-version=4.0 --shared-libpath=out out/TestExternalLibrary
/opt/lsb32/bin/lsbappchk --no-journal --missing-symbols --lsb-version=4.0 --shared-libpath=out out/TestSMLEvents
/opt/lsb32/bin/lsbappchk --no-journal --missing-symbols --lsb-version=4.0 --shared-libpath=out out/TestSMLPerformance
/opt/lsb32/bin/lsbappchk --no-journal --missing-symbols --lsb-version=4.0 --shared-libpath=out out/TestSoarPerformance
/opt/lsb32/bin/lsbappchk --no-journal --missing-symbols --lsb-version=4.0 --shared-libpath=out out/UnitTests
