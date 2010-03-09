@echo off
setlocal

call soar-vars.bat %1

if not defined ANT_HOME goto fail
if not defined JAVA_HOME goto fail
if not defined PYTHON_HOME goto fail
if not defined SWIG_HOME goto fail

call "%VS90COMNTOOLS%\vsvars32.bat"

if "%PROCESSOR_ARCHITECTURE%"=="AMD64" goto x64

set CONFSUB="Release|Win32"
if "%2" == "noscu" (
	set CONFIGURATION="Release|Win32"
) else (
	set CONFIGURATION="Release SCU|Win32"
)

goto buildit

:x64

set CONFSUB="Release|x64"
if "%2" == "noscu" (
	set CONFIGURATION="Release|x64"
) else (
	set CONFIGURATION="Release SCU|x64"
)

:buildit

devenv /%TARGET% %CONFIGURATION% Core\Core.sln

for /f "tokens=* delims= " %%a in ('dir/b/ad') do @(
	for /f "tokens=* delims= " %%b in ('dir/b %%a\*.sln 2^>nul') do (
		if not %%b == Core.sln (
			devenv /%TARGET% %CONFSUB% %%a\%%b
		)
	)
)

call build-java.bat %1

endlocal
exit /b 0

