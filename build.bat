@echo off
setlocal EnableDelayedExpansion

if not exist user-env.bat (
	call :findexe python.exe
	if not "!retval!"=="fail" set PYTHON_HOME=!retval!
	
	call :findexe javac.exe
	if not "!retval!"=="fail" set JAVA_HOME=!retval:\bin=!
	
	call :findexe swig.exe
	if not "!retval!"=="fail" set SWIG_HOME=!retval!
	
	echo set PYTHON_HOME=!PYTHON_HOME!>>user-env.bat
	echo set JAVA_HOME=!JAVA_HOME!>>user-env.bat
	echo set SWIG_HOME=!SWIG_HOME!>>user-env.bat
) else (
	echo Reading local environment variables from user-env.bat
	call user-env.bat
)

if not exist %PYTHON_HOME%\python.exe (
	echo cannot locate python executable
	exit /B
)

set PATH=%PYTHON_HOME%;%JAVA_HOME%\bin;%SWIG_HOME%;%PATH%
%PYTHON_HOME%\python.exe scons\scons.py %*
exit /B

rem a "function" that tries to find an executable
:findexe
	set retval=fail
	set tempxx="%PATH:;=";"%"
	for %%i in (%tempxx%) do (
		if exist "%%i\%~1" (
			set retval=%%~i
			goto :EOF
		)
	)
	
	if "%~1"=="python.exe" (
		call :findpython
		if not "!retval!"=="fail" goto :EOF
	)
	
	call :askuser %~1
goto :EOF

rem Search for python. If python.exe not in PATH, search registry for installation directory.
rem In the case of multiple installations, the highest version number will be used
:findpython
	set retval=fail
	for /F %%i in ('reg query HKLM\SOFTWARE\Python\PythonCore') do set pyver=%%i
	if not X%pyver%==X (
		for /F "tokens=3" %%i in ('reg query %pyver%\InstallPath /ve') do (
			if exist %%i\python.exe set retval=%%i
		)
	)
goto :EOF

:askuser
	:whilebegin
	set tempdir=
	set /P tempdir=Enter directory that contains %~1, or nothing to ignore it:
	
	if "%tempdir%"=="" (
		set retval=fail
		goto :EOF
	)
	if not exist "%tempdir%\%~1" (
		echo it's not there
		goto :whilebegin
	)
	set retval=%tempdir%
goto :EOF
