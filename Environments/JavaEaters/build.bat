@echo off
rem %2 is an optional command line argument specifying the java bin directory to use to build everything
rem NOTE: if this directory is specified, the trailing slash must be included!
rem Also, if there are spaces in the path, it must be wrapped in quotes

mkdir bin
del /s /Q bin\*.class
%2javac -source 1.4 -d bin -classpath ..\..\SoarLibrary\bin\swt.jar;..\..\SoarLibrary\bin\sml.jar;..\..\SoarLibrary\bin\JavaBaseEnvironment.jar -sourcepath source source\eaters\Eaters.java
xcopy /y source\* bin

%2jar cfm JavaEaters.jar JarManifest -C bin .

IF NOT "%1"=="--nopause" pause