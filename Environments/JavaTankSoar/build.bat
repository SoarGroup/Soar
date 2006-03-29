mkdir bin
javac -d bin -classpath ..\..\SoarLibrary\bin\swt.jar;..\..\SoarLibrary\bin\sml.jar;..\..\SoarLibrary\bin\JavaBaseEnvironment.jar -sourcepath source source\tanksoar\TankSoar.java
xcopy /y source\* bin
mkdir bin\images
xcopy /y /s source\images\* bin\images

jar cfm JavaTankSoar.jar JarManifest -C bin .

IF NOT "%1"=="--nopause" pause
