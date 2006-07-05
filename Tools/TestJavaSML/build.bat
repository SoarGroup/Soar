@echo off
rem %2 is an optional command line argument specifying the java bin directory to use to build everything
rem NOTE: if this directory is specified, the trailing slash must be included!
rem Also, if there are spaces in the path, it must be wrapped in quotes

@REM *** MAKE SURE CHANGES TO THIS FILE ARE REFLECTED IN THE .SH FILE

set SOARLIB=..\..\SoarLibrary\bin

%2javac.exe -source 1.4 -classpath %SOARLIB%\sml.jar Application.java
%2jar cfm %SOARLIB%\TestJavaSML.jar JarManifest .

IF NOT "%1"=="--nopause" pause