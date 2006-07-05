@echo off
rem %1 is an optional command line argument specifying the java bin directory to use to build everything
rem NOTE: if this directory is specified, the trailing slash must be included!
rem Also, if there are spaces in the path, it must be wrapped in quotes

cd Environments\JavaBaseEnvironment
call build.bat --nopause %1
cd ..\JavaEaters
call build.bat --nopause %1
cd ..\JavaMissionaries
call build.bat --nopause %1
cd ..\JavaTankSoar
call build.bat --nopause %1
cd ..\JavaTOH
call build.bat --nopause %1
cd ..\..\Tools\LoggerJava
call build.bat --nopause %1
cd ..\SoarJavaDebugger
call build.bat --nopause %1
cd ..\TestJavaSML
call build.bat --nopause %1
cd ..\VisualSoar
call build.bat --nopause %1
cd ..\..