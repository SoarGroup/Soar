@echo off
setlocal EnableDelayedExpansion

if not exist user-env.bat (
	call :findexe python.exe
	if not "!retval!"=="fail" (
		set PYTHON_HOME=!retval!
		echo python.exe found
	)
	
	call :findexe javac.exe
	if not "!retval!"=="fail" (
		set JAVA_HOME=!retval:\bin=!
		echo javac.exe found
	)
	
	call :findexe swig.exe
	if not "!retval!"=="fail" (
		set SWIG_HOME=!retval!
		echo swig.exe found
	)
	
	echo set PYTHON_HOME=!PYTHON_HOME!>>user-env.bat
	echo set JAVA_HOME=!JAVA_HOME!>>user-env.bat
	echo set SWIG_HOME=!SWIG_HOME!>>user-env.bat
	echo user-env.bat created. I will read directories from there next time.
) else (
	echo Reading local environment variables from user-env.bat
	call user-env.bat
)

echo PYTHON_HOME=%PYTHON_HOME%
echo JAVA_HOME=%JAVA_HOME%
echo SWIG_HOME=%SWIG_HOME%

if not exist %PYTHON_HOME%\python.exe (
	echo cannot locate python executable
	exit /B
)

set PATH=%PYTHON_HOME%;%JAVA_HOME%\bin;%SWIG_HOME%;%PATH%
%PYTHON_HOME%\python.exe scons\scons.py -Q %*

copy out\java\sml.jar ..\AgentDevelopmentTools\VisualSoar\lib\
pushd ..\AgentDevelopmentTools\VisualSoar
copy java\soar-visualsoar-snapshot.jar ..\..\SoarSuite\out\VisualSoar.jar
ant
popd

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
	) else if "%~1"=="javac.exe" (
		call :findjava
		if not "!retval!"=="fail" goto :EOF
	)
	
	call :askuser %~1
goto :EOF

rem Search for python in registry
:findpython
	set retval=fail
	for /F "tokens=1,2,*" %%i in ('reg query HKLM\SOFTWARE\Python\PythonCore /s') do (
		if exist %%k\python.exe set retval=%%k
	)
goto :EOF

rem Search for java in registry
:findjava
	set retval=fail
	for /F "tokens=1,2,*" %%i in ('reg query "HKLM\SOFTWARE\JavaSoft\Java Development Kit" /s') do (
		if exist %%k\bin\javac.exe set retval=%%k\bin
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
