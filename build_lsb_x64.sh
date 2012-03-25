#!/bin/bash

mkdir -p out

scons \
  --cxx=x86_64-ccache-lsbc++-4.4.sh \
  --lnflags="--lsb-shared-libs=python2.7 --lsb-shared-libpath=out" \
  --pretend-cxx=g++ \
  all

cp out/java/sml.jar ../AgentDevelopmentTools/VisualSoar/lib/
pushd ../AgentDevelopmentTools/VisualSoar
ant
cp java/soar-visualsoar-snapshot.jar ../../SoarSuite/out/VisualSoar.jar
popd

rm out/Python_sml_ClientInterface.py out/_Python_sml_ClientInterface.so

/opt/lsb/bin/lsbappchk --no-journal --missing-symbols --lsb-version=4.0 --shared-libpath=out out/cli
/opt/lsb/bin/lsbappchk --no-journal --missing-symbols --lsb-version=4.0 --shared-libpath=out out/TestCLI
/opt/lsb/bin/lsbappchk --no-journal --missing-symbols --lsb-version=4.0 --shared-libpath=out out/TestExternalLibrary
/opt/lsb/bin/lsbappchk --no-journal --missing-symbols --lsb-version=4.0 --shared-libpath=out out/TestSMLEvents
/opt/lsb/bin/lsbappchk --no-journal --missing-symbols --lsb-version=4.0 --shared-libpath=out out/TestSMLPerformance
/opt/lsb/bin/lsbappchk --no-journal --missing-symbols --lsb-version=4.0 --shared-libpath=out out/TestSoarPerformance
/opt/lsb/bin/lsbappchk --no-journal --missing-symbols --lsb-version=4.0 --shared-libpath=out out/UnitTests
