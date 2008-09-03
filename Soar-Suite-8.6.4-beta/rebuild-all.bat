@rem TODO: exit with non-zero value on failure!

@rem Step 1: Set up build environment
call "%VS80COMNTOOLS%\vsvars32.bat"

@rem Step 2: Build C++
devenv /rebuild Distribution SML.sln
devenv /build Release Tools\TestCSharpSML\TestCSharpSML.sln
@rem devenv /rebuild Distribution-libs SML.sln

@rem Step 3: Build Java
call buildJavaApps.bat
