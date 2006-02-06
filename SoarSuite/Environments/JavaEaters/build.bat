del /s /Q *.class

javac -classpath ..\..\SoarLibrary\bin\swt.jar;..\..\SoarLibrary\bin\sml.jar;..\..\SoarLibrary\bin\JavaBaseEnvironment.jar -sourcepath src src\edu\umich\eaters\Eaters.java

jar cfm ..\..\SoarLibrary\bin\eaters.jar JarManifest -C src .

IF NOT "%1"=="--nopause" pause