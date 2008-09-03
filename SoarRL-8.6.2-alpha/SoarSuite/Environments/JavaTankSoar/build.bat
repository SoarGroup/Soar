@echo off
rem %2 is an optional command line argument specifying the java bin directory to use to build everything
rem NOTE: if this directory is specified, the trailing slash must be included!
rem Also, if there are spaces in the path, it must be wrapped in quotes

IF NOT EXIST ..\..\SoarLibrary\bin\swt.jar GOTO no_swt
IF NOT EXIST ..\..\SoarLibrary\bin\sml.jar GOTO no_sml
IF NOT EXIST ..\..\SoarLibrary\bin\JavaBaseEnvironment.jar GOTO no_JBE

IF NOT EXIST bin mkdir bin

%2javac -source 1.4 -d bin -classpath ..\..\SoarLibrary\bin\swt.jar;..\..\SoarLibrary\bin\sml.jar;..\..\SoarLibrary\bin\JavaBaseEnvironment.jar -sourcepath source source\tanksoar\TankSoar.java

xcopy /q /y source\* bin
IF NOT EXIST bin\images mkdir bin\images
xcopy /q /y /s source\images\* bin\images

@echo on
%2jar cfm JavaTankSoar.jar JarManifest -C bin .
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

:no_JBE
echo ERROR:  ..\..\SoarLibrary\bin\JavaBaseEnvironment.jar is missing.  
echo           Did you remember to build it?
echo Build Stopped
goto end

:success
echo Build Complete

:end
IF NOT "%1" == "--nopause" pause

