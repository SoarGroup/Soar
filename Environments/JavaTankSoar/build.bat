@echo off
IF NOT EXIST ..\..\SoarLibrary\bin\swt.jar GOTO no_swt
IF NOT EXIST ..\..\SoarLibrary\bin\sml.jar GOTO no_sml
IF NOT EXIST ..\..\SoarLibrary\bin\JavaBaseEnvironment.jar GOTO no_JBE

IF NOT EXIST bin mkdir bin

@echo on
javac -d bin -classpath ..\..\SoarLibrary\bin\swt.jar;..\..\SoarLibrary\bin\sml.jar;..\..\SoarLibrary\bin\JavaBaseEnvironment.jar -sourcepath source source\tanksoar\TankSoar.java
@echo off

xcopy /q /y source\* bin
IF NOT EXIST bin\images mkdir bin\images
xcopy /q /y /s source\images\* bin\images

@echo on
jar cfm JavaTankSoar.jar JarManifest -C bin .
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

