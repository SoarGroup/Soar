@echo off

rem %1 is an optional command line argument specifying the java bin directory to use to build everything
rem NOTE: if this directory is specified, the trailing slash must be included!
rem Also, if there are spaces in the path, it must be wrapped in quotes

cd Environments\JavaMissionaries
call build.bat --nopause %1
if errorlevel 1 goto fail

cd ..\JavaTOH
call build.bat --nopause %1
if errorlevel 1 goto fail

cd ..\Soar2D
call build.bat --nopause %1
if errorlevel 1 goto fail

cd ..\..\Tools\LoggerJava
call build.bat --nopause %1
if errorlevel 1 goto fail

cd ..\SoarJavaDebugger
call build.bat --nopause %1
if errorlevel 1 goto fail

cd ..\TestJavaSML
call build.bat --nopause %1
if errorlevel 1 goto fail

cd ..\VisualSoar
call build.bat --nopause %1
if errorlevel 1 goto fail

cd ..\..
exit 0

:fail
exit 1