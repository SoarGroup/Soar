@REM Builds the debugger
@if NOT EXIST *.class goto compile
echo ------========Deleting old class files========---------
del /S /Q *.class 

:compile
@echo ----------=========Compiling=========----------
javac.exe -classpath .;..\soar-library\swt-windows.jar;..\soar-library\sml.jar -sourcepath . debugger\Application.java
@echo ----------=========Jarring...=========-----------
jar.exe cfm ..\soar-library\SoarJavaDebugger.jar JarManifest-windows .

@echo.
@echo.
@IF "%1"=="--nopause" goto :end
@echo Build Complete
@pause

:end