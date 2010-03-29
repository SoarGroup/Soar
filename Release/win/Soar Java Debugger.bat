@echo off
set SOAR_HOME=%~dp0
set PATH=%SOAR_HOME%bin;%PATH%
start javaw -jar share\java\soar-debugger-9.3.0.jar

