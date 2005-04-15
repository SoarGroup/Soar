@REM Need to run this to copy the Java files over to our project
@REM where we can build and work with them.
@REM Later, we may choose to just copy around a JAR file containing the Java files
set SOARLIB=..\..\..\soar-library
set SOARIO=..\..

xcopy %SOARIO%\ClientSMLSWIG\java\build\*.java sml\*.java /y

@REM Assuming javac is in the path, builds Application and 
@REM copies it to soar-library.

javac Application.java

@REM FIXME better way to jar this?
jar cvfm TestJavaSML.jar JarManifest *.class sml/*.class

xcopy *.class %SOARLIB% /y

