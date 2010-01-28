@echo off

if "%PROCESSOR_ARCHITECTURE%"=="AMD64" goto x64

echo Checking for 32-bit swt.jar
if not exist swt.jar goto fetch32
md5sum.exe --check --status swt32.md5sum
if %errorlevel% == 0 goto fetched
del swt.jar

:fetch32
echo Fetching swt.jar
wget.exe -q http://ai.eecs.umich.edu/~soar/sitemaker/misc/jars/win32/swt.jar
goto fetched

:x64
echo Checking for 64-bit swt.jar
if not exist swt.jar goto fetch64
md5sum.exe --check --status swt64.md5sum
if %errorlevel% == 0 goto fetched
del swt.jar

:fetch64
echo Fetching swt.jar
wget.exe -q http://ai.eecs.umich.edu/~soar/sitemaker/misc/jars/win64/swt.jar

:fetched

