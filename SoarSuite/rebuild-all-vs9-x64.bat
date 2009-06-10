@echo off

call "%VS90COMNTOOLS%\vsvars32.bat"

rem Cleaning Java projects here, they are built as a post-build
rem step inside of the ClientSMLJava project.
call %ANT_HOME%\bin\ant clean

devenv /rebuild "Distribution SCU|x64" SML9.sln
if not errorlevel 0 goto fail

devenv /build Release Tools\TestCSharpSML\TestCSharpSML9.sln
if not errorlevel 0 goto fail
exit 0

:fail
exit 1
