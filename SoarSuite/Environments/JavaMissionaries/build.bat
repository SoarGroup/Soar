del /S /Q *.class 
javac -classpath .;..\..\SoarLibrary\bin\swt.jar;..\..\SoarLibrary\bin\sml.jar -sourcepath src src\edu\umich\mac\MissionariesAndCannibals.java
jar cfm ..\..\SoarLibrary\bin\mac.jar JarManifest -C src .
xcopy src\mac\mac.soar ..\..\SoarLibrary\bin\mac\* /Y
IF NOT "%1"=="--nopause" pause