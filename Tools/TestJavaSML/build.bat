@REM *** MAKE SURE CHANGES TO THIS FILE ARE REFLECTED IN THE .SH FILE

set SOARLIB=..\..\SoarLibrary\bin

javac.exe -classpath %SOARLIB%\sml.jar Application.java
jar cfm %SOARLIB%\TestJavaSML.jar JarManifest .

IF NOT "%1"=="--nopause" pause