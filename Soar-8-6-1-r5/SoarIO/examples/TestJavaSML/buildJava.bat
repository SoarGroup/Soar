@REM *** MAKE SURE CHANGES TO THIS FILE ARE REFLECTED IN THE .SH FILE

set SOARLIB=..\..\..\soar-library

javac.exe -classpath %SOARLIB%\sml.jar Application.java
jar cvfm %SOARLIB%\TestJavaSML.jar JarManifest .

pause