@echo off
set SOAR_HOME=%~dp0
set PATH=%SOAR_HOME%;%PATH%
start javaw -Djava.library.path=%SOAR_HOME% -jar SoarJavaDebugger.jar %1 %2 %3 %4 %5 
