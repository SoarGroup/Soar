@echo off
@rem TODO: exit with non-zero value on failure!

@rem Step 1: Set up build environment
call "%VS80COMNTOOLS%\vsvars32.bat"

@rem Step 2: Build C++
devenv /rebuild "Distribution SCU" SML.sln
if not errorlevel 0 goto fail
devenv /build Release Tools\TestCSharpSML\TestCSharpSML.sln
if not errorlevel 0 goto fail

@rem Step 3: Build Java
call buildJavaApps.bat
if not errorlevel 0 goto fail
exit 0

:fail
exit 1
