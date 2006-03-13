@set pfroot=C:\Program Files (x86)
@set soarurl=https://winter.eecs.umich.edu/svn/soar/trunk/SoarSuite
@set distdir=SoarSuiteDist

@rem Step 1: Set up build environment
call "%pfroot%\Microsoft Visual Studio .NET 2003\Common7\Tools\vsvars32.bat"

@rem Step 2: Build C++
devenv /rebuild Release SML.sln

@rem Step 3: Build Java
call buildJavaApps.bat

@rem Step 4: Check out source from SVN
svn export %soarurl% ../%distdir%

@rem Step 5: Remove files that are not to be distributed with the release

@rem Step 6: Copy binaries over to dist
xcopy SoarLibrary/bin/*.dll ../%distdir%/SoarLibrary/bin
xcopy SoarLibrary/bin/*.exe ../%distdir%/SoarLibrary/bin
xcopy SoarLibrary/bin/*.jar ../%distdir%/SoarLibrary/bin
xcopy SoarLibrary/bin/*.lib ../%distdir%/SoarLibrary/bin
xcopy SoarLibrary/bin/Tcl_sml_ClientInterface ../%distdir%/SoarLibrary/bin/Tcl_sml_ClientInterface /I /Y

pause