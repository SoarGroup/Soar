@REM Need to run this to copy the Java files over to our project
@REM where we can build and work with them.
@REM Later, we may choose to just copy around a JAR file containing the Java files
set SOARIO=..\soario

xcopy %SOARIO%\ClientSMLSWIG\java\*.java sml\*.java /y
xcopy %SOARIO%\ClientSMLSWIG\java\*.dll *.dll /y

@REM We also need libraries that Java_sml_ClientInterface.dll is dependent on.
xcopy %SOARIO%\bin\KernelSML.dll *.dll /y
xcopy %SOARIO%\bin\ElementXML.dll *.dll /y

@REM Bring over the alias files and help file so that they are found when the libs are loaded
xcopy %SOARIO%\bin\aliases.txt *.txt /y
xcopy %SOARIO%\bin\usage.txt *.txt /y


