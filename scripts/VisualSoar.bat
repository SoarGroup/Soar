@echo off
set SOAR_HOME=%~dp0
set PATH=%SOAR_HOME%;%PATH%
start javaw -Djava.library.path=%SOAR_HOME% -jar VisualSoar.jar

