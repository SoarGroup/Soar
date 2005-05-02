xcopy ..\SoarIO\ClientSMLSWIG\Java\build\*.java src\sml /Y /I
javac -classpath .;swt.jar -sourcepath src\edu\umich\toh;src\sml;.;src src\edu\umich\toh\TowersOfHanoi.java
jar cvfm ..\soar-library\toh.jar tohJarManifest -C src .
pause