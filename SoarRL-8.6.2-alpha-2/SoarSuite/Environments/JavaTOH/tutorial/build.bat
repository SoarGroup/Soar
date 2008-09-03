rem %2 is an optional command line argument specifying the java bin directory to use to build everything
rem NOTE: if this directory is specified, the trailing slash must be included!
rem Also, if there are spaces in the path, it must be wrapped in quotes

del /S /Q *.class
%2javac -classpath .;..\..\..\SoarLibrary\bin\swt.jar;..\..\..\SoarLibrary\bin\sml.jar -sourcepath src src\edu\umich\toh\TowersOfHanoi.java
%2jar cfm ..\..\..\SoarLibrary\bin\toh.jar ..\JarManifest -C src .
xcopy ..\towers-of-hanoi-SML.soar ..\..\..\SoarLibrary\bin\* /Y
IF NOT "%1"=="--nopause" pause