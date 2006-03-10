mkdir bin
del /s /Q bin\*.class
javac -d bin -classpath ..\..\SoarLibrary\bin\swt.jar;..\..\SoarLibrary\bin\sml.jar;..\..\SoarLibrary\bin\JavaBaseEnvironment.jar -sourcepath source source\eaters\Eaters.java
xcopy /y source\* bin

jar cfm ..\..\SoarLibrary\bin\JavaEaters.jar JarManifest -C bin .

IF NOT "%1"=="--nopause" pause