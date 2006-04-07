@echo off
del /S /Q *.class
javac -source 1.4 -classpath .;..\..\SoarLibrary\bin\swt.jar;..\..\SoarLibrary\bin\sml.jar -sourcepath src src\edu\umich\toh\TowersOfHanoi.java
jar cfm toh.jar JarManifest -C src .
IF NOT "%1"=="--nopause" pause