@echo off

call soar-vars.bat %1

rem Need to build Java if it exists, first.
if exist "Java\build.xml" @(

	ant -q "-Dsoarprefix=%SOAR_HOME%" "-Dversion=%SOAR_VERSION%" -f "Java\build.xml" %TARGET%

	for /f "tokens=* delims= " %%a in ('dir/b/ad') do @(
		if not %%a == Java (
			if exist "%%a\build.xml" (
				ant -q "-Dsoarprefix=%SOAR_HOME%" "-Dversion=%SOAR_VERSION%" -f "%%a\build.xml" %TARGET%
			)
		)
	)
)

