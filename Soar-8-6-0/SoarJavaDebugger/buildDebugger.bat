@REM Builds the debugger

set SOARIO=..\soario
set SOARLIB=..\soar-library

xcopy %SOARIO%\ClientSMLSWIG\java\build\*.java sml\*.java /y

javac.exe -classpath .;swt.jar debugger\Application.java
jar.exe cvfm ..\soar-library\SoarJavaDebugger.jar JarManifest .
pause