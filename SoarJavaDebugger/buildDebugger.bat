@REM Builds the debugger
copySML.bat
javac.exe -classpath .;swt.jar debugger\Application.java
jar.exe cvfm ..\soar-library\SoarJavaDebugger.jar JarManifest .