@echo off
setlocal

REM try to find python.exe in PATH
set tempxx="%PATH:;=";"%"
for %%i in (%tempxx%) do (
	if exist %%i\python.exe (
		set python=%%i\python.exe
		goto :found
	)
)

REM python.exe not in PATH, search registry for installation directory
REM In the case of multiple installations, the highest version number will be used
for /F %%i in ('reg query HKLM\SOFTWARE\Python\PythonCore') do set pyver=%%i
if X%pyver%==X goto :fail

for /F "tokens=3" %%i in ('reg query %pyver%\InstallPath /ve') do set pydir=%%i
if exist %pydir%\python.exe (
	set python=%pydir%\python.exe
	goto :found
)

:fail
echo cannot locate python executable
exit /B

:found
echo using %python%
%python% scons\scons.py %*
