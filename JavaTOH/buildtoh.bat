javac -classpath .;..\soar-library\swt.jar;..\soar-library\sml.jar -sourcepath src src\edu\umich\toh\TowersOfHanoi.java
jar cfm ..\soar-library\toh.jar tohJarManifest -C src .
pause