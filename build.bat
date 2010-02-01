@echo off
setlocal

set SOAR_HOME=%CD%\out
set SOAR_VERSION=9.2.0
if "%1" == "" (
	set TARGET=build
) else (
	set TARGET=%1
)

echo Make sure the following environment variables are set correctly:
echo.
echo ANT_HOME: %ANT_HOME%
echo JAVA_HOME: %JAVA_HOME%
echo PYTHON_HOME: %PYTHON_HOME%
echo SWIG_HOME: %SWIG_HOME%
echo.
echo Edit the build.bat file to check these variables:
echo SOAR_HOME: %SOAR_HOME%
echo SOAR_VERSION: %SOAR_VERSION%
echo.
echo TARGET: %TARGET%
echo.

if not defined ANT_HOME goto fail
if not defined JAVA_HOME goto fail
if not defined PYTHON_HOME goto fail
if not defined SWIG_HOME goto fail

call "%VS90COMNTOOLS%\vsvars32.bat"

if "%PROCESSOR_ARCHITECTURE%"=="AMD64" goto x64

devenv /%TARGET% "Release SCU|Win32" Core\Core.sln
if not errorlevel 0 goto fail

goto built

:x64
devenv /%TARGET% "Release SCU|x64" Core\Core.sln
if not errorlevel 0 goto fail

:built

for /f "tokens=* delims= " %%a in ('dir/b/ad') do @(
	if exist "%%a\build.xml" (
		ant "-Dsoarprefix=%SOAR_HOME%" "-Dversion=%SOAR_VERSION%" -f "%%a\build.xml" %TARGET%
	)
)

endlocal
exit /b 0

:fail
endlocal
exit /b 1

