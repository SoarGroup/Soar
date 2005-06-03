@REM Builds the debugger

del /S /Q *.class 
javac.exe -classpath .;..\soar-library\swt.jar;..\soar-library\sml.jar debugger\Application.java
jar.exe cfm ..\soar-library\SoarJavaDebugger.jar JarManifest .

IF NOT "%1"=="--nopause" pause