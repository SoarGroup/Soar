@echo off
setlocal

echo In order for the rebuild-all to work correctly, the following environment
echo variables must be set:
echo ANT_HOME: %ANT_HOME%
echo JAVA_HOME: %JAVA_HOME%
echo PYTHON_HOME: %PYTHON_HOME%
echo SWIG_HOME: %SWIG_HOME%

if not defined ANT_HOME goto fail
if not defined JAVA_HOME goto fail
if not defined PYTHON_HOME goto fail
if not defined SWIG_HOME goto fail

call "%VS90COMNTOOLS%\vsvars32.bat"

if "%PROCESSOR_ARCHITECTURE%"=="AMD64" goto x64

devenv /rebuild "Distribution SCU|Win32" SML.sln
if not errorlevel 0 goto fail

goto built

:x64
devenv /rebuild "Distribution SCU|x64" SML.sln
if not errorlevel 0 goto fail

:built

devenv /build Release soar-csharp\TestCSharpSML.sln
if not errorlevel 0 goto fail

endlocal
exit /b 0

:fail
endlocal
exit /b 1

