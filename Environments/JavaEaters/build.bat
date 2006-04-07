@echo off
mkdir bin
del /s /Q bin\*.class
javac -source 1.4 -d bin -classpath ..\..\SoarLibrary\bin\swt.jar;..\..\SoarLibrary\bin\sml.jar;..\..\SoarLibrary\bin\JavaBaseEnvironment.jar -sourcepath source source\eaters\Eaters.java
xcopy /y source\* bin

jar cfm JavaEaters.jar JarManifest -C bin .

IF NOT "%1"=="--nopause" pause