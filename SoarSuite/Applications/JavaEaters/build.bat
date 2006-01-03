del /s /Q *.class

javac -classpath ..\soar-library\swt-windows.jar;..\soar-library\sml.jar;..\soar-library\JavaBaseEnvironment.jar -sourcepath src src\edu\umich\eaters\Eaters.java

jar cfm ..\soar-library\eaters.jar JarManifest -C src .

IF NOT "%1"=="--nopause" pause