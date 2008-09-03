@echo off
@REM Builds the debugger
rem %2 is an optional command line argument specifying the java bin directory to use to build everything
rem NOTE: if this directory is specified, the trailing slash must be included!
rem Also, if there are spaces in the path, it must be wrapped in quotes

@REM @if NOT EXIST *.class goto compile
echo ------========Deleting old class files========---------
del /S /Q *.class 

:compile
@echo ----------=========Compiling=========----------
%2javac.exe -source 1.4 -classpath .;..\..\SoarLibrary\bin\swt.jar;..\..\SoarLibrary\bin\sml.jar -sourcepath . debugger\Application.java
@echo ----------=========Jarring...=========-----------
%2jar.exe cfm ..\..\SoarLibrary\bin\SoarJavaDebugger.jar JarManifest .

@echo.
@echo.
@IF "%1"=="--nopause" goto :end
@echo Build Complete
@pause

:end
