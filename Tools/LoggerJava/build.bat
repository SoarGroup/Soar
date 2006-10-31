@echo off
@REM *** MAKE SURE CHANGES TO THIS FILE ARE REFLECTED IN THE .SH FILE
rem %2 is an optional command line argument specifying the java bin directory to use to build everything
rem NOTE: if this directory is specified, the trailing slash must be included!
rem Also, if there are spaces in the path, it must be wrapped in quotes

echo.
echo ************* Building LoggerJava ****************
echo.

set SOARBIN=..\..\SoarLibrary\bin

IF NOT EXIST %SOARBIN%\sml.jar GOTO no_sml

echo ----------=====Setting up tmp dir====----------
IF EXIST tmp rmdir /S /Q tmp
mkdir tmp

@echo ----------=========Compiling=========----------
%2javac.exe -source 1.4 -d tmp -classpath %SOARBIN%\sml.jar;. log\MainFrame.java

@echo ----------==========Jarring==========----------
%2jar cfm %SOARBIN%\LoggerJava.jar JarManifest -C tmp .

GOTO success

:no_sml
echo ERROR:  %SOARBIN%\sml.jar is missing.  
echo Build Stopped
goto end

:success
echo Build Complete

:end
IF NOT "%1" == "--nopause" pause