@echo off

rem %1 is an optional command line argument specifying the java bin directory to use to build everything
rem NOTE: if this directory is specified, the trailing slash must be included!
rem Also, if there are spaces in the path, it must be wrapped in quotes

@rem Step 1: Set up build environment
call "%VS80COMNTOOLS%\vsvars32.bat"

@rem Step 2: Build C++
devenv /rebuild "Distribution SCU" SML.sln
if not errorlevel 0 goto fail
devenv /build Release Tools\TestCSharpSML\TestCSharpSML.sln
if not errorlevel 0 goto fail

@rem Step 3: Build Java
call buildJavaApps.bat %1
if not errorlevel 0 goto fail
exit 0

:fail
exit 1
