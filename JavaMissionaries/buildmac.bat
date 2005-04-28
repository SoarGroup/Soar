xcopy ..\SoarIO\ClientSMLSWIG\Java\build\*.java src\sml /Y /I

javac -classpath .;swt.jar -sourcepath src\edu\umich\toh;src\sml;.;src src\edu\umich\mac\MissionariesAndCannibals.java

jar cvfm ..\soar-library\mac.jar macJarManifest -C src .

xcopy mac ..\soar-library\mac /Y /I