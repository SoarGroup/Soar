set SOARLIB=../../SoarLibrary/bin
set PATH=%SOARLIB%;%PATH%

rem I prefer this method (keeps the java line clean)
rem set CLASSPATH=%SOARLIB%/sml.jar;%SOARLIB%/swt.jar;.
rem java -jar toh.jar

rem this is an alternative method that should also work
rem java -cp %SOARLIB%/swt.jar;%SOARLIB%/sml.jar;. -jar toh.jar

rem this will work if the classpath is specified in the JarManifest
start javaw -jar toh.jar