@REM Need to run this to copy the Java files over to our project
@REM where we can build and work with them.
@REM Later, we may choose to just copy around a JAR file containing the Java files

set SOARIO=..\..

xcopy %SOARIO%\ClientSMLSWIG\java\*.java sml\*.java /y
xcopy %SOARIO%\ClientSMLSWIG\java\*.dll *.dll /y

@REM We also need libraries that Java_sml_ClientInterface.dll is dependent on.
xcopy %SOARIO%\bin\KernelSML.dll *.dll /y
xcopy %SOARIO%\bin\ElementXML.dll *.dll /y

