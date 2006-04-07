@echo off
@REM *** MAKE SURE CHANGES TO THIS FILE ARE REFLECTED IN THE .SH FILE

set SOARLIB=..\..\SoarLibrary\bin

javac.exe -source 1.4 -classpath %SOARLIB%\sml.jar;. log\MainFrame.java
jar cfm %SOARLIB%\LoggerJava.jar JarManifest .

IF NOT "%1"=="--nopause" pause