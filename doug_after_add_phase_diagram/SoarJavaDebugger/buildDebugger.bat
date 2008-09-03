@REM Builds the debugger

del /S /Q *.class 
javac.exe -classpath .;..\soar-library\swt-windows.jar;..\soar-library\sml.jar -sourcepath . debugger\Application.java
jar.exe cfm ..\soar-library\SoarJavaDebugger.jar JarManifest-windows .

IF NOT "%1"=="--nopause" pause