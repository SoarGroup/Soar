del /s /Q *.class

javac -classpath ..\..\SoarLibrary\bin\swt.jar;..\..\SoarLibrary\bin\sml.jar;..\..\SoarLibrary\bin\JavaBaseEnvironment.jar -sourcepath src src\edu\umich\tanksoar\TanksoarJ.java

jar cfm ..\..\SoarLibrary\bin\tanksoar.jar JarManifest -C src .

IF NOT "%1"=="--nopause" pause