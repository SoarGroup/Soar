@REM Need to run this to copy the Java files over to our project
@REM where we can build and work with them.
@REM Later, we may choose to just copy around a JAR file containing the Java files

xcopy ..\..\ClientSMLSWIG\java\*.java sml\*.java /y
xcopy ..\..\ClientSMLSWIG\java\*.dll *.dll /y

@REM We also need libraries that Java_sml_ClientInterface.dll is dependent on.
@REM Hopefully this set will be reduced down the road if we use more static linking.
xcopy ..\..\bin\*.dll *.dll /y

@REM Remove DLLs we don't need (so I can be sure we don't have odd dependencies)
del pthreadVC.dll
del tcl84.dll
del TgDId.dll
del tk84.dll

