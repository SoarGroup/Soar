@echo off
set SOAR_HOME=%~dp0
set PATH=%SOAR_HOME%bin;%PATH%
start javaw -jar share\java\soar-soar2d-9.3.1.jar soar2d\config\tanksoar.cnf

