IMPORTANT NOTE:
===============
You may also build this project in the java IDE Eclipse, available at
http://www.eclipse.org
the files .project and .classpath define the build settings.
Tested on Eclipse 3.0 with java 1.4.2.
***VisualSoar has changed somewhat significantly since the Eclipse
***settings were last updated. Most notably, STI no longer exists
***and it now requires soar-library/sml.jar.  So some Eclipse settings
***will need to be modified before this will build in Eclipse.

Building
========

1) Build everything in SoarIO/SML.sln.

2) Rebuild the ClientSMLJava project in SoarIO/ClientSMLSWIG/ClientSMLSWIG.sln.

3) Run buildVisualSoar.bat (Windows) or buildVisualSoar.sh (Linux)