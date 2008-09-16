@echo off
@REM Builds the debugger
rem %2 is an optional command line argument specifying the java bin directory to use to build everything
rem NOTE: if this directory is specified, the trailing slash must be included!
rem Also, if there are spaces in the path, it must be wrapped in quotes

echo.
echo ************* Building SoarJavaDebugger ****************
echo.

set SOARBIN=..\..\SoarLibrary\bin

IF NOT EXIST %SOARBIN%\swt.jar GOTO no_swt
IF NOT EXIST %SOARBIN%\sml.jar GOTO no_sml

echo ----------=====Setting up tmp dir====----------
IF EXIST tmp rmdir /S /Q tmp
mkdir tmp
mkdir tmp\altimages
mkdir tmp\images
xcopy /q /y src\*.dlf tmp
xcopy /q /y src\altimages\* tmp\altimages
xcopy /q /y src\images\* tmp\images

@echo ----------=========Compiling=========----------
%2javac.exe -source 1.5 -d tmp -classpath src;%SOARBIN%\swt.jar;%SOARBIN%\sml.jar;%SOARBIN%\jcommon-1.0.10.jar;%SOARBIN%\jfreechart-1.0.6.jar;%SOARBIN%\jfreechart-1.0.6-swt.jar;%SOARBIN%\swtgraphics2d.jar -sourcepath src src\debugger\Application.java

@echo ----------==========Jarring==========----------
%2jar.exe cfm %SOARBIN%\SoarJavaDebugger.jar JarManifest -C tmp .

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

:success
echo Build Complete
IF NOT "%1" == "--nopause" pause
exit /b 0

:end
IF NOT "%1" == "--nopause" pause
exit /b 1

