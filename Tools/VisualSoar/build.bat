@echo off
rem %2 is an optional command line argument specifying the java bin directory to use to build everything
rem NOTE: if this directory is specified, the trailing slash must be included!
rem Also, if there are spaces in the path, it must be wrapped in quotes

del /S /Q *.class 
%2javac.exe -source 1.4 -classpath .;..\..\SoarLibrary\bin\sml.jar -sourcepath Source Source\edu\umich\visualsoar\VisualSoar.java
%2jar cfm ..\..\SoarLibrary\bin\VisualSoar.jar Source\meta-inf\manifest.mf -C Source .

IF NOT "%1"=="--nopause" pause