@echo off
rem %2 is an optional command line argument specifying the java bin directory to use to build everything
rem NOTE: if this directory is specified, the trailing slash must be included!
rem Also, if there are spaces in the path, it must be wrapped in quotes

set SOARBIN=..\..\SoarLibrary\bin

IF NOT EXIST %SOARBIN%\swt.jar GOTO no_swt
IF NOT EXIST %SOARBIN%\sml.jar GOTO no_sml
IF NOT EXIST %SOARBIN%\JavaBaseEnvironment.jar GOTO no_JBE

echo ------========Setting up tmp dir========---------
IF EXIST tmp rmdir /S /Q tmp
mkdir tmp
mkdir tmp\images
xcopy /q /y source\* tmp

@echo ----------=========Compiling=========----------
%2javac -source 1.4 -d tmp -classpath %SOARBIN%\swt.jar;%SOARBIN%\sml.jar;%SOARBIN%\JavaBaseEnvironment.jar -sourcepath source source\eaters\Eaters.java

@echo ----------=========Jarring...=========-----------
xcopy /y source\* tmp
%2jar cfm JavaEaters.jar JarManifest -C tmp .

GOTO success

:no_swt
echo ERROR:  %SOARBIN%\swt.jar is missing.  
echo         Did you remember to download it from: http://winter.eecs.umich.edu/jars/
echo Build Stopped
goto end

:no_sml
echo ERROR:  %SOARBIN%\sml.jar is missing.  
echo Build Stopped
goto end

:no_JBE
echo ERROR:  %SOARBIN%\JavaBaseEnvironment.jar is missing.  
echo           Did you remember to build it?
echo Build Stopped
goto end

:success
echo Build Complete

:end
IF NOT "%1" == "--nopause" pause