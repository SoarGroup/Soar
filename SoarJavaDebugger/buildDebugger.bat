@REM Builds the debugger

javac.exe -classpath .;swt.jar;sml.jar debugger\Application.java
jar.exe cvfm ..\soar-library\SoarJavaDebugger.jar JarManifest .
pause