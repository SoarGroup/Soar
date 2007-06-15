@echo off
rem %2 is an optional command line argument specifying the java bin directory to use to build everything
rem NOTE: if this directory is specified, the trailing slash must be included!
rem Also, if there are spaces in the path, it must be wrapped in quotes

echo.
echo ************* Building VisualSoar ****************
echo.

set SOARBIN=..\..\SoarLibrary\bin

IF NOT EXIST %SOARBIN%\sml.jar GOTO no_sml

echo ----------=====Setting up tmp dir====----------
IF EXIST tmp rmdir /S /Q tmp
mkdir tmp

@echo ----------=========Compiling=========----------
%2javac.exe -source 1.4 -d tmp -classpath .;%SOARBIN%\sml.jar -sourcepath Source Source\edu\umich\visualsoar\VisualSoar.java

@echo ----------==========Jarring==========----------
%2jar cfm %SOARBIN%\VisualSoar.jar Source\meta-inf\manifest.mf -C tmp .

GOTO success

:no_sml
echo ERROR:  %SOARBIN%\sml.jar is missing.  
echo Build Stopped
goto end

:success
echo Build Complete
IF NOT "%1" == "--nopause" pause
exit /b 0

:end
IF NOT "%1" == "--nopause" pause
exit /b 1
