@REM Builds the debugger
@if NOT EXIST *.class goto compile
echo ------========Deleting old class files========---------
del /S /Q *.class 

:compile
@echo ----------=========Compiling=========----------
javac.exe -classpath .;..\..\SoarLibrary\swt-windows.jar;..\..\SoarLibrary\sml.jar -sourcepath . debugger\Application.java
@echo ----------=========Jarring...=========-----------
jar.exe cfm ..\..\SoarLibrary\SoarJavaDebugger.jar JarManifest-windows .

@echo.
@echo.
@IF "%1"=="--nopause" goto :end
@echo Build Complete
@pause

:end