del /s /Q *.class

javac -classpath ..\..\SoarLibrary\bin\swt.jar;..\..\SoarLibrary\bin\sml.jar -sourcepath src src\edu\umich\JavaBaseEnvironment\*.java

jar cf ..\..\SoarLibrary\bin\javabaseenvironment.jar -C src .

IF NOT "%1"=="--nopause" pause