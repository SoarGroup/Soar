xcopy ..\SoarIO\ClientSMLSWIG\Java\build\*.java src\sml /Y /I
javac -classpath .;..\soar-library\swt.jar -sourcepath src\sml;src src\edu\umich\toh\TowersOfHanoi.java
jar cvfm ..\soar-library\toh.jar tohJarManifest -C src .
pause