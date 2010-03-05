@echo off
setlocal

call soar-vars.bat %1

if not defined ANT_HOME goto fail
if not defined JAVA_HOME goto fail
if not defined PYTHON_HOME goto fail
if not defined SWIG_HOME goto fail

call "%VS90COMNTOOLS%\vsvars32.bat"

if "%PROCESSOR_ARCHITECTURE%"=="AMD64" goto x64

if "%2" == "noscu" (
	set CONFIGURATION="Release|Win32"
) else (
	set CONFIGURATION="Release SCU|Win32"
)

devenv /%TARGET% %CONFIGURATION% Core\Core.sln
if not errorlevel 0 goto fail

goto built

:x64

if "%2" == "noscu" (
	set CONFIGURATION="Release|x64"
) else (
	set CONFIGURATION="Release SCU|x64"
)

devenv /%TARGET% %CONFIGURATION% Core\Core.sln
if not errorlevel 0 goto fail

:built

call build-java.bat %1

endlocal
exit /b 0

:fail
endlocal
exit /b 1

