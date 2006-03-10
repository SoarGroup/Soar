mkdir bin
del /s /Q bin\*.class
javac -d bin -classpath ..\..\SoarLibrary\bin\swt.jar;..\..\SoarLibrary\bin\sml.jar -sourcepath source source\utilities\*.java source\simulation\*.java source\simulation\visuals\*.java

jar cf ..\..\SoarLibrary\bin\JavaBaseEnvironment.jar -C bin .

IF NOT "%1"=="--nopause" pause