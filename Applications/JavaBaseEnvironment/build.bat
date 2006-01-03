del /s /Q *.class

javac -classpath ..\soar-library\swt-windows.jar;..\soar-library\sml.jar -sourcepath src src\edu\umich\JavaBaseEnvironment\*.java

jar cf ..\soar-library\javabaseenvironment.jar -C src .

IF NOT "%1"=="--nopause" pause