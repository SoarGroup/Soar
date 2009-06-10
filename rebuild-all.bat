@echo off
setlocal

echo In order for the rebuild-all to work correctly, the following environment
echo variables must be set:
echo ANT_HOME: %ANT_HOME%
echo JAVA_HOME: %JAVA_HOME%
echo PYTHON_HOME: %PYTHON_HOME%
echo SWIG_HOME: %SWIG_HOME%
echo TCL_HOME: %TCL_HOME%

if not defined ANT_HOME goto fail
if not defined JAVA_HOME goto fail
if not defined PYTHON_HOME goto fail
if not defined SWIG_HOME goto fail
if not defined TCL_HOME goto fail

rem Cleaning Java projects here, they are built as a post-build
rem step inside of the ClientSMLJava project.
call %ANT_HOME%\bin\ant clean

call "%VS80COMNTOOLS%\vsvars32.bat"
devenv /rebuild "Distribution SCU|Win32" SML.sln
if not errorlevel 0 goto fail

devenv /build Release Tools\TestCSharpSML\TestCSharpSML.sln
if not errorlevel 0 goto fail
endlocal
exit /b 0

:fail
endlocal
exit /b 1

