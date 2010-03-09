@echo off

set SOAR_HOME=%CD%\out
set SOAR_VERSION=9.3.0
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
echo Edit the soar-vars.bat file to check these variables:
echo SOAR_HOME: %SOAR_HOME%
echo SOAR_VERSION: %SOAR_VERSION%
echo TARGET: %TARGET%
echo.

