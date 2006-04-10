@echo off
IF NOT EXIST ..\..\SoarLibrary\bin\swt.jar GOTO no_swt
IF NOT EXIST ..\..\SoarLibrary\bin\sml.jar GOTO no_sml

IF NOT EXIST bin mkdir bin
del /s /Q bin\*.class

@echo on
javac -d bin -classpath ..\..\SoarLibrary\bin\swt.jar;..\..\SoarLibrary\bin\sml.jar -sourcepath source source\utilities\*.java source\simulation\*.java source\simulation\visuals\*.java

jar cf ..\..\SoarLibrary\bin\JavaBaseEnvironment.jar -C bin .
@echo off

GOTO success

:no_swt
echo ERROR:  ..\..\SoarLibrary\bin\swt.jar is missing.  
echo         Did you remember to download it from: http://winter.eecs.umich.edu/jars/
echo Build Stopped
goto end

:no_sml
echo ERROR:  ..\..\SoarLibrary\bin\sml.jar is missing.  
echo Build Stopped
goto end

:success
echo Build Complete

:end
IF NOT "%1" == "--nopause" pause

