#!/bin/bash

scons

cp out/java/sml.jar ../AgentDevelopmentTools/VisualSoar/lib/
cd ../AgentDevelopmentTools/VisualSoar
ant
cd ../../SoarSuite
cp ../AgentDevelopmentTools/VisualSoar/java/soar-visualsoar-snapshot.jar out/VisualSoar.jar
