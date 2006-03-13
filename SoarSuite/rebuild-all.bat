@rem Step 1: Set up build environment
call "%pfroot%\Microsoft Visual Studio .NET 2003\Common7\Tools\vsvars32.bat"

@rem Step 2: Build C++
devenv /rebuild Release SML.sln

@rem Step 3: Build Java
call buildJavaApps.bat
