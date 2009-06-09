@echo off

call "%VS90COMNTOOLS%\vsvars32.bat"

call %ANT_HOME%\bin\ant clean

devenv /rebuild "Distribution SCU|x64" SML9.sln
if not errorlevel 0 goto fail

devenv /build Release Tools\TestCSharpSML\TestCSharpSML9.sln
if not errorlevel 0 goto fail
exit 0

:fail
exit 1
