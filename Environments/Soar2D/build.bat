@echo off
rem %2 is an optional command line argument specifying the java bin directory to use to build everything
rem NOTE: if this directory is specified, the trailing slash must be included!
rem Also, if there are spaces in the path, it must be wrapped in quotes

echo.
echo ************* Building Soar2D ****************
echo.

set SOARBIN=..\..\SoarLibrary\bin
set IMAGES=org\msoar\gridmap2d\images

IF NOT EXIST %SOARBIN%\swt.jar GOTO no_swt
IF NOT EXIST %SOARBIN%\sml.jar GOTO no_sml

echo ----------=====Setting up tmp dir====----------
IF EXIST tmp rmdir /S /Q tmp
mkdir tmp
mkdir tmp\%IMAGES%
mkdir tmp\%IMAGES%\tanksoar
xcopy /q /y src\* tmp
xcopy /q /y /s src\%IMAGES%\* tmp\%IMAGES%\
xcopy /q /y /s src\%IMAGES%\tanksoar* tmp\%IMAGES%\tanksoar

@echo ----------=========Compiling=========----------
%2javac -source 1.5 -d tmp -classpath jdom.jar;%SOARBIN%\log4j-1.2.15.jar;%SOARBIN%\swt.jar;%SOARBIN%\sml.jar;%SOARBIN%\tosca.jar -sourcepath src src\org\msoar\gridmap2d\Gridmap2D.java

@echo ----------==========Jarring==========----------
%2jar cfm Soar2D.jar JarManifest -C tmp .

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

