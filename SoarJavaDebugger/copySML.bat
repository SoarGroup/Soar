@REM Need to run this to copy the Java files over to our project
@REM where we can build and work with them.
@REM Later, we may choose to just copy around a JAR file containing the Java files
set SOARIO=..\soario
set SOARLIB=..\soar-library

xcopy %SOARIO%\ClientSMLSWIG\java\build\*.java sml\*.java /y
@REM xcopy %SOARIO%\ClientSMLSWIG\java\build\*.dll *.dll /y

@REM We also need libraries that Java_sml_ClientInterface.dll is dependent on.
@REM xcopy %SOARLIB%\SoarKernelSML.dll *.dll /y
@REM xcopy %SOARLIB%\ElementXML.dll *.dll /y



