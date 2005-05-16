xcopy ..\SoarIO\ClientSMLSWIG\Java\build\*.java src\sml /Y /I
javac -classpath .;..\soar-library\swt.jar -sourcepath src\sml;src src\edu\umich\mac\MissionariesAndCannibals.java
jar cvfm ..\soar-library\mac.jar macJarManifest mac -C src .
xcopy mac\mac.soar ..\soar-library\mac\* /Y
pause