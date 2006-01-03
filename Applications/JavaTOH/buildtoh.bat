del /S /Q *.class
javac -classpath .;..\soar-library\swt-windows.jar;..\soar-library\sml.jar -sourcepath src src\edu\umich\toh\TowersOfHanoi.java
jar cfm ..\soar-library\toh.jar JarManifest-windows -C src .
IF NOT "%1"=="--nopause" pause