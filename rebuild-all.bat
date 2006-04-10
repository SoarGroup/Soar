@rem Step 1: Set up build environment
call "%VS71COMNTOOLS%\vsvars32.bat"

@rem Step 2: Build C++
devenv /rebuild Release SML.sln

@rem Step 3: Build Java
call buildJavaApps.bat
