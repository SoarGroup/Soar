@echo off
PATH=%CD%\wintools;%PATH%
sh make-release.sh
echo sh returned with status %errorlevel%
pause
