javac -classpath .;..\soar-library\swt.jar;..\soar-library\sml.jar -sourcepath src src\edu\umich\mac\MissionariesAndCannibals.java
jar cvfm ..\soar-library\mac.jar macJarManifest mac -C src .
xcopy mac\mac.soar ..\soar-library\mac\* /Y
pause