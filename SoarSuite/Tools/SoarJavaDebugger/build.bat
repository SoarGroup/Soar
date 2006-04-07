@echo off
@REM Builds the debugger
@REM @if NOT EXIST *.class goto compile
echo ------========Deleting old class files========---------
del /S /Q *.class 

:compile
@echo ----------=========Compiling=========----------
javac.exe -source 1.4 -classpath .;..\..\SoarLibrary\bin\swt.jar;..\..\SoarLibrary\bin\sml.jar -sourcepath . debugger\Application.java
@echo ----------=========Jarring...=========-----------
jar.exe cfm ..\..\SoarLibrary\bin\SoarJavaDebugger.jar JarManifest .

@echo.
@echo.
@IF "%1"=="--nopause" goto :end
@echo Build Complete
@pause

:end
