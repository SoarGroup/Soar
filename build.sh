#!/bin/bash

scons

cp out/java/sml.jar ../AgentDevelopmentTools/VisualSoar/lib/
pushd ../AgentDevelopmentTools/VisualSoar
ant
cp java/soar-visualsoar-snapshot.jar ../../SoarSuite/out/VisualSoar.jar
popd
